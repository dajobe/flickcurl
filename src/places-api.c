/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * places-api.c - Flickr flickr.places.* API calls
 *
 * Places API announced 2008-01-11
 * http://tech.groups.yahoo.com/group/yws-flickr/message/3688
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
 * flickcurl_places_find:
 * @fc: flickcurl context
 * @query: The query string to use for place ID lookups
 * 
 * Return a list of place IDs for a query string.
 *
 * The flickr.places.find method is NOT a geocoder. It will round up
 * to the nearest place type to which place IDs apply. For example,
 * if you pass it a street level address it will return the city that
 * contains the address rather than the street, or building, itself.
 *
 * This API announced 2008-01-18
 * http://tech.groups.yahoo.com/group/yws-flickr/message/3716
 *
 * Implements flickr.places.find (1.1)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_place**
flickcurl_places_find(flickcurl* fc, const char* query)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place** places=NULL;
  
  if(!query)
    return NULL;

  parameters[count][0]  = "query";
  parameters[count++][1]= query;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.find", parameters, count))
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

  places=flickcurl_build_places(fc, xpathCtx, (const xmlChar*)"/rsp/places/place", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    places=NULL;

  return places;
}


/**
 * flickcurl_places_findByLatLon:
 * @fc: flickcurl context
 * @lat: The latitude whose valid range is -90 to 90. Anything more than 4 decimal places will be truncated.
 * @lon: The longitude whose valid range is -180 to 180. Anything more than 4 decimal places will be truncated.
 * @accuracy: Recorded accuracy level of the location information. World level is 1, Country is ~3, Region ~6, City ~11, Street ~16. Current range is 1-16. The default is 16.
 * 
 * Return a place ID for a latitude, longitude and accuracy triple.
 *
 * The flickr.places.findByLatLon method is not meant to be a
 * (reverse) geocoder in the traditional sense. It is designed to
 * allow users to find photos for "places" and will round up to the
 * nearest place type to which corresponding place IDs apply.
 *
 * This API announced 2008-01-23
 * http://tech.groups.yahoo.com/group/yws-flickr/message/3735
 *
 * Implements flickr.places.findByLatLon (1.1)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_place*
flickcurl_places_findByLatLon(flickcurl* fc, double lat, double lon,
                              int accuracy)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place* place=NULL;
  char lat_str[20];
  char lon_str[20];
  char accuracy_str[4];
  
  if(accuracy < 0 || accuracy > 16)
    accuracy=16;
  
  sprintf(lat_str, "%f", lat);
  parameters[count][0]  = "lat";
  parameters[count++][1]= lat_str;
  sprintf(lon_str, "%f", lon);
  parameters[count][0]  = "lon";
  parameters[count++][1]= lon_str;
  sprintf(accuracy_str, "%d", accuracy);
  parameters[count][0]  = "accuracy";
  parameters[count++][1]= accuracy_str;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.findByLatLon", parameters, count))
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

  place=flickcurl_build_place(fc, xpathCtx,
                              (const xmlChar*)"/rsp/places/place");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    place=NULL;

  return place;
}


/**
 * flickcurl_places_resolvePlaceId:
 * @fc: flickcurl context
 * @place_id: A Flickr Places ID
 * 
 * Find Flickr Places information by Place Id
 *
 * Implements flickr.places.resolvePlaceId (1.0)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_place*
flickcurl_places_resolvePlaceId(flickcurl* fc, const char* place_id)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place* place=NULL;
  
  if(!place_id)
    return NULL;

  parameters[count][0]  = "place_id";
  parameters[count++][1]= place_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare_noauth(fc, "flickr.places.resolvePlaceId",
                              parameters, count))
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

  place=flickcurl_build_place(fc, xpathCtx,
                              (const xmlChar*)"/rsp/location");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    place=NULL;

  return place;
}


/**
 * flickcurl_places_resolvePlaceURL:
 * @fc: flickcurl context
 * @url: A Flickr Places URL.  Flickr Place URLs are of the form /country/region/city
 * 
 * Find Flickr Places information by Place URL
 *
 * Implements flickr.places.resolvePlaceURL (1.0)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_place*
flickcurl_places_resolvePlaceURL(flickcurl* fc, const char* url)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place* place=NULL;
  
  if(!url)
    return NULL;

  parameters[count][0]  = "url";
  parameters[count++][1]= url;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare_noauth(fc, "flickr.places.resolvePlaceURL",
                              parameters, count))
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

  place=flickcurl_build_place(fc, xpathCtx, (const xmlChar*)"/rsp/location");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    place=NULL;

  return place;
}


