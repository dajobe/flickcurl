/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickr.photos.upload-api.c - Flickr flickr.photos.upload.* API calls
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
 * flickcurl_photos_upload_checkTickets:
 * @fc: flickcurl context
 * @tickets_ids: Array of ticket ids
 * 
 * Checks the status of one or more asynchronous photo upload tickets.
 *
 * Implements flickr.photos.upload.checkTickets (0.13)
 * 
 * Return value: non-0 on failure
 **/
flickcurl_ticket**
flickcurl_photos_upload_checkTickets(flickcurl* fc,
                                     const char** tickets_ids)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_ticket** tickets=NULL;
  char* tickets_ids_string=NULL;
  
  if(!tickets_ids)
    return NULL;

  tickets_ids_string=flickcurl_array_join(tickets_ids, ',');
  parameters[count][0]  = "tickets";
  parameters[count++][1]= tickets_ids_string;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.upload.checkTickets", parameters, count))
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

  tickets=flickcurl_build_tickets(fc, xpathCtx,
                                  (const xmlChar*)"/rsp/uploader/ticket",
                                  NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tickets=NULL;
  if(tickets_ids_string)
    free(tickets_ids_string);

  return tickets;
}


