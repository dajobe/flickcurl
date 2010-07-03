/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * stat.c - Flickcurl statistic functions
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

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_free_stat:
 * @stat: stat object
 *
 * Destructor for stat object
 */
void
flickcurl_free_stat(flickcurl_stat *stat)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(stat, flickcurl_stat);

  if(stat->name)
    free(stat->name);
  if(stat->url)
    free(stat->url);
  if(stat->searchterms)
    free(stat->searchterms);
  
  free(stat);
}


/**
 * flickcurl_free_stats:
 * @stats_object: stat object array
 *
 * Destructor for array of stat objects
 */
void
flickcurl_free_stats(flickcurl_stat **stats_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(stats_object, flickcurl_stat_array);

  for(i = 0; stats_object[i]; i++)
    flickcurl_free_stat(stats_object[i]);
  
  free(stats_object);
}


/**
 * flickcurl_free_view_stats:
 * @view_stats: view stat object
 *
 * Destructor for view stat object
 */
void
flickcurl_free_view_stats(flickcurl_view_stats *view_stats)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(view_stats, flickcurl_view_stats);

  free(view_stats);
}


flickcurl_stat**
flickcurl_build_stats(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr, int* stat_count_p)
{
  flickcurl_stat** stats = NULL;
  int nodes_count;
  int stat_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed = 1;
    goto tidy;
  }
  
  nodes = xpathObj->nodesetval;
  /* This is a max size - it can include nodes that are CDATA */
  nodes_count = xmlXPathNodeSetGetLength(nodes);
  stats = (flickcurl_stat**)calloc(sizeof(flickcurl_stat*), nodes_count+1);
  
  for(i = 0, stat_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_stat* s;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    s = (flickcurl_stat*)calloc(sizeof(flickcurl_stat), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "views")) {
        s->views = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "comments")) {
        s->comments = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "favorites")) {
        s->favorites = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "name"))
        s->name = attr_value;
      else if(!strcmp(attr_name, "url"))
        s->url = attr_value;
      else if(!strcmp(attr_name, "searchterms"))
        s->searchterms = attr_value;
    } /* end attributes */


#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "statistic: views %d comments %d favorites %d name %s url %s searchterms %s\n",
            s->views, s->comments,
            s->favorites, s->name,
            s->url ? s->url : "", 
            s->searchterms ? s->searchterms : "");
#endif
    
    stats[stat_count++] = s;
  } /* for nodes */

  if(stat_count_p)
    *stat_count_p = stat_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return stats;
}
