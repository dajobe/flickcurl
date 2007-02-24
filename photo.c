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
  "originalsecret"
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


