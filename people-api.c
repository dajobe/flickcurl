/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * people-api.c - Flickr flickr.people.* API calls
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


static char*
flickcurl_get_nsid(flickcurl* fc, const char* key, const char* value,
                   const char* method)
{
  const char * parameters[5][2];
  int count=0;
  char *nsid=NULL;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 

  if(!value)
    return NULL;
  
  parameters[count][0]  = key;
  parameters[count++][1]= value;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, method, parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    nsid=flickcurl_xpath_eval(fc, xpathCtx,
                              (const xmlChar*)"/rsp/user/@nsid");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return nsid;
}


/**
 * flickcurl_people_findByEmail:
 * @fc: flickcurl context
 * @email: user email address
 * 
 * Get a user's NSID, given their email address
 *
 * Implements flickr.people.findByEmail (0.8)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_people_findByEmail(flickcurl* fc, const char* email)
{
  return flickcurl_get_nsid(fc, "find_email", email, 
                            "flickr.people.findByEmail");
}


/**
 * flickcurl_people_findByUsername:
 * @fc: flickcurl context
 * @username: username
 * 
 * Get a user's NSID, given their username address
 *
 * Implements flickr.people.findByUsername (0.8)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_people_findByUsername(flickcurl* fc, const char* username)
{
  return flickcurl_get_nsid(fc, "username", username, 
                            "flickr.people.findByUsername");
}


/**
 * flickcurl_people_getInfo - 
 * @fc: flickcurl context
 * @user_id: user NSID
 * 
 * Get information about a person
 *
 * Implements flickr.people.getInfo (0.6)
 *
 * NSID can be found by flickcurl_people_findByEmail() or
 * flickcurl_people_findByUsername().
 * 
 * Return value: #flickcurl_person object or NULL on failure
 **/
flickcurl_person*
flickcurl_people_getInfo(flickcurl* fc, const char* user_id)
{
  const char * parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_person* person=NULL;
  
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.people.getInfo", parameters, count))
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

  person=flickcurl_build_person(fc, xpathCtx, (const xmlChar*)"/rsp/person");

 tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);
  if(fc->failed)
    person=NULL;

  return person;
}


/*
 * flickr.people.getPublicGroups:
 *
 * Get the list of public groups a user is a member of.
 */


/*
 * flickr.people.getPublicPhotos:
 *
 * Get a list of public photos for the given user.
 */


/*
 * flickr.people.getUploadStatus:
 *
 * Get information for the calling user related to photo uploads.
 */
