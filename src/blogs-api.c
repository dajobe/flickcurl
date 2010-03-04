/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * blogs-api.c - Flickr flickr.blogs.* API calls
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
 * flickcurl_blogs_getList:
 * @fc: flickcurl context
 * 
 * Get a list of configured blogs for the calling user.
 *
 * Implements flickr.blogs.getList (1.0)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_blog**
flickcurl_blogs_getList(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_blog** blogs = NULL;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.blogs.getList", parameters, count))
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

  blogs = flickcurl_build_blogs(fc, xpathCtx,
                              (const xmlChar*)"/rsp/blogs/blog", NULL);

  tidy:
  if(fc->failed)
    blogs = NULL;

  return blogs;
}


/**
 * flickcurl_blogs_getServices:
 * @fc: flickcurl context
 * 
 * Return a list of Flickr supported blogging services
 *
 * Implements flickr.blogs.getServices (1.12)
 * 
 * Return value: list of services or NULL on failure
 **/
flickcurl_blog_service**
flickcurl_blogs_getServices(flickcurl* fc)
{
  const char* parameters[7][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_blog_service **services = NULL;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.blogs.getServices", parameters, count))
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

  services = flickcurl_build_blog_services(fc, xpathCtx,
                                           (const xmlChar*)"/rsp/services/service", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    services = NULL;

  return services;
}


/**
 * flickcurl_blogs_postPhoto:
 * @fc: flickcurl context
 * @blog_id: The id of the blog to post to
 * @photo_id: The id of the photo to blog
 * @title: The blog post title
 * @description: The blog post body
 * @blog_password: The password for the blog (used when the blog does not have a stored password) (or NULL)
 *
 * Post a photo to a blog/
 *
 * Implements flickr.blogs.postPhoto (1.0)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_blogs_postPhoto(flickcurl* fc, const char* blog_id,
                          const char* photo_id, const char* title,
                          const char* description, const char* blog_password)
{
  const char* parameters[12][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  
  if(!blog_id || !photo_id || !title || !description)
    return 1;

  parameters[count][0]  = "blog_id";
  parameters[count++][1]= blog_id;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "title";
  parameters[count++][1]= title;
  parameters[count][0]  = "description";
  parameters[count++][1]= description;
  if(blog_password) {
    parameters[count][0]  = "blog_password";
    parameters[count++][1]= blog_password;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.blogs.postPhoto", parameters, count))
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

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  return fc->failed;
}
