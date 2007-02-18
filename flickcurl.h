/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl.h - Flickcurl API
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


typedef enum {
  PHOTO_FIELD_none,
  PHOTO_FIELD_dateuploaded,
  PHOTO_FIELD_farm,
  PHOTO_FIELD_isfavorite,
  PHOTO_FIELD_license,
  PHOTO_FIELD_originalformat,
  PHOTO_FIELD_rotation,
  PHOTO_FIELD_server,
  PHOTO_FIELD_dates_lastupdate,
  PHOTO_FIELD_dates_posted,
  PHOTO_FIELD_dates_taken,
  PHOTO_FIELD_dates_takengranularity,
  PHOTO_FIELD_description,
  PHOTO_FIELD_editability_canaddmeta,
  PHOTO_FIELD_editability_cancomment,
  PHOTO_FIELD_geoperms_iscontact,
  PHOTO_FIELD_geoperms_isfamily,
  PHOTO_FIELD_geoperms_isfriend,
  PHOTO_FIELD_geoperms_ispublic,
  PHOTO_FIELD_location_accuracy,
  PHOTO_FIELD_location_latitude,
  PHOTO_FIELD_location_longitude,
  PHOTO_FIELD_owner_location,
  PHOTO_FIELD_owner_nsid,
  PHOTO_FIELD_owner_realname,
  PHOTO_FIELD_owner_username,
  PHOTO_FIELD_title,
  PHOTO_FIELD_visibility_isfamily,
  PHOTO_FIELD_visibility_isfriend,
  PHOTO_FIELD_visibility_ispublic,
  PHOTO_FIELD_secret,
  PHOTO_FIELD_originalsecret,
  PHOTO_FIELD_LAST = PHOTO_FIELD_originalsecret
} flickcurl_photo_field;


typedef enum {
  VALUE_TYPE_NONE, /* empty field */
  VALUE_TYPE_PHOTO_ID, /* internal */
  VALUE_TYPE_PHOTO_URI, /* internal */
  VALUE_TYPE_UNIXTIME,
  VALUE_TYPE_BOOLEAN,
  VALUE_TYPE_DATETIME,
  VALUE_TYPE_FLOAT,
  VALUE_TYPE_INTEGER,
  VALUE_TYPE_STRING,
  VALUE_TYPE_URI,
  VALUE_TYPE_PERSON_ID, /* internal */
  VALUE_TYPE_LAST = VALUE_TYPE_PERSON_ID
} flickcurl_field_value_type;
  

typedef struct flickcurl_s flickcurl;

struct flickcurl_photo_s;

typedef struct flickcurl_tag_s {
  struct flickcurl_photo_s* photo;
  char* id;
  char* author;
  char* raw;
  char* cooked;
  int machine_tag;
} flickcurl_tag;


typedef struct flickcurl_photo_s {
  /* photo id */
  char *id;
  /* photo page uri */
  char *uri;
  
  flickcurl_tag* tags[20];
  int tags_count;
  
  struct {
    char* string;
    int integer;
    flickcurl_field_value_type type;
  } fields[PHOTO_FIELD_LAST + 1];
} flickcurl_photo;


typedef struct {
  /* license id */
  int id;
  /* license url or NULL if none */
  char *url;
  /* license name */
  char *name;
} flickcurl_license;


typedef enum {
  FLICKCURL_CONTEXT_NONE,
  FLICKCURL_CONTEXT_SET,
  FLICKCURL_CONTEXT_POOL,
  FLICKCURL_CONTEXT_PREV,
  FLICKCURL_CONTEXT_NEXT,
  FLICKCURL_CONTEXT_LAST = FLICKCURL_CONTEXT_NEXT
} flickcurl_context_type;


typedef struct flickcurl_context_s {
  flickcurl_context_type type;
  char* id;
  char* secret; /* may be NULL */
  int server;   /* may be 0 */
  int farm;     /* may be 0 */
  char* title;  /* may be NULL */
  char* url;    /* may be NULL */
  char* thumb;  /* may be NULL */
} flickcurl_context;


