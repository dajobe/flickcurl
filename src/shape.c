/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * shape.c - Flickr shape support calls
 *
 * Copyright (C) 2009, David Beckett http://www.dajobe.org/
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


/**
 * flickcurl_free_shape:
 * @shape: shape object
 *
 * Destructor for shape object
 */
void
flickcurl_free_shape(flickcurl_shapedata *shape)
{
  int i;

  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(shape, flickcurl_shapedata);

  if(shape->data)
    free(shape->data);

  if(shape->file_urls) {
    for(i = 0 ; i < shape->file_urls_count; i++)
      free(shape->file_urls[i]);
    free(shape->file_urls);
  }

  free(shape);
}


/**
 * flickcurl_free_shapes:
 * @shapes_object: shape object array
 *
 * Destructor for array of shape object
 */
void
flickcurl_free_shapes(flickcurl_shapedata **shapes_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(shapes_object, flickcurl_shapedata_array);

  for(i = 0; shapes_object[i]; i++)
    flickcurl_free_shape(shapes_object[i]);
  
  free(shapes_object);
}


/* flickcurl_shapedata fields */
typedef enum {
  SHAPE_NONE = 0,
  /* shape->created */
  SHAPE_CREATED,
  /* shape->alpha */
  SHAPE_ALPHA,
  /* shape->points */
  SHAPE_POINTS,
  /* shape->edges */
  SHAPE_EDGES,
  /* shape->data */
  SHAPE_DATA,
  /* shape->file_urls */
  SHAPE_FILE_URL,
  /* shape->is_donuthole */
  SHAPE_IS_DONUTHOLE,
  /* shape->has_donuthole */
  SHAPE_HAS_DONUTHOLE
} shape_field_type;


#define SHAPE_FIELDS_TABLE_SIZE 34

/*
 * The XPaths here are relative, such as prefixed by /rsp/shape
 */
static struct {
  const xmlChar* xpath;
  shape_field_type shape_field;
} shape_fields_table[SHAPE_FIELDS_TABLE_SIZE+1] = {
  {
    (const xmlChar*)"./@created",
    SHAPE_CREATED,
  }
  ,
  {
    (const xmlChar*)"./@alpha",
    SHAPE_ALPHA
  }
  ,
  {
    (const xmlChar*)"./@count_points",
    SHAPE_POINTS
  }
  ,
  {
    (const xmlChar*)"./@count_edges",
    SHAPE_EDGES
  }
  ,
  {
    (const xmlChar*)"./@is_donuthole",
    SHAPE_IS_DONUTHOLE
  }
  ,
  {
    (const xmlChar*)"./@has_donuthole",
    SHAPE_HAS_DONUTHOLE
  }
  ,
  {
    (const xmlChar*)"./polylines/.", /* special */
    SHAPE_DATA,
  }
  ,
  {
    (const xmlChar*)"./urls/shapefile", /* special */
    SHAPE_FILE_URL,
  }
  ,
  { 
    NULL,
    SHAPE_NONE
  }
};



/* get shapedata from value */
flickcurl_shapedata**
flickcurl_build_shapes(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* xpathExpr, int* shape_count_p)
{
  flickcurl_shapedata** shapes = NULL;
  int nodes_count;
  int shape_count;
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
  shapes = (flickcurl_shapedata**)calloc(sizeof(flickcurl_shapedata*), nodes_count+1);

  for(i = 0, shape_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    int expri;
    xmlXPathContextPtr xpathNodeCtx = NULL;
    flickcurl_shapedata* shape;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    shape = (flickcurl_shapedata*)calloc(sizeof(flickcurl_shapedata), 1);

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;

    for(expri = 0; shape_fields_table[expri].xpath; expri++) {
      shape_field_type shape_field = shape_fields_table[expri].shape_field;
      const xmlChar* shape_xpathExpr = shape_fields_table[expri].xpath;
      char *value = NULL;
      
      if(shape_field == SHAPE_DATA) {
        shape->data = flickcurl_xpath_eval_to_tree_string(fc,
                                                          xpathNodeCtx,
                                                          shape_xpathExpr,
                                                          &shape->data_length);
        continue;
      }
      
      value = flickcurl_xpath_eval(fc, xpathNodeCtx, shape_xpathExpr);
      if(!value)
        continue;

#if FLICKCURL_DEBUG > 1
      fprintf(stderr, "field %d with value: '%s'\n", (int)shape_field, value);
#endif
      
      switch(shape_field) {
        case SHAPE_CREATED:
          shape->created = atoi(value);
          free(value); value = NULL;
          break;
          
        case SHAPE_ALPHA:
          shape->alpha = atof(value);
          free(value); value = NULL;
          break;

        case SHAPE_POINTS:
          shape->points = atoi(value);
          free(value); value = NULL;
          break;

        case SHAPE_EDGES:
          shape->edges = atoi(value);
          free(value); value = NULL;
          break;

        case SHAPE_IS_DONUTHOLE:
          shape->is_donuthole = atoi(value);
          free(value); value = NULL;
          break;

        case SHAPE_HAS_DONUTHOLE:
          shape->has_donuthole = atoi(value);
          free(value); value = NULL;
          break;

        case SHAPE_DATA:
          /* handled above */
          break;

        case SHAPE_FILE_URL:
          if(1) {
            int size = shape->file_urls_count;
            char** shapefile_urls;
            shapefile_urls = (char**)calloc(size+1+1, sizeof(char*));
            if(shapefile_urls) {
              if(size)
                memcpy(shapefile_urls, shape->file_urls, size * sizeof(char*));
              shapefile_urls[size++] = value;
              shapefile_urls[size] = NULL;
              value = NULL;
              
              shape->file_urls_count++;
              free(shape->file_urls);
              shape->file_urls = shapefile_urls;
            } else
              fc->failed = 1;

            if(value) {
              free(value); value = NULL;
            }
          }
          break;

        case SHAPE_NONE:
        default:
          flickcurl_error(fc, "Unknown shape field %d",  shape_field);
          fc->failed = 1;
      }
      
      if(fc->failed)
        goto shapestidy;
    } /* end for shape fields */

   shapestidy:
    if(xpathNodeCtx)
      xmlXPathFreeContext(xpathNodeCtx);

    shapes[shape_count++] = shape;
  } /* for shapes */
  
  if(shape_count_p)
    *shape_count_p = shape_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    shapes = NULL;

  return shapes;
}


flickcurl_shapedata*
flickcurl_build_shape(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr)
{
  flickcurl_shapedata** shapes;
  flickcurl_shapedata* result = NULL;

  shapes = flickcurl_build_shapes(fc, xpathCtx, xpathExpr, NULL);

  if(shapes) {
    result = shapes[0];
    free(shapes);
  }
  
  return result;
}
