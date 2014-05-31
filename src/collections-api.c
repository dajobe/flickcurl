/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * collections-api.c - Flickr flickr.collections.* API calls
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


/**
 * flickcurl_collections_getInfo:
 * @fc: flickcurl context
 * @collection_id: The ID of the collection to fetch information for.
 * 
 * Returns information for a single collection.  Currently can only
 * be called by the collection owner, this may change.
 *
 * Implements flickr.collections.getInfo (1.12)
 * 
 * Return value: a collection or NULL on failure
 **/
flickcurl_collection*
flickcurl_collections_getInfo(flickcurl* fc, const char* collection_id)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_collection* collection = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!collection_id)
    return NULL;

  flickcurl_add_param(fc, "collection_id", collection_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.collections.getInfo"))
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

  collection = flickcurl_build_collection(fc, xpathCtx,
                                          (const xmlChar*)"/rsp/collection");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(collection)
      flickcurl_free_collection(collection);
    collection = NULL;
  }

  return collection;
}


/**
 * flickcurl_collections_getTree:
 * @fc: flickcurl context
 * @collection_id: The ID of the collection to fetch a tree for, or zero to fetch the root collection. Defaults to zero. (or NULL)
 * @user_id: The ID of the account to fetch the collection tree for. Deafults to the calling user. (or NULL)
 * 
 * Returns a tree (or sub tree) of collections belonging to a given user.
 *
 * Implements flickr.collections.getTree (1.12)
 * 
 * Return value: a collection or NULL on failure
 **/
flickcurl_collection*
flickcurl_collections_getTree(flickcurl* fc, const char* collection_id,
                              const char* user_id)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_collection* collection  =  NULL;
  
  flickcurl_init_params(fc, 0);

  if(collection_id) {
    flickcurl_add_param(fc, "collection_id", collection_id);
  }
  if(user_id) {
    flickcurl_add_param(fc, "user_id", user_id);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.collections.getTree"))
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

  collection = flickcurl_build_collection(fc, xpathCtx,
                                          (const xmlChar*)"/rsp/collections/collection");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(collection)
      flickcurl_free_collection(collection);

    collection = NULL;
  }

  return collection;
}
