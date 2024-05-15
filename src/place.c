/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * place.c - Flickr place support calls
 *
 * Copyright (C) 2008-2009, David Beckett http://www.dajobe.org/
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


static const char* flickcurl_place_type_label[FLICKCURL_PLACE_LAST+1] = {
  "location",
  "neighbourhood",
  "locality",
  "county",
  "region",
  "country",
  "continent"
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
  for(i = 0; flickcurl_place_type_label[i]; i++) {
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

  for(i = 0; i <= FLICKCURL_PLACE_LAST; i++) {
    if(place->names[i])
      free(place->names[i]);
    if(place->ids[i])
      free(place->ids[i]);
    if(place->urls[i])
      free(place->urls[i]);
    if(place->woe_ids[i])
      free(place->woe_ids[i]);
  }
  
  if(place->shape)
    flickcurl_free_shape(place->shape);

  if(place->timezone)
    free(place->timezone);

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

  for(i = 0; places_object[i]; i++)
    flickcurl_free_place(places_object[i]);
  
  free(places_object);
}


/* flickcurl_place fields */
typedef enum {
  PLACE_NONE = 0,
  /* place->names[place_type] */
  PLACE_NAME,
  /* place->ids[place_type] */
  PLACE_ID,
  /* place->urls[place_type] */
  PLACE_URL,
  /* place->woe_ids[place_type] */
  PLACE_WOE_ID,
  /* place->type */
  PLACE_TYPE,
  /* place->latitude */
  PLACE_LATITUDE,
  /* place->longitude */
  PLACE_LONGITUDE,
  /* place->count */
  PLACE_PHOTO_COUNT,
  /* place->shape: source of derived DEPRECATED fields:
   * shapedata, shapedata_length, shapfile_urls and shapefile_urls_count 
   */
  PLACE_SHAPE,
  /* place->timezone */
  PLACE_TIMEZONE
} place_field_type;


#define PLACE_FIELDS_TABLE_SIZE 35

/*
 * The XPaths here are relative, such as prefixed by /rsp/place
 */
static struct {
  const xmlChar* xpath;
  flickcurl_place_type place_type;
  place_field_type place_field;
} place_fields_table[PLACE_FIELDS_TABLE_SIZE+1] = {
  {
    (const xmlChar*)"./@name",
    FLICKCURL_PLACE_LOCATION,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)".",
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
    (const xmlChar*)"./@timezone",
    FLICKCURL_PLACE_LOCATION,
    PLACE_TIMEZONE
  }
  ,
  {
    (const xmlChar*)"./neighborhood/@place_id",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./neighbourhood/@place_id",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./neighborhood/@woeid",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_WOE_ID,
  }
  ,
  {
    (const xmlChar*)"./neighbourhood/@woeid",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_WOE_ID,
  }
  ,
  {
    (const xmlChar*)"./neighborhood/@place_url",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_URL,
  }
  ,
  {
    (const xmlChar*)"./neighbourhood/@place_url",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_URL,
  }
  ,
  {
    (const xmlChar*)"./neighborhood",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./neighbourhood",
    FLICKCURL_PLACE_NEIGHBOURHOOD,
    PLACE_NAME,
  }
  ,
  {
    (const xmlChar*)"./locality/@place_id",
    FLICKCURL_PLACE_LOCALITY,
    PLACE_ID,
  }
  ,
  {
    (const xmlChar*)"./locality/@place_url",
    FLICKCURL_PLACE_LOCALITY,
    PLACE_URL,
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
    (const xmlChar*)"./county/@place_url",
    FLICKCURL_PLACE_COUNTY,
    PLACE_URL,
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
    (const xmlChar*)"./region/@place_url",
    FLICKCURL_PLACE_REGION,
    PLACE_URL,
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
    (const xmlChar*)"./country/@place_url",
    FLICKCURL_PLACE_COUNTRY,
    PLACE_URL,
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
    (const xmlChar*)"./@place_type", /* special */
    (flickcurl_place_type)0,
    PLACE_TYPE,
  }
  ,
  {
    (const xmlChar*)"./@latitude", /* special */
    (flickcurl_place_type)0,
    PLACE_LATITUDE,
  }
  ,
  {
    (const xmlChar*)"./@longitude", /* special */
    (flickcurl_place_type)0,
    PLACE_LONGITUDE,
  }
  ,
  {
    (const xmlChar*)"./@photo_count", /* special */
    (flickcurl_place_type)0,
    PLACE_PHOTO_COUNT,
  }
  ,
  {
    (const xmlChar*)"./shapedata", /* special */
    (flickcurl_place_type)0,
    PLACE_SHAPE,
  }
  ,
  { 
    NULL,
    (flickcurl_place_type)0,
    PLACE_NONE
  }
};



