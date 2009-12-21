/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * auth-api.c - Flickr flickr.auth.* API calls
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
 * flickcurl_auth_checkToken:
 * @fc: flickcurl context
 * @token: token string
 * 
 * Get the credentials attached to an authentication token.
 *
 * Implements flickr.auth.checkToken (0.9)
 * Must be signed.
 * 
 * FIXME: Cannot confirm this works, get intermittent results.
 *
 * Return value: permissions string or NULL on failure
 **/
char*
flickcurl_auth_checkToken(flickcurl* fc, const char* token)
{
  const char * parameters[6][2];
  int count = 0;
  char *perms = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 

  if(!token)
    return NULL;
  
  parameters[count][0]   = "auth_token";
  parameters[count++][1] = (char*)token;

  parameters[count][0]   = NULL;

  flickcurl_set_sign(fc);
  
  if(flickcurl_prepare(fc, "flickr.auth.checkToken", parameters, count))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;
  
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    perms = flickcurl_xpath_eval(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/auth/perms");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return perms;
}


/**
 * flickcurl_auth_getFrob:
 * @fc: flickcurl context
 * 
 * Get a frob to be used during authentication
 *
 * Implements flickr.auth.getFrob (0.9)
 * Must be signed.  Does not require authentication.
 * 
 * Return value: frob string or NULL on failure
 **/
char*
flickcurl_auth_getFrob(flickcurl* fc)
{
  const char * parameters[5][2];
  int count = 0;
  char *frob = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  parameters[count][0]   = NULL;

  flickcurl_set_sign(fc);
  
  if(flickcurl_prepare(fc, "flickr.auth.getFrob", parameters, count))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;
  
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    frob = flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/frob");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return frob;
}


/**
 * flickcurl_auth_getFullToken:
 * @fc: flickcurl context
 * @frob: frob string
 * 
 * Turn a frob into an auth_token
 *
 * Implements flickr.auth.getFullToken (0.5)
 * Must be signed.
 * 
 * Return value: token string or NULL on failure
 **/
char*
flickcurl_auth_getFullToken(flickcurl* fc, const char* frob)
{
  const char * parameters[6][2];
  int count = 0;
  char *auth_token = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  parameters[count][0]   = "mini_token";
  parameters[count++][1] = (char*)frob;

  parameters[count][0]   = NULL;

  flickcurl_set_sign(fc);

  if(flickcurl_prepare(fc, "flickr.auth.getFullToken", parameters, count))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;
  
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    auth_token = flickcurl_xpath_eval(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/auth/token");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return auth_token;
}


/**
 * flickcurl_auth_getToken:
 * @fc: flickcurl context
 * @frob: frob string
 * 
 * Get the auth token for the given frob, if one has been attached.
 *
 * Implements flickr.auth.getToken (0.9)
 * Must be signed.
 * 
 * Return value: token string or NULL on failure
 **/
char*
flickcurl_auth_getToken(flickcurl* fc, const char* frob)
{
  const char * parameters[6][2];
  int count = 0;
  char *auth_token = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  parameters[count][0]   = "frob";
  parameters[count++][1] = (char*)frob;

  parameters[count][0]   = NULL;

  flickcurl_set_sign(fc);

  if(flickcurl_prepare(fc, "flickr.auth.getToken", parameters, count))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;
  
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    auth_token = flickcurl_xpath_eval(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/auth/token");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return auth_token;
}
