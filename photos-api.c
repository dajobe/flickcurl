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

 tidy:

  return fc->failed;
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


/*
 * flickr.photos.getContactsPhotos
 */


/*
 * flickr.photos.getContactsPublicPhotos
 */


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


/*
 * flickr.photos.getExif
 */


/*
 * flickr.photos.getFavorites
 */


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


/*
 * flickr.photos.getPerms
 */


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
    parameters[count][0]  = "min_upload_date";
    parameters[count++][1]= params->min_upload_date;
  }
  if(params->max_upload_date) {
    parameters[count][0]  = "max_upload_date";
    parameters[count++][1]= params->max_upload_date;
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
    parameters[count][0]  = "accuracy";
    parameters[count++][1]= params->accuracy;
  }
  if(params->safe_search) {
    parameters[count][0]  = "safe_search";
    parameters[count++][1]= params->safe_search;
  }
  if(params->content_type) {
    parameters[count][0]  = "content_type";
    parameters[count++][1]= params->content_type;
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
    parameters[count][0]  = "per_page";
    parameters[count++][1]= params->per_page;
  }
  if(params->page) {
    parameters[count][0]  = "page";
    parameters[count++][1]= params->page;
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
                                (const xmlChar*)"/rsp/photos", NULL);

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
  xmlXPathContextPtr xpathCtx=NULL; 
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
 * @is_public: non-0 to set the photo to public else private
 * @is_friend: non-0 to make the photo visible to friends when private
 * @is_family: non-0 to make the photo visible to family when private
 * @perm_comment: who can add comments to the photo and it's notes. one of: 0 nobody,  1 friends & family, 2 contacts, 3 everybody
 * @perm_addmeta: who can add notes and tags to the photo. one of: 0 nobody / just the owner, 1 friends & family, 2 contacts, 3 everybody
 * 
 * Set permissions for a photo.
 *
 * Implements flickr.photos.setPerms (0.11)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_setPerms(flickcurl* fc, const char* photo_id, 
                          int is_public, int is_friend,
                          int is_family, int perm_comment,
                          int perm_addmeta)
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
  
  if(!photo_id || !is_public || !is_friend || !is_family || !perm_comment ||
     !perm_addmeta)
    return 1;

  if(perm_comment <0 || perm_comment >3)
    return 1;

  if(perm_addmeta <0 || perm_addmeta >3)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "is_public";
  sprintf(is_public_str, "%d", (is_public ? 1 : 0));
  parameters[count++][1]= is_public_str;
  parameters[count][0]  = "is_friend";
  sprintf(is_friend_str, "%d", (is_friend ? 1 : 0));
  parameters[count++][1]= is_friend_str;
  parameters[count][0]  = "is_family";
  sprintf(is_family_str, "%d", (is_family ? 1 : 0));
  parameters[count++][1]= is_family_str;
  parameters[count][0]  = "perm_comment";
  sprintf(perm_comment_str, "%d", perm_comment);
  parameters[count++][1]= perm_comment_str;
  parameters[count][0]  = "perm_addmeta";
  sprintf(perm_addmeta_str, "%d", perm_addmeta);
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
