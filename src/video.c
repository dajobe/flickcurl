/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * video.c - Flickr video support calls
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


/**
 * flickcurl_free_video:
 * @video: video object
 *
 * Destructor for video object
 */
void
flickcurl_free_video(flickcurl_video *video)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(video, flickcurl_video);

  free(video);
}


flickcurl_video*
flickcurl_build_video(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr)
{
  flickcurl_video* v = NULL;
  int nodes_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  int count = 0;
  
  /* Now do video */
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
  
  v = (flickcurl_video*)calloc(1, sizeof(flickcurl_video));
  if(!v) {
    flickcurl_error(fc, "Unable to allocate the memory needed for video.");
    fc->failed = 1;
    goto tidy;
  }

  

  for(i = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    const char *node_name = (const char*)node->name;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    if(strcmp(node_name, "video"))
      continue;
    
    count++;
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      int attr_value = atoi((const char*)attr->children->content);
      if(!strcmp(attr_name, "ready"))
        v->ready = attr_value;
      else if(!strcmp(attr_name, "failed"))
        v->failed = attr_value;
      else if(!strcmp(attr_name, "pending"))
        v->pending = attr_value;
      else if(!strcmp(attr_name, "duration"))
        v->duration = attr_value;
      else if(!strcmp(attr_name, "width"))
        v->width = attr_value;
      else if(!strcmp(attr_name, "height"))
        v->height = attr_value;
    }

  } /* for nodes */

  if(!count) {
    flickcurl_free_video(v);
    v = NULL;
  } 
#if FLICKCURL_DEBUG > 1
else {
    fprintf(stderr, "video: ready %d  failed %d  pending %d  duration %d  width %d  height %d\n",
            v->ready, v->failed, v->pending, v->duration,
            v->width, v->height);
  }
#endif
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return v;
}
