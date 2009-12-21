/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickr.prefs-api.c - Flickr flickr.prefs.* API calls
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
 * flickcurl_prefs_getContentType:
 * @fc: flickcurl context
 * 
 * Returns the default content type preference for the user.
 *
 * Implements flickr.prefs.getContentType (1.3)
 * 
 * Return value: content type or <0 on failure
 **/
int
flickcurl_prefs_getContentType(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char* content_type_str = NULL;
  int content_type= -1;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.prefs.getContentType", parameters, count))
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
  
  content_type_str = flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/person/@content_type");
  if(content_type_str) {
    content_type = atoi(content_type_str);
    free(content_type_str);
  }
  
  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    content_type= -1;

  return content_type;
}


/**
 * flickcurl_prefs_getGeoPerms:
 * @fc: flickcurl context
 * 
 * Returns the default privacy level for geographic information attached to the user's photos. 
 *
 * Possible values are: 0: no default, 1: public, 2: contacts only, 3: friends
 * and family only, 4: friends only, 5: family only, 6: private.
 *
 * Implements flickr.prefs.getGeoPerms (1.4)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_prefs_getGeoPerms(flickcurl* fc)
{
  char* v;
  int r= -1;

  v = flickcurl_call_get_one_string_field(fc, NULL, NULL,
                                        "flickr.prefs.getGeoPerms",
                                        (const xmlChar*)"/rsp/person/@geoperms");
  if(v) {
    r = atoi(v);
    free(v);
  }

  return r;
}


/**
 * flickcurl_prefs_getHidden:
 * @fc: flickcurl context
 * 
 * Returns the default hidden preference for the user.
 *
 * Implements flickr.prefs.getHidden (1.3)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_prefs_getHidden(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char* hidden_str = NULL;
  int hidden= -1;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.prefs.getHidden", parameters, count))
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

  hidden_str = flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/person/@hidden");
  if(hidden_str) {
    hidden = atoi(hidden_str);
    free(hidden_str);
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    hidden= -1;

  return hidden;
}


/**
 * flickcurl_prefs_getPrivacy:
 * @fc: flickcurl context
 * 
 * Returns the default privacy level preference for the user.
 * 
 * Possible values are: Public (1), Friends only (2),  Family only (3)
 * Friends and Family (4) and Private (5)
 *
 * Implements flickr.prefs.getPrivacy (1.3)
 * 
 * Return value: privacy level or <0 on failure
 **/
int
flickcurl_prefs_getPrivacy(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char* privacy_level_str= NULL;
  int privacy_level= -1;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.prefs.getPrivacy", parameters, count))
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

  privacy_level_str = flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/person/@privacy");
  if(privacy_level_str) {
    privacy_level = atoi(privacy_level_str);
    free(privacy_level_str);
  }


  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    privacy_level= -1;

  return privacy_level;
}


/**
 * flickcurl_prefs_getSafetyLevel:
 * @fc: flickcurl context
 * 
 * Returns the default safety level preference for the user.
 *
 * Implements flickr.prefs.getSafetyLevel (1.3)
 * 
 * Return value: safety level or <0 on failure
 **/
int
flickcurl_prefs_getSafetyLevel(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char* safety_level_str= NULL;
  int safety_level= -1;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.prefs.getSafetyLevel", parameters, count))
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

  safety_level_str = flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/person/@safety_level");
  if(safety_level_str) {
    safety_level = atoi(safety_level_str);
    free(safety_level_str);
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    safety_level= -1;

  return safety_level;
}


