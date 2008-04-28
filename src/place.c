/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * place.c - Flickr place support calls
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
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


static const char* flickcurl_place_type_label[FLICKCURL_PLACE_LAST+1]={
  "location",
  "neighborhood",
  "locality",
  "county",
  "region",
  "country"
};


/**
 * flickcurl_get_place_type_label:
 * @place_type: place type
 *
 * Get label for a place type
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_place_type_label(flickcurl_place_type place_type)
{
  if(place_type <= FLICKCURL_PLACE_LAST)
    return flickcurl_place_type_label[(int)place_type];
  return NULL;
}


/**
 * flickcurl_get_place_type_by_label:
 * @place_label: place type
 *
 * Get a place type by label
 *
 * Return value: place type
 */
flickcurl_place_type
flickcurl_get_place_type_by_label(const char* place_label)
{
  int i;
  for(i=0; flickcurl_place_type_label[i]; i++) {
    if(!strcmp(flickcurl_place_type_label[i], place_label))
      return (flickcurl_place_type)i;
  }
  
  return FLICKCURL_PLACE_LOCATION;
}


/**
 * flickcurl_free_place:
 * @place: place object
 *
 * Destructor for place object
 */
void
flickcurl_free_place(flickcurl_place *place)
{
  int i;

  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(place, flickcurl_place);

  for(i=0; i <= FLICKCURL_PLACE_LAST; i++) {
    if(place->names[i])
      free(place->names[i]);
    if(place->ids[i])
      free(place->ids[i]);
    if(place->urls[i])
      free(place->urls[i]);
    if(place->woe_ids[i])
      free(place->woe_ids[i]);
  }
  
  free(place);
}


/**
 * flickcurl_free_places:
 * @places_object: place object array
 *
 * Destructor for array of place object
 */
void
flickcurl_free_places(flickcurl_place **places_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(places_object, flickcurl_place_array);

  for(i=0; places_object[i]; i++)
    flickcurl_free_place(places_object[i]);
  
  free(places_object);
}


/* flickcurl_place arrays */
/* place->names[x] */
#define PLACE_NAME 0
/* place->ids[x] */
#define PLACE_ID   1
/* place->urls[x] */
#define PLACE_URL  2
/* place->woe_ids[x] */
#define PLACE_WOE_ID  3

#define PLACE_TYPE 4

/*
 * The XPaths here are relative, such as prefixed by /rsp/place
 */
static struct {
  const xmlChar* xpath;
  flickcurl_place_type place_type;
  unsigned short place_array;
} place_fields_table[PHOTO_FIELD_LAST + 4]={
  {
    (const xmlChar*)"./@name",
    FLICKCURL_PLACE_LOCATION,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./@place_id",
    FLICKCURL_PLACE_LOCATION,
    PLACE_ID
  }
  ,
  {
    (const xmlChar*)"./@place_url",
    FLICKCURL_PLACE_LOCATION,
    PLACE_URL
  }
  ,
  {
    (const xmlChar*)"./@woeid",
    FLICKCURL_PLACE_LOCATION,
    PLACE_WOE_ID
  }
  ,
  {
    (const xmlChar*)"./locality/@place_id",
    FLICKCURL_PLACE_LOCALITY,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./locality/@woeid",
    FLICKCURL_PLACE_LOCALITY,
    PLACE_WOE_ID,
  }
  ,
  {
    (const xmlChar*)"./locality",
    FLICKCURL_PLACE_LOCALITY,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./county/@place_id",
    FLICKCURL_PLACE_COUNTY,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./county/@woeid",
    FLICKCURL_PLACE_COUNTY,
    PLACE_WOE_ID,
  }
  ,
  {
    (const xmlChar*)"./county",
    FLICKCURL_PLACE_COUNTY,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./region/@place_id",
    FLICKCURL_PLACE_REGION,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./region/@woeid",
    FLICKCURL_PLACE_REGION,
    PLACE_WOE_ID,
  }
  ,
  {
    (const xmlChar*)"./region",
    FLICKCURL_PLACE_REGION,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./country/@place_id",
    FLICKCURL_PLACE_COUNTRY,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./country/@woeid",
    FLICKCURL_PLACE_COUNTRY,
    PLACE_WOE_ID,
  }
  ,
  {
    (const xmlChar*)"./country",
    FLICKCURL_PLACE_COUNTRY,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./@place_type",
    (flickcurl_place_type)0,
    PLACE_TYPE,
  }
  ,
  { 
    NULL,
    (flickcurl_place_type)0,
    (unsigned short)0
  }
};



flickcurl_place**
flickcurl_build_places(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* xpathExpr, int* place_count_p)
{
  flickcurl_place** places=NULL;
  int nodes_count;
  int place_count;
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
  places=(flickcurl_place**)calloc(sizeof(flickcurl_place*), nodes_count+1);

  for(i=0, place_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    int expri;
    xmlXPathContextPtr xpathNodeCtx=NULL;
    flickcurl_place* place;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    place=(flickcurl_place*)calloc(sizeof(flickcurl_place), 1);
    place->type=FLICKCURL_PLACE_LOCATION;

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;

    for(expri=0; expri <= FLICKCURL_PLACE_LAST; expri++) {
      if(place->names[expri]) {
        free(place->names[expri]);
        place->names[expri]=NULL;
      }
      if(place->ids[expri]) {
        free(place->ids[expri]);
        place->ids[expri]=NULL;
      }
      if(place->urls[expri]) {
        free(place->urls[expri]);
        place->urls[expri]=NULL;
      }
    }

    for(expri=0; place_fields_table[expri].xpath; expri++) {
      flickcurl_place_type place_type=place_fields_table[expri].place_type;
      unsigned short place_array=place_fields_table[expri].place_array;
      char *value;
      
      value=flickcurl_xpath_eval(fc, xpathNodeCtx,
                                 place_fields_table[expri].xpath);
      if(!value)
        continue;

      if(place_array == PLACE_TYPE) {
        place->type=flickcurl_get_place_type_by_label(value);
        continue;
      }
      
      switch(place_array) {
        case PLACE_NAME:
          place->names[(int)place_type]=value;
          break;
          
        case PLACE_ID:
          place->ids[(int)place_type]=value;
          break;

        case PLACE_WOE_ID:
          place->woe_ids[(int)place_type]=value;
          break;

        case PLACE_URL:
          place->urls[(int)place_type]=value;
          break;
      }
      
#if FLICKCURL_DEBUG > 1
      fprintf(stderr, "field %d array #%d with value: '%s'\n",
              place_type, place_array, value);
#endif
      
      if(fc->failed)
        goto placestidy;
    } /* end for place fields */

   placestidy:
    if(xpathNodeCtx)
      xmlXPathFreeContext(xpathNodeCtx);

    places[place_count++]=place;
  } /* for places */
  
  if(place_count_p)
    *place_count_p=place_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    places=NULL;

  return places;
}


flickcurl_place*
flickcurl_build_place(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr)
{
  flickcurl_place** places;
  flickcurl_place* result=NULL;

  places=flickcurl_build_places(fc, xpathCtx, xpathExpr, NULL);

  if(places) {
    result=places[0];
    free(places);
  }
  
  return result;
}


