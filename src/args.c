/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * args.c - Flickcurl method arg functions
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
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


void
flickcurl_free_arg(flickcurl_arg *arg)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(arg, flickcurl_arg);

  if(arg->name)
    free(arg->name);
  if(arg->description)
    free(arg->description);
  free(arg);
}


flickcurl_arg**
flickcurl_build_args(flickcurl* fc, 
                     xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr,
                     int* arg_count_p)
{
  flickcurl_arg** args = NULL;
  int nodes_count;
  int arg_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do args */
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
  args = (flickcurl_arg**)calloc(nodes_count+1, sizeof(flickcurl_arg*));
  
  for(i = 0, arg_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_arg* arg;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    arg = (flickcurl_arg*)calloc(1, sizeof(flickcurl_arg));
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      if(!strcmp(attr_name, "name")) {
        size_t len = strlen((const char*)attr->children->content);
        arg->name = (char*)malloc(len + 1);
        memcpy(arg->name, attr->children->content, len + 1);
      } else if(!strcmp(attr_name, "optional"))
        arg->optional = atoi((const char*)attr->children->content);
    }

    /* Walk children nodes for description text */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        size_t len = strlen((const char*)chnode->content);
        arg->description = (char*)malloc(len + 1);
        memcpy(arg->description, (const char*)chnode->content, len + 1);
        break;
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "arg: name %s %s description '%s'\n",
            arg->name, (arg->optional? "" : "(required)"), arg->description);
#endif
    
    args[arg_count++] = arg;
  } /* for nodes */

  if(arg_count_p)
    *arg_count_p = arg_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return args;
}
