/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * collection.c - Flickcurl collection functions
 *
 * Copyright (C) 2009-2012, David Beckett http://www.dajobe.org/
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
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_free_collection:
 * @collection: collection
 *
 * Destructor collection
 */
void
flickcurl_free_collection(flickcurl_collection *collection)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(collection, flickcurl_collection);

  if(collection->collections) {
    int i;
  
    for(i = 0; collection->collections[i]; i++)
      flickcurl_free_collection(collection->collections[i]);
  }
  
  if(collection->photos)
    flickcurl_free_photos(collection->photos);
  
  if(collection->description)
    free(collection->description);
  
  if(collection->title)
    free(collection->title);
  
  if(collection->secret)
    free(collection->secret);
  
  if(collection->iconsmall)
    free(collection->iconsmall);
  
  if(collection->iconlarge)
    free(collection->iconlarge);
  
  if(collection->id)
    free(collection->id);
  
  free(collection);
}


typedef enum {
  COLLECTION_FIELD_id,
  COLLECTION_FIELD_child_count,
  COLLECTION_FIELD_date_created,
  COLLECTION_FIELD_iconlarge,
  COLLECTION_FIELD_iconsmall,
  COLLECTION_FIELD_server,
  COLLECTION_FIELD_secret,
  COLLECTION_FIELD_title,
  COLLECTION_FIELD_description,
  COLLECTION_FIELD_iconphotos
} flickcurl_collection_field_type;


/*
 * The XPaths here are relative, such as prefixed by /rsp/person
 */
static struct {
  const xmlChar* xpath;
  flickcurl_collection_field_type field;
  flickcurl_field_value_type type;
} collection_fields_table[PHOTO_FIELD_LAST + 4] = {
  {
    (const xmlChar*)"./@id",
    COLLECTION_FIELD_id,
    VALUE_TYPE_COLLECTION_ID,
  }
  ,
  {
    (const xmlChar*)"./@child_count",
    COLLECTION_FIELD_child_count,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@datecreate",
    COLLECTION_FIELD_date_created,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./@iconlarge",
    COLLECTION_FIELD_iconlarge,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@iconsmall",
    COLLECTION_FIELD_iconsmall,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@server",
    COLLECTION_FIELD_server,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@secret",
    COLLECTION_FIELD_secret,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./title",
    COLLECTION_FIELD_title,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./description",
    COLLECTION_FIELD_description,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./iconphotos/photo",
    COLLECTION_FIELD_iconphotos,
    VALUE_TYPE_ICON_PHOTOS
  }
  ,
  { 
    NULL,
    (flickcurl_collection_field_type)0,
    (flickcurl_field_value_type)0
  }
};


