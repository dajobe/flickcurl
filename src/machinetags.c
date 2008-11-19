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
 * flickcurl_free_namespace:
 * @namespace: namespace object
 *
 * Destructor for namespace object
 */
void
flickcurl_free_namespace(flickcurl_namespace *nspace)
{
  if(nspace->name)
    free(nspace->name);
  
  free(nspace);
}


flickcurl_namespace**
flickcurl_build_namespaces(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                           const xmlChar* xpathExpr, int* namespace_count_p)
{
  flickcurl_namespace** namespaces=NULL;
  int nodes_count;
  int namespace_count;
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
  namespaces=(flickcurl_namespace**)calloc(sizeof(flickcurl_namespace*), nodes_count+1);
  
  for(i=0, namespace_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_namespace* n;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    n = (flickcurl_namespace*)calloc(sizeof(flickcurl_namespace), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "usage")) {
        n->usage_count = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "predicates")) {
        n->predicates_count = atoi(attr_value);
        free(attr_value);
      }
    }

    /* Walk children for text */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        n->name = (char*)malloc(strlen((const char*)chnode->content)+1);
        strcpy(n->name, (const char*)chnode->content);
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "namespace: name %s usage %d predicates count %d\n",
            n->name, n->usage_count, n->predicates_count);
#endif
    
    namespaces[namespace_count++] = n;
  } /* for nodes */

  if(namespace_count_p)
    *namespace_count_p=namespace_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return namespaces;
}


flickcurl_namespace*
flickcurl_build_namespace(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                          const xmlChar* root_xpathExpr)
{
  flickcurl_namespace** namespaces;
  flickcurl_namespace* result=NULL;

  namespaces = flickcurl_build_namespaces(fc, xpathCtx, root_xpathExpr, NULL);
  if(namespaces) {
    result = namespaces[0];
    free(namespaces);
  }
  
  return result;
}


/**
 * flickcurl_free_namespaces:
 * @namespaces: namespace object array
 *
 * Destructor for array of namespace object
 */
void
flickcurl_free_namespaces(flickcurl_namespace** namespaces)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(namespaces, flickcurl_namespace_array);

  for(i=0; namespaces[i]; i++)
    flickcurl_free_namespace(namespaces[i]);
  free(namespaces);
}
