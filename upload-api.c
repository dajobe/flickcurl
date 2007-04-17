/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * upload-api.c - Flickr photo upload API calls
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
 * flickcurl_photos_upload:
 * @fc: flickcurl context
 * @photo_file: photo filename
 * @title: title or NULL
 * @description: description of photo (HTML) (or NULL)
 * @tags: space-separated list of tags (or NULL)
 * @is_public: is public photo boolean (non-0 true)
 * @is_friend: is friend photo boolean (non-0 true)
 * @is_family: is family photo boolean (non-0 true)
 * 
 * Checks the status of one or more asynchronous photo upload tickets.
 *
 * Implements Uploading Photos (0.10)
 * 
 * Return value: #flickcurl_upload_status or NULL on failure
 **/
flickcurl_upload_status*
flickcurl_photos_upload(flickcurl* fc, const char* photo_file,
                        const char *title,
                        const char *description,
                        const char *tags,
                        int is_public, int is_friend, int is_family)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_upload_status* status=NULL;
  char is_public_s[2];
  char is_friend_s[2];
  char is_family_s[2];
  
  if(!photo_file)
    return NULL;

  if(access((const char*)photo_file, R_OK)) {
    flickcurl_error(fc, "Photo file %s cannot be read: %s",
                    photo_file, strerror(errno));
    return NULL;
  }

  is_public_s[0]=is_public ? '1' : '0';
  is_public_s[1]='\0';
  is_friend_s[0]=is_friend ? '1' : '0';
  is_friend_s[1]='\0';
  is_family_s[0]=is_family ? '1' : '0';
  is_family_s[1]='\0';

  if(title) {
    parameters[count][0]  = "title";
    parameters[count++][1]= title;
  }
  if(description) {
    parameters[count][0]  = "description";
    parameters[count++][1]= description;
  }
  if(tags) {
    parameters[count][0]  = "tags";
    parameters[count++][1]= tags;
  }
  parameters[count][0]  = "is_public";
  parameters[count++][1]= is_public_s;
  parameters[count][0]  = "is_friend";
  parameters[count++][1]= is_friend_s;
  parameters[count][0]  = "is_family";
  parameters[count++][1]= is_family_s;

  parameters[count][0]  = NULL;


  if(flickcurl_prepare_upload(fc,
                            "http://api.flickr.com/services/upload/",
                            "photo", photo_file,
                            parameters, count))
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

  status=(flickcurl_upload_status*)calloc(1, sizeof(flickcurl_upload_status));
  status->photoid=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/photoid");
  /* when async is true */
  status->ticketid=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/ticketid");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    status=NULL;

  return status;
}



/**
 * flickcurl_photos_replace:
 * @fc: flickcurl context
 * @photo_file: photo filename
 * @photo_id: photo ID to replace
 * @async: upload asynchronously boolean (non-0 true)
 * 
 * Replace a photo with a new file.
 *
 * Implements Replaciing Photos (0.10)
 * 
 * Return value: #flickcurl_upload_status or NULL on failure
 **/
flickcurl_upload_status*
flickcurl_photos_replace(flickcurl* fc, const char* photo_file,
                         const char *photo_id, int async)
{
  const char* parameters[7][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_upload_status* status=NULL;
  char async_s[2];
  
  if(!photo_file || !photo_id)
    return NULL;

  if(access((const char*)photo_file, R_OK)) {
    flickcurl_error(fc, "Photo file %s cannot be read: %s",
                    photo_file, strerror(errno));
    return NULL;
  }

  async_s[0]=async ? '1' : '0';
  async_s[1]='\0';
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "async";
  parameters[count++][1]= async_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare_upload(fc,
                              "http://api.flickr.com/services/replace/",
                              "photo", photo_file,
                              parameters, count))
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

  status=(flickcurl_upload_status*)calloc(1, sizeof(flickcurl_upload_status));
  status->secret=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/photoid/@secret");
  status->originalsecret=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/photoid/@originalsecret");
  /* when async is true */
  status->ticketid=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/ticketid");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    status=NULL;

  return status;
}


void
flickcurl_upload_status_free(flickcurl_upload_status* status)
{
  if(status->photoid)
    free(status->photoid);
  if(status->secret)
    free(status->secret);
  if(status->originalsecret)
    free(status->originalsecret);
  if(status->ticketid)
    free(status->ticketid);
}
