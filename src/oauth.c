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

/* Only used in the test in this file so needs to be in the library */
int flickcurl_oauth_build_key(flickcurl_oauth_data* od);


#ifndef STANDALONE

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
static char*
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


/*
 * flickcurl_oauth_free:
 * @od: oauth data
 *
 * INTERNAL - Free OAuth data
 *
 */
void
flickcurl_oauth_free(flickcurl_oauth_data* od)
{
  if(od->client_key)
    free(od->client_key);
  
  if(od->client_secret)
    free(od->client_secret);
  
  if(od->request_token)
    free(od->request_token);
  
  if(od->request_token_secret)
    free(od->request_token_secret);
  
  /* od->verifier always shared */
  
  if(od->token)
    free(od->token);
  
  if(od->token_secret)
    free(od->token_secret);
  
  /* od->callback always shared */
  
  if(od->nonce)
    free(od->nonce);
  
  if(od->key)
    free(od->key);
  
  if(od->data)
    free(od->data);
}


/*
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

  od->key_len = od->client_secret_len + 1;
  if(od->request_token_secret_len)
    od->key_len += od->request_token_secret_len;
  else
    od->key_len += od->token_secret_len;

  od->key = malloc(od->key_len + 1); /* for NUL */
  if(!od->key)
    return 1;
  
  p = od->key;
  if(od->client_secret_len) {
    memcpy(p, od->client_secret, od->client_secret_len);
    p += od->client_secret_len;
  }
  *p++ = '&';
  if(od->request_token_secret_len) {
    memcpy(p, od->request_token_secret, od->request_token_secret_len);
    p += od->request_token_secret_len;
  } else if(od->token_secret_len) {
    memcpy(p, od->token_secret, od->token_secret_len);
    p += od->token_secret_len;
  }
  *p = '\0'; /* Not part of HMAC-SHA1 data */
  
  return 0;
}


/*
 * flickcurl_oauth_compute_signature:
 * @od: oauth data
 * @len_p: pointer to store size of result
 *
 * INTERNAL - Compute OAuth signature over 'key' and 'data' fields of @od
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


/*
 * flickcurl_oauth_prepare_common:
 * ...
 *
 * INTERNAL - prepare an oauth request
 */
int
flickcurl_oauth_prepare_common(flickcurl *fc,
                               const char* url,
                               const char* method,
                               const char* upload_field,
                               const char* upload_value,
                               const char* parameters[][2], int count,
                               int parameters_in_url, int need_auth)
{
  flickcurl_oauth_data* od = &fc->od;
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
    is_oauth_method = !strncmp(method, "flickr.oauth.", 13);
  } else
    fc->method = NULL;

  /* OAuth parameters
   *
   * oauth_callback         <URL> or "oob" [request token request]
   * oauth_consumer_key     API key
   * oauth_nonce            <random value - different each time>
   * oauth_signature        [ADDED AFTER COMPUTING]
   * oauth_signature_method "HMAC-SHA1"
   * oauth_timestamp        <value of gettimeofday()>
   * oauth_version          "1.0"
   *
   * oauth_verifier         verifier [access token request]
   * oauth_token            access token or request token
   */

  if(fc->method && !is_oauth_method) {
    parameters[count][0]  = "method";
    parameters[count++][1]= fc->method;
  }

  if(od->callback) {
    parameters[count][0]  = "oauth_callback";
    parameters[count++][1]= od->callback;
  }
  
  parameters[count][0]  = "oauth_consumer_key";
  parameters[count++][1]= od->client_key;

  nonce = (char*)od->nonce;
  if(!nonce) {
    nonce = (char*)malloc(20);
    free_nonce = 1;
    sprintf(nonce, "%ld", mtwist_u32rand(fc->mt));
  }
  parameters[count][0]  = "oauth_nonce";
  parameters[count++][1]= nonce;

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
  parameters[count][0]  = "oauth_timestamp";
  parameters[count++][1]= timestamp;

  parameters[count][0]  = "oauth_version";
  parameters[count++][1]= "1.0";

  if(od->token) {
    parameters[count][0]  = "oauth_token";
    parameters[count++][1]= od->token;
  } else if(od->request_token) {
    parameters[count][0]  = "oauth_token";
    parameters[count++][1]= od->request_token;
  }
  if(od->verifier) {
    parameters[count][0]  = "oauth_verifier";
    parameters[count++][1]= od->verifier;
  }

  parameters[count][0]  = NULL;

  /* +FLICKCURL_FLICKCURL_MAX_OAUTH_PARAM_COUNT for oauth fields +1 for NULL terminating pointer */
  fc->param_fields = (char**)calloc(count + FLICKCURL_MAX_OAUTH_PARAM_COUNT + 1, sizeof(char*));
  fc->param_values = (char**)calloc(count + FLICKCURL_MAX_OAUTH_PARAM_COUNT + 1, sizeof(char*));
  values_len       = (size_t*)calloc(count + FLICKCURL_MAX_OAUTH_PARAM_COUNT + 1, sizeof(size_t));

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
    od->key = NULL;

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
    od->data = NULL;
    od->data_len = 0;
    
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



