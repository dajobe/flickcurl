/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * category.c - Flickcurl category functions
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

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_free_category:
 * @category: category object
 *
 * Destructor for category object
 */
void
flickcurl_free_category(flickcurl_category *category)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(category, flickcurl_category);

  if(category->id)
    free(category->id);
  
  if(category->name)
    free(category->name);

  if(category->categories)
    flickcurl_free_categories(category->categories);
  
  if(category->groups)
    flickcurl_free_groups(category->groups);
  
  free(category);
}


/**
 * flickcurl_free_categories:
 * @categories_object: category object array
 *
 * Destructor for array of category object
 */
void
flickcurl_free_categories(flickcurl_category **categories_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(categories_object, flickcurl_category);

  for(i=0; categories_object[i]; i++)
    flickcurl_free_category(categories_object[i]);
  
  free(categories_object);
}


flickcurl_category**
flickcurl_build_categories(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                           const xmlChar* xpathExpr, int* category_count_p)
{
  flickcurl_category** categories=NULL;
  int nodes_count;
  int category_count;
  int i;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed=1;
    goto tidy;
  }
  
  nodes=xpathObj->nodesetval;
  /* This is a max size - it can include nodes that are CDATA */
  nodes_count=xmlXPathNodeSetGetLength(nodes);
  categories=(flickcurl_category**)calloc(sizeof(flickcurl_category*), nodes_count+1);
  
  for(i=0, category_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_category* c;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    c=(flickcurl_category*)calloc(sizeof(flickcurl_category), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        c->id=attr_value;
      else if(!strcmp(attr_name, "name"))
        c->name=attr_value;
      else if(!strcmp(attr_name, "path"))
        c->path=attr_value;
      else if(!strcmp(attr_name, "count")) {
        c->count=atoi(attr_value);
        free(attr_value);
      }
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr,
            "category: id %s  name '%s'  path '%s'  count %d\n",
            c->id, c->name, c->path, c->count);
#endif
    
    categories[category_count++]=c;
  } /* for nodes */

  if(category_count_p)
    *category_count_p=category_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return categories;
}
