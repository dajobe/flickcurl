/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * blog.c - Flickcurl blog functions
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

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


static void
flickcurl_free_blog(flickcurl_blog *blog)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(blog, flickcurl_blog);

  if(blog->id)
    free(blog->id);
  
  if(blog->name)
    free(blog->name);
  
  free(blog);
}


/**
 * flickcurl_free_blogs:
 * @blogs_object: blog object array
 *
 * Destructor for array of blog objects
 */
void
flickcurl_free_blogs(flickcurl_blog **blogs_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(blogs_object, flickcurl_blog_array);

  for(i = 0; blogs_object[i]; i++)
    flickcurl_free_blog(blogs_object[i]);
  
  free(blogs_object);
}


flickcurl_blog**
flickcurl_build_blogs(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr, int* blog_count_p)
{
  flickcurl_blog** blogs = NULL;
  int nodes_count;
  int blog_count;
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
  blogs = (flickcurl_blog**)calloc(sizeof(flickcurl_blog*), nodes_count+1);
  
  for(i = 0, blog_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_blog* b;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    b = (flickcurl_blog*)calloc(sizeof(flickcurl_blog), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        b->id = attr_value;
      else if(!strcmp(attr_name, "name"))
        b->name = attr_value;
      else if(!strcmp(attr_name, "needspassword")) {
        b->needs_password = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "url"))
        b->url = attr_value;
    } /* end attributes */


#if FLICKCURL_DEBUG > 1
    fprintf(stderr,
            "blog: id %s  name '%s'  needs password '%d'  url '%s'\n",
            b->id, b->name, b->needs_password, b->url);
#endif
    
    blogs[blog_count++] = b;
  } /* for nodes */

  if(blog_count_p)
    *blog_count_p = blog_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return blogs;
}


static void
flickcurl_free_blog_service(flickcurl_blog_service *blog_service)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(blog_service, flickcurl_blog_service);

  if(blog_service->id)
    free(blog_service->id);
  
  if(blog_service->name)
    free(blog_service->name);
  
  free(blog_service);
}


/**
 * flickcurl_free_blog_services:
 * @blog_services_object: blog services object array
 *
 * Destructor for array of blog services objects
 */
void
flickcurl_free_blog_services(flickcurl_blog_service **blog_services_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(blog_services_object, flickcurl_blog_service_array);

  for(i = 0; blog_services_object[i]; i++)
    flickcurl_free_blog_service(blog_services_object[i]);
  
  free(blog_services_object);
}


flickcurl_blog_service**
flickcurl_build_blog_services(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                              const xmlChar* xpathExpr,
                              int* blog_services_count_p)
{
  flickcurl_blog_service** blog_services = NULL;
  int nodes_count;
  int blog_services_count;
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
  blog_services = (flickcurl_blog_service**)calloc(sizeof(flickcurl_blog_service*), nodes_count+1);
  
  for(i = 0, blog_services_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_blog_service* b;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    b = (flickcurl_blog_service*)calloc(sizeof(flickcurl_blog_service), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        b->id = attr_value;
    } /* end attributes */

    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        b->name = (char*)malloc(strlen((const char*)chnode->content)+1);
        strcpy(b->name, (const char*)chnode->content);
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "blog service: id %s  name '%s'\n", b->id, b->name);
#endif
    
    blog_services[blog_services_count++] = b;
  } /* for nodes */

  if(blog_services_count_p)
    *blog_services_count_p = blog_services_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return blog_services;
}
