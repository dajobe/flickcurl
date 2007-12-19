/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * interestingness-api.c - Flickr flickr.interestingness.* API calls
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


/**
 * flickcurl_interestingness_getList:
 * @fc: flickcurl context
 * @date: A specific date, formatted as YYYY-MM-DD, to return interesting photos for. (or NULL)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: <code>license</code>, <code>date_upload</code>, <code>date_taken</code>, <code>owner_name</code>, <code>icon_server</code>, <code>original_format</code>, <code>last_update</code>, <code>geo</code>, <code>tags</code>, <code>machine_tags</code>. (or NULL)
 * @per_page: Number of photos to return per page default 100, max 500
 * @page: The page of results to return, default 1
 * 
 * Returns the list of interesting photos for the most recent day or a user-specified date.
 *
 * Implements flickr.interestingness.getList (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_interestingness_getList(flickcurl* fc, const char* date, const char* extras, int per_page, int page)
{
  const char* parameters[11][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  char page_str[10];
  char per_page_str[10];
  flickcurl_photo** photos=NULL;

  if(date) {
    parameters[count][0]  = "date";
    parameters[count++][1]= date;
  }
  if(extras) {
    parameters[count][0]  = "extras";
    parameters[count++][1]= extras;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.interestingness.getList", parameters, count))
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

  photos=flickcurl_build_photos(fc, xpathCtx,
                                (const xmlChar*)"/rsp/photos/photo", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    photos=NULL;

  return photos;
}


