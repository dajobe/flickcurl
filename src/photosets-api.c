/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photosets-api.c - Flickr flickr.photosets.* API calls
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
 * @fc: flickcurl context
 * @photoset_id: The id of the photoset to add a photo to.
 * @photo_id: The id of the photo to add to the set.
 * 
 * Add a photo to the end of an existing photoset.
 *
 * Implements flickr.photosets.addPhoto (0.13)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photosets_addPhoto(flickcurl* fc, const char* photoset_id,
                             const char* photo_id)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  
  if(!photoset_id || !photo_id)
    return 1;

  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.addPhoto", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;

  return result;
}


/**
 * flickcurl_photosets_create:
 * @fc: flickcurl context
 * @title: A title for the photoset.
 * @description: A description of the photoset which may contain limited html (or NULL)
 * @primary_photo_id: The id of the photo to represent this set. The photo must belong to the calling user.
 * @photoset_url_p: pointer to variable to store new photoset URL (or NULL)
 * 
 * Create a new photoset for the calling user.
 *
 * Implements flickr.photosets.create (0.13)
 * 
 * Return value: photoset ID or NULL on failure
 **/
char*
flickcurl_photosets_create(flickcurl* fc, const char* title,
                           const char* description,
                           const char* primary_photo_id,
                           char** photoset_url_p)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  char* photoset_id=NULL;
  
  if(!title || !primary_photo_id)
    return NULL;

  parameters[count][0]  = "title";
  parameters[count++][1]= title;
  if(description) {
    parameters[count][0]  = "description";
    parameters[count++][1]= description;
  }
  parameters[count][0]  = "primary_photo_id";
  parameters[count++][1]= primary_photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.create", parameters, count))
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

  photoset_id=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/photoset/@id");
  if(photoset_url_p) {
    *photoset_url_p=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/photoset/@url");
  }
  
  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photoset_id=NULL;

  return photoset_id;
}


/**
 * flickcurl_photosets_delete:
 * @fc: flickcurl context
 * @photoset_id: The id of the photoset to delete. Must be owned by the calling user.
 * 
 * Delete a photoset.
 *
 * Implements flickr.photosets.delete (0.13)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photosets_delete(flickcurl* fc, const char* photoset_id)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  
  if(!photoset_id)
    return 1;

  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.delete", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:

  return fc->failed;
}


/**
 * flickcurl_photosets_editMeta:
 * @fc: flickcurl context
 * @photoset_id: The id of the photoset to modify.
 * @title: The new title for the photoset.
 * @description: A description of the photoset which may contain limited html (or NULL)
 * 
 * Modify the meta-data for a photoset.
 *
 * Implements flickr.photosets.editMeta (0.13)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photosets_editMeta(flickcurl* fc, const char* photoset_id,
                             const char* title, const char* description)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  
  if(!photoset_id || !title)
    return 1;

  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;
  parameters[count][0]  = "title";
  parameters[count++][1]= title;
  if(description) {
    parameters[count][0]  = "description";
    parameters[count++][1]= description;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.editMeta", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;

  return result;
}


/**
 * flickcurl_photosets_editPhotos:
 * @fc: flickcurl context
 * @photoset_id: The id of the photoset to modify. Must belong to the calling user.
 * @primary_photo_id: The id of the photo to use as the 'primary' photo for the set. This id must also be passed along in photo_ids list argument.
 * @photo_ids_array: Array of photo ids to include in the set. They will appear in the set in the order sent. This list MUST contain the primary photo id. All photos must belong to the owner of the set. This list of photos replaces the existing list. Call flickr.photosets.addPhoto to append a photo to a set.
 * 
 * Modify the photos in a photoset.
 *
 * Use this method to add, remove and re-order photos.
 *
 * Implements flickr.photosets.editPhotos (0.13)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photosets_editPhotos(flickcurl* fc, const char* photoset_id,
                               const char* primary_photo_id,
                               const char** photo_ids_array)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  char* photo_ids=NULL;
  
  if(!photoset_id || !primary_photo_id || !photo_ids_array)
    return 1;

  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;
  parameters[count][0]  = "primary_photo_id";
  parameters[count++][1]= primary_photo_id;
  photo_ids=flickcurl_array_join(photo_ids_array, ',');
  parameters[count][0]  = "photo_ids";
  parameters[count++][1]= photo_ids;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.editPhotos", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;
  if(photo_ids)
    free(photo_ids);

  return result;
}


/**
 * flickcurl_photosets_getContext:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * @photoset_id: photoset ID
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
  const char * parameters[7][2];
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
 * @fc: flickcurl context
 * @photoset_id: The ID of the photoset to fetch information for.
 * 
 * Gets information about a photoset.
 *
 * Implements flickr.photosets.getInfo (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photoset*
flickcurl_photosets_getInfo(flickcurl* fc, const char* photoset_id)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_photoset* photoset=NULL;
  
  if(!photoset_id)
    return NULL;

  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.getInfo", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  photoset=flickcurl_build_photoset(fc, xpathCtx);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photoset=NULL;

  return photoset;
}


/**
 * flickcurl_photosets_getList:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to get a photoset list for. If none is specified, the calling user is assumed (or NULL)
 * 
 * Returns the photosets belonging to the specified user.
 *
 * Implements flickr.photosets.getList (0.13)
 * 
 * Return value: array of photoset IDs or NULL on failure
 **/
