/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * places-api.c - Flickr flickr.places.* API calls
 *
 * Places API announced 2008-01-11
 * http://tech.groups.yahoo.com/group/yws-flickr/message/3688
 *
 * Copyright (C) 2008-2009, David Beckett http://www.dajobe.org/
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
 * Return value: array of places or NULL on failure
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
 * flickcurl_places_getChildrenWithPhotosPublic:
 * @fc: flickcurl context
 * @place_id: A Places ID. (While optional, you must pass either a valid Places ID or a WOE ID.) (or NULL)
 * @woe_id: A Where On Earth (WOE) ID. (While optional, you must pass either a valid Places ID or a WOE ID.) (or NULL)
 * 
 * Return a list of locations with public photos that are parented by a Where on Earth (WOE) or Places ID.
 *
 * Implements flickr.places.getChildrenWithPhotosPublic (1.7)
 * 
 * @deprecated: Replaced by flickcurl_places_getChildrenWithPhotosPublic2() with integer woe_id argument.
 *
 * Return value: array of places or NULL on failure
 **/
flickcurl_place**
flickcurl_places_getChildrenWithPhotosPublic(flickcurl* fc,
                                             const char* place_id,
                                             const char* woe_id)
{
  int woe_id_i = -1;
  if(woe_id)
    woe_id_i = atoi(woe_id);
  return flickcurl_places_getChildrenWithPhotosPublic2(fc, place_id, woe_id_i);
}



/**
 * flickcurl_places_getChildrenWithPhotosPublic2:
 * @fc: flickcurl context
 * @place_id: A Places ID (or NULL)
 * @woe_id: A Where On Earth (WOE) ID (or <0)
 * 
 * Return a list of locations with public photos that are parented by a Where on Earth (WOE) or Places ID.
 *
 * You must pass either a valid Places ID or a WOE ID.
 *
 * Replaces flickcurl_places_getChildrenWithPhotosPublic() with integer @woe_id arg.
 * 
 * Return value: array of places or NULL on failure
 **/
flickcurl_place**
flickcurl_places_getChildrenWithPhotosPublic2(flickcurl* fc,
                                              const char* place_id, int woe_id)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place** places=NULL;
  char woe_id_str[10];

  if(place_id) {
    parameters[count][0]  = "place_id";
    parameters[count++][1]= place_id;
  } else if(woe_id >= 0) {
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count][0]  = "woe_id";
    parameters[count++][1]= woe_id_str;
  } else
    return NULL;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.getChildrenWithPhotosPublic",
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

  places=flickcurl_build_places(fc, xpathCtx,
                                (const xmlChar*)"/rsp/places/place", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    places=NULL;

  return places;
}


/**
 * flickcurl_places_getInfo:
 * @fc: flickcurl context
 * @place_id: A Places ID (or NULL)
 * @woe_id: A Where On Earth (WOE) ID. (or NULL)
 * 
 * Get information about a place.
 *
 * While optional, you must pass either a valid Places ID or a WOE ID.
 *
 * Implements flickr.places.getInfo (1.7)
 * 
 * Announced 2008-10-30
 * http://code.flickr.com/blog/2008/10/30/the-shape-of-alpha/
 * and in detail 2008-11-05
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4510
 * 
 * @deprecated: Replaced by flickcurl_places_getInfo2() with integer woe_id argument.
 *
 * Return value: new place object or NULL on failure
 **/
flickcurl_place* flickcurl_places_getInfo(flickcurl* fc,
                                          const char* place_id,
                                          const char* woe_id)
{
  int woe_id_i = -1;
  if(woe_id)
    woe_id_i = atoi(woe_id);
  return flickcurl_places_getInfo2(fc, place_id, woe_id_i);
}


/**
 * flickcurl_places_getInfo2:
 * @fc: flickcurl context
 * @place_id: A Places ID. (or NULL)
 * @woe_id: A Where On Earth (WOE) ID (or <0)
 * 
 * Get information about a place.
 *
 * While optional, you must pass either a valid Places ID or a WOE ID.
 *
 * Replaces flickcurl_places_getInfo() with integer woe_id argument.
 *
 * Return value: new place object or NULL on failure
 **/
