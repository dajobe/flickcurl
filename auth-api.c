/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * auth-api.c - Flickr auth API calls
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
 * flickr.auth.checkToken:
 *
 * Get the credentials attached to an authentication token.
 * Must be signed.
 */


/*
 * flickr.auth.getFrob:
 *
 * Get a frob to be used during authentication.
 * Must be signed.
 */


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
  const char * parameters[10][2];
  int count=0;
  char *auth_token=NULL;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  
  parameters[count][0]   = "mini_token";
  parameters[count++][1] = (char*)frob;

  parameters[count][0]   = NULL;

  flickcurl_set_sig_key(fc, "api_sig");

  if(flickcurl_prepare(fc, "flickr.auth.getFullToken", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;
  
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    auth_token=flickcurl_xpath_eval(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/auth/token");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return auth_token;
}


/*
 * flickr.auth.getToken:
 *
 * Get the auth token for the given frob, if one has been attached. 
 * Must be signed.
 */
