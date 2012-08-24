/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * groups-discuss-topics-api.c - Flickr flickr.groups.discuss.topics.* API calls
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
 * flickcurl_groups_discuss_topics_add:
 * @fc: flickcurl context
 * @group_id: The NSID of the group to add a topic to.

 * @subject: The topic subject.
 * @message: The topic message.
 * 
 * Post a new discussion topic to a group.
 *
 * Implements flickr.groups.discuss.topics.add (1.23)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_discuss_topics_add(flickcurl* fc, const char* group_id,
                                    const char* subject, const char* message)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  
  if(!group_id || !subject || !message)
    return 1;

  parameters[count][0]  = "group_id";
  parameters[count++][1]= group_id;
  parameters[count][0]  = "subject";
  parameters[count++][1]= subject;
  parameters[count][0]  = "message";
  parameters[count++][1]= message;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.discuss.topics.add",
                       parameters, count))
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
 * flickcurl_groups_discuss_topics_getInfo:
 * @fc: flickcurl context
 * @topic_id: The ID for the topic to edit.
 * 
 * Get information about a group discussion topic.
 *
 * Implements flickr.groups.discuss.topics.getInfo (1.23)
 * 
 * Return value: topic or NULL on failure
 **/
flickcurl_topic*
flickcurl_groups_discuss_topics_getInfo(flickcurl* fc, const char* topic_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_topic* topic = NULL;
  
  if(!topic_id)
    return NULL;

  parameters[count][0]  = "topic_id";
  parameters[count++][1]= topic_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.discuss.topics.getInfo",
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

  topic = flickcurl_build_topic(fc, xpathCtx, (const xmlChar*)"/rsp/topic");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    topic = NULL;

  return topic;
}


/**
 * flickcurl_groups_discuss_topics_getList:
 * @fc: flickcurl context
 * @group_id: The NSID of the group to fetch information for.
 * @per_page: Number of photos to return per page. The maximum allowed value is 500. (or < 0 for default of 100)
 * @page: The page of results to return. (or < 0 for default of 1)
 * 
 * Get a list of discussion topics in a group.
 *
 * Implements flickr.groups.discuss.topics.getList (1.23)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_topic_list*
flickcurl_groups_discuss_topics_getList(flickcurl* fc,
                                        const char* group_id,
                                        int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_topic_list* topic_list = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!group_id)
    return NULL;

  parameters[count][0]  = "group_id";
  parameters[count++][1]= group_id;
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

  if(flickcurl_prepare(fc, "flickr.groups.discuss.topics.getList",
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


