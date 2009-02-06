/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * contacts-api.c - Flickr flickr.contacts.* API calls
 *
 * Copyright (C) 2007-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2007 Vanilla I. Shu <vanilla -at- fatpipi.cirx.org>
 *   (flickcurl_contacts_getList, flickcurl_contacts_getPublicList)
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
 * flickcurl_contacts_getList:
 * @fc: flickcurl context
 * @filter: An optional filter of the results. The following values are valid:  friends, family, both or neither (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 1000. The maximum allowed value is 1000. (or NULL)
 * 
 * Get a list of contacts for the calling user.
 *
 * Implements flickr.contacts.getList (0.11)
 * 
 * Return value: NULL on failure
 **/
flickcurl_contact**
flickcurl_contacts_getList(flickcurl* fc, const char* filter,
                           int page, int per_page)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_contact** contacts=NULL;
  int contacts_count=0;
  char page_str[10];
  char per_page_str[10];

  if(filter) {
    parameters[count][0]  = "filter";
    parameters[count++][1]= filter;
  }
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

  if(flickcurl_prepare(fc, "flickr.contacts.getList", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  contacts=flickcurl_build_contacts(fc, xpathCtx, 
                                    (xmlChar*)"/rsp/contacts/contact", 
                                    &contacts_count);


  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    contacts=NULL;

  return contacts;
}


/**
 * flickcurl_contacts_getListRecentlyUploaded:
 * @fc: flickcurl context
 * @date_lastupload: Limits the results to contacts that have uploaded photos since this date (in the form of a Unix timestamp).  The default, and maximum, offset is 1 hour.  (or < 0)
 * @filter: Limit the result set to all contacts or only those who are friends or family. Valid options are: ff: friends and family, all: all your contacts. Default value is "all". (or NULL)
 * 
 * Return a list of contacts for a user who have recently uploaded
 * photos along with the total count of photos uploaded.
 *
 * This API added 2009-01-14 as announced in
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4668
 *
 * Implements flickr.contacts.getListRecentlyUploaded (1.8)
 * 
 * Return value: NULL on failure
 **/
flickcurl_contact**
flickcurl_contacts_getListRecentlyUploaded(flickcurl* fc,
                                           int date_lastupload,
                                           const char* filter)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_contact** contacts=NULL;
  int contacts_count=0;
  char date_lastupload_str[20];
  
  if(date_lastupload >= 0) {
    parameters[count][0]  = "date_lastupload";
    sprintf(date_lastupload_str, "%d", date_lastupload);
    parameters[count++][1]= date_lastupload_str;
  }
  if(filter) {
    parameters[count][0]  = "filter";
    parameters[count++][1]= filter;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.contacts.getListRecentlyUploaded",
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

  contacts=flickcurl_build_contacts(fc, xpathCtx, 
                                    (xmlChar*)"/rsp/contacts/contact", 
                                    &contacts_count);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    contacts = NULL;

  return contacts;
}


/**
 * flickcurl_contacts_getPublicList:
 * @fc: flickcurl context
 * @user_id: The NSID of the user to fetch the contact list for.
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or NULL)
 * @per_page: Number of photos to return per page. If this argument is omitted, it defaults to 1000. The maximum allowed value is 1000. (or NULL)
 * 
 * Get the contact list for a user.
 *
 * Implements flickr.contacts.getPublicList (0.11)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_contact**
flickcurl_contacts_getPublicList(flickcurl* fc, const char* user_id,
                                 int page, int per_page)
{
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_contact** contacts=NULL;
  int contacts_count=0;
  char page_str[10];
  char per_page_str[10];
 
  if (!user_id)
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

  if(flickcurl_prepare(fc, "flickr.contacts.getPublicList", parameters,
                       count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  contacts=flickcurl_build_contacts(fc, xpathCtx, 
                                    (xmlChar*)"/rsp/contacts/contact", 
                                    &contacts_count);


  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    contacts=NULL;

  return contacts;
}
