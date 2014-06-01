/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * common.c - Flickcurl common functions
 *
 * Copyright (C) 2007-2012, David Beckett http://www.dajobe.org/
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

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

/* for access() and R_OK */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef OFFLINE
#ifdef HAVE_RAPTOR
#include <raptor2.h>
#endif
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>

#include <libxml/xmlsave.h>

const char* const flickcurl_short_copyright_string = "Copyright 2007-2014 David Beckett.";

const char* const flickcurl_copyright_string = "Copyright (C) 2007-2014 David Beckett - http://www.dajobe.org/";

const char* const flickcurl_license_string = "LGPL 2.1 or newer, GPL 2 or newer, Apache 2.0 or newer.\nSee http://librdf.org/flickcurl/ for full terms.";

const char* const flickcurl_home_url_string = "http://librdf.org/flickcurl/";
const char* const flickcurl_version_string = VERSION;


const char* const flickcurl_flickr_service_uri =  "https://api.flickr.com/services/rest/";
const char* const flickcurl_flickr_upload_service_uri =  "https://up.flickr.com/services/upload/";
const char* const flickcurl_flickr_replace_service_uri =  "https://up.flickr.com/services/replace/";
const char* const flickcurl_flickr_oauth_request_token_uri =  "https://www.flickr.com/services/oauth/request_token";
const char* const flickcurl_flickr_oauth_authorize_uri =  "https://www.flickr.com/services/oauth/authorize";
const char* const flickcurl_flickr_oauth_access_token_uri =  "https://www.flickr.com/services/oauth/access_token";


static void
flickcurl_error_varargs(flickcurl* fc, const char *message, 
                        va_list arguments)
{
  if(fc && fc->error_handler) {
    char *buffer = my_vsnprintf(message, arguments);
    if(!buffer) {
      fprintf(stderr, "flickcurl: Out of memory\n");
      return;
    }
    fc->error_handler(fc->error_data, buffer);
    free(buffer);
  } else {
    fprintf(stderr, "flickcurl error - ");
    vfprintf(stderr, message, arguments);
    fputc('\n', stderr);
  }
}

  
void
flickcurl_error(flickcurl* fc, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);
  flickcurl_error_varargs(fc, message, arguments);
  va_end(arguments);
}

  
static size_t
flickcurl_write_callback(void *ptr, size_t size, size_t nmemb, 
                         void *userdata) 
{
  flickcurl* fc = (flickcurl*)userdata;
  int len = size*nmemb;
  int rc = 0;
  
  if(fc->failed)
    return 0;

  fc->total_bytes += len;

  if(fc->save_content) {
    char *b;
    flickcurl_chunk *chunk;
    
    b = (char*)malloc(len);
    chunk = (flickcurl_chunk*)malloc(sizeof(*chunk));
    if(b && chunk) {
      fc->chunks_count++;

      memcpy(b, ptr, len);
      chunk->content = b;
      chunk->size = len;
      chunk->prev= fc->chunks;

      fc->chunks = chunk;
    } else {
      if(b)
        free(b);
      if(chunk)
        free(chunk);
      flickcurl_error(fc, "Out of memory");
    }
      
  }
  
  if(fc->xml_parse_content) {
    if(!fc->xc) {
      xmlParserCtxtPtr xc;

      xc = xmlCreatePushParserCtxt(NULL, NULL,
                                   (const char*)ptr, len,
                                   (const char*)fc->uri);
      if(!xc)
        rc = 1;
      else {
        xc->replaceEntities = 1;
        xc->loadsubset = 1;
      }
      fc->xc = xc;
    } else
      rc = xmlParseChunk(fc->xc, (const char*)ptr, len, 0);

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "Got >>%s<< (%d bytes)\n", (const char*)ptr, len);
#endif

    if(rc)
      flickcurl_error(fc, "XML Parsing failed");
  }

#ifdef CAPTURE
  if(fc->fh)
    fwrite(ptr, size, nmemb, fc->fh);
#endif
  return len;
}


#if FLICKCURL_DEBUG > 1
static int
flickcurl_debug_callback(CURL *handle, curl_infotype type,
                         char *data, size_t size, void *userptr)
{
  if(type == CURLINFO_TEXT) {
    fprintf(stderr, "INFO %s", data);
  } else {
    char dir = (type == CURLINFO_HEADER_IN || type == CURLINFO_DATA_IN || type == CURLINFO_SSL_DATA_IN) ? '<' : '>';
    fprintf(stderr, "%c %s", dir, data);
  }
  return 0;
}
#endif


/**
 * flickcurl_new_with_handle:
 * @curl_handle: CURL* handle
 *
 * Create a Flickcurl sesssion from an existing CURL* handler
 *
 * This allows setting up or re-using an existing CURL handle with
 * Flickcurl, however the library will call curl_easy_setopt to set
 * options based on the operation being performed.  If these need to
 * be over-ridden, use flickcurl_set_curl_setopt_handler() to adjust
 * the options.
 *
 * NOTE: The type of @handle is void* so that curl headers are
 * optional when compiling against flickcurl.
 *
 * Return value: new #flickcurl object or NULL on fialure
 */
flickcurl*
flickcurl_new_with_handle(void* curl_handle)
{
  flickcurl* fc;
  size_t len;
  
  fc = (flickcurl*)calloc(1, sizeof(flickcurl));
  if(!fc)
    return NULL;

  len = strlen(flickcurl_flickr_service_uri);
  fc->service_uri = (char*)malloc(len + 1);
  memcpy(fc->service_uri, flickcurl_flickr_service_uri, len + 1);
  
  len = strlen(flickcurl_flickr_upload_service_uri);
  fc->upload_service_uri = (char*)malloc(len + 1);
  memcpy(fc->upload_service_uri, flickcurl_flickr_upload_service_uri, len +1);

  len = strlen(flickcurl_flickr_replace_service_uri);
  fc->replace_service_uri = (char*)malloc(len + 1);
  memcpy(fc->replace_service_uri, flickcurl_flickr_replace_service_uri, len +1);

  len = strlen(flickcurl_flickr_oauth_request_token_uri);
  fc->oauth_request_token_uri = (char*)malloc(len + 1);
  memcpy(fc->oauth_request_token_uri, flickcurl_flickr_oauth_request_token_uri, len +1);

  len = strlen(flickcurl_flickr_oauth_access_token_uri);
  fc->oauth_access_token_uri = (char*)malloc(len + 1);
  memcpy(fc->oauth_access_token_uri, flickcurl_flickr_oauth_access_token_uri, len +1);

  /* DEFAULT delay between requests is 1000ms i.e 1 request/second max */
  fc->request_delay = 1000;

  fc->mt = mtwist_new();
  if(!fc->mt) {
    free(fc);
    return NULL;
  }
  mtwist_init(fc->mt, mtwist_seed_from_system(fc->mt));

  fc->curl_handle = (CURL*)curl_handle;
  if(!fc->curl_handle) {
    fc->curl_handle = curl_easy_init();
    fc->curl_init_here = 1;
  }

#ifndef CURLOPT_WRITEDATA
#define CURLOPT_WRITEDATA CURLOPT_FILE
#endif

  /* send all data to this function  */
  curl_easy_setopt(fc->curl_handle, CURLOPT_WRITEFUNCTION, 
                   flickcurl_write_callback);
  /* ... using this data pointer */
  curl_easy_setopt(fc->curl_handle, CURLOPT_WRITEDATA, fc);


  /* Make it follow Location: headers */
  curl_easy_setopt(fc->curl_handle, CURLOPT_FOLLOWLOCATION, 1);

#if FLICKCURL_DEBUG > 1
  curl_easy_setopt(fc->curl_handle, CURLOPT_VERBOSE, (void*)1);
  curl_easy_setopt(fc->curl_handle, CURLOPT_DEBUGFUNCTION,
                   flickcurl_debug_callback);
#endif

  curl_easy_setopt(fc->curl_handle, CURLOPT_ERRORBUFFER, fc->error_buffer);

  return fc;
}


/**
 * flickcurl_new:
 *
 * Create a Flickcurl sesssion
 *
 * Return value: new #flickcurl object or NULL on fialure
 */
