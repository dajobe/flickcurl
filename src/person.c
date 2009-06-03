/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * person.c - Flickr person support calls
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


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
  "photos_views",
  "favedate"
};


/**
 * flickcurl_get_person_field_label:
 * @field: field enum
 *
 * Get label for person field
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_person_field_label(flickcurl_person_field_type field)
{
  if(field <= PERSON_FIELD_LAST)
    return flickcurl_person_field_label[(int)field];
  return NULL;
}


/**
 * flickcurl_free_person:
 * @person: person object
 *
 * Destructor for person object
 */
void
flickcurl_free_person(flickcurl_person *person)
{
  int i;

  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(person, flickcurl_person);

  for(i=0; i <= PERSON_FIELD_LAST; i++) {
    if(person->fields[i].string)
      free(person->fields[i].string);
  }
  
  if(person->nsid)
    free(person->nsid);
  
  free(person);
}


/*
 * The XPaths here are relative, such as prefixed by /rsp/person
 */
static struct {
  const xmlChar* xpath;
  flickcurl_person_field_type field;
  flickcurl_field_value_type type;
} person_fields_table[PHOTO_FIELD_LAST + 4]={
  {
    (const xmlChar*)"./@nsid",
    PERSON_FIELD_none,
    VALUE_TYPE_PERSON_ID,
  }
  ,
  {
    (const xmlChar*)"./@isadmin",
    PERSON_FIELD_isadmin,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@ispro",
    PERSON_FIELD_ispro,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@iconserver",
    PERSON_FIELD_iconserver,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@iconfarm",
    PERSON_FIELD_iconfarm,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./username",
    PERSON_FIELD_username,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./realname",
    PERSON_FIELD_realname,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./mbox_sha1sum",
    PERSON_FIELD_mbox_sha1sum,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location",
    PERSON_FIELD_location,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./photosurl",
    PERSON_FIELD_photosurl,
    VALUE_TYPE_URI
  }
  ,
  {
    (const xmlChar*)"./profileurl",
    PERSON_FIELD_profileurl,
    VALUE_TYPE_URI
  }
  ,
  {
    (const xmlChar*)"./mobileurl",
    PERSON_FIELD_mobileurl,
    VALUE_TYPE_URI
  }
  ,
  {
    (const xmlChar*)"./photos/firstdate",
    PERSON_FIELD_photos_firstdate,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./photos/firstdatetaken",
    PERSON_FIELD_photos_firstdatetaken,
    VALUE_TYPE_DATETIME
  }
  ,
  {
    (const xmlChar*)"./photos/count",
    PERSON_FIELD_photos_count,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./photos/views",
    PERSON_FIELD_photos_views,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@username",
    PERSON_FIELD_username,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@favedate",
    PERSON_FIELD_favedate,
    VALUE_TYPE_DATETIME
  }
  ,
  { 
    NULL,
    (flickcurl_person_field_type)0,
    (flickcurl_field_value_type)0
  }
};



flickcurl_person**
flickcurl_build_persons(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                        const xmlChar* xpathExpr, int* person_count_p)
{
  flickcurl_person** persons=NULL;
  int nodes_count;
  int person_count;
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
  persons=(flickcurl_person**)calloc(sizeof(flickcurl_person*), nodes_count+1);

  for(i=0, person_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    flickcurl_person* person;
    int expri;
    xmlXPathContextPtr xpathNodeCtx=NULL;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    person=(flickcurl_person*)calloc(sizeof(flickcurl_person), 1);

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;

    for(expri=0; expri <= PERSON_FIELD_LAST; expri++) {
      if(person->fields[expri].string)
        free(person->fields[expri].string);
      person->fields[expri].string = NULL;
      person->fields[expri].integer= (flickcurl_person_field_type)-1;
      person->fields[expri].type   = VALUE_TYPE_NONE;
    }

    for(expri=0; person_fields_table[expri].xpath; expri++) {
      flickcurl_person_field_type field=person_fields_table[expri].field;
      flickcurl_field_value_type datatype=person_fields_table[expri].type;
      char *string_value;
      int int_value= -1;
      time_t unix_time;
      
      string_value=flickcurl_xpath_eval(fc, xpathNodeCtx,
                                        person_fields_table[expri].xpath);
      if(!string_value) {
        person->fields[field].string = NULL;
        person->fields[field].integer= (flickcurl_person_field_type)-1;
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
          
        case VALUE_TYPE_PHOTO_ID:
        case VALUE_TYPE_PHOTO_URI:
        case VALUE_TYPE_MEDIA_TYPE:
        case VALUE_TYPE_TAG_STRING:
        case VALUE_TYPE_COLLECTION_ID:
        case VALUE_TYPE_ICON_PHOTOS:
          abort();
      }
      
      person->fields[field].string = string_value;
      person->fields[field].integer= (flickcurl_person_field_type)int_value;
      person->fields[field].type   = datatype;
      
#if FLICKCURL_DEBUG > 1
      fprintf(stderr, "field %d with %s value: '%s' / %d\n",
              field, flickcurl_field_value_type_label[datatype], 
              string_value, int_value);
#endif
      
      if(fc->failed)
        goto tidy;
    }

    persons[person_count++]=person;
  } /* for persons */
  
  if(person_count_p)
    *person_count_p=person_count;

 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    persons=NULL;

  return persons;
}


flickcurl_person*
flickcurl_build_person(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* root_xpathExpr)
{
  flickcurl_person** persons;
  flickcurl_person* result=NULL;

  persons=flickcurl_build_persons(fc, xpathCtx, root_xpathExpr, NULL);
  if(persons) {
    result=persons[0];
    free(persons);
  }
  
  return result;
}


/**
 * flickcurl_free_persons:
 * @persons: person object array
 *
 * Destructor for array of person object
 */
void
flickcurl_free_persons(flickcurl_person** persons)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(persons, flickcurl_person_array);

  for(i=0; persons[i]; i++)
    flickcurl_free_person(persons[i]);
  free(persons);
}
