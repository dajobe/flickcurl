/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photoset.c - Flickcurl photoset functions
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
 * flickcurl_free_photoset:
 * @photoset: photoset object
 *
 * Destructor for photoset object
 */
void
flickcurl_free_photoset(flickcurl_photoset *photoset)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(photoset, flickcurl_photoset);

  if(photoset->id)
    free(photoset->id);
  
  if(photoset->primary)
    free(photoset->primary);
  
  if(photoset->secret)
    free(photoset->secret);
  
  if(photoset->title)
    free(photoset->title);
  
  if(photoset->description)
    free(photoset->description);
  
  free(photoset);
}


/**
 * flickcurl_free_photosets:
 * @photosets_object: photoset object array
 *
 * Destructor for array of photoset object
 */
void
flickcurl_free_photosets(flickcurl_photoset **photosets_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(photosets_object, flickcurl_photoset_array);

  for(i=0; photosets_object[i]; i++)
    flickcurl_free_photoset(photosets_object[i]);
  
  free(photosets_object);
}


flickcurl_photoset**
flickcurl_build_photosets(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                          const xmlChar* xpathExpr, int* photoset_count_p)
{
  flickcurl_photoset** photosets=NULL;
  int nodes_count;
  int photoset_count;
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
  photosets=(flickcurl_photoset**)calloc(sizeof(flickcurl_photoset*), nodes_count+1);
  
  for(i=0, photoset_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_photoset* ps;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    ps=(flickcurl_photoset*)calloc(sizeof(flickcurl_photoset), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        ps->id=attr_value;
      else if(!strcmp(attr_name, "primary"))
        ps->primary=attr_value;
      else if(!strcmp(attr_name, "secret")) {
        ps->secret=attr_value;
      } else if(!strcmp(attr_name, "server")) {
        ps->server=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "farm")) {
        ps->farm=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "photos")) {
        ps->photos_count=atoi(attr_value);
        free(attr_value);
      }
    }

    /* Walk children nodes for <title> or <description> elements */
    for(chnode=node->children; chnode; chnode=chnode->next) {
      const char *chnode_name=(const char*)chnode->name;
      if(chnode->type == XML_ELEMENT_NODE) {
        if(!strcmp(chnode_name, "title")) {
          if(chnode->children) {
            ps->title=(char*)malloc(strlen((const char*)chnode->children->content)+1);
            strcpy(ps->title, (const char*)chnode->children->content);
          }
        } else if(!strcmp(chnode_name, "description")) {
          if(chnode->children) {
            ps->description=(char*)malloc(strlen((const char*)chnode->children->content)+1);
            strcpy(ps->description, (const char*)chnode->children->content);
          }
        }
      }
    }
    

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "photoset: id %s  primary '%s'  secret %s  server %d  farm %d photos count %d  title '%s'  description '%s'\n",
            ps->id, ps->primary, ps->secret, ps->server, ps->farm,
            ps->photos_count,
            ps->title, 
            (ps->description ? ps->description : "(No description)"));
#endif
    
    photosets[photoset_count++]=ps;
  } /* for nodes */

  if(photoset_count_p)
    *photoset_count_p=photoset_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return photosets;
}


flickcurl_photoset*
flickcurl_build_photoset(flickcurl* fc, xmlXPathContextPtr xpathCtx)
{
  flickcurl_photoset** photosets;
  flickcurl_photoset* result=NULL;

  photosets=flickcurl_build_photosets(fc, xpathCtx,
                                      (const xmlChar*)"/rsp/photoset", NULL);
  if(photosets) {
    result=photosets[0];
    free(photosets);
  }
  
  return result;
}


