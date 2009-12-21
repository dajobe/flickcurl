/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * contacts.c - Flickcurl method contact functions
 *
 * Copyright (C) 2007-2009, David Beckett http://www.dajobe.org/
 * Copyright (C) 2007 Vanilla I. Shu 
 *   (flickcurl_free_contact, flickcurl_free_contacts,
 *    flickcurl_build_contacts)
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
 * flickcurl_free_contact:
 * @contact_object: contact object
 *
 * Destructor for contact object
 */
void
flickcurl_free_contact(flickcurl_contact *contact_object)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(contact_object, flickcurl_contact);

  if(contact_object->nsid)
    free(contact_object->nsid);
  if(contact_object->username)
    free(contact_object->username);
  if(contact_object->realname)
    free(contact_object->realname);

  free(contact_object);
}


/**
 * flickcurl_free_contacts:
 * @contacts_object: contact object array
 *
 * Destructor for array of contact object
 */
void
flickcurl_free_contacts(flickcurl_contact **contacts_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(contacts_object, flickcurl_contact);

  for(i = 0; contacts_object[i]; i++)
    flickcurl_free_contact(contacts_object[i]);
  
  free(contacts_object);
}


flickcurl_contact**
flickcurl_build_contacts(flickcurl* fc, 
                         xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr,
                         int* contact_count_p)
{
  flickcurl_contact** contacts = NULL;
  int nodes_count;
  int contact_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do contacts */
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
  contacts = (flickcurl_contact**)calloc(sizeof(flickcurl_contact*), nodes_count+1);
  
  for(i = 0, contact_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_contact* contact_object;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    contact_object = (flickcurl_contact*)calloc(sizeof(flickcurl_contact), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;
      
      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "nsid"))
        contact_object->nsid = attr_value;
      else if(!strcmp(attr_name, "username"))
        contact_object->username = attr_value;
      else if(!strcmp(attr_name, "iconserver")) {
        contact_object->iconserver = atoi((const char*)attr_value);
        free(attr_value);
      }
      else if(!strcmp(attr_name, "realname"))
        contact_object->realname = attr_value;
      else if(!strcmp(attr_name, "friend")) {
        contact_object->is_friend = atoi((const char*)attr_value);
        free(attr_value);
      }
      else if(!strcmp(attr_name, "family")) {
        contact_object->is_family = atoi((const char*)attr_value);
        free(attr_value);
      }
      else if(!strcmp(attr_name, "ignored")) {
        contact_object->ignored = atoi((const char*)attr_value);
        free(attr_value);
      }
      else if(!strcmp(attr_name, "uploaded")) {
        contact_object->uploaded = atoi((const char*)attr_value);
        free(attr_value);
      }
      else
        free(attr_value);
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "contact: NSID %s username %s iconserver %d realname %s friend %d family %d ignored %d uploaded %d\n",
            contact_object->nsid,
            contact_object->username,
            contact_object->iconserver,
            contact_object->realname,
            contact_object->is_friend,
            contact_object->is_family,
            contact_object->ignored,
            contact_object->uploaded);
#endif
    
    contacts[contact_count++] = contact_object;
  } /* for nodes */

  if(contact_count_p)
    *contact_count_p = contact_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return contacts;
}
