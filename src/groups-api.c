/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * groups-api.c - Flickr flickr.groups.* API calls
 *
 * Copyright (C) 2007-2012, David Beckett http://www.dajobe.org/
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
 * flickcurl_groups_browse:
 * @fc: flickcurl context
 * @cat_id: The category id to fetch a list of groups and sub-categories for. If not specified, it defaults to zero, the root of the category tree. (or NULL)
 * 
 * Browse the group category tree, finding groups and sub-categories.
 *
 * Implements flickr.groups.browse (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_category*
flickcurl_groups_browse(flickcurl* fc, int cat_id)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_category* category = NULL;
  char cat_id_str[10];
  
  flickcurl_init_params(fc, 0);

  if(cat_id >= 0) {
    sprintf(cat_id_str, "%d", cat_id);
    flickcurl_add_param(fc, "cat_id", cat_id_str);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.browse"))
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

  category = (flickcurl_category*)calloc(sizeof(flickcurl_category), 1);
  category->categories = flickcurl_build_categories(fc, xpathCtx,
     (const xmlChar*)"/rsp/category/subcat", &category->categories_count);
  category->groups = flickcurl_build_groups(fc, xpathCtx,
     (const xmlChar*)"/rsp/category/group", &category->groups_count);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(category)
      flickcurl_free_category(category);
    category = NULL;
  }

  return category;
}


/**
 * flickcurl_groups_getInfo:
 * @fc: flickcurl context
 * @group_id: The NSID of the group to fetch information for.
 * @lang: The language of the group name and description to fetch.  If the language is not found, the primary language of the group will be returned (or NULL)
 *
 * Get information about a group.
 *
 * Implements flickr.groups.getInfo (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_group*
flickcurl_groups_getInfo(flickcurl* fc, const char* group_id, const char* lang)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_group **groups = NULL;
  flickcurl_group *group = NULL;
  
  flickcurl_init_params(fc, 0);

  if(!group_id)
    return NULL;

  flickcurl_add_param(fc, "group_id", group_id);
  if(lang) {
    flickcurl_add_param(fc, "lang", lang);
  }

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.getInfo"))
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

  groups = flickcurl_build_groups(fc, xpathCtx,
                                (const xmlChar*)"/rsp/group", NULL);
  if(groups) {
    group = groups[0];
    free(groups);
    groups = NULL;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(group)
      flickcurl_free_group(group);
    group = NULL;
  }

  return group;
}


/**
 * flickcurl_groups_join:
 * @fc: flickcurl context
 * @group_id: The NSID of the Group in question
 * @accept_rules: If the group has rules, they must be displayed to the user prior to joining. Passing a true value for this argument specifies that the application has displayed the group rules to the user, and that the user has agreed to them. (See flickr.groups.getInfo). Probably takes a '1' or '0' value (or NULL)
 * 
 * Join a public group as a member.
 *
 * Implements flickr.groups.join (1.25)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_join(flickcurl* fc, const char* group_id,
                      const char* accept_rules)
{
  xmlDocPtr doc = NULL;
  int result = 1;

  flickcurl_init_params(fc, 0);

  if(!group_id)
    return 1;

  flickcurl_add_param(fc, "group_id", group_id);
  if(accept_rules)
    flickcurl_add_param(fc, "accept_rules", accept_rules);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.join"))
    goto tidy;

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
 * flickcurl_groups_joinRequest:
 * @fc: flickcurl context
 * @group_id: The NSID of the group to request joining.
 * @message: Message to the administrators.
 * @accept_rules: If the group has rules, they must be displayed to the user prior to joining. Passing a true value for this argument specifies that the application has displayed the group rules to the user, and that the user has agreed to them. (See flickr.groups.getInfo).  Probably takes a '1' or '0' value.
 * 
 * Request to join a group that is invitation-only.
 *
 * Implements flickr.groups.joinRequest (1.25)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_joinRequest(flickcurl* fc, const char* group_id,
                             const char* message, const char* accept_rules)
{
  xmlDocPtr doc = NULL;
  int result = 1;

  flickcurl_init_params(fc, 0);

  if(!group_id || !message || !accept_rules)
    return 1;

  flickcurl_add_param(fc, "group_id", group_id);
  flickcurl_add_param(fc, "message", message);
  flickcurl_add_param(fc, "accept_rules", accept_rules);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.joinRequest"))
    goto tidy;

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
 * flickcurl_groups_leave:
 * @fc: flickcurl context
 * @group_id: The NSID of the Group to leave
 * @delete_photos: Delete all photos by this user from the group.  Probably takes a '1' or '0' value (or NULL)
 * 
 * Leave a group.
 *
 * If the user is the only administrator left, and there are other
 * members, the oldest member will be promoted to administrator.
 * 
 * If the user is the last person in the group, the group will be deleted.
 *
 * Implements flickr.groups.leave (1.25)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_groups_leave(flickcurl* fc, const char* group_id,
                       const char* delete_photos)
{
  xmlDocPtr doc = NULL;
  int result = 1;

  flickcurl_init_params(fc, 0);

  if(!group_id)
    return 1;

  flickcurl_add_param(fc, "group_id", group_id);
  if(delete_photos)
    flickcurl_add_param(fc, "delete_photos", delete_photos);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.leave"))
    goto tidy;

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
 * flickcurl_groups_search:
 * @fc: flickcurl context
 * @text: The text to search for.
 * @per_page: Number of groups to return per page, default 100, max 500 (or NULL)
 * @page: The page of results to return, default 1 (or NULL)
 * 
 * Search for groups. 18+ groups will only be returned for
 * authenticated calls where the authenticated user is over 18.
 *
 * Implements flickr.groups.search (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_group**
flickcurl_groups_search(flickcurl* fc, const char* text, int per_page, int page)
{
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_group **groups = NULL;
  char per_page_s[10];
  char page_s[10];
  
  flickcurl_init_params(fc, 0);

  if(!text)
    return NULL;

  flickcurl_add_param(fc, "text", text);
  sprintf(per_page_s, "%d", per_page);
  flickcurl_add_param(fc, "per_page", per_page_s);
  sprintf(page_s, "%d", page);
  flickcurl_add_param(fc, "page", page_s);

  flickcurl_end_params(fc);

  if(flickcurl_prepare(fc, "flickr.groups.search"))
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

  groups = flickcurl_build_groups(fc, xpathCtx,
                                (const xmlChar*)"/rsp/groups/group", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(groups)
      flickcurl_free_groups(groups);
    groups = NULL;
  }

  return groups;
}
