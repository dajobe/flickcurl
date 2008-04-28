/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * user_upload_status.c - Flickcurl user_upload_status functions
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
 * flickcurl_free_user_upload_status:
 * @u: user upload status object
 *
 * Destructor for user upload status object
 */
void
flickcurl_free_user_upload_status(flickcurl_user_upload_status *u)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(u, flickcurl_user_upload_status);

  if(u->username)
    free(u->username);
  
  if(u->sets_remaining)
    free(u->sets_remaining);
  
  free(u);
}


flickcurl_user_upload_status*
flickcurl_build_user_upload_status(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                                   const xmlChar* xpathExpr)
{
  flickcurl_user_upload_status* u=NULL;
  int nodes_count;
  int i;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do user_upload_status */
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
  
  u=(flickcurl_user_upload_status*)calloc(sizeof(flickcurl_user_upload_status), 1);


  for(i=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    const char *node_name=(const char*)node->name;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    if(!strcmp(node_name, "username")) {
      xmlNodePtr chnode;
      for(chnode=node->children; chnode; chnode=chnode->next) {
        if(chnode->type != XML_TEXT_NODE)
          continue;
        u->username=(char*)malloc(strlen((const char*)chnode->content)+1);
        strcpy(u->username, (const char*)chnode->content);
        break;
      }
    } else if(!strcmp(node_name, "bandwidth")) {
      for(attr=node->properties; attr; attr=attr->next) {
        const char *attr_name=(const char*)attr->name;
        int attr_value=atoi((const char*)attr->children->content);
        if(!strcmp(attr_name, "maxbytes"))
          u->bandwidth_maxbytes=attr_value;
        else if(!strcmp(attr_name, "maxkb"))
          u->bandwidth_maxkb=attr_value;
        else if(!strcmp(attr_name, "usedbytes"))
          u->bandwidth_usedbytes=attr_value;
        else if(!strcmp(attr_name, "usedkb"))
          u->bandwidth_usedkb=attr_value;
        else if(!strcmp(attr_name, "remainingbytes"))
          u->bandwidth_remainingbytes=attr_value;
        else if(!strcmp(attr_name, "remainingkb"))
          u->bandwidth_remainingkb=attr_value;
      }
    } else if(!strcmp(node_name, "filesize")) {
      for(attr=node->properties; attr; attr=attr->next) {
        const char *attr_name=(const char*)attr->name;
        int attr_value=atoi((const char*)attr->children->content);
        if(!strcmp(attr_name, "maxbytes"))
          u->filesize_maxbytes=attr_value;
        else if(!strcmp(attr_name, "maxkb"))
          u->filesize_maxkb=attr_value;
      }
    } else if(!strcmp(node_name, "sets")) {
      for(attr=node->properties; attr; attr=attr->next) {
        const char *attr_name=(const char*)attr->name;
        char *attr_value;
        
        attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
        strcpy(attr_value, (const char*)attr->children->content);
        
        if(!strcmp(attr_name, "created")) {
          u->sets_created=atoi(attr_value);
          free(attr_value);
        } else if(!strcmp(attr_name, "remaining"))
          u->sets_remaining=attr_value;
      }
    }

  } /* for nodes */

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "user_upload_status: user %s  bandwidth max %d/%d K  used %d/%d K  remaining %d/%d K  max filesize %d/%d K  sets created %d remaining %s\n",
            u->username,
            u->bandwidth_maxbytes,
            u->bandwidth_maxkb,
            u->bandwidth_usedbytes,
            u->bandwidth_usedkb,
            u->bandwidth_remainingbytes,
            u->bandwidth_remainingkb,
            u->filesize_maxbytes,
            u->filesize_maxkb,
            u->sets_created,
            (u->sets_remaining ? u->sets_remaining : ""));
#endif

 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return u;
}
