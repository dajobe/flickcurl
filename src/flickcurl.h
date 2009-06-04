/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl.h - Flickcurl API
 *
 * Copyright (C) 2007-2009, David Beckett http://www.dajobe.org/
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


/**
 * FLICKCURL_API:
 *
 * Macro for wrapping API function call declarations.
 *
 */
#ifndef FLICKCURL_API
#  ifdef WIN32
#    ifdef __GNUC__
#      undef _declspec
#      define _declspec(x) __declspec(x)
#    endif
#    ifdef FLICKCURL_STATIC
#      define FLICKCURL_API
#    else
#      ifdef FLICKCURL_INTERNAL
#        define FLICKCURL_API _declspec(dllexport)
#      else
#        define FLICKCURL_API _declspec(dllimport)
#      endif
#    endif
#  else
#    define FLICKCURL_API
#  endif
#endif


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

/**
 * flickcurl_field_value_type:
 * @VALUE_TYPE_UNIXTIME: a unixtime
 * @VALUE_TYPE_BOOLEAN: boolean
 * @VALUE_TYPE_DATETIME: date time
 * @VALUE_TYPE_FLOAT: floating point number
 * @VALUE_TYPE_INTEGER: integer
 * @VALUE_TYPE_STRING: string
 * @VALUE_TYPE_URI: URI
 * @VALUE_TYPE_PERSON_ID: person ID
 * @VALUE_TYPE_PHOTO_ID: internal
 * @VALUE_TYPE_PHOTO_URI: internal
 * @VALUE_TYPE_MEDIA_TYPE: internal
 * @VALUE_TYPE_NONE: internal
 * @VALUE_TYPE_TAG_STRING: internal
 * @VALUE_TYPE_COLLECTION_ID: internal
 * @VALUE_TYPE_ICON_PHOTOS: internal 
 * @VALUE_TYPE_LAST: internal offset to last in enum list
 * 
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
  VALUE_TYPE_MEDIA_TYPE, /* internal */
  VALUE_TYPE_TAG_STRING, /* internal */
  VALUE_TYPE_COLLECTION_ID, /* internal */
  VALUE_TYPE_ICON_PHOTOS, /* internal */
  VALUE_TYPE_LAST = VALUE_TYPE_ICON_PHOTOS
} flickcurl_field_value_type;
  

/**
 * flickcurl_photo_field_type:
 * @PHOTO_FIELD_dateuploaded: date uploaded
 * @PHOTO_FIELD_farm: farm number
 * @PHOTO_FIELD_isfavorite: is favorite boolean
 * @PHOTO_FIELD_license: license
 * @PHOTO_FIELD_originalformat: original format
 * @PHOTO_FIELD_rotation: rotation
 * @PHOTO_FIELD_server: server
 * @PHOTO_FIELD_dates_lastupdate: last update date
 * @PHOTO_FIELD_dates_posted: posted date
 * @PHOTO_FIELD_dates_taken: taken date
 * @PHOTO_FIELD_dates_takengranularity: taken granularity
 * @PHOTO_FIELD_description: description
 * @PHOTO_FIELD_editability_canaddmeta: can add metadata boolean
 * @PHOTO_FIELD_editability_cancomment: can comment boolean
 * @PHOTO_FIELD_geoperms_iscontact: geo perms are for contacts
 * @PHOTO_FIELD_geoperms_isfamily: geo perms are for family
 * @PHOTO_FIELD_geoperms_isfriend: geo perms are for frind
 * @PHOTO_FIELD_geoperms_ispublic: geo perms are for public
 * @PHOTO_FIELD_location_accuracy: location accuracy
 * @PHOTO_FIELD_location_latitude: location latitude
 * @PHOTO_FIELD_location_longitude: location longitude
 * @PHOTO_FIELD_owner_location: owner location
 * @PHOTO_FIELD_owner_nsid: owner NSID
 * @PHOTO_FIELD_owner_realname: owner real name
 * @PHOTO_FIELD_owner_username: owner user name
 * @PHOTO_FIELD_title: title
 * @PHOTO_FIELD_visibility_isfamily: visibility is for family
 * @PHOTO_FIELD_visibility_isfriend: visibility is for friend
 * @PHOTO_FIELD_visibility_ispublic: visibility is for public
 * @PHOTO_FIELD_secret: photo secret
 * @PHOTO_FIELD_originalsecret: photo original secret
 * @PHOTO_FIELD_location_neighbourhood: location neighbourhood
 * @PHOTO_FIELD_location_neighborhood: deprecated
 * @PHOTO_FIELD_location_locality: location locality
 * @PHOTO_FIELD_location_county: location county
 * @PHOTO_FIELD_location_region: location region
 * @PHOTO_FIELD_location_country: location country
 * @PHOTO_FIELD_location_placeid: location place ID
 * @PHOTO_FIELD_neighbourhood_placeid: neighborhood place ID
 * @PHOTO_FIELD_neighborhood_placeid: dprecated
 * @PHOTO_FIELD_locality_placeid: locality place ID
 * @PHOTO_FIELD_county_placeid: county place ID
 * @PHOTO_FIELD_region_placeid: region place ID
 * @PHOTO_FIELD_country_placeid: country place ID
 * @PHOTO_FIELD_location_woeid: location WOE ID
 * @PHOTO_FIELD_neighbourhood_woeid: neighborhood WOE ID
 * @PHOTO_FIELD_neighborhood_woeid: deprecated
 * @PHOTO_FIELD_locality_woeid: locality WOE ID
 * @PHOTO_FIELD_county_woeid: county WOE ID
 * @PHOTO_FIELD_region_woeid: region WOE ID
 * @PHOTO_FIELD_country_woeid: country WOE ID
 * @PHOTO_FIELD_usage_candownload: can download
 * @PHOTO_FIELD_usage_canblog: can blog
 * @PHOTO_FIELD_usage_canprint: can print
 * @PHOTO_FIELD_owner_iconserver: server of owner's icon
 * @PHOTO_FIELD_owner_iconfarm: farm of owner's icon
 * @PHOTO_FIELD_original_width: original photo width
 * @PHOTO_FIELD_original_height: original photo height
 * @PHOTO_FIELD_views: number of photo views
 * @PHOTO_FIELD_none: internal
 * @PHOTO_FIELD_FIRST: internal offset to first in enum list
 * @PHOTO_FIELD_LAST: internal offset to last in enum list
 *
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
  PHOTO_FIELD_location_neighbourhood,
  PHOTO_FIELD_location_neighborhood = PHOTO_FIELD_location_neighbourhood,
  PHOTO_FIELD_location_locality,
  PHOTO_FIELD_location_county,
  PHOTO_FIELD_location_region,
  PHOTO_FIELD_location_country,
  PHOTO_FIELD_location_placeid,
  PHOTO_FIELD_neighbourhood_placeid,
  PHOTO_FIELD_neighborhood_placeid = PHOTO_FIELD_neighbourhood_placeid,
  PHOTO_FIELD_locality_placeid,
  PHOTO_FIELD_county_placeid,
  PHOTO_FIELD_region_placeid,
  PHOTO_FIELD_country_placeid,
  PHOTO_FIELD_location_woeid,
  PHOTO_FIELD_neighbourhood_woeid,
  PHOTO_FIELD_neighborhood_woeid  = PHOTO_FIELD_neighbourhood_woeid,
  PHOTO_FIELD_locality_woeid,
  PHOTO_FIELD_county_woeid,
  PHOTO_FIELD_region_woeid,
  PHOTO_FIELD_country_woeid,
  PHOTO_FIELD_usage_candownload,
  PHOTO_FIELD_usage_canblog,
  PHOTO_FIELD_usage_canprint,
  PHOTO_FIELD_owner_iconserver,
  PHOTO_FIELD_owner_iconfarm,
  PHOTO_FIELD_original_width,
  PHOTO_FIELD_original_height,
  PHOTO_FIELD_views,
  PHOTO_FIELD_FIRST = PHOTO_FIELD_dateuploaded,
  PHOTO_FIELD_LAST = PHOTO_FIELD_views
} flickcurl_photo_field_type;


/**
 * flickcurl:
 *
 * Flickcurl session object created by flickcurl_new() and destroyed
 * by flickcurl_free()
 */
