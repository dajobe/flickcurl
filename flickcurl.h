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


/* Use gcc 3.1+ feature to allow marking of deprecated API calls.
 * This gives a warning during compiling.
 */
#if ( __GNUC__ == 3 && __GNUC_MINOR__ > 0 ) || __GNUC__ > 3
#ifdef __APPLE_CC__
/* OSX gcc cpp-precomp is broken */
#define FLICKCURL_DEPRECATED
#else
#define FLICKCURL_DEPRECATED __attribute__((deprecated))
#endif
#else
#define FLICKCURL_DEPRECATED
#endif

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
  PHOTO_FIELD_location_neighborhood,
  PHOTO_FIELD_location_locality,
  PHOTO_FIELD_location_region,
  PHOTO_FIELD_location_country,
  PHOTO_FIELD_LAST = PHOTO_FIELD_location_country,
} flickcurl_photo_field_type;


/* The main object type */
typedef struct flickcurl_s flickcurl;
  

/* Forward structure references */
struct flickcurl_s;
struct flickcurl_photo_s;


/**
 * flickcurl_arg:
 * @name: Argument name
 * @optional: boolean flag (non-0 true) if argument is optional
 * @description: description of argument (HTML)
 *
 * An API method argument.
 */
typedef struct flickcurl_arg_s {
  char* name;
  int optional;
  char *description;
} flickcurl_arg;


/**
 * flickcurl_method: 
 * @name: Method name
 * @needslogin: boolean flag (non-0 true) if method requires login
 * @description: description of method
 * @response: example response (HTML)
 * @explanation: explanation of example response or NULL if missing
 * @args: method arguments
 * @arg_count: number of arguments, may be 0
 *
 * An API method
 */
typedef struct flickcurl_method_s {
  char *name;
  int   needslogin;
  char *description;
  char *response;
  char *explanation;

  /* argument list */
  flickcurl_arg** args;
  int args_count;
  
} flickcurl_method;



/**
 * flickcurl_comment:
 * @name: Argument name
 * @optional: boolean flag (non-0 true) if argument is optional
 * @description: description of argument (HTML)
 *
 * An API method argument.
 */
typedef struct flickcurl_comment_s {
  char* id;
  char* author;
  char* authorname;
  int datecreate;
  char* permalink;
  char* text;
} flickcurl_comment;


/**
 * flickcurl_perms:
 * @is_public: non-0 to set the photo to public else private
 * @is_contact: 
 * @is_friend: non-0 to make the photo visible to friends when private
 * @is_family: non-0 to make the photo visible to family when private
 * @perm_comment: who can add comments to the photo and it's notes. one of: 0 nobody,  1 friends & family, 2 contacts, 3 everybody
 * @perm_addmeta: who can add notes and tags to the photo. one of: 0 nobody / just the owner, 1 friends & family, 2 contacts, 3 everybody
 *
 * Permissions as used by flickcurl_photos_getPerms() and 
 * flickcurl_photos_setPerms() which use public, friend, family,
 * perm_comment and perm-addmeta.  flickr.photos.geo.setPerms() uses
 * public, contact, friend and family.
 */
typedef struct 
{
  int is_public;
  int is_contact;
  int is_friend;
  int is_family;
  int perm_comment;
  int perm_addmeta;
} flickcurl_perms;


/**
 * flickcurl_location:
 * @latitude: The latitude from -90 to 90
 * @longitude: The longitude from -180 to 180
 * @accuracy: Recorded accuracy level of the location.
 *   World level is 1, Country is ~3, Region ~6, City ~11, Street
 *   ~16. Current range is 1-16. Defaults to 16 if not specified. (<0 for none)
 */
typedef struct 
{
  double latitude;
  double longitude;
  int accuracy;
} flickcurl_location;
  

/**
 * flickcurl_tag: 
 * @photo: Associated photo object if any
 * @id: tag identifier
 * @author: author (may be NULL)
 * @authornamae: author real name (may be NULL)
 * @raw: raw tag as user typed it (may be NULL, but if so @cooked must be not NULL)
 * @cooked: cooked tag (may be NULL, but if so @raw must not be NULL)
 * @machine_tag: boolean (non-0 true) if tag is a Machine Tag
 * @count: tag count in a histogram (or 0)
 *
 * A tag OR a posting of a tag about a photo by a user OR a tag in a histogram
 *
 * Most of these fields may be NULL, 0 for numbers
 * but not all.  Either @raw or @cooked MUST appear. 
 */
