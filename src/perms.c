/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * perms.c - Flickcurl method perms functions
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
 * flickcurl_free_perms:
 * @perms: perms object
 *
 * Destructor for perms object
 */
void
flickcurl_free_perms(flickcurl_perms *perms)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(perms, flickcurl_perms);

  free(perms);
}


flickcurl_perms*
flickcurl_build_perms(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr)
{
  flickcurl_perms* perms=NULL;
  int nodes_count;
  int i;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do perms */
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
  
  for(i=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    perms=(flickcurl_perms*)calloc(sizeof(flickcurl_perms), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        ; /* perms->id=attr_value; */
      else if(!strcmp(attr_name, "ispublic"))
        perms->is_public=atoi(attr_value);
      else if(!strcmp(attr_name, "iscontact"))
        perms->is_contact=atoi(attr_value);
      else if(!strcmp(attr_name, "isfriend"))
        perms->is_friend=atoi(attr_value);
      else if(!strcmp(attr_name, "isfamily"))
        perms->is_family=atoi(attr_value);
      else if(!strcmp(attr_name, "permcomment"))
        perms->perm_comment=atoi(attr_value);
      else if(!strcmp(attr_name, "permaddmeta"))
        perms->perm_addmeta=atoi(attr_value);
    }

    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "perms: ispublic %d iscontact %d isfriend %d isfamily %d permcomment %d permaddmeta %d\n",
            perms->is_public, perms->is_contact,
            perms->is_friend, perms->is_family,
            perms->perm_comment, perms->perm_addmeta);
#endif

    /* Handle only first perm */
    break;
  } /* for nodes */

 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return perms;
}
