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
    *out_len_p = p - out + 1;

  return out;
}