flickcurl_place*
flickcurl_places_getInfo2(flickcurl* fc, const char* place_id, int woe_id)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place* place=NULL;
  char woe_id_str[10];

  if(place_id) {
    parameters[count][0]  = "place_id";
    parameters[count++][1]= place_id;
  } else if(woe_id >= 0) {
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count][0]  = "woe_id";
    parameters[count++][1]= woe_id_str;
  } else
    return NULL;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare_noauth(fc, "flickr.places.getInfo", parameters, count))
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

  place=flickcurl_build_place(fc, xpathCtx, (const xmlChar*)"/rsp/place");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    place=NULL;

  return place;
}


/**
 * flickcurl_places_getInfoByUrl:
 * @fc: flickcurl context
 * @url: A flickr.com/places URL in the form of /country/region/city. For example: /Canada/Quebec/Montreal
 * 
 * Lookup information about a place, by its flickr.com/places URL.
 *
 * Implements flickr.places.getInfoByUrl (1.7)
 *
 * Announced 2008-10-30
 * http://code.flickr.com/blog/2008/10/30/the-shape-of-alpha/
 * and in detail 2008-11-05
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4510
 * 
 * Return value: new place object or NULL on failure
 **/
flickcurl_place*
flickcurl_places_getInfoByUrl(flickcurl* fc, const char* url)
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

  if(flickcurl_prepare(fc, "flickr.places.getInfoByUrl", parameters, count))
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

  place=flickcurl_build_place(fc, xpathCtx, (const xmlChar*)"/rsp/place");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    place=NULL;

  return place;
}


/**
 * flickcurl_places_getPlaceTypes:
 * @fc: flickcurl context
 * 
 * Get a list of available place types
 *
 * Implements flickr.places.getPlaceTypes (1.8)
 * 
 * Return value: array of #flickcurl_place_type_info or NULL on failure
 **/
