/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * oauth.c - OAuth 1.0 for Flickr
 *
 * Copyright (C) 2011, David Beckett http://www.dajobe.org/
 * 
 * This file is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/*
 * flickcurl_base64_encode_digit:
 * @c: input digit 0..63
 *
 * INTERNAL - base64 encode a digit
 *
 * Note: this the output is not URL safe since '+' and '/' will need
 * %-escaping
 *
 * Return value: base64 encoded char of input value
 */
static char
flickcurl_base64_encode_digit(unsigned char c)
{
  if(c < 26)
    return 'A' + c;
  else if(c < 52)
    return 'a' + (c - 26);
  else if(c < 62)
    return '0' + (c - 52);
  else if(c == 62)
    return '+';
  else
    return '/';
}


/*
 * flickcurl_base64_encode:
 * @data: The data to base64 encode
 * @len: The size of the data in src
 * @out_len_p: pointer to store output length (or NULL)
 *
 * INTERNAL - Base64 encode data into a new string.
 *
 * Return value: base64 encoded string or NULL on failure
 */
char*
flickcurl_base64_encode(const unsigned char *data, size_t len,
                        size_t *out_len_p)
{
  char* out;
  char* p;
  unsigned int i;

  if(!data)
    return NULL;

  /* len + 1 to round up for partial sizes when (len % 3) is not 0 */
  out = (char*)calloc(sizeof(char), (len + 1) * 4/3 + 1);
  if(!out)
    return NULL;
  
  /* Encode 1-3 input bytes at a time (8, 16 or 24 input bits) into
   * 2-4 output chars
   */
  p = out;
  for(i = 0; i < len; i += 3) {
    unsigned char in_char_1 = data[i];
    unsigned char in_char_2 = ((i + 1) < len) ? data[i + 1] : 0;
    unsigned char in_char_3 = ((i + 2) < len) ? data[i + 2] : 0;
    unsigned char out_digit_1;
    unsigned char out_digit_2;
    unsigned char out_digit_3;
    unsigned char out_digit_4;

    out_digit_1 =   in_char_1 >> 2;
    out_digit_2 = ((in_char_1 & 0x03) << 4) | (in_char_2 >> 4);
    out_digit_3 = ((in_char_2 & 0x0f) << 2) | (in_char_3 >> 6);
    out_digit_4 =   in_char_3 & 0x3f;
    
    *p++ = flickcurl_base64_encode_digit(out_digit_1);
    *p++ = flickcurl_base64_encode_digit(out_digit_2);
      
    if((i + 1) < len)
      *p++ = flickcurl_base64_encode_digit(out_digit_3);
    else
      *p++ = '=';
      
    if((i + 2) < len)
      *p++ = flickcurl_base64_encode_digit(out_digit_4);
    else
      *p++ = '=';
  }

  *p = '\0';

  if(out_len_p)
    *out_len_p = p - out;

  return out;
}


/**
 * flickcurl_oauth_build_key:
 * @od: oauth data
 *
 * INTERNAL - Build OAuth 1.0 key
 *
 * KEY
 * http://tools.ietf.org/html/rfc5849#section-3.4.2
 * key = concat(client-credentials-secret, '&', token-credentials-secret) 
 *
 * Stores result in od->key and od->key_len
 *
 * Return value: non-0 on failure
 */
int
flickcurl_oauth_build_key(flickcurl_oauth_data* od)
{
  unsigned char *p;
  
  if(od->key)
    free(od->key);

  od->key_len = od->client_secret_len + 1 + od->token_secret_len;
  od->key = malloc(od->key_len + 1); /* for NUL */
  if(!od->key)
    return 1;
  
  p = od->key;
  if(od->client_secret_len) {
    memcpy(p, od->client_secret, od->client_secret_len);
    p += od->client_secret_len;
  }
  *p++ = '&';
  if(od->token_secret_len) {
    memcpy(p, od->token_secret, od->token_secret_len);
    p += od->token_secret_len;
  }
  *p = '\0'; /* Not part of HMAC-SHA1 data */
  
  return 0;
}


/**
 * flickcurl_oauth_build_key_data:
 * @od: oauth data
 *
 * INTERNAL - Build OAuth 1.0 key and data from parameters
 *
 * Builds key using flickcurl_oauth_build_key()
 *
 *
 * DATA
 * http://tools.ietf.org/html/rfc5849#section-3.4.1
 *
 * http-request-method:
 *   uppercase of method
 *
 * uri-base-string: scheme://authority/path (NO query or fragment)
 *   http://tools.ietf.org/html/rfc5849#section-3.4.1.2
 *
 * normalized-request-parameters:
 *   uri escaped
 *   http://tools.ietf.org/html/rfc5849#section-3.4.1.3.2
 *
 * data = concat(http-request-method, '&'
 *               uri-base-string, '&',
 *               normalized-request-parameters)
 *
 * Return value: non-0 on failure
 */
int
flickcurl_oauth_build_key_data(flickcurl_oauth_data* od,
                               const char* http_request_method,
                               const char* uri_base_string, 
                               const char* request_parameters)
{
  unsigned char *p;
  size_t s_len;
  char *escaped_s = NULL;

  if(flickcurl_oauth_build_key(od))
    return 1;
  
  od->data_len = strlen(http_request_method) +
    1 +
    (strlen(uri_base_string) * 3) + /* PESSIMAL; every char %-escaped */
    1 +
    (strlen(request_parameters) * 3); /* PESSIMAL */
  od->data = malloc(od->data_len + 1); /* for NUL */
  
  if(!od->data)
    return 1;

  /* Prepare data */
  p = od->data;
  s_len = strlen(http_request_method);
  memcpy(p, http_request_method, s_len);
  p += s_len;
  
  *p++ = '&';
  
  escaped_s = curl_escape(uri_base_string, strlen(uri_base_string));
  s_len = strlen(escaped_s);
  memcpy(p, escaped_s, s_len);
  p += s_len;
  curl_free(escaped_s);
  
  *p++ = '&';
  
  escaped_s = curl_escape(request_parameters, strlen(request_parameters));
  s_len = strlen(escaped_s);
  memcpy(p, escaped_s, s_len);
  p += s_len;
  curl_free(escaped_s);
  
  *p = '\0'; /* Not part of HMAC-SHA1 data */
  /* calculate actual data len */
  od->data_len = p - od->data;
  
  return 0;
}


/**
 * flickcurl_oauth_build_key_data:
 * @od: oauth data
 * @len_p: pointer to store size of result
 *
 * INTERNAL - Compute OAuth signature over data prepared by flickcurl_oauth_build_key()
 *
 * Result: signature string or NULL on failure
 *
 */
char*
flickcurl_oauth_compute_signature(flickcurl_oauth_data* od, size_t* len_p)
{
  unsigned char *s1;
  char *result;
  
  s1 = flickcurl_hmac_sha1(od->data, od->data_len, od->key, od->key_len);
  if(!s1)
    return NULL;
  
  result = flickcurl_base64_encode(s1, SHA1_DIGEST_LENGTH, len_p);
  free(s1);
  
  return result;
}
