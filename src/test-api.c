/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * test-api.c - Flickr flickr.test.* API calls
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
 * flickcurl_test_echo:
 * @fc: flickcurl context
 * @key: test key
 * @value: test value
 * 
 * A testing method which echo's all parameters back in the response.
 *
 * Actually prints the returned byte count to stderr.
 *
 * Implements flickr.test.echo (0.5)
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_test_echo(flickcurl* fc, const char* key, const char* value)
{
  const char * parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int rc=0;
  
  parameters[count][0]  = key;
  parameters[count++][1]= value;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.test.echo", parameters, count)) {
    rc=1;
    goto tidy;
  }

  doc=flickcurl_invoke(fc);
  if(!doc) {
    rc=1;
    goto tidy;
  }

  fprintf(stderr, "Flickr echo returned %d bytes\n", fc->total_bytes);
  
  tidy:
  
  return rc;
}


/**
 * flickcurl_test_login:
 * @fc: flickcurl context
 * 
 * A testing method which checks if the caller is logged in then
 * returns their username.
 *
 * Implements flickr.test.login (1.0)
 * 
 * Return value: username or NULL on failure
 **/
char*
flickcurl_test_login(flickcurl* fc)
{
  const char* parameters[7][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  char* username=NULL;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.test.login", parameters, count))
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

  username=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/user/username");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    username=NULL;

  return username;
}


/**
 * flickcurl_test_null:
 * @fc: flickcurl context
 * 
 * Null test
 *
 * Implements flickr.test.null (1.0)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_test_null(flickcurl* fc)
{
  const char* parameters[7][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.test.null", parameters, count))
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

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  return fc->failed;
}


