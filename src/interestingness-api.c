/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * interestingness-api.c - Flickr flickr.interestingness.* API calls
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
 * flickcurl_interestingness_getList_params:
 * @fc: flickcurl context
 * @date: A specific date, formatted as YYYY-MM-DD, to return interesting photos for. (or NULL)
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns the list of interesting photos for the most recent day or a user-specified date.
 *
 * Optional extra type 'media' that will return an extra media = VALUE
 * for VALUE "photo" or "video".  API addition 2008-04-07.
 *
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_interestingness_getList_params(flickcurl* fc, const char* date,
                                         flickcurl_photos_list_params* list_params)
{
  const char* parameters[12][2];
  int count = 0;
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;

  /* API parameters */
  if(date) {
    parameters[count][0]  = "date";
    parameters[count++][1]= date;
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.interestingness.getList", parameters, count))
    goto tidy;

  photos_list = flickcurl_invoke_photos_list(fc,
                                             (const xmlChar*)"/rsp/photos",
                                             format);

  tidy:
  if(fc->failed) {
    if(photos_list)
      flickcurl_free_photos_list(photos_list);
    photos_list = NULL;
  }

  return photos_list;
}


/**
 * flickcurl_interestingness_getList:
 * @fc: flickcurl context
 * @date: A specific date, formatted as YYYY-MM-DD, to return interesting photos for. (or NULL)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. See #flickcurl_photos_list_params for the full list (or NULL)
 * @per_page: Number of photos to return per page default 100, max 500
 * @page: The page of results to return, default 1
 * 
 * Returns the list of interesting photos for the most recent day or a user-specified date.
 *
 * See flickcurl_interestingness_getList() for full description of arguments.
 *
 * Implements flickr.interestingness.getList (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photo**
flickcurl_interestingness_getList(flickcurl* fc, const char* date, const char* extras, int per_page, int page)
{
  flickcurl_photos_list_params list_params;
  flickcurl_photos_list* photos_list;
  flickcurl_photo** photos;
  
  memset(&list_params, '\0', sizeof(list_params));
  list_params.format   = NULL;
  list_params.extras   = extras;
  list_params.per_page = per_page;
  list_params.page     = page;

  photos_list = flickcurl_interestingness_getList_params(fc, date, &list_params);
  if(!photos_list)
    return NULL;

  photos = photos_list->photos; photos_list->photos = NULL;  
  /* photos array is now owned by this function */

  flickcurl_free_photos_list(photos_list);

  return photos;
}


