/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl_internal.h - Flickcurl internal API calls
 *
 * All API calls and defines here many change in any release.
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

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <mtwist_config.h>
#include <mtwist.h>

#if defined (OFFLINE) && defined (CAPTURE)
#error "Cannot define both OFFLINE and CAPTURE"
#endif


#ifdef FLICKCURL_DEBUG

#ifndef FLICKCURL_ASSERT_DIE
#define FLICKCURL_ASSERT_DIE abort();
#endif

#else
/* No debugging messages */
#ifndef FLICKCURL_ASSERT_DIE
#define FLICKCURL_ASSERT_DIE
#endif

#endif


#ifdef FLICKCURL_DISABLE_ASSERT_MESSAGES
#define FLICKCURL_ASSERT_REPORT(line)
#else
#define FLICKCURL_ASSERT_REPORT(msg) fprintf(stderr, "%s:%d: (%s) assertion failed: " msg "\n", __FILE__, __LINE__, __func__);
#endif


#ifdef FLICKCURL_DISABLE_ASSERT

#define FLICKCURL_ASSERT(condition, msg) 
#define FLICKCURL_ASSERT_RETURN(condition, msg, ret) 
#define FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(pointer, type) do { \
  if(!pointer) \
    return; \
} while(0)
#define FLICKCURL_ASSERT_OBJECT_POINTER_RETURN_VALUE(pointer, type, ret) do { \
  if(!pointer) \
    return ret; \
} while(0)

#else

#define FLICKCURL_ASSERT(condition, msg) do { \
  if(condition) { \
    FLICKCURL_ASSERT_REPORT(msg) \
    FLICKCURL_ASSERT_DIE \
  } \
} while(0)

#define FLICKCURL_ASSERT_RETURN(condition, msg, ret) do { \
  if(condition) { \
    FLICKCURL_ASSERT_REPORT(msg) \
    FLICKCURL_ASSERT_DIE \
    return ret; \
  } \
} while(0)

#define FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(pointer, type) do { \
  if(!pointer) { \
    FLICKCURL_ASSERT_REPORT("object pointer of type " #type " is NULL.") \
    FLICKCURL_ASSERT_DIE \
    return; \
  } \
} while(0)

#define FLICKCURL_ASSERT_OBJECT_POINTER_RETURN_VALUE(pointer, type, ret) do { \
  if(!pointer) { \
    FLICKCURL_ASSERT_REPORT("object pointer of type " #type " is NULL.") \
    FLICKCURL_ASSERT_DIE \
    return ret; \
  } \
} while(0)

#endif


/* flickcurl.c */
/* Prepare Flickr API request - GET or POST with URI parameters with auth */
int flickcurl_prepare(flickcurl *fc, const char* method);
/* Prepare Flickr API request - GET or POST with URI parameters without auth */
int flickcurl_prepare_noauth(flickcurl *fc, const char* method);
/* Prepare Flickr API request - POST with form-data parameters */
int flickcurl_prepare_upload(flickcurl *fc, const char* url, const char* upload_field, const char* upload_value);

/* Invoke Flickr API at URi prepared above and get back an XML document DOM */
xmlDocPtr flickcurl_invoke(flickcurl *fc);
/* Invoke Flickr API at URi prepared above and get back raw content */
char* flickcurl_invoke_get_content(flickcurl *fc, size_t* size_p);
/* Invoke URI prepared above and get back 'count' key/values */
char** flickcurl_invoke_get_form_content(flickcurl *fc, int* count_p);
void flickcurl_free_form(char **form, int count);

/* args.c */
void flickcurl_free_arg(flickcurl_arg *arg);
flickcurl_arg** flickcurl_build_args(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* arg_count_p);

/* blog.c */
flickcurl_blog** flickcurl_build_blogs(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* blog_count_p);
flickcurl_blog_service** flickcurl_build_blog_services(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* blog_services_count_p);

/* collection.c */
flickcurl_collection** flickcurl_build_collections(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* collection_count_p);
flickcurl_collection* flickcurl_build_collection(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* root_xpathExpr);


/* common.c */
/* invoke an error */
void flickcurl_error(flickcurl* fc, const char *message, ...);

/* Convert a unix timestamp into an ISO dateTime string */
char* flickcurl_unixtime_to_isotime(time_t unix_time);

/* Convert a unix timestamp into an SQL timestamp string */
char* flickcurl_unixtime_to_sqltimestamp(time_t unix_time);

/* Convert a SQL timestamp to an ISO dateTime string */
char* flickcurl_sqltimestamp_to_isotime(const char* timestamp);