flickcurl*
flickcurl_new(void)
{
  return flickcurl_new_with_handle(NULL);
}

/**
 * flickcurl_free:
 * @fc: flickcurl object
 * 
 * Destroy flickcurl session
 *
 */
void
flickcurl_free(flickcurl *fc)
{
  if(fc->xc) {
    if(fc->xc->myDoc) {
      xmlFreeDoc(fc->xc->myDoc);
      fc->xc->myDoc = NULL;
    }
    xmlFreeParserCtxt(fc->xc); 
  }

  if(fc->secret)
    free(fc->secret);
  if(fc->auth_token)
    free(fc->auth_token);
  if(fc->method)
    free(fc->method);

  /* only tidy up if we did all the work */
  if(fc->curl_init_here && fc->curl_handle) {
    curl_easy_cleanup(fc->curl_handle);
    fc->curl_handle = NULL;
  }

  if(fc->error_msg)
    free(fc->error_msg);

  if(fc->licenses) {
    int i;
    flickcurl_license *license;
    
    for(i = 0; (license = fc->licenses[i]); i++) {
      free(license->name);
      if(license->url)
        free(license->url);
      free(license);
    }
    
    free(fc->licenses);
  }

  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
  }

  if(fc->param_fields) {
    int i;
    
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
  if(fc->upload_field)
    free(fc->upload_field);
  if(fc->upload_value)
    free(fc->upload_value);

  if(fc->service_uri)
    free(fc->service_uri);
  if(fc->upload_service_uri)
    free(fc->upload_service_uri);
  if(fc->replace_service_uri)
    free(fc->replace_service_uri);
  if(fc->oauth_request_token_uri)
    free(fc->oauth_request_token_uri);
  if(fc->oauth_access_token_uri)
    free(fc->oauth_access_token_uri);

  if(fc->user_agent)
    free(fc->user_agent);

  if(fc->uri)
    free(fc->uri);

  if(fc->mt)
    mtwist_free(fc->mt);

  flickcurl_oauth_free(&fc->od);

  free(fc);
}


/**
 * flickcurl_init:
 *
 * Initialise Flickcurl library.
 *
 * Return value: non-0 on failure
 */
int
flickcurl_init(void)
{
  curl_global_init(CURL_GLOBAL_ALL);
  xmlInitParser();
  flickcurl_serializer_init();
  return 0;
}


/**
 * flickcurl_finish:
 *
 * Terminate Flickcurl library.
 */
void
flickcurl_finish(void)
{
  flickcurl_serializer_terminate();
  xmlCleanupParser();
  curl_global_cleanup();
}


/**
 * flickcurl_set_error_handler:
 * @fc: flickcurl object
 * @error_handler: error handler function
 * @error_data: error handler data
 *
 * Set Flickcurl error handler.
 */
void
flickcurl_set_error_handler(flickcurl* fc, 
                            flickcurl_message_handler error_handler, 
                            void *error_data)
{
  fc->error_handler = error_handler;
  fc->error_data = error_data;
}


/**
 * flickcurl_set_tag_handler:
 * @fc: flickcurl object
 * @tag_handler: tag handler function
 * @tag_data: tag handler data
 *
 * Set Flickcurl tag handler.
 */
void
flickcurl_set_tag_handler(flickcurl* fc, 
                          flickcurl_tag_handler tag_handler, 
                          void *tag_data)
{
  fc->tag_handler = tag_handler;
  fc->tag_data = tag_data;
}


/**
 * flickcurl_set_user_agent:
 * @fc: flickcurl object
 * @user_agent: user agent string
 *
 * Set Flickcurl HTTP user agent string
 */
void
flickcurl_set_user_agent(flickcurl* fc, const char *user_agent)
{
  size_t len = strlen(user_agent);
  char *ua_copy = (char*)malloc(len + 1);
  if(!ua_copy)
    return;

  memcpy(ua_copy, user_agent, len + 1);
  
  fc->user_agent = ua_copy;
}


/**
 * flickcurl_set_proxy:
 * @fc: flickcurl object
 * @proxy: HTTP proxy string
 *
 * Set HTTP proxy for flickcurl requests
 */
void
flickcurl_set_proxy(flickcurl* fc, const char *proxy)
{
  size_t len = strlen(proxy);
  char *proxy_copy = (char*)malloc(len + 1);
  if(!proxy_copy)
    return;

  memcpy(proxy_copy, proxy, len + 1);
  
  fc->proxy = proxy_copy;
}


/**
 * flickcurl_set_http_accept:
 * @fc: flickcurl object
 * @value: HTTP Accept header value
 *
 * Set HTTP accept header value for flickcurl requests
 */
void
flickcurl_set_http_accept(flickcurl* fc, const char *value)
{
  char *value_copy;
  size_t len = 7; /* strlen("Accept:") */
  
  if(value)
    len += 1 + strlen(value); /* " "+value */
  
  value_copy = (char*)malloc(len + 1);
  if(!value_copy)
    return;

  fc->http_accept = value_copy;

  memcpy(value_copy, "Accept:", 8); /* copy NUL */
  value_copy += 7;
  if(value) {
    *value_copy++ = ' ';
    memcpy(value_copy, value, (len - 8) + 1);
  }

}


/**
 * flickcurl_set_service_uri:
 * @fc: flickcurl object
 * @uri: Service URI (or NULL)
 *
 * Set Web Service URI for flickcurl requests
 *
 * Sets the service to the default (Flickr API web service) if @uri is NULL.
 */
void
flickcurl_set_service_uri(flickcurl *fc, const char *uri)
{
  size_t len;
  
  if(!uri)
    uri = flickcurl_flickr_service_uri;
    
#if FLICKCURL_DEBUG > 1
  fprintf(stderr, "Service URI set to: '%s'\n", uri);
#endif
  if(fc->service_uri)
    free(fc->service_uri);

  len = strlen(uri);
  fc->service_uri = (char*)malloc(len + 1);
  memcpy(fc->service_uri, uri, len + 1);
}


/**
 * flickcurl_set_upload_service_uri:
 * @fc: flickcurl object
 * @uri: Upload Service URI (or NULL)
 *
 * Set Web Upload Service URI for flickcurl requests
 *
 * Sets the upload service to the default (Flickr API web
 * upload_service) if @uri is NULL.
 */
void
flickcurl_set_upload_service_uri(flickcurl *fc, const char *uri)
{
  size_t len;

  if(!uri)
    uri = flickcurl_flickr_upload_service_uri;
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "Upload Service URI set to: '%s'\n", uri);
#endif
    if(fc->upload_service_uri)
      free(fc->upload_service_uri);

  len = strlen(uri);
  fc->upload_service_uri = (char*)malloc(len + 1);
  memcpy(fc->upload_service_uri, uri, len + 1);
}


/**
 * flickcurl_set_replace_service_uri:
 * @fc: flickcurl object
 * @uri: Replace Service URI (or NULL)
 *
 * Set Web Replace Service URI for flickcurl requests
 *
 * Sets the replace service to the default (Flickr API web
 * replace_service) if @uri is NULL.
 */
void
flickcurl_set_replace_service_uri(flickcurl *fc, const char *uri)
{
  size_t len;

  if(!uri)
    uri = flickcurl_flickr_replace_service_uri;
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "Replace Service URI set to: '%s'\n", uri);
#endif
    if(fc->replace_service_uri)
      free(fc->replace_service_uri);

  len = strlen(uri);
  fc->replace_service_uri = (char*)malloc(len + 1);
  memcpy(fc->replace_service_uri, uri, len + 1);
}


/**
 * flickcurl_set_api_key:
 * @fc: flickcurl object
 * @api_key: API Key
 *
 * Set legacy Flickr auth application API Key (OAuth Client key)
 *
 * For OAuth this is not sufficient and
 * flickcurl_set_oauth_client_key() and
 * flickcurl_set_oauth_client_secret() be used
 * to pass both the client key and client secret.
 */
