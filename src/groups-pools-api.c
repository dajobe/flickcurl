/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * groups-pools-api.c - Flickr flickr.groups.pool.* API calls
 *
 * Copyright (C) 2007-2012, David Beckett http://www.dajobe.org/
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
 * flickcurl_groups_pools_add:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to add to the group pool.
 * @group_id: The NSID of the group who's pool the photo is to be added to.
 * 
 * Add a photo to a group's pool.
 *
 * Implements flickr.groups.pools.add (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_pools_add(flickcurl* fc, const char* photo_id,
                           const char* group_id)
{
  xmlDocPtr doc = NULL;
  int result = 1;
  
  flickcurl_init_params(fc, 1);

  if(!photo_id || !group_id)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);
  flickcurl_add_param(fc, "group_id", group_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.pools.add"))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result = 0;

  tidy:
  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_groups_pools_getContext:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * @group_id: group ID
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
  xmlDocPtr doc = NULL;
  flickcurl_context** contexts = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!photo_id || !group_id)
    return NULL;
  
  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_add_param(fc, "group_id", group_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.pools.getContext"))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  contexts = flickcurl_build_contexts(fc, doc);

 tidy:
  if(fc->failed) {
    if(contexts)
      flickcurl_free_contexts(contexts);
    contexts = NULL;
  }

  return contexts;
}


/**
 * flickcurl_groups_pools_getGroups:
 * @fc: flickcurl context
 * @per_page: Number of groups to return per page (default 400, max 400)
 * @page: The page of results to return (default 1)
 * 
 * Returns a list of groups to which you can add photos.
 *
 * Implements flickr.groups.pools.getGroups (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_group**
flickcurl_groups_pools_getGroups(flickcurl* fc, int page, int per_page)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_group** groups = NULL;
  char per_page_s[10];
  char page_s[10];
  
  flickcurl_init_params(fc, 0);

  sprintf(page_s, "%d", page);
  flickcurl_add_param(fc, "page", page_s);
  sprintf(per_page_s, "%d", per_page);
  flickcurl_add_param(fc, "per_page", per_page_s);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.pools.getGroups"))
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

  groups = flickcurl_build_groups(fc, xpathCtx,
                                (const xmlChar*)"/rsp/groups/group", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(groups)
      flickcurl_free_groups(groups);
    groups = NULL;
  }

  return groups;
}


/**
 * flickcurl_groups_pools_getPhotos_params:
 * @fc: flickcurl context
 * @group_id: The id of the group who's pool you which to get the photo list for.
 * @tags: A tag to filter the pool with. At the moment only one tag at a time is supported. (or NULL)
 * @user_id: The nsid of a user (or NULL).  If given, retrieves only photos that the user has contributed to the group pool.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of pool photos for a given group.
 *
 * Currently supported extra fields are: license, date_upload,
 * date_taken, owner_name, icon_server, original_format,
 * last_update, geo, tags, machine_tags.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_groups_pools_getPhotos_params(flickcurl* fc, const char* group_id,
                                        const char* tags, const char* user_id,
                                        flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!group_id)
    return NULL;

  /* API parameters */
  flickcurl_add_param(fc, "group_id", group_id);
  if(tags) {
    flickcurl_add_param(fc, "tags", tags);
  }
  if(user_id) {
    flickcurl_add_param(fc, "user_id", user_id);
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);
  
  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.pools.getPhotos"))
    goto tidy;

  photos_list = flickcurl_invoke_photos_list(fc,
                                             (const xmlChar*)"/rsp/photos",
                                             format);

  tidy:
  if(fc->failed) {
    if(photos_list)
      flickcurl_free_photos_list(photos_list);
    photos_list = NULL;
  }

  return photos_list;
}


/**
 * flickcurl_groups_pools_getPhotos:
 * @fc: flickcurl context
 * @group_id: The id of the group who's pool you which to get the photo list for.
 * @tags: A tag to filter the pool with. At the moment only one tag at a time is supported. (or NULL)
 * @user_id: The nsid of a user (or NULL).  If given, retrieves only photos that the user has contributed to the group pool.
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * @per_page: Number of photos to return per page (default 100, max 500)
 * @page: The page of results to return (default 1)
 * 
 * Returns a list of pool photos for a given group.
 *
 * See flickcurl_groups_pools_getPhotos_params() for details
 * of the fields.
 *
 * Implements flickr.groups.pools.getPhotos (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_groups_pools_getPhotos(flickcurl* fc, const char* group_id,
                                 const char* tags, const char* user_id,
                                 const char* extras, int per_page, int page)
{
  flickcurl_photos_list_params list_params;
  flickcurl_photos_list* photos_list;
  flickcurl_photo** photos;
  
  memset(&list_params, '\0', sizeof(list_params));
  list_params.format   = NULL;
  list_params.extras   = extras;
  list_params.per_page = per_page;
  list_params.page     = page;

  photos_list = flickcurl_groups_pools_getPhotos_params(fc, group_id, tags,
                                                      user_id, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_groups_pools_remove:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to remove from the group pool.
 * @group_id: The NSID of the group who's pool the photo is to removed from.
 * 
 * Remove a photo from a group pool.
 *
 * Implements flickr.groups.pools.remove (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_pools_remove(flickcurl* fc, const char* photo_id,
                              const char* group_id)
{
  xmlDocPtr doc = NULL;
  int result = 1;
  
  flickcurl_init_params(fc, 1);

  if(!photo_id || !group_id)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);
  flickcurl_add_param(fc, "group_id", group_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.pools.remove"))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result = 0;

  tidy:
  if(fc->failed)
    result = 1;

  return result;
}