/* Evaluate an XPath to get the string value */
char* flickcurl_xpath_eval(flickcurl *fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);
char* flickcurl_xpath_eval_to_tree_string(flickcurl* fc, xmlXPathContextPtr xpathNodeCtx, const xmlChar* xpathExpr, size_t* length_p);

char* flickcurl_call_get_one_string_field(flickcurl* fc, const char* key, const char* value, const char* method, const xmlChar* xpathExpr);

int flickcurl_append_photos_list_params(flickcurl* fc, flickcurl_photos_list_params* list_params, const char** format_p);

void flickcurl_init_params(flickcurl *fc, int is_write);
void flickcurl_add_param(flickcurl *fc, const char* key, const char* value);
void flickcurl_end_params(flickcurl *fc);


/* activity.c */
flickcurl_activity** flickcurl_build_activities(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* activity_count_p);

/* category.c */
flickcurl_category** flickcurl_build_categories(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* category_count_p);

/* comments.c */
flickcurl_comment** flickcurl_build_comments(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* comment_count_p);

/* contacts.c */
flickcurl_contact** flickcurl_build_contacts(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* contact_count_p);

/* context.c */
flickcurl_context** flickcurl_build_contexts(flickcurl* fc, xmlDocPtr doc);

/* exif.c */
flickcurl_exif** flickcurl_build_exifs(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* exif_count_p);

/* activity.c */
flickcurl_gallery** flickcurl_build_galleries(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* gallery_count_p);

/* group.c */
flickcurl_group** flickcurl_build_groups(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* group_count_p);