typedef struct flickcurl_s flickcurl;
  

/* Forward structure references */
struct flickcurl_s;
struct flickcurl_photo_s;
struct flickcurl_shapedata_s;
  

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
 * @args_count: number of arguments, may be 0
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
 * flickcurl_activity_event:
 * @type: activty event type
 * @id: ID
 * @user: user ID
 * @username: user name
 * @value: event value
 * @date_added: date added
 *
 * Comment or photo activity event
 */
typedef struct {
  char *type; /* comment or note */
  char *id;
  char *user;
  char *username;
  char *value;
  int date_added;
} flickcurl_activity_event;


/**
 * FLICKCURL_MAX_ACTIVITY_EVENTS:
 *
 * Max number of activity events.
 */
#define FLICKCURL_MAX_ACTIVITY_EVENTS 20

/**
 * flickcurl_activity:
 * @type: activity type photoset or photo
 * @owner: owner NSID
 * @owner_name: owner name
 * @primary: primary
 * @id: photo id
 * @secret: photo secret
 * @server: photo server
 * @farm: photo farm
 * @comments_old: old comments count
 * @comments_new: new comments count
 * @notes_old: old notes count
 * @notes_new: new notes count
 * @views: views count
 * @comments: comments count
 * @photos: photos count
 * @faves: favourites count
 * @more: more boolean flag
 * @title: title of acitivty
 * @events: array of events associated with this actiivty
 *
 * Comments or photos item with activity
 */
typedef struct {
  char *type; /* photoset or photo */
  char *owner;
  char *owner_name;
  char *primary;

  /* photo info: ID/secret/server/farm */
  char *id;
  char *secret;
  int server;
  int farm;

  /* counts */
  int comments_old;
  int comments_new;
  int notes_old;
  int notes_new;
  int views;
  int comments;
  int photos;
  int faves;

  /* flags */
  int more;
  char* title;

  /* Array of events on this item */
  flickcurl_activity_event* events[FLICKCURL_MAX_ACTIVITY_EVENTS+1];
} flickcurl_activity;


/**
 * flickcurl_comment:
 * @id: comment ID
 * @author: author ID
 * @authorname: author name
 * @datecreate: date of creation
 * @permalink: permanent link of comment
 * @text: comment text
 *
 * A photo comment.
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
 * @is_contact: non-0 to make the photo visible to contacts when private
 * @is_friend: non-0 to make the photo visible to friends when private
 * @is_family: non-0 to make the photo visible to family when private
 * @perm_comment: who can add comments to the photo and it's notes. one of: 0 nobody,  1 friends & family, 2 contacts, 3 everybody
 * @perm_addmeta: who can add notes and tags to the photo. one of: 0 nobody / just the owner, 1 friends & family, 2 contacts, 3 everybody
 *
 * Permissions as used by flickcurl_photos_getPerms() and 
 * flickcurl_photos_setPerms() which use public, friend, family,
 * perm_comment and perm-addmeta.  flickcurl_photos_geo_setPerms() uses
 * public, contact, friend and family.
 *
 * A Photo permission.
 */
typedef struct {
  int is_public;
  int is_contact;
  int is_friend;
  int is_family;
  int perm_comment;
  int perm_addmeta;
} flickcurl_perms;


/**
 * flickcurl_tag_namespace:
 * @name: Name
 * @usage_count: Number of uses of this namespace
 * @predicates_count: Number of predicates for this namespace
 *
 * A machine tags namespace
 */
typedef struct {
  char *name;
  int usage_count;
  int predicates_count;
} flickcurl_tag_namespace;
  

/**
 * flickcurl_tag_predicate_value:
 * @usage_count: Number of uses of this predicate-value pair
 * @predicate: Predicate name or NULL
 * @used_in_namespace_count: number of namespaces this pair is used in
 * @value: Value or NULL
 *
 * A machine tag predicate-value pair
 */
typedef struct {
  int usage_count;
  char *predicate;
  int used_in_namespace_count;
  char *value;
} flickcurl_tag_predicate_value;
  

/**
 * flickcurl_institution:
 * @nsid: NSID
 * @date_launch: Date launched in unix timestamp format
 * @name: Institution name
 * @urls: Array of related urls.
 *
 * Flickr Commons institution
 *
 */
typedef struct {
  char *nsid;
  int date_launch;
  char *name;
  char **urls;
} flickcurl_institution;


/**
 * flickcurl_institution_url_type:
 * @FLICKCURL_INSTITUTION_URL_NONE: internal
 * @FLICKCURL_INSTITUTION_URL_SITE: site URL
 * @FLICKCURL_INSTITUTION_URL_LICENSE: license URL
 * @FLICKCURL_INSTITUTION_URL_FLICKR: flickr photos page URL
 * @FLICKCURL_INSTITUTION_URL_LAST: internal offset to last in enum list
 *
 * Institution URL type
*/
typedef enum {
  FLICKCURL_INSTITUTION_URL_NONE = 0,
  FLICKCURL_INSTITUTION_URL_SITE,
  FLICKCURL_INSTITUTION_URL_LICENSE,
  FLICKCURL_INSTITUTION_URL_FLICKR,
  FLICKCURL_INSTITUTION_URL_LAST = FLICKCURL_INSTITUTION_URL_FLICKR
} flickcurl_institution_url_type;


/**
 * flickcurl_location:
 * @latitude: The latitude from -90 to 90
 * @longitude: The longitude from -180 to 180
 * @accuracy: Recorded accuracy level of the location.
 *   World level is 1, Country is ~3, Region ~6, City ~11, Street
 *   ~16. Current range is 1-16. (<0 for unknown accuracy)
 *
 * A Location in the world with an optional accuracy
 */
typedef struct {
  double latitude;
  double longitude;
  int accuracy;
} flickcurl_location;
  

/**
 * flickcurl_place_type:
 * @FLICKCURL_PLACE_LOCATION: a general location
 * @FLICKCURL_PLACE_NEIGHBOURHOOD: neighborhood (narrowest place)
 * @FLICKCURL_PLACE_NEIGHBORHOOD: deprecated
 * @FLICKCURL_PLACE_LOCALITY: locality
 * @FLICKCURL_PLACE_COUNTY: county
 * @FLICKCURL_PLACE_REGION: region
 * @FLICKCURL_PLACE_COUNTRY: country
 * @FLICKCURL_PLACE_CONTINENT: continent (widest place) (Flickcurl 1.8)
 * @FLICKCURL_PLACE_LAST: internal offset to last in enum list
 *
 * Place type
*/
typedef enum {
  FLICKCURL_PLACE_LOCATION,
  FLICKCURL_PLACE_NEIGHBOURHOOD,
  FLICKCURL_PLACE_NEIGHBORHOOD = FLICKCURL_PLACE_NEIGHBOURHOOD,
  FLICKCURL_PLACE_LOCALITY,
  FLICKCURL_PLACE_COUNTY,
  FLICKCURL_PLACE_REGION,
  FLICKCURL_PLACE_COUNTRY,
  FLICKCURL_PLACE_CONTINENT,
  FLICKCURL_PLACE_LAST = FLICKCURL_PLACE_CONTINENT
} flickcurl_place_type;


/**
 * flickcurl_place_type_info:
 * @type: type enum ID
 * @id: web service call ID
 * @name: name
 *
 * Place type information
 */
typedef struct {
  flickcurl_place_type type;
  int id;
  char *name;
} flickcurl_place_type_info;


/**
 * flickcurl_place:
 * @names: Array of place names
 * @ids: Array of place IDs
 * @urls: Array of place urls.
 * @type: Location type of index 0 (the location) usually 
 *        FLICKCURL_PLACE_LOCATION but may be wider
 * @woe_ids: Array of WOE IDs
 * @location: location for this place
 * @count: count of photos (when used for flickcurl_places_placesForUser() )
 * @shapedata: DEPRECATED for @shape->data: XML string of &lt;shapedata&gt; element and content elements when present (or NULL)
 * @shapedata_length: DEPRECATED for @shape->data_length: size of @shapedate string
 * @shapefile_urls: DEPRECATED for @shape->file_urls: NULL-terminated array of pointers to shapefile URLs when present (or NULL)
 * @shapefile_urls_count: DEPRECATED for @shape->file_urls_count: number of entries in @shapefile_urls array
 * @shape: shapefile data (inline data and shapefile urls)
 * @timezone: timezone of location in 'zoneinfo' format such as “Europe/Paris”.
 *
 * A Place.
 *
 * Index 0 in the array is the location itself. flickcurl_get_place_type_label()
 * can give labels for the array indexes of type #flickcurl_place_type.
 *
 */
