/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * location.c - Flickcurl method location functions
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

/* for atof() */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_free_location:
 * @location: location object
 *
 * Destructor for location object
 */
void
flickcurl_free_location(flickcurl_location *location)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(location, flickcurl_location);

  free(location);
}


flickcurl_location*
flickcurl_build_location(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                         const xmlChar* xpathExpr)
{
  flickcurl_location* location = NULL;
  int nodes_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do location */
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
  
  for(i = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    
    if(node->type != XML_ELEMENT_NODE)
      continue;

    location = (flickcurl_location*)calloc(1, sizeof(flickcurl_location));
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;
      size_t attr_value_len = strlen((const char*)attr->children->content);
      
      attr_value = (char*)malloc(attr_value_len + 1);
      memcpy(attr_value, attr->children->content, attr_value_len + 1);
      
      if(!strcmp(attr_name, "latitude"))
        location->latitude = atof(attr_value);
      else if(!strcmp(attr_name, "longitude"))
        location->longitude = atof(attr_value);
      else if(!strcmp(attr_name, "accuracy"))
        location->accuracy = atoi(attr_value);

      free(attr_value);
    }

    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "location: lat %f long %f accuracy %d\n",
            location->latitude, location->longitude, location->accuracy);
#endif

    /* Handle only first location */
    break;
  } /* for nodes */

 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return location;
}


static const char* flickcurl_accuracy_labels[16] = {
  "world",   "world",  "country", "country",
  "country", "region", "region",  "region",
  "region",  "region", "city",    "city",
  "city",    "city",   "city",    "street"
};


/**
 * flickcurl_get_location_accuracy_label:
 * @accuracy: accuracy
 *
 * Get label for an accuracy
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_location_accuracy_label(int accuracy)
{
  if(accuracy >= 1 && accuracy <= 16)
    return flickcurl_accuracy_labels[accuracy-1];
  return NULL;
}

