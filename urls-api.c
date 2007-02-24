/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * urls-api.c - Flickr flickr.urls.* API calls
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


/*
 * flickr.urls.getGroup:
 *
 * Get the url to a group's page.
 */


/*
 * flickr.urls.getUserPhotos:
 *
 * Get the url to a user's photos.
 */


/*
 * flickr.urls.getUserProfile:
 *
 * Get the url to a user's profile.
 */


/*
 * flickr.urls.lookupGroup:
 *
 * Get a group NSID, given the url to a group's page or photo pool.
 */


/**
 * flickcurl_urls_lookupUser - 
 * @fc: flickcurl context
 * @url: URL of user's photo or user's profile
 * 
 * Get a user NSID, given the url to a user's photos or profile.
 *
 * Implements flickr.urls.lookupUser (0.6)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_urls_lookupUser(flickcurl* fc, const char* url)
{
  const char * parameters[5][2];
  int count=0;
  char *nsid=NULL;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 

  if(!url)
    return NULL;
  
  parameters[count][0]  = "url";
  parameters[count++][1]= url;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.urls.lookupUser", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    nsid=flickcurl_xpath_eval(fc, xpathCtx,
                              (const xmlChar*)"/rsp/user/@id");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return nsid;
}
