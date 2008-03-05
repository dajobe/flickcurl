/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photo.c - Flickcurl photo functions
 *
 * Copyright (C) 2007-2008, David Beckett http://www.dajobe.org/
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
  "location_county",
  "location_region",
  "location_country",
  "location_placeid",
  "neighborhood_placeid",
  "locality_placeid",
  "county_placeid",
  "region_placeid",
  "country_placeid"
};


/**
 * flickcurl_get_photo_field_label:
 * @field: field enum
 *
 * Get label for photo field.
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_photo_field_label(flickcurl_photo_field_type field)
{
  if(field <= PHOTO_FIELD_LAST)
    return flickcurl_photo_field_label[(int)field];
  return NULL;
}


/**
 * flickcurl_free_photo:
 * @photo: photo object
 *
 * Destructor for photo object
 */
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
  
  if(photo->place)
    flickcurl_free_place(photo->place);
  
  free(photo);
}


/**
 * flickcurl_photo_as_source_uri:
 * @photo: photo object
 * @c: size s, m, t or b
 *
 * Get a photo's image source URIs
 *
 * @c can be s,m,t,b for sizes, o for original, otherwise default
 * http://www.flickr.com/services/api/misc.urls.html
 *
 * Return value: source URI or NULL on failure
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
} photo_fields_table[]={
  {
    (const xmlChar*)"./@id",
    PHOTO_FIELD_none,
    VALUE_TYPE_PHOTO_ID,
  }
  ,
  {
    (const xmlChar*)"./urls/url[@type=\"photopage\"]",
    PHOTO_FIELD_none,
    VALUE_TYPE_PHOTO_URI
  }
  ,
  {
    (const xmlChar*)"./@dateuploaded",
    PHOTO_FIELD_dateuploaded,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./@farm",
    PHOTO_FIELD_farm,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@isfavorite",
    PHOTO_FIELD_isfavorite,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@license",
    PHOTO_FIELD_license,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@originalformat",
    PHOTO_FIELD_originalformat,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@rotation",
    PHOTO_FIELD_rotation,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@server",
    PHOTO_FIELD_server,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./dates/@lastupdate",
    PHOTO_FIELD_dates_lastupdate,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./dates/@posted",
    PHOTO_FIELD_dates_posted,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./dates/@taken",
    PHOTO_FIELD_dates_taken,
    VALUE_TYPE_DATETIME
  }
  ,
  {
    (const xmlChar*)"./dates/@takengranularity",
    PHOTO_FIELD_dates_takengranularity,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./description",
    PHOTO_FIELD_description,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./editability/@canaddmeta",
    PHOTO_FIELD_editability_canaddmeta,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./editability/@cancomment",
    PHOTO_FIELD_editability_cancomment,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@iscontact",
    PHOTO_FIELD_geoperms_iscontact,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@isfamily",
    PHOTO_FIELD_geoperms_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@isfriend",
    PHOTO_FIELD_geoperms_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@ispublic",
    PHOTO_FIELD_geoperms_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./location/@accuracy",
    PHOTO_FIELD_location_accuracy,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./location/@latitude",
    PHOTO_FIELD_location_latitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"./location/@longitude",
    PHOTO_FIELD_location_longitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"./location/neighborhood",
    PHOTO_FIELD_location_neighborhood,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/locality",
    PHOTO_FIELD_location_locality,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighborhood",
    PHOTO_FIELD_location_neighborhood,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/region",
    PHOTO_FIELD_location_region,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/country",
    PHOTO_FIELD_location_country,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@location",
    PHOTO_FIELD_owner_location,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@nsid",
    PHOTO_FIELD_owner_nsid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@realname",
    PHOTO_FIELD_owner_realname,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@username",
    PHOTO_FIELD_owner_username,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./title",
    PHOTO_FIELD_title,
    VALUE_TYPE_STRING
  }
  ,
  /* title can also appear as an attribute in a photo summary */
  {
    (const xmlChar*)"./@title",
    PHOTO_FIELD_title,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./visibility/@isfamily",
    PHOTO_FIELD_visibility_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./visibility/@isfriend",
    PHOTO_FIELD_visibility_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./visibility/@ispublic",
    PHOTO_FIELD_visibility_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  /* these can also appear in an attribute in a photo summary */
  {
    (const xmlChar*)"./@isfamily",
    PHOTO_FIELD_visibility_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@isfriend",
    PHOTO_FIELD_visibility_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@ispublic",
    PHOTO_FIELD_visibility_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@secret",
    PHOTO_FIELD_secret,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@originalsecret",
    PHOTO_FIELD_originalsecret,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/@place_id",
    PHOTO_FIELD_location_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighborhood/@place_id",
    PHOTO_FIELD_neighborhood_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/locality/@place_id",
    PHOTO_FIELD_locality_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/region/@place_id",
    PHOTO_FIELD_region_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/county/@place_id",
    PHOTO_FIELD_county_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/region/@place_id",
    PHOTO_FIELD_region_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/country/@place_id",
    PHOTO_FIELD_country_placeid,
    VALUE_TYPE_STRING
  }
  ,
  { 
    NULL,
    (flickcurl_photo_field_type)0,
    (flickcurl_field_value_type)0
  }
};


