/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * people-api.c - Flickr people API calls
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


/*
 * flickr.people.findByEmail - get a user's NSID, given their email address
 */

/*
 * flickr.people.findByUsername - get a user's NSID, given their username.
 */


/*
 * flickr.people.getInfo - get information about a person 
 */
flickcurl_person*
flickcurl_people_getInfo(flickcurl* fc, const char* user_id)
{
  const char * parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  int expri;
  xmlXPathObjectPtr xpathObj=NULL;
  flickcurl_person* person=NULL;
  
  if(!fc->auth_token) {
    flickcurl_error(fc, "No auth_token for method flickr.people.getInfo");
    return NULL;
  }


  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  /* does not require authentication */
  if(fc->auth_token) {
    parameters[count][0]  = "token";
    parameters[count++][1]= fc->auth_token;
  }

  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

  if(flickcurl_prepare(fc, "flickr.people.getInfo", parameters, count))
    goto tidy;

#ifdef OFFLINE
  flickcurl_debug_set_uri(fc, "file:people_getInfo.xml");
#endif

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  person=(flickcurl_person*)calloc(sizeof(flickcurl_person), 1);
  
  for(expri=0; person_fields_table[expri].xpath; expri++) {
    char *string_value=flickcurl_xpath_eval(fc, xpathCtx, 
                                            person_fields_table[expri].xpath);
    flickcurl_field_value_type datatype=person_fields_table[expri].type;
    int int_value= -1;
    flickcurl_person_field field=person_fields_table[expri].field;
    time_t unix_time;
    
    if(!string_value) {
      person->fields[field].string = NULL;
      person->fields[field].integer= -1;
      person->fields[field].type   = VALUE_TYPE_NONE;
      continue;
    }

    switch(datatype) {
      case VALUE_TYPE_PERSON_ID:
        person->nsid=string_value;
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

      case VALUE_TYPE_PHOTO_ID:
      case VALUE_TYPE_PHOTO_URI:
        abort();
    }

    person->fields[field].string = string_value;
    person->fields[field].integer= int_value;
    person->fields[field].type   = datatype;

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "field %d with %s value: '%s' / %d\n",
            field, flickcurl_field_value_type_label[datatype], 
            string_value, int_value);
#endif
      
    if(fc->failed)
      goto tidy;
  }


 tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    doc=NULL;

  return person;
}


/*
 * flickr.people.getPublicGroups - get the list of public groups a user is a member of.
 */

/*
 * flickr.people.getPublicPhotos - get a list of public photos for the given user.
 */

/*
 * flickr.people.getUploadStatus - get information for the calling user related to photo uploads.
 */
