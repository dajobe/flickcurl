/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photosets-api.c - Flickr photosets API calls
 *
 * Copyright (C) 2007, David Beckett http://purl.org/net/dajobe/
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
 * flickcurl_photosets_addPhoto:
 *
 * flickr.photosets.addPhoto
 */


/**
 * flickcurl_photosets_create:
 *
 * flickr.photosets.create
 */


/**
 * flickcurl_photosets_delete:
 *
 * flickr.photosets.delete
 */


/**
 * flickcurl_photosets_editMeta:
 *
 * flickr.photosets.editMeta
 */


/**
 * flickcurl_photosets_editPhotos:
 *
 * flickr.photosets.editPhotos
 */


/**
 * flickcurl_photosets_getContext:
 * @fc: flickcurl context
 * @id: photo ID
 *
 * Get next and previous photos for a photo in a set.
 * 
 * Implements flickr.photosets.getContext (0.7)
 *
 * Return value: an array of size 3 [prev, next, NULL] flickcurl_context*
 * or NULL on error
 **/
flickcurl_context**
flickcurl_photosets_getContext(flickcurl* fc, const char* photo_id,
                               const char* photoset_id)
{
  const char * parameters[5][2];
  int count=0;
  xmlDocPtr doc=NULL;
  flickcurl_context** contexts=NULL;
  
  if(!photo_id || !photoset_id)
    return NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.getContext", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  contexts=flickcurl_build_contexts(fc, doc);

 tidy:
  if(fc->failed)
    contexts=NULL;

  return contexts;
}


/**
 * flickcurl_photosets_getInfo:
 *
 * flickr.photosets.getInfo
 */


/**
 * flickcurl_photosets_getList:
 *
 * flickr.photosets.getList
 */


/**
 * flickcurl_photosets_getPhotos:
 *
 * flickr.photosets.getPhotos
 */


/**
 * flickcurl_photosets_orderSets:
 *
 * flickr.photosets.orderSets
 */


/**
 * flickcurl_photosets_removePhoto:
 *
 * flickr.photosets.removePhoto
 */
