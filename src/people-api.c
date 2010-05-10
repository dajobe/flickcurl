/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * people-api.c - Flickr flickr.people.* API calls
 *
 * Copyright (C) 2007-2010, David Beckett http://www.dajobe.org/
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
 * flickcurl_people_findByEmail:
 * @fc: flickcurl context
 * @email: user email address
 * 
 * Get a user's NSID, given their email address
 *
 * Implements flickr.people.findByEmail (0.8)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_people_findByEmail(flickcurl* fc, const char* email)
{
  return flickcurl_call_get_one_string_field(fc, "find_email", email, 
                                             "flickr.people.findByEmail",
                                             (const xmlChar*)"/rsp/user/@nsid");
}


/**
 * flickcurl_people_findByUsername:
 * @fc: flickcurl context
 * @username: username
 * 
 * Get a user's NSID, given their username address
 *
 * Implements flickr.people.findByUsername (0.8)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_people_findByUsername(flickcurl* fc, const char* username)
{
  return flickcurl_call_get_one_string_field(fc, "username", username, 
                                             "flickr.people.findByUsername",
                                             (const xmlChar*)"/rsp/user/@nsid");
}


/**
 * flickcurl_people_getInfo:
 * @fc: flickcurl context
 * @user_id: user NSID
 * 
 * Get information about a person
 *
 * Implements flickr.people.getInfo (0.6)
 *
 * NSID can be found by flickcurl_people_findByEmail() or
 * flickcurl_people_findByUsername().
 * 
 * Return value: #flickcurl_person object or NULL on failure
 **/
flickcurl_person*
flickcurl_people_getInfo(flickcurl* fc, const char* user_id)
{
  const char * parameters[6][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_person* person = NULL;
  
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.people.getInfo", parameters, count))
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

  person = flickcurl_build_person(fc, xpathCtx, (const xmlChar*)"/rsp/person");

 tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);
  if(fc->failed)
    person = NULL;

  return person;
}


/**
 * flickcurl_people_getPublicGroups:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to fetch groups for.
 * 
 * Returns the list of public groups a user is a member of.
 *
 * Implements flickr.people.getPublicGroups (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_group**
flickcurl_people_getPublicGroups(flickcurl* fc, const char* user_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_group** groups = NULL;
  
  if(!user_id)
    return NULL;

  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.people.getPublicGroups", parameters, count))
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

  if(fc->failed)
    groups = NULL;

  return groups;
}


/**
 * flickcurl_people_getPublicPhotos_params:
 * @fc: flickcurl context
 * @user_id: The NSID of the user who's photos to return.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Get a list of public photos for the given user.
 *
 * Currently supported extras fields are: license, date_upload,
 * date_taken, owner_name, icon_server, original_format,
 * last_update, geo, tags, machine_tags.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: list of people public photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_people_getPublicPhotos_params(flickcurl* fc, const char* user_id, 
                                        flickcurl_photos_list_params* list_params)
{
  const char* parameters[12][2];
  int count = 0;
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  
  if(!user_id)
    return NULL;

  /* API parameters */
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.people.getPublicPhotos", parameters, count))
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
 * flickcurl_people_getPublicPhotos:
 * @fc: flickcurl context
 * @user_id: The NSID of the user who's photos to return.
 * @extras: A comma-delimited list of extra information to fetch for each returned record.
 * @per_page: Number of photos to return per page (default 100, max 500)
 * @page: The page of results to return (default 1)
 * 
 * Get a list of public photos for the given user.
 *
 * See flickcurl_people_getPublicPhotos_params() for details of extras.
 *
 * Implements flickr.people.getPublicPhotos (0.12)
 * 
 * Return value: list of photos or NULL on failure
 **/