flickcurl_collection**
flickcurl_build_collections(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                            const xmlChar* xpathExpr, int* collection_count_p)
{
  flickcurl_collection** collections = NULL;
  int nodes_count;
  int collection_count;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  xmlChar full_xpath[512];
  size_t xpathExpr_len;
  int i;
  
  xpathExpr_len = strlen((const char*)xpathExpr);
  memcpy(full_xpath, xpathExpr, xpathExpr_len + 1);
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed = 1;
    goto tidy;
  }
  
  nodes = xpathObj->nodesetval;
  /* This is a max size - it can include nodes that are CDATA */
  nodes_count = xmlXPathNodeSetGetLength(nodes);
  collections = (flickcurl_collection**)calloc(sizeof(flickcurl_collection*), nodes_count+1);

  for(i = 0, collection_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    flickcurl_collection* collection;
    int expri;
    xmlXPathContextPtr xpathNodeCtx = NULL;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    collection = (flickcurl_collection*)calloc(sizeof(flickcurl_collection), 1);

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;

    for(expri = 0; collection_fields_table[expri].xpath; expri++) {
      flickcurl_collection_field_type field = collection_fields_table[expri].field;
      flickcurl_field_value_type datatype = collection_fields_table[expri].type;
      const xmlChar *field_xpath = collection_fields_table[expri].xpath;
      char *string_value;
      int int_value= -1;
      time_t unix_time;

      if(datatype == VALUE_TYPE_ICON_PHOTOS) {
        collection->photos = flickcurl_build_photos(fc, xpathNodeCtx,
                                                    field_xpath,
                                                    &collection->photos_count);
        continue;
      }
      
      string_value = flickcurl_xpath_eval(fc, xpathNodeCtx, field_xpath);
      if(!string_value)
        continue;
      
      switch(datatype) {
        case VALUE_TYPE_UNIXTIME:
        case VALUE_TYPE_DATETIME:
          
          if(datatype == VALUE_TYPE_UNIXTIME)
            unix_time = atoi(string_value);
          else
            unix_time = curl_getdate((const char*)string_value, NULL);
          
          if(unix_time >= 0) {
#if FLICKCURL_DEBUG > 1
            char* new_value = flickcurl_unixtime_to_isotime(unix_time);
            fprintf(stderr, "  date from: '%s' unix time %ld to '%s'\n",
                    value, (long)unix_time, new_value);
#endif
            int_value = (int)unix_time;
          } else {
            /* else failed to convert */
            int_value = -1;
          }
          break;
          
        case VALUE_TYPE_INTEGER:
        case VALUE_TYPE_BOOLEAN:
          int_value = atoi(string_value);
          break;
          
        case VALUE_TYPE_COLLECTION_ID:
        case VALUE_TYPE_NONE:
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_FLOAT:
        case VALUE_TYPE_URI:
          break;

        /* This case is handled above */
        case VALUE_TYPE_ICON_PHOTOS:
          abort();
          
        case VALUE_TYPE_PHOTO_ID:
        case VALUE_TYPE_PHOTO_URI:
        case VALUE_TYPE_MEDIA_TYPE:
        case VALUE_TYPE_TAG_STRING:
        case VALUE_TYPE_PERSON_ID:
          abort();
      }
      
      switch(field) {
        case COLLECTION_FIELD_id:
          collection->id = string_value; string_value = NULL;
          break;
      
        case COLLECTION_FIELD_child_count:
          collection->child_count = int_value;
          break;
          
        case COLLECTION_FIELD_date_created:
          collection->date_created = int_value;
          break;
          
        case COLLECTION_FIELD_iconlarge:
          collection->iconlarge = string_value; string_value = NULL;
          break;
          
        case COLLECTION_FIELD_iconsmall:
          collection->iconsmall = string_value; string_value = NULL;
          break;
          
        case COLLECTION_FIELD_server:
          collection->server = int_value;
          break;
          
        case COLLECTION_FIELD_secret:
          collection->secret = string_value; string_value = NULL;
          break;
          
        case COLLECTION_FIELD_title:
          collection->title = string_value; string_value = NULL;
          break;
          
        case COLLECTION_FIELD_description:
          collection->description = string_value; string_value = NULL;
          break;
          
        case COLLECTION_FIELD_iconphotos:
          fprintf(stderr, "Do not know how to handle iconphotos field yet\n");
          break;
      }
      
      if(string_value)
        free(string_value);

      if(fc->failed) {
        if(collection)
          flickcurl_free_collection(collection);
        goto tidy;
      }
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "Collection id %s  secret %s  server %d\n"
                    "  Title %s\n"
                    "  Description %s\n"
                    "  Large icon %s\n"
                    "  Small Icon %s\n",
            collection->id, collection->secret, collection->server,
            collection->title, collection->description,
            collection->iconlarge, collection->iconsmall);
#endif

    collections[collection_count++] = collection;
  } /* for collections */
  
  if(collection_count_p)
    *collection_count_p = collection_count;

 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed) {
    if(collections)
      flickcurl_free_collections(collections);
    collections = NULL;
  }

  return collections;
}


flickcurl_collection*
flickcurl_build_collection(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                           const xmlChar* root_xpathExpr)
{
  flickcurl_collection** collections;
  flickcurl_collection* result = NULL;

  collections = flickcurl_build_collections(fc, xpathCtx, root_xpathExpr, NULL);
  if(collections) {
    result = collections[0];
    free(collections);
  }
  
  return result;
}


/**
 * flickcurl_free_collections:
 * @collections: collection object array
 *
 * Destructor for array of collection object
 */
void
flickcurl_free_collections(flickcurl_collection** collections)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(collections, flickcurl_collection_array);

  for(i = 0; collections[i]; i++)
    flickcurl_free_collection(collections[i]);
  free(collections);
}
