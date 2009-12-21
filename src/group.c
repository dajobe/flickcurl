/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * group.c - Flickcurl group functions
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
 * flickcurl_free_group:
 * @group: group object
 *
 * Destructor for group object
 */
void
flickcurl_free_group(flickcurl_group *group)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(group, flickcurl_group);

  if(group->nsid)
    free(group->nsid);
  
  if(group->name)
    free(group->name);
  
  if(group->description)
    free(group->description);
  
  if(group->lang)
    free(group->lang);
  
  if(group->throttle_mode)
    free(group->throttle_mode);
  
  free(group);
}


/**
 * flickcurl_free_groups:
 * @groups_object: group object array
 *
 * Destructor for array of group object
 */
void
flickcurl_free_groups(flickcurl_group **groups_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(groups_object, flickcurl_group_array);

  for(i = 0; groups_object[i]; i++)
    flickcurl_free_group(groups_object[i]);
  
  free(groups_object);
}


flickcurl_group**
flickcurl_build_groups(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* xpathExpr, int* group_count_p)
{
  flickcurl_group** groups = NULL;
  int nodes_count;
  int group_count;
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
  groups = (flickcurl_group**)calloc(sizeof(flickcurl_group*), nodes_count+1);
  
  for(i = 0, group_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_group* g;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    g = (flickcurl_group*)calloc(sizeof(flickcurl_group), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "nsid") || !strcmp(attr_name, "id"))
        g->nsid = attr_value;
      else if(!strcmp(attr_name, "name"))
        g->name = attr_value;
      else if(!strcmp(attr_name, "lang"))
        g->lang = attr_value;
      else if(!strcmp(attr_name, "admin")) {
        g->is_admin = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "privacy")) {
        g->privacy = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "photos")) {
        g->photos = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "iconserver")) {
        g->iconserver = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "ispoolmoderated")) {
        g->is_pool_moderated = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "eightteenplus")) {
        g->is_eighteenplus = atoi(attr_value);
        free(attr_value);
      }
    } /* end attributes */


    /* Walk children nodes for elements */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      const char *chnode_name = (const char*)chnode->name;
      char* value;
      if(chnode->type != XML_ELEMENT_NODE)
        continue;
      
      if(!strcmp(chnode_name, "throttle")) {
        for(attr = chnode->properties; attr; attr = attr->next) {
          const char *attr_name = (const char*)attr->name;
          char *attr_value;
          
          attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
          strcpy(attr_value, (const char*)attr->children->content);
          if(!strcmp(attr_name, "count")) {
            g->throttle_count = atoi(attr_value);
            free(attr_value);
          } else if(!strcmp(attr_name, "mode"))
            g->throttle_mode = attr_value;
          else if(!strcmp(attr_name, "remaining")) {
            g->throttle_remaining = atoi(attr_value);
            free(attr_value);
          } else
            free(attr_value);
        }
        continue;
      }

      if(!chnode->children)
        continue;
      
      value = (char*)malloc(strlen((const char*)chnode->children->content)+1);
      strcpy(value, (const char*)chnode->children->content);

      if(!strcmp(chnode_name, "name"))
        g->name = value;
      else if(!strcmp(chnode_name, "description"))
        g->description = value;
      else if(!strcmp(chnode_name, "members")) {
        g->members = atoi(value);
        free(value);
      } else if(!strcmp(chnode_name, "privacy")) {
        g->privacy = atoi(value);
        free(value);
      }
    }
    

#if FLICKCURL_DEBUG > 1
    fprintf(stderr,
            "group: nsid %s  name '%s'  description '%s'  lang '%s'\n"
            "  admin %d  pool moderated %d  18+ %d  privacy %d\n"
            "  photos %d  iconserver %d  members %d\n"
            "  throttle count %d  mode '%s'  remaining %d\n",
            g->nsid, g->name, g->description, g->lang,
            g->is_admin, g->is_pool_moderated, g->is_eighteenplus, g->privacy,
            g->photos, g->iconserver, g->members,
            g->throttle_count, g->throttle_mode, g->throttle_remaining);
#endif
    
    groups[group_count++] = g;
  } /* for nodes */

  if(group_count_p)
    *group_count_p = group_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return groups;
}