/* institution.c */
flickcurl_institution** flickcurl_build_institutions(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* institution_count_p);
flickcurl_institution* flickcurl_build_institution(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* location.c */
flickcurl_location* flickcurl_build_location(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* machinetags.c */
flickcurl_tag_namespace** flickcurl_build_tag_namespaces(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* namespace_count_p);
flickcurl_tag_namespace* flickcurl_build_tag_namespace(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* root_xpathExpr);
flickcurl_tag_predicate_value** flickcurl_build_tag_predicate_values(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int content_mode, int* predicate_value_count_p);

/* user_upload_status.c */
flickcurl_user_upload_status* flickcurl_build_user_upload_status(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* md5.c - MD5 as hex string */
extern char* MD5_string(char *string);

/* members.c */
flickcurl_member** flickcurl_build_members(flickcurl* fc,  xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* member_count_p);

/* method.c */
flickcurl_method* flickcurl_build_method(flickcurl* fc, xmlXPathContextPtr xpathCtx);

/* note.c  */
void flickcurl_free_note(flickcurl_note *note);
flickcurl_note** flickcurl_build_notes(flickcurl* fc, flickcurl_photo* photo, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* note_count_p);

/* perms.c */
flickcurl_perms* flickcurl_build_perms(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* person.c */
flickcurl_person** flickcurl_build_persons(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* person_count_p);
flickcurl_person* flickcurl_build_person(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* root_xpathExpr);

/* photo.c */
flickcurl_photos_list* flickcurl_new_photos_list(flickcurl* fc);
flickcurl_photo** flickcurl_build_photos(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* photo_count_p);
flickcurl_photo* flickcurl_build_photo(flickcurl* fc, xmlXPathContextPtr xpathCtx);
flickcurl_photos_list* flickcurl_invoke_photos_list(flickcurl* fc, const xmlChar* xpathExpr, const char* format);

/* photoset.c */
flickcurl_photoset** flickcurl_build_photosets(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* photoset_count_p);
flickcurl_photoset* flickcurl_build_photoset(flickcurl* fc, xmlXPathContextPtr xpathCtx);

/* place.c */
flickcurl_place** flickcurl_build_places(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* place_count_p);
flickcurl_place* flickcurl_build_place(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);
flickcurl_place_type_info** flickcurl_build_place_types(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* place_type_count_p);

/* shape.c */
flickcurl_shapedata** flickcurl_build_shapes(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* shape_count_p);
flickcurl_shapedata* flickcurl_build_shape(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* size.c */
flickcurl_size** flickcurl_build_sizes(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* size_count_p);

/* stat.c */
flickcurl_stat** flickcurl_build_stats(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* stat_count_p);

/* tags.c  */
flickcurl_tag** flickcurl_build_tags(flickcurl* fc, flickcurl_photo* photo, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* tag_count_p);
flickcurl_tag** flickcurl_build_tags_from_string(flickcurl* fc, flickcurl_photo* photo, const char *string, int *tag_count_p);
flickcurl_tag_clusters* flickcurl_build_tag_clusters(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* ticket.c */
flickcurl_ticket** flickcurl_build_tickets(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr, int* ticket_count_p);

/* vsnprintf.c */
extern char* my_vsnprintf(const char *message, va_list arguments);

/* video.c */
flickcurl_video* flickcurl_build_video(flickcurl* fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);


/* flickcurl_photos_search_params */
#define FLICKCURL_MAX_PARAM_COUNT 30

#define FLICKCURL_MAX_LIST_PARAM_COUNT 4

#define FLICKCURL_MAX_OAUTH_PARAM_COUNT 8

#define FLICKCURL_TOTAL_PARAM_COUNT (FLICKCURL_MAX_PARAM_COUNT + FLICKCURL_MAX_LIST_PARAM_COUNT + FLICKCURL_MAX_OAUTH_PARAM_COUNT + 1)

struct flickcurl_chunk_s {
  char* content;
  size_t size;
  struct flickcurl_chunk_s *prev;
};

typedef struct flickcurl_chunk_s flickcurl_chunk;


typedef struct {
  /* client credentials */
  char* client_key; /* AKA consumer key or the Flickr API key */
  size_t client_key_len;
  char* client_secret;
  size_t client_secret_len;

  /* temporary credentials */
  char* request_token;
  size_t request_token_len;
  char* request_token_secret;
  size_t request_token_secret_len;
  
  const char* verifier; /* OAuth verifier - always shared */
  size_t verifier_len;

  /* token credentials */
  char* token; /* AKA access token */
  size_t token_len;
  char* token_secret;
  size_t token_secret_len;

  /* username and user_nsid */
  char* username;
  size_t username_len;
  char* user_nsid;
  size_t user_nsid_len;
  
  /* callback URI or NULL for "oob" */
  const char* callback; /* always shared */
  size_t callback_len;

  /* Generally leave as NULL to make it new each time */
  char* nonce;

  /* Generally leave as 0 to make it use current time with gettimeofday() */
  time_t timestamp;

  /* HMAC-SHA1 key */
  unsigned char *key;
  size_t key_len;

  /* HMAC-SHA1 data */
  unsigned char* data;
  size_t data_len;
} flickcurl_oauth_data;


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
  
  /* uri buffer for internal use of size @uri_len */
  char* uri;

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

  /* Flickr legacy auth token secret - flickcurl_set_shared_secret() */
  char* secret;

  /* Flickr application/api key  - flickcurl_set_api_key() 
   *
   * Not owned freed here - a pointer into od.client_key
   *
   * FIXME: remove at some point
   */
  char* api_key;

  /* Flickr legacy auth token - flickcurl_set_auth_token() */
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

  /* if non-0 then run content through an XML parser and make a DOM in @xc */
  int xml_parse_content;
  
  /* if non-0 then save content */
  int save_content;

  /* saved content */
  /* reverse-ordered list of chunks of data read */
  flickcurl_chunk* chunks;
  /* count of chunks */
  int chunks_count;

  /* Web Service URI that is called */
  char *service_uri;

  /* Upload Web Service URI that is called */
  char *upload_service_uri;

  /* Replace Web Service URI that is called */
  char *replace_service_uri;

  unsigned int uri_len;

  flickcurl_curl_setopt_handler curl_setopt_handler;
  void* curl_setopt_handler_data;

  /* OAuth request token Web Service URI */
  char *oauth_request_token_uri;

  /* OAuth authorize URI */
  char *oauth_authorize_uri;

  /* OAuth access token Web Service URI */
  char *oauth_access_token_uri;

  /*  Used for OAuth nonce generation */
  mtwist* mt;

  flickcurl_oauth_data od;

  const char *parameters[FLICKCURL_TOTAL_PARAM_COUNT][2];
  int count;
};

struct flickcurl_serializer_s
{
  flickcurl* fc;
  void *data;
  flickcurl_serializer_factory* factory;
};

void flickcurl_serializer_init(void);
void flickcurl_serializer_terminate(void);


/* sha1.c */
#define SHA1_DIGEST_LENGTH 20
unsigned char* flickcurl_hmac_sha1(const void *data, size_t data_len, const void *key, size_t key_len);


/* legacy-auth.c */
int flickcurl_legacy_prepare_common(flickcurl *fc, const char* url, const char* method, const char* upload_field, const char* upload_value, int parameters_in_url, int need_auth);

/* oauth.c */
void flickcurl_oauth_free(flickcurl_oauth_data* od);
char* flickcurl_oauth_compute_signature(flickcurl_oauth_data* od, size_t* len_p);
int flickcurl_oauth_prepare_common(flickcurl *fc, const char* url, const char* method, const char* upload_field, const char* upload_value, int parameters_in_url, int need_auth);
