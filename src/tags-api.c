/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * tags-api.c - Flickr flickr.tags.* API calls
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
 * flickcurl_tags_getClusterPhotos:
 * @fc: flickcurl context
 * @tag: The tag that this cluster belongs to.
 * @cluster_id: The top three tags for the cluster, separated by dashes (just like the url).
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Returns the first 24 photos for a given tag cluster
 *
 * Implements flickr.tags.getClusterPhotos (1.7)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_photos_list*
flickcurl_tags_getClusterPhotos(flickcurl* fc, const char* tag,
                                const char* cluster_id,
                                flickcurl_photos_list_params* list_params)
{
  const char* parameters[10][2];
  int count=0;
  flickcurl_photos_list* photos_list=NULL;
  const char* format=NULL;
  
  if(!tag || !cluster_id)
    return 1;

  parameters[count][0]  = "tag";
  parameters[count++][1]= tag;
  parameters[count][0]  = "cluster_id";
  parameters[count++][1]= cluster_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getClusterPhotos", parameters, count))
    goto tidy;

  photos_list=flickcurl_invoke_photos_list(fc,
                                           (const xmlChar*)"/rsp/photos/photo",
                                           format);
  tidy:
  if(fc->failed) {
    if(photos_list)
      flickcurl_free_photos_list(photos_list);
    photos_list=NULL;
  }

  return photos_list;
}



/**
 * flickcurl_tags_getClusters:
 * @fc: flickcurl context
 * @tag: The tag to fetch clusters for.
 * 
 * Gives you a list of tag clusters for the given tag.
 *
 * Implements flickr.tags.getClusters (1.5)
 *
 * "There is no pagination for this method as the number of clusters
 * for a single tag is capped at 5 and each cluster may contain
 * between 1 - 50 tags (give or take)."
 *
 * As announced 2008-07-17
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4218
 * 
 * Return value: NULL on failure
 **/
flickcurl_tag_clusters*
flickcurl_tags_getClusters(flickcurl* fc, const char* tag)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  void* result=NULL;
  
  if(!tag)
    return NULL;

  parameters[count][0]  = "tag";
  parameters[count++][1]= tag;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getClusters", parameters, count))
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

  result=flickcurl_build_tag_clusters(fc, xpathCtx,
                                      (const xmlChar*)"/rsp/clusters/cluster");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result=NULL;

  return result;
}


/**
 * flickcurl_tags_getHotList:
 * @fc: flickcurl context
 * @period: The period for which to fetch hot tags. Valid values are day and
 *   week (defaults to day).
 * @tag_count: The number of tags to return. Defaults to 20. Maximum allowed
 *   value is 200 (or -1 to leave as default)
 *
 * Returns a list of hot tags for the given period.
 *
 * Implements flickr.tags.getHotList (0.9)
 * 
 * Return value: array of #flickcurl_tag or NULL on failure
 **/
flickcurl_tag**
flickcurl_tags_getHotList(flickcurl* fc, const char* period, int tag_count)
{
  const char* parameters[7][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_tag** tags=NULL;
  char tag_count_str[10];
  
  if(period) {
    if(!strcmp(period, "day") || !strcmp(period, "week")) {
      parameters[count][0]  = "period";
      parameters[count++][1]= period;
    } else
      return NULL;
  }

  if(tag_count >= 0) {
    sprintf(tag_count_str, "%d", tag_count);
    parameters[count][0]  = "count";
    parameters[count++][1]= tag_count_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getHotList", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/hottags/tag", 
                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}


/**
 * flickcurl_tags_getListPhoto:
 * @fc: flickcurl context
 * @photo_id: photo ID
 *
 * Get the tag list for a given photo.
 *
 * Implements flickr.tags.getListPhoto (0.9)
 * 
 * Return value: array of #flickcurl_tag or NULL on failure
 **/
flickcurl_tag**
flickcurl_tags_getListPhoto(flickcurl* fc, const char* photo_id)
{
  const char* parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_tag** tags=NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getListPhoto", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/photo/tags/tag", 
                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}


/**
 * flickcurl_tags_getListUser:
 * @fc: flickcurl context
 * @user_id: user NSID (or NULL)
 *
 * Get the tag list for a given user (or current user)
 *
 * Implements flickr.tags.getListUser (0.9)
 * 
 * FIXME: API docs says user_id is optional but it is not.
 *
 * Return value: array of #flickcurl_tag or NULL on failure
 **/
flickcurl_tag**
flickcurl_tags_getListUser(flickcurl* fc, const char* user_id)
{
  const char* parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_tag** tags=NULL;

  if(user_id) {
    parameters[count][0]  = "user_id";
    parameters[count++][1]= user_id;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getListUser", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/who/tags/tag", 
                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}


/**
 * flickcurl_tags_getListUserPopular:
 * @fc: flickcurl context
 * @user_id: user NSID (or NULL)
 * @pop_count: number of popular tags to return (or <0 for default)
 *
 * Get the popular tag list for a given user (or current user)
 *
 * Implements flickr.tags.getListUserPopular (0.9)
 * 
 * Return value: array of #flickcurl_tag or NULL on failure
 **/
flickcurl_tag**
flickcurl_tags_getListUserPopular(flickcurl* fc, const char* user_id,
                                  int pop_count)
{
  const char* parameters[7][2];
  char pop_count_str[10];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_tag** tags=NULL;

  if(user_id) {
    parameters[count][0]  = "user_id";
    parameters[count++][1]= user_id;
  }
  if(pop_count >= 0) {
    sprintf(pop_count_str, "%d", pop_count);
    parameters[count][0]  = "count";
    parameters[count++][1]= pop_count_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getListUserPopular", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/who/tags/tag", 
                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}


/**
 * flickcurl_tags_getListUserRaw:
 * @fc: flickcurl context
 * @tag: tag to get raw version of (or NULL for all)
 *
 * Get the raw versions of a given tag (or all tags) for the currently logged-in user.
 *
 * Implements flickr.tags.getListUserRaw (0.9)
 * 
 * Return value: array of #flickcurl_tag or NULL on failure
 **/
flickcurl_tag**
flickcurl_tags_getListUserRaw(flickcurl* fc, const char* tag)
{
  const char* parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_tag** tags=NULL;

  if(tag) {
    parameters[count][0]  = "tag";
    parameters[count++][1]= tag;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getListUserRaw", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/who/tags/tag", 
                            NULL);
    
  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}


/**
 * flickcurl_tags_getRelated:
 * @fc: flickcurl context
 * @tag: tag to fetch related tags for
 *
 * Get a list of tags 'related' to the given tag, based on clustered usage analysis.
 *
 * Implements flickr.tags.getRelated (0.9)
 * 
 * Return value: array of #flickcurl_tag or NULL on failure
 **/
flickcurl_tag**
flickcurl_tags_getRelated(flickcurl* fc, const char* tag)
{
  const char* parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_tag** tags=NULL;

  if(!tag)
    return NULL;
  
  parameters[count][0]  = "tag";
  parameters[count++][1]= tag;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.tags.getRelated", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/tags/tag", 
                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}
