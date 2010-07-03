/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * galleries-api.c - Flickr flickr.galleries.* API calls
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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
 * flickcurl_galleries_addPhoto:
 * @fc: flickcurl context
 * @gallery_id: The ID of the gallery to add a photo to as returned by flickcurl_galleries_getList() and flickcurl_galleries_getListForPhoto().
 * @photo_id: The photo ID to add to the gallery.
 * @comment_text: A short comment or story to accompany the photo (or NULL).
 * 
 * Add a photo to a gallery.
 *
 * Implements flickr.galleries.addPhoto (1.17)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_galleries_addPhoto(flickcurl* fc, const char* gallery_id,
                             const char* photo_id, const char* comment_text)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  
  if(!gallery_id || !photo_id)
    return 1;

  parameters[count][0]  = "gallery_id";
  parameters[count++][1]= gallery_id;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  if(comment_text) {
    parameters[count][0]  = "comment";
    parameters[count++][1]= comment_text;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.addPhoto", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_galleries_create:
 * @fc: flickcurl context
 * @title: The name of the gallery
 * @description: A short description for the gallery
 * @primary_photo_id: The first photo to add to your gallery (or NULL)
 * @gallery_url_p: pointer to variable to store new gallery URL (or NULL)
 * 
 * Create a new gallery for the calling user.
 *
 * Implements flickr.galleries.create (1.18)
 * 
 * Announced 2010-04-08
 * http://code.flickr.com/blog/2010/04/08/galleries-apis/
 *
 * Return value: gallery ID or NULL on failure
 **/
char*
flickcurl_galleries_create(flickcurl* fc,
                           const char* title,
                           const char* description,
                           const char* primary_photo_id,
                           char** gallery_url_p)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char* gallery_id = NULL;
  
  if(!title || !description)
    return NULL;

  parameters[count][0]  = "title";
  parameters[count++][1]= title;
  parameters[count][0]  = "description";
  parameters[count++][1]= description;
  if(primary_photo_id) {
    parameters[count][0]  = "primary_photo_id";
    parameters[count++][1]= primary_photo_id;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.create", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  gallery_id = flickcurl_xpath_eval(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/gallery/@id");
  if(gallery_url_p) {
    *gallery_url_p = flickcurl_xpath_eval(fc,
                                          xpathCtx, (const xmlChar*)"/rsp/gallery/@url");
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    gallery_id = NULL;

  return gallery_id;
}


/**
 * flickcurl_galleries_editMeta:
 * @fc: flickcurl context
 * @gallery_id: The gallery ID to update.
 * @title: The new title for the gallery.
 * @description: The new description for the gallery. (or NULL)
 * 
 * Modify the meta-data for a gallery.
 *
 * Implements flickr.galleries.editMeta (1.18)
 * 
 * Announced 2010-04-08
 * http://code.flickr.com/blog/2010/04/08/galleries-apis/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_galleries_editMeta(flickcurl* fc,
                             const char* gallery_id,
                             const char* title, const char* description)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  
  if(!gallery_id || !title)
    return 1;

  parameters[count][0]  = "gallery_id";
  parameters[count++][1]= gallery_id;
  parameters[count][0]  = "title";
  parameters[count++][1]= title;
  if(description) {
    parameters[count][0]  = "description";
    parameters[count++][1]= description;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.editMeta", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

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
 * flickcurl_galleries_editPhoto:
 * @fc: flickcurl context
 * @gallery_id: The ID of the gallery to add a photo to. Note: this is the compound ID returned in methods like flickr.galleries.getList, and flickr.galleries.getListForPhoto.
 * @photo_id: The photo ID to add to the gallery.
 * @new_comment: The updated comment the photo.
 * 
 * Edit the comment for a gallery photo.
 *
 * Implements flickr.galleries.editPhoto (1.18)
 * 
 * Announced 2010-04-08
 * http://code.flickr.com/blog/2010/04/08/galleries-apis/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_galleries_editPhoto(flickcurl* fc, const char* gallery_id,
                              const char* photo_id, const char* new_comment)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  
  if(!gallery_id || !photo_id || !new_comment)
    return 1;

  parameters[count][0]  = "gallery_id";
  parameters[count++][1]= gallery_id;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "comment";
  parameters[count++][1]= new_comment;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.editPhoto", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

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
 * flickcurl_galleries_editPhotos:
 * @fc: flickcurl context
 * @gallery_id: The id of the gallery to modify. The gallery must belong to the calling user.
 * @primary_photo_id: The id of the photo to use as the 'primary' photo for the gallery. This id must also be passed along in photo_ids list argument.
 * @photo_ids_array: Array of photo ids to include in the gallery. They will appear in the set in the order sent. This list MUST contain the primary photo id. This list of photos replaces the existing list.
 * 
 * Modify the photos in a gallery. Use this method to add, remove and re-order photos.
 *
 * Implements flickr.galleries.editPhotos (1.18)
 * 
 * Announced 2010-04-08
 * http://code.flickr.com/blog/2010/04/08/galleries-apis/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_galleries_editPhotos(flickcurl* fc, const char* gallery_id,
                               const char* primary_photo_id,
                               const char** photo_ids_array)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  char* photo_ids = NULL;
  
  if(!gallery_id || !primary_photo_id || !photo_ids_array)
    return 1;

  parameters[count][0]  = "gallery_id";
  parameters[count++][1]= gallery_id;
  parameters[count][0]  = "primary_photo_id";
  parameters[count++][1]= primary_photo_id;
  photo_ids = flickcurl_array_join(photo_ids_array, ',');
  parameters[count][0]  = "photo_ids";
  parameters[count++][1]= photo_ids;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.editPhotos", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  result = 0;
  
  tidy:
  if(photo_ids)
    free(photo_ids);

  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_galleries_getInfo:
 * @fc: flickcurl context
 * @gallery_id: The gallery ID you are requesting information for.
 * 
 * Get information for a gallery.
 *
 * Implements flickr.galleries.getInfo (1.18)
 * 
 * Announced 2010-04-08
 * http://code.flickr.com/blog/2010/04/08/galleries-apis/
 *
 * Return value: gallery or NULL on failure
 **/
flickcurl_gallery*
flickcurl_galleries_getInfo(flickcurl* fc, const char* gallery_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_gallery* gallery = NULL;
  flickcurl_gallery** galleries = NULL;
  
  if(!gallery_id)
    return NULL;

  parameters[count][0]  = "gallery_id";
  parameters[count++][1]= gallery_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.getInfo", parameters, count))
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

  galleries = flickcurl_build_galleries(fc, xpathCtx,
                                        (const xmlChar*)"/rsp/galleries/gallery",
                                        NULL);
  if(galleries) {
    gallery = galleries[0];
    galleries[0] = NULL;
    flickcurl_free_galleries(galleries);
  }
  
  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    gallery = NULL;

  return gallery;
}


/**
 * flickcurl_galleries_getList:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to get a galleries list for. If none is specified, the calling user is assumed.
 * @per_page: Number of galleries to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Return the list of galleries created by a user.
 *
 * Galleries are returned sorted from newest to oldest.
 *
 * Implements flickr.galleries.getList (1.17)
 * 
 * Return value: array of galleries or NULL on failure
 **/
flickcurl_gallery**
flickcurl_galleries_getList(flickcurl* fc, const char* user_id,
                            int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_gallery** galleries = NULL;
  char page_str[10];
  char per_page_str[10];
  
  if(!user_id)
    return NULL;

  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.getList", parameters, count))
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

  galleries = flickcurl_build_galleries(fc, xpathCtx,
                                        (const xmlChar*)"/rsp/galleries/gallery",
                                        NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    galleries = NULL;

  return galleries;
}


/**
 * flickcurl_galleries_getListForPhoto:
 * @fc: flickcurl context
 * @photo_id: The ID of the photo to fetch a list of galleries for.
 * @per_page: Number of galleries to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Return the list of galleries to which a photo has been added.
 *
 * Galleries are returned sorted by date which the photo was added to
 * the gallery.
 *
 * Implements flickr.galleries.getListForPhoto (1.17)
 * 
 * Return value: array of galleries or NULL on failure
 **/
flickcurl_gallery**
flickcurl_galleries_getListForPhoto(flickcurl* fc, const char* photo_id,
                                    int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_gallery** galleries = NULL;
  char page_str[10];
  char per_page_str[10];
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.getListForPhoto", parameters,
                       count))
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

  galleries = flickcurl_build_galleries(fc, xpathCtx,
                                        (const xmlChar*)"/rsp/galleries/gallery",
                                        NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    galleries = NULL;

  return galleries;
}


/**
 * flickcurl_galleries_getPhotos_params:
 * @fc: flickcurl context
 * @gallery_id: The ID of the gallery of photos to return
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 *
 * Return the list of photos for a gallery
 * 
 * Currently supported extras fields are: description, license,
 * date_upload, date_taken, owner_name, icon_server, original_format,
 * last_update, geo, tags, machine_tags, o_dims, views, media,
 * path_alias, url_sq, url_t, url_s, url_m, url_o
 *
 * Return value: list of people public photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_galleries_getPhotos_params(flickcurl* fc, const char* gallery_id,
                                     flickcurl_photos_list_params* list_params)
{
  const char* parameters[11][2];
  int count = 0;
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  
  if(!gallery_id)
    return NULL;

  /* API parameters */
  parameters[count][0]  = "gallery_id";
  parameters[count++][1]= gallery_id;

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.getPhotos", parameters, count))
    goto tidy;

  photos_list = flickcurl_invoke_photos_list(fc,
                                             (const xmlChar*)"/rsp/gallery",
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
 * flickcurl_galleries_getPhotos:
 * @fc: flickcurl context
 * @gallery_id: The ID of the gallery of photos to return
 * @extras: A comma-delimited list of extra information to fetch for each returned record.
 * @per_page: Number of photos to return per page (default 100, max 500)
 * @page: The page of results to return (default 1)
 * 
 * Return the list of photos for a gallery
 *
 * See flickcurl_galleries_getPhotos_params() for details of @extras.
 *
 * Implements flickr.galleries.getPhotos (1.18)
 * 
 * Announced 2010-04-08
 * http://code.flickr.com/blog/2010/04/08/galleries-apis/
 *
 * Return value: list of photos or NULL on failure
 **/
flickcurl_photo**
flickcurl_galleries_getPhotos(flickcurl* fc, const char* gallery_id,
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

  photos_list = flickcurl_galleries_getPhotos_params(fc, gallery_id,
                                                     &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}
