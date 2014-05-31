/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickr.panda-api.c - Flickr flickr.panda.* API calls
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


static char**
flickcurl_build_pandas(flickcurl* fc, 
                       xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr,
                       int* panda_count_p)
{
  char** pandas = NULL;
  int nodes_count;
  int panda_count;
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
  pandas = (char**)calloc(sizeof(char*), nodes_count+1);
  
  for(i = 0, panda_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    char *panda = NULL;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    /* Use first text child node of <panda> as panda name */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type  == XML_TEXT_NODE) {
        size_t len = strlen((const char*)chnode->content);
        panda = (char*)malloc(len + 1);
        memcpy(panda, chnode->content, len + 1);
        break;
      }
    }
    
    if(panda) {
#if FLICKCURL_DEBUG > 1
      fprintf(stderr, "panda: %s\n", panda);
#endif
      pandas[panda_count++] = panda;
    }
  } /* for nodes */

  if(panda_count_p)
    *panda_count_p = panda_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return pandas;
}


/**
 * flickcurl_panda_getList:
 * @fc: flickcurl context
 * 
 * Get the current list of Flickr Pandas
 *
 * Can be used with flickcurl_panda_getPhotos() to get photos for the
 * given <ulink url="https://www.flickr.com/explore/panda">Flickr Panda</ulink>
 *
 * Announced 2009-03-03
 * http://code.flickr.com/blog/2009/03/03/panda-tuesday-the-history-of-the-panda-new-apis-explore-and-you/
 *
 * Implements flickr.panda.getList (1.9)
 * 
 * Return value: non-0 on failure
 **/
char**
flickcurl_panda_getList(flickcurl* fc)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char **pandas = NULL;
  
  flickcurl_init_params(fc, 0);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.panda.getList"))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }


  pandas = flickcurl_build_pandas(fc, 
                                  xpathCtx,
                                  (const xmlChar*)"/rsp/pandas/panda", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(pandas)
      free(pandas);
    pandas = NULL;
  }

  return pandas;
}


/**
 * flickcurl_panda_getPhotos:
 * @fc: flickcurl context
 * @panda_name: The name of the panda to ask for photos from.
 * 
 * Ask the Flickr Pandas for a list of recent public (and "safe") photos.
 *
 * Use flickcurl_panda_getList() to get the list of
 * <ulink url="https://www.flickr.com/explore/panda">Flickr Pandas</ulink>
 *
 * Announced 2009-03-03
 * http://code.flickr.com/blog/2009/03/03/panda-tuesday-the-history-of-the-panda-new-apis-explore-and-you/
 *
 * Only "supports the following extras: ownername, license,
 * date_upload, date_taken, tags, machine_tags, views, media" -
 * Sergey M., Flickr staff 2011-09-16
 * http://tech.groups.yahoo.com/group/yws-flickr/message/7203
 *
 * Implements flickr.panda.getPhotos (1.9)
 * 
 * Return value: photos array or NULL on failure
 **/
flickcurl_photo**
flickcurl_panda_getPhotos(flickcurl *fc, const char *panda_name)
{
  flickcurl_photo** photos = NULL;
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!panda_name)
    return NULL;

  flickcurl_add_param(fc, "panda_name", panda_name);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.panda.getPhotos"))
    goto tidy;

  photos_list = flickcurl_invoke_photos_list(fc,
                                             (const xmlChar*)"/rsp/photos",
                                             format);
  if(!photos_list)
    fc->failed = 1;
  
  tidy:
  if(fc->failed) {
    if(photos_list)
      flickcurl_free_photos_list(photos_list);
    photos_list = NULL;
  }

  if(photos_list) {
    photos = photos_list->photos; photos_list->photos = NULL;  

    /* photos array is now owned by this function */
    
    flickcurl_free_photos_list(photos_list);
  }

  return photos;
}