typedef struct {
  char* names[FLICKCURL_PLACE_LAST+1];
  char* ids[FLICKCURL_PLACE_LAST+1];
  char* urls[FLICKCURL_PLACE_LAST+1];
  flickcurl_place_type type;
  char* woe_ids[FLICKCURL_PLACE_LAST+1];
  flickcurl_location location;
  int count;

  /* DEPRECATED shapefile fields; now are pointers into @shape */
  char* shapedata;
  size_t shapedata_length;
  char** shapefile_urls;
  int shapefile_urls_count;

  struct flickcurl_shapedata_s* shape;
  char* timezone;
} flickcurl_place;
  

/**
 * flickcurl_shapedata:
 * @created: creation date as a UNIX timestamp
 * @alpha: Alpha value
 * @points: Number of points
 * @edges: Number of edges
 * @data: XML string of &lt;shapedata&gt; element and content elements when present (or NULL)
 * @data_length: size of @shapedate string
 * @file_urls: NULL-terminated array of pointers to shapefile URLs when present (or NULL)
 * @file_urls_count: number of entries in @shapefile_urls array
 * @is_donuthole: non-0 if shape IS a donut (a hole)
 * @has_donuthole: non-0 if shape HAS a donut inside it and it is worth calling places.getShapeHistory on it / flickcurl_places_getShapeHistory()
 *
 * Shape data for a place.
 *
 **/
typedef struct flickcurl_shapedata_s {
  int created;
  double alpha;
  int points;
  int edges;
  char* data;
  size_t data_length;
  char** file_urls;
  int file_urls_count;
  int is_donuthole;
  int has_donuthole;
} flickcurl_shapedata;


/**
 * flickcurl_tag: 
 * @photo: Associated photo object if any
 * @id: tag identifier
 * @author: author (may be NULL)
 * @authorname: author real name (may be NULL)
 * @raw: raw tag as user typed it (may be NULL, but if so @cooked must be not NULL)
 * @cooked: cooked tag (may be NULL, but if so @raw must not be NULL)
 * @machine_tag: boolean (non-0 true) if tag is a Machine Tag
 * @count: tag count in a histogram (or 0)
 *
 * A tag OR a posting of a tag about a photo by a user OR a tag in a histogram
 *
 * Most of these fields may be NULL, 0 for numbers
 * but not all.  Either @raw or @cooked MUST appear. 
 *
 * A Photo Tag.
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


/**
 * flickcurl_tag_cluster: 
 * @count: number of tags
 * @tags: tags in this cluster
 *
 * A cluster (set) of tag names
 */
typedef struct {
  int count;
  char** tags;
} flickcurl_tag_cluster;


/**
 * flickcurl_tag_clusters:
 * @count: number of tag clusters
 * @clusters: tag clusters
 *
 * A set of clusters of tag names
 */
typedef struct {
  int count;
  flickcurl_tag_cluster** clusters;
} flickcurl_tag_clusters;

/**
 * flickcurl_photo_field:
 * @string: string field value
 * @integer: integer field value
 * @type: field type
 *
 * Field of a photo structure
 */
typedef struct {
  char* string;
  flickcurl_photo_field_type integer;
  flickcurl_field_value_type type;
} flickcurl_photo_field;


/**
 * flickcurl_video: 
 * @ready: video is ready flag
 * @failed: video failed
 * @pending: video pending
 * @duration: video duration in seconds
 * @width: video width
 * @height: video height
 *
 * A video.
 *
 **/
typedef struct {
  int ready;
  int failed;
  int pending;
  int duration;
  int width;
  int height;
} flickcurl_video;


/**
 * flickcurl_photo: 
 * @id: photo/video ID
 * @uri: photo/video page URI
 * @tags: array of tags (may be NULL)
 * @tags_count: size of tags array
 * @fields: metadata fields
 * @place: place
 * @video: video (may be NULL)
 * @media_type: "photo" or "video"
 *
 * A photo or video.
 *
 */
typedef struct flickcurl_photo_s {
  char *id;
  char *uri;
  
  flickcurl_tag** tags;
  int tags_count;
  
  flickcurl_photo_field fields[PHOTO_FIELD_LAST + 1];

  flickcurl_place* place;

  flickcurl_video* video;

  char *media_type;
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
 * @iconserver: icon server
 * @realname: real name
 * @is_friend: is friend boolean
 * @is_family: is family boolean
 * @ignored: ignored
 * @uploaded: count of number of photos uploaded (for flickcurl_contacts_getListRecentlyUploaded() )
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
  int uploaded;
} flickcurl_contact;


/**
 * flickcurl_context_type:
 * @FLICKCURL_CONTEXT_SET: context is a set
 * @FLICKCURL_CONTEXT_POOL: context is a pool
 * @FLICKCURL_CONTEXT_PREV: context is a previous photo
 * @FLICKCURL_CONTEXT_NEXT: context is a next photo
 * @FLICKCURL_CONTEXT_NONE: internal
 * @FLICKCURL_CONTEXT_LAST: internal offset to last in enum list
 *
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


/**
 * flickcurl_context:
 * @type: Type of context
 * @id: ID
 * @secret: secret 
 * @server: server
 * @farm: farm
 * @title: use title
 * @url: url
 * @thumb: thumbnail
 *
 * Photo use context.
 */
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
 * flickcurl_blog:
 * @id: ID
 * @name: Group Name
 * @needs_password: needs password
 * @url: URL
 *
 * A blog.
 */
typedef struct {
  char* id;
  char* name;
  int needs_password;
  char* url;
} flickcurl_blog;


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


/**
 * flickcurl_set:
 * @id: set ID
 * @title: title
 * @description: description
 * @photos: photos
 * @photos_count: number of photos
 *
 * A photo set containing photos.
 */
struct flickcurl_set_s {
  char* id;
  char *title;
  char *description;
  struct flickcurl_photo_s** photos;
  int photos_count;
};
typedef struct flickcurl_set_s flickcurl_set;


/**
 * flickcurl_collection:
 * @id: ID
 * @child_count: children??
 * @date_created: date created
 * @iconlarge: large icon url
 * @iconsmall: small icon url
 * @server: server ID
 * @secret: secret
 * @title: title
 * @description: description
 * @photos: icon photos
 * @photos_count: number of icon photos
 * @collections: sub-collections
 * @collections_count: number of sub-collections
 * @sets: photo sets
 * @sets_count: number of photo sets
 *
 * A photo collection.  May contain collections OR sets but not both.
 */
struct flickcurl_collection_s {
  char* id;
  int child_count;
  int date_created;
  char *iconlarge;
  char *iconsmall;
  int server;
  char *secret;
  char *title;
  char *description;
  flickcurl_photo **photos;
  int photos_count;
  struct flickcurl_collection_s** collections;
  int collections_count;
  flickcurl_set** sets;
  int sets_count;
};
typedef struct flickcurl_collection_s flickcurl_collection;


