/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * api.c - Flickr API calls
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
 * flickcurl_tags_getHotList:
 *
 * flickr.tags.getHotList
 */

/**
 * flickcurl_tags_getListPhoto:
 * @fc: flickcurl context
 * @id: photo ID
 *
 * Get the tag list for a given photo.
 *
 * Implements flickr.tags.getListPhoto (0.9)
 * 
 * Return value: #flickcurl_photo or NULL on failure
 **/
flickcurl_tag**
flickcurl_tags_getListInfo(flickcurl* fc, const char* photo_id)
{
  const char* parameters[5][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_tag** tags=NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  /* does not require authentication */
  if(fc->auth_token) {
    parameters[count][0]  = "token";
    parameters[count++][1]= fc->auth_token;
  }

  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

  if(flickcurl_prepare(fc, "flickr.tags.getListPhoto", parameters, count))
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

  tags=flickcurl_build_tags(fc, NULL,
                            xpathCtx, 
                            (xmlChar*)"/rsp/photo/tags/tag", 
                            NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tags=NULL;

  return tags;
}


/**
 * flickcurl_tags_getListUser:
 *
 * flickr.tags.getListUser
 */

/**
 * flickcurl_tags_getListUserPopular:
 *
 * flickr.tags.getListUserPopular
 */

/**
 * flickcurl_tags_getListUserRaw:
 *
 * flickr.tags.getListUserRaw
 */

/**
 * flickcurl_tags_getRelated:
 *
 * flickr.tags.getRelated
 */


