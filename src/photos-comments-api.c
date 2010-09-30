/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-comments-api.c - Flickr flickr.photos.comments.* API calls
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
 * flickcurl_photos_comments_addComment:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to add a comment to.
 * @comment_text: Text of the comment
 * 
 * Add comment to a photo as the currently authenticated user.
 *
 * Implements flickr.photos.comments.addComment (0.10)
 * 
 * Return value: new comment ID or non-NULL on failure
 **/
char*
flickcurl_photos_comments_addComment(flickcurl* fc, const char* photo_id,
                                     const char* comment_text)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char* id = NULL;
  
  if(!photo_id || !comment_text)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "comment_text";
  parameters[count++][1]= comment_text;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.comments.addComment", parameters,
                       count))
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

  id = flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/comment/@id");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    id = NULL;

  return id;
}


/**
 * flickcurl_photos_comments_deleteComment:
 * @fc: flickcurl context
 * @comment_id: The id of the comment to edit.
 * 
 * Delete a comment as the currently authenticated user.
 *
 * Implements flickr.photos.comments.deleteComment (0.10)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_comments_deleteComment(flickcurl* fc, const char* comment_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  
  if(!comment_id)
    return 1;

  parameters[count][0]  = "comment_id";
  parameters[count++][1]= comment_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.comments.deleteComment", parameters,
                       count))
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
 * flickcurl_photos_comments_editComment:
 * @fc: flickcurl context
 * @comment_id: The id of the comment to edit.
 * @comment_text: Update the comment to this text.
 * 
 * Edit the text of a comment as the currently authenticated user.
 *
 * Implements flickr.photos.comments.editComment (0.10)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_comments_editComment(flickcurl* fc, const char* comment_id,
                                      const char* comment_text)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int result = 1;
  
  if(!comment_id || !comment_text)
    return 1;

  parameters[count][0]  = "comment_id";
  parameters[count++][1]= comment_id;
  parameters[count][0]  = "comment_text";
  parameters[count++][1]= comment_text;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.comments.editComment", parameters,
                       count))
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
 * flickcurl_photos_comments_getList:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to fetch comments for.
 * 
 * Returns the comments for a photo
 *
 * Implements flickr.photos.comments.getList (0.10)
 * 
 * Return value: array of comments or NULL on failure
 **/
flickcurl_comment**
flickcurl_photos_comments_getList(flickcurl* fc, const char* photo_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_comment** comments = NULL;
  int comments_count = 0;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.comments.getList", parameters, count))
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

  comments = flickcurl_build_comments(fc, xpathCtx, 
                                    (xmlChar*)"/rsp/comments/comment", 
                                    &comments_count);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    comments = NULL;

  return comments;
}


/**
 * flickcurl_photos_comments_getRecentForContacts_params:
 * @fc: flickcurl context
 * @date_lastcomment: Limits the resultset to photos that have been commented on since this date. The date should be in the form of a Unix timestamp. The default, and maximum, offset is (1) hour (or <0)
 * @contacts_filter: A comma-separated list of contact NSIDs to limit the scope of the query to (or NULL)
 * @list_params: #flickcurl_photos_list_params result parameters (or NULL)
 * 
 * Return the list of photos belonging to your contacts that have
 * been commented on recently.
 *
 * Implements flickr.photos.comments.getRecentForContacts (1.12)
 * 
 * Return value: list of photos or NULL on failure
 **/
flickcurl_photos_list*
flickcurl_photos_comments_getRecentForContacts_params(flickcurl* fc,
                                                      int date_lastcomment,
                                                      const char* contacts_filter,
                                                      flickcurl_photos_list_params* list_params)
{
  const char* parameters[12][2];
  int count = 0;
  flickcurl_photos_list* photos_list = NULL;
  const char* format = NULL;
  char date_lastcomment_str[20];
  
  /* API parameters */
  if(date_lastcomment >= 0) {
    parameters[count][0]  = "date_lastcomment";
    sprintf(date_lastcomment_str, "%d", date_lastcomment);
    parameters[count++][1]= date_lastcomment_str;
  }
  if(contacts_filter) {
    parameters[count][0]  = "contacts_filter";
    parameters[count++][1]= contacts_filter;
  }

  /* Photos List parameters */
  flickcurl_append_photos_list_params(list_params, parameters, &count, &format);
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.comments.getRecentForContacts",
                       parameters, count))
    goto tidy;

  photos_list = flickcurl_invoke_photos_list(fc,
                                             (const xmlChar*)"/rsp/photos",
                                             format);

  tidy:

  return photos_list;
}