flickcurl_photo**
flickcurl_build_photos(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* xpathExpr, int* photo_count_p)
{
  flickcurl_photo** photos=NULL;
  int nodes_count;
  int photo_count;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  xmlChar full_xpath[512];
  size_t xpathExpr_len;
  int i;
  
  xpathExpr_len=strlen((const char*)xpathExpr);
  strncpy((char*)full_xpath, (const char*)xpathExpr, xpathExpr_len+1);
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed=1;
    goto tidy;
  }
  
  nodes=xpathObj->nodesetval;
  /* This is a max size - it can include nodes that are CDATA */
  nodes_count=xmlXPathNodeSetGetLength(nodes);
  photos=(flickcurl_photo**)calloc(sizeof(flickcurl_photo*), nodes_count+1);

  for(i=0, photo_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    flickcurl_photo* photo;
    int expri;
    xmlXPathContextPtr xpathNodeCtx=NULL;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    photo=(flickcurl_photo*)calloc(sizeof(flickcurl_photo), 1);

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;
    
    for(expri=0; expri <= PHOTO_FIELD_LAST; expri++) {
      if(photo->fields[expri].string)
        free(photo->fields[expri].string);
      photo->fields[expri].string = NULL;
      photo->fields[expri].integer= (flickcurl_photo_field_type)-1;
      photo->fields[expri].type   = VALUE_TYPE_NONE;
    }

    for(expri=0; photo_fields_table[expri].xpath; expri++) {
      char *string_value;
      flickcurl_field_value_type datatype=photo_fields_table[expri].type;
      int int_value= -1;
      flickcurl_photo_field_type field=photo_fields_table[expri].field;
      time_t unix_time;

      string_value=flickcurl_xpath_eval(fc, xpathNodeCtx,
                                        photo_fields_table[expri].xpath);
      if(!string_value)
        continue;

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
            int_value= (int)unix_time;
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
      photo->fields[field].integer= (flickcurl_photo_field_type)int_value;
      photo->fields[field].type   = datatype;

  #if FLICKCURL_DEBUG > 1
      fprintf(stderr, "field %d with %s value: '%s' / %d\n",
              field, flickcurl_get_field_value_type_label(datatype), 
              string_value, int_value);
  #endif

      if(fc->failed)
        goto tidy;
    } /* end for */

    photo->tags=flickcurl_build_tags(fc, photo, xpathNodeCtx, 
                                     (const xmlChar*)"./tags/tag",
                                     &photo->tags_count);

    photo->place=flickcurl_build_place(fc, xpathNodeCtx,
                                       (const xmlChar*)"./location");

    if(xpathNodeCtx)
      xmlXPathFreeContext(xpathNodeCtx);

    photos[photo_count++]=photo;
  } /* for photos */
  
  if(photo_count_p)
    *photo_count_p=photo_count;

  tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  if(fc->failed)
    photos=NULL;

  return photos;
}


flickcurl_photo*
flickcurl_build_photo(flickcurl* fc, xmlXPathContextPtr xpathCtx)
{
  flickcurl_photo** photos;
  flickcurl_photo* result=NULL;

  photos=flickcurl_build_photos(fc, xpathCtx,
                                (const xmlChar*)"/rsp/photo", NULL);
  if(photos) {
    result=photos[0];
    free(photos);
  }
  
  return result;
}


/**
 * flickcurl_free_photos:
 * @photos: photo object array
 *
 * Destructor for array of photo objects
 */
void
flickcurl_free_photos(flickcurl_photo** photos)
{
  int i;
  
  for(i=0; photos[i]; i++)
    flickcurl_free_photo(photos[i]);
  free(photos);
}