/* get shapedata from value */
flickcurl_place**
flickcurl_build_places(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* xpathExpr, int* place_count_p)
{
  flickcurl_place** places = NULL;
  int nodes_count;
  int place_count;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  int i;
  
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
  places = (flickcurl_place**)calloc(nodes_count+1, sizeof(flickcurl_place*));

  for(i = 0, place_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    int expri;
    xmlXPathContextPtr xpathNodeCtx = NULL;
    flickcurl_place* place;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    place = (flickcurl_place*)calloc(1, sizeof(flickcurl_place));
    place->type = FLICKCURL_PLACE_LOCATION;

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;

    for(expri = 0; expri <= FLICKCURL_PLACE_LAST; expri++) {
      if(place->names[expri]) {
        free(place->names[expri]);
        place->names[expri] = NULL;
      }
      if(place->ids[expri]) {
        free(place->ids[expri]);
        place->ids[expri] = NULL;
      }
      if(place->urls[expri]) {
        free(place->urls[expri]);
        place->urls[expri] = NULL;
      }
    }

    for(expri = 0; place_fields_table[expri].xpath; expri++) {
      flickcurl_place_type place_type = place_fields_table[expri].place_type;
      place_field_type place_field = place_fields_table[expri].place_field;
      const xmlChar* place_xpathExpr = place_fields_table[expri].xpath;
      char *value = NULL;
      
      if(place_field == PLACE_SHAPE) {
        place->shape = flickcurl_build_shape(fc, xpathNodeCtx, place_xpathExpr);
        if(place->shape) {
          /* copy pointers to DEPRECATED fields */
          place->shapedata            = place->shape->data;
          place->shapedata_length     = place->shape->data_length;
          place->shapefile_urls       = place->shape->file_urls;
          place->shapefile_urls_count = place->shape->file_urls_count;
        }
        continue;
      }
      
      value = flickcurl_xpath_eval(fc, xpathNodeCtx, place_xpathExpr);
      if(!value)
        continue;

#if FLICKCURL_DEBUG > 1
      fprintf(stderr, "field %d array #%d with value: '%s'\n",
              place_type, (int)place_field, value);
#endif
      
      switch(place_field) {
        case PLACE_NAME:
          place->names[(int)place_type] = value;
          break;
          
        case PLACE_ID:
          place->ids[(int)place_type] = value;
          break;

        case PLACE_WOE_ID:
          place->woe_ids[(int)place_type] = value;
          break;

        case PLACE_URL:
          place->urls[(int)place_type] = value;
          break;

        case PLACE_TYPE:
          place->type = flickcurl_get_place_type_by_label(value);
          free(value); value = NULL;
          break;

        case PLACE_LATITUDE:
          place->location.accuracy= -1;
          place->location.latitude = atof(value);
          free(value); value = NULL;
          break;

        case PLACE_LONGITUDE:
          place->location.accuracy= -1;
          place->location.longitude = atof(value);
          free(value); value = NULL;
          break;

        case PLACE_PHOTO_COUNT:
          place->count = atoi(value);
          free(value); value = NULL;
          break;

        case PLACE_TIMEZONE:
          place->timezone = value;
          break;

        case PLACE_SHAPE:
          /* handled above */
          break;

        case PLACE_NONE:
        default:
          flickcurl_error(fc, "Unknown place type %d",  (int)place_field);
          fc->failed = 1;
      }
      
      if(fc->failed) {
        free(value);
        goto placestidy;
      }
    } /* end for place fields */

   placestidy:
    xmlXPathFreeContext(xpathNodeCtx);

    places[place_count++] = place;
  } /* for places */
  
  if(place_count_p)
    *place_count_p = place_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed) {
    if(places)
      flickcurl_free_places(places);
    places = NULL;
  }

  return places;
}