/**
 * flickcurl_person_field_type:
 * @PERSON_FIELD_isadmin: is admin field boolean
 * @PERSON_FIELD_ispro:  is pro field boolean
 * @PERSON_FIELD_iconserver: icon server integer
 * @PERSON_FIELD_iconfarm: icon farm integer
 * @PERSON_FIELD_username: username
 * @PERSON_FIELD_realname: real name
 * @PERSON_FIELD_mbox_sha1sum: Email SHA1 sum
 * @PERSON_FIELD_location: location
 * @PERSON_FIELD_photosurl: photos URL
 * @PERSON_FIELD_profileurl: profile URL
 * @PERSON_FIELD_mobileurl: mobile URL
 * @PERSON_FIELD_photos_firstdate: photos first date
 * @PERSON_FIELD_photos_firstdatetaken: photos first date taken
 * @PERSON_FIELD_photos_count: photos count
 * @PERSON_FIELD_photos_views: photos views
 * @PERSON_FIELD_favedate: favorite date
 * @PERSON_FIELD_none: internal
 * @PERSON_FIELD_FIRST: internal offset to first in enum list
 * @PERSON_FIELD_LAST: internal offset to last in enum list
 *
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
  PERSON_FIELD_FIRST = PERSON_FIELD_isadmin,
  PERSON_FIELD_LAST = PERSON_FIELD_favedate
} flickcurl_person_field_type;


/**
 * flickcurl_person_field:
 * @string: string field value
 * @integer: integer field value
 * @type: field type
 *
 * Field of a person structure
 */
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
 * A user.
 */
typedef struct {
  char *nsid;

  flickcurl_person_field fields[PERSON_FIELD_LAST + 1];
} flickcurl_person;


/**
 * flickcurl_photos_list:
 * @format: requested content format or NULL if a list of photos was wanted.  On the result from API calls this is set to the requested feed format or "xml" if none was given.
 * @photos: list of photos if @format is NULL.  Also may be NULL on failure.
 * @photos_count: number of photos in @photos array if @format is NULL. Undefined on failure
 * @content: raw content if @format is not NULL.  Also may be NULL on failure.
 * @content_length: size of @content if @format is not NULL. Undefined on failure
 *
 * Photos List result.
 */
typedef struct {
  char *format;
  flickcurl_photo** photos;
  int photos_count;
  char* content;
  size_t content_length;
} flickcurl_photos_list;


/**
 * flickcurl_photos_list_params:
 * @version: structure version (currently 1)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: <code>license</code>, <code>date_upload</code>, <code>date_taken</code>, <code>owner_name</code>, <code>icon_server</code>, <code>original_format</code>, <code>last_update</code>, <code>geo</code>, <code>tags</code>, <code>machine_tags</code>. <code>'media</code> will return an extra media=VALUE for VALUE "photo" or "video".  API addition 2008-04-07. (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * @format: Feed format.  If given, the photos list result will return raw content.  This paramter is EXPERIMENTAL as annouced 2008-08-25 http://code.flickr.com/blog/2008/08/25/api-responses-as-feeds/  The current formats are  <code>feed-rss_100</code> for RSS 1.0, <code>feed-rss_200</code> for RSS 2.0, <code>feed-atom_10</code> for Atom 1.0, <code>feed-georss</code> for RSS 2.0 with GeoRSS and W3C Geo for geotagged photos, <code>feed-geoatom</code> for Atom 1.0 with GeoRSS and W3C Geo for geotagged photos, <code>feed-geordf</code> for RSS 1.0 with GeoRSS and W3C Geo for geotagged photos, <code>feed-kml</code> for KML 2.1, <code>feed-kml_nl</code> for KML 2.1 network link (or NULL)
 *
 * Photos List API parameters for multiple functions that return
 * a #flickcurl_photos_list
 *
 * Use flickcurl_get_extras_format_info() to enumerate the list of
 * known extra values and flickcurl_get_feed_format_info() to
 * enumerate the list of known format values.
 */
