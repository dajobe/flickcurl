/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * galleries-api.c - Flickr flickr.galleries.* API calls
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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
 * flickcurl_galleries_addPhoto:
 * @fc: flickcurl context
 * @gallery_id: The ID of the gallery to add a photo to as returned by flickcurl_galleries_getList() and flickcurl_galleries_getListForPhoto().
 * @photo_id: The photo ID to add to the gallery.
 * @comment_text: A short comment or story to accompany the photo (or NULL).
 * 
 * Add a photo to a gallery.
 *
 * Implements flickr.galleries.addPhoto (1.17)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_galleries_addPhoto(flickcurl* fc, const char* gallery_id,
                             const char* photo_id, const char* comment_text)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  
  if(!gallery_id || !photo_id)
    return 1;

  parameters[count][0]  = "gallery_id";
  parameters[count++][1]= gallery_id;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  if(comment_text) {
    parameters[count][0]  = "comment";
    parameters[count++][1]= comment_text;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.galleries.addPhoto", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_galleries_getList:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to get a galleries list for. If none is specified, the calling user is assumed.
 * @per_page: Number of galleries to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Return the list of galleries created by a user.
 *
 * Galleries are returned sorted from newest to oldest.
 *
 * Implements flickr.galleries.getList (1.17)
 * 
 * Return value: array of galleries or NULL on failure
 **/
flickcurl_gallery**
flickcurl_galleries_getList(flickcurl* fc, const char* user_id,
                            int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_gallery** galleries = NULL;
  char page_str[10];
  char per_page_str[10];
  
  if(!user_id)
    return NULL;

  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;
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

  if(flickcurl_prepare(fc, "flickr.galleries.getList", parameters, count))
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

  galleries = flickcurl_build_galleries(fc, xpathCtx,
                                        (const xmlChar*)"/rsp/galleries/gallery",
                                        NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    galleries = NULL;

  return galleries;
}


/**
 * flickcurl_galleries_getListForPhoto:
 * @fc: flickcurl context
 * @photo_id: The ID of the photo to fetch a list of galleries for.
 * @per_page: Number of galleries to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Return the list of galleries to which a photo has been added.
 *
 * Galleries are returned sorted by date which the photo was added to
 * the gallery.
 *
 * Implements flickr.galleries.getListForPhoto (1.17)
 * 
 * Return value: array of galleries or NULL on failure
 **/
flickcurl_gallery**
flickcurl_galleries_getListForPhoto(flickcurl* fc, const char* photo_id,
                                    int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_gallery** galleries = NULL;
  char page_str[10];
  char per_page_str[10];
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
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

  if(flickcurl_prepare(fc, "flickr.galleries.getListForPhoto", parameters,
                       count))
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

  galleries = flickcurl_build_galleries(fc, xpathCtx,
                                        (const xmlChar*)"/rsp/galleries/gallery",
                                        NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    galleries = NULL;

  return galleries;
}
