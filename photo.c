/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photo.c - Flickcurl photo functions
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <flickcurl.h>
#include <flickcurl_internal.h>


static const char* flickcurl_photo_field_label[PHOTO_FIELD_LAST+1]={
  "(none)",
  "dateuploaded",
  "farm",
  "isfavorite",
  "license",
  "originalformat",
  "rotation",
  "server",
  "dates_lastupdate",
  "dates_posted",
  "dates_taken",
  "dates_takengranularity",
  "description",
  "editability_canaddmeta",
  "editability_cancomment",
  "geoperms_iscontact",
  "geoperms_isfamily",
  "geoperms_isfriend",
  "geoperms_ispublic",
  "location_accuracy",
  "location_latitude",
  "location_longitude",
  "owner_location",
  "owner_nsid",
  "owner_realname",
  "owner_username",
  "title",
  "visibility_isfamily",
  "visibility_isfriend",
  "visibility_ispublic",
  "secret",
  "originalsecret",
  "location_neighborhood",
  "location_locality",
  "location_region",
  "location_country"
};


const char*
flickcurl_get_photo_field_label(flickcurl_photo_field_type field)
{
  if(field <= PHOTO_FIELD_LAST)
    return flickcurl_photo_field_label[(int)field];
  return NULL;
}


void
flickcurl_free_photo(flickcurl_photo *photo)
{
  int i;
  for(i=0; i <= PHOTO_FIELD_LAST; i++) {
    if(photo->fields[i].string)
      free(photo->fields[i].string);
  }
  
  for(i=0; i < photo->tags_count; i++)
    flickcurl_free_tag(photo->tags[i]);
  free(photo->tags);

  if(photo->id)
    free(photo->id);
  
  if(photo->uri)
    free(photo->uri);
  
  free(photo);
}


/*
 * Get a photo's image source URIs
 * @c can be s,m,t,b for sizes, o for original, otherwise default
 * http://www.flickr.com/services/api/misc.urls.html
 */
char*
flickcurl_photo_as_source_uri(flickcurl_photo *photo, const char c)
{
  char buf[1024];
  char *result;
  size_t len;
  
  if(c == 'o') {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{o-secret}_o.(jpg|gif|png) */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s_o.%s", 
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_originalsecret].string,
            photo->fields[PHOTO_FIELD_originalformat].string);
  } else if (c == 'm' || c == 's' || c == 't' || c == 'b') {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}_[mstb].jpg */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s_%c.jpg",
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_secret].string,
            c);
  } else {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}.jpg */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s.jpg",
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_secret].string);
  }
  len=strlen(buf);
  result=(char*)malloc(len+1);
  strcpy(result, buf);
  return result;
}


