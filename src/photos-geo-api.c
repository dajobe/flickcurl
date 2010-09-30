/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-geo-api.c - Flickr flickr.photos.geo.* API calls
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
 * flickcurl_photos_geo_batchCorrectLocation:
 * @fc: flickcurl context
 * @location: The location (lat, long, accuracy) of the photos to update.
 * @place_id: A Flickr Places ID (or NULL)
 * @woe_id: A Where On Earth (WOE) ID (or 0)
 *
 * Correct the places hierarchy for all the photos for a user at a
 * given location (latitude, longitude and accuracy).
 *
 * You must pass either a valid Places ID in @place_id or a WOE ID in
 * @woe_id.
 * 
 * Batch corrections are processed in a delayed queue so it may take
 * a few minutes before the changes are reflected in a user's photos.
 *
 * Implements flickr.photos.geo.batchCorrectLocation (1.8)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_geo_batchCorrectLocation(flickcurl* fc,
                                          flickcurl_location* location,
                                          const char* place_id, int woe_id)
{
  const char* parameters[12][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  char latitude_s[50];
  char longitude_s[50];
  char accuracy_s[50];
  char woe_id_str[10];
  int result = 0;

  if(!place_id || !woe_id)
    return 1;

  if(location->latitude < -90.0)
    location->latitude= -90.0;
  if(location->latitude > 90.0)
    location->latitude= 90.0;
  if(location->longitude < -180.0)
    location->longitude= -180.0;
  if(location->longitude > 180.0)
    location->longitude= 180.0;
  if(location->accuracy < 1 || location->accuracy > 16)
    location->accuracy = 0;
  

  parameters[count][0]  = "lat";
  sprintf(latitude_s, "%f", location->latitude);
  parameters[count++][1]= latitude_s;
  parameters[count][0]  = "lon";
  sprintf(longitude_s, "%f", location->longitude);
  parameters[count++][1]= longitude_s;
  parameters[count][0]  = "accuracy";
  sprintf(accuracy_s, "%d", location->accuracy);
  parameters[count++][1]= accuracy_s;
  if(place_id) {
    parameters[count][0]  = "place_id";
    parameters[count++][1]= place_id;
  }
  if(woe_id > 0) {
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count][0]  = "woe_id";
    parameters[count++][1]= woe_id_str;
  }
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.batchCorrectLocation",
                       parameters, count))
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
 * flickcurl_photos_geo_correctLocation:
 * @fc: flickcurl context
 * @photo_id: The ID of the photo whose WOE location is being corrected.
 * @place_id: A Flickr Places ID (or NULL)
 * @woe_id: A Where On Earth (WOE) ID (or NULL)
 * 
 * Correct a photo location.
 *
 * You must pass either a valid Places ID in @place_id or a WOE ID in @woe_id.
 * 
 * Implements flickr.photos.geo.correctLocation (1.8)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_geo_correctLocation(flickcurl* fc, const char* photo_id,
                                     const char* place_id, int woe_id)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char woe_id_str[10];
  
  if(!photo_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "place_id";
  parameters[count++][1]= place_id;
  if(woe_id > 0) {
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count][0]  = "woe_id";
    parameters[count++][1]= woe_id_str;
  }
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.correctLocation", parameters,
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_photos_geo_getLocation:
 * @fc: flickcurl context
 * @photo_id: The id of the photo you want to retrieve location data for.
 * 
 * Get the geo data (latitude and longitude and the accuracy level) for a photo.
 *
 * Implements flickr.photos.geo.getLocation (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_location*
flickcurl_photos_geo_getLocation(flickcurl* fc, const char* photo_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_location* location = NULL;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.getLocation", parameters, count))
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

  location = flickcurl_build_location(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/photo/location");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    location = NULL;

  return location;
}


/**
 * flickcurl_photos_geo_getPerms:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to get permissions for.
 * 
 * Get permissions for who may view geo data for a photo.
 *
 * Implements flickr.photos.geo.getPerms (0.12)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_perms*
flickcurl_photos_geo_getPerms(flickcurl* fc, const char* photo_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_perms* perms = NULL;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.getPerms", parameters, count))
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

  perms = flickcurl_build_perms(fc, xpathCtx,
                              (const xmlChar*)"/rsp/perms");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    perms = NULL;

  return perms;
}


/**
 * flickcurl_photos_geo_photosForLocation_params:
 * @fc: flickcurl context
 * @location: The location (lat, long, accuracy) of the photos
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Get a list of photos for a user at a specific location (latitude, longitude
 * and accuracy)
 *
 * Return value: list of photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_photos_geo_photosForLocation_params(flickcurl* fc,
                                              flickcurl_location* location,
                                              flickcurl_photos_list_params* list_params)
{
  const char* parameters[13][2];
  int count = 0;
  flickcurl_photos_list* photos_list = NULL;
  char latitude_s[50];
  char longitude_s[50];
  char accuracy_s[50];
  const char* format = NULL;

  if(!location)
    return NULL;
  
  if(location->latitude < -90.0)
    location->latitude= -90.0;
  if(location->latitude > 90.0)
    location->latitude= 90.0;
  if(location->longitude < -180.0)
    location->longitude= -180.0;
  if(location->longitude > 180.0)
    location->longitude= 180.0;
  if(location->accuracy < 1 || location->accuracy > 16)
    location->accuracy = 0;
  

  parameters[count][0]  = "lat";
  sprintf(latitude_s, "%f", location->latitude);
  parameters[count++][1]= latitude_s;
  parameters[count][0]  = "lon";
  sprintf(longitude_s, "%f", location->longitude);
  parameters[count++][1]= longitude_s;
  parameters[count][0]  = "accuracy";
  sprintf(accuracy_s, "%d", location->accuracy);
  parameters[count++][1]= accuracy_s;

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.photosForLocation", parameters,
                       count))
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
 * flickcurl_photos_geo_photosForLocation:
 * @fc: flickcurl context
 * @location: The location (lat, long, accuracy) of the photos
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: license, date_upload, date_taken, owner_name, icon_server, original_format, last_update, geo, tags, machine_tags, o_dims, views, media (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500 (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1 (or NULL)
 * 
 * Get a list of photos for a user at a specific location (latitude,
 * longitude and accuracy)
 *
 * Implements flickr.photos.geo.photosForLocation (1.8)
 * 
 * Return value: list of photos or NULL on failure
 **/
flickcurl_photo**
flickcurl_photos_geo_photosForLocation(flickcurl* fc,
                                       flickcurl_location* location,
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

  photos_list = flickcurl_photos_geo_photosForLocation_params(fc, location,
                                                              &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


/**
 * flickcurl_photos_geo_removeLocation:
 * @fc: flickcurl context
 * @photo_id: The id of the photo you want to remove location data from.
 * 
 * Removes the geo data associated with a photo.
 *
 * Implements flickr.photos.geo.removeLocation (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_geo_removeLocation(flickcurl* fc, const char* photo_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  
  if(!photo_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.removeLocation", parameters,
                       count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  tidy:

  return fc->failed;
}


/**
 * flickcurl_photos_geo_setContext:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to set context data for.
 * @context: Context is a numeric value representing the photo's geotagginess beyond latitude and longitude. The current values are: 0: not defined, 1: indoors, 2: outdoors.
 * 
 * Indicate the state of a photo's geotagginess beyond latitude and longitude.
 * 
 * Note : photos passed to this method must already be geotagged
 * using the flickcurl_photos_geo_setLocation() method.
 *
 * Implements flickr.photos.geo.setContext (1.8)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_geo_setContext(flickcurl* fc, const char* photo_id,
                                int context)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char context_str[3];
  void* result = NULL;
  
  if(!photo_id || context < 0 || context > 2)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "context";
  sprintf(context_str, "%d", context);
  parameters[count++][1]= context_str;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.setContext", parameters, count))
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
 * flickcurl_photos_geo_setLocation:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to set location data for.
 * @location: The location
 * 
 * Sets the geo data (latitude and longitude and, optionally, the
 * accuracy level) for a photo.
 *
 * Implements flickr.photos.geo.setLocation (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_geo_setLocation(flickcurl* fc, const char* photo_id,
                                 flickcurl_location* location)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  char latitude_s[50];
  char longitude_s[50];
  char accuracy_s[50];
  int result = 1;
  
  if(!photo_id)
    return 1;

  if(location->latitude < -90.0)
    location->latitude= -90.0;
  if(location->latitude > 90.0)
    location->latitude= 90.0;
  if(location->longitude < -180.0)
    location->longitude= -180.0;
  if(location->longitude > 180.0)
    location->longitude= 180.0;
  if(location->accuracy < 1 || location->accuracy > 16)
    location->accuracy = 0;
  

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "lat";
  sprintf(latitude_s, "%f", location->latitude);
  parameters[count++][1]= latitude_s;
  parameters[count][0]  = "lon";
  sprintf(longitude_s, "%f", location->longitude);
  parameters[count++][1]= longitude_s;
  if(location->accuracy >= 1) {
    parameters[count][0]  = "accuracy";
    sprintf(accuracy_s, "%d", location->accuracy);
    parameters[count++][1]= accuracy_s;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.setLocation", parameters, count))
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
 * flickcurl_photos_geo_setPerms:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to get permissions for.
 * @perms: Geo permissions
 * 
 * Set the permission for who may view the geo data associated with a photo.
 *
 * Implements flickr.photos.geo.setPerms (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_geo_setPerms(flickcurl* fc, const char* photo_id,
                              flickcurl_perms* perms)
{
  const char* parameters[12][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  char is_public_str[2];
  char is_contact_str[2];
  char is_friend_str[2];
  char is_family_str[2];
  int result = 1;
  
  if(!photo_id || !perms)
    return 1;

  parameters[count][0]  = "is_public";
  sprintf(is_public_str, "%d", (perms->is_public ? 1 : 0));
  parameters[count++][1]= is_public_str;
  parameters[count][0]  = "is_contact";
  sprintf(is_contact_str, "%d", (perms->is_contact ? 1 : 0));
  parameters[count++][1]= is_contact_str;
  parameters[count][0]  = "is_friend";
  sprintf(is_friend_str, "%d", (perms->is_friend ? 1 : 0));
  parameters[count++][1]= is_friend_str;
  parameters[count][0]  = "is_family";
  sprintf(is_family_str, "%d", (perms->is_family ? 1 : 0));
  parameters[count++][1]= is_family_str;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.setPerms", parameters, count))
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


