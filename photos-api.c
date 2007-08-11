/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-api.c - Flickr flickr.photos.* API calls
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
 * flickcurl_photos_addTags:
 * @fc: flickcurl context
 * @photo_id: photo ID
 * @tags: tags to add as a space-separated list
 *
 * Add tags to a photo.
 *
 * Implements flickr.photos.addTags (0.9)
 */
int
flickcurl_photos_addTags(flickcurl* fc, const char* photo_id, const char* tags)
{
  const char * parameters[7][2];
  int count=0;
  xmlDocPtr doc=NULL;
  
  if(!photo_id || !tags)
    return 1;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = "tags";
  parameters[count++][1]= tags;
    
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.addTags", parameters, count))
    goto tidy;
  
  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);
    
  doc=flickcurl_invoke(fc);
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
 */
int
flickcurl_photos_delete(flickcurl* fc, const char* photo_id)
{
  const char * parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  
  if(!photo_id)
    return 1;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.delete", parameters, count))
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
 * flickcurl_photos_getAllContexts:
 * @fc: flickcurl context
 * @id: photo ID
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
  const char * parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  flickcurl_context** contexts=NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getAllContexts", parameters, count))
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
 * flickcurl_photos_getContactsPhotos:
 * @fc: flickcurl context
 * @count: Number of photos to return (Default 10, maximum 50)
 * @just_friends: Set to non-0 to only show photos from friends and family (excluding regular contacts).
 * @single_photo: Set to non-0 to only fetch one photo (the latest) per contact, instead of all photos in chronological order.
 * @include_self: Set to non-0 to include photos from the calling user.
 * @extras: A comma-delimited list of extra information to fetch for
 * each returned record. Currently supported fields are: license,
 * date_upload, date_taken, owner_name, icon_server, original_format,
 * last_update (or NULL for no extras)
 * 
 * Fetch a list of recent photos from the calling users' contacts.
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
  const char* parameters[12][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_photo** photos=NULL;
  char true_s[2]="1";

  if(contact_count > 1) {
    char count_s[20];
    sprintf(count_s, "%d", contact_count);
    parameters[count][0]  = "count";
    parameters[count++][1]= count_s;
  }
  if(just_friends) {
    parameters[count][0]  = "just_friends";
    parameters[count++][1]= true_s;
  }
  if(single_photo) {
    parameters[count][0]  = "single_photo";
    parameters[count++][1]= true_s;
  }
  if(include_self) {
    parameters[count][0]  = "include_self";
    parameters[count++][1]= true_s;
  }
  if(extras) {
    parameters[count][0]  = "extras";
    parameters[count++][1]= extras;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getContactsPhotos", parameters,
                       count))
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

  photos=flickcurl_build_photos(fc, xpathCtx,
                                (const xmlChar*)"/rsp/photos/photo", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photos=NULL;

  return photos;
}


/**
 * flickcurl_photos_getContactsPublicPhotos:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to fetch photos for.
 * @photo_count: Number of photos to return (Default 10, maximum 50)
 * @just_friends: Set to non-0 to only show photos from friends and family (excluding regular contacts)
 * @single_photo: Set to non-0 to only fetch one photo (the latest) per contact, instead of all photos in chronological order.
 * @include_self: Set to non-0 to include photos from the user specified by user_id.
 * @extras: A comma-delimited list of extra information to fetch for
 * each returned record. Currently supported fields are: license,
 * date_upload, date_taken, owner_name, icon_server, original_format,
 * last_update. (or NULL)
 * 
 * Fetch a list of recent public photos from a users' contacts.
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
  const char* parameters[13][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_photo** photos=NULL;
  char true_s[2]="1";
  char photo_count_s[10];
  
  if(!user_id)
    return NULL;

  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;
  parameters[count][0]  = "count";
  sprintf(photo_count_s, "%d", photo_count);
  parameters[count++][1]= photo_count_s;
  if(just_friends) {
    parameters[count][0]  = "just_friends";
    parameters[count++][1]= true_s;
  }
  if(single_photo) {
    parameters[count][0]  = "single_photo";
    parameters[count++][1]= true_s;
  }
  if(include_self) {
    parameters[count][0]  = "include_self";
    parameters[count++][1]= true_s;
  }
  if(extras) {
    parameters[count][0]  = "extras";
    parameters[count++][1]= extras;
  }
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getContactsPublicPhotos", parameters,
                       count))
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

  photos=flickcurl_build_photos(fc, xpathCtx,
                                (const xmlChar*)"/rsp/photos/photo", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photos=NULL;

  return photos;
}


/**
 * flickcurl_photos_getContext:
 * @fc: flickcurl context
 * @id: photo ID
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
  const char * parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  flickcurl_context** contexts=NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getContext", parameters, count))
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


/*
 * flickr.photos.getCounts
 */


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
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_exif** exifs=NULL;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  if(secret) {
    parameters[count][0]  = "secret";
    parameters[count++][1]= secret;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getExif", parameters, count))
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

  exifs=flickcurl_build_exifs(fc, xpathCtx,
                              (const xmlChar*)"/rsp/photo/exif", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    exifs=NULL;

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
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_person** persons=NULL;
  char per_page_s[4];
  char page_s[4];
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "page";
  sprintf(page_s, "%d", page);
  parameters[count++][1]= page_s;
  parameters[count][0]  = "per_page";
  sprintf(per_page_s, "%d", per_page);
  parameters[count++][1]= per_page_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getFavorites", parameters, count))
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

  persons=flickcurl_build_persons(fc, xpathCtx,
                                  (const xmlChar*)"/rsp/photo/person", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    persons=NULL;

  return persons;
}


/**
 * flickcurl_photos_getInfo:
 * @fc: flickcurl context
 * @id: photo ID
 * 
 * Get information about a photo
 *
 * Implements flickr.photos.getInfo (0.5)
 * 
 * Return value: #flickcurl_photo or NULL on failure
 **/
flickcurl_photo*
flickcurl_photos_getInfo(flickcurl* fc, const char* photo_id)
{
  const char * parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_photo* photo=NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getInfo", parameters, count))
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

  photo=flickcurl_build_photo(fc, xpathCtx);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photo=NULL;

  return photo;
}


/*
 * flickr.photos.getNotInSet
 */


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
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_perms* perms=NULL;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.getPerms", parameters, count))
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

  perms=flickcurl_build_perms(fc, xpathCtx, (const xmlChar*)"/rsp/perms");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    perms=NULL;

  return perms;
}


/*
 * flickr.photos.getRecent
 */


/*
 * flickr.photos.getSizes
 */


/*
 * flickr.photos.getUntagged
 */


/*
 * flickr.photos.getWithGeoData
 */


/*
 * flickr.photos.getWithoutGeoData
 */


/*
 * flickr.photos.recentlyUpdated
 */


/**
 * flickcurl_photos_removeTag:
 * @fc: flickcurl context
 * @tag_id: tag ID to remove from the photo
 *
 * Remove a tag from a photo.
 *
 * The @tag_id is returned such as from flickr_photos_getInfo()
 *
 * Implements flickr.photos.removeTag (0.9)
 */
int
flickcurl_photos_removeTag(flickcurl* fc, const char* tag_id)
{
  const char * parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  
  if(!tag_id)
    return 1;
  
  parameters[count][0]  = "tag_id";
  parameters[count++][1]= tag_id;
    
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.removeTag", parameters, count))
    goto tidy;
  
  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);
    
  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

 tidy:

  return fc->failed;
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
 * Return value: an array of #flickcurl_photo or NULL
 **/
flickcurl_photo**
flickcurl_photos_search(flickcurl* fc, flickcurl_search_params* params)
{
  const char* parameters[28][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_photo** photos=NULL;
  char min_upload_date_s[15];
  char max_upload_date_s[15];
  char accuracy_s[3];
  char safe_search_s[2];
  char content_type_s[2];
  char per_page_s[4];
  char page_s[4];
  
  if(params->user_id) {
    parameters[count][0]  = "user_id";
    parameters[count++][1]= params->user_id;
  }
  if(params->tags) {
    parameters[count][0]  = "tags";
    parameters[count++][1]= params->tags;
  }
  if(params->tag_mode) {
    parameters[count][0]  = "tag_mode";
    parameters[count++][1]= params->tag_mode;
  }
  if(params->text) {
    parameters[count][0]  = "text";
    parameters[count++][1]= params->text;
  }
  if(params->min_upload_date) {
    sprintf(min_upload_date_s, "%d", params->min_upload_date);
    parameters[count][0]  = "min_upload_date";
    parameters[count++][1]= min_upload_date_s;
  }
  if(params->max_upload_date) {
    sprintf(max_upload_date_s, "%d", params->max_upload_date);
    parameters[count][0]  = "max_upload_date";
    parameters[count++][1]= max_upload_date_s;
  }
  if(params->min_taken_date) {
    parameters[count][0]  = "min_taken_date";
    parameters[count++][1]= params->min_taken_date;
  }
  if(params->max_taken_date) {
    parameters[count][0]  = "max_taken_date";
    parameters[count++][1]= params->max_taken_date;
  }
  if(params->license) {
    parameters[count][0]  = "license";
    parameters[count++][1]= params->license;
  }
  if(params->sort) {
    parameters[count][0]  = "sort";
    parameters[count++][1]= params->sort;
  }
  if(params->privacy_filter) {
    parameters[count][0]  = "privacy_filter";
    parameters[count++][1]= params->privacy_filter;
  }
  if(params->bbox) {
    parameters[count][0]  = "bbox";
    parameters[count++][1]= params->bbox;
  }
  if(params->accuracy) {
    if(params->accuracy >=0 && params->accuracy <=16) {
      sprintf(accuracy_s, "%d", params->accuracy);
      parameters[count][0]  = "accuracy";
      parameters[count++][1]= accuracy_s;
    }
  }
  if(params->safe_search) {
    if(params->safe_search >=0 && params->safe_search <=3) {
      sprintf(safe_search_s, "%d", params->safe_search);
      parameters[count][0]  = "safe_search";
      parameters[count++][1]= safe_search_s;
    }
  }
  if(params->content_type) {
    if(params->content_type >=0 && params->content_type <=4) {
      sprintf(content_type_s, "%d", params->content_type);
      parameters[count][0]  = "content_type";
      parameters[count++][1]= content_type_s;
    }
  }
  if(params->machine_tags) {
    parameters[count][0]  = "machine_tags";
    parameters[count++][1]= params->machine_tags;
  }
  if(params->machine_tag_mode) {
    parameters[count][0]  = "machine_tag_mode";
    parameters[count++][1]= params->machine_tag_mode;
  }
  if(params->group_id) {
    parameters[count][0]  = "group_id";
    parameters[count++][1]= params->group_id;
  }
  if(params->extras) {
    parameters[count][0]  = "extras";
    parameters[count++][1]= params->extras;
  }
  if(params->per_page) {
    if(params->per_page >=0 && params->per_page <=999) {
      sprintf(per_page_s, "%d", params->per_page);
      parameters[count][0]  = "per_page";
      parameters[count++][1]= per_page_s;
    }
  }
  if(params->page) {
    if(params->page >=0 && params->page <=999) {
      sprintf(page_s, "%d", params->page);
      parameters[count][0]  = "page";
      parameters[count++][1]= page_s;
    }
  }
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.search", parameters, count))
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

  photos=flickcurl_build_photos(fc, xpathCtx,
                                (const xmlChar*)"/rsp/photos/photo", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photos=NULL;

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
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  char content_type_str[2];

  if(!photo_id || !content_type)
    return 1;

  if(content_type <1 || content_type > 3)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "content_type";
  sprintf(content_type_str, "%d", content_type);
  parameters[count++][1]= content_type_str;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.setContentType", parameters, count))
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
  const char* parameters[11][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  int result=1;
  char date_posted_str[10];
  char* date_taken_str=NULL;
  char date_taken_granularity_str[3];
  
  if(!photo_id)
    return 1;

  /* Nothing to do */
  if(date_posted <0 && date_taken <0 && date_taken_granularity <0)
    return 0;

  if(date_taken_granularity > 10)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  if(date_posted >= 0) {
    parameters[count][0]  = "date_posted";
    /* posted date is always a unix time */
    sprintf(date_posted_str, "%d", date_posted);
    parameters[count++][1]= date_posted_str;
  }
  if(date_taken >= 0) {
    parameters[count][0]  = "date_taken";
    /* taken date is always a SQL timestamp */
    date_taken_str=flickcurl_unixtime_to_sqltimestamp(date_taken);
  }
  if(date_taken_granularity >= 0) {
    parameters[count][0]  = "date_taken_granularity";
    sprintf(date_taken_granularity_str, "%d", date_taken_granularity);
    parameters[count++][1]= date_taken_granularity_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.setDates", parameters, count))
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

  result=0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result=1;

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
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  int result=1;
  
  if(!photo_id || !title || !description)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "title";
  parameters[count++][1]= title;
  parameters[count][0]  = "description";
  parameters[count++][1]= description;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.setMeta", parameters, count))
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

  result=0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result=1;

  return result;
}


/**
 * flickcurl_photos_setPerms:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to set permissions for.
 * @params: The #flickcurl_perms photo permissions
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
  const char* parameters[13][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  int result=1;
  char is_public_str[2];
  char is_friend_str[2];
  char is_family_str[2];
  char perm_comment_str[2];
  char perm_addmeta_str[2];
  
  if(!photo_id || !perms ||
     !perms->is_public || !perms->is_friend || !perms->is_family ||
     !perms->perm_comment || !perms->perm_addmeta)
    return 1;

  if(perms->perm_comment <0 || perms->perm_comment >3)
    return 1;

  if(perms->perm_addmeta <0 || perms->perm_addmeta >3)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "is_public";
  sprintf(is_public_str, "%d", (perms->is_public ? 1 : 0));
  parameters[count++][1]= is_public_str;
  parameters[count][0]  = "is_friend";
  sprintf(is_friend_str, "%d", (perms->is_friend ? 1 : 0));
  parameters[count++][1]= is_friend_str;
  parameters[count][0]  = "is_family";
  sprintf(is_family_str, "%d", (perms->is_family ? 1 : 0));
  parameters[count++][1]= is_family_str;
  parameters[count][0]  = "perm_comment";
  sprintf(perm_comment_str, "%d", perms->perm_comment);
  parameters[count++][1]= perm_comment_str;
  parameters[count][0]  = "perm_addmeta";
  sprintf(perm_addmeta_str, "%d", perms->perm_addmeta);
  parameters[count++][1]= perm_addmeta_str;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.setPerms", parameters, count))
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

  result=0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result=1;

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
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  int result=1;
  char safety_level_str[2];
  char hidden_str[2];
  
  if(!photo_id)
    return 1;

  if(safety_level <=0 && hidden <0)
    return 0;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  if(safety_level >0) {
    parameters[count][0]  = "safety_level";
    sprintf(safety_level_str, "%d", safety_level);
    parameters[count++][1]= safety_level_str;
  }
  if(hidden >=0) {
    parameters[count][0]  = "hidden";
    sprintf(hidden_str, "%d", hidden ? 1 : 0);
    parameters[count++][1]= hidden_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.setSafetyLevel", parameters, count))
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

  result=0;

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result=1;

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
 */
int
flickcurl_photos_setTags(flickcurl* fc, const char* photo_id, const char* tags)
{
  const char * parameters[7][2];
  int count=0;
  xmlDocPtr doc=NULL;
  
  if(!photo_id || !tags)
    return 1;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
    
  parameters[count][0]  = "tags";
  parameters[count++][1]= tags;
    
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.setTags", parameters, count))
    goto tidy;
  
  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);
    
  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

 tidy:

  return fc->failed;
}
