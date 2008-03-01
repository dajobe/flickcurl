/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * reflection-api.c - Flickr flickr.reflection.* API calls
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
 * flickcurl_reflection_getMethods:
 * @fc: flickcurl context
 * 
 * Get the list of available API method names
 *
 * Implements flickr.reflection.getMethods (0.10)
 * 
 * Return value: arry of names or NULL on failure
 **/
char**
flickcurl_reflection_getMethods(flickcurl* fc)
{
  const char * parameters[5][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  const xmlChar* xpathExpr=NULL;
  int i;
  int size;
  char **methods=NULL;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.reflection.getMethods", parameters, count))
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

  xpathExpr=(const xmlChar*)"/rsp/methods/method";
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed=1;
    goto tidy;
  }

  nodes=xpathObj->nodesetval;
  size=xmlXPathNodeSetGetLength(nodes);
  methods=(char**)calloc(1+size, sizeof(char*));

  count=0;
  for(i=0; i < size; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }

    /* Walk children nodes for description text */
    for(chnode=node->children; chnode; chnode=chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        methods[count]=(char*)malloc(strlen((const char*)chnode->content)+1);
        strcpy(methods[count], (const char*)chnode->content);
        count++;
        break;
      }
    }
    
  } /* for nodes */
  methods[count]=NULL;
  
  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return methods;
}


/**
 * flickcurl_reflection_getMethodInfo:
 * @fc: flickcurl context
 * @name: method name
 * 
 * Get information about an API method
 *
 * Implements flickr.reflection.getMethodInfo (0.10)
 * 
 * Return value: #flickcurl_method or NULL on failure
 **/
flickcurl_method*
flickcurl_reflection_getMethodInfo(flickcurl* fc, const char* name)
{
  const char * parameters[6][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  flickcurl_method* method=NULL;
  
  parameters[count][0]  = "method_name";
  parameters[count++][1]= name;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.reflection.getMethodInfo", parameters, count))
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

  method=flickcurl_build_method(fc, xpathCtx);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    method=NULL;

  return method;
}
