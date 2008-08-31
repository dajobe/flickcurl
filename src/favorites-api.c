/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * favorites-api.c - Flickr flickr.favorites.* API calls
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
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  
  if(!photo_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.favorites.add", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  return fc->failed;
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
 * Optional extra type 'media' that will return an extra media=VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_favorites_getList_params(flickcurl* fc, const char* user_id,
                                   flickcurl_photos_list_params* list_params)
{
  const char* parameters[12][2];
  int count=0;
  flickcurl_photos_list* photos_list=NULL;
  const char* format=NULL;
   
  /* API parameters */
  if(user_id) {
    parameters[count][0]  = "user_id";
    parameters[count++][1]= user_id;
  }
  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.favorites.getList", parameters, count))
    goto tidy;

  photos_list=flickcurl_invoke_photos_list(fc,
                                           (const xmlChar*)"/rsp/photos/photo",
                                           format);

  tidy:
  if(fc->failed) {
    if(photos_list)
      flickcurl_free_photos_list(photos_list);
    photos_list=NULL;
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

  photos_list=flickcurl_favorites_getList_params(fc, user_id, &list_params);
  if(!photos_list)
    return NULL;

  photos=photos_list->photos; photos_list->photos=NULL;  
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
 * Optional extra type 'media' that will return an extra media=VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_favorites_getPublicList_params(flickcurl* fc, const char* user_id,
                                         flickcurl_photos_list_params* list_params)
{
  const char* parameters[13][2];
  int count=0;
  flickcurl_photos_list* photos_list=NULL;
  const char* format=NULL;
  
  if(!user_id)
    return NULL;

  /* API parameters */
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.favorites.getPublicList", parameters, count))
    goto tidy;

  photos_list=flickcurl_invoke_photos_list(fc,
                                           (const xmlChar*)"/rsp/photos/photo",
                                           format);

  tidy:
  if(fc->failed) {
    if(photos_list)
      flickcurl_free_photos_list(photos_list);
    photos_list=NULL;
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

  photos_list=flickcurl_favorites_getPublicList_params(fc, user_id,
                                                       &list_params);
  if(!photos_list)
    return NULL;

  photos=photos_list->photos; photos_list->photos=NULL;  
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
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  
  if(!photo_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.favorites.remove", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  return fc->failed;
}