typedef struct flickcurl_tag_s {
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


/**
 * flickcurl_photo: 
 * @id: photo ID
 * @uri: photo page URI
 * @tags: array of tags (may be NULL)
 * @tags_count: size of tags array
 * @fields: photo fields
 *
 * A photo.
 *
 */
typedef struct flickcurl_photo_s {
  char *id;
  char *uri;
  
  flickcurl_tag** tags;
  int tags_count;
  
  flickcurl_photo_field fields[PHOTO_FIELD_LAST + 1];
} flickcurl_photo;


/**
 * flickcurl_license: 
 * @id: license ID
 * @url: license URL
 * @name: license short name
 *
 * A photo license.
 *
 */
typedef struct {
  /* license id */
  int id;
  /* license url or NULL if none */
  char *url;
  /* license name */
  char *name;
} flickcurl_license;


/**
 * flickcurl_contact:
 * @nsid: NSID
 * @username: user name
 * @iconserver:
 * @realname:
 * @is_friend:
 * @is_family:
 * @ignored:
 *
 * A contact.
 */
typedef struct flickcurl_contact_s {
  char *nsid;
  char *username;
  int iconserver;
  char *realname;
  int is_friend;
  int is_family;
  int ignored;
} flickcurl_contact;


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


/**
 * flickcurl_exif:
 * @tagspace: Tagspace name
 * @tagspaceid: ID of tagspace
 * @tag: tag ID
 * @label: tag label
 * @raw: raw tag name
 * @clean: pretty-formatted tag name
 *
 * An EXIF tag.
 */
typedef struct {
  char* tagspace;
  int tagspaceid;
  int tag;
  char* label;
  char* raw;
  char* clean;
} flickcurl_exif;


/**
 * flickcurl_group:
 * @nsid: NSID
 * @name: Group Name
 * @description: Description
 * @lang: Language
 * @is_admin: is admin flag
 * @is_pool_moderated: is the pool moderated
 * @is_eighteenplus: 18+ group
 * @privacy: privacy level
 * @photos: photos in group count
 * @iconserver: icon server ID
 * @members: member count
 * @throttle_count: throttle count
 * @throttle_mode: throttle mode (day, ...)
 * @throttle_remaining: throttle remaining
 *
 * A group.
 */
typedef struct {
  char* nsid;
  char* name;
  char* description;
  char* lang;
  int is_admin;
  int is_pool_moderated;
  int is_eighteenplus;
  int privacy;
  int photos;
  int iconserver;
  int members;
  int throttle_count;
  char* throttle_mode;
  int throttle_remaining;
} flickcurl_group;


/**
 * flickcurl_category:
 * @id: category ID
 * @name: Name
 * @path: path to category
 * @count: count
 *
 * A category.
 */
struct flickcurl_category_s {
  char* id;
  char* name;
  char* path;
  int count;
  struct flickcurl_category_s** categories;
  int categories_count;
  flickcurl_group** groups;
  int groups_count;
};
typedef struct flickcurl_category_s flickcurl_category;


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
  PERSON_FIELD_favedate,              /* dateTime - flickr.photos.getFavorites() */
  PERSON_FIELD_LAST = PERSON_FIELD_favedate
} flickcurl_person_field_type;


typedef struct {
  char* string;
  flickcurl_person_field_type integer;
  flickcurl_field_value_type type;
} flickcurl_person_field;
  

/**
 * flickcurl_person: 
 * @nsid: user NSID
 * @fields: person fields
 *
 * A flickr user.
 *
 */
typedef struct {
  char *nsid;

  flickcurl_person_field fields[PERSON_FIELD_LAST + 1];
} flickcurl_person;


/**
 * flickcurl_upload_params:
 * @photo_file: photo filename
 * @title: title or NULL
 * @description: description of photo (HTML) (or NULL)
 * @tags: space-separated list of tags (or NULL)
 * @is_public: is public photo boolean (non-0 true)
 * @is_friend: is friend photo boolean (non-0 true)
 * @is_family: is family photo boolean (non-0 true)
 * @safety_level: 1=safe, 2=moderate, 3=restricted
 * @content_type: 1=photo, 2=screenshot, 3=other/artwork
 *
 * Photo upload parameters
 */