void
flickcurl_set_api_key(flickcurl* fc, const char *api_key)
{
  size_t len;
  
#if FLICKCURL_DEBUG > 1
  fprintf(stderr, "API Key: '%s'\n", api_key);
#endif
  if(fc->od.client_key)
    free(fc->od.client_key);

  len = strlen(api_key);
  fc->od.client_key = (char*)malloc(len + 1);
  memcpy(fc->od.client_key, api_key, len + 1);
  fc->od.client_key_len = len;

  /* Mainly for flickcurl_auth_oauth_getAccessToken() to sign the call
   * exchanging tokens 
   */
  fc->api_key = fc->od.client_key;
}


/**
 * flickcurl_get_api_key:
 * @fc: flickcurl object
 *
 * Get current application API Key (OAuth Client key)
 *
 * Return value: API key or NULL if none set
 */
const char*
flickcurl_get_api_key(flickcurl* fc)
{
  return fc->api_key;
}


/**
 * flickcurl_set_shared_secret:
 * @fc: flickcurl object
 * @secret: shared secret
 *
 * Set legacy Flickr auth secret
 */
void
flickcurl_set_shared_secret(flickcurl* fc, const char *secret)
{
  size_t len;
  
#if FLICKCURL_DEBUG > 1
  fprintf(stderr, "Legacy Flickr auth Secret: '%s'\n", secret);
#endif
  if(fc->secret)
    free(fc->secret);
  len = strlen(secret);
  fc->secret = (char*)malloc(len + 1);
  memcpy(fc->secret, secret, len + 1);
}


/**
 * flickcurl_get_shared_secret:
 * @fc: flickcurl object
 *
 * Get legacy Flickr auth Secret
 *
 * Return value: shared secret or NULL if none set
 */
const char*
flickcurl_get_shared_secret(flickcurl* fc)
{
  return fc->secret;
}


/**
 * flickcurl_set_auth_token:
 * @fc: flickcurl object
 * @auth_token: auth token
 *
 * Set legacy Flickr auth Token
 */
void
flickcurl_set_auth_token(flickcurl *fc, const char* auth_token)
{
  size_t len;
  
#if FLICKCURL_DEBUG > 1
  fprintf(stderr, "Legacy Flickr auth token: '%s'\n", auth_token);
#endif
  if(fc->auth_token)
    free(fc->auth_token);

  len = strlen(auth_token);
  fc->auth_token = (char*)malloc(len + 1);
  memcpy(fc->auth_token, auth_token, len + 1);
}


/**
 * flickcurl_get_auth_token:
 * @fc: flickcurl object
 *
 * Get legacy Flickr auth Token
 *
 * Return value: auth token or NULL if none set
 */
const char*
flickcurl_get_auth_token(flickcurl *fc)
{
  return fc->auth_token;
}


/**
 * flickcurl_set_sign:
 * @fc: flickcurl object
 *
 * Make the next request signed.
 */
void
flickcurl_set_sign(flickcurl *fc)
{
  fc->sign = 1;
}


/**
 * flickcurl_set_request_delay:
 * @fc: flickcurl object
 * @delay_msec: web service delay in milliseconds
 *
 * Set web service request delay
 */
void
flickcurl_set_request_delay(flickcurl *fc, long delay_msec)
{
  if(delay_msec >= 0)
    fc->request_delay = delay_msec;
}


/*
 * INTERNAL: initialise parameter array
 */
void
flickcurl_init_params(flickcurl *fc, int is_write)
{
  fc->count = 0;
  fc->parameters[fc->count][0] = NULL;

  /* Default is read only */
  fc->is_write = is_write;

  /* Default to no data */
  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
    fc->data = NULL;
    fc->data_length = 0;
    fc->data_is_xml = 0;
  }
  if(is_write)
    flickcurl_set_data(fc, (void*)"", 0);
}


/*
 * INTERNAL: add a new (key, value) to array of parameters
 */
void
flickcurl_add_param(flickcurl *fc, const char* key, const char* value)
{
  fc->parameters[fc->count][0] = key;
  fc->parameters[fc->count][1] = value;
  fc->count++;
}

/*
 * INTERNAL: finish parameters
 */
void
flickcurl_end_params(flickcurl *fc)
{
  fc->parameters[fc->count][0] = NULL;
}


static int
flickcurl_prepare_common(flickcurl *fc, 
                         const char* service_uri,
                         const char* method,
                         const char* upload_field,
                         const char* upload_value,
                         int parameters_in_url, int need_auth)
{
  int rc = 1;

  if(fc->api_key && fc->secret)
    /* Call with legacy Flickr auth */
    rc = flickcurl_legacy_prepare_common(fc, service_uri, method,
                                         upload_field, upload_value,
                                         parameters_in_url, need_auth);
  else if(fc->od.token && fc->od.token_secret)
    /* Call with OAuth */
    rc = flickcurl_oauth_prepare_common(fc, service_uri, method,
                                        upload_field, upload_value,
                                        parameters_in_url, need_auth);
  else
    flickcurl_error(fc, "No legacy or OAuth authentication tokens or secrets");

  return rc;
}



int
flickcurl_prepare_noauth(flickcurl *fc, const char* method)
{
  if(!method) {
    flickcurl_error(fc, "No method to prepare");
    return 1;
  }
  
  return flickcurl_prepare_common(fc,
                                  fc->service_uri,
                                  method,
                                  NULL, NULL,
                                  1, 0);
}


int
flickcurl_prepare(flickcurl *fc, const char* method)
{
  if(!method) {
    flickcurl_error(fc, "No method to prepare");
    return 1;
  }
  
  return flickcurl_prepare_common(fc,
                                  fc->service_uri,
                                  method,
                                  NULL, NULL,
                                  /* parameters_in_url */ 1,
                                  /* need_auth */ 1);
}


int
flickcurl_prepare_upload(flickcurl *fc, 
                         const char* url,
                         const char* upload_field, const char* upload_value)
{
  return flickcurl_prepare_common(fc,
                                  url,
                                  NULL,
                                  upload_field, upload_value,
                                  /* parameters_in_url */ 0,
                                  /* need_auth */ 1);
}


/* Need gettimeofday() which is a BSD function not POSIX so may not
 * be in standard C libraries
 */

#ifdef HAVE_GETTIMEOFDAY
#ifdef WIN32
/* have it as an external function */
int gettimeofday(struct timeval* tp, void *tzp);
#endif

#else

/* seconds between 1 Jan 1601 (windows epoch) and 1 Jan 1970 (unix epoch) */
#define EPOCH_WIN_UNIX_DELTA 11644473600.0

/* 100 nano-seconds ( = 1/10 usec) in seconds */
#define NSEC100 (1e-7)

/* factor to convert high-dword count into seconds = NSEC100 * (2<<32) */
#define FOUR_GIGA_NSEC100 (4294967296e-7)

static int
gettimeofday(struct timeval* tp, void* tzp)
{
  FILETIME ft;
  double t;
  
  /* returns time since windows epoch in 100ns (1/10us) units */
  GetSystemTimeAsFileTime(&ft);

  /* convert time into seconds as a double */
  t = ((ft.dwHighDateTime * FOUR_GIGA_NSEC100) - EPOCH_WIN_UNIX_DELTA) +
      (ft.dwLowDateTime  * NSEC100);

  tp->tv_sec  = (long) t;
  tp->tv_usec = (long) ((t - tp->tv_sec) * 1e6);

  /* tzp is ignored */

  return 0;
}
#endif
/* end HAVE_GETTIMEOFDAY */


/* Need nanosleep() to wait between service calls */
#ifdef HAVE_NANOSLEEP
/* nop */
#else

#ifdef WIN32
struct timespec
{
  long int tv_sec;              /* seconds */
  long int tv_nsec;             /* nanoseconds */
};
#endif

static int
nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
  unsigned int msec;
  unsigned int sec;

  sec= rqtp->tv_sec;
  msec= (rqtp->tv_nsec / 1000000);

  /* carefully avoid sleeping forever with a sleep(0) */
#ifdef WIN32
  msec += 1000 * sec;
  if(!msec)
    msec = 1;

  Sleep(msec);
#else
  /* otherwise use sleep() (POSIX) and possibly usleep() (4.3BSD) */
  if(sec > 0)
    sleep(sec);
  else {
    /* 0 seconds so ensure msec is at least 1 */
    if(!msec)
      msec = 1;
  }