/*
 * flickcurl_oauth_request_token:
 * @fc: flickcurl object
 *
 * INTERNAL - get a Flickr OAuth request token
 *
 * Calls the Flickr OAuth endpoint to get a request token.
 *
 * Stores the request token in @od fields 'request_token' and
 * 'request_token_secret' on success.
 *
 * Return value: non-0 on failure
 */
int
flickcurl_oauth_request_token(flickcurl* fc)
{
  flickcurl_oauth_data* od = &fc->od;
  const char * parameters[2 + FLICKCURL_MAX_OAUTH_PARAM_COUNT][2];
  int count = 0;
  char* request_token = NULL;
  char* request_token_secret = NULL;
  char** form = NULL;
  int rc = 0;
  const char* uri = fc->oauth_request_token_uri;
  int i;

  parameters[count][0]  = NULL;

  /* Require signature */
  flickcurl_set_sign(fc);

  od->callback = "oob";
  rc = flickcurl_oauth_prepare_common(fc,
                                      uri,
                                      /* method */ "flickr.oauth.request_token",
                                      /* upload_field */ NULL,
                                      /* upload_value */ NULL,
                                      parameters, count,
                                      /* parameters_in_url */ 1,
                                      /* need_auth */ 1);
  od->callback = NULL;

  if(rc)
    goto tidy;

  form = flickcurl_invoke_get_form_content(fc, &count);
  if(!form) {
    rc = 1;
    goto tidy;
  }

#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "OAuth request token request %s response was %d params\n",
          uri, count);
#endif

  for(i = 0; i < (2 * count); i += 2) {
    if(!strcmp(form[i], "oauth_token")) {
      request_token = form[i+1];
    } else if(!strcmp(form[i], "oauth_token_secret")) {
      request_token_secret = form[i+1];
    }
  }

  if(request_token && request_token_secret) {
    /* Take copies that are owned by od */
    od->request_token = strdup(request_token);
    od->request_token_len = strlen(od->request_token);
    od->request_token_secret = strdup(request_token_secret);
    od->request_token_secret_len = strlen(od->request_token_secret);

#ifdef FLICKCURL_DEBUG
    fprintf(stderr,
            "OAuth request token returned token '%s' secret token '%s'\n",
            od->request_token, od->request_token_secret);
#endif
  } else
    rc = 1;
  
  tidy:
  if(form)
    flickcurl_free_form(form, count);
  
  return rc;
}


/*
 * flickcurl_oauth_get_authorize_uri:
 * @fc: flickcurl object
 *
 * INTERNAL - get the URL for the user to authorize an application
 *
 * Forms the URL the user needs to start at to authorize the
 * application.  The application should pass the verifier to
 * flickcurl_oauth_access_token() for the final step in OAuth.
 *
 * Return value: authorize URI or NULL on failure
 */
char*
flickcurl_oauth_get_authorize_uri(flickcurl* fc)
{
  flickcurl_oauth_data* od = &fc->od;
#define PARAM_LEN 13
  const char* param = "?oauth_token=";
  size_t len;
  char* uri;
  char *p;

  if(!od->request_token)
    return NULL;
  
  len = strlen(flickcurl_flickr_oauth_authorize_uri);
  uri = (char*)malloc(len + PARAM_LEN + od->request_token_len + 1);
  if(!uri)
    return NULL;

  p = uri;
  memcpy(p, flickcurl_flickr_oauth_authorize_uri, len);
  p += len;
  memcpy(p, param, PARAM_LEN);
  p += PARAM_LEN;
  memcpy(p, od->request_token, od->request_token_len);
  p += od->request_token_len;
  *p = '\0';
  
  return uri;
}


