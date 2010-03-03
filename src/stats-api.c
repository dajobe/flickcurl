/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * stats-api.c - Flickr flickr.stats.* API calls
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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
 * flickcurl_stats_getCollectionDomains:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @collection_id: The id of the collection to get stats for. If not provided, stats for all collections will be returned. (or NULL)
 * @per_page: Number of domains to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referring domains for a collection
 *
 * Implements flickr.stats.getCollectionDomains (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getCollectionDomains(flickcurl* fc, const char* date,
                                     const char* collection_id,
                                     int per_page, int page)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  if(collection_id) {
    parameters[count][0]  = "collection_id";
    parameters[count++][1]= collection_id;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getCollectionDomains", parameters,
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getCollectionReferrers:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @domain: The domain to return referrers for. This should be a hostname (eg: "flickr.com") with no protocol or pathname.
 * @collection_id: The id of the collection to get stats for. If not provided, stats for all collections will be returned. (or NULL)
 * @per_page: Number of referrers to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referrers from a given domain to a collection
 *
 * Implements flickr.stats.getCollectionReferrers (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getCollectionReferrers(flickcurl* fc, const char* date,
                                       const char* domain,
                                       const char* collection_id,
                                       int per_page, int page)
{
  const char* parameters[12][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date || !domain)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  parameters[count][0]  = "domain";
  parameters[count++][1]= domain;
  if(collection_id) {
    parameters[count][0]  = "collection_id";
    parameters[count++][1]= collection_id;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getCollectionReferrers", parameters,
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getCollectionStats:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
either be in YYYY-MM-DD or unix timestamp format.  A day according to
Flickr Stats starts at midnight GMT for all users, and timestamps
will automatically be rounded down to the start of the day.
 * @collection_id: The id of the collection to get stats for.
 * 
 * Get the number of views on a collection for a given date.
 *
 * Implements flickr.stats.getCollectionStats (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: views count or <0 on failure
 **/
int
flickcurl_stats_getCollectionStats(flickcurl* fc, const char* date,
                                   const char* collection_id)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int result;
  
  if(!date || !collection_id)
    return -1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  parameters[count][0]  = "collection_id";
  parameters[count++][1]= collection_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getCollectionStats", parameters, count))
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

  result = -1; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = -1;

  return result;
}


/**
 * flickcurl_stats_getPhotoDomains:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @photo_id: The id of the photo to get stats for. If not provided, stats for all photos will be returned. (or NULL)
 * @per_page: Number of domains to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referring domains for a photo
 *
 * Implements flickr.stats.getPhotoDomains (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPhotoDomains(flickcurl* fc, const char* date,
                                const char* photo_id, int per_page, int page)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  if(photo_id) {
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotoDomains", parameters, count))
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPhotoReferrers:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @domain: The domain to return referrers for. This should be a hostname (eg: "flickr.com") with no protocol or pathname.
 * @photo_id: The id of the photo to get stats for. If not provided, stats for all photos will be returned. (or NULL)
 * @per_page: Number of referrers to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referrers from a given domain to a photo
 *
 * Implements flickr.stats.getPhotoReferrers (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPhotoReferrers(flickcurl* fc, const char* date,
                                  const char* domain, const char* photo_id,
                                  int per_page, int page)
{
  const char* parameters[12][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date || !domain)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  parameters[count][0]  = "domain";
  parameters[count++][1]= domain;
  if(photo_id) {
    parameters[count][0]  = "photo_id";
    parameters[count++][1]= photo_id;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotoReferrers", parameters, count))
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPhotosetDomains:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @photoset_id: The id of the photoset to get stats for. If not provided, stats for all sets will be returned. (or NULL)
 * @per_page: Number of domains to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referring domains for a photoset
 *
 * Implements flickr.stats.getPhotosetDomains (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPhotosetDomains(flickcurl* fc, const char* date,
                                   const char* photoset_id,
                                   int per_page, int page)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  if(photoset_id) {
    parameters[count][0]  = "photoset_id";
    parameters[count++][1]= photoset_id;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotosetDomains", parameters, count))
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPhotosetReferrers:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @domain: The domain to return referrers for. This should be a hostname (eg: "flickr.com") with no protocol or pathname.
 * @photoset_id: The id of the photoset to get stats for. If not provided, stats for all sets will be returned. (or NULL)
 * @per_page: Number of referrers to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referrers from a given domain to a photoset
 *
 * Implements flickr.stats.getPhotosetReferrers (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPhotosetReferrers(flickcurl* fc, const char* date,
                                     const char* domain,
                                     const char* photoset_id,
                                     int per_page, int page)
{
  const char* parameters[12][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date || !domain)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  parameters[count][0]  = "domain";
  parameters[count++][1]= domain;
  if(photoset_id) {
    parameters[count][0]  = "photoset_id";
    parameters[count++][1]= photoset_id;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotosetReferrers", parameters,
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPhotosetStats:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @photoset_id: The id of the photoset to get stats for.
 * 
 * Get the number of views on a photoset for a given date.
 *
 * Implements flickr.stats.getPhotosetStats (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: number of views or <0 on failure
 **/
int
flickcurl_stats_getPhotosetStats(flickcurl* fc, const char* date,
                                 const char* photoset_id)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int result;
  
  if(!date || !photoset_id)
    return -1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotosetStats", parameters, count))
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

  result = -1; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = -1;

  return result;
}


/**
 * flickcurl_stats_getPhotoStats:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @photo_id: The id of the photo to get stats for.
 * 
 * Get the number of views, comments and favorites on a photo for a given date.
 *
 * Implements flickr.stats.getPhotoStats (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPhotoStats(flickcurl* fc, const char* date,
                              const char* photo_id)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  
  if(!date || !photo_id)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotoStats", parameters, count))
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPhotostreamDomains:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @per_page: Number of domains to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100 (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referring domains for a photostream
 *
 * Implements flickr.stats.getPhotostreamDomains (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPhotostreamDomains(flickcurl* fc, const char* date,
                                      int per_page, int page)
{
  const char* parameters[10][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotostreamDomains", parameters,
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPhotostreamReferrers:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * @domain: The domain to return referrers for. This should be a hostname (eg: "flickr.com") with no protocol or pathname.
 * @per_page: Number of referrers to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * 
 * Get a list of referrers from a given domain to a user's photostream
 *
 * Implements flickr.stats.getPhotostreamReferrers (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPhotostreamReferrers(flickcurl* fc, const char* date,
                                        const char* domain,
                                        int per_page, int page)
{
  const char* parameters[11][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(!date || !domain)
    return 1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;
  parameters[count][0]  = "domain";
  parameters[count++][1]= domain;
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotostreamReferrers", parameters,
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPhotostreamStats:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.
 * 
 * Get the number of views on a user's photostream for a given date.
 *
 * Implements flickr.stats.getPhotostreamStats (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: number of views or <0 on failure
 **/
int
flickcurl_stats_getPhotostreamStats(flickcurl* fc, const char* date)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  
  if(!date)
    return -1;

  parameters[count][0]  = "date";
  parameters[count++][1]= date;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPhotostreamStats", parameters, count))
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getPopularPhotos:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in
 * either be in YYYY-MM-DD or unix timestamp format.  A day according
 * to Flickr Stats starts at midnight GMT for all users, and
 * timestamps will automatically be rounded down to the start of the
 * day.  If no date is provided, all time view counts will be
 * returned. (or NULL)
 * @sort: The order in which to sort returned photos. Defaults to views. The possible values are views, comments and favorites.  Other sort options are available through search. (or NULL)
 * @per_page: Number of referrers to return per page. If this argument is omitted, it defaults to 25. The maximum allowed value is 100. (or < 0)
 * @page: The page of results to return. If this argument is omitted, it defaults to 1. (or < 0)
 * @extras: A comma-delimited list of extra information to fetch for each returned record. Currently supported fields are: <code>description</code>, <code>license</code>, <code>date_upload</code>, <code>date_taken</code>, <code>owner_name</code>, <code>icon_server</code>, <code>original_format</code>, <code>last_update</code>, <code>geo</code>, <code>tags</code>, <code>machine_tags</code>, <code>o_dims</code>, <code>views</code>, <code>media</code>, <code>path_alias</code>, <code>url_sq</code>, <code>url_t</code>, <code>url_s</code>, <code>url_m</code>, <code>url_o</code> (or NULL)
 * 
 * Get stats for popular photos
 *
 * Implements flickr.stats.getPopularPhotos (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_stats_getPopularPhotos(flickcurl* fc, const char* date,
                                 const char* sort, int per_page, int page,
                                 const char* extras)
{
  const char* parameters[14][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  char per_page_str[10];
  char page_str[10];
  
  if(date) {
    parameters[count][0]  = "date";
    parameters[count++][1]= date;
  }
  if(sort) {
    parameters[count][0]  = "sort";
    parameters[count++][1]= sort;
  }
  if(per_page >= 0) {
    sprintf(per_page_str, "%d", per_page);
    parameters[count][0]  = "per_page";
    parameters[count++][1]= per_page_str;
  }
  if(page >= 0) {
    sprintf(page_str, "%d", page);
    parameters[count][0]  = "page";
    parameters[count++][1]= page_str;
  }
  if(extras) {
    parameters[count][0]  = "extras";
    parameters[count++][1]= extras;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getPopularPhotos", parameters, count))
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

  result = NULL; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_stats_getTotalViews:
 * @fc: flickcurl context
 * @date: Stats will be returned for this date. This should be in either be in YYYY-MM-DD or unix timestamp format.

A day according to Flickr Stats starts at midnight GMT for all users, and timestamps will automatically be rounded down to the start of the day.

If no date is provided, all time view counts will be returned. (or NULL)
 * 
 * Get the overall view counts for an account
 *
 * Implements flickr.stats.getTotalViews (1.17)
 * 
 * Announced 2010-03-03
 * http://code.flickr.com/blog/2010/03/03/flickr-stats-api/
 *
 * Return value: view count or <0 on failure
 **/
int
flickcurl_stats_getTotalViews(flickcurl* fc, const char* date)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int result;
  
  if(date) {
    parameters[count][0]  = "date";
    parameters[count++][1]= date;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.stats.getTotalViews", parameters, count))
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

  result = -1; /* your code here */

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = -1;

  return result;
}


