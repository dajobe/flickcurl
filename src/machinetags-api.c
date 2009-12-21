/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * machinetags-api.c - Flickr flickr.machinetags.* API calls
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
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
 * flickcurl_machinetags_getNamespaces:
 * @fc: flickcurl context
 * @predicate: Limit the list of namespaces returned to those that have the following predicate (or NULL)
 * @per_page: Number of namespaces to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500 (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1 (or NULL)
 * 
 * Return a list of unique namespaces, optionally limited by a given predicate, in alphabetical order.
 *
 * Implements flickr.machinetags.getNamespaces (1.7)
 *
 * As announced 2008-11-18
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4545
 * and
 * http://code.flickr.com/blog/2008/12/15/machine-tag-hierarchies/
 *
 * Return value: array of namespaces or NULL on failure
 **/
flickcurl_tag_namespace**
flickcurl_machinetags_getNamespaces(flickcurl* fc, const char* predicate,
                                    int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char per_page_s[4];
  char page_s[4];
  flickcurl_tag_namespace** tag_namespaces = NULL;
  
  parameters[count][0]  = "predicate";
  parameters[count++][1]= predicate;
  parameters[count][0]  = "per_page";
  sprintf(per_page_s, "%d", per_page);
  parameters[count++][1]= per_page_s;
  parameters[count][0]  = "page";
  sprintf(page_s, "%d", page);
  parameters[count++][1]= page_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.machinetags.getNamespaces", parameters,
                       count))
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

  tag_namespaces = flickcurl_build_tag_namespaces(fc, xpathCtx, 
                                                  (const xmlChar*)"/rsp/namespaces/namespace", 
                                                  NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tag_namespaces = NULL;

  return tag_namespaces;
}


/**
 * flickcurl_machinetags_getPairs:
 * @fc: flickcurl context
 * @nspace: Limit the list of pairs returned to those that have the following namespace (or NULL)
 * @predicate: Limit the list of pairs returned to those that have the following predicate (or NULL)
 * @per_page: Number of pairs to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500 (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1 (or NULL)
 * 
 * Return a list of unique namespace and predicate pairs, optionally limited by predicate or namespace, in alphabetical order.
 *
 * Implements flickr.machinetags.getPairs (1.7)
 * 
 * As announced 2008-11-18
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4545
 * and
 * http://code.flickr.com/blog/2008/12/15/machine-tag-hierarchies/
 *
 * Return value: array of pairs or NULL on failure
 **/
flickcurl_tag_predicate_value**
flickcurl_machinetags_getPairs(flickcurl* fc, const char *nspace,
                               const char* predicate,
                               int per_page, int page)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char per_page_s[4];
  char page_s[4];
  flickcurl_tag_predicate_value** tag_pvs = NULL;
  
  parameters[count][0]  = "namespace";
  parameters[count++][1]= nspace;
  parameters[count][0]  = "predicate";
  parameters[count++][1]= predicate;
  parameters[count][0]  = "per_page";
  sprintf(per_page_s, "%d", per_page);
  parameters[count++][1]= per_page_s;
  parameters[count][0]  = "page";
  sprintf(page_s, "%d", page);
  parameters[count++][1]= page_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.machinetags.getPairs", parameters, count))
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

  tag_pvs = flickcurl_build_tag_predicate_values(fc, xpathCtx, 
                                                 (const xmlChar*)"/rsp/pairs/pair", 
                                                 0 /* content not used */,
                                                 NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tag_pvs = NULL;

  return tag_pvs;
}


/**
 * flickcurl_machinetags_getPredicates:
 * @fc: flickcurl context
 * @nspace: Limit the list of predicates returned to those that have the following namespace (or NULL)
 * @per_page: Number of predicates to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500 (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1 (or NULL)
 * 
 * Return a list of unique predicates, optionally limited by a given namespace.
 *
 * Implements flickr.machinetags.getPredicates (1.7)
 * 
 * As announced 2008-11-18
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4545
 * and
 * http://code.flickr.com/blog/2008/12/15/machine-tag-hierarchies/
 *
 * Return value: array of predicates or NULL on failure
 **/
