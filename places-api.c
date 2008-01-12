/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickr.places-api.c - Flickr flickr.places.* API calls
 *
 * Places API announced 2008-01-11
 * http://tech.groups.yahoo.com/group/yws-flickr/message/3688
 *
 * Copyright (C) 2008, David Beckett http://purl.org/net/dajobe/
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
 * flickcurl_places_resolvePlaceId:
 * @fc: flickcurl context
 * @place_id: A Flickr Places ID
 * 
 * Find Flickr Places information by Place Id
 *
 * Implements flickr.places.resolvePlaceId (0.14)
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

  place=flickcurl_build_place(fc, xpathCtx, (const xmlChar*)"/rsp");

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
 * Implements flickr.places.resolvePlaceURL (0.14)
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

  place=flickcurl_build_place(fc, xpathCtx, (const xmlChar*)"/rsp");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    place=NULL;

  return place;
}