#ifdef HAVE_USLEEP
  /* use usleep() for fractions of a second only (when available)
   * since some implementations won't let it sleep for more than a
   * second.
   */
  if(msec > 0)
    usleep(msec * 1000);
#endif
#endif

  return 0;
}

#endif
/* end HAVE_NANOSLEEP */


static size_t 
flickcurl_curl_header_callback(void* ptr,  size_t  size, size_t nmemb,
                               void *userdata) 
{
  flickcurl* fc = (flickcurl*)userdata;
  int bytes = size*nmemb;

  /* If flickcurl has already failed, return nothing so that
   * libcurl will abort the transfer
   */
  if(fc->failed)
    return 0;
  
#define EC_HEADER_LEN 17
#define EM_HEADER_LEN 20

  if(!strncmp((char*)ptr, "X-FlickrErrCode: ", EC_HEADER_LEN)) {
    fc->error_code = atoi((char*)ptr+EC_HEADER_LEN);
  } else if(!strncmp((char*)ptr, "X-FlickrErrMessage: ", EM_HEADER_LEN)) {
    int len = bytes - EM_HEADER_LEN;
    if(fc->error_msg)
      free(fc->error_msg);
    fc->error_msg = (char*)malloc(len + 1);
    memcpy(fc->error_msg, (char*)ptr + EM_HEADER_LEN, len + 1);
    fc->error_msg[len] = '\0';
    while(fc->error_msg[len-1] == '\r' || fc->error_msg[len-1] == '\n') {
      fc->error_msg[len-1] = '\0';
      len--;
    }
  }
  
  return bytes;
}


/**
 * flickcurl_get_current_request_wait:
 * @fc: flickcurl object
 *
 * Get current wait that would be applied for a web service request called now
 *
 * Returns the wait time that would be applied in order to delay a
 * web service request such that the web service rate limit is met.
 *
 * See flickcurl_set_request_delay() which by default is set to 1000ms.
 * 
 * Return value: delay in usecs or < 0 if delay is more than 247 seconds ('infinity')
 */
int
flickcurl_get_current_request_wait(flickcurl *fc)
{
#ifdef OFFLINE
  return 0;
#else
  int wait_usec = 0;
  struct timeval now;
  struct timeval uwait;
  
  /* If there was no previous request, return 0 */
  if(!fc->last_request_time.tv_sec)
    return 0;
  
  gettimeofday(&now, NULL);

  memcpy(&uwait, &fc->last_request_time, sizeof(struct timeval));

  /* Calculate in micro-seconds */
  uwait.tv_usec += 1000 * fc->request_delay;
  if(uwait.tv_usec >= 1000000) {
    uwait.tv_sec+= uwait.tv_usec / 1000000;
    uwait.tv_usec= uwait.tv_usec % 1000000;
  }

  if(now.tv_sec > uwait.tv_sec ||
     (now.tv_sec == uwait.tv_sec && now.tv_usec > uwait.tv_usec)) {
    wait_usec = 0; /* No need to delay */
  } else {
    /* Calculate wait in usec-seconds */
    uwait.tv_sec = (uwait.tv_sec - now.tv_sec);
    uwait.tv_usec = (uwait.tv_usec - now.tv_usec);
    if(uwait.tv_usec < 0) {
      uwait.tv_sec--;
      uwait.tv_usec += 1000000;
    }

    if(uwait.tv_sec > 247)
      wait_usec = -1; /* 'infinity' */
    else
      wait_usec = uwait.tv_sec * 1000000 + uwait.tv_usec;
  }

  return wait_usec;
#endif
}


static int
flickcurl_invoke_common(flickcurl *fc, char** content_p, size_t* size_p,
                        xmlDocPtr* docptr_p)
{
  struct curl_slist *slist = NULL;
  xmlDocPtr doc = NULL;
  struct timeval now;
#if defined(OFFLINE) || defined(CAPTURE)
  char filename[200];
#endif
  int rc = 0;
  
#if defined(OFFLINE) || defined(CAPTURE)

  if(1) {
    if(fc->method)
      sprintf(filename, "captured/%s.xml", fc->method+7); /* skip "flickr." */
    else
      sprintf(filename, "captured/upload.xml");
  }
#endif

#ifdef OFFLINE
  if(1) {
#ifdef HAVE_RAPTOR
    char* uri_string;
    size_t len;
#endif
    
    if(access(filename, R_OK)) {
      fprintf(stderr, "Method %s cannot run offline - no %s XML result available\n",
              fc->method, filename);
      return 1;
    }
#ifdef HAVE_RAPTOR
    uri_string = raptor_uri_filename_to_uri_string(filename);
    len = strlen(uri_string);
    memcpy(fc->uri, uri_string, len + 1);
    raptor_free_memory(uri_string);
#else
    sprintf(fc->uri, "file:%s", filename);
#endif
    fprintf(stderr, "Method %s: running offline using result from %s\n", 
            fc->method, filename);
  }
#endif

  if(!fc->uri) {
    flickcurl_error(fc, "No Flickr URI prepared to invoke");
    return 1;
  }

  if(content_p)
    fc->save_content = 1;
  else
    fc->xml_parse_content = 1;
  
  gettimeofday(&now, NULL);
#ifndef OFFLINE
  if(fc->last_request_time.tv_sec) {
    /* If there was a previous request, check it's not too soon to
     * do another
     */
    struct timeval uwait;

    memcpy(&uwait, &fc->last_request_time, sizeof(struct timeval));

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "Previous request was at %lu.N%lu\n",
            (unsigned long)uwait.tv_sec, (unsigned long)1000*uwait.tv_usec);
#endif

    /* Calculate in micro-seconds */
    uwait.tv_usec += 1000 * fc->request_delay;
    if(uwait.tv_usec >= 1000000) {
      uwait.tv_sec+= uwait.tv_usec / 1000000;
      uwait.tv_usec= uwait.tv_usec % 1000000;
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "Next request is no earlier than %lu.N%lu\n",
            (unsigned long)uwait.tv_sec, (unsigned long)1000*uwait.tv_usec);
    fprintf(stderr, "Now is %lu.N%lu\n",
            (unsigned long)now.tv_sec, (unsigned long)1000*now.tv_usec);
#endif
    
    if(now.tv_sec > uwait.tv_sec ||
       (now.tv_sec == uwait.tv_sec && now.tv_usec > uwait.tv_usec)) {
      /* No need to delay */
    } else {
      struct timespec nwait;
      /* Calculate in nano-seconds */
      nwait.tv_sec= uwait.tv_sec - now.tv_sec;
      nwait.tv_nsec= 1000*(uwait.tv_usec - now.tv_usec);
      if(nwait.tv_nsec < 0) {
        nwait.tv_sec--;
        nwait.tv_nsec+= 1000000000;
      }
      
      /* Wait until timeval 'wait' happens */
#if FLICKCURL_DEBUG > 1
      fprintf(stderr, "Waiting for %lu sec N%lu nsec period\n",
              (unsigned long)nwait.tv_sec, (unsigned long)nwait.tv_nsec);
#endif
      while(1) {
        struct timespec rem;
        if(nanosleep(&nwait, &rem) < 0 && errno == EINTR) {
          memcpy(&nwait, &rem, sizeof(struct timeval));
#if FLICKCURL_DEBUG > 1
          fprintf(stderr, "EINTR - waiting for %lu sec N%lu nsec period\n",
                  (unsigned long)nwait.tv_sec, (unsigned long)nwait.tv_nsec);
#endif
          continue;
        }
        break;
      }
    }
  }
#endif
  memcpy(&fc->last_request_time, &now, sizeof(struct timeval));

#ifdef CAPTURE
  if(1) {
    fc->fh = fopen(filename, "wb");
    if(!fc->fh)
      flickcurl_error(fc, "Capture failed to write to %s - %s",
                      filename, strerror(errno));
  }