/*
 * flickcurl_oauth_access_token:
 * @fc: flickcurl object
 * @verifier: verifier from OOB authentication
 *
 * INTERNAL - get a Flickr OAuth access token from a verifier
 *
 * Calls the Flickr OAuth access token endpoint using the verifier
 * from out of band authentication to get an access token.
 *
 * Uses the @verifier and the @od fields 'request_token' and
 * 'request_token_secret' to get an access token stored which on
 * success is stored in the @od fields 'access_token' and
 * 'access_token_secret'.  The request token fields are deleted on
 * success.
 *
 * Return value: non-0 on failure
 */
int
flickcurl_oauth_access_token(flickcurl* fc, const char* verifier)
{
  flickcurl_oauth_data* od = &fc->od;
  const char * parameters[2 + FLICKCURL_MAX_OAUTH_PARAM_COUNT][2];
  int count = 0;
  char* access_token = NULL;
  char* access_token_secret = NULL;
  char** form = NULL;
  int rc = 0;
  const char* uri = fc->oauth_access_token_uri;
  int i;
  
  parameters[count][0]  = NULL;

  /* Require signature */
  flickcurl_set_sign(fc);

  od->verifier = verifier;
  od->verifier_len = strlen(verifier);

  rc = flickcurl_oauth_prepare_common(fc,
                                      uri,
                                      /* method */ "flickr.oauth.access_token",
                                      /* upload_field */ NULL,
                                      /* upload_value */ NULL,
                                      parameters, count,
                                      /* parameters_in_url */ 1,
                                      /* need_auth */ 1);

  od->verifier = NULL;
  od->verifier_len = 0;
  if(rc)
    goto tidy;

  form = flickcurl_invoke_get_form_content(fc, &count);
  if(!form) {
    rc = 1;
    goto tidy;
  }

#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "OAuth access token request %s response was %d params\n", 
          uri, count);
#endif

  for(i = 0; i < (2 * count); i += 2) {
    if(!strcmp(form[i], "oauth_token")) {
      access_token = form[i+1];
    } else if(!strcmp(form[i], "oauth_token_secret")) {
      access_token_secret = form[i+1];
    }
    /* ignoring: fullname, user_nsid, username */
  }

  if(access_token && access_token_secret) {
    /* Take copies that are owned by od */
    od->token = strdup(access_token);
    od->token_len = strlen(od->token);
    od->token_secret = strdup(access_token_secret);
    od->token_secret_len = strlen(od->token_secret);

    /* Delete temporary request token and secret */
    free(od->request_token);
    od->request_token = NULL;
    od->request_token_len = 0;

    free(od->request_token_secret);
    od->request_token_secret = NULL;
    od->request_token_secret_len = 0;

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, 
            "OAuth access token returned token '%s' secret token '%s'\n",
            od->token, od->token_secret);
#endif
  } else
    rc = 1;
  
  tidy:
  if(form)
    flickcurl_free_form(form, count);
  
  return rc;
}
#endif


#ifdef STANDALONE
#include <stdio.h>

int main(int argc, char *argv[]);


/* Test KEY fields */
static const char* test_client_secret = "a9567d986a7539fe";
static const char* test_token_secret  = NULL;

/* Test DATA fields */
static const char* test_http_request_method = "GET";
static const char* test_uri_base_string     = "http://www.flickr.com/services/oauth/request_token";
static const char* test_oauth_callback_url  = "http://www.example.com";
static const char* test_oauth_consumer_key  = "653e7a6ecc1d528c516cc8f92cf98611";
static const char* test_oauth_nonce         = "95613465";
static const time_t test_oauth_timestamp     = (time_t)1305586162;

static const char* test_request_parameters  = "oauth_callback=http%3A%2F%2Fwww.example.com&oauth_consumer_key=653e7a6ecc1d528c516cc8f92cf98611&oauth_nonce=95613465&oauth_signature_method=HMAC-SHA1&oauth_timestamp=1305586162&oauth_version=1.0";

/* Expected results */
static const char* expected_key = 
      "a9567d986a7539fe"
      "&";
