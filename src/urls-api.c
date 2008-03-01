/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * urls-api.c - Flickr flickr.urls.* API calls
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
 * flickcurl_urls_getGroup:
 * @fc: flickcurl context
 * @group_id: group ID
 *
 * Get the url to a group's page.
 *
 * Implements flickr.urls.getGroup (0.9)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_urls_getGroup(flickcurl* fc, const char* group_id)
{
  return flickcurl_call_get_one_string_field(fc, "group_id", group_id, 
                                             "flickr.urls.getGroup",
                                             (const xmlChar*)"/rsp/group/@url");
}


/**
 * flickcurl_urls_getUserPhotos:
 * @fc: flickcurl context
 * @user_id: user ID
 *
 * Get the url to a user's photos.
 *
 * Implements flickr.urls.getUserPhotos (0.9)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_urls_getUserPhotos(flickcurl* fc, const char* user_id)
{
  return flickcurl_call_get_one_string_field(fc, "user_id", user_id, 
                                             "flickr.urls.getUserPhotos",
                                             (const xmlChar*)"/rsp/user/@url");
}


/**
 * flickcurl_urls_getUserProfile:
 * @fc: flickcurl context
 * @user_id: user ID
 *
 * Get the url to a user's profile.
 *
 * Implements flickr.urls.getUserProfile (0.9)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_urls_getUserProfile(flickcurl* fc, const char* user_id)
{
  return flickcurl_call_get_one_string_field(fc, "user_id", user_id, 
                                             "flickr.urls.getUserProfile",
                                             (const xmlChar*)"/rsp/user/@url");
}


/**
 * flickcurl_urls_lookupGroup:
 * @fc: flickcurl context
 * @url: URL of group's page or photo pool
 *
 * Get a group NSID, given the url to a group's page or photo pool.
 *
 * Implements flickr.urls.lookupGroup (0.9)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_urls_lookupGroup(flickcurl* fc, const char* url)
{
  return flickcurl_call_get_one_string_field(fc, "url", url, 
                                             "flickr.urls.lookupGroup",
                                             (const xmlChar*)"/rsp/group/@id");
}


/**
 * flickcurl_urls_lookupUser:
 * @fc: flickcurl context
 * @url: URL of user's photo or user's profile
 * 
 * Get a user NSID, given the url to a user's photos or profile.
 *
 * Implements flickr.urls.lookupUser (0.6)
 * 
 * Return value: NSID or NULL on failure
 **/
char*
flickcurl_urls_lookupUser(flickcurl* fc, const char* url)
{
  return flickcurl_call_get_one_string_field(fc, "url", url, 
                                             "flickr.urls.lookupUser",
                                             (const xmlChar*)"/rsp/user/@id");
}