typedef struct {
  /* NOTE: Bump @version and update
   * flickcurl_photos_list_params_init() when adding fields 
   */
  int version; /* 1 */
  const char* format;
  const char* extras;
  int per_page;
  int page;
} flickcurl_photos_list_params;


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
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: <code>license</code>, <code>date_upload</code>, <code>date_taken</code>, <code>owner_name</code>, <code>icon_server</code>, <code>original_format</code>, <code>last_update</code>, <code>geo</code>, <code>tags</code>, <code>machine_tags</code>, <code>o_dims</code>, <code>views</code>, <code>media</code>. (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or NULL)
 * @place_id: A Flickr place id. (only used if bbox argument isn't present). Experimental.  Geo queries require some sort of limiting agent in order to prevent the database from crying. This is basically like the check against "parameterless searches" for queries without a geo component.   A tag, for instance, is considered a limiting agent as are user defined min_date_taken and min_date_upload parameters - If no limiting factor is passed we return only photos added in the last 12 hours (though we may extend the limit in the future) (or NULL)
 * @media: "photos" or "videos" (or NULL)
 * @has_geo: non-0 if a photo has been geotagged (or 0)
 * @lat: A valid latitude, in decimal format, for doing radial geo queries (or ignored if radius is 0.0)
 * @lon: A valid longitude, in decimal format, for doing radial geo queries (or ignored if radius is 0.0)
 * @radius: A valid radius used for geo queries, greater than zero and less than 20 miles (or 32 kilometers), for use with point-based geo queries. The default value is 5 (km) (or 0.0 for not used)
 * @radius_units: The unit of measure when doing radial geo queries. Valid options are "mi" (miles) and "km" (kilometers). The default is "km" (or NULL)
 * @contacts: (Experimental) Requires @user_id field be set and limits queries to photos beloing to that user's photos.  Valid arguments are 'all' or 'ff' for just friends and family.
 * @woe_id: A 32-bit identifier that uniquely represents spatial entities. (not used if bbox argument is present).  Same restrictions as @place_id (or <0)
 *
 * Search parameters for flickcurl_photos_search()
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
  char* place_id;
  char* media;
  int has_geo;
  double lat;
  double lon;
  double radius;
  char* radius_units;
  char* contacts;
  int woe_id;
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
 * @media: 'photo' or 'video'
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
  char* media;
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


/**
 * flickcurl_serializer:
 *
 * Serializer from Photo and metadata to RDF triples
 */
struct flickcurl_serializer_s;


/**
 * flickcurl_term_type:
 * @FLICKCURL_TERM_TYPE_RESOURCE: resource/URI term
 * @FLICKCURL_TERM_TYPE_BLANK: blank node/anonymous term
 * @FLICKCURL_TERM_TYPE_LITERAL: literal term
 *
 * Triple part types
 *
 * These are the same as raptor_identifier_type values.
 */
typedef enum {
  FLICKCURL_TERM_TYPE_RESOURCE = 1,
  FLICKCURL_TERM_TYPE_BLANK    = 2,
  FLICKCURL_TERM_TYPE_LITERAL  = 5
} flickcurl_term_type;


/**
 * flickcurl_serializer_factory:
 * @version: API version
 * @emit_namespace: (V1) emit namespace callback
 * @emit_triple: (V1) emit a triple
 * @emit_finish: (V1) finish emitting
 *
 * Triples serializer factory
 *
 * API version 1 is all that is supported.
 */
typedef struct {
  int version;
  void (*emit_namespace)(void* user_data, const char* prefix, size_t prefix_len, const char* uri, size_t uri_len);
  void (*emit_triple)(void* user_data,
                      const char* subject, int subject_type,
                      const char* predicate_nspace, const char* predicate_name,
                      const char *object, int object_type,
                      const char *datatype_uri);
  void (*emit_finish)(void* user_data);
} flickcurl_serializer_factory;

typedef struct flickcurl_serializer_s flickcurl_serializer;

FLICKCURL_API
flickcurl_serializer* flickcurl_new_serializer(flickcurl* fc, void* data, flickcurl_serializer_factory* factory);
FLICKCURL_API
void flickcurl_free_serializer(flickcurl_serializer* serializer);
FLICKCURL_API
int flickcurl_serialize_photo(flickcurl_serializer* fcs, flickcurl_photo* photo);


/**
 * flickcurl_member:
 * @nsid: NSID
 * @username: User name
 * @iconserver: icon server
 * @iconfarm: icon farm
 * @member_type: member type - 1: narwhal, 2: member, 3: moderator 4: admin
 *
 * Member in a group
 */
typedef struct {
  char *nsid;
  char *username;
  int iconserver;
  int iconfarm;
  int member_type;
} flickcurl_member;


/* callback handlers */

/**
 * flickcurl_message_handler
 * @user_data: user data pointer
 * @message: error message
 *
 * Flickcurl Message handler callback.
 */
typedef void (*flickcurl_message_handler)(void *user_data, const char *message);

/**
 * flickcurl_tag_handler
 * @user_data: user data pointer
 * @tag: tag
 *
 * Flickcurl Tag handler callback.
 */
typedef void (*flickcurl_tag_handler)(void *user_data, flickcurl_tag* tag);


/* library constants */
FLICKCURL_API
extern const char* const flickcurl_short_copyright_string;
FLICKCURL_API
extern const char* const flickcurl_copyright_string;
FLICKCURL_API
extern const char* const flickcurl_license_string;
FLICKCURL_API
extern const char* const flickcurl_home_url_string;
FLICKCURL_API
extern const char* const flickcurl_version_string;

FLICKCURL_API
extern const char* const flickcurl_flickr_service_uri;
FLICKCURL_API
extern const char* const flickcurl_flickr_upload_service_uri;
FLICKCURL_API
extern const char* const flickcurl_flickr_replace_service_uri;


/* library init - call once before creating anything */
FLICKCURL_API
int flickcurl_init(void);
/* library cleanup - call once before exit */
FLICKCURL_API
void flickcurl_finish(void);


/* flickcurl* object constructor */
FLICKCURL_API
flickcurl* flickcurl_new(void);

/* flickcurl* object destructor */
FLICKCURL_API
void flickcurl_free(flickcurl *fc);

/* flickcurl* object set methods */
FLICKCURL_API
void flickcurl_set_service_uri(flickcurl *fc, const char *uri);
FLICKCURL_API
void flickcurl_set_upload_service_uri(flickcurl *fc, const char *uri);
FLICKCURL_API
void flickcurl_set_replace_service_uri(flickcurl *fc, const char *uri);
FLICKCURL_API
void flickcurl_set_api_key(flickcurl* fc, const char *api_key);
FLICKCURL_API
void flickcurl_set_auth_token(flickcurl *fc, const char* auth_token);
FLICKCURL_API
void flickcurl_set_data(flickcurl *fc, void* data, size_t data_length);
FLICKCURL_API
void flickcurl_set_error_handler(flickcurl* fc, flickcurl_message_handler error_handler,  void *error_data);
FLICKCURL_API
void flickcurl_set_http_accept(flickcurl* fc, const char *value);
FLICKCURL_API
void flickcurl_set_proxy(flickcurl* fc, const char *proxy);
FLICKCURL_API
void flickcurl_set_request_delay(flickcurl *fc, long delay_msec);
FLICKCURL_API
void flickcurl_set_shared_secret(flickcurl* fc, const char *secret);
FLICKCURL_API
void flickcurl_set_sign(flickcurl *fc);
FLICKCURL_API
void flickcurl_set_tag_handler(flickcurl* fc,  flickcurl_tag_handler tag_handler, void *tag_data);
FLICKCURL_API
void flickcurl_set_user_agent(flickcurl* fc, const char *user_agent);
FLICKCURL_API
void flickcurl_set_write(flickcurl *fc, int is_write);
FLICKCURL_API
void flickcurl_set_xml_data(flickcurl *fc, xmlDocPtr doc);

/* flickcurl* object set methods */
FLICKCURL_API
const char* flickcurl_get_api_key(flickcurl *fc);
FLICKCURL_API
const char* flickcurl_get_shared_secret(flickcurl *fc);
FLICKCURL_API
const char* flickcurl_get_auth_token(flickcurl *fc);

/* other flickcurl class destructors */
FLICKCURL_API
void flickcurl_free_collection(flickcurl_collection *collection);
FLICKCURL_API
void flickcurl_free_collections(flickcurl_collection** collections);
FLICKCURL_API
void flickcurl_free_tag_namespace(flickcurl_tag_namespace *tag_nspace);
FLICKCURL_API
void flickcurl_free_tag_namespaces(flickcurl_tag_namespace** tag_nspaces);
FLICKCURL_API
void flickcurl_free_tag(flickcurl_tag *t);
FLICKCURL_API
void flickcurl_free_tag_clusters(flickcurl_tag_clusters *tcs);
FLICKCURL_API
void flickcurl_free_photo(flickcurl_photo *photo);
FLICKCURL_API
void flickcurl_free_photos(flickcurl_photo** photos);
FLICKCURL_API
void flickcurl_free_photos_list(flickcurl_photos_list* photos_list);
FLICKCURL_API
void flickcurl_free_photoset(flickcurl_photoset *photoset);
FLICKCURL_API
void flickcurl_free_photosets(flickcurl_photoset **photosets_object);
/* void flickcurl_free_license(flickcurl_person *license); */
FLICKCURL_API
void flickcurl_free_person(flickcurl_person *person);
FLICKCURL_API
void flickcurl_free_persons(flickcurl_person** persons);
FLICKCURL_API
void flickcurl_free_context(flickcurl_context *context);
FLICKCURL_API
void flickcurl_free_contexts(flickcurl_context** contexts);
FLICKCURL_API
void flickcurl_free_institution(flickcurl_institution *institution);
FLICKCURL_API
void flickcurl_free_institutions(flickcurl_institution **institutions_object);
FLICKCURL_API
void flickcurl_free_perms(flickcurl_perms *perms);
FLICKCURL_API
void flickcurl_free_location(flickcurl_location *location);
FLICKCURL_API
void flickcurl_free_exif(flickcurl_exif *exif);
FLICKCURL_API
void flickcurl_free_exifs(flickcurl_exif **exifs_object);
FLICKCURL_API
void flickcurl_free_ticket(flickcurl_ticket *ticket);
FLICKCURL_API
void flickcurl_free_tickets(flickcurl_ticket **tickets_object);
FLICKCURL_API
void flickcurl_free_user_upload_status(flickcurl_user_upload_status *u);
FLICKCURL_API
void flickcurl_free_place(flickcurl_place* place);
FLICKCURL_API
void flickcurl_free_places(flickcurl_place** places_object);
FLICKCURL_API
void flickcurl_free_place_type_infos(flickcurl_place_type_info **ptis_object);
FLICKCURL_API
void flickcurl_free_shape(flickcurl_shapedata *shape);
FLICKCURL_API
void flickcurl_free_shapes(flickcurl_shapedata **shapes_object);
FLICKCURL_API
void flickcurl_free_video(flickcurl_video *video);
FLICKCURL_API
void flickcurl_free_tag_predicate_value(flickcurl_tag_predicate_value* tag_pv);
FLICKCURL_API
void flickcurl_free_tag_predicate_values(flickcurl_tag_predicate_value **tag_pvs);

/* utility methods */
/* get an image URL for a photo in some size */
FLICKCURL_API
char* flickcurl_photo_as_source_uri(flickcurl_photo *photo, const char c);
/* get a page URL for a photo */
FLICKCURL_API
char* flickcurl_photo_as_page_uri(flickcurl_photo *photo);
/* get a owner icon URL for a photo */
FLICKCURL_API
char* flickcurl_user_icon_uri(int farm, int server, char *nsid);
FLICKCURL_API
char* flickcurl_photo_as_user_icon_uri(flickcurl_photo *photo);

/* get labels for various field/types */
FLICKCURL_API
const char* flickcurl_get_photo_field_label(flickcurl_photo_field_type field);
FLICKCURL_API
const char* flickcurl_get_person_field_label(flickcurl_person_field_type field);
FLICKCURL_API
const char* flickcurl_get_field_value_type_label(flickcurl_field_value_type datatype);
FLICKCURL_API
const char* flickcurl_get_context_type_field_label(flickcurl_context_type type);

FLICKCURL_API
const char* flickcurl_get_content_type_label(int content_type);
FLICKCURL_API
int flickcurl_get_content_type_from_string(const char* content_type_string);
FLICKCURL_API
const char* flickcurl_get_safety_level_label(int safety_level);
FLICKCURL_API
int flickcurl_get_safety_level_from_string(const char* safety_level_string);
FLICKCURL_API
int flickcurl_get_feed_format_info(int feed_format, const char** name_p, const char** label_p, const char** mime_type_p);
FLICKCURL_API
int flickcurl_get_extras_format_info(int extras_format, const char** name_p, const char** label_p);
FLICKCURL_API
int flickcurl_photos_list_params_init(flickcurl_photos_list_params* list_params);
FLICKCURL_API
int flickcurl_search_params_init(flickcurl_search_params* params);


/**
 * set_config_var_handler:
 * @userdata: user data pointer
 * @key: key
 * @value: value
 *
 * Handler to get variables returned from an 'INI' style configuration file
*/
typedef void (*set_config_var_handler)(void* userdata, const char* key, const char* value);
FLICKCURL_API
int read_ini_config(const char* filename, const char* application, void* user_data, set_config_var_handler handler);



/* Flickr API calls */

/* flickr.activity */
FLICKCURL_API
flickcurl_activity** flickcurl_activity_userComments(flickcurl* fc, int per_page, int page);
FLICKCURL_API
flickcurl_activity** flickcurl_activity_userPhotos(flickcurl* fc, const char* timeframe, int per_page, int page);
FLICKCURL_API
void flickcurl_free_activities(flickcurl_activity **activities_object);

/* flickr.auth */
FLICKCURL_API
char* flickcurl_auth_checkToken(flickcurl* fc, const char* token);
FLICKCURL_API
char* flickcurl_auth_getFrob(flickcurl* fc);
FLICKCURL_API
char* flickcurl_auth_getFullToken(flickcurl* fc, const char* frob);
FLICKCURL_API
char* flickcurl_auth_getToken(flickcurl* fc, const char* frob);

/* flickr.blogs */
FLICKCURL_API
flickcurl_blog** flickcurl_blogs_getList(flickcurl* fc);
FLICKCURL_API
int flickcurl_blogs_postPhoto(flickcurl* fc, const char* blog_id, const char* photo_id, const char* title, const char* description, const char* blog_password);
FLICKCURL_API
void flickcurl_free_blogs(flickcurl_blog **blogs_object);

/* flickr.collections */
FLICKCURL_API
flickcurl_collection* flickcurl_collections_getInfo(flickcurl* fc, const char* collection_id);
FLICKCURL_API
flickcurl_collection* flickcurl_collections_getTree(flickcurl* fc, const char* collection_id, const char* user_id);

/* flickr.commons */
FLICKCURL_API
flickcurl_institution** flickcurl_commons_getInstitutions(flickcurl* fc);
const char* flickcurl_get_institution_url_type_label(flickcurl_institution_url_type url_type);

/* flickr.favorites */
FLICKCURL_API
int flickcurl_favorites_add(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_photo** flickcurl_favorites_getList(flickcurl* fc, const char* user_id, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_favorites_getList_params(flickcurl* fc, const char* user_id, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_photo** flickcurl_favorites_getPublicList(flickcurl* fc, const char* user_id, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_favorites_getPublicList_params(flickcurl* fc, const char* user_id, flickcurl_photos_list_params* list_params);
FLICKCURL_API
int flickcurl_favorites_remove(flickcurl* fc, const char* photo_id);

/* flickr.groups */
FLICKCURL_API
void flickcurl_free_category(flickcurl_category *category);
FLICKCURL_API
void flickcurl_free_categories(flickcurl_category **categories_object);
FLICKCURL_API
flickcurl_category* flickcurl_groups_browse(flickcurl* fc, int cat_id);
FLICKCURL_API
flickcurl_group* flickcurl_groups_getInfo(flickcurl* fc, const char* group_id, const char* lang);
FLICKCURL_API
flickcurl_group** flickcurl_groups_search(flickcurl* fc, const char* text, int per_page, int page);

/* flickr.groups.members */
FLICKCURL_API
void flickcurl_free_member(flickcurl_member *member_object);
FLICKCURL_API
void flickcurl_free_members(flickcurl_member **members_object);
FLICKCURL_API
flickcurl_member** flickcurl_groups_members_getList(flickcurl* fc, const char* group_id, const char* membertypes, int per_page, int page);


/* flickr.groups.pools */
FLICKCURL_API
int flickcurl_groups_pools_add(flickcurl* fc, const char* photo_id, const char* group_id);
FLICKCURL_API
flickcurl_context** flickcurl_groups_pools_getContext(flickcurl* fc, const char* photo_id, const char* group_id);
FLICKCURL_API
flickcurl_group** flickcurl_groups_pools_getGroups(flickcurl* fc, int page, int per_page);
FLICKCURL_API
flickcurl_photo** flickcurl_groups_pools_getPhotos(flickcurl* fc, const char* group_id, const char* tags, const char* user_id, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_groups_pools_getPhotos_params(flickcurl* fc, const char* group_id, const char* tags, const char* user_id, flickcurl_photos_list_params* list_params);
FLICKCURL_API
int flickcurl_groups_pools_remove(flickcurl* fc, const char* photo_id, const char* group_id);
FLICKCURL_API
void flickcurl_free_group(flickcurl_group *group);
FLICKCURL_API
void flickcurl_free_groups(flickcurl_group **groups_object);

/* flickr.interestingness */
FLICKCURL_API
flickcurl_photo** flickcurl_interestingness_getList(flickcurl* fc, const char* date, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_interestingness_getList_params(flickcurl* fc, const char* date, flickcurl_photos_list_params* list_params);

/* flickr.machinetags */
FLICKCURL_API
flickcurl_tag_namespace** flickcurl_machinetags_getNamespaces(flickcurl* fc, const char* predicate, int per_page, int page);
FLICKCURL_API
flickcurl_tag_predicate_value** flickcurl_machinetags_getPairs(flickcurl* fc, const char *nspace, const char* predicate, int per_page, int page);
FLICKCURL_API
flickcurl_tag_predicate_value** flickcurl_machinetags_getPredicates(flickcurl* fc, const char *nspace, int per_page, int page);
FLICKCURL_API
flickcurl_tag_predicate_value** flickcurl_machinetags_getValues(flickcurl* fc, const char *nspace, const char* predicate, int per_page, int page);

/* flickr.panda */
FLICKCURL_API
char** flickcurl_panda_getList(flickcurl* fc);
FLICKCURL_API
flickcurl_photo** flickcurl_panda_getPhotos(flickcurl *fc, const char *panda_name);

/* flickr.photo.getSizes */
FLICKCURL_API
void flickcurl_free_size(flickcurl_size *size);
FLICKCURL_API
void flickcurl_free_sizes(flickcurl_size **sizes_object);

/* flickr.people */
FLICKCURL_API
char* flickcurl_people_findByEmail(flickcurl* fc, const char* email);
FLICKCURL_API
char* flickcurl_people_findByUsername(flickcurl* fc, const char* username);
FLICKCURL_API
flickcurl_person* flickcurl_people_getInfo(flickcurl* fc, const char* user_id);
FLICKCURL_API
flickcurl_group** flickcurl_people_getPublicGroups(flickcurl* fc, const char* user_id);
FLICKCURL_API
flickcurl_photo** flickcurl_people_getPublicPhotos(flickcurl* fc, const char* user_id,  const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_people_getPublicPhotos_params(flickcurl* fc, const char* user_id,  flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_user_upload_status* flickcurl_people_getUploadStatus(flickcurl* fc);

/* flickr.photos */
FLICKCURL_API
int flickcurl_photos_addTags(flickcurl* fc, const char* photo_id, const char* tags);
FLICKCURL_API
int flickcurl_photos_delete(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_context** flickcurl_photos_getAllContexts(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_getContactsPhotos(flickcurl* fc, int contact_count, int just_friends, int single_photo, int include_self, const char* extras);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_getContactsPhotos_params(flickcurl* fc, int contact_count, int just_friends, int single_photo, int include_self, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_getContactsPublicPhotos(flickcurl* fc, const char* user_id, int photo_count, int just_friends,  int single_photo, int include_self, const char* extras);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_getContactsPublicPhotos_params(flickcurl* fc, const char* user_id, int photo_count, int just_friends,  int single_photo, int include_self, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_context** flickcurl_photos_getContext(flickcurl* fc, const char* photo_id);
FLICKCURL_API
int** flickcurl_photos_getCounts(flickcurl* fc, const char** dates_array, const char** taken_dates_array);
FLICKCURL_API
flickcurl_exif** flickcurl_photos_getExif(flickcurl* fc, const char* photo_id, const char* secret);
FLICKCURL_API
flickcurl_person** flickcurl_photos_getFavorites(flickcurl* fc, const char* photo_id, int page, int per_page);
FLICKCURL_API
flickcurl_photo* flickcurl_photos_getInfo(flickcurl *fc, const char* photo_id);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_getNotInSet(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_getNotInSet_params(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_perms* flickcurl_photos_getPerms(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_getRecent(flickcurl* fc, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_getRecent_params(flickcurl* fc, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_size** flickcurl_photos_getSizes(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_getUntagged(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_getUntagged_params(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_getWithGeoData(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_getWithGeoData_params(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_getWithoutGeoData(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_getWithoutGeoData_params(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_recentlyUpdated(flickcurl* fc, int min_date, const char* extras, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_recentlyUpdated_params(flickcurl* fc, int min_date, flickcurl_photos_list_params* list_params);
FLICKCURL_API
int flickcurl_photos_removeTag(flickcurl* fc, const char* tag_id);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_search(flickcurl* fc, flickcurl_search_params* params);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_search_params(flickcurl* fc, flickcurl_search_params* params, flickcurl_photos_list_params* list_params);
FLICKCURL_API
int flickcurl_photos_setContentType(flickcurl* fc, const char* photo_id, int content_type);
FLICKCURL_API
int flickcurl_photos_setDates(flickcurl* fc, const char* photo_id, int date_posted, int date_taken, int date_taken_granularity);
FLICKCURL_API
int flickcurl_photos_setMeta(flickcurl* fc, const char* photo_id, const char* title, const char* description);
FLICKCURL_API
int flickcurl_photos_setPerms(flickcurl* fc, const char* photo_id, flickcurl_perms* perms);
FLICKCURL_API
int flickcurl_photos_setSafetyLevel(flickcurl* fc, const char* photo_id, int safety_level, int hidden);
FLICKCURL_API
int flickcurl_photos_setTags(flickcurl* fc, const char* photo_id, const char* tags);

/* flickr.places */
FLICKCURL_API
flickcurl_place** flickcurl_places_find(flickcurl* fc, const char* query);
FLICKCURL_API
flickcurl_place* flickcurl_places_findByLatLon(flickcurl* fc, double lat, double lon, int accuracy);
FLICKCURL_API FLICKCURL_DEPRECATED
flickcurl_place** flickcurl_places_getChildrenWithPhotosPublic(flickcurl* fc, const char* place_id, const char* woe_id);
FLICKCURL_API
flickcurl_place** flickcurl_places_getChildrenWithPhotosPublic2(flickcurl* fc, const char* place_id, int woe_id);
FLICKCURL_API FLICKCURL_DEPRECATED
flickcurl_place* flickcurl_places_getInfo(flickcurl* fc, const char* place_id, const char* woe_id);
FLICKCURL_API
flickcurl_place* flickcurl_places_getInfo2(flickcurl* fc, const char* place_id, const int woe_id);
FLICKCURL_API
flickcurl_place* flickcurl_places_getInfoByUrl(flickcurl* fc, const char* url);
FLICKCURL_API
flickcurl_place* flickcurl_places_resolvePlaceId(flickcurl* fc, const char* place_id);
FLICKCURL_API
flickcurl_place* flickcurl_places_resolvePlaceURL(flickcurl* fc, const char* url);
FLICKCURL_API
const char* flickcurl_get_place_type_label(flickcurl_place_type place_type);
flickcurl_place_type flickcurl_get_place_type_by_label(const char* place_label);
FLICKCURL_API
flickcurl_place_type_info** flickcurl_places_getPlaceTypes(flickcurl* fc);
FLICKCURL_API
flickcurl_shapedata** flickcurl_places_getShapeHistory(flickcurl* fc, const char* place_id, int woe_id);
FLICKCURL_API
flickcurl_place** flickcurl_places_placesForBoundingBox(flickcurl* fc, flickcurl_place_type place_type, double minimum_longitude, double minimum_latitude, double maximum_longitude, double maximum_latitude);
FLICKCURL_API
flickcurl_place** flickcurl_places_placesForContacts(flickcurl* fc, flickcurl_place_type place_type, int woe_id, const char* place_id, int threshold, const char* contacts, int min_upload_date, int max_upload_date, int min_taken_date, int max_taken_date);
FLICKCURL_API
int flickcurl_places_placesForTags(flickcurl* fc, flickcurl_place_type place_type, int woe_id, const char* place_id, const char* threshold, const char* tags, const char* tag_mode, const char* machine_tags, const char* machine_tag_mode, const char* min_upload_date, const char* max_upload_date, const char* min_taken_date, const char* max_taken_date);
FLICKCURL_API
flickcurl_place** flickcurl_places_placesForUser(flickcurl* fc, flickcurl_place_type place_type, int woe_id, const char* place_id, int threshold);
FLICKCURL_API FLICKCURL_DEPRECATED
flickcurl_place** flickcurl_places_forUser(flickcurl* fc, flickcurl_place_type place_type, int woe_id, const char* place_id, int threshold);
FLICKCURL_API
flickcurl_tag** flickcurl_places_tagsForPlace(flickcurl* fc, int woe_id, const char* place_id, int min_upload_date, int max_upload_date, int min_taken_date, int max_taken_date);
int flickcurl_place_type_to_id(flickcurl_place_type place_type);
flickcurl_place_type flickcurl_place_id_to_type(int place_type_id);

/* flickr.contacts */
FLICKCURL_API
void flickcurl_free_contact(flickcurl_contact *contact_object);
FLICKCURL_API
void flickcurl_free_contacts(flickcurl_contact **contacts_object);
FLICKCURL_API
flickcurl_contact** flickcurl_contacts_getList(flickcurl* fc, const char* filter, int page, int per_page);
FLICKCURL_API
flickcurl_contact** flickcurl_contacts_getListRecentlyUploaded(flickcurl* fc, int date_lastupload, const char* filter);
FLICKCURL_API
flickcurl_contact** flickcurl_contacts_getPublicList(flickcurl* fc, const char* user_id, int page, int per_page);

/* flickr.photos.comments */
FLICKCURL_API
void flickcurl_free_comment(flickcurl_comment *comment_object);
FLICKCURL_API
void flickcurl_free_comments(flickcurl_comment **comments_object);
FLICKCURL_API
char* flickcurl_photos_comments_addComment(flickcurl* fc, const char* photo_id, const char* comment_text);
FLICKCURL_API
int flickcurl_photos_comments_deleteComment(flickcurl* fc, const char* comment_id);
FLICKCURL_API
int flickcurl_photos_comments_editComment(flickcurl* fc, const char* comment_id, const char* comment_text);
FLICKCURL_API
flickcurl_comment** flickcurl_photos_comments_getList(flickcurl* fc, const char* photo_id);

/* flickr.photos.geo */
FLICKCURL_API
int flickcurl_photos_geo_batchCorrectLocation(flickcurl* fc, flickcurl_location* location, const char* place_id, int woe_id);
FLICKCURL_API
int flickcurl_photos_geo_correctLocation(flickcurl* fc, const char* photo_id, const char* place_id, int woe_id);
FLICKCURL_API
flickcurl_location* flickcurl_photos_geo_getLocation(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_perms* flickcurl_photos_geo_getPerms(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photos_geo_photosForLocation_params(flickcurl* fc, flickcurl_location* location, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_photo** flickcurl_photos_geo_photosForLocation(flickcurl* fc, flickcurl_location* location, const char* extras, int per_page, int page);
FLICKCURL_API
int flickcurl_photos_geo_removeLocation(flickcurl* fc, const char* photo_id);
FLICKCURL_API
int flickcurl_photos_geo_setContext(flickcurl* fc, const char* photo_id, int context);
FLICKCURL_API
int flickcurl_photos_geo_setLocation(flickcurl* fc, const char* photo_id, flickcurl_location* location);
FLICKCURL_API
int flickcurl_photos_geo_setPerms(flickcurl* fc, const char* photo_id, flickcurl_perms* perms);
FLICKCURL_API
const char* flickcurl_get_location_accuracy_label(int accuracy);

/* flickr.photos.licenses */
FLICKCURL_API
flickcurl_license** flickcurl_photos_licenses_getInfo(flickcurl *fc);
FLICKCURL_API
flickcurl_license* flickcurl_photos_licenses_getInfo_by_id(flickcurl *fc, int id);
FLICKCURL_API
int flickcurl_photos_licenses_setLicense(flickcurl* fc, const char* photo_id, int license_id);

/* flickr.photos.notes */
FLICKCURL_API
char* flickcurl_photos_notes_add(flickcurl* fc, const char* photo_id, int note_x, int note_y, int note_w, int note_h, const char* note_text);
FLICKCURL_API
int flickcurl_photos_notes_delete(flickcurl* fc, const char* note_id);
FLICKCURL_API
int flickcurl_photos_notes_edit(flickcurl* fc, const char* note_id, int note_x, int note_y, int note_w, int note_h, const char* note_text);
FLICKCURL_API

/* flickr.photos.upload */
FLICKCURL_API
flickcurl_ticket** flickcurl_photos_upload_checkTickets(flickcurl* fc, const char** tickets_ids);

/* flickr.photos.transform */
FLICKCURL_API
int flickcurl_photos_transform_rotate(flickcurl* fc, const char* photo_id, int degrees);

/* flickr.photosets */
FLICKCURL_API
int flickcurl_photosets_addPhoto(flickcurl* fc, const char* photoset_id, const char* photo_id);
FLICKCURL_API
char* flickcurl_photosets_create(flickcurl* fc, const char* title, const char* description, const char* primary_photo_id, char** photoset_url_p);
FLICKCURL_API
int flickcurl_photosets_delete(flickcurl* fc, const char* photoset_id);
FLICKCURL_API
int flickcurl_photosets_editMeta(flickcurl* fc, const char* photoset_id, const char* title, const char* description);
FLICKCURL_API
int flickcurl_photosets_editPhotos(flickcurl* fc, const char* photoset_id, const char* primary_photo_id, const char** photo_ids_array);
FLICKCURL_API
flickcurl_context** flickcurl_photosets_getContext(flickcurl* fc, const char* photo_id, const char* photoset_id);
FLICKCURL_API
flickcurl_photoset* flickcurl_photosets_getInfo(flickcurl* fc, const char* photoset_id);
FLICKCURL_API
flickcurl_photoset** flickcurl_photosets_getList(flickcurl* fc, const char* user_id);
FLICKCURL_API
flickcurl_photo** flickcurl_photosets_getPhotos(flickcurl* fc, const char* photoset_id, const char* extras, int privacy_filter, int per_page, int page);
FLICKCURL_API
flickcurl_photos_list* flickcurl_photosets_getPhotos_params(flickcurl* fc, const char* photoset_id, int privacy_filter, flickcurl_photos_list_params* list_params);
FLICKCURL_API
int flickcurl_photosets_orderSets(flickcurl* fc, const char** photoset_ids_array);
FLICKCURL_API
int flickcurl_photosets_removePhoto(flickcurl* fc, const char* photoset_id, const char* photo_id);

/* flickr.photosets.comments */
FLICKCURL_API
char* flickcurl_photosets_comments_addComment(flickcurl* fc, const char* photoset_id, const char* comment_text);
FLICKCURL_API
int flickcurl_photosets_comments_deleteComment(flickcurl* fc, const char* comment_id);
FLICKCURL_API
int flickcurl_photosets_comments_editComment(flickcurl* fc, const char* comment_id, const char* comment_text);
FLICKCURL_API
flickcurl_comment** flickcurl_photosets_comments_getList(flickcurl* fc, const char* photoset_id);

/* flickr.prefs */
FLICKCURL_API
int flickcurl_prefs_getContentType(flickcurl* fc);
FLICKCURL_API
int flickcurl_prefs_getGeoPerms(flickcurl* fc);
FLICKCURL_API
int flickcurl_prefs_getHidden(flickcurl* fc);
FLICKCURL_API
int flickcurl_prefs_getPrivacy(flickcurl* fc);
FLICKCURL_API
int flickcurl_prefs_getSafetyLevel(flickcurl* fc);

/* flickr.reflection */
FLICKCURL_API
void flickcurl_free_method(flickcurl_method *method);
FLICKCURL_API
char** flickcurl_reflection_getMethods(flickcurl* fc);
FLICKCURL_API
flickcurl_method* flickcurl_reflection_getMethodInfo(flickcurl* fc, const char* name);

/* flickr.tag */
FLICKCURL_API
flickcurl_photos_list* flickcurl_tags_getClusterPhotos(flickcurl* fc, const char* tag, const char* cluster_id, flickcurl_photos_list_params* list_params);
FLICKCURL_API
flickcurl_tag_clusters* flickcurl_tags_getClusters(flickcurl* fc, const char* tag);
FLICKCURL_API
flickcurl_tag** flickcurl_tags_getHotList(flickcurl* fc, const char* period, int tag_count);
FLICKCURL_API
flickcurl_tag** flickcurl_tags_getListPhoto(flickcurl* fc, const char* photo_id);
FLICKCURL_API
flickcurl_tag** flickcurl_tags_getListUser(flickcurl* fc, const char* user_id);
FLICKCURL_API
flickcurl_tag** flickcurl_tags_getListUserPopular(flickcurl* fc, const char* user_id, int pop_count);
FLICKCURL_API
flickcurl_tag** flickcurl_tags_getListUserRaw(flickcurl* fc, const char* tag);
FLICKCURL_API
flickcurl_tag** flickcurl_tags_getRelated(flickcurl* fc, const char* tag);

/* flickr.test */
FLICKCURL_API
int flickcurl_test_echo(flickcurl* fc, const char* key, const char* value);
FLICKCURL_API
char* flickcurl_test_login(flickcurl* fc);
FLICKCURL_API
int flickcurl_test_null(flickcurl* fc);

/* flickr.urls */
FLICKCURL_API
char* flickcurl_urls_getGroup(flickcurl* fc, const char* group_id);
FLICKCURL_API
char* flickcurl_urls_getUserPhotos(flickcurl* fc, const char* user_id);
FLICKCURL_API
char* flickcurl_urls_getUserProfile(flickcurl* fc, const char* user_id);
FLICKCURL_API
char* flickcurl_urls_lookupGroup(flickcurl* fc, const char* url);
FLICKCURL_API
char* flickcurl_urls_lookupUser(flickcurl* fc, const char* url);

/* Upload API */
FLICKCURL_API
FLICKCURL_DEPRECATED flickcurl_upload_status* flickcurl_photos_upload(flickcurl* fc, const char* photo_file, const char *title, const char *description, const char *tags, int is_public, int is_friend, int is_family);
FLICKCURL_API
flickcurl_upload_status* flickcurl_photos_upload_params(flickcurl* fc, flickcurl_upload_params* params);
FLICKCURL_API
flickcurl_upload_status* flickcurl_photos_replace(flickcurl* fc, const char* photo_file, const char *photo_id, int async);
FLICKCURL_API
void flickcurl_free_upload_status(flickcurl_upload_status* status);
FLICKCURL_API
FLICKCURL_DEPRECATED void flickcurl_upload_status_free(flickcurl_upload_status* status);

FLICKCURL_API
char* flickcurl_array_join(const char *array[], char delim);
FLICKCURL_API
char** flickcurl_array_split(const char *str, char delim);
FLICKCURL_API
void flickcurl_array_free(char *array[]);

/* ignore these */
/**
 * FLICKCURL_DEPRECATED:
 *
 * deprecated
 */

/**
 * flickcurl_category_s:
 * @id: ignore
 * @name: ignore
 * @path: ignore
 * @count: ignore
 * @categories: ignore
 * @categories_count: ignore
 * @groups: ignore
 * @groups_count: ignore
 *
 * category_s
 */

/**
 * flickcurl_photo_s:
 *
 * photo_s
 */

/**
 * flickcurl_s:
 *
 * flickcurl_s
 */

/**
 * flickcurl_serializer_s:
 *
 * flickcurl_serializer_s
 */

/**
 * flickcurl_shapedata_s:
 *
 * flickcurl_shapedata_s
 */

#ifdef __cplusplus
}
#endif

#endif