static const char* expected_data =
      "GET" "&"
      "http%3A%2F%2Fwww.flickr.com%2Fservices%2Foauth%2Frequest_token" "&"
      "oauth_callback%3Dhttp%253A%252F%252Fwww.example.com%26oauth_consumer_key%3D653e7a6ecc1d528c516cc8f92cf98611%26oauth_nonce%3D95613465%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D1305586162%26oauth_version%3D1.0";

static const char* expected_signature = "7w18YS2bONDPL%2FzgyzP5XTr5af4%3D";

static const char* program;


static void
oauth_init_test_secrets(flickcurl_oauth_data *od)
{
  /* Set up test data */
  od->client_secret = (char*)test_client_secret;
  if(od->client_secret)
    od->client_secret_len = strlen(od->client_secret);

  od->token_secret = (char*)test_token_secret;
  if(od->token_secret)
    od->token_secret_len = strlen(od->token_secret);
}


static int
test_request_token(flickcurl* fc) 
{
  flickcurl_oauth_data od;
  int rc;
  
  memset(&od, '\0', sizeof(od));

  od.callback = test_oauth_callback_url;
  od.client_key = (char*)test_oauth_consumer_key;
  od.client_key_len = strlen(od.client_key);
  od.nonce = test_oauth_nonce;
  od.timestamp = test_oauth_timestamp;

  oauth_init_test_secrets(&od);

  rc = flickcurl_oauth_request_token(fc, &od);

  return rc;
}


static int
test_access_token(flickcurl* fc) 
{
  flickcurl_oauth_data od;
  int rc;
  const char* verifier = "123-456-789";
  
  memset(&od, '\0', sizeof(od));

  od.callback = test_oauth_callback_url;
  od.client_key = (char*)test_oauth_consumer_key;
  od.client_key_len = strlen(od.client_key);
  od.nonce = test_oauth_nonce;
  od.timestamp = test_oauth_timestamp;
  
  oauth_init_test_secrets(&od);

  rc = flickcurl_oauth_access_token(fc, &od, verifier);

  return rc;
}


static int
test_oauth_build_key_data(flickcurl_oauth_data* od,
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


static int
test_signature_calc(flickcurl* fc)
{
  char *s = NULL;
  char *escaped_s = NULL;
  size_t escaped_s_len;
  char* signature;
  flickcurl_oauth_data od;
  int rc;

  memset(&od, '\0', sizeof(od));
  
  oauth_init_test_secrets(&od);
  
  rc = test_oauth_build_key_data(&od, test_http_request_method,
                                 test_uri_base_string, 
                                 test_request_parameters);
  
  fprintf(stderr, "%s: key is (%d bytes)\n  %s\n", program, (int)od.key_len, od.key);
  fprintf(stderr, "%s: expected key is\n  %s\n", program, expected_key);
  
  fprintf(stderr, "%s: data is (%d bytes)\n  %s\n", program, (int)od.data_len, od.data);
  fprintf(stderr, "%s: expected data is\n  %s\n", program, expected_data);
  
  signature = flickcurl_oauth_compute_signature(&od, &escaped_s_len);
  
  escaped_s = curl_escape((char*)signature, 0);
  free(signature);
  escaped_s_len = strlen(escaped_s);
  
  fprintf(stdout, "URI Escaped result (%d bytes):\n   %s\n",
          (int)escaped_s_len, escaped_s);
  
  fprintf(stdout, "Expected URI escaped result\n   %s\n", 
          expected_signature);
  
  curl_free(escaped_s);
  
  if(s)
    free(s);
  if(od.data)
    free(od.data);
  if(od.key)
    free(od.key);

  return rc;
}


static void
my_message_handler(void *user_data, const char *message)
{
  fprintf(stderr, "%s: ERROR: %s\n", program, message);
}


int
main(int argc, char *argv[])
{
  flickcurl *fc = NULL;
  int failures = 0;
  
  program = "flickcurl_oauth_test"; /* No raptor_basename */

  flickcurl_init();

  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    failures++;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(0) {
    failures += test_request_token(fc);
    failures += test_access_token(fc);
  }
  failures += test_signature_calc(fc);

  tidy:
  if(fc)
    flickcurl_free(fc);

  return failures;
}
#endif
