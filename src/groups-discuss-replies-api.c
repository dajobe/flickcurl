/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * groups-discuss-replies-api.c - Flickr flickr.groups.discuss.replies.* API calls
 *
 * Copyright (C) 2012, David Beckett http://www.dajobe.org/
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
 * flickcurl_groups_discuss_replies_add:
 * @fc: flickcurl context
 * @topic_id: The ID of the topic to post a comment to.
 * @message: The message to post to the topic.
 * 
 * Post a new reply to a group discussion topic.
 *
 * Implements flickr.groups.discuss.replies.add (1.23)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_discuss_replies_add(flickcurl* fc,
                                     const char* topic_id, const char* message)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  
  if(!topic_id || !message)
    return 1;

  parameters[count][0]  = "topic_id";
  parameters[count++][1]= topic_id;
  parameters[count][0]  = "message";
  parameters[count++][1]= message;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.discuss.replies.add",
                       parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);
    
  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result = 0;

  tidy:
  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_groups_discuss_replies_delete:
 * @fc: flickcurl context
 * @topic_id: The ID of the topic the post is in.
 * @reply_id: The ID of the reply to delete.
 * 
 * Delete a reply from a group topic.
 *
 * Implements flickr.groups.discuss.replies.delete (1.23)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_discuss_replies_delete(flickcurl* fc,
                                        const char* topic_id,
                                        const char* reply_id)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  
  if(!topic_id || !reply_id)
    return 1;

  parameters[count][0]  = "topic_id";
  parameters[count++][1]= topic_id;
  parameters[count][0]  = "reply_id";
  parameters[count++][1]= reply_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.discuss.replies.delete",
                       parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);
    
  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result = 0;

  tidy:
  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_groups_discuss_replies_edit:
 * @fc: flickcurl context
 * @topic_id: The ID of the topic the post is in.
 * @reply_id: The ID of the reply post to edit.
 * @message: The message to edit the post with.
 * 
 * Edit a topic reply.
 *
 * Implements flickr.groups.discuss.replies.edit (1.23)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_discuss_replies_edit(flickcurl* fc,
                                      const char* topic_id,
                                      const char* reply_id,
                                      const char* message)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  
  if(!topic_id || !reply_id || !message)
    return 1;

  parameters[count][0]  = "topic_id";
  parameters[count++][1]= topic_id;
  parameters[count][0]  = "reply_id";
  parameters[count++][1]= reply_id;
  parameters[count][0]  = "message";
  parameters[count++][1]= message;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.discuss.replies.edit",
                       parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);
    
  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result = 0;

  tidy:
  if(fc->failed)
    result = 1;

  return result;
}


/**
 * flickcurl_groups_discuss_replies_getInfo:
 * @fc: flickcurl context
 * @topic_id: The ID of the topic the post is in.
 * @reply_id: The ID of the reply to fetch.
 * 
 * Get information on a group topic reply.
 *
 * Implements flickr.groups.discuss.replies.getInfo (1.23)
 * 
 * Return value: reply (topic) or NULL on failure
 **/
flickcurl_topic*
flickcurl_groups_discuss_replies_getInfo(flickcurl* fc,
                                         const char* topic_id,
                                         const char* reply_id)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_topic* reply = NULL;
  
  if(!topic_id || !reply_id)
    return NULL;

  parameters[count][0]  = "topic_id";
  parameters[count++][1]= topic_id;
  parameters[count][0]  = "reply_id";
  parameters[count++][1]= reply_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.discuss.replies.getInfo",
                       parameters, count))
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

  reply = flickcurl_build_topic(fc, xpathCtx, (const xmlChar*)"/rsp/reply");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    reply = NULL;

  return reply;
}


/**
 * flickcurl_groups_discuss_replies_getList:
 * @fc: flickcurl context
 * @topic_id: The ID of the topic to fetch replies for.
 * @per_page: Number of replies to return per page. The maximum allowed value is 500. (or < 0 for default of 100)
 * @page: The page of results to return (or < 0 for default of 1)
 * 
 * Get a list of replies from a group discussion topic.
 *
 * Implements flickr.groups.discuss.replies.getList (1.23)
 * 
 * Return value: topic list or NULL on failure
 **/
flickcurl_topic_list*
flickcurl_groups_discuss_replies_getList(flickcurl* fc,
                                         const char* topic_id,
                                         int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_topic_list* topic_list = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!topic_id || !per_page)
    return NULL;

  parameters[count][0]  = "topic_id";
  parameters[count++][1]= topic_id;
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

  if(flickcurl_prepare(fc, "flickr.groups.discuss.replies.getList",
                       parameters, count))
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

  topic_list = flickcurl_build_topic_list(fc, xpathCtx,
                                          (const xmlChar*)"/rsp/topics");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    topic_list = NULL;

  return topic_list;
}


