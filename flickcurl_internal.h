/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl_internal.h - Flickcurl internal API calls
 *
 * All API calls and defines here many change in any release.
 *
 * Copyright (C) 2007, David Beckett http://purl.org/net/dajobe/
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

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


#if defined (OFFLINE) && defined (CAPTURE)
#error "Cannot define both OFFLINE and CAPTURE"
#endif

/* flickcurl.c */
/* Prepare Flickr API request - GET or POST with URI parameters */
int flickcurl_prepare(flickcurl *fc, const char* method, const char* parameters[][2], int count);
/* Prepare Flickr API request - POST with form-data parameters */
int flickcurl_prepare_upload(flickcurl *fc, const char* url, const char* upload_field, const char* upload_value, const char* parameters[][2], int count);

/* Invoke Flickr API at URi prepared above and get back an XML document */
xmlDocPtr flickcurl_invoke(flickcurl *fc);

/* args.c */
void flickcurl_free_arg(flickcurl_arg *arg);
flickcurl_arg** flickcurl_build_args(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* arg_count_p);

/* common.c */
/* invoke an error */
void flickcurl_error(flickcurl* fc, const char *message, ...);

/* Convert a unix timestamp into an ISO string */
char* flickcurl_unixtime_to_isotime(time_t unix_time);

/* Convert a unix timestamp into an SQL timestamp string */
char* flickcurl_unixtime_to_sqltimestamp(time_t unix_time);

/* Evaluate an XPath to get the string value */
char* flickcurl_xpath_eval(flickcurl *fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

char* flickcurl_call_get_one_string_field(flickcurl* fc, const char* key, const char* value, const char* method, const xmlChar* xpathExpr);

/* comments.c */
flickcurl_comment** flickcurl_build_comments(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* comment_count_p);

/* contacts.c */
flickcurl_contact** flickcurl_build_contacts(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* contact_count_p);

/* context.c */
flickcurl_context** flickcurl_build_contexts(flickcurl* fc, xmlDocPtr doc);

/* md5.c - MD5 as hex string */
extern char* MD5_string(char *string);

/* method.c */
flickcurl_method* flickcurl_build_method(flickcurl* fc, xmlXPathContextPtr xpathCtx);

/* perms.c */
flickcurl_perms* flickcurl_build_perms(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* person.c */
flickcurl_person* flickcurl_build_person(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* root_xpathExpr);

/* photo.c */
flickcurl_photo** flickcurl_build_photos(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* photo_count_p);
flickcurl_photo* flickcurl_build_photo(flickcurl* fc, xmlXPathContextPtr xpathCtx);

/* tags.c  */
flickcurl_tag** flickcurl_build_tags(flickcurl* fc, flickcurl_photo* photo, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* tag_count_p);

/* vsnprintf.c */
extern char* my_vsnprintf(const char *message, va_list arguments);


struct flickcurl_s {
  int total_bytes;

  /* Something failed */
  int failed;

  /* Flickr API error code */
  int error_code;

  /* Flickr API error message */
  char* error_msg;
  
  int status_code;

  char** param_fields;
  char** param_values;
  int parameter_count;
  char* upload_field;
  char* upload_value;
  
  char uri[2048];

  CURL* curl_handle;
  char error_buffer[CURL_ERROR_SIZE];
  int curl_init_here;

  char* user_agent;

  /* proxy URL string or NULL for none */
  char* proxy;
  
  void* error_data;
  flickcurl_message_handler error_handler;

  char *http_accept;

  /* XML parser */
  xmlParserCtxtPtr xc;

  /* The next three fields need to be set before authenticated
   * operations can be done (in most cases).
   */

  /* Flickr shared secret - flickcurl_set_shared_secret() */
  char* secret;

  /* Flickr application/api key  - flickcurl_set_api_key() */
  char* api_key;

  /* Flickr authentication token - flickcurl_set_auth_token() */
  char* auth_token;

  /* API call must be signed even if 'auth_token' is NULL - flickcurl_set_sign()
   */
  int sign;

  /* Flickr API method to invoke - set by flickcurl_prepare */
  char* method;

  flickcurl_tag_handler tag_handler;
  void* tag_data;

  /* licenses returned by flickr.photos.licenses.getInfo 
   * as initialised by flickcurl_read_licenses() 
   */
  flickcurl_license** licenses;

  /* Time the last request was made */
  struct timeval last_request_time;
  
  /* Delay between HTTP requests in microseconds - default is none (0) */
  long request_delay;

  /* write = POST, else read = GET */
  int is_write;
  
  /* data to send in a request */
  void* data;
  size_t data_length;
  int data_is_xml; /* if non-0, us xmlFree(fc->data) else free(fc->data) */
  
#ifdef CAPTURE
  FILE* fh;
#endif
};
