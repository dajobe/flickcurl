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


static int
compare_args(const void *a, const void *b) 
{
  return strcmp(*(char**)a, *(char**)b);
}


static void
flickcurl_sort_args(flickcurl *fc, const char *parameters[][2], int count)
{
  qsort((void*)parameters, count, sizeof(char*[2]), compare_args);
}


int
flickcurl_oauth_prepare_common(flickcurl *fc, flickcurl_oauth_data* od,
                               const char* url,
                               const char* method,
                               const char* upload_field,
                               const char* upload_value,
                               const char* parameters[][2], int count,
                               int parameters_in_url, int need_auth,
                               int is_request)
{
  int i;
  char *signature_string = NULL;
  size_t* values_len = NULL;
  unsigned int fc_uri_len = 0;
  char* nonce = NULL;
  int free_nonce = 0;
  char* timestamp = NULL;
  int rc = 0;
  int need_to_add_query = 0;
  const char* http_method = "GET";
  int is_oauth_method = 0;

  if(!url || !parameters)
    return 1;
  
  /* If one is given, both are required */
  if((upload_field || upload_value) && (!upload_field || !upload_value))
    return 1;
  
  fc->failed = 0;
  fc->error_code = 0;
  if(fc->error_msg) {
    free(fc->error_msg);
    fc->error_msg = NULL;
  }
  /* Default to read */
  fc->is_write = 0;
  /* Default to no data */
  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
    fc->data = NULL;
    fc->data_length = 0;
    fc->data_is_xml = 0;
  }
  if(fc->param_fields) {
    for(i = 0; fc->param_fields[i]; i++) {
      free(fc->param_fields[i]);
      free(fc->param_values[i]);
    }
    free(fc->param_fields);
    free(fc->param_values);
    fc->param_fields = NULL;
    fc->param_values = NULL;
    fc->parameter_count = 0;
  }
  if(fc->upload_field) {
    free(fc->upload_field);
    fc->upload_field = NULL;
  }
  if(fc->upload_value) {
    free(fc->upload_value);
    fc->upload_value = NULL;
  }
  
  if(fc->method)
    free(fc->method);
  if(method) {
    fc->method = strdup(method);
    is_oauth_method = !strncmp(method, "oauth.", 6);
  } else
    fc->method = NULL;

  /* OAuth parameters
   *
   * oauth_callback         <URL> or "oob" [request token request]
   * oauth_consumer_key     API key
   * oauth_nonce            <random value - different each time> [request token request]
   * oauth_signature        [ADDED AFTER COMPUTING]
   * oauth_signature_method "HMAC-SHA1"
   * oauth_timestamp        <value of gettimeofday()> [request token request]
   * oauth_version          "1.0"
   *
   * oauth_verifier         verifier [access token request]
   * oauth_token            access token [access token request]
   */

  if(fc->method && !is_oauth_method) {
    parameters[count][0]  = "method";
    parameters[count++][1]= fc->method;
  }

  if(is_request) {
    parameters[count][0]  = "oauth_callback";
    parameters[count++][1]= (od->callback ? od->callback : "oob");
  }
  
  parameters[count][0]  = "oauth_consumer_key";
  parameters[count++][1]= od->client_key;

  if(is_request) {
    nonce = (char*)od->nonce;
    if(!nonce) {
      nonce = (char*)malloc(20);
      free_nonce = 1;
      sprintf(nonce, "%ld", mtwist_u32rand(fc->mt));
    }
    parameters[count][0]  = "oauth_nonce";
    parameters[count++][1]= nonce;
  }

  /* oauth_signature - computed over these fields */
  parameters[count][0]  = "oauth_signature_method";
  parameters[count++][1]= "HMAC-SHA1";

  timestamp = (char*)malloc(20);
  if(od->timestamp)
    sprintf(timestamp, "%ld", (long)od->timestamp);
  else {
    struct timeval tp;
    (void)gettimeofday(&tp, NULL);
    sprintf(timestamp, "%ld", (long)tp.tv_sec);
  }
  if(is_request) {
    parameters[count][0]  = "oauth_timestamp";
    parameters[count++][1]= timestamp;
  }

  parameters[count][0]  = "oauth_version";
  parameters[count++][1]= "1.0";

  if(od->tmp_token) {
    parameters[count][0]  = "oauth_token";
    parameters[count++][1]= od->tmp_token;
  }
  if(od->verifier) {
    parameters[count][0]  = "oauth_verifier";
    parameters[count++][1]= od->verifier;
  }

  parameters[count][0]  = NULL;

  /* +MAX_OAUTH_PARAM_COUNT for oauth fields +1 for NULL terminating pointer */
  fc->param_fields = (char**)calloc(count + MAX_OAUTH_PARAM_COUNT + 1, sizeof(char*));
  fc->param_values = (char**)calloc(count + MAX_OAUTH_PARAM_COUNT + 1, sizeof(char*));
  values_len       = (size_t*)calloc(count + MAX_OAUTH_PARAM_COUNT + 1, sizeof(size_t));

  if((need_auth && (od->client_secret || od->token_secret)) || fc->sign)
    flickcurl_sort_args(fc, parameters, count);


  fc_uri_len = strlen(url);
  if(url[fc_uri_len -1] != '?')
    need_to_add_query++;
  
  /* Save away the parameters and calculate the value lengths */
  for(i = 0; parameters[i][0]; i++) {
    size_t param_len = strlen(parameters[i][0]);

    if(parameters[i][1])
      values_len[i] = strlen(parameters[i][1]);
    else {
      values_len[i] = 0;
      parameters[i][1] = "";
    }
    fc->param_fields[i] = (char*)malloc(param_len + 1);
    strcpy(fc->param_fields[i], parameters[i][0]);
    fc->param_values[i] = (char*)malloc(values_len[i] + 1);
    strcpy(fc->param_values[i], parameters[i][1]);

    /* 3x value len is conservative URI %XX escaping on every char */
    fc_uri_len += param_len + 1 /* = */ + 3 * values_len[i];
  }

  if(upload_field) {
    fc->upload_field = (char*)malloc(strlen(upload_field) + 1);
    strcpy(fc->upload_field, upload_field);

    fc->upload_value = (char*)malloc(strlen(upload_value) + 1);
    strcpy(fc->upload_value, upload_value);
  }


  if(((need_auth && (od->client_secret || od->token_secret))) ||
     fc->sign) {
    char *buf = NULL;
    size_t buf_len = 0;
    char *param_buf = NULL;
    size_t param_buf_len = 0;
    size_t vlen = 0;
    char *escaped_value = NULL;
    
    for(i = 0; parameters[i][0]; i++)
      param_buf_len += strlen(parameters[i][0]) + 3 + (3 * values_len[i]) + 3;
    param_buf = (char*)malloc(param_buf_len + 1);
    *param_buf = '\0';
    
    for(i = 0; parameters[i][0]; i++) {
      if(i > 0)
        strcat(param_buf, "&");
      strcat(param_buf, parameters[i][0]);
      strcat(param_buf, "=");
      escaped_value = curl_escape(parameters[i][1], 0);
      strcat(param_buf, escaped_value);
      curl_free(escaped_value);
    }

    buf_len = strlen(http_method);
    buf_len += 1; /* & */
    buf_len += (3 * strlen(url));
    buf_len += 1; /* & */
    buf_len += param_buf_len * 3;

    buf = (char*)malloc(buf_len + 1);
    strcpy(buf, http_method);
    strcat(buf, "&");
    escaped_value = curl_escape(url, 0);
    strcat(buf, escaped_value);
    curl_free(escaped_value);
    strcat(buf, "&");
    escaped_value = curl_escape(param_buf, 0);
    strcat(buf, escaped_value);
    curl_free(escaped_value);

    free(param_buf);

    if(flickcurl_oauth_build_key(od)) {
#ifdef FLICKCURL_DEBUG
      fprintf(stderr, "flickcurl_oauth_build_key() failed\n");
#endif
      rc = 1;
      goto tidy;
    }

    /* build data */
    od->data = (unsigned char*)buf;
    od->data_len = strlen((const char*)od->data);

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "data for signature (%d bytes)\n   %s\n", 
            (int)od->data_len, (char*)od->data);
