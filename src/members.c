/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * members.c - Flickcurl method member functions
 *
 * Copyright (C) 2009, David Beckett http://www.dajobe.org/
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
 * flickcurl_free_member:
 * @member_object: member object
 *
 * Destructor for member object
 */
void
flickcurl_free_member(flickcurl_member *member_object)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(member_object, flickcurl_member);

  if(member_object->nsid)
    free(member_object->nsid);
  if(member_object->username)
    free(member_object->username);

  free(member_object);
}


/**
 * flickcurl_free_members:
 * @members_object: member object array
 *
 * Destructor for array of member object
 */
void
flickcurl_free_members(flickcurl_member **members_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(members_object, flickcurl_member);

  for(i=0; members_object[i]; i++)
    flickcurl_free_member(members_object[i]);
  
  free(members_object);
}


flickcurl_member**
flickcurl_build_members(flickcurl* fc, 
                        xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr,
                        int* member_count_p)
{
  flickcurl_member** members = NULL;
  int nodes_count;
  int member_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do members */
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
  members = (flickcurl_member**)calloc(sizeof(flickcurl_member*),
                                       nodes_count+1);
  
  for(i=0, member_count=0; i < nodes_count; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_member* member_object;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    member_object = (flickcurl_member*)calloc(sizeof(flickcurl_member), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;
      
      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "nsid"))
        member_object->nsid=attr_value;
      else if(!strcmp(attr_name, "username"))
        member_object->username=attr_value;
      else if(!strcmp(attr_name, "iconserver")) {
        member_object->iconserver = atoi((const char*)attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "iconfarm")) {
        member_object->iconserver = atoi((const char*)attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "membertype")) {
        member_object->member_type = atoi((const char*)attr_value);
        free(attr_value);
      } else
        free(attr_value);
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "member: NSID %s username %s iconserver %d iconfarm %d member type %d\n",
            member_object->nsid,
            member_object->username,
            member_object->iconserver,
            member_object->iconfarm,
            member_object->member_type);
#endif
    
    members[member_count++] = member_object;
  } /* for nodes */

  if(member_count_p)
    *member_count_p = member_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return members;
}
