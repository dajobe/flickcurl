/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * place.c - Flickr place support calls
 *
 * Copyright (C) 2008, David Beckett http://purl.org/net/dajobe/
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


static const char* flickcurl_place_type_label[FLICKCURL_PLACE_LAST+1]={
  "location",
  "neighborhood",
  "locality",
  "county",
  "region",
  "country"
};


const char*
flickcurl_get_place_type_label(flickcurl_place_type place_type)
{
  if(place_type <= FLICKCURL_PLACE_LAST)
    return flickcurl_place_type_label[(int)place_type];
  return NULL;
}


void
flickcurl_free_place(flickcurl_place *place)
{
  int i;
  for(i=0; i <= FLICKCURL_PLACE_LAST; i++) {
    if(place->names[i])
      free(place->names[i]);
    if(place->ids[i])
      free(place->ids[i]);
    if(place->urls[i])
      free(place->urls[i]);
  }
  
  free(place);
}

/* flickcurl_place arrays */
/* place->names[x] */
#define PLACE_NAME 0
/* place->ids[x] */
#define PLACE_ID   1
/* place->urls[x] */
#define PLACE_URL  2

/*
 * The XPaths here are relative, such as prefixed by /rsp/place
 */
static struct {
  const xmlChar* xpath;
  flickcurl_place_type place_type;
  unsigned short place_array;
} place_fields_table[PHOTO_FIELD_LAST + 4]={
  {
    (const xmlChar*)"./location/@name",
    FLICKCURL_PLACE_LOCATION,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./location/@place_id",
    FLICKCURL_PLACE_LOCATION,
    PLACE_ID
  }
  ,
  {
    (const xmlChar*)"./location/@place_url",
    FLICKCURL_PLACE_LOCATION,
    PLACE_URL
  }
  ,
  {
    (const xmlChar*)"./location/locality/@place_id",
    FLICKCURL_PLACE_LOCALITY,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./location/locality",
    FLICKCURL_PLACE_LOCALITY,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./location/county/@place_id",
    FLICKCURL_PLACE_COUNTY,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./location/county",
    FLICKCURL_PLACE_COUNTY,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./location/region/@place_id",
    FLICKCURL_PLACE_REGION,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./location/region",
    FLICKCURL_PLACE_REGION,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./location/country/@place_id",
    FLICKCURL_PLACE_COUNTRY,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./location/country",
    FLICKCURL_PLACE_COUNTRY,
    PLACE_NAME,
  }
  ,
  { 
    NULL,
    (flickcurl_place_type)0,
    (unsigned short)0
  }
};



flickcurl_place*
flickcurl_build_place(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr)
{
  flickcurl_place* place=NULL;
  int nodes_count;
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

  for(i=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    int expri;
    xmlXPathContextPtr xpathNodeCtx=NULL;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    place=(flickcurl_place*)calloc(sizeof(flickcurl_place), 1);

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
      
      switch(place_array) {
        case PLACE_NAME:
          place->names[(int)place_type]=value;
          break;
          
        case PLACE_ID:
          place->ids[(int)place_type]=value;
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

    /* Handle only first place */
    break;

  } /* for places */
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    place=NULL;

  return place;
}