#endif
    signature_string = flickcurl_oauth_compute_signature(od, &vlen);

    /* set by flickcurl_oauth_build_key() above */
    free(od->key);

    parameters[count][0]  = "oauth_signature";
    parameters[count][1]  = signature_string;

    /* Add a new parameter pair */
    values_len[count] = vlen;
    /* 15 = strlen(oauth_signature) */
    fc->param_fields[count] = (char*)malloc(15 + 1);
    strcpy(fc->param_fields[count], parameters[count][0]);
    fc->param_values[count] = (char*)malloc(vlen + 1);
    strcpy(fc->param_values[count], parameters[count][1]);

    fc_uri_len += 15 /* "oauth_signature" */ + 1 /* = */ + vlen;

    count++;
    
#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "HMAC-SHA1 signature:\n  %s\n", signature_string);
#endif
    
    free(buf);
    
    parameters[count][0] = NULL;
  }

  /* add &s between parameters */
  fc_uri_len += count-1;

  /* reuse or grow uri buffer */
  if(fc->uri_len < fc_uri_len) {
    free(fc->uri);
    fc->uri = (char*)malloc(fc_uri_len+1);
    fc->uri_len = fc_uri_len;
  }
  strcpy(fc->uri, url);

  if(need_to_add_query)
    strcat(fc->uri, "?");

  if(parameters_in_url) {
    for(i = 0; parameters[i][0]; i++) {
      char *value = (char*)parameters[i][1];
      char *escaped_value = NULL;

      if(!parameters[i][1])
        continue;

      strcat(fc->uri, parameters[i][0]);
      strcat(fc->uri, "=");
      escaped_value = curl_escape(value, values_len[i]);
      strcat(fc->uri, escaped_value);
      curl_free(escaped_value);
      strcat(fc->uri, "&");
    }

    /* zap last & */
    fc->uri[strlen(fc->uri)-1] = '\0';
  }

#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "Request URI:\n  %s\n", fc->uri);

  FLICKCURL_ASSERT((strlen(fc->uri) == fc_uri_len),
                   "Final URI does not match expected length");
#endif

  tidy:
  if(signature_string)
    free(signature_string);

  if(values_len)
    free(values_len);

  if(nonce && free_nonce)
    free(nonce);

  if(timestamp)
    free(timestamp);

  return rc;
}


