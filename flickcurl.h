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


#ifndef FLICKCURL_H
#define FLICKCURL_H


#ifdef __cplusplus
extern "C" {
#endif

/* needed for xmlDocPtr */
#include <libxml/tree.h>


/*
 * Field data types
 */
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
  

/*
 * Fields of a flickcurl_photo*
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
} flickcurl_photo_field_type;


/* The main object type */
typedef struct flickcurl_s flickcurl;
  

/* Forward structure references */
struct flickcurl_s;
struct flickcurl_photo_s;


/**
 * flickcurl_tag: 
 *
 * Most of these fields may be NULL, 0 for numbers
 * but not all.  Either 'raw' or 'cooked' MUST appear. 
 */
typedef struct flickcurl_tag_s {
  /* Associated photo object if any */
  struct flickcurl_photo_s* photo;
  char* id;
  char* author;
  char* authorname;
  char* raw;
  char* cooked;
  int machine_tag;
  int count;
} flickcurl_tag;


typedef struct {
  char* string;
  flickcurl_photo_field_type integer;
  flickcurl_field_value_type type;
} flickcurl_photo_field;


typedef struct flickcurl_photo_s {
  /* photo id */
  char *id;
  /* photo page uri */
  char *uri;
  
  flickcurl_tag** tags;
  int tags_count;
  
  flickcurl_photo_field fields[PHOTO_FIELD_LAST + 1];
} flickcurl_photo;


typedef struct {
  /* license id */
  int id;
  /* license url or NULL if none */
  char *url;
  /* license name */
  char *name;
} flickcurl_license;


/*
 * Types of photo context: relationship between photo and another item
 */
typedef enum {
  FLICKCURL_CONTEXT_NONE,
  FLICKCURL_CONTEXT_SET,  /* other thing is a set */
  FLICKCURL_CONTEXT_POOL, /* other thing is a pool */
  FLICKCURL_CONTEXT_PREV, /* other thing is a previous photo */
  FLICKCURL_CONTEXT_NEXT, /* other thing is a next photo */
  FLICKCURL_CONTEXT_LAST = FLICKCURL_CONTEXT_NEXT
} flickcurl_context_type;


typedef struct {
  flickcurl_context_type type;
  char* id;
  char* secret; /* may be NULL */
  int server;   /* may be 0 */
  int farm;     /* may be 0 */
  char* title;  /* may be NULL */
  char* url;    /* may be NULL */
  char* thumb;  /* may be NULL */
} flickcurl_context;


/*
 * Fields of a flickcurl_person*
 */
typedef enum {
  PERSON_FIELD_none,
  PERSON_FIELD_isadmin,               /* boolean */
  PERSON_FIELD_ispro,                 /* boolean */
  PERSON_FIELD_iconserver,            /* integer */
  PERSON_FIELD_iconfarm,              /* integer - not in API docs */
  PERSON_FIELD_username,              /* string */
  PERSON_FIELD_realname,              /* string */
  PERSON_FIELD_mbox_sha1sum,          /* string */
  PERSON_FIELD_location,              /* string */
  PERSON_FIELD_photosurl,             /* string */
  PERSON_FIELD_profileurl,            /* string */
  PERSON_FIELD_mobileurl,             /* string - not in API docs */
  PERSON_FIELD_photos_firstdate,      /* dateTime */
  PERSON_FIELD_photos_firstdatetaken, /* dateTime */
  PERSON_FIELD_photos_count,          /* integer */
  PERSON_FIELD_photos_views,          /* integer - not in API docs */
  PERSON_FIELD_LAST = PERSON_FIELD_photos_views
} flickcurl_person_field_type;


typedef struct {
  char* string;
  flickcurl_person_field_type integer;
  flickcurl_field_value_type type;
} flickcurl_person_field;
  

typedef struct {
  /* user nsid */
  char *nsid;

  flickcurl_person_field fields[PERSON_FIELD_LAST + 1];
} flickcurl_person;


/* callback handlers */
typedef void (*flickcurl_message_handler)(void *user_data, const char *message);
typedef void (*flickcurl_tag_handler)(void *user_data, flickcurl_tag* tag);


/* library constants */
extern const char* const flickcurl_short_copyright_string;
extern const char* const flickcurl_copyright_string;
extern const char* const flickcurl_license_string;
extern const char* const flickcurl_home_url_string;
extern const char* const flickcurl_version_string;


/* library init - call once before creating anything */
void flickcurl_init(void);
/* library cleanup - call once before exit */
void flickcurl_finish(void);


/* flickcurl* object constructor */
flickcurl* flickcurl_new(void);

/* flickcurl* object destructor */
void flickcurl_free(flickcurl *fc);

/* flickcurl* object set methods */
void flickcurl_set_api_key(flickcurl* fc, const char *api_key);
void flickcurl_set_auth_token(flickcurl *fc, const char* auth_token);
void flickcurl_set_data(flickcurl *fc, void* data, size_t data_length);
void flickcurl_set_error_handler(flickcurl* fc, flickcurl_message_handler error_handler,  void *error_data);
void flickcurl_set_http_accept(flickcurl* fc, const char *value);
void flickcurl_set_proxy(flickcurl* fc, const char *proxy);
void flickcurl_set_request_delay(flickcurl *fc, long delay_msec);
void flickcurl_set_shared_secret(flickcurl* fc, const char *secret);
void flickcurl_set_sign(flickcurl *fc);
void flickcurl_set_tag_handler(flickcurl* fc,  flickcurl_tag_handler tag_handler, void *tag_data);
void flickcurl_set_user_agent(flickcurl* fc, const char *user_agent);
void flickcurl_set_write(flickcurl *fc, int is_write);
void flickcurl_set_xml_data(flickcurl *fc, xmlDocPtr doc);

/* flickcurl* object set methods */
const char* flickcurl_get_api_key(flickcurl *fc);
const char* flickcurl_get_shared_secret(flickcurl *fc);
const char* flickcurl_get_auth_token(flickcurl *fc);

/* other flickcurl class destructors */
void flickcurl_free_tag(flickcurl_tag *t);
void flickcurl_free_photo(flickcurl_photo *photo);
/* void flickcurl_free_license(flickcurl_person *license); */
void flickcurl_free_person(flickcurl_person *person);
void flickcurl_free_context(flickcurl_context *context);
void flickcurl_free_contexts(flickcurl_context** contexts);


/* utility methods */
/* get an image URL for a photo in some size */
char* flickcurl_photo_as_source_uri(flickcurl_photo *photo, const char c);
/* get labels for various field/types */
const char* flickcurl_get_photo_field_label(flickcurl_photo_field_type field);
const char* flickcurl_get_person_field_label(flickcurl_person_field_type field);
const char* flickcurl_get_field_value_type_label(flickcurl_field_value_type datatype);
const char* flickcurl_get_context_type_field_label(flickcurl_context_type type);


/* read a 'INI' style configuration file  */
typedef void (*set_config_var_handler)(void* userdata, const char* key, const char* value);
int read_ini_config(const char* filename, const char* application, void* user_data, set_config_var_handler handler);



/* Flickr API calls */

/* flickr.auth */
char* flickcurl_auth_checkToken(flickcurl* fc, const char* token);
char* flickcurl_auth_getFrob(flickcurl* fc);
char* flickcurl_auth_getFullToken(flickcurl* fc, const char* frob);
char* flickcurl_auth_getToken(flickcurl* fc, const char* frob);

/* flickr.groups */
flickcurl_context** flickcurl_groups_pools_getContext(flickcurl* fc, const char* photo_id, const char* group_id);

/* flickr.people */
char* flickcurl_people_findByEmail(flickcurl* fc, const char* email);
char* flickcurl_people_findByUsername(flickcurl* fc, const char* username);
flickcurl_person* flickcurl_people_getInfo(flickcurl* fc, const char* user_id);

/* flickr.photos */
int flickcurl_photos_addTags(flickcurl* fc, const char* photo_id, const char* tags);
int flickcurl_photos_delete(flickcurl* fc, const char* photo_id);
flickcurl_context** flickcurl_photos_getAllContexts(flickcurl* fc, const char* photo_id);
flickcurl_context** flickcurl_photos_getContext(flickcurl* fc, const char* photo_id);
flickcurl_photo* flickcurl_photos_getInfo(flickcurl *fc, const char* photo_id);
int flickcurl_photos_removeTag(flickcurl* fc, const char* tag_id);
int flickcurl_photos_setTags(flickcurl* fc, const char* photo_id, const char* tags);

/* flickr.photos.licenses */
flickcurl_license** flickcurl_photos_licenses_getInfo(flickcurl *fc);
flickcurl_license* flickcurl_photos_licenses_getInfo_by_id(flickcurl *fc, int id);

/* flickr.photosets */
flickcurl_context** flickcurl_photosets_getContext(flickcurl* fc, const char* photo_id, const char* photoset_id);

/* flickr.tag */
flickcurl_tag** flickcurl_tags_getHotList(flickcurl* fc, const char* period, int tag_count);
flickcurl_tag** flickcurl_tags_getListPhoto(flickcurl* fc, const char* photo_id);
flickcurl_tag** flickcurl_tags_getListUser(flickcurl* fc, const char* user_id);
flickcurl_tag** flickcurl_tags_getListUserPopular(flickcurl* fc, const char* user_id, int pop_count);
flickcurl_tag** flickcurl_tags_getListUserRaw(flickcurl* fc, const char* tag);
flickcurl_tag** flickcurl_tags_getRelated(flickcurl* fc, const char* tag);

/* flickr.test */
int flickcurl_test_echo(flickcurl* fc, const char* key, const char* value);

/* flickr.urls */
char* flickcurl_urls_getGroup(flickcurl* fc, const char* group_id);
char* flickcurl_urls_getUserPhotos(flickcurl* fc, const char* user_id);
char* flickcurl_urls_getUserProfile(flickcurl* fc, const char* user_id);
char* flickcurl_urls_lookupGroup(flickcurl* fc, const char* url);
char* flickcurl_urls_lookupUser(flickcurl* fc, const char* url);

#ifdef __cplusplus
}
#endif

#endif
