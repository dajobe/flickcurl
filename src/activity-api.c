/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * activity-api.c - Flickr flickr.activity.* API calls
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
 * flickcurl_activity_userComments:
 * @fc: flickcurl context
 * @per_page: Number of items to return per page. If this argument is omitted, it defaults to 10. The maximum allowed value is 50.
 * @page: The page of results to return. If this argument is omitted, it defaults to 1.
 * 
 * Returns a list of recent activity on photos commented on by the calling user. 
 *
 * Implements flickr.activity.userComments (1.0)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_activity**
flickcurl_activity_userComments(flickcurl* fc, int per_page, int page)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_activity** activities=NULL;
  char page_str[10];
  char per_page_str[10];
  
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

  if(flickcurl_prepare(fc, "flickr.activity.userComments", parameters, count))
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

  activities=flickcurl_build_activities(fc, xpathCtx,
                                        (const xmlChar*)"/rsp/items/item", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    activities=NULL;

  return activities;
}


/**
 * flickcurl_activity_userPhotos:
 * @fc: flickcurl context
 * @timeframe: The timeframe in which to return updates for. This can be specified in days (<code>'2d'</code>) or hours (<code>'4h'</code>). The default behavoir is to return changes since the beginning of the previous user session. (or NULL)
 * @per_page: Number of items to return per page. If this argument is omitted, it defaults to 10. The maximum allowed value is 50. (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or NULL)
 * 
 * Returns a list of recent activity on photos belonging to the calling user. Do not poll this method more than once an hour.
 *
 * Implements flickr.activity.userPhotos (1.0)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_activity**
flickcurl_activity_userPhotos(flickcurl* fc, const char* timeframe,
                              int per_page, int page)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_activity** activities=NULL;
  char page_str[10];
  char per_page_str[10];

  if(timeframe) {
    parameters[count][0]  = "timeframe";
    parameters[count++][1]= timeframe;
  }
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

  if(flickcurl_prepare(fc, "flickr.activity.userPhotos", parameters, count))
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

  activities=flickcurl_build_activities(fc, xpathCtx,
                                        (const xmlChar*)"/rsp/items/item", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    activities=NULL;

  return activities;
}


