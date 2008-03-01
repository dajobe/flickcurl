/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * groups-api.c - Flickr flickr.groups.* API calls
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
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_category* category=NULL;
  char cat_id_str[10];
  
  if(cat_id >= 0) {
    sprintf(cat_id_str, "%d", cat_id);
    parameters[count][0]  = "cat_id";
    parameters[count++][1]= cat_id_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.browse", parameters, count))
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

  category=(flickcurl_category*)calloc(sizeof(flickcurl_category), 1);
  category->categories=flickcurl_build_categories(fc, xpathCtx,
     (const xmlChar*)"/rsp/category/subcat", &category->categories_count);
  category->groups=flickcurl_build_groups(fc, xpathCtx,
     (const xmlChar*)"/rsp/category/group", &category->groups_count);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    category=NULL;

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
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_group **groups=NULL;
  flickcurl_group *group=NULL;
  
  if(!group_id)
    return NULL;

  parameters[count][0]  = "group_id";
  parameters[count++][1]= group_id;
  if(lang) {
    parameters[count][0]  = "lang";
    parameters[count++][1]= lang;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.getInfo", parameters, count))
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

  groups=flickcurl_build_groups(fc, xpathCtx,
                                (const xmlChar*)"/rsp/group", NULL);
  if(groups) {
    group=groups[0];
    free(groups);
    groups=NULL;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    group=NULL;

  return group;
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
  const char* parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_group **groups=NULL;
  char per_page_s[10];
  char page_s[10];
  
  if(!text)
    return NULL;

  parameters[count][0]  = "text";
  parameters[count++][1]= text;
  parameters[count][0]  = "per_page";
  sprintf(per_page_s, "%d", per_page);
  parameters[count++][1]= per_page_s;
  parameters[count][0]  = "page";
  sprintf(page_s, "%d", page);
  parameters[count++][1]= page_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.search", parameters, count))
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

  groups=flickcurl_build_groups(fc, xpathCtx,
                                (const xmlChar*)"/rsp/groups/group", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    groups=NULL;

  return groups;
}


