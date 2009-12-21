/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * commons-api.c - Flickr flickr.commons.* API calls
 *
 * Commons API announced 2009-01-29
 * http://flickr.com/groups/api/discuss/72157613093793775/
 *
 * Copyright (C) 2009, David Beckett http://www.dajobe.org/
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
 * flickcurl_commons_getInstitutions:
 * @fc: flickcurl context
 * 
 * Retrieves a list of the current Commons institutions.
 *
 * Implements flickr.commons.getInstitutions (1.8)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_institution**
flickcurl_commons_getInstitutions(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_institution** institutions = NULL;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.commons.getInstitutions", parameters, count))
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

  institutions = flickcurl_build_institutions(fc, xpathCtx,
                                              (const xmlChar*)"/rsp/institutions/institution",
                                              NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    institutions = NULL;

  return institutions;
}