#endif

  if(fc->xc) {
    if(fc->xc->myDoc) {
      xmlFreeDoc(fc->xc->myDoc);
      fc->xc->myDoc = NULL;
    }
    xmlFreeParserCtxt(fc->xc); 
    fc->xc = NULL;
  }

  if(fc->proxy)
    curl_easy_setopt(fc->curl_handle, CURLOPT_PROXY, fc->proxy);

  if(fc->user_agent)
    curl_easy_setopt(fc->curl_handle, CURLOPT_USERAGENT, fc->user_agent);

  /* Insert HTTP Accept: header */
  if(fc->http_accept)
    slist = curl_slist_append(slist, (const char*)fc->http_accept);

  /* specify URL to call */
  curl_easy_setopt(fc->curl_handle, CURLOPT_URL, fc->uri);

  fc->total_bytes = 0;

  /* default: read with no data: GET */
  curl_easy_setopt(fc->curl_handle, CURLOPT_NOBODY, 1);
  curl_easy_setopt(fc->curl_handle, CURLOPT_HTTPGET, 1);

  if(fc->data) {
    /* write with some data: POST */
    /* CURLOPT_NOBODY = 0 sets http request to HEAD - do it first to override */
    curl_easy_setopt(fc->curl_handle, CURLOPT_NOBODY, 0);
    /* this function only resets no-body flag for curl >= 7.14.1 */
    curl_easy_setopt(fc->curl_handle, CURLOPT_POST, 1);
    curl_easy_setopt(fc->curl_handle, CURLOPT_POSTFIELDS, fc->data);
    curl_easy_setopt(fc->curl_handle, CURLOPT_POSTFIELDSIZE, fc->data_length);
  } else if(fc->is_write) {
    /* write with no data: POST */
    /* CURLOPT_NOBODY = 0 sets http request to HEAD - do it first to override */
    curl_easy_setopt(fc->curl_handle, CURLOPT_NOBODY, 0);
    /* this function only resets no-body flag for curl >= 7.14.1 */
    curl_easy_setopt(fc->curl_handle, CURLOPT_POST, 1);
  }


  /* set slist always - either a list of headers or none (NULL) */
  curl_easy_setopt(fc->curl_handle, CURLOPT_HTTPHEADER, slist);

  /* send all headers to this function */
  curl_easy_setopt(fc->curl_handle, CURLOPT_HEADERFUNCTION, 
                   flickcurl_curl_header_callback);
  /* ... using this data pointer */
  curl_easy_setopt(fc->curl_handle, CURLOPT_WRITEHEADER, fc);


#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "Preparing CURL with URI '%s' and method %s\n", 
          fc->uri, ((fc->is_write || fc->upload_field) ? "POST" : "GET"));
#endif
  
  if(fc->upload_field) {
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;
    int i;
    
    /* Main parameters */
    for(i = 0; fc->param_fields[i]; i++) {
#ifdef FLICKCURL_DEBUG
      fprintf(stderr, "  form param %2d) %-23s: '%s'\n",
              i, fc->param_fields[i], fc->param_values[i]);
#endif
      curl_formadd(&post, &last, CURLFORM_PTRNAME, fc->param_fields[i],
                   CURLFORM_PTRCONTENTS, fc->param_values[i],
                   CURLFORM_END);
    }
    
    /* Upload parameter */
#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "  Upload form parameter %s: <File '%s'>\n",
            fc->upload_field, fc->upload_value);
#endif
    curl_formadd(&post, &last, CURLFORM_PTRNAME, fc->upload_field,
                 CURLFORM_FILE, fc->upload_value, CURLFORM_END);

    /* Set the form info */
    curl_easy_setopt(fc->curl_handle, CURLOPT_HTTPPOST, post);
  }

  if(fc->curl_setopt_handler)
    fc->curl_setopt_handler(fc->curl_handle, fc->curl_setopt_handler_data);

#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "Invoking CURL to resolve the URL\n");
#endif

  if(curl_easy_perform(fc->curl_handle)) {
    /* failed */
    fc->failed = 1;
    flickcurl_error(fc, "Method %s failed with CURL error %s",
                    fc->method, fc->error_buffer);
  } else {
    long lstatus;

#ifndef CURLINFO_RESPONSE_CODE
#define CURLINFO_RESPONSE_CODE CURLINFO_HTTP_CODE
#endif

    fc->status_code = 0;
    /* Requires pointer to a long */
    if(CURLE_OK == 
       curl_easy_getinfo(fc->curl_handle, CURLINFO_RESPONSE_CODE, &lstatus) )
      fc->status_code = lstatus;

    if(fc->status_code != 200) {
      if(fc->method)
        flickcurl_error(fc, "Method %s failed with error %d - %s (HTTP %d)", 
                        fc->method, fc->error_code, fc->error_msg,
                        fc->status_code);
      else
        flickcurl_error(fc, "Call failed with error %d - %s (HTTP %d)", 
                        fc->error_code, fc->error_msg,
                        fc->status_code);
      fc->failed = 1;
    }

  }

  if(slist)
    curl_slist_free_all(slist);

  if(fc->failed)
    goto tidy;
  
  if(fc->save_content) {
    char* c;
    flickcurl_chunk** chunks;

    c = (char*)malloc(fc->total_bytes+1); /* +1 for NUL */
    chunks = (flickcurl_chunk**)malloc(sizeof(flickcurl_chunk*) * fc->chunks_count);
    if(c && chunks) {
      flickcurl_chunk* chunk = fc->chunks;
      int i;
      char *p;

      /* create the ordered list of chunks */
      for(i = fc->chunks_count-1; i >= 0; i--) {
        chunks[i] = chunk;
        chunk = chunk->prev;
      }

      p = c;
      for(i = 0; i < fc->chunks_count; i++) {
        memcpy(p, chunks[i]->content, chunks[i]->size);
        p += chunks[i]->size;

        /* free saved chunk once it has been copied */
        free(chunks[i]->content);
        free(chunks[i]);
      }
      free(chunks);

      /* saved chunks list is now freed */
      fc->chunks = NULL; 
      fc->chunks_count = 0;

      *p = '\0';
      
      if(content_p)
        *content_p = c;
      else
        free(c);
      if(size_p)
        *size_p = fc->total_bytes;
      
    } else {
      if(c)
        free(c);
      if(chunks)
        free(chunks);
      flickcurl_error(fc, "Out of memory");
    }
  }

  if(fc->xml_parse_content) {
    xmlNodePtr xnp;
    xmlAttr* attr;
    int failed = 0;
    
    xmlParseChunk(fc->xc, NULL, 0, 1);

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "Got %d bytes content from URI '%s'\n",
            fc->total_bytes, fc->uri);
#endif

    doc = fc->xc->myDoc;
    if(!doc) {
      flickcurl_error(fc, "Failed to create XML DOM for document");
      fc->failed = 1;
      goto tidy;
    }

    xnp = xmlDocGetRootElement(doc);
    if(!xnp) {
      flickcurl_error(fc, "Failed to parse XML");
      fc->failed = 1;
      goto tidy;
    }

    for(attr = xnp->properties; attr; attr = attr->next) {
      if(!strcmp((const char*)attr->name, "stat")) {
        const char *attr_value = (const char*)attr->children->content;
#ifdef FLICKCURL_DEBUG
        fprintf(stderr, "Request returned stat '%s'\n", attr_value);
#endif
        if(strcmp(attr_value, "ok"))
          failed = 1;
        break;
      }
    }

    if(failed) {
      xmlNodePtr err = xnp->children->next;
      for(attr = err->properties; attr; attr = attr->next) {
        const char *attr_name = (const char*)attr->name;
        const char *attr_value = (const char*)attr->children->content;
        if(!strcmp(attr_name, "code"))
          fc->error_code = atoi(attr_value);
        else if(!strcmp(attr_name, "msg")) {
          size_t attr_len = strlen(attr_value);
          fc->error_msg = (char*)malloc(attr_len + 1);
          memcpy(fc->error_msg, attr_value, attr_len + 1);
        }
      }
      if(fc->method)
        flickcurl_error(fc, "Method %s failed with error %d - %s", 
                        fc->method, fc->error_code, fc->error_msg);
      else
        flickcurl_error(fc, "Call failed with error %d - %s", 
                        fc->error_code, fc->error_msg);
      fc->failed = 1;
    } else {
      /* pass DOM as an output parameter */
      if(docptr_p)
        *docptr_p = doc;
    }
  }

  tidy:
  if(fc->failed)
    rc = 1;
  
#ifdef CAPTURE
  if(1) {
    if(fc->fh)
      fclose(fc->fh);
  }
