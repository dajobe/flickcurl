/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * namespace.c - Flickr machinetag support calls
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
 * flickcurl_free_tag_namespace:
 * @tag_nspace: machinetag namespace object
 *
 * Destructor for machinetag namespace object
 */
void
flickcurl_free_tag_namespace(flickcurl_tag_namespace *tag_nspace)
{
  if(tag_nspace->name)
    free(tag_nspace->name);
  
  free(tag_nspace);
}


flickcurl_tag_namespace**
flickcurl_build_tag_namespaces(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                               const xmlChar* xpathExpr, int* namespace_count_p)
{
  flickcurl_tag_namespace** tag_namespaces = NULL;
  int nodes_count;
  int tag_namespace_count;
  int i;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do namespaces */
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
  tag_namespaces = (flickcurl_tag_namespace**)calloc(sizeof(flickcurl_tag_namespace*), nodes_count + 1);
  
  for(i=0, tag_namespace_count = 0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_tag_namespace* tn;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    tn = (flickcurl_tag_namespace*)calloc(sizeof(flickcurl_tag_namespace), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "usage")) {
        tn->usage_count = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "predicates")) {
        tn->predicates_count = atoi(attr_value);
        free(attr_value);
      }
    }

    /* Walk children for text */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        tn->name = (char*)malloc(strlen((const char*)chnode->content)+1);
        strcpy(tn->name, (const char*)chnode->content);
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "namespace: name %s usage %d predicates count %d\n",
            tn->name, tn->usage_count, tn->predicates_count);
#endif
    
    tag_namespaces[tag_namespace_count++] = tn;
  } /* for nodes */

  if(namespace_count_p)
    *namespace_count_p = tag_namespace_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return tag_namespaces;
}


flickcurl_tag_namespace*
flickcurl_build_tag_namespace(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                              const xmlChar* root_xpathExpr)
{
  flickcurl_tag_namespace** tag_namespaces;
  flickcurl_tag_namespace* result = NULL;

  tag_namespaces = flickcurl_build_tag_namespaces(fc, xpathCtx, root_xpathExpr,
                                                  NULL);
  if(tag_namespaces) {
    int i;
    
    result = tag_namespaces[0];

    for(i = 1; tag_namespaces[i]; i++)
      flickcurl_free_tag_namespace(tag_namespaces[i]);
    free(tag_namespaces);
  }
  
  return result;
}


/**
 * flickcurl_free_tag_namespaces:
 * @tag_nspaces: namespace object array
 *
 * Destructor for array of namespace object
 */
void
flickcurl_free_tag_namespaces(flickcurl_tag_namespace** tag_nspaces)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(tag_nspaces,
                                         flickcurl_tag_namespace_array);

  for(i = 0; tag_nspaces[i]; i++)
    flickcurl_free_tag_namespace(tag_nspaces[i]);
  free(tag_nspaces);
}


/**
 * flickcurl_free_tag_predicate_value:
 * @tag_pv: machinetag predicate_value object
 *
 * Destructor for machinetag predicate-value pair  object
 */
void
flickcurl_free_tag_predicate_value(flickcurl_tag_predicate_value *tag_pv)
{
  if(tag_pv->predicate)
    free(tag_pv->predicate);
  
  if(tag_pv->value)
    free(tag_pv->value);
  
  free(tag_pv);
}


/*
 * @content_mode: set use of element content: 1 (predicate), 2 (value) otherwise ignored
 */
flickcurl_tag_predicate_value**
flickcurl_build_tag_predicate_values(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                                     const xmlChar* xpathExpr,
                                     int content_mode,
                                     int* predicate_value_count_p)
{
  flickcurl_tag_predicate_value** tag_pvs = NULL;
  int nodes_count;
  int tag_predicate_value_count;
  int i;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do predicate_values */
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
  tag_pvs = (flickcurl_tag_predicate_value**)calloc(sizeof(flickcurl_tag_predicate_value*), nodes_count + 1);
  
  for(i=0, tag_predicate_value_count = 0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_tag_predicate_value* tpv;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    tpv = (flickcurl_tag_predicate_value*)calloc(sizeof(flickcurl_tag_predicate_value), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "usage")) {
        tpv->usage_count = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "namespaces")) {
        tpv->used_in_namespace_count = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "predicate")) {
        tpv->predicate=attr_value;
      } else if(!strcmp(attr_name, "value")) {
        tpv->value=attr_value;
      }
    }

    if(content_mode >=1 && content_mode <= 2) {
      /* Walk children for predicate */
      for(chnode = node->children; chnode; chnode = chnode->next) {
        if(chnode->type == XML_TEXT_NODE) {
          char **ptr = (content_mode == 1) ? &tpv->predicate : &tpv->value;
          *ptr = (char*)malloc(strlen((const char*)chnode->content)+1);
          strcpy(*ptr, (const char*)chnode->content);
        }
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "predicate_value: predicate %s value %s\n",
            tpv->predicate, tpv->value);
#endif
    
    tag_pvs[tag_predicate_value_count++] = tpv;
  } /* for nodes */

  if(predicate_value_count_p)
    *predicate_value_count_p = tag_predicate_value_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return tag_pvs;
}


/**
 * flickcurl_free_tag_predicate_values:
 * @tag_pvs: predicate_value object array
 *
 * Destructor for array of predicate_value object
 */
void
flickcurl_free_tag_predicate_values(flickcurl_tag_predicate_value** tag_pvs)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(tag_pvs,
                                         flickcurl_tag_predicate_value_array);

  for(i = 0; tag_pvs[i]; i++)
    flickcurl_free_tag_predicate_value(tag_pvs[i]);
  free(tag_pvs);
}