static struct {
  const xmlChar* xpath;
  flickcurl_photo_field_type field;
  flickcurl_field_value_type type;
} photo_fields_table[PHOTO_FIELD_LAST + 3]={
  {
    (const xmlChar*)"/rsp/photo/@id",
    PHOTO_FIELD_none,
    VALUE_TYPE_PHOTO_ID,
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/urls/url[@type=\"photopage\"]",
    PHOTO_FIELD_none,
    VALUE_TYPE_PHOTO_URI
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@dateuploaded",
    PHOTO_FIELD_dateuploaded,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@farm",
    PHOTO_FIELD_farm,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@isfavorite",
    PHOTO_FIELD_isfavorite,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@license",
    PHOTO_FIELD_license,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@originalformat",
    PHOTO_FIELD_originalformat,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@rotation",
    PHOTO_FIELD_rotation,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@server",
    PHOTO_FIELD_server,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/dates/@lastupdate",
    PHOTO_FIELD_dates_lastupdate,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/dates/@posted",
    PHOTO_FIELD_dates_posted,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/dates/@taken",
    PHOTO_FIELD_dates_taken,
    VALUE_TYPE_DATETIME
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/dates/@takengranularity",
    PHOTO_FIELD_dates_takengranularity,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/description",
    PHOTO_FIELD_description,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/editability/@canaddmeta",
    PHOTO_FIELD_editability_canaddmeta,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/editability/@cancomment",
    PHOTO_FIELD_editability_cancomment,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/geoperms/@iscontact",
    PHOTO_FIELD_geoperms_iscontact,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/geoperms/@isfamily",
    PHOTO_FIELD_geoperms_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/geoperms/@isfriend",
    PHOTO_FIELD_geoperms_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/geoperms/@ispublic",
    PHOTO_FIELD_geoperms_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/location/@accuracy",
    PHOTO_FIELD_location_accuracy,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/location/@latitude",
    PHOTO_FIELD_location_latitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/location/@longitude",
    PHOTO_FIELD_location_longitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/location/neighborhood",
    PHOTO_FIELD_location_neighborhood,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/location/locality",
    PHOTO_FIELD_location_locality,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/location/region",
    PHOTO_FIELD_location_region,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/location/country",
    PHOTO_FIELD_location_country,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/owner/@location",
    PHOTO_FIELD_owner_location,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/owner/@nsid",
    PHOTO_FIELD_owner_nsid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/owner/@realname",
    PHOTO_FIELD_owner_realname,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/owner/@username",
    PHOTO_FIELD_owner_username,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/title",
    PHOTO_FIELD_title,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/visibility/@isfamily",
    PHOTO_FIELD_visibility_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/visibility/@isfriend",
    PHOTO_FIELD_visibility_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/visibility/@ispublic",
    PHOTO_FIELD_visibility_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@secret",
    PHOTO_FIELD_secret,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/photo/@originalsecret",
    PHOTO_FIELD_originalsecret,
    VALUE_TYPE_STRING
  }
  ,
  { 
    NULL,
    0,
    0
  }
};


flickcurl_photo*
flickcurl_build_photo(flickcurl* fc, xmlXPathContextPtr xpathCtx)
{
  int expri;
  flickcurl_photo* photo=NULL;
  
  photo=(flickcurl_photo*)calloc(sizeof(flickcurl_photo), 1);
  
  for(expri=0; photo_fields_table[expri].xpath; expri++) {
    char *string_value=flickcurl_xpath_eval(fc, xpathCtx, 
                                            photo_fields_table[expri].xpath);
    flickcurl_field_value_type datatype=photo_fields_table[expri].type;
    int int_value= -1;
    flickcurl_photo_field_type field=photo_fields_table[expri].field;
    time_t unix_time;
    
    if(!string_value) {
      photo->fields[field].string = NULL;
      photo->fields[field].integer= -1;
      photo->fields[field].type   = VALUE_TYPE_NONE;
      continue;
    }

    switch(datatype) {
      case VALUE_TYPE_PHOTO_ID:
        photo->id=string_value;
        string_value=NULL;
        datatype=VALUE_TYPE_NONE;
        break;

      case VALUE_TYPE_PHOTO_URI:
        photo->uri=string_value;
        string_value=NULL;
        datatype=VALUE_TYPE_NONE;
        break;

      case VALUE_TYPE_UNIXTIME:
      case VALUE_TYPE_DATETIME:
      
        if(datatype == VALUE_TYPE_UNIXTIME)
          unix_time=atoi(string_value);
        else
          unix_time=curl_getdate((const char*)string_value, NULL);
        
        if(unix_time >= 0) {
          char* new_value=flickcurl_unixtime_to_isotime(unix_time);
#if FLICKCURL_DEBUG > 1
          fprintf(stderr, "  date from: '%s' unix time %ld to '%s'\n",
                  value, (long)unix_time, new_value);
#endif
          free(string_value);
          string_value= new_value;
          int_value= unix_time;
          datatype=VALUE_TYPE_DATETIME;
        } else
          /* failed to convert, make it a string */
          datatype=VALUE_TYPE_STRING;
        break;
        
      case VALUE_TYPE_INTEGER:
      case VALUE_TYPE_BOOLEAN:
        int_value=atoi(string_value);
        break;
        
      case VALUE_TYPE_NONE:
      case VALUE_TYPE_STRING:
      case VALUE_TYPE_FLOAT:
      case VALUE_TYPE_URI:
        break;

      case VALUE_TYPE_PERSON_ID:
        abort();
    }

    photo->fields[field].string = string_value;
    photo->fields[field].integer= int_value;
    photo->fields[field].type   = datatype;

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "field %d with %s value: '%s' / %d\n",
            field, flickcurl_get_field_value_type_label(datatype), 
            string_value, int_value);
#endif
      
    if(fc->failed)
      goto tidy;
  }


  photo->tags=flickcurl_build_tags(fc, photo,
                                   xpathCtx, 
                                   (xmlChar*)"/rsp/photo/tags/tag", 
                                   &photo->tags_count);

  tidy:
  if(fc->failed)
    photo=NULL;

  return photo;
}