#endif

  /* reset special flags */
  fc->sign = 0;
  
  return rc;
}


xmlDocPtr
flickcurl_invoke(flickcurl *fc)
{
  xmlDocPtr docptr = NULL;
  if(!flickcurl_invoke_common(fc, NULL, NULL, &docptr))
    return docptr;
  return NULL;
}


char*
flickcurl_invoke_get_content(flickcurl *fc, size_t* size_p)
{
  char* content = NULL;
  if(!flickcurl_invoke_common(fc, &content, size_p, NULL))
    return content;
  return NULL;
}


void
flickcurl_free_form(char **form, int count)
{
  if(!form)
    return;

  /* free content which is the first key */
  free(form[0]);

  free(form);
}


/*
* flickcurl_invoke_get_form_content:
* @fc: flickcurl object
* @count_p: pointer to store count (or NULL)
*
* INTERNAL - decoded content from current request as HTTP FORM and return fields
*
* NOTE: The result may be an empty array with just two NULL
* terminating pointers if there are no fields.
*
* Return value: array of [char* field name, char* field value] with
* NULL pair terminating or NULL on failure
*/
char**
flickcurl_invoke_get_form_content(flickcurl *fc, int* count_p)
{
  char* content = NULL;
  char** form = NULL;
  char *p;
  int count = 0;
  int i;

  if(flickcurl_invoke_common(fc, &content, NULL, NULL))
    return NULL;

  if(content) {
    for(p = content; *p; p++) {
      if(*p == '&')
        count++;
    }
    count++; /* counting separators so need +1 for number of contents */
  }
  
  /* Allocate count + 1 sized array of char* (key, value) pointers
   * The last pair are always (NULL, NULL).
   *
   * The pointers are into the 'content' buffer which is kept around
   * and owned by this array and stored in form[0].
   */
  form = (char**)calloc(2*(count + 1), sizeof(char*));
  if(!form) {
    if(content)
      free(content);
    return NULL;
  }

  if(content) {
    for(p = content, i = 0; *p; p++) {
      char *start = p;

      while(*p && *p != '&' && *p != '=')
        p++;

      form[i++] = start;

      if(!*p)
        break;
      *p = '\0';
    }
    form[i++] = NULL;
    form[i] = NULL;

    free(content);
  }

  if(count_p)
    *count_p = count;

  return form;
}


char*
flickcurl_unixtime_to_isotime(time_t unix_time)
{
  struct tm* structured_time;
#define ISO_DATE_FORMAT "%Y-%m-%dT%H:%M:%SZ"
#define ISO_DATE_LEN 20
  static char date_buffer[ISO_DATE_LEN + 1];
  size_t len;
  char *value = NULL;
  
  structured_time = (struct tm*)gmtime(&unix_time);
  len = ISO_DATE_LEN;
  strftime(date_buffer, len+1, ISO_DATE_FORMAT, structured_time);
  
  value = (char*)malloc(len + 1);
  memcpy(value, date_buffer, len + 1);
  return value;
}


char*
flickcurl_unixtime_to_sqltimestamp(time_t unix_time)
{
  struct tm* structured_time;
#define SQL_DATETIME_FORMAT "%Y %m %d %H:%M:%S"
#define SQL_DATETIME_LEN 19
  static char date_buffer[SQL_DATETIME_LEN + 1];
  size_t len;
  char *value = NULL;
  
  structured_time = (struct tm*)gmtime(&unix_time);
  len = SQL_DATETIME_LEN;
  strftime(date_buffer, sizeof(date_buffer), SQL_DATETIME_FORMAT, structured_time);
  
  value = (char*)malloc(len + 1);
  memcpy(value, date_buffer, len + 1);
  return value;
}


char*
flickcurl_sqltimestamp_to_isotime(const char* timestamp)
{
/* SQL DATETIME FORMAT "%Y %m %d %H:%M:%S"  (19 chars) */
#define SQL_DATE_LEN 19
/* ISO DATE FORMAT     "%Y-%m-%dT%H:%M:%SZ" (20 chars) */
#define ISO_DATE_LEN 20
  size_t len = ISO_DATE_LEN;
  char *value = NULL;
  
  value = (char*)malloc(len + 1);
  memcpy(value, timestamp, SQL_DATE_LEN);
  value[4] = '-';
  value[7] = '-';
  value[10] = 'T';
  value[13] = ':';
  value[16] = ':';
  value[19] = 'Z';
  value[ISO_DATE_LEN] = '\0';
  
  return value;
}


char*
flickcurl_xpath_eval(flickcurl *fc, xmlXPathContextPtr xpathCtx,
                     const xmlChar* xpathExpr) 
{
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  int i;
  char* value = NULL;
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed = 1;
    goto tidy;
  }
    
  nodes = xpathObj->nodesetval;
  if(xmlXPathNodeSetIsEmpty(nodes))
    goto tidy;
  
  for(i = 0; i < xmlXPathNodeSetGetLength(nodes); i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    
    if(node->type != XML_ATTRIBUTE_NODE &&
       node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    if(node->children) {
      size_t len = strlen((char*)node->children->content);
      value = (char*)malloc(len + 1);
      memcpy(value, node->children->content, len + 1);
    }
    break;
  }

  tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return value;
}


char*
flickcurl_xpath_eval_to_tree_string(flickcurl* fc,
                                    xmlXPathContextPtr xpathNodeCtx,
                                    const xmlChar* xpathExpr, size_t* length_p)
{
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodePtr sd_node;
  xmlBufferPtr buffer = NULL;
  xmlSaveCtxtPtr save_ctxt = NULL;
  char* value = NULL;
  size_t value_len = 0;
  xmlNodeSetPtr nodes;
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathNodeCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed = 1;
    goto tidy;
  }

  nodes = xpathObj->nodesetval;
  if(xmlXPathNodeSetIsEmpty(nodes))
    goto tidy;
    
  sd_node = nodes->nodeTab[0];
  
  buffer = xmlBufferCreate();
  if(!buffer)
    goto tidy;
  save_ctxt = xmlSaveToBuffer(buffer, NULL /* encoding */, 0 /* opts */);
  
  xmlSaveTree(save_ctxt, sd_node);
  xmlSaveFlush(save_ctxt);
  
  value_len = xmlBufferLength(buffer);
  if(!value_len)
    goto tidy;
  
  value = (char*)malloc(value_len+1);
  if(!value)
    goto tidy;
  memcpy(value, xmlBufferContent(buffer), value_len + 1);

  tidy:
  if(buffer)
    xmlBufferFree(buffer);

  if(xpathObj)  
    xmlXPathFreeObject(xpathObj);

  if(value && length_p)
    *length_p = value_len;

  return value;
}


/**
 * flickcurl_set_write:
 * @fc: flickcurl object
 * @is_write: writeable flag
 *
 * Set writeable flag.
 */
void
flickcurl_set_write(flickcurl *fc, int is_write)
{
  fc->is_write = is_write;
}


/**
 * flickcurl_set_data:
 * @fc: flickcurl object
 * @data: data pointer
 * @data_length: data length
 *
 * Set web service request content data.
 */
void
flickcurl_set_data(flickcurl *fc, void* data, size_t data_length)
{
  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
  }
  
  fc->data = data;
  fc->data_length = data_length;
  fc->data_is_xml = 0;
}


/**
 * flickcurl_set_xml_data:
 * @fc: flickcurl object
 * @doc: XML dom
 *
 * Set web service request content data from XML DOM.
 */
void
flickcurl_set_xml_data(flickcurl *fc, xmlDocPtr doc)
{
  xmlChar* mem;
  int size;

  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
  }

  xmlDocDumpFormatMemory(doc, &mem, &size, 1); /* format 1 means indent */
  
  fc->data = mem;
  fc->data_length = (size_t)size;
  fc->data_is_xml = 1;
}


static const char* flickcurl_field_value_type_label[VALUE_TYPE_LAST+1] = {
  "(none)",
  "photo id",
  "photo URI",
  "unix time",
  "boolean",
  "dateTime",
  "float",
  "integer",
  "string",
  "uri",
  "<internal>person ID",
  "<internal>media type",
  "<internal>tag string",
  "<internal>collection id",
  "<internal>icon photos"
};


