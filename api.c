/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * api.c - Flickr API calls
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
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
flickcurl_get_photo_field_label(flickcurl_photo_field field)
{
  if(field <= PHOTO_FIELD_LAST)
    return flickcurl_photo_field_label[(int)field];
  return NULL;
}


static const char* flickcurl_field_value_type_label[VALUE_TYPE_LAST+1]={
  "(none)",
  "photo id",
  "photo URI",
  "unix time",
  "boolean",
  "dateTime",
  "float",
  "integer",
  "string",
  "uri"
};


const char*
flickcurl_get_field_value_type_label(flickcurl_field_value_type datatype)
{
  if(datatype <= VALUE_TYPE_LAST)
    return flickcurl_field_value_type_label[(int)datatype];
  return NULL;
}


static const char* flickcurl_person_field_label[PERSON_FIELD_LAST+1]={
  "(none)",
  "isadmin",
  "ispro",
  "iconserver",
  "iconfarm",
  "username",
  "realname",
  "mbox_sha1sum",
  "location",
  "photosurl",
  "profileurl",
  "mobileurl",
  "photos_firstdate",
  "photos_firstdatetaken",
  "photos_count",
  "photos_views"
};


const char*
flickcurl_get_person_field_label(flickcurl_person_field field)
{
  if(field <= PERSON_FIELD_LAST)
    return flickcurl_person_field_label[(int)field];
  return NULL;
}


/* This is the element name and the label - lazy */
const char* flickcurl_context_type_element[FLICKCURL_CONTEXT_LAST+2]={
  "---",
  "set",
  "pool",
  "prevphoto",
  "nextphoto",
  NULL
};


const char*
flickcurl_get_context_type_field_label(flickcurl_context_type type)
{
  if(type > FLICKCURL_CONTEXT_NONE && type <= FLICKCURL_CONTEXT_LAST)
    return flickcurl_context_type_element[(int)type];
  return NULL;
}