flickcurl_tag_predicate_value**
flickcurl_machinetags_getPredicates(flickcurl* fc, const char *nspace,
                                    int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char per_page_s[4];
  char page_s[4];
  flickcurl_tag_predicate_value** tag_pvs = NULL;
  
  parameters[count][0]  = "namespace";
  parameters[count++][1]= nspace;
  parameters[count][0]  = "per_page";
  sprintf(per_page_s, "%d", per_page);
  parameters[count++][1]= per_page_s;
  parameters[count][0]  = "page";
  sprintf(page_s, "%d", page);
  parameters[count++][1]= page_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.machinetags.getPredicates", parameters,
                       count))
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

  tag_pvs = flickcurl_build_tag_predicate_values(fc, xpathCtx, 
                                                 (const xmlChar*)"/rsp/predicates/predicate", 
                                                 1 /* content is predicate */,
                                                 NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tag_pvs = NULL;

  return tag_pvs;
}


/**
 * flickcurl_machinetags_getValues:
 * @fc: flickcurl context
 * @nspace: The namespace that all values should be restricted to.
 * @predicate: The predicate that all values should be restricted to.
 * @per_page: Number of values to return per page. If this argument is omitted, it defaults to 100. The maximum allowed value is 500 (or NULL)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1 (or NULL)
 * 
 * Return a list of unique values for a namespace and predicate.
 *
 * Implements flickr.machinetags.getValues (1.7)
 * 
 * As announced 2008-11-18
 * http://tech.groups.yahoo.com/group/yws-flickr/message/4545
 * and
 * http://code.flickr.com/blog/2008/12/15/machine-tag-hierarchies/
 *
 * Return value: array of values or NULL on failure
 **/
flickcurl_tag_predicate_value**
flickcurl_machinetags_getValues(flickcurl* fc, const char *nspace,
                                const char* predicate,
                                int per_page, int page)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  char per_page_s[4];
  char page_s[4];
  flickcurl_tag_predicate_value** tag_pvs = NULL;
  
  if(!nspace || !predicate)
    return NULL;

  parameters[count][0]  = "namespace";
  parameters[count++][1]= nspace;
  parameters[count][0]  = "predicate";
  parameters[count++][1]= predicate;
  parameters[count][0]  = "per_page";
  sprintf(per_page_s, "%d", per_page);
  parameters[count++][1]= per_page_s;
  parameters[count][0]  = "page";
  sprintf(page_s, "%d", page);
  parameters[count++][1]= page_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.machinetags.getValues", parameters, count))
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

  tag_pvs = flickcurl_build_tag_predicate_values(fc, xpathCtx, 
                                                 (const xmlChar*)"/rsp/values/value", 
                                                 2 /* content is value */,
                                                 NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tag_pvs = NULL;

  return tag_pvs;
}


/**
 * flickcurl_machinetags_getRecentValues:
 * @fc: flickcurl context
 * @nspace: A namespace that all values should be restricted to (or NULL)
 * @predicate: A predicate that all values should be restricted to (or NULL)
 * @added_since: Only return machine tags values that have been added since this timestamp, in epoch seconds (or <0)
 * 
 * Fetch recently used machine tags values.
 *
 * Implements flickr.machinetags.getRecentValues (1.12)
 * 
 * Return value: array of values or NULL on failure
 **/
flickcurl_tag_predicate_value**
flickcurl_machinetags_getRecentValues(flickcurl* fc,
                                      const char *nspace,
                                      const char* predicate,
                                      int added_since)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_tag_predicate_value** tag_pvs = NULL;
  char added_since_s[20];

  if(nspace) {
    parameters[count][0]  = "namespace";
    parameters[count++][1]= nspace;
  }

  if(predicate) {
    parameters[count][0]  = "predicate";
    parameters[count++][1]= predicate;
  }

  if(added_since >= 0) {
    sprintf(added_since_s, "%d", added_since);
    parameters[count][0]  = "added_since";
    parameters[count++][1]= added_since_s;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.machinetags.getRecentValues",
                       parameters, count))
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

  tag_pvs = flickcurl_build_tag_predicate_values(fc, xpathCtx, 
                                                 (const xmlChar*)"/rsp/values/value", 
                                                 2 /* content is value */,
                                                 NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    tag_pvs = NULL;

  return tag_pvs;
}