/**
 * flickcurl_get_field_value_type_label:
 * @datatype: datatype enum
 *
 * Get label for datatype
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_field_value_type_label(flickcurl_field_value_type datatype)
{
  if(datatype <= VALUE_TYPE_LAST)
    return flickcurl_field_value_type_label[(int)datatype];
  return NULL;
}


char*
flickcurl_call_get_one_string_field(flickcurl* fc, 
                                    const char* key, const char* value,
                                    const char* method,
                                    const xmlChar* xpathExpr)
{
  char *result = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 

  flickcurl_init_params(fc, 0);
  if(key && value) {
    flickcurl_add_param(fc, key, value);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, method))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx)
    result = flickcurl_xpath_eval(fc, xpathCtx, xpathExpr);
  
  xmlXPathFreeContext(xpathCtx);

  tidy:

  return result;
}


/**
 * flickcurl_array_join:
 * @array: C array
 * @delim: delimeter character
 *
 * Join elements of a C array into a string
 *
 * Return value: newly allocated string or NULL on failure
 */
char*
flickcurl_array_join(const char *array[], char delim)
{
  int i;
  int array_size;
  size_t len = 0;
  char* str;
  char* p;
  
  for(i = 0; array[i]; i++)
    len += strlen(array[i])+1;
  array_size = i;
  
  str = (char*)malloc(len+1);
  if(!str)
    return NULL;
  
  p = str;
  for(i = 0; array[i]; i++) {
    size_t item_len = strlen(array[i]);
    memcpy(p, array[i], item_len);
    p += item_len;
    if(i < array_size)
      *p++ = delim;
  }
  *p = '\0';

  return str;
}


/**
 * flickcurl_array_split:
 * @str: string
 * @delim: delimeter character
 *
 * Split a string into a C array
 *
 * Return value: newly allocated array or NULL on failure
 */
char**
flickcurl_array_split(const char *str, char delim)
{
  int i;
  int array_size = 1;
  char** array;
  
  for(i = 0; str[i]; i++) {
    if(str[i] == delim)
      array_size++;
  }
  
  array = (char**)malloc(sizeof(char*)*(array_size+1));
  if(!array)
    return NULL;

  for(i = 0; *str; i++) {
    size_t item_len;
    const char* p;

    for(p = str; *p && *p != delim; p++)
      ;
    item_len = p - str;
    array[i] = (char*)malloc(item_len + 1);
    if(!array[i]) {
      while(--i >= 0)
        free(array[i]);
      free(array);
      return NULL;
    }
    memcpy(array[i], str, item_len);
    array[i][item_len] = '\0';
    str+= item_len;
    if(*str == delim)
      str++;
  }
  array[i] = NULL;
  
  return array;
}


/**
 * flickcurl_array_free:
 * @array: C array
 *
 * Free an array.
 */
void
flickcurl_array_free(char* array[])
{
  int i;
  
  for(i = 0; array[i]; i++)
    free(array[i]);

  free(array);
}


#define CONTENT_TYPE_COUNT 3
static const char* flickcurl_content_type_labels[CONTENT_TYPE_COUNT+1]=
  {"unknown", "photo", "screenshot", "other"};


/**
 * flickcurl_get_content_type_label:
 * @content_type: safety level index
 * 
 * Get label for a content type.
 *
 * Return value: pointer to shared string label for content type or "unknown"
 **/
const char*
flickcurl_get_content_type_label(int content_type)
{
  if(content_type < 1 || content_type > CONTENT_TYPE_COUNT)
    content_type= 0;
  return flickcurl_content_type_labels[content_type];
}


/**
 * flickcurl_get_content_type_from_string:
 * @content_type_string: string
 * 
 * Get the enumeration value for a content type string.
 *
 * Parses the string value into a content type either from an
 * integer form like '1' or a label like 'photo'.
 * 
 * Returns: content type enumeration value or <0 on error
 **/
int
flickcurl_get_content_type_from_string(const char* content_type_string)
{
  char* endptr = NULL;
  int content_type = -1;

  content_type = (int)strtol(content_type_string, &endptr, 10);
  /* If not all of string was used - fail */
  if(endptr && *endptr)
    content_type= -1;
  if(content_type < 1 || content_type > CONTENT_TYPE_COUNT) {
    int i;
    for(i = 1; i< CONTENT_TYPE_COUNT; i++)
      if(!strcmp(flickcurl_content_type_labels[i], content_type_string)) {
        content_type = i;
        break;
      }
  }

  return content_type;
}


#define SAFETY_LEVEL_COUNT 4
static const char* flickcurl_safety_level_labels[SAFETY_LEVEL_COUNT+1]=
  {"unknown", "safe", "moderate", "restricted", "(no change)"};


/**
 * flickcurl_get_safety_level_label:
 * @safety_level: safety level index
 * 
 * Get label for a safety level.
 *
 * Return value: pointer to shared string label for safety level or "unknown"
 **/
const char*
flickcurl_get_safety_level_label(int safety_level)
{
  if(safety_level < 1 || safety_level > SAFETY_LEVEL_COUNT)
    safety_level= 0;
  return flickcurl_safety_level_labels[safety_level];
}


/**
 * flickcurl_get_safety_level_from_string:
 * @safety_level_string: string
 * 
 * Get the enumeration value for a safety level string.
 *
 * Parses the string value into a safety level either from an
 * integer form like '1' or a label like 'safe'.
 * 
 * Returns: safety level enumeration value or <0 on error
 **/
int
flickcurl_get_safety_level_from_string(const char* safety_level_string)
{
  char* endptr = NULL;
  int safety_level= -1;

  safety_level = (int)strtol(safety_level_string, &endptr, 10);
  /* If not all of string was used - fail */
  if(endptr && *endptr)
    safety_level= -1;
  if(safety_level < 1 || safety_level > SAFETY_LEVEL_COUNT) {
    int i;
    for(i = 1; i< SAFETY_LEVEL_COUNT; i++)
      if(!strcmp(flickcurl_safety_level_labels[i], safety_level_string)) {
        safety_level = i;
        break;
      }
  }

  return safety_level;
}


#define HIDDEN_COUNT 2
static const char* flickcurl_hidden_labels[HIDDEN_COUNT + 1] =
  {"unknown", "public", "hidden" };


/**
 * flickcurl_get_hidden_label:
 * @hidden: safety level index
 * 
 * Get label for a hidden status
 *
 * Return value: pointer to shared string label for hidden status or "unknown"
 **/
const char*
flickcurl_get_hidden_label(int hidden)
{
  if(hidden < 1 || hidden > HIDDEN_COUNT)
    hidden = 0;

  return flickcurl_hidden_labels[hidden];
}


/**
 * flickcurl_get_hidden_from_string:
 * @hidden_string: string
 * 
 * Get the enumeration value for a hidden status string.
 *
 * Parses the string value into a safety level either from an
 * integer form like '1' or a label like 'hidden'.
 * 
 * Returns: safety level enumeration value or <0 on error
 **/
int
flickcurl_get_hidden_from_string(const char* hidden_string)
{
  char* endptr = NULL;
  int hidden = -1;

  hidden = (int)strtol(hidden_string, &endptr, 10);
  /* If not all of string was used - fail */
  if(endptr && *endptr)
    hidden = -1;

  if(hidden < 1 || hidden > HIDDEN_COUNT) {
    int i;
    for(i = 1; i< HIDDEN_COUNT; i++)
      if(!strcmp(flickcurl_hidden_labels[i], hidden_string)) {
        hidden = i;
        break;
      }
  }

  return hidden;
}


#define FEED_FORMAT_COUNT 8
static struct {
  const char* name;
  const char* label;
  const char* mime_type;
}
flickcurl_feed_format_info[FEED_FORMAT_COUNT+1] = {
  { "feed-rss_100", "RSS 1.0", "application/rdf+xml" },
  { "feed-rss_200", "RSS 2.0", "application/rss+xml" },
  { "feed-atom_10", "Atom 1.0", "application/atom+xml" },
  { "feed-georss",  "RSS 2.0 with GeoRSS and W3C Geo for geotagged photos", "application/rss+xml" },
  { "feed-geoatom", "Atom 1.0 with GeoRSS and W3C Geo for geotagged photos", "application/atom+xml" },
  { "feed-geordf",  "RSS 1.0 with GeoRSS and W3C Geo for geotagged photos", "application/rdf+xml" },
  { "feed-kml",     "KML 2.1", "application/vnd.google-earth.kml+xml" },
  { "feed-kml_nl",  "KML 2.1 network link", "application/vnd.google-earth.kml+xml" },
  { NULL, NULL, NULL }
};


