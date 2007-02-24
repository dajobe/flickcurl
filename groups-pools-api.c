/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * groups-pools-api.c - Flickr groups.pool API calls
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
 * flickcurl_groups_pools_add:
 *
 * flickr.groups.pools.add
 */


/**
 * flickcurl_groups_pools_getContext:
 * @fc: flickcurl context
 * @id: photo ID
 * 
 * Get next and previous photos for a photo in a group pool.
 * 
 * Implements flickr.groups.pools.getContext (0.7)
 *
 * Return value: an array of size 3 [prev, next, NULL] flickcurl_context*
 * or NULL on error
 **/
flickcurl_context**
flickcurl_groups_pools_getContext(flickcurl* fc, const char* photo_id,
                                  const char* group_id)
{
  const char * parameters[5][2];
  int count=0;
  xmlDocPtr doc=NULL;
  flickcurl_context** contexts=NULL;
  
  if(!photo_id || !group_id)
    return NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "group_id";
  parameters[count++][1]= group_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.pools.getContext", parameters, count))
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
 * flickcurl_groups_pools_getGroups:
 *
 * flickr.groups.pools.getGroups
 */


/**
 * flickcurl_groups_pools_getPhotos:
 *
 * flickr.groups.pools.getPhotos
 */


/**
 * flickcurl_groups_pools_remove:
 *
 * flickr.groups.pools.remove
 */