typedef struct {
  const char* photo_file;
  const char *title;
  const char *description;
  const char *tags;  
  int is_public;
  int is_friend;
  int is_family;  
  int safety_level;
  int content_type;
} flickcurl_upload_params;


/**
 * flickcurl_upload_status:
 * @photoid: photo ID that was uploaded/replaced (upload only)
 * @secret: secret of photo uploaded (replace only)
 * @originalsecret: secret of original photo (replace only)
 * @ticketid: ticket ID for asynchronous upload (replace only)
 *
 * An upload response.
 *
 */
typedef struct {
  char *photoid;
  char *secret;
  char *originalsecret;
  char *ticketid;
} flickcurl_upload_status;


/**
 * flickcurl_search_params:
 * @user_id: The NSID of the user who's photo to search (or "me" or NULL).
 * @tags: A comma-delimited list of tags (or NULL)
 * @tag_mode: Either 'any' for an OR combination of tags, or 'all' for an AND combination. Defaults to 'any' if not specified (or NULL)
 * @text: Free text search (or NULL)
 * @min_upload_date: Minimum upload date as a unix timestamp (or NULL)
 * @max_upload_date: Maximum upload date as a unix timestamp (or NULL)
 * @min_taken_date: Minimum taken date in the form of a mysql datetime (or NULL)
 * @max_taken_date: Maximum taken date in the form of a mysql datetime (or NULL)
 * @license: Comma-separated list of photo licenses (or NULL)
 * @sort: The order in which to sort returned photos. Defaults to date-posted-desc. The possible values are: date-posted-asc, date-posted-desc, date-taken-asc, date-taken-desc, interestingness-desc, interestingness-asc, and relevance (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level.
 * @bbox: A comma-delimited list of 4 values defining the Bounding Box of the area that will be searched.
 * @accuracy: Recorded accuracy level of the location information.  Current range is 1-16 
 * @safe_search: Safe search setting: 1 safe, 2 moderate, 3 restricted.
 * @content_type: Content Type setting: 1 for photos only, 2 for screenshots only, 3 for 'other' only, 4 for all types. (or NULL)
 * @machine_tags: Machine tag search syntax 
 * @machine_tag_mode: Either 'any' for an OR combination of tags, or 'all' for an AND combination. Defaults to 'any' if not specified.
 * @group_id: The id of a group who's pool to search.  If specified, only matching photos posted to the group's pool will be returned. (or NULL)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: <code>license</code>, <code>date_upload</code>, <code>date_taken</code>, <code>owner_name</code>, <code>icon_server</code>, <code>original_format</code>, <code>last_update</code>, <code>geo</code>, <code>tags</code>, <code>machine_tags</code>. (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or NULL)
 *
 * Search parameters for &flickcurl_photos_search()
 */
typedef struct {
  char* user_id;
  char* tags;
  char* tag_mode;
  char* text;
  int min_upload_date;
  int max_upload_date;
  char* min_taken_date;
  char* max_taken_date;
  char* license;
  char* sort;
  char* privacy_filter;
  char* bbox;
  int accuracy;
  int safe_search;
  int content_type;
  char* machine_tags;
  char* machine_tag_mode;
  char* group_id;
  char* extras;
  int per_page;
  int page;
} flickcurl_search_params;
  

/**
 * flickcurl_photoset:
 * @id: photoset ID
 * @primary: primary photo ID
 * @secret: secret
 * @server: server
 * @farm: farm
 * @photos_count: count of photos in set
 * @title: title of photoset
 * @description: description of photoset (may be NULL)
 *
 * A photoset.
 *
 */
typedef struct {
  char *id;
  char *primary;
  char *secret;
  int server;
  int farm;
  int photos_count;
  char* title;
  char *description;
} flickcurl_photoset;


/**
 * flickcurl_size:
 * @label: label
 * @width: width in pixels
 * @height: height in pixels
 * @source: raw image source URL
 * @url: url of photo page
 *
 * A photo at a size.
 *
 */
typedef struct {
  char *label;
  int width;
  int height;
  char *source;
  char *url;
} flickcurl_size;