/**
 * flickcurl_get_feed_format_info:
 * @feed_format: input param - feed format index
 * @name_p: output param - pointer to store feed format name
 * @label_p: output param - pointer to store feed format label
 * @mime_type_p: output param - pointer to store feed format mime type
 * 
 * Get feed format parameter value information
 *
 * As announced 2008-08-25 in
 * http://code.flickr.com/blog/2008/08/25/api-responses-as-feeds/
 *
 * Return value: non-0 if feed_format is out of range
 **/
int
flickcurl_get_feed_format_info(int feed_format,
                               const char** name_p,
                               const char** label_p,
                               const char** mime_type_p)
{
  if(feed_format < 0 || feed_format >= FEED_FORMAT_COUNT)
    return 1;

  if(name_p)
    *name_p = flickcurl_feed_format_info[feed_format].name;

  if(label_p)
    *label_p = flickcurl_feed_format_info[feed_format].label;

  if(mime_type_p)
    *mime_type_p = flickcurl_feed_format_info[feed_format].mime_type;

  return 0;
}


/*
 * flickcurl_append_photos_list_params:
 * @fc: fc
 * @list_params: in parameter - photos list paramater
 * @format_p: out parameter - result format requested or NULL
 *
 * INTERNAL - append #flickcurl_photos_list_params to parameter list for API call
 *
 * Return value: number of parameters added
 */
int
flickcurl_append_photos_list_params(flickcurl* fc,
                                    flickcurl_photos_list_params* list_params,
                                    const char** format_p)
{
  /* NOTE: These are SHARED and pointed to by flickcurl_prepare() to
   * build the URL */
  static char per_page_s[FLICKCURL_MAX_LIST_PARAM_COUNT];
  static char page_s[FLICKCURL_MAX_LIST_PARAM_COUNT];
  int this_count = 0;
  
  if(format_p)
    *format_p = NULL;

  if(!list_params)
    return 0;
  
  if(list_params->extras) {
    flickcurl_add_param(fc, "extras", list_params->extras);
    this_count++;
  }
  if(list_params->per_page) {
    if(list_params->per_page >= 0 && list_params->per_page <= 999) {
      sprintf(per_page_s, "%d", list_params->per_page);
      flickcurl_add_param(fc, "per_page", per_page_s);
      this_count++;
    }
  }
  if(list_params->page) {
    if(list_params->page >= 0 && list_params->page <= 999) {
      sprintf(page_s, "%d", list_params->page);
      flickcurl_add_param(fc, "page", page_s);
      this_count++;
    }
  }
  if(list_params->format) {
    flickcurl_add_param(fc, "format", list_params->format);
    this_count++;

    if(format_p)
      *format_p = list_params->format;
  }

  return this_count;
}



#define EXTRAS_FORMAT_COUNT 22
static struct {
  const char* name;
  const char* label;
}
flickcurl_extras_format_info[EXTRAS_FORMAT_COUNT+1] = {
  { "date_taken", "Date item was taken"},
  { "date_upload", "Date item was uploaded"},
  { "geo", "Geotagging latitude, longitude and accuracy"},
  { "icon_server", "Item owner icon fields"},
  { "last_update", "Date item was last updated"},
  { "license", "Item License "},
  { "machine_tags", "Machine tags"},
  { "media", "Item Format: photo or video"},
  { "o_dims", "Original item dimensions"},
  { "original_format", "Original item secret and format"},
  { "owner_name", "Item owner ID"},

  /* http://tech.groups.yahoo.com/group/yws-flickr/message/5053 */
  { "path_alias", "Path alias for owner like /photos/USERNAME"},

  { "tags", "Item clean tags (safe for HTML, URLs)"},

  /* https://www.flickr.com/services/api/misc.urls.html */
  { "url_c", "URL of medium 800, 800 on longest size image"},
  { "url_m", "URL of small, medium size image"},
  { "url_n", "URL of small, 320 on longest side size image"},
  { "url_o", "URL of original size image"},
  { "url_q", "URL of large square 150x150 size image"},
  { "url_s", "URL of small suqare 75x75 size image"},
  { "url_sq", "URL of square size image"},
  { "url_t", "URL of thumbnail, 100 on longest side size image"},

  { "views", "Number of times item has been viewed"},

  { NULL, NULL }
};


/**
 * flickcurl_get_extras_format_info:
 * @extras_format: input param - extras format index
 * @name_p: output param - pointer to store feed format name
 * @label_p: output param - pointer to store feed format label
 * 
 * Get APi extras format parameter value information
 *
 * As described 2008-08-19 in
 * http://code.flickr.com/blog/2008/08/19/standard-photos-response-apis-for-civilized-age/
 *
 * Return value: non-0 if extras_format is out of range
 **/
int
flickcurl_get_extras_format_info(int extras_format,
                                 const char** name_p,
                                 const char** label_p)
{
  if(extras_format < 0 || extras_format >= EXTRAS_FORMAT_COUNT)
    return 1;

  if(name_p)
    *name_p = flickcurl_extras_format_info[extras_format].name;

  if(label_p)
    *label_p = flickcurl_extras_format_info[extras_format].label;

  return 0;
}


/**
 * flickcurl_photos_list_params_init:
 * @list_params: photos list params to init
 *
 * Initialise an existing photos list parameter structure
 *
 * Return value: non-0 on failure
 */
int
flickcurl_photos_list_params_init(flickcurl_photos_list_params* list_params)
{
  if(!list_params)
    return 1;
  
  memset(list_params, '\0', sizeof(*list_params));
  list_params->version = 1;

  list_params->extras = NULL;
  list_params->format = NULL;
  list_params->page= -1;
  list_params->per_page= -1;

  return 0;
}


/**
 * flickcurl_search_params_init:
 * @params: search params to init
 *
 * Initialise an existing search parameters structure
 *
 * Return value: non-0 on failure
 */
int
flickcurl_search_params_init(flickcurl_search_params* params)
{
  memset(params, '\0', sizeof(flickcurl_search_params));

  /* These are the numeric fields and are all set to 0 or 0.0 by the memset() */
#if 0
  params->min_upload_date = 0;
  params->max_upload_date = 0;
  params->accuracy = 0;
  params->safe_search = 0;
  params->content_type = 0;
  params->per_page = 0;
  params->page= 0;
  params->has_geo = 0;
  /* strictly lat and lon are ignored if radius is 0.0 */
  params->lat = 0.0;
  params->lon = 0.0;
  params->radius = 0.0;
  params->woe_id = 0;
  params->geo_context = 0;
  params->is_commons = 0;
#endif

  /* The remaining fields are pointers and are set to NULL by the memset() */

  return 0;
}


/**
 * flickcurl_set_curl_setopt_handler:
 * @fc: flickcurl object
 * @curl_handler: curl set options handler (or NULL)
 * @curl_handler_data: user data for handler (or NULL)
 *
 * Set curl set option callback handler.
 *
 * This handler is called for every curl request after all internal
 * curl_easy_setopt calls are made on the internal CURL* handle and
 * just before curl_easy_perform is invoked to start the retrieval.
 * Thus, this callback can override any internal configuration.
 *
 * If a simple once-only CURL configuration is needed, using
 * flickcurl_new_with_handle() may be easier.
 *
 * WARNING: The @curl_handler callback is called with 2 void args in
 * the order user data (@curl_handler_data value), curl_handle (CURL*
 * pointer) - take care to use them correct in implementation.
 *
 */
void
flickcurl_set_curl_setopt_handler(flickcurl *fc,
                                  flickcurl_curl_setopt_handler curl_handler,
                                  void* curl_handler_data)
{
  fc->curl_setopt_handler = curl_handler;
  fc->curl_setopt_handler_data = curl_handler_data;
}