flickcurl_photo**
flickcurl_people_getPublicPhotos(flickcurl* fc, const char* user_id, 
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

  photos_list = flickcurl_people_getPublicPhotos_params(fc, user_id, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_people_getUploadStatus:
 * @fc: flickcurl context
 * 
 * Returns information for the calling user related to photo uploads.
 *
 * Implements flickr.people.getUploadStatus (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_user_upload_status*
flickcurl_people_getUploadStatus(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_user_upload_status* status = NULL;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.people.getUploadStatus", parameters, count))
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

  status = flickcurl_build_user_upload_status(fc, xpathCtx, (const xmlChar*)"/rsp/user/*");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    status = NULL;

  return status;
}


/**
 * flickcurl_people_getPhotos_params:
 * @fc: flickcurl context
 * @user_id: The NSID of the user who's photos to return. A value of "me" will return the calling user's photos.
 * @safe_search: Safe search setting: 1 for safe, 2 for moderate, 3 for restricted. (Please note: Un-authed calls can only see Safe content.) (or < 0)
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @content_type: Content Type setting: 1 for photos only, 2 for screenshots only, 3 for 'other' only, 4 for photos and screenshots, 5 for screenshots and 'other', 6 for photos and 'other', 7 for photos, screenshots, and 'other' (all) (or < 0)
 * @privacy_filter: Return photos only matching a certain privacy level. This only applies when making an authenticated call to view photos you own. Valid values are: 1 public photos, 2 private photos visible to friends, 3 private photos visible to family, 4 private photos visible to friends & family, 5 completely private photos (or < 0)
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Get photos from the given user's photostream.
 *
 * Only photos visible to the calling user will be returned. This
 * method must be authenticated; to return public photos for a user,
 * use flickcurl_people_getPublicPhotos().
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_people_getPhotos_params(flickcurl* fc, const char* user_id,
                                  int safe_search,
                                  const char* min_upload_date,
                                  const char* max_upload_date,
                                  const char* min_taken_date,
                                  const char* max_taken_date,
                                  int content_type,
                                  int privacy_filter,
                                  flickcurl_photos_list_params* list_params)
{
  const char* parameters[18][2];
  int count = 0;
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  char safe_search_s[4];
  char content_type_s[4];
  char privacy_filter_s[4];
  
  if(!user_id)
    return NULL;

  parameters[count][0] = "user_id";
  parameters[count++][1] = user_id;
  if(safe_search >= 0 && safe_search < 10) {
    sprintf(safe_search_s, "%d", safe_search);
    parameters[count][0] = "safe_search";
    parameters[count++][1] = safe_search_s;
  }
  if(min_upload_date) {
    parameters[count][0] = "min_upload_date";
    parameters[count++][1] = min_upload_date;
  }
  if(max_upload_date) {
    parameters[count][0] = "max_upload_date";
    parameters[count++][1] = max_upload_date;
  }
  if(min_taken_date) {
    parameters[count][0] = "min_taken_date";
    parameters[count++][1] = min_taken_date;
  }
  if(max_taken_date) {
    parameters[count][0] = "max_taken_date";
    parameters[count++][1] = max_taken_date;
  }
  if(content_type >= 0 && content_type < 10) {
    sprintf(content_type_s, "%d", content_type);
    parameters[count][0] = "content_type";
    parameters[count++][1] = content_type_s;
  }
  if(privacy_filter >= 0 && privacy_filter < 10) {
    sprintf(privacy_filter_s, "%d", privacy_filter);
    parameters[count][0] = "privacy_filter";
    parameters[count++][1] = privacy_filter_s;
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.people.getPhotos", parameters, count))
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
 * flickcurl_people_getPhotos:
 * @fc: flickcurl context
 * @user_id: The NSID of the user who's photos to return. A value of "me" will return the calling user's photos.
 * @safe_search: Safe search setting: 1 for safe, 2 for moderate, 3 for restricted. (Please note: Un-authed calls can only see Safe content.) (or < 0)
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @content_type: Content Type setting: 1 for photos only, 2 for screenshots only, 3 for 'other' only, 4 for photos and screenshots, 5 for screenshots and 'other', 6 for photos and 'other', 7 for photos, screenshots, and 'other' (all) (or < 0)
 * @privacy_filter: Return photos only matching a certain privacy level. This only applies when making an authenticated call to view photos you own. Valid values are: 1 public photos, 2 private photos visible to friends, 3 private photos visible to family, 4 private photos visible to friends & family, 5 completely private photo (or < 0)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: <code>description</code>, <code>license</code>, <code>date_upload</code>, <code>date_taken</code>, <code>owner_name</code>, <code>icon_server</code>, <code>original_format</code>, <code>last_update</code>, <code>geo</code>, <code>tags</code>, <code>machine_tags</code>, <code>o_dims</code>, <code>views</code>, <code>media</code>, <code>path_alias</code>, <code>url_sq</code>, <code>url_t</code>, <code>url_s</code>, <code>url_m</code>, <code>url_o</code> (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get photos from the given user's photostream.
 *
 * Only photos visible to the calling user will be returned. This
 * method must be authenticated; to return public photos for a user,
 * use flickcurl_people_getPublicPhotos().
 *
 * Implements flickr.people.getPhotos (1.18)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_people_getPhotos(flickcurl* fc, const char* user_id,
                           int safe_search,
                           const char* min_upload_date,
                           const char* max_upload_date,
                           const char* min_taken_date,
                           const char* max_taken_date,
                           int content_type,
                           int privacy_filter,
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

  photos_list = flickcurl_people_getPhotos_params(fc, user_id,
                                                  safe_search,
                                                  min_upload_date,
                                                  max_upload_date,
                                                  min_taken_date,
                                                  max_taken_date,
                                                  content_type,
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
 * flickcurl_people_getPhotosOf_params:
 * @fc: flickcurl context
 * @user_id: The NSID of the user who's photo to search. A value of "me" will search against the calling user's photos for authenticated calls.
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns a list of photos containing a particular Flickr member.
 *
 * Announced 2010-01-21
 * http://code.flickr.com/blog/2010/01/21/people-in-photos-the-api-methods/
 * 
 * Return value: photos list or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_people_getPhotosOf_params(flickcurl* fc, const char* user_id,
                                    flickcurl_photos_list_params* list_params)
{
  const char* parameters[11][2];
  int count = 0;
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  
  if(!user_id)
    return photos_list;

  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.people.getPhotosOf", parameters, count))
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
 * flickcurl_people_getPhotosOf:
 * @fc: flickcurl context
 * @user_id: The NSID of the user who's photo to search. A value of "me" will search against the calling user's photos for authenticated calls.
 * @extras: A comma-delimited list of extra information to fetch for each returned record (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Returns a list of photos containing a particular Flickr member.
 *
 * Implements flickr.people.getPhotosOf (1.17) 
 *
 * Announced 2010-01-21
 * http://code.flickr.com/blog/2010/01/21/people-in-photos-the-api-methods/
 * 
 * Return value: photos array or NULL on failure
 **/
flickcurl_photo**
flickcurl_people_getPhotosOf(flickcurl* fc, const char* user_id,
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

  photos_list = flickcurl_people_getPhotosOf_params(fc, user_id, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}
