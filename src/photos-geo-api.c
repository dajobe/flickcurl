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
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_location* location=NULL;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.getLocation", parameters, count))
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

  location=flickcurl_build_location(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/photo/location");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    location=NULL;

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
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_perms* perms=NULL;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.geo.getPerms", parameters, count))
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

  perms=flickcurl_build_perms(fc, xpathCtx,
                              (const xmlChar*)"/rsp/perms");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    perms=NULL;

  return perms;
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
  int count=0;
  xmlDocPtr doc=NULL;
  
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

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  tidy:

  return fc->failed;
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
  int count=0;
  xmlDocPtr doc=NULL;
  char latitude_s[50];
  char longitude_s[50];
  char accuracy_s[50];
  int result=1;
  
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
  sprintf(longitude_s, "%f", location->latitude);
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
  int count=0;
  xmlDocPtr doc=NULL;
  char is_public_str[2];
  char is_contact_str[2];
  char is_friend_str[2];
  char is_family_str[2];
  int result=1;
  
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

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;

  return result;
}