flickcurl_photoset**
flickcurl_photosets_getList(flickcurl* fc, const char* user_id)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_photoset** photoset_list=NULL;

  if(user_id) {
    parameters[count][0]  = "user_id";
    parameters[count++][1]= user_id;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.getList", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  photoset_list=flickcurl_build_photosets(fc, xpathCtx,
                                          (const xmlChar*)"/rsp/photosets/photoset", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photoset_list=NULL;

  return photoset_list;
}


/**
 * flickcurl_photosets_getPhotos_params:
 * @fc: flickcurl context
 * @photoset_id: The id of the photoset to return the photos for.
 * @privacy_filter: Return photos only matching a certain privacy level 1-5 (or <0)
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Get the list of photos in a set.
 *
 * Currently supported extra fields are: license, date_upload,
 * date_taken, owner_name, icon_server, original_format,
 * last_update.
 *
 * Optional extra type 'media' that will return an extra media=VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: list of photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_photosets_getPhotos_params(flickcurl* fc, const char* photoset_id,
                                     int privacy_filter,
                                     flickcurl_photos_list_params* list_params)
{
  const char* parameters[13][2];
  int count=0;
  flickcurl_photos_list* photos_list=NULL;
  char privacy_filter_str[2];
  const char* format=NULL;
  
  if(!photoset_id)
    return NULL;

  /* API parameters */
  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;
  if(privacy_filter >=1 && privacy_filter <= 5) {
    parameters[count][0]  = "privacy_filter";
    sprintf(privacy_filter_str, "%d", privacy_filter);
    parameters[count++][1]= privacy_filter_str;
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.getPhotos", parameters, count))
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
 * flickcurl_photosets_getPhotos:
 * @fc: flickcurl context
 * @photoset_id: The id of the photoset to return the photos for.
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level 1-5 (or <0)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 500. The maximum allowed value is 500. (or <0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or <0)
 * 
 * Get the list of photos in a set.
 *
 * See flickcurl_photosets_getPhotos_params() for details of parameters.
 *
 * Implements flickr.photosets.getPhotos (0.13)
 * 
 * Return value: list of photos or NULL on failure
 **/
flickcurl_photo**
flickcurl_photosets_getPhotos(flickcurl* fc, const char* photoset_id,
                              const char* extras, int privacy_filter,
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

  photos_list=flickcurl_photosets_getPhotos_params(fc, photoset_id,
                                                   privacy_filter,
                                                   &list_params);
  if(!photos_list)
    return NULL;

  photos=photos_list->photos; photos_list->photos=NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photosets_orderSets:
 * @fc: flickcurl context
 * @photoset_ids_array: Array of photoset IDs, ordered with the set to show first, first in the list. Any set IDs not given in the list will be set to appear at the end of the list, ordered by their IDs.
 * 
 * Set the order of photosets for the calling user.
 *
 * Implements flickr.photosets.orderSets (0.13)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photosets_orderSets(flickcurl* fc, const char** photoset_ids_array)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  char* photoset_ids;
  
  if(!photoset_ids_array)
    return 1;
  
  photoset_ids=flickcurl_array_join(photoset_ids_array, ',');
  parameters[count][0]  = "photoset_ids";
  parameters[count++][1]= photoset_ids;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.orderSets", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;
  if(photoset_ids)
    free(photoset_ids);
  
  return result;
}


/**
 * flickcurl_photosets_removePhoto:
 * @fc: flickcurl context
 * @photoset_id: The id of the photoset to remove a photo from.
 * @photo_id: The id of the photo to remove from the set.
 * 
 * Remove a photo from a photoset.
 *
 * Implements flickr.photosets.removePhoto (0.13)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photosets_removePhoto(flickcurl* fc, const char* photoset_id,
                                const char* photo_id)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  
  if(!photoset_id || !photo_id)
    return 1;

  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photosets.removePhoto", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;

  return result;
}