flickcurl_place_type_info**
flickcurl_places_getPlaceTypes(flickcurl* fc)
{
  const char* parameters[7][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place_type_info** place_types=NULL;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.getPlaceTypes", parameters, count))
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

  place_types = flickcurl_build_place_types(fc, xpathCtx,
                                            (const xmlChar*)"/rsp/place_types/place",
                                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    place_types = NULL;

  return place_types;
}


/**
 * flickcurl_places_getShapeHistory:
 * @fc: flickcurl context
 * @place_id: A Flickr Places ID (or NULL)
 * @woe_id: A Where On Earth (WOE) ID (or <0)
 * 
 * Return an historical list of all the shape data generated for a
 * Places or Where on Earth (WOE) ID.
 *
 * While optional, you must pass either a valid Places ID or a WOE ID.
 *
 * Implements flickr.places.getShapeHistory (1.8)
 * 
 * Announced 2009-01-12 in
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4669
 * 
 * Return value: NULL on failure
 **/
flickcurl_shapedata**
flickcurl_places_getShapeHistory(flickcurl* fc, const char* place_id,
                                 int woe_id)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_shapedata** shapes=NULL;
  char woe_id_str[20];
  
  if(!place_id && woe_id < 0)
    return NULL;

  if(place_id) {
    parameters[count][0]  = "place_id";
    parameters[count++][1]= place_id;
  }
  if(woe_id >=0) {
    parameters[count][0]  = "woe_id";
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count++][1]= woe_id_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.getShapeHistory", parameters, count))
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

  shapes = flickcurl_build_shapes(fc, xpathCtx,
                                  (const xmlChar*)"/rsp/shapes/shapedata",
                                  NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    shapes = NULL;

  return shapes;
}


/**
 * flickcurl_places_placesForBoundingBox:
 * @fc: flickcurl context
 * @place_type: The place type to cluster photos by
 * @minimum_longitude: Bound Box bottom-left corner longitude
 * @minimum_latitude: Bound Box bottom-left corner latitude
 * @maximum_longitude: Bound Box top-right corner longitude
 * @maximum_latitude: Bound Box top-right corner latitude
 * 
 * Return all the locations of a matching place type for a bounding box.
 *
 * The maximum allowable size of a bounding box (the distance between
 * the SW and NE corners) is governed by the place type you are
 * requesting. Allowable sizes are as follows:
 * neighbourhood: 3km (1.8mi), locality: 7km (4.3mi), county: 50km (31mi),
 * region: 200km (124mi), country: 500km (310mi), continent: 1500km (932mi)
 *
 * Implements flickr.places.placesForBoundingBox (1.8)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_place**
flickcurl_places_placesForBoundingBox(flickcurl* fc,
                                      flickcurl_place_type place_type,
                                      double minimum_longitude,
                                      double minimum_latitude,
                                      double maximum_longitude,
                                      double maximum_latitude)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place** places=NULL;
  char place_type_id_str[3];
  int place_type_id = -1;
  char bbox[255];

  place_type_id = flickcurl_place_type_to_id(place_type);
  if(place_type_id < 0)
    return NULL;

  sprintf(bbox, "%f,%f,%f,%f",  minimum_longitude, minimum_latitude,
          maximum_longitude, maximum_latitude);
  parameters[count][0]  = "bbox";
  parameters[count++][1]= bbox;
  /* deliberately not using deprecated parameter place_type */
/*
  parameters[count][0]  = "place_type";
  parameters[count++][1]= place_type;
*/
  parameters[count][0]  = "place_type_id";
  sprintf(place_type_id_str, "%d", place_type_id);
  parameters[count++][1]= place_type_id_str;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.placesForBoundingBox", parameters,
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

  places = flickcurl_build_places(fc, xpathCtx,
                                  (const xmlChar*)"/rsp/places/place", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    places = NULL;

  return places;
}


/**
 * flickcurl_places_placesForContacts:
 * @fc: flickcurl context
 * @place_type: A specific place type to cluster photos by.
 * @woe_id: A Where on Earth ID to use to filter photo clusters (or NULL)
 * @place_id: A Places ID to use to filter photo clusters (or NULL)
 * @threshold: The minimum number of photos that a place type must have to be included. If the number of photos is lowered then the parent place type for that place will be used.
 * @contacts: Search your contacts. Either 'all' or 'ff' for just friends and family. (Default is 'all') (or NULL)
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned (or <0)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned (or <0)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned (or <0)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned (or <0)
 * 
 * Return a list of the top 100 unique places clustered by a given
 * placetype for a user's contacts.
 *
 * One of @woe_id or @place_id must be given.
 *
 * Implements flickr.places.placesForContacts (1.8)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_place**
flickcurl_places_placesForContacts(flickcurl* fc,
                                   flickcurl_place_type place_type,
                                   int woe_id,
                                   const char* place_id,
                                   int threshold,
                                   const char* contacts,
                                   int min_upload_date,
                                   int max_upload_date,
                                   int min_taken_date,
                                   int max_taken_date)
{
  const char* parameters[17][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place** places=NULL;
  char place_type_id_str[3];
  int place_type_id;
  char min_upload_date_s[20];
  char max_upload_date_s[20];
  char min_taken_date_s[20];
  char max_taken_date_s[20];
  char woe_id_str[10];
  char threshold_str[10];
  
  if(!woe_id && !place_id)
    return NULL;
  
  place_type_id = flickcurl_place_type_to_id(place_type);
  if(place_type_id < 0)
    return NULL;
  
  /* deliberately not using deprecated parameter place_type */
/*  
  parameters[count][0]  = "place_type";
  parameters[count++][1]= place_type;
*/
  parameters[count][0]  = "place_type_id";
  sprintf(place_type_id_str, "%d", place_type_id);
  parameters[count++][1]= place_type_id_str;
  if(woe_id >= 0) {
    parameters[count][0]  = "woe_id";
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count++][1]= woe_id_str;
  }
  if(place_id) {
    parameters[count][0]  = "place_id";
    parameters[count++][1]= place_id;
  }
  parameters[count][0]  = "threshold";
  sprintf(threshold_str, "%d", threshold);
  parameters[count++][1]= threshold_str;

  if(contacts) {
    parameters[count][0]  = "contacts";
    parameters[count++][1]= contacts;
  }
  if(min_upload_date >= 0) {
    sprintf(min_upload_date_s, "%d", min_upload_date);
    parameters[count][0]  = "min_upload_date";
    parameters[count++][1]= min_upload_date_s;
  }
  if(max_upload_date >= 0) {
    sprintf(max_upload_date_s, "%d", max_upload_date);
    parameters[count][0]  = "max_upload_date";
    parameters[count++][1]= max_upload_date_s;
  }
  if(min_taken_date >= 0) {
    sprintf(min_taken_date_s, "%d", min_taken_date);
    parameters[count][0]  = "min_taken_date";
    parameters[count++][1]= min_taken_date_s;
  }
  if(max_taken_date >= 0) {
    sprintf(max_taken_date_s, "%d", max_taken_date);
    parameters[count][0]  = "max_taken_date";
    parameters[count++][1]= max_taken_date_s;
  }
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.placesForContacts", parameters,
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

  places = flickcurl_build_places(fc, xpathCtx,
                                  (const xmlChar*)"/rsp/places/place", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    places = NULL;

  return places;
}


/**
 * flickcurl_places_placesForTags:
 * @fc: flickcurl context
 * @place_type: The place type to cluster photos by
 * @woe_id: A Where on Earth ID to use to filter photo clusters (or NULL)
 * @place_id: A Places ID to use to filter photo clusters (or NULL)
 * @threshold: The minimum number of photos that a place type must have to be included. If the number of photos is lowered then the parent place type for that place will be used.
 * @tags: A comma-delimited list of tags. Photos with one or more of the tags listed will be returned. (or NULL)
 * @tag_mode: Either 'any' for an OR combination of tags, or 'all' for an AND combination. Defaults to 'any' if not specified. (or NULL)
 * @machine_tags: Multiple machine tags may be queried by passing a comma-separated list. The number of machine tags you can pass in a single query depends on the tag mode (AND or OR) that you are querying with. "AND" queries are limited to (16) machine tags. "OR" queries are limited to (8). See below. (or NULL)
 * @machine_tag_mode: Either 'any' for an OR combination of tags, or 'all' for an AND combination. Defaults to 'any' if not specified. (or NULL)
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned (or NULL)
 * 
 * Return a list of the top 100 unique places clustered by a given
 * placetype for set of tags or machine tags.
 *
 * Machine tags extra information.  Aside from passing in a fully
 * formed machine tag, there is a special syntax for searching on
 * specific properties :
 *
 * <itemizedlist>
 *   <listitem>Find photos using the 'dc' namespace : <literal>"machine_tags" => "dc:"</literal></listitem>
 *   <listitem> Find photos with a title in the 'dc' namespace : <literal>"machine_tags" => "dc:title="</literal></listitem>
 *   <listitem>Find photos titled "mr. camera" in the 'dc' namespace : <literal>"machine_tags" => "dc:title=\"mr. camera\"</literal></listitem>
 *   <listitem>Find photos whose value is "mr. camera" : <literal>"machine_tags" => "*:*=\"mr. camera\""</literal></listitem>
 *   <listitem>Find photos that have a title, in any namespace : <literal>"machine_tags" => "*:title="</literal></listitem>
 *   <listitem>Find photos that have a title, in any namespace, whose value is "mr. camera" : <literal>"machine_tags" => "*:title=\"mr. camera\""</literal></listitem>
 *   <listitem>Find photos, in the 'dc' namespace whose value is "mr. camera" : <literal>"machine_tags" => "dc:*=\"mr. camera\""</literal></listitem>
 *  </itemizedlist>
 * 
 * Implements flickr.places.placesForTags (1.8)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_places_placesForTags(flickcurl* fc,
                               flickcurl_place_type place_type,
                               int woe_id,
                               const char* place_id,
                               const char* threshold,
                               const char* tags, const char* tag_mode,
                               const char* machine_tags, const char* machine_tag_mode,
                               const char* min_upload_date, const char* max_upload_date,
                               const char* min_taken_date, const char* max_taken_date)
{
  const char* parameters[19][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  void* result=NULL;
  char place_type_id_str[3];
  int place_type_id;
  char woe_id_str[10];
  
  place_type_id = flickcurl_place_type_to_id(place_type);
  if(place_type_id < 0)
    return 1;

  parameters[count][0]  = "place_type_id";
  sprintf(place_type_id_str, "%d", place_type_id);
  parameters[count++][1]= place_type_id_str;
  parameters[count][0]  = "woe_id";
  sprintf(woe_id_str, "%d", woe_id);
  parameters[count++][1]= woe_id_str;
  parameters[count][0]  = "place_id";
  parameters[count++][1]= place_id;
  parameters[count][0]  = "threshold";
  parameters[count++][1]= threshold;
  parameters[count][0]  = "tags";
  parameters[count++][1]= tags;
  parameters[count][0]  = "tag_mode";
  parameters[count++][1]= tag_mode;
  parameters[count][0]  = "machine_tags";
  parameters[count++][1]= machine_tags;
  parameters[count][0]  = "machine_tag_mode";
  parameters[count++][1]= machine_tag_mode;
  parameters[count][0]  = "min_upload_date";
  parameters[count++][1]= min_upload_date;
  parameters[count][0]  = "max_upload_date";
  parameters[count++][1]= max_upload_date;
  parameters[count][0]  = "min_taken_date";
  parameters[count++][1]= min_taken_date;
  parameters[count][0]  = "max_taken_date";
  parameters[count++][1]= max_taken_date;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.placesForTags", parameters, count))
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

  result=NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result=NULL;

  return (result == NULL);
}


/**
 * flickcurl_places_resolvePlaceId:
 * @fc: flickcurl context
 * @place_id: A Places ID
 * 
 * Find places information by Place ID
 *
 * Implements flickr.places.resolvePlaceId (1.0)
 * 
 * Return value: new place object or NULL on failure
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
 * @url: A Places URL.  Place URLs are of the form /country/region/city
 * 
 * Find Places information by Place URL
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


/**
 * flickcurl_places_placesForUser:
 * @fc: flickcurl context
 * @place_type: A specific place type to cluster photos by.  Valid places types are neighbourhood, locality, region or country
 * @woe_id: A Where on Earth ID to use to filter photo clusters. (or <0)
 * @place_id: A Places ID to use to filter photo clusters. (or NULL)
 * @threshold: The minimum number of photos that a place type must have to be included. If the number of photos is lowered then the parent place type for that place will be used. (or <0)
 * 
 * Return a list of the top 100 unique places clustered by a given place type for a user.
 *
 * This API added 2008-09-04 as announced in
 * http://code.flickr.com/blog/2008/09/04/whos-on-first/
 *
 * Implements flickr.places.placesForUser (1.6)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_place**
flickcurl_places_placesForUser(flickcurl* fc,
                               flickcurl_place_type place_type,
                               int woe_id, const char* place_id,
                               int threshold)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_place** places=NULL;
  const char* place_type_str;
  char woe_id_str[20];
  char threshold_str[4];

  place_type_str=flickcurl_get_place_type_label(place_type);
  if(!place_type_str) {
    flickcurl_error(fc, "Invalid place type %d", place_type);
    return NULL;
  }

  if(place_type != FLICKCURL_PLACE_NEIGHBOURHOOD &&
     place_type != FLICKCURL_PLACE_LOCALITY &&
     place_type != FLICKCURL_PLACE_REGION &&
     place_type != FLICKCURL_PLACE_COUNTRY) {
    flickcurl_error(fc, "Place type '%s' (%d) is not valid for places.forUser",
                    place_type_str, place_type);
    return NULL;
  }

  parameters[count][0]  = "place_type";
  parameters[count++][1]= place_type_str;

  if(woe_id >= 0) {
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count][0]  = "woe_id";
    parameters[count++][1]= woe_id_str;
  }

  if(place_id) {
    parameters[count][0]  = "place_id";
    parameters[count++][1]= place_id;
  }

  if(threshold >=0) {
    sprintf(threshold_str, "%d", threshold);
    parameters[count][0]  = "threshold";
    parameters[count++][1]= threshold_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.placesForUser", parameters, count))
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
 * flickcurl_places_forUser:
 * @fc: flickcurl context
 * @place_type: A specific place type to cluster photos by.  Valid places types are neighbourhood, locality, region or country
 * @woe_id: A Where on Earth ID to use to filter photo clusters. (or <0)
 * @place_id: A Places ID to use to filter photo clusters. (or NULL)
 * @threshold: The minimum number of photos that a place type must have to be included. If the number of photos is lowered then the parent place type for that place will be used. (or <0)
 * 
 * Return a list of the top 100 unique places clustered by a given place type for a user.
 *
 * @deprecated: Use flickcurl_places_placesForUser()
 *
 * Return value: non-0 on failure
 **/
flickcurl_place**
flickcurl_places_forUser(flickcurl* fc, flickcurl_place_type place_type,
                         int woe_id, const char *place_id, int threshold)
{
  return flickcurl_places_placesForUser(fc, place_type, woe_id, place_id,
                                        threshold);
}


/**
 * flickcurl_places_tagsForPlace:
 * @fc: flickcurl context
 * @woe_id: A Where on Earth identifier to use to filter photo clusters (or <0)
 * @place_id: A Flickr Places identifier to use to filter photo clusters (or NULL)
 * @min_upload_date: Minimum upload date. Photos with an upload date greater than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @max_upload_date: Maximum upload date. Photos with an upload date less than or equal to this value will be returned. The date should be in the form of a unix timestamp. (or NULL)
 * @min_taken_date: Minimum taken date. Photos with an taken date greater than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * @max_taken_date: Maximum taken date. Photos with an taken date less than or equal to this value will be returned. The date should be in the form of a mysql datetime. (or NULL)
 * 
 * Return a list of the top 100 unique tags for a Flickr Places or
 * Where on Earth (WOE) ID
 *
 * (While optional, you must pass either a valid Places ID or a WOE ID.)
*
 * Implements flickr.places.tagsForPlace (1.8)
 * 
 * Return value: NULL on failure
 **/
flickcurl_tag**
flickcurl_places_tagsForPlace(flickcurl* fc, int woe_id, const char* place_id,
                              int min_upload_date, int max_upload_date,
                              int min_taken_date, int max_taken_date)
{
  const char* parameters[13][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  char woe_id_str[20];
  char min_upload_date_str[20];
  char max_upload_date_str[20];
  char min_taken_date_str[20];
  char max_taken_date_str[20];
  flickcurl_tag** tags=NULL;
  
  if(woe_id < 0 && !place_id)
    return NULL;

  if(woe_id >= 0) {
    parameters[count][0]  = "woe_id";
    sprintf(woe_id_str, "%d", woe_id);
    parameters[count++][1]= woe_id_str;
  }
  if(place_id) {
    parameters[count][0]  = "place_id";
    parameters[count++][1]= place_id;
  }
  if(min_upload_date) {
    parameters[count][0]  = "min_upload_date";
    sprintf(min_upload_date_str, "%d", min_upload_date);
    parameters[count++][1]= min_upload_date_str;
  }
  if(min_upload_date) {
    parameters[count][0]  = "max_upload_date";
    sprintf(min_upload_date_str, "%d", min_upload_date);    
    parameters[count++][1]= max_upload_date_str;
  }
  if(max_upload_date) {
    parameters[count][0]  = "min_taken_date";
    sprintf(max_upload_date_str, "%d", max_upload_date);    
    parameters[count++][1]= min_taken_date_str;
  }
  if(min_taken_date) {
    parameters[count][0]  = "max_taken_date";
    sprintf(min_taken_date_str, "%d", min_taken_date);    
    parameters[count++][1]= max_taken_date_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.places.tagsForPlace", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/tags/tag", 
                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}
