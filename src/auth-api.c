/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * auth-api.c - Flickr flickr.auth.* API calls
 *
 * Copyright (C) 2007-2012, David Beckett http://www.dajobe.org/
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
  char *perms = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 

  flickcurl_init_params(fc, 0);

  if(!token)
    return NULL;
  
  flickcurl_add_param(fc, "auth_token", (char*)token);

  flickcurl_end_params(fc);

  flickcurl_set_sign(fc);
  
  if(flickcurl_prepare(fc, "flickr.auth.checkToken"))
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
  char *frob = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  flickcurl_init_params(fc, 0);

  flickcurl_end_params(fc);

  flickcurl_set_sign(fc);
  
  if(flickcurl_prepare(fc, "flickr.auth.getFrob"))
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
  char *auth_token = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  flickcurl_init_params(fc, 0);

  flickcurl_add_param(fc, "mini_token", (char*)frob);

  flickcurl_end_params(fc);

  flickcurl_set_sign(fc);

  if(flickcurl_prepare(fc, "flickr.auth.getFullToken"))
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
  char *auth_token = NULL;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  flickcurl_init_params(fc, 0);

  flickcurl_add_param(fc, "frob", (char*)frob);

  flickcurl_end_params(fc);

  flickcurl_set_sign(fc);

  if(flickcurl_prepare(fc, "flickr.auth.getToken"))
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
 * flickcurl_auth_oauth_getAccessToken:
 * @fc: flickcurl context
 * 
 * Exchange tokens from the legacy Flickr auth to ones for OAuth
 *
 * Calling this method will delete the legacy auth tokens used to
 * make the request since they will expire within 24 hours of this
 * call.
 *
 * The OAuth token and secret should be saved and can be read from
 * flickcurl_get_oauth_token() and flickcurl_get_oauth_token_secret()
 *
 * Implements flickr.auth.oauth.getAccessToken (1.23)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_auth_oauth_getAccessToken(flickcurl* fc)
{
  flickcurl_oauth_data* od = &fc->od;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int rc = 0;
  
  flickcurl_init_params(fc, 0);

  flickcurl_end_params(fc);

  flickcurl_set_sign(fc);

  if(flickcurl_prepare(fc, "flickr.auth.oauth.getAccessToken"))
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

  if(xpathCtx) {
    char* auth_token;
    char* auth_token_secret;

    auth_token = flickcurl_xpath_eval(fc, xpathCtx,
                                      (const xmlChar*)"/rsp/auth/access_token[@oauth_token]");

    auth_token_secret = flickcurl_xpath_eval(fc, xpathCtx,
                                             (const xmlChar*)"/rsp/auth/access_token[@oauth_token_secret]");

#ifdef FLICKCURL_DEBUG
    fprintf(stderr,
            "Exchanged legacy auth tokens for OAuth access token '%s' secret token '%s'\n",
            auth_token, auth_token_secret);
#endif

    /* Old shared secret becomes OAuth client_secret (paired with client_key) */
    od->client_secret = fc->secret;
    fc->secret = NULL;

    /* Remove legacy Flickr auth token - not valid or needed */
    if(fc->auth_token) {
      free(fc->auth_token);
      fc->auth_token = NULL;
    }

    /* Store OAuth token and token secret in the oauth structure */
    od->token = auth_token;
    od->token_len = strlen(auth_token);
    od->token_secret = auth_token_secret;
    od->token_secret_len = strlen(auth_token_secret);

    xmlXPathFreeContext(xpathCtx);
    xpathCtx = NULL;
  }

  tidy:
  if(fc->failed)
    rc = 1;

  return rc;
}