/**
 * flickcurl_ticket:
 * @id: ticket ID
 * @photoID: photoID
 * @complete: complete flag
 * @invalid: invalid flag
 *
 * An aysnchronous photo upload ticket.
 *
 */
typedef struct {
  int id;
  int photoid;
  int complete;
  int invalid;
} flickcurl_ticket;


/**
 * flickcurl_user_upload_status:
 * @username: user name
 * @bandwidth_maxbytes: max bytes
 * @bandwidth_maxkb: max kbytes
 * @bandwidth_usedbytes: used bytes
 * @bandwidth_usedkb: used kbytes
 * @bandwidth_remainingbytes: remaining bytes
 * @bandwidth_remainingkb: remaining kbytes
 * @filesize_maxbytes: max file size in bytes
 * @filesize_maxkb: max file size in kbytes
 * @sets_created: number of sets created
 * @sets_remaining: remaining sets: 0, 1, 2, 3 or "lots"
 *
 * A user's upload status
 *
 */
typedef struct {
  char* username;
  int bandwidth_maxbytes;
  int bandwidth_maxkb;
  int bandwidth_usedbytes;
  int bandwidth_usedkb;
  int bandwidth_remainingbytes;
  int bandwidth_remainingkb;

  int filesize_maxbytes;
  int filesize_maxkb;

  int sets_created;
  char* sets_remaining;
} flickcurl_user_upload_status;


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
void flickcurl_free_photos(flickcurl_photo** photos);
void flickcurl_free_photoset(flickcurl_photoset *photoset);
void flickcurl_free_photosets(flickcurl_photoset **photosets_object);
/* void flickcurl_free_license(flickcurl_person *license); */
void flickcurl_free_person(flickcurl_person *person);
void flickcurl_free_persons(flickcurl_person** persons);
void flickcurl_free_context(flickcurl_context *context);
void flickcurl_free_contexts(flickcurl_context** contexts);
void flickcurl_free_perms(flickcurl_perms *perms);
void flickcurl_free_location(flickcurl_location *location);
void flickcurl_free_exif(flickcurl_exif *exif);
void flickcurl_free_exifs(flickcurl_exif **exifs_object);
void flickcurl_free_ticket(flickcurl_ticket *ticket);
void flickcurl_free_tickets(flickcurl_ticket **tickets_object);
void flickcurl_free_user_upload_status(flickcurl_user_upload_status *u);

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
void flickcurl_free_category(flickcurl_category *category);
void flickcurl_free_categories(flickcurl_category **categories_object);
flickcurl_category* flickcurl_groups_browse(flickcurl* fc, int cat_id);
flickcurl_group* flickcurl_groups_getInfo(flickcurl* fc, const char* group_id, const char* lang);
flickcurl_group** flickcurl_groups_search(flickcurl* fc, const char* text, int per_page, int page);

/* flickr.groups.pools */
int flickcurl_groups_pools_add(flickcurl* fc, const char* photo_id, const char* group_id);
flickcurl_context** flickcurl_groups_pools_getContext(flickcurl* fc, const char* photo_id, const char* group_id);
flickcurl_group** flickcurl_groups_pools_getGroups(flickcurl* fc, int page, int per_page);
flickcurl_photo** flickcurl_groups_pools_getPhotos(flickcurl* fc, const char* group_id, const char* tags, const char* user_id, const char* extras, int per_page, int page);
int flickcurl_groups_pools_remove(flickcurl* fc, const char* photo_id, const char* group_id);
void flickcurl_free_group(flickcurl_group *group);
void flickcurl_free_groups(flickcurl_group **groups_object);

/* flickr.interestingness */
flickcurl_photo** flickcurl_interestingness_getList(flickcurl* fc, const char* date, const char* extras, int per_page, int page);

/* flickr.photo.getSizes */
void flickcurl_free_size(flickcurl_size *size);
void flickcurl_free_sizes(flickcurl_size **sizes_object);

/* flickr.people */
char* flickcurl_people_findByEmail(flickcurl* fc, const char* email);
char* flickcurl_people_findByUsername(flickcurl* fc, const char* username);
flickcurl_person* flickcurl_people_getInfo(flickcurl* fc, const char* user_id);
flickcurl_group** flickcurl_people_getPublicGroups(flickcurl* fc, const char* user_id);
flickcurl_photo** flickcurl_people_getPublicPhotos(flickcurl* fc, const char* user_id,  const char* extras, int per_page, int page);
flickcurl_user_upload_status* flickcurl_people_getUploadStatus(flickcurl* fc);

