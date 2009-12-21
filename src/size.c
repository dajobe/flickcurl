/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * size.c - Flickcurl size functions
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

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_free_size:
 * @size: size object
 *
 * Destructor for size object
 */
void
flickcurl_free_size(flickcurl_size *size)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(size, flickcurl_size);

  if(size->label)
    free(size->label);
  
  if(size->source)
    free(size->source);
  
  if(size->url)
    free(size->url);
  
  if(size->media)
    free(size->media);
  
  free(size);
}


/**
 * flickcurl_free_sizes:
 * @sizes_object: size object array
 *
 * Destructor for array of size objects
 */
void
flickcurl_free_sizes(flickcurl_size **sizes_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(sizes_object, flickcurl_size_array);

  for(i = 0; sizes_object[i]; i++)
    flickcurl_free_size(sizes_object[i]);
  
  free(sizes_object);
}


flickcurl_size**
flickcurl_build_sizes(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr, int* size_count_p)
{
  flickcurl_size** sizes = NULL;
  int nodes_count;
  int size_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
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
  sizes = (flickcurl_size**)calloc(sizeof(flickcurl_size*), nodes_count+1);
  
  for(i = 0, size_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_size* s;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    s = (flickcurl_size*)calloc(sizeof(flickcurl_size), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "label"))
        s->label = attr_value;
      else if(!strcmp(attr_name, "width")) {
        s->width = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "height")) {
        s->height = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "source")) {
        s->source = attr_value;
      } else if(!strcmp(attr_name, "url")) {
        s->url = attr_value;
      } else if(!strcmp(attr_name, "media")) {
        s->media = attr_value;
      }
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "size: label %s width %d height %d  source %s url %s\n",
            s->label, s->width, s->height, s->source, s->url);
#endif
    
    sizes[size_count++] = s;
  } /* for nodes */

  if(size_count_p)
    *size_count_p = size_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return sizes;
}