typedef enum {
  PERSON_FIELD_none,
  PERSON_FIELD_isadmin, /* boolean */
  PERSON_FIELD_ispro, /* boolean */
  PERSON_FIELD_iconserver, /* integer */
  PERSON_FIELD_iconfarm, /* integer - not in API docs */
  PERSON_FIELD_username, /* string */
  PERSON_FIELD_realname, /* string */
  PERSON_FIELD_mbox_sha1sum, /* string */
  PERSON_FIELD_location, /* string */
  PERSON_FIELD_photosurl, /* string */
  PERSON_FIELD_profileurl, /* string */
  PERSON_FIELD_mobileurl, /* string - not in API docs */
  PERSON_FIELD_photos_firstdate, /* dateTime */
  PERSON_FIELD_photos_firstdatetaken, /* dateTime */
  PERSON_FIELD_photos_count, /* integer */
  PERSON_FIELD_LAST = PERSON_FIELD_photos_count
} flickcurl_person_field;


typedef struct {
  /* user nsid */
  char *nsid;

  struct {
    char* string;
    int integer;
    flickcurl_field_value_type type;
  } fields[PERSON_FIELD_LAST + 1];
} flickcurl_person;


typedef void (*flickcurl_message_handler)(void *user_data, const char *message);

typedef void (*set_config_var_handler)(void* userdata, const char* key, const char* value);

typedef void (*flickcurl_tag_handler)(void *user_data, flickcurl_tag* tag);


extern const char* const flickcurl_short_copyright_string;
extern const char* const flickcurl_copyright_string;
extern const char* const flickcurl_license_string;
extern const char* const flickcurl_home_url_string;
extern const char* const flickcurl_version_string;


/* constructor */
flickcurl* flickcurl_new(void);

/* destructor */
void flickcurl_free(flickcurl *fc);

/* error handler */
void flickcurl_set_error_handler(flickcurl* fc, flickcurl_message_handler error_handler,  void *error_data);
void flickcurl_set_tag_handler(flickcurl* fc,  flickcurl_tag_handler tag_handler, void *tag_data);

/* set methods */
void flickcurl_set_user_agent(flickcurl* fc, const char *user_agent);
void flickcurl_set_proxy(flickcurl* fc, const char *proxy);
void flickcurl_set_http_accept(flickcurl* fc, const char *value);
void flickcurl_set_api_key(flickcurl* fc, const char *api_key);
void flickcurl_set_shared_secret(flickcurl* fc, const char *secret);
void flickcurl_set_auth_token(flickcurl *fc, const char* auth_token);
void flickcurl_set_sig_key(flickcurl *fc, const char* sig_key);
void flickcurl_set_request_delay(flickcurl *fc, long delay_msec);

/* get methods */
const char* flickcurl_get_api_key(flickcurl *fc);
const char* flickcurl_get_shared_secret(flickcurl *fc);
const char* flickcurl_get_auth_token(flickcurl *fc);

const char* flickcurl_get_photo_field_label(flickcurl_photo_field field);
const char* flickcurl_get_person_field_label(flickcurl_person_field field);
const char* flickcurl_get_field_value_type_label(flickcurl_field_value_type datatype);
const char* flickcurl_get_context_type_field_label(flickcurl_context_type type);

/* utility methods */
char* flickcurl_photo_as_source_uri(flickcurl_photo *photo, const char c);


/* Flickr API calls */
char* flickcurl_auth_getFullToken(flickcurl* fc, const char* frob);

flickcurl_context** flickcurl_groups_pools_getContext(flickcurl* fc, const char* photo_id, const char* group_id);

flickcurl_person* flickcurl_people_getInfo(flickcurl* fc, const char* user_id);

flickcurl_context** flickcurl_photos_getAllContexts(flickcurl* fc, const char* photo_id);
flickcurl_context** flickcurl_photos_getContext(flickcurl* fc, const char* photo_id);
flickcurl_photo* flickcurl_photos_getInfo(flickcurl *fc, const char* photo_id);

flickcurl_license** flickcurl_photos_licenses_getInfo(flickcurl *fc);
flickcurl_license* flickcurl_photos_licenses_getInfo_by_id(flickcurl *fc, int id);

flickcurl_context** flickcurl_photosets_getContext(flickcurl* fc, const char* photo_id, const char* photoset_id);

int flickcurl_test_echo(flickcurl* fc, const char* key, const char* value);

char* flickcurl_urls_lookupUser(flickcurl* fc, const char* url);

void flickcurl_free_tag(flickcurl_tag *t);
void flickcurl_free_photo(flickcurl_photo *photo);
/* void flickcurl_free_license(flickcurl_person *license); */
void flickcurl_free_person(flickcurl_person *person);
void flickcurl_free_context(flickcurl_context *context);
void flickcurl_free_contexts(flickcurl_context** contexts);


/* config.c */
int read_ini_config(const char* filename, const char* application, void* user_data, set_config_var_handler handler);