flickcurl_place*
flickcurl_build_place(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr)
{
  flickcurl_place** places;
  flickcurl_place* result = NULL;

  places = flickcurl_build_places(fc, xpathCtx, xpathExpr, NULL);

  if(places) {
    result = places[0];
    free(places);
  }
  
  return result;
}


/**
 * flickcurl_place_type_to_id:
 * @place_type: place type
 *
 * Turn a place type into a place ID
 *
 * Return value: place ID for type or <0 on failure
 */
int
flickcurl_place_type_to_id(flickcurl_place_type place_type)
{
  int place_type_id = -1;
  
  if(place_type == FLICKCURL_PLACE_NEIGHBORHOOD)
    place_type_id = 22;
  else if(place_type == FLICKCURL_PLACE_LOCALITY)
    place_type_id = 7;
  else if(place_type == FLICKCURL_PLACE_REGION)
    place_type_id = 8;
  else if(place_type == FLICKCURL_PLACE_COUNTRY)
    place_type_id = 12;
  else if(place_type == FLICKCURL_PLACE_CONTINENT)
    place_type_id = 29;
  else
    place_type_id = -1;

  return place_type_id;
}


/**
 * flickcurl_place_id_to_type:
 * @place_type_id: place type ID
 *
 * Turn a place type into a place ID
 *
 * Return value: place type for fID or FLICKCURL_PLACE_LOCATION on failure
 */
flickcurl_place_type
flickcurl_place_id_to_type(int place_type_id)
{
  flickcurl_place_type place_type = FLICKCURL_PLACE_LOCATION;
  
  if(place_type_id == 22)
    place_type = FLICKCURL_PLACE_NEIGHBORHOOD;
  else if(place_type_id == 7)
    place_type = FLICKCURL_PLACE_LOCALITY;
  else if(place_type_id == 8)
    place_type = FLICKCURL_PLACE_REGION;
  else if(place_type_id == 12)
    place_type = FLICKCURL_PLACE_COUNTRY;
  else if(place_type_id == 29)
    place_type = FLICKCURL_PLACE_CONTINENT;
  else
    place_type = FLICKCURL_PLACE_LOCATION;

  return place_type;
}


flickcurl_place_type_info**
flickcurl_build_place_types(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                            const xmlChar* xpathExpr, int* place_type_count_p)
{
  flickcurl_place_type_info** place_types = NULL;
  int nodes_count;
  int place_type_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do place_types */
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
  place_types = (flickcurl_place_type_info**)calloc(nodes_count + 1,
                                                    sizeof(flickcurl_place_type_info*));
  
  for(i = 0, place_type_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_place_type_info* pt;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    pt = (flickcurl_place_type_info*)calloc(1, sizeof(*pt));
    
    for(attr = node->properties; attr; attr = attr->next) {
      size_t attr_len = strlen((const char*)attr->children->content);
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(attr_len + 1);
      memcpy(attr_value, attr->children->content, attr_len + 1);
      
      if(!strcmp(attr_name, "id")) {
        pt->id = atoi(attr_value);
        free(attr_value);
        pt->type = flickcurl_place_id_to_type(pt->id);
      } else
        free(attr_value);
    }

    /* Walk children nodes for name text */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        size_t len = strlen((const char*)chnode->content);
        pt->name = (char*)malloc(len + 1);
        memcpy(pt->name, chnode->content, len + 1);
      }
    }

//#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "place_type: id %d  type %d  name %s\n",
            pt->id, pt->type, pt->name);
//#endif
    
    place_types[place_type_count++] = pt;
  } /* for nodes */

  if(place_type_count_p)
    *place_type_count_p = place_type_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return place_types;
}


/**
 * flickcurl_free_place_type_infos:
 * @ptis_object: list of place type info
 *
 * Destructor for place type info list
 */
void
flickcurl_free_place_type_infos(flickcurl_place_type_info **ptis_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(ptis_object,
                                         flickcurl_place_type_info);

  for(i = 0; ptis_object[i]; i++) {
    flickcurl_place_type_info *pti = ptis_object[i];
    char * n = pti->name;
    if(n)
      free(n);
    free(pti);
  }

  free(ptis_object);
}


