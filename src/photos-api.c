/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-api.c - Flickr flickr.photos.* API calls
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
 * flickcurl_photos_addTags:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * @tags: tags to add as a space-separated list
 *
 * Add tags to a photo.
 *
 * Implements flickr.photos.addTags (0.9)
 *
 * Return value: non-0 on failure
 */
int
flickcurl_photos_addTags(flickcurl* fc, const char* photo_id, const char* tags)
{
  xmlDocPtr doc = NULL;
  
  flickcurl_init_params(fc, 1);

  if(!photo_id || !tags)
    return 1;
  
  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_add_param(fc, "tags", tags);
    
  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.addTags"))
    goto tidy;
  
  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

 tidy:

  return fc->failed;
}


/**
 * flickcurl_photos_delete:
 * @fc: flickcurl context
 * @photo_id: photo ID
 *
 * Delete a photo.
 *
 * Implements flickr.photos.delete (0.9)
 *
 * Return value: non-0 on failure
 */
int
flickcurl_photos_delete(flickcurl* fc, const char* photo_id)
{
  xmlDocPtr doc = NULL;
  int result = 1;
  
  flickcurl_init_params(fc, 1);

  if(!photo_id)
    return 1;
  
  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.delete"))
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
 * flickcurl_photos_getAllContexts:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * 
 * Get all visible sets and pools the photo belongs to.
 *
 * Implements flickr.photos.getAllContexts (0.7)
 *
 * Returns an array of size 3 [prev, next, NULL] flickcurl_context*
 * or NULL on error
 * 
 * Return value: prev, next contexts or NULL
 **/
flickcurl_context**
flickcurl_photos_getAllContexts(flickcurl* fc, const char* photo_id)
{
  xmlDocPtr doc = NULL;
  flickcurl_context** contexts = NULL;
  
  flickcurl_init_params(fc, 0);

  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getAllContexts"))
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
 * flickcurl_photos_getContactsPhotos_params:
 * @fc: flickcurl context
 * @contact_count: Number of photos to return (Default 10, maximum 50)
 * @just_friends: Set to non-0 to only show photos from friends and family (excluding regular contacts).
 * @single_photo: Set to non-0 to only fetch one photo (the latest) per contact, instead of all photos in chronological order.
 * @include_self: Set to non-0 to include photos from the calling user.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Fetch a list of recent photos from the calling users' contacts.
 *
 * Currently supported extras fields are: license, date_upload,
 * date_taken, owner_name, icon_server, original_format, last_update
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_photos_getContactsPhotos_params(flickcurl* fc, 
                                          int contact_count, int just_friends,
                                          int single_photo, int include_self,
                                          flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  char true_s[2] = "1";
  const char* format = NULL;

  flickcurl_init_params(fc, 0);

  /* API parameters */
  if(contact_count > 1) {
    char count_s[20];
    sprintf(count_s, "%d", contact_count);
    flickcurl_add_param(fc, "count", count_s);
  }
  if(just_friends) {
    flickcurl_add_param(fc, "just_friends", true_s);
  }
  if(single_photo) {
    flickcurl_add_param(fc, "single_photo", true_s);
  }
  if(include_self) {
    flickcurl_add_param(fc, "include_self", true_s);
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getContactsPhotos"))
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
 * flickcurl_photos_getContactsPhotos:
 * @fc: flickcurl context
 * @contact_count: Number of photos to return (Default 10, maximum 50)
 * @just_friends: Set to non-0 to only show photos from friends and family (excluding regular contacts).
 * @single_photo: Set to non-0 to only fetch one photo (the latest) per contact, instead of all photos in chronological order.
 * @include_self: Set to non-0 to include photos from the calling user.
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * 
 * Fetch a list of recent photos from the calling users' contacts.
 *
 * See flickcurl_photos_getContactsPhotos() for details of parameters.
 *
 * Implements flickr.photos.getContactsPhotos (0.11)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_photos_getContactsPhotos(flickcurl* fc, 
                                   int contact_count, int just_friends,
                                   int single_photo, int include_self,
                                   const char* extras)
{
  flickcurl_photos_list_params list_params;
  flickcurl_photos_list* photos_list;
  flickcurl_photo** photos;
  
  memset(&list_params, '\0', sizeof(list_params));
  list_params.format   = NULL;
  list_params.extras   = extras;
  list_params.per_page = -1;
  list_params.page     = -1;

  photos_list = flickcurl_photos_getContactsPhotos_params(fc, contact_count,
                                                        just_friends,
                                                        single_photo, 
                                                        include_self,
                                                        &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_getContactsPublicPhotos_params:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to fetch photos for.
 * @photo_count: Number of photos to return (Default 10, maximum 50)
 * @just_friends: Set to non-0 to only show photos from friends and family (excluding regular contacts)
 * @single_photo: Set to non-0 to only fetch one photo (the latest) per contact, instead of all photos in chronological order.
 * @include_self: Set to non-0 to include photos from the user specified by user_id.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Fetch a list of recent public photos from a users' contacts.
 *
 * Currently supported extras fields are: license,
 * date_upload, date_taken, owner_name, icon_server, original_format,
 * last_update.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: a list of photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_photos_getContactsPublicPhotos_params(flickcurl* fc,
                                                const char* user_id,
                                                int photo_count,
                                                int just_friends, 
                                                int single_photo,
                                                int include_self,
                                                flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  char true_s[2] = "1";
  char photo_count_s[10];
  const char* format = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!user_id)
    return NULL;

  /* API parameters */
  flickcurl_add_param(fc, "user_id", user_id);
  sprintf(photo_count_s, "%d", photo_count);
  flickcurl_add_param(fc, "count", photo_count_s);
  if(just_friends) {
    flickcurl_add_param(fc, "just_friends", true_s);
  }
  if(single_photo) {
    flickcurl_add_param(fc, "single_photo", true_s);
  }
  if(include_self) {
    flickcurl_add_param(fc, "include_self", true_s);
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);
  
  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getContactsPublicPhotos"))
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
 * flickcurl_photos_getContactsPublicPhotos:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to fetch photos for.
 * @photo_count: Number of photos to return (Default 10, maximum 50)
 * @just_friends: Set to non-0 to only show photos from friends and family (excluding regular contacts)
 * @single_photo: Set to non-0 to only fetch one photo (the latest) per contact, instead of all photos in chronological order.
 * @include_self: Set to non-0 to include photos from the user specified by user_id.
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * 
 * Fetch a list of recent public photos from a users' contacts.
 *
 * See flickcurl_photos_getContactsPublicPhotos() for details of
 * parameters.
 *
 * Implements flickr.photos.getContactsPublicPhotos (0.12)
 * 
 * Return value: a list of photos or NULL on failure
 **/
flickcurl_photo** 
flickcurl_photos_getContactsPublicPhotos(flickcurl* fc, const char* user_id,
                                         int photo_count, int just_friends, 
                                         int single_photo, int include_self,
                                         const char* extras)
{
  flickcurl_photos_list_params list_params;
  flickcurl_photos_list* photos_list;
  flickcurl_photo** photos;
  
  memset(&list_params, '\0', sizeof(list_params));
  list_params.format   = NULL;
  list_params.extras   = extras;
  list_params.per_page = -1;
  list_params.page     = -1;

  photos_list = flickcurl_photos_getContactsPublicPhotos_params(fc, user_id,
                                                              photo_count,
                                                              just_friends, 
                                                              single_photo,
                                                              include_self,
                                                              &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_getContext:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * 
 * Get next and previous photos for a photo in a photostream.
 *
 * Implements flickr.photos.getContext (0.7)
 *
 * Returns an array of size 3 [prev, next, NULL] flickcurl_context*
 * or NULL on error
 * 
 * Return value: prev, next contexts or NULL
 **/
flickcurl_context**
flickcurl_photos_getContext(flickcurl* fc, const char* photo_id)
{
  xmlDocPtr doc = NULL;
  flickcurl_context** contexts = NULL;
  
  flickcurl_init_params(fc, 0);

  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getContext"))
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


static int**
flickcurl_build_photocounts(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                            const xmlChar* xpathExpr, int* photocount_count_p)
{
  int** photocounts = NULL;
  int nodes_count;
  int photocount_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  const int row_size = 3;
  
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

  photocounts = (int**)calloc(nodes_count+1, sizeof(int*));
  
  for(i = 0, photocount_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    int* row;
    int j;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    row = (int*)calloc(row_size, sizeof(int));
    for(j = 0; j < row_size; j++)
      row[j]= -1;
    
    for(attr = node->properties; attr; attr = attr->next) {
      size_t attr_len = strlen((const char*)attr->children->content);
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(attr_len + 1);
      memcpy(attr_value, attr->children->content, attr_len + 1);
      
      if(!strcmp(attr_name, "count")) {
        row[0] = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "fromdate")) {
        row[1] = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "todate")) {
        row[2] = atoi(attr_value);
        free(attr_value);
      } else
        free(attr_value);
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "photocount: count %d  fromdate %d  todate %d\n",
            row[0], row[1], row[2]);
#endif
    
    photocounts[photocount_count++] = row;
  } /* for nodes */

  if(photocount_count_p)
    *photocount_count_p = photocount_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return photocounts;
}


/**
 * flickcurl_photos_getCounts:
 * @fc: flickcurl context
 * @dates_array: A comma delimited list of unix timestamps, denoting the periods to return counts for. They should be specified smallest first. (or NULL)
 * @taken_dates_array: A comma delimited list of mysql datetimes, denoting the periods to return counts for. They should be specified mallest first. (or NULL)
 * 
 * Gets a list of photo counts for the given date ranges for the calling user.
 *
 * Implements flickr.photos.getCounts (0.13)
 * 
 * The return array fields are:
 *   [0] count
 *   [1] fromdate as unixtime
 *   [2] todate as unixtime
 *
 * Return value: array of int[3] or NULL on failure
 */
int**
flickcurl_photos_getCounts(flickcurl* fc,
                           const char** dates_array,
                           const char** taken_dates_array)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int** counts = NULL;
  char* dates = NULL;
  char* taken_dates = NULL;
  
  flickcurl_init_params(fc, 0);

  /* one must be not empty */
  if(!dates_array && !taken_dates_array)
    return NULL;
  
  if(dates_array) {
    dates = flickcurl_array_join(dates_array, ',');
    flickcurl_add_param(fc, "dates", dates);
  }
  if(taken_dates_array) {
    taken_dates = flickcurl_array_join(taken_dates_array, ',');
    flickcurl_add_param(fc, "taken_dates", taken_dates);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getCounts"))
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

  counts = flickcurl_build_photocounts(fc, xpathCtx,
                                     (const xmlChar*)"/rsp/photocounts/photocount",
                                     NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(counts)
      free(counts);
    counts = NULL;
  }

  if(dates)
    free(dates);
  
  if(taken_dates)
    free(taken_dates);
  
  return counts;
}


/**
 * flickcurl_photos_getExif:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to fetch information for.
 * @secret: The secret for the photo (or NULL)
 *   If the correct secret is passed then permissions checking is
 *   skipped. This enables the 'sharing' of individual photos by
 *   passing around the id and secret.
 *
 * Retrieves a list of EXIF/TIFF/GPS tags for a given photo.
 *
 * Implements flickr.photos.getExif (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_exif**
flickcurl_photos_getExif(flickcurl* fc, const char* photo_id,
                         const char* secret)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_exif** exifs = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!photo_id)
    return NULL;

  flickcurl_add_param(fc, "photo_id", photo_id);
  if(secret) {
    flickcurl_add_param(fc, "secret", secret);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getExif"))
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

  exifs = flickcurl_build_exifs(fc, xpathCtx,
                              (const xmlChar*)"/rsp/photo/exif", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(exifs)
      flickcurl_free_exifs(exifs);
    exifs = NULL;
  }

  return exifs;
}


/**
 * flickcurl_photos_getFavorites:
 * @fc: flickcurl context
 * @photo_id: The ID of the photo to fetch the favoriters list for.
 * @page: The page of results to return (default 1)
 * @per_page: Number of users to return per page (default 10, max 50)
 * 
 * Returns the list of people who have favorited a given photo.
 *
 * Implements flickr.photos.getFavorites (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_person**
flickcurl_photos_getFavorites(flickcurl* fc, const char* photo_id,
                              int page, int per_page)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_person** persons = NULL;
  char per_page_s[4];
  char page_s[4];
  
  flickcurl_init_params(fc, 0);

  if(!photo_id)
    return NULL;

  flickcurl_add_param(fc, "photo_id", photo_id);
  sprintf(page_s, "%d", page);
  flickcurl_add_param(fc, "page", page_s);
  sprintf(per_page_s, "%d", per_page);
  flickcurl_add_param(fc, "per_page", per_page_s);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getFavorites"))
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

  persons = flickcurl_build_persons(fc, xpathCtx,
                                  (const xmlChar*)"/rsp/photo/person", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(persons)
      flickcurl_free_persons(persons);
    persons = NULL;
  }

  return persons;
}


/**
 * flickcurl_photos_getInfo2:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * @secret: secret (or NULL)
 * 
 * Get information about a photo
 *
 * Implements flickr.photos.getInfo (0.5)
 * 
 * Return value: #flickcurl_photo or NULL on failure
 **/
flickcurl_photo*
flickcurl_photos_getInfo2(flickcurl* fc, const char* photo_id,
                          const char* secret)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_photo* photo = NULL;
  
  flickcurl_init_params(fc, 0);

  flickcurl_add_param(fc, "photo_id", photo_id);

  if(secret)
    flickcurl_add_param(fc, "secret", secret);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getInfo"))
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

  photo = flickcurl_build_photo(fc, xpathCtx);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(photo)
      flickcurl_free_photo(photo);
    photo = NULL;
  }

  return photo;
}


/**
 * flickcurl_photos_getInfo:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * 
 * Get information about a photo
 *
 * Deprecated for flickcurl_photos_getInfo2() that allows passing the
 * optional secret.
 * 
 * Return value: #flickcurl_photo or NULL on failure
 **/
flickcurl_photo*
flickcurl_photos_getInfo(flickcurl* fc, const char* photo_id)
{
  return flickcurl_photos_getInfo2(fc, photo_id, NULL);
}


static flickcurl_photos_list*
flickcurl_get_photoslist_params(flickcurl* fc, 
                                const char* method,
                                int min_upload_date, int max_upload_date,
                                const char* min_taken_date,
                                const char* max_taken_date,
                                int privacy_filter, 
                                flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  char min_upload_date_s[20];
  char max_upload_date_s[20];
  char privacy_filter_s[20];
  const char* format = NULL;

  flickcurl_init_params(fc, 0);

  /* API parameters */
  if(min_upload_date > 0) {
    sprintf(min_upload_date_s, "%d", min_upload_date);
    flickcurl_add_param(fc, "min_upload_date", min_upload_date_s);
  }
  if(max_upload_date > 0) {
    sprintf(max_upload_date_s, "%d", max_upload_date);
    flickcurl_add_param(fc, "max_upload_date", max_upload_date_s);
  }
  if(min_taken_date) {
    flickcurl_add_param(fc, "min_taken_date", min_taken_date);
  }
  if(max_taken_date) {
    flickcurl_add_param(fc, "max_taken_date", max_taken_date);
  }
  if(privacy_filter >= 1 && privacy_filter <= 5) {
    sprintf(privacy_filter_s, "%d", privacy_filter);
    flickcurl_add_param(fc, "privacy_filter", privacy_filter_s);
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, method))
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
 * flickcurl_photos_getNotInSet_params:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date.
 * @max_upload_date: Maximum upload date.
 * @min_taken_date: Minimum taken date (or NULL).
 * @max_taken_date: Maximum taken date (or NULL).
 * @privacy_filter: Return photos only matching a certain privacy level.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of your photos that are not part of any sets.
 *
 * Currently supported extras fields are: license, date_upload, date_taken,
 *  owner_name, icon_server, original_format, last_update, geo, tags,
 *  machine_tags.
 *
 * Return value: a list of photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_photos_getNotInSet_params(flickcurl* fc, 
                                    int min_upload_date, int max_upload_date,
                                    const char* min_taken_date,
                                    const char* max_taken_date,
                                    int privacy_filter,
                                    flickcurl_photos_list_params* list_params)
{
  return flickcurl_get_photoslist_params(fc, "flickr.photos.getNotInSet",
                                         min_upload_date, max_upload_date,
                                         min_taken_date, max_taken_date,
                                         privacy_filter, list_params);
}


/**
 * flickcurl_photos_getNotInSet:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date.
 * @max_upload_date: Maximum upload date.
 * @min_taken_date: Minimum taken date (or NULL).
 * @max_taken_date: Maximum taken date (or NULL).
 * @privacy_filter: Return photos only matching a certain privacy level.
 * Valid privacy values are: 1 public, 2 friends, 3 family, 4 friends and
 * family, 5 private
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * @per_page: Number of photos to return per page (default 100, max 500).
 * @page: The page of results to return (default 1).
 * 
 * Returns a list of your photos that are not part of any sets.
 *
 * See flickcurl_photos_getNotInSet_params() for details of parameters.
 *
 * Implements flickr.photos.getNotInSet (0.12)
 * 
 * Return value: a list of photos or NULL on failure
 **/
flickcurl_photo**
flickcurl_photos_getNotInSet(flickcurl* fc, 
                             int min_upload_date, int max_upload_date,
                             const char* min_taken_date,
                             const char* max_taken_date,
                             int privacy_filter, const char* extras,
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


  photos_list = flickcurl_get_photoslist_params(fc, "flickr.photos.getNotInSet",
                                              min_upload_date, max_upload_date,
                                              min_taken_date, max_taken_date,
                                              privacy_filter, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_getPerms:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to get permissions for.
 * 
 * Get permissions for a photo.
 *
 * Implements flickr.photos.getPerms (0.11)
 * 
 * Return value: permissions or NULL on failure
 **/
flickcurl_perms*
flickcurl_photos_getPerms(flickcurl* fc, const char* photo_id)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_perms* perms = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!photo_id)
    return NULL;

  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getPerms"))
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

  perms = flickcurl_build_perms(fc, xpathCtx, (const xmlChar*)"/rsp/perms");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(perms)
      flickcurl_free_perms(perms);
    perms = NULL;
  }

  return perms;
}


/**
 * flickcurl_photos_getRecent_params:
 * @fc: flickcurl context
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of the latest public photos uploaded to flickr.
 *
 * Currently supported extras fields are: license, date_upload, date_taken,
 * owner_name, icon_server, original_format, last_update, geo, tags,
 * machine_tags.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_photos_getRecent_params(flickcurl* fc,
                                  flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;

  flickcurl_init_params(fc, 0);

  /* No API parameters */

  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getRecent"))
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
 * flickcurl_photos_getRecent:
 * @fc: flickcurl context
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * @per_page: Number of photos to return per page (default 100, max 500)
 * @page: The page of results to return (default 1)
 * 
 * Returns a list of the latest public photos uploaded to flickr.
 *
 * See flickcurl_photos_getRecent_params() for details of parameters.
 *
 * Implements flickr.photos.getRecent (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_photos_getRecent(flickcurl* fc, const char* extras,
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

  photos_list = flickcurl_photos_getRecent_params(fc, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_getSizes:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to fetch size information for.
 * 
 * Returns the available sizes for a photo.  The calling user must have permission to view the photo.
 *
 * Implements flickr.photos.getSizes (0.13)
 * 
 * Return value: non-0 on failure
 */
flickcurl_size**
flickcurl_photos_getSizes(flickcurl* fc, const char* photo_id)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_size** sizes = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!photo_id)
    return NULL;

  flickcurl_add_param(fc, "photo_id", photo_id);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.getSizes"))
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

  sizes = flickcurl_build_sizes(fc, xpathCtx, (const xmlChar*)"/rsp/sizes/size",
                              NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(sizes)
      flickcurl_free_sizes(sizes);
    sizes = NULL;
  }

  return sizes;
}


/**
 * flickcurl_photos_getUntagged_params:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date.
 * @max_upload_date: Maximum upload date.
 * @min_taken_date: Minimum taken date (or NULL)
 * @max_taken_date: Maximum taken date (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level.
 * Valid privacy values are: 1 public, 2 friends, 3 family, 4 friends and
 * family, 5 private
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of your photos with no tags.
 *
 * Currently supported extras fields are: license, date_upload, date_taken,
 * owner_name, icon_server, original_format, last_update, geo, tags,
 * machine_tags.
 *
 * Return value: a list of photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_photos_getUntagged_params(flickcurl* fc,
                                    int min_upload_date, int max_upload_date,
                                    const char* min_taken_date,
                                    const char* max_taken_date,
                                    int privacy_filter,
                                    flickcurl_photos_list_params* list_params)
{
  return flickcurl_get_photoslist_params(fc, "flickr.photos.getUntagged",
                                         min_upload_date, max_upload_date,
                                         min_taken_date, max_taken_date,
                                         privacy_filter, list_params);
}


/**
 * flickcurl_photos_getUntagged:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date.
 * @max_upload_date: Maximum upload date.
 * @min_taken_date: Minimum taken date (or NULL)
 * @max_taken_date: Maximum taken date (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level.
 * Valid privacy values are: 1 public, 2 friends, 3 family, 4 friends and
 * family, 5 private
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * @per_page: Number of photos to return per page (default 100, max 500).
 * @page: The page of results to return (default 1).
 * 
 * Returns a list of your photos with no tags.
 *
 * See flickcurl_photos_getUntagged_params() for details of parameters.
 *
 * Implements flickr.photos.getUntagged (0.12)
 * 
 * Return value: a list of photos or NULL on failure
 **/
flickcurl_photo**
flickcurl_photos_getUntagged(flickcurl* fc,
                             int min_upload_date, int max_upload_date,
                             const char* min_taken_date,
                             const char* max_taken_date,
                             int privacy_filter, const char* extras,
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


  photos_list = flickcurl_get_photoslist_params(fc, "flickr.photos.getUntagged",
                                              min_upload_date, max_upload_date,
                                              min_taken_date, max_taken_date,
                                              privacy_filter, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_getWithGeoData_params:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level. Valid values are:
1 public photos;
2 private photos visible to friends;
3 private photos visible to family;
4 private photos visible to friends and family;
5 completely private photos. (or NULL)
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of your geo-tagged photos.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_photos_getWithGeoData_params(flickcurl* fc,
                                       int min_upload_date, int max_upload_date,
                                       const char* min_taken_date,
                                       const char* max_taken_date,
                                       int privacy_filter,
                                       flickcurl_photos_list_params* list_params)
{
  return flickcurl_get_photoslist_params(fc, "flickr.photos.getWithGeoData",
                                         min_upload_date, max_upload_date,
                                         min_taken_date, max_taken_date,
                                         privacy_filter, list_params);
}


/**
 * flickcurl_photos_getWithGeoData:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level. Valid values are:
1 public photos;
2 private photos visible to friends;
3 private photos visible to family;
4 private photos visible to friends and family;
5 completely private photos. (or NULL)
 * @extras: A comma-delimited list of extra information to fetch for each returned record.
 * @per_page: Number of photos to return per page.
 * @page: The page of results to return.
 * 
 * Returns a list of your geo-tagged photos.
 *
 * See flickcurl_photos_getWithGeoData_params() for details of parameters.
 *
 * Implements flickr.photos.getWithGeoData (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_photos_getWithGeoData(flickcurl* fc,
                                int min_upload_date, int max_upload_date,
                                const char* min_taken_date,
                                const char* max_taken_date,
                                int privacy_filter, const char* extras,
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

  photos_list = flickcurl_photos_getWithGeoData_params(fc, min_upload_date,
                                                     max_upload_date,
                                                     min_taken_date,
                                                     max_taken_date,
                                                     privacy_filter,
                                                     &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_getWithoutGeoData_params:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level. Valid values are:
1 public photos;
2 private photos visible to friends;
3 private photos visible to family;
4 private photos visible to friends and family;
5 completely private photos.
 (or NULL)
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of your photos which haven't been geo-tagged.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_photos_getWithoutGeoData_params(flickcurl* fc,
                                          int min_upload_date, int max_upload_date,
                                          const char* min_taken_date,
                                          const char* max_taken_date,
                                          int privacy_filter,
                                          flickcurl_photos_list_params* list_params)
{
  return flickcurl_get_photoslist_params(fc, "flickr.photos.getWithoutGeoData",
                                         min_upload_date, max_upload_date,
                                         min_taken_date, max_taken_date,
                                         privacy_filter, list_params);
}


/**
 * flickcurl_photos_getWithoutGeoData:
 * @fc: flickcurl context
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @privacy_filter: Return photos only matching a certain privacy level. Valid values are:
1 public photos;
2 private photos visible to friends;
3 private photos visible to family;
4 private photos visible to friends and family;
5 completely private photos.
 (or NULL)
 * @extras: A comma-delimited list of extra information to fetch for each returned record.
 * @per_page: Number of photos to return per page.
 * @page: The page of results to return.
 * 
 * Returns a list of your photos which haven't been geo-tagged.
 *
 * See flickcurl_photos_getWithoutGeoData_params() for details of parameters.
 *
 * Implements flickr.photos.getWithoutGeoData (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_photos_getWithoutGeoData(flickcurl* fc,
                                   int min_upload_date, int max_upload_date,
                                   const char* min_taken_date,
                                   const char* max_taken_date,
                                   int privacy_filter, const char* extras,
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

  photos_list = flickcurl_photos_getWithoutGeoData_params(fc, min_upload_date,
                                                        max_upload_date,
                                                        min_taken_date,
                                                        max_taken_date,
                                                        privacy_filter,
                                                        &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_recentlyUpdated_params:
 * @fc: flickcurl context
 * @min_date: A Unix timestamp indicating the date from which modifications should be compared.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Return a list of your photos that have been recently created or which have been recently modified
 * 
 * Recently modified may mean that the photo's metadata (title,
 * description, tags) may have been changed or a comment has been
 * added (or just modified somehow :-)
 *
 * Currently supported extra fields are: license, date_upload, date_taken,
 * owner_name, icon_server, original_format, last_update, geo, tags,
 * machine_tags.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_photos_recentlyUpdated_params(flickcurl* fc, int min_date,
                                         flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  char min_date_s[20];
  const char* format = NULL;
  
  flickcurl_init_params(fc, 0);

  if(min_date <= 0)
    return NULL;

  /* API parameters */
  if(min_date >0) {
    sprintf(min_date_s, "%d", min_date);
    flickcurl_add_param(fc, "min_date", min_date_s);
  }
  
  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.recentlyUpdated"))
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
 * flickcurl_photos_recentlyUpdated:
 * @fc: flickcurl context
 * @min_date: A Unix timestamp indicating the date from which modifications should be compared.
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * @per_page: Number of photos to return per page (default 100, max 500)
 * @page: The page of results to return (default 1)
 * 
 * Return a list of your photos that have been recently created or which have been recently modified
 * 
 * See flickcurl_photos_recentlyUpdated() for details of parameters.
 *
 * Implements flickr.photos.recentlyUpdated (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_photos_recentlyUpdated(flickcurl* fc, int min_date,
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

  photos_list = flickcurl_photos_recentlyUpdated_params(fc, min_date,
                                                      &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_removeTag:
 * @fc: flickcurl context
 * @tag_id: tag ID to remove from the photo
 *
 * Remove a tag from a photo.
 *
 * The @tag_id is returned such as from flickcurl_photos_getInfo()
 *
 * Implements flickr.photos.removeTag (0.9)
 *
 * Return value: non-0 on failure
 */
int
flickcurl_photos_removeTag(flickcurl* fc, const char* tag_id)
{
  xmlDocPtr doc = NULL;
  
  flickcurl_init_params(fc, 1);

  if(!tag_id)
    return 1;
  
  flickcurl_add_param(fc, "tag_id", tag_id);
    
  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.removeTag"))
    goto tidy;
  
  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

 tidy:

  return fc->failed;
}


/**
 * flickcurl_photos_search_params:
 * @fc: flickcurl context
 * @params: #flickcurl_search_params search parameters
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Return a photos list or raw content matching some criteria.
 * 
 * Only photos visible to the calling user will be returned. To
 * return private or semi-private photos, the caller must be
 * authenticated with 'read' permissions, and have permission to view
 * the photos. Unauthenticated calls will only return public photos.
 *
 * Ensure the @params object is propertly initialized to zeros/NULLs
 * or use flickcurl_search_params_init() to initialize this.
 *
 * Flickcurl 1.15; Added in_gallery search parameter as announced
 * 2010-04-08 http://code.flickr.com/blog/2010/04/08/galleries-apis/
 *
 * Flickcurl 1.14: Added geo_context and is_commons search parameters
 * to #flickcurl_search_params.  Added more extras to docs.
 *
 * Flickcurl 1.6: Added @list_params beyond flickcurl_photos_search()
 * to allow returning raw content if @list_params is present and
 * field @format is not NULL as announced 2008-08-25
 * http://code.flickr.com/blog/2008/08/25/api-responses-as-feeds/
 *
 * NOTE: The @params fields: extras, per_page and page are ignored -
 * the values are taken from the @list_params fields.
 *
 * Flickcurl 1.0: Added place_id for places API as announced 2008-01-11
 * http://tech.groups.yahoo.com/group/yws-flickr/message/3688
 *
 * Optional parameter "media" that defaults to "all" but can also be
 * set to "photos" or "videos" to filter results by media type.
 * API addition 2008-04-07.
 *
 * Optional parameter "has_geo" for any photo that has been geotagged.
 * As announced 2008-06-27
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4146
 *
 * Optional parameters "lat", "lon", "radius" and "radius_units" added
 * for doing radial geo queries from point (lat, lon) within
 * radius/radius_units.  radius_units default is "km".
 * As announced 2008-06-27
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4146
 *
 * (Experimental) Optional parameter "contacts" requires requires
 * that the "user_id" field also is set.  Valid values are "all" or
 * "ff" for just friends and family.
 * As announced 2008-06-30
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4162
 * 
 * Return value: a photos list or NULL
 **/
flickcurl_photos_list*
flickcurl_photos_search_params(flickcurl* fc,
                               flickcurl_search_params* params,
                               flickcurl_photos_list_params* list_params)
{
  flickcurl_photos_list* photos_list = NULL;
  char min_upload_date_s[15];
  char max_upload_date_s[15];
  char accuracy_s[3];
  char safe_search_s[2];
  char content_type_s[2];
  char lat_s[32];
  char lon_s[32];
  char radius_s[32];
  char woe_id_s[32];
  char geo_context_s[2];
  const char* format = NULL;
  
  flickcurl_init_params(fc, 0);

  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN_VALUE(params, flickcurl_search_params, NULL);
  
  /* Search parameters */
  if(params->user_id)
    flickcurl_add_param(fc, "user_id", params->user_id);
  if(params->tags)
    flickcurl_add_param(fc, "tags", params->tags);
  if(params->tag_mode)
    flickcurl_add_param(fc, "tag_mode", params->tag_mode);
  if(params->text)
    flickcurl_add_param(fc, "text", params->text);
  if(params->min_upload_date) {
    sprintf(min_upload_date_s, "%d", params->min_upload_date);
    flickcurl_add_param(fc, "min_upload_date", min_upload_date_s);
  }
  if(params->max_upload_date) {
    sprintf(max_upload_date_s, "%d", params->max_upload_date);
    flickcurl_add_param(fc, "max_upload_date", max_upload_date_s);
  }
  if(params->min_taken_date)
    flickcurl_add_param(fc, "min_taken_date", params->min_taken_date);
  if(params->max_taken_date)
    flickcurl_add_param(fc, "max_taken_date", params->max_taken_date);
  if(params->license)
    flickcurl_add_param(fc, "license", params->license);
  if(params->sort)
    flickcurl_add_param(fc, "sort", params->sort);
  if(params->privacy_filter)
    flickcurl_add_param(fc, "privacy_filter", params->privacy_filter);
  if(params->bbox)
    flickcurl_add_param(fc, "bbox", params->bbox);
  if(params->accuracy) {
    if(params->accuracy >= 0 && params->accuracy <= 16) {
      sprintf(accuracy_s, "%d", params->accuracy);
      flickcurl_add_param(fc, "accuracy", accuracy_s);
    }
  }
  if(params->safe_search) {
    if(params->safe_search >= 0 && params->safe_search <= 3) {
      sprintf(safe_search_s, "%d", params->safe_search);
      flickcurl_add_param(fc, "safe_search", safe_search_s);
    }
  }
  if(params->content_type) {
    if(params->content_type >= 0 && params->content_type <= 4) {
      sprintf(content_type_s, "%d", params->content_type);
      flickcurl_add_param(fc, "content_type", content_type_s);
    }
  }
  if(params->machine_tags)
    flickcurl_add_param(fc, "machine_tags", params->machine_tags);
  if(params->machine_tag_mode)
    flickcurl_add_param(fc, "machine_tag_mode", params->machine_tag_mode);
  if(params->group_id)
    flickcurl_add_param(fc, "group_id", params->group_id);
  if(params->place_id)
    flickcurl_add_param(fc, "place_id", params->place_id);
  if(params->media)
    flickcurl_add_param(fc, "media", params->media);
  if(params->has_geo)
    flickcurl_add_param(fc, "has_geo", "1");
  if(params->geo_context > 0 && params->geo_context < 3) {
    sprintf(geo_context_s, "%d", params->geo_context);
    flickcurl_add_param(fc, "geo_context", geo_context_s);
  }
  if(params->radius) {
    if(params->lat) {
      sprintf(lat_s, "%f", params->lat);
      flickcurl_add_param(fc, "lat", lat_s);
    }
    if(params->lon) {
      sprintf(lon_s, "%f", params->lon);
      flickcurl_add_param(fc, "lon", lon_s);
    }

    if(params->radius) {
      sprintf(radius_s, "%f", params->radius);
      flickcurl_add_param(fc, "radius", radius_s);
    
      if(params->radius_units) {
        flickcurl_add_param(fc, "radius_units", params->radius_units);
      }
    }
  }
  if(params->contacts && params->user_id) {
    flickcurl_add_param(fc, "contacts", params->contacts);
  }
  if(params->woe_id > 0) {
    sprintf(woe_id_s, "%d", params->woe_id);
    flickcurl_add_param(fc, "woe_id", woe_id_s);
  }
  if(params->is_commons)
    flickcurl_add_param(fc, "is_commons", "");
  if(params->in_gallery)
    flickcurl_add_param(fc, "in_gallery", "");

  /* Photos List parameters */
  flickcurl_append_photos_list_params(fc, list_params, &format);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.search"))
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
 * flickcurl_photos_search:
 * @fc: flickcurl context
 * @params: #flickcurl_search_params search parameters
 * 
 * Return a list of photos matching some criteria.
 * 
 * Only photos visible to the calling user will be returned. To
 * return private or semi-private photos, the caller must be
 * authenticated with 'read' permissions, and have permission to view
 * the photos. Unauthenticated calls will only return public photos.
 *
 * Implements flickr.photos.search (0.11)
 *
 * Flickcurl 1.0: Added place_id for places API as announced 2008-01-11
 * http://tech.groups.yahoo.com/group/yws-flickr/message/3688
 *
 * See flickcurl_photos_search_params() for details on the the search
 * parameters.
 *
 * Ensure the @params object is propertly initialized to zeros/NULLs
 * or use flickcurl_search_params_init() to initialize this.
 * 
 * Return value: an array of #flickcurl_photo pointers (may be length 0) or NULL on failure
 **/
flickcurl_photo**
flickcurl_photos_search(flickcurl* fc, flickcurl_search_params* params)
{
  flickcurl_photos_list_params list_params;
  flickcurl_photos_list* photos_list;
  flickcurl_photo** photos;
  
  memset(&list_params, '\0', sizeof(list_params));
  list_params.format   = NULL;
  list_params.extras   = params->extras;
  list_params.per_page = params->per_page;
  list_params.page     = params->page;

  photos_list = flickcurl_photos_search_params(fc, params, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_setContentType:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to set the adultness of.
 * @content_type: The content type of the photo: 1 for Photo, 2 for Screenshot, and 3 for Other.
 * 
 * Set the content type of a photo.
 *
 * Implements flickr.photos.setContentType (0.11)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_setContentType(flickcurl* fc, const char* photo_id,
                                int content_type)
{
  xmlDocPtr doc = NULL;
  int result = 1;
  char content_type_str[2];

  flickcurl_init_params(fc, 1);

  if(!photo_id || !content_type)
    return 1;

  if(content_type <1 || content_type > 3)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);
  sprintf(content_type_str, "%d", content_type);
  flickcurl_add_param(fc, "content_type", content_type_str);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.setContentType"))
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
 * flickcurl_photos_setDates:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to edit dates for.
 * @date_posted: The date the photo was uploaded to flickr as a unix time (or -1)
 * @date_taken: The date the photo was taken as a unix time (or -1)
 * @date_taken_granularity: The granularity of the date the photo was taken: 0 second, 4 month, 6 year (or -1)
 * 
 * Set one or both of the dates for a photo.
 *
 * Implements flickr.photos.setDates (0.11)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_setDates(flickcurl* fc, const char* photo_id,
                          int date_posted, int date_taken,
                          int date_taken_granularity)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int result = 1;
  char date_posted_str[20];
  char* date_taken_str = NULL;
  char date_taken_granularity_str[3];
  
  flickcurl_init_params(fc, 1);

  if(!photo_id)
    return 1;

  /* Nothing to do */
  if(date_posted <0 && date_taken <0 && date_taken_granularity <0)
    return 0;

  if(date_taken_granularity > 10)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);
  if(date_posted >= 0) {
    /* posted date is always a unix time */
    sprintf(date_posted_str, "%d", date_posted);
    flickcurl_add_param(fc, "date_posted", date_posted_str);
  }
  if(date_taken >= 0) {
    /* taken date is always a SQL timestamp */
    date_taken_str = flickcurl_unixtime_to_sqltimestamp(date_taken);
    flickcurl_add_param(fc, "date_taken", date_taken_str);
  }
  if(date_taken_granularity >= 0) {
    sprintf(date_taken_granularity_str, "%d", date_taken_granularity);
    flickcurl_add_param(fc, "date_taken_granularity", date_taken_granularity_str);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.setDates"))
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

  result = 0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = 1;

  if(date_taken_str)
    free(date_taken_str);

  return result;
}


/**
 * flickcurl_photos_setMeta:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to set information for.
 * @title: The title for the photo.
 * @description: The description for the photo.
 * 
 * Set the meta information for a photo.
 *
 * Implements flickr.photos.setMeta (0.11)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_setMeta(flickcurl* fc, const char* photo_id,
                         const char* title, const char* description)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int result = 1;
  
  flickcurl_init_params(fc, 1);

  if(!photo_id || !title || !description)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);
  flickcurl_add_param(fc, "title", title);
  flickcurl_add_param(fc, "description", description);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.setMeta"))
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

  result = 0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_photos_setPerms:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to set permissions for.
 * @perms: The #flickcurl_perms photo permissions
 * 
 * Set permissions for a photo.
 *
 * Implements flickr.photos.setPerms (0.11)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_setPerms(flickcurl* fc, const char* photo_id, 
                          flickcurl_perms* perms)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int result = 1;
  char is_public_str[2];
  char is_friend_str[2];
  char is_family_str[2];
  char perm_comment_str[2];
  char perm_addmeta_str[2];
  
  flickcurl_init_params(fc, 1);

  if(!photo_id || !perms)
    return 1;

  if(perms->perm_comment <0 || perms->perm_comment >3)
    return 1;

  if(perms->perm_addmeta <0 || perms->perm_addmeta >3)
    return 1;

  flickcurl_add_param(fc, "photo_id", photo_id);
  sprintf(is_public_str, "%d", (perms->is_public ? 1 : 0));
  flickcurl_add_param(fc, "is_public", is_public_str);
  sprintf(is_friend_str, "%d", (perms->is_friend ? 1 : 0));
  flickcurl_add_param(fc, "is_friend", is_friend_str);
  sprintf(is_family_str, "%d", (perms->is_family ? 1 : 0));
  flickcurl_add_param(fc, "is_family", is_family_str);
  sprintf(perm_comment_str, "%d", perms->perm_comment);
  flickcurl_add_param(fc, "perm_comment", perm_comment_str);
  sprintf(perm_addmeta_str, "%d", perms->perm_addmeta);
  flickcurl_add_param(fc, "perm_addmeta", perm_addmeta_str);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.setPerms"))
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

  result = 0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_photos_setSafetyLevel:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to set the adultness of.
 * @safety_level: The safety level of the photo.  1 for Safe, 2 for Moderate, and 3 for Restricted (or <0 for no change)
 * @hidden: >0 to hide the photo from public searches. 0 to not. <0 for no change.
 * 
 * Set the safety level of a photo.
 *
 * Implements flickr.photos.setSafetyLevel (0.11)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_setSafetyLevel(flickcurl* fc, const char* photo_id,
                                int safety_level, int hidden)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int result = 1;
  char safety_level_str[11];
  char hidden_str[11];
  
  flickcurl_init_params(fc, 1);

  if(!photo_id)
    return 1;

  if(safety_level <= 0 && hidden <0)
    return 0;

  flickcurl_add_param(fc, "photo_id", photo_id);
  if(safety_level >0) {
    sprintf(safety_level_str, "%d", safety_level);
    flickcurl_add_param(fc, "safety_level", safety_level_str);
  }
  if(hidden >= 0) {
    sprintf(hidden_str, "%d", hidden ? 1 : 0);
    flickcurl_add_param(fc, "hidden", hidden_str);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.setSafetyLevel"))
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

  result = 0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_photos_setTags:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * @tags: all tags for the photo as a space-separated list
 *
 * Set the tags for a photo.
 *
 * Note that this replaces all existing tags with the @tags here.
 *
 * Implements flickr.photos.setTags (0.9)
 *
 * Return value: non-0 on failure
 */
int
flickcurl_photos_setTags(flickcurl* fc, const char* photo_id, const char* tags)
{
  xmlDocPtr doc = NULL;
  
  flickcurl_init_params(fc, 1);

  if(!photo_id || !tags)
    return 1;
  
  flickcurl_add_param(fc, "photo_id", photo_id);
    
  flickcurl_add_param(fc, "tags", tags);
    
  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.photos.setTags"))
    goto tidy;
  
  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

 tidy:

  return fc->failed;
}
