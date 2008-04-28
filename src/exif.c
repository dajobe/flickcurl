/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * exif.c - Flickcurl EXIF functions
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
 * flickcurl_free_exif:
 * @exif: exif object
 *
 * Destructor for exif object
 */
void
flickcurl_free_exif(flickcurl_exif *exif)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(exif, flickcurl_exif);

  if(exif->tagspace)
    free(exif->tagspace);
  
  if(exif->label)
    free(exif->label);
  if(exif->raw)
    free(exif->raw);
  if(exif->clean)
    free(exif->clean);
  
  free(exif);
}


/**
 * flickcurl_free_exifs:
 * @exifs_object: array of exif objects
 *
 * Destructor for array of exif objects
 */
void
flickcurl_free_exifs(flickcurl_exif **exifs_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(exifs_object, flickcurl_exif_array);

  for(i=0; exifs_object[i]; i++)
    flickcurl_free_exif(exifs_object[i]);
  
  free(exifs_object);
}


flickcurl_exif**
flickcurl_build_exifs(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr, int* exif_count_p)
{
  flickcurl_exif** exifs=NULL;
  int nodes_count;
  int exif_count;
  int i;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do exifs */
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
  exifs=(flickcurl_exif**)calloc(sizeof(flickcurl_exif*), nodes_count+1);
  
  for(i=0, exif_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_exif* e;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    e=(flickcurl_exif*)calloc(sizeof(flickcurl_exif), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "tagspace"))
        e->tagspace=attr_value;
      else if(!strcmp(attr_name, "tagspaceid")) {
        e->tagspaceid=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "tag")) {
        e->tag=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "label"))
        e->label=attr_value;
    }

    /* Walk children nodes for <raw> or <clean> elements */
    for(chnode=node->children; chnode; chnode=chnode->next) {
      const char *chnode_name=(const char*)chnode->name;
      if(chnode->type == XML_ELEMENT_NODE) {
        if(!strcmp(chnode_name, "raw")) {
          e->raw=(char*)malloc(strlen((const char*)chnode->children->content)+1);
          strcpy(e->raw, (const char*)chnode->children->content);
        } else if(!strcmp(chnode_name, "clean")) {
          e->clean=(char*)malloc(strlen((const char*)chnode->children->content)+1);
          strcpy(e->clean, (const char*)chnode->children->content);
        }
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "exif: tagspace %s tagspace ID %d tag %d label '%s' raw '%s' clean '%s'\n",
            e->tagspace, e->tagspaceid, e->tag, e->label, e->raw, e->clean);
#endif
    
    exifs[exif_count++]=e;
  } /* for nodes */

  if(exif_count_p)
    *exif_count_p=exif_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return exifs;
}
