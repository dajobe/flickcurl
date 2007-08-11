/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * group.c - Flickcurl group functions
 *
 * Copyright (C) 2007, David Beckett http://purl.org/net/dajobe/
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


#include <flickcurl.h>
#include <flickcurl_internal.h>


void
flickcurl_free_group(flickcurl_group *group)
{
  if(group->nsid)
    free(group->nsid);
  
  if(group->name)
    free(group->name);
  
  free(group);
}


void
flickcurl_free_groups(flickcurl_group **groups_object)
{
  int i;
  
  for(i=0; groups_object[i]; i++)
    flickcurl_free_group(groups_object[i]);
  
  free(groups_object);
}


flickcurl_group**
flickcurl_build_groups(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* xpathExpr, int* group_count_p)
{
  flickcurl_group** groups=NULL;
  int nodes_count;
  int group_count;
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
  groups=(flickcurl_group**)calloc(sizeof(flickcurl_group*), nodes_count+1);
  
  for(i=0, group_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_group* g;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    g=(flickcurl_group*)calloc(sizeof(flickcurl_group), 1);
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;

      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "nsid"))
        g->nsid=attr_value;
      else if(!strcmp(attr_name, "name"))
        g->name=attr_value;
      else if(!strcmp(attr_name, "admin")) {
        g->is_admin=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "privacy")) {
        g->privacy=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "photos")) {
        g->photos=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "iconserver")) {
        g->iconserver=atoi(attr_value);
        free(attr_value);
      }
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "group: nsid %s name '%s' admin %d  privacy %d  photos %d  iconserver %d\n",
            g->nsid, g->name, g->is_admin, g->privacy, g->photos, g->iconserver);
#endif
    
    groups[group_count++]=g;
  } /* for nodes */

  if(group_count_p)
    *group_count_p=group_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return groups;
}
