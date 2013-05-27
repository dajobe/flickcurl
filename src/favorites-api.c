/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * favorites-api.c - Flickr flickr.favorites.* API calls
 *
 * Copyright (C) 2008-2012, David Beckett http://www.dajobe.org/
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
 * flickcurl_favorites_add:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to add to the user's favorites.
 * 
 * Adds a photo to a user's favorites list.
 *
 * Implements flickr.favorites.add (1.0)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_favorites_add(flickcurl* fc, const char* photo_id)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  flickcurl_init_params(fc, 1);

  if(!photo_id)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.favorites.add"))
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

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  return fc->failed;
}


/**
 * flickcurl_favorites_getContext:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to fetch the context for.
 * @user_id: The user who counts the photo as a favorite.
 * @num_prev: number of previous photos to return (?) (or < 0)
 * @num_next: number of next photos to return (?) (or < 0)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: description, license, date_upload, date_taken, owner_name, icon_server, original_format, last_update, geo, tags, machine_tags, o_dims, views, media, path_alias, url_sq, url_t, url_s, url_m, url_z, url_l, url_o (or NULL)
 * 
 * Returns next and previous favorites for a photo in a user's favorites.
 *
 * Implements flickr.favorites.getContext (1.22)
 * 
 * Return value: NULL-terminated array of photo lists (prev, next) or non-0 on failure
 **/
flickcurl_photos_list**
flickcurl_favorites_getContext(flickcurl* fc, const char* photo_id,
                               const char* user_id,
                               int num_prev,
                               int num_next,
                               const char* extras)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_photos_list** photos_lists = NULL;
  char num_prev_str[10];
  char num_next_str[10];
  int i;

  flickcurl_init_params(fc, 0);

  if(!photo_id || !user_id)
    return NULL;

  flickcurl_add_param(fc, "photo_id", photo_id);
  flickcurl_add_param(fc, "user_id", user_id);
  if(num_prev >= 0) {
    sprintf(num_prev_str, "%d", num_prev);
    flickcurl_add_param(fc, "num_prev", num_prev_str);
  }
  if(num_next >= 0) {
    sprintf(num_next_str, "%d", num_next);
    flickcurl_add_param(fc, "num_next", num_next_str);
  }
  /* this is the only standard photos response parameter supported 
   * so using flickcurl_append_photos_list_params() is not really needed
   */
  if(extras) {
    flickcurl_add_param(fc, "extras", extras);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.favorites.getContext"))
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

  /* 3 lists of photo lists: prev, next and NULL to end the list */
  photos_lists = (flickcurl_photos_list**)calloc(sizeof(flickcurl_photos_list*), 3);

  /* Decode the prev and next into photo lists */
  for(i = 0; i < 2; i++) {
    const xmlChar* xpathExpr = (i == 0) ? (const xmlChar*)"/rsp/prevphoto" : (const xmlChar*)"/rsp/nextphoto";
    flickcurl_photos_list* photos_list;
    xmlXPathObjectPtr xpathObj = NULL;

    xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
    if(!xpathObj) {
      flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                      xpathExpr);
      fc->failed = 1;
      goto tidy;
    }
    
    if(!xpathObj->nodesetval || !xpathObj->nodesetval->nodeTab) {
      /* No elements found in content - probably not a failure */
      xmlXPathFreeObject(xpathObj);
      continue;
    }

    photos_list = flickcurl_new_photos_list(fc);
    if(!photos_list) {
      fc->failed = 1;
      goto tidy;
    }

    photos_list->page = -1;
    photos_list->per_page = -1;
    photos_list->total_count = -1;

    photos_list->photos = flickcurl_build_photos(fc, xpathCtx, xpathExpr,
                                                 &photos_list->photos_count);
    xmlXPathFreeObject(xpathObj);

    photos_lists[i] = photos_list;
  }
  photos_lists[2] = NULL;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(photos_lists)
      free(photos_lists);
    photos_lists = NULL;
  }

  return photos_lists;
}



/**
 * flickcurl_favorites_getList_params:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to fetch the favorites list for. If this argument is omitted, the favorites list for the calling user is returned. (or NULL)
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of the user's favorite photos.
 *
 * Flickcurl 1.6: Added @list_params beyond flickcurl_favorites_getList()
 * to allow returning raw content if @list_params is present and
 * field @format is not NULL as announced 2008-08-25
 * http://code.flickr.com/blog/2008/08/25/api-responses-as-feeds/
 *
 * Only photos which the calling user has permission to see are returned.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_favorites_getList_params(flickcurl* fc, const char* user_id,
                                   flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
   
  flickcurl_init_params(fc, 0);

  /* API parameters */
  if(user_id) {
    flickcurl_add_param(fc, "user_id", user_id);
  }
  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);
  
  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.favorites.getList"))
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
 * flickcurl_favorites_getList:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to fetch the favorites list for. If this argument is omitted, the favorites list for the calling user is returned. (or NULL)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: license, date_upload, date_taken, owner_name, icon_server, original_format, last_update, geo, tags, machine_tags. (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or NULL)
 * 
 * Returns a list of the user's favorite photos.
 *
 * See flickcurl_favorites_getList_params() for details of parameters.
 *
 * Implements flickr.favorites.getList (1.0)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_favorites_getList(flickcurl* fc, const char* user_id,
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

  photos_list = flickcurl_favorites_getList_params(fc, user_id, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_favorites_getPublicList_params:
 * @fc: flickcurl context
 * @user_id: The user to fetch the favorites list for.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of favorite public photos for the given user.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_favorites_getPublicList_params(flickcurl* fc, const char* user_id,
                                         flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!user_id)
    return NULL;

  /* API parameters */
  flickcurl_add_param(fc, "user_id", user_id);

  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.favorites.getPublicList"))
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
 * flickcurl_favorites_getPublicList:
 * @fc: flickcurl context
 * @user_id: The user to fetch the favorites list for.
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: license, date_upload, date_taken, owner_name, icon_server, original_format, last_update, geo, tags, machine_tags. (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or NULL)
 * 
 * Returns a list of favorite public photos for the given user.
 *
 * See flickcurl_favorites_getPublicList_params() for details of parameters.
 *
 * Implements flickr.favorites.getPublicList (1.0)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_favorites_getPublicList(flickcurl* fc, const char* user_id,
                                  const char* extras,
                                  int per_page, int page)
{
  flickcurl_photos_list_params list_params;
  flickcurl_photos_list* photos_list;
  flickcurl_photo** photos;
  
  memset(&list_params, '\0', sizeof(list_params));
  list_params.format   = NULL;
  list_params.extras   = extras;
  list_params.per_page = per_page;
  list_params.page     = page;

  photos_list = flickcurl_favorites_getPublicList_params(fc, user_id,
                                                       &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_favorites_remove:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to remove from the user's favorites.
 * 
 * Removes a photo from a user's favorites list.
 *
 * Implements flickr.favorites.remove (1.0)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_favorites_remove(flickcurl* fc, const char* photo_id)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  flickcurl_init_params(fc, 1);

  if(!photo_id)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.favorites.remove"))
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

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  return fc->failed;
}


