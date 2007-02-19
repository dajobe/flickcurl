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
 * Added in 0.8
 */
char*
flickcurl_people_findByEmail(flickcurl* fc, const char* email)
{
  const char * parameters[5][2];
  int count=0;
  char *nsid=NULL;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 

  if(!email)
    return NULL;
  
  parameters[count][0]  = "find_email";
  parameters[count++][1]= email;

  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

  if(flickcurl_prepare(fc, "flickr.people.findByEmail", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    nsid=flickcurl_xpath_eval(fc, xpathCtx,
                              (const xmlChar*)"/rsp/user/@nsid");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return nsid;
}


/*
 * flickr.people.findByUsername - get a user's NSID, given their username.
 * Added in 0.8
 */
char*
flickcurl_people_findByUsername(flickcurl* fc, const char* username)
{
  const char * parameters[5][2];
  int count=0;
  char *nsid=NULL;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 

  if(!username)
    return NULL;
  
  parameters[count][0]  = "username";
  parameters[count++][1]= username;

  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

  if(flickcurl_prepare(fc, "flickr.people.findByUsername", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    nsid=flickcurl_xpath_eval(fc, xpathCtx,
                              (const xmlChar*)"/rsp/user/@nsid");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return nsid;
}


static struct {
  const xmlChar* xpath;
  flickcurl_person_field field;
  flickcurl_field_value_type type;
} person_fields_table[PHOTO_FIELD_LAST + 3]={
  {
    (const xmlChar*)"/rsp/person/@nsid",
    PHOTO_FIELD_none,
    VALUE_TYPE_PERSON_ID,
  }
  ,
  {
    (const xmlChar*)"/rsp/person/@isadmin",
    PERSON_FIELD_isadmin,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/person/@ispro",
    PERSON_FIELD_ispro,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"/rsp/person/@iconserver",
    PERSON_FIELD_iconserver,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/person/@iconfarm",
    PERSON_FIELD_iconfarm,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"/rsp/person/username",
    PERSON_FIELD_username,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/person/realname",
    PERSON_FIELD_realname,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/person/mbox_sha1sum",
    PERSON_FIELD_mbox_sha1sum,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/person/location",
    PERSON_FIELD_location,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"/rsp/person/photosurl",
    PERSON_FIELD_photosurl,
    VALUE_TYPE_URI
  }
  ,
  {
    (const xmlChar*)"/rsp/person/profileurl",
    PERSON_FIELD_profileurl,
    VALUE_TYPE_URI
  }
  ,
  {
    (const xmlChar*)"/rsp/person/mobileurl",
    PERSON_FIELD_mobileurl,
    VALUE_TYPE_URI
  }
  ,
  {
    (const xmlChar*)"/rsp/person/photos/firstdate",
    PERSON_FIELD_photos_firstdate,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"/rsp/person/photos/firstdatetaken",
    PERSON_FIELD_photos_firstdatetaken,
    VALUE_TYPE_DATETIME
  }
  ,
  {
    (const xmlChar*)"/rsp/person/photos/count",
    PERSON_FIELD_photos_count,
    VALUE_TYPE_INTEGER
  }
  ,
  { 
    NULL,
    0,
    0
  }
};


/*
 * flickr.people.getInfo - get information about a person 
 * Added in 0.6
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