/* flickr.photos */
int flickcurl_photos_addTags(flickcurl* fc, const char* photo_id, const char* tags);
int flickcurl_photos_delete(flickcurl* fc, const char* photo_id);
flickcurl_context** flickcurl_photos_getAllContexts(flickcurl* fc, const char* photo_id);
flickcurl_photo** flickcurl_photos_getContactsPhotos(flickcurl* fc, int contact_count, int just_friends, int single_photo, int include_self, const char* extras);
flickcurl_photo** flickcurl_photos_getContactsPublicPhotos(flickcurl* fc, const char* user_id, int count, int just_friends,  int single_photo, int include_self, const char* extras);
flickcurl_context** flickcurl_photos_getContext(flickcurl* fc, const char* photo_id);
int** flickcurl_photos_getCounts(flickcurl* fc, const char** dates_array, const char** taken_dates_array);
flickcurl_exif** flickcurl_photos_getExif(flickcurl* fc, const char* photo_id, const char* secret);
flickcurl_person** flickcurl_photos_getFavorites(flickcurl* fc, const char* photo_id, int page, int per_page);
flickcurl_photo* flickcurl_photos_getInfo(flickcurl *fc, const char* photo_id);
flickcurl_photo** flickcurl_photos_getNotInSet(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
flickcurl_perms* flickcurl_photos_getPerms(flickcurl* fc, const char* photo_id);
flickcurl_photo** flickcurl_photos_getRecent(flickcurl* fc, const char* extras, int per_page, int page);
flickcurl_size** flickcurl_photos_getSizes(flickcurl* fc, const char* photo_id);
flickcurl_photo** flickcurl_photos_getUntagged(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
flickcurl_photo** flickcurl_photos_getWithGeoData(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
flickcurl_photo** flickcurl_photos_getWithoutGeoData(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
flickcurl_photo** flickcurl_photos_recentlyUpdated(flickcurl* fc, int min_date, const char* extras, int per_page, int page);
int flickcurl_photos_removeTag(flickcurl* fc, const char* tag_id);
flickcurl_photo** flickcurl_photos_search(flickcurl* fc, flickcurl_search_params* params);
int flickcurl_photos_setContentType(flickcurl* fc, const char* photo_id, int content_type);
int flickcurl_photos_setDates(flickcurl* fc, const char* photo_id, int date_posted, int date_taken, int date_taken_granularity);
int flickcurl_photos_setMeta(flickcurl* fc, const char* photo_id, const char* title, const char* description);
int flickcurl_photos_setPerms(flickcurl* fc, const char* photo_id, flickcurl_perms* perms);
int flickcurl_photos_setSafetyLevel(flickcurl* fc, const char* photo_id, int safety_level, int hidden);
int flickcurl_photos_setTags(flickcurl* fc, const char* photo_id, const char* tags);

/* flickr.contacts */
void flickcurl_free_contact(flickcurl_contact *contact_object);
void flickcurl_free_contacts(flickcurl_contact **contacts_object);
flickcurl_contact** flickcurl_contacts_getList(flickcurl* fc, const char* filter, int page, int per_page);
flickcurl_contact** flickcurl_contacts_getPublicList(flickcurl* fc, const char* user_id, int page, int per_page);

/* flickr.photos.comments */
void flickcurl_free_comment(flickcurl_comment *comment_object);
void flickcurl_free_comments(flickcurl_comment **comments_object);
char* flickcurl_photos_comments_addComment(flickcurl* fc, const char* photo_id, const char* comment_text);
int flickcurl_photos_comments_deleteComment(flickcurl* fc, const char* comment_id);
int flickcurl_photos_comments_editComment(flickcurl* fc, const char* comment_id, const char* comment_text);
flickcurl_comment** flickcurl_photos_comments_getList(flickcurl* fc, const char* photo_id);

/* flickr.photos.geo */
flickcurl_location* flickcurl_photos_geo_getLocation(flickcurl* fc, const char* photo_id);
flickcurl_perms* flickcurl_photos_geo_getPerms(flickcurl* fc, const char* photo_id);
int flickcurl_photos_geo_removeLocation(flickcurl* fc, const char* photo_id);
int flickcurl_photos_geo_setLocation(flickcurl* fc, const char* photo_id, flickcurl_location* location);
int flickcurl_photos_geo_setPerms(flickcurl* fc, const char* photo_id, flickcurl_perms* perms);
const char* flickcurl_get_location_accuracy_label(int accuracy);

/* flickr.photos.licenses */
flickcurl_license** flickcurl_photos_licenses_getInfo(flickcurl *fc);
flickcurl_license* flickcurl_photos_licenses_getInfo_by_id(flickcurl *fc, int id);
int flickcurl_photos_licenses_setLicense(flickcurl* fc, const char* photo_id, int license_id);

/* flickr.photos.notes */
char* flickcurl_photos_notes_add(flickcurl* fc, const char* photo_id, int note_x, int note_y, int note_w, int note_h, const char* note_text);
int flickcurl_photos_notes_delete(flickcurl* fc, const char* note_id);
int flickcurl_photos_notes_edit(flickcurl* fc, const char* note_id, int note_x, int note_y, int note_w, int note_h, const char* note_text);

/* flickr.photos.upload */
flickcurl_ticket** flickcurl_photos_upload_checkTickets(flickcurl* fc, const char** tickets_ids);

/* flickr.photos.transform */
int flickcurl_photos_transform_rotate(flickcurl* fc, const char* photo_id, int degrees);

/* flickr.photosets */
int flickcurl_photosets_addPhoto(flickcurl* fc, const char* photoset_id, const char* photo_id);
char* flickcurl_photosets_create(flickcurl* fc, const char* title, const char* description, const char* primary_photo_id, char** photoset_url_p);
int flickcurl_photosets_delete(flickcurl* fc, const char* photoset_id);
int flickcurl_photosets_editMeta(flickcurl* fc, const char* photoset_id, const char* title, const char* description);
int flickcurl_photosets_editPhotos(flickcurl* fc, const char* photoset_id, const char* primary_photo_id, const char** photo_ids_array);
flickcurl_context** flickcurl_photosets_getContext(flickcurl* fc, const char* photo_id, const char* photoset_id);
flickcurl_photoset* flickcurl_photosets_getInfo(flickcurl* fc, const char* photoset_id);
flickcurl_photoset** flickcurl_photosets_getList(flickcurl* fc, const char* user_id);
flickcurl_photo** flickcurl_photosets_getPhotos(flickcurl* fc, const char* photoset_id, const char* extras, int privacy_filter, int per_page, int page);
int flickcurl_photosets_orderSets(flickcurl* fc, const char** photoset_ids_array);
int flickcurl_photosets_removePhoto(flickcurl* fc, const char* photoset_id, const char* photo_id);

/* flickr.photosets.comments */
char* flickcurl_photosets_comments_addComment(flickcurl* fc, const char* photoset_id, const char* comment_text);
int flickcurl_photosets_comments_deleteComment(flickcurl* fc, const char* comment_id);
int flickcurl_photosets_comments_editComment(flickcurl* fc, const char* comment_id, const char* comment_text);
flickcurl_comment** flickcurl_photosets_comments_getList(flickcurl* fc, const char* photoset_id);
  
/* flickr.reflection */
void flickcurl_free_method(flickcurl_method *method);
char** flickcurl_reflection_getMethods(flickcurl* fc);
flickcurl_method* flickcurl_reflection_getMethodInfo(flickcurl* fc, const char* name);

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

/* Upload API */
FLICKCURL_DEPRECATED flickcurl_upload_status* flickcurl_photos_upload(flickcurl* fc, const char* photo_file, const char *title, const char *description, const char *tags, int is_public, int is_friend, int is_family);
flickcurl_upload_status* flickcurl_photos_upload_params(flickcurl* fc, flickcurl_upload_params* params);
flickcurl_upload_status* flickcurl_photos_replace(flickcurl* fc, const char* photo_file, const char *photo_id, int async);
void flickcurl_free_upload_status(flickcurl_upload_status* status);
FLICKCURL_DEPRECATED void flickcurl_upload_status_free(flickcurl_upload_status* status);

char* flickcurl_array_join(const char *array[], char delim);
char** flickcurl_array_split(const char *str, char delim);
void flickcurl_array_free(char *array[]);

#ifdef __cplusplus
}
#endif

#endif
