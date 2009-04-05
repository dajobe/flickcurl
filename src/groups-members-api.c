/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * groups-members-api.c - Flickr flickr.groups.members.* API calls
 *
 * Copyright (C) 2009, David Beckett http://www.dajobe.org/
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
 * flickcurl_groups_members_getList:
 * @fc: flickcurl context
 * @group_id: Return a list of members for this group.  The group must be viewable by the Flickr member on whose behalf the API call is made.
 * @membertypes: Comma separated list of member types:  2: member, 3: moderator 4: admin.  By default returns all types.  (Returning super rare member type "1: narwhal" isn't supported by this API method) (or NULL)
 * @per_page: Number of members to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500 (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1 (or < 0)
 * 
 * Get a list of the members of a group.
 *
 * The call must be signed on behalf of a Flickr member, and the
 * ability to see the group membership will be determined by the
 * Flickr member's group privileges.
 *
 * Implements flickr.groups.members.getList (1.9)
 * as announced as an experimental API on 2009-02-24:
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4749
 * 
 * Return value: list of members or NULL on failure
 **/
flickcurl_member**
flickcurl_groups_members_getList(flickcurl* fc, const char* group_id,
                                 const char* membertypes,
                                 int per_page, int page)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_member** members = NULL;
  int members_count = 0;
  char per_page_s[10];
  char page_s[10];
  
  if(!group_id)
    return NULL;

  parameters[count][0]  = "group_id";
  parameters[count++][1]= group_id;
  if(membertypes) {
    parameters[count][0]  = "membertypes";
    parameters[count++][1]= membertypes;
  }
  if(per_page >= 0) {
    sprintf(per_page_s, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_s;
  }
  if(page >= 0) {
    sprintf(page_s, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_s;
  }
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.groups.members.getList", parameters, count))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  members = flickcurl_build_members(fc, xpathCtx, 
                                    (xmlChar*)"/rsp/members/member", 
                                    &members_count);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    members = NULL;

  return members;
}
