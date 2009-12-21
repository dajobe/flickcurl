/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * ticket.c - Flickcurl ticket functions
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
 * flickcurl_free_ticket:
 * @ticket: ticket object
 *
 * Destructor for ticket object
 */
void
flickcurl_free_ticket(flickcurl_ticket *ticket)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(ticket, flickcurl_ticket);

  free(ticket);
}


/**
 * flickcurl_free_tickets:
 * @tickets_object: ticket object array
 *
 * Destructor for array of ticket objects
 */
void
flickcurl_free_tickets(flickcurl_ticket **tickets_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(tickets_object, flickcurl_ticket_array);

  for(i = 0; tickets_object[i]; i++)
    flickcurl_free_ticket(tickets_object[i]);
  
  free(tickets_object);
}


flickcurl_ticket**
flickcurl_build_tickets(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* xpathExpr, int* ticket_count_p)
{
  flickcurl_ticket** tickets = NULL;
  int nodes_count;
  int ticket_count;
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
  /* This is a max ticket - it can include nodes that are CDATA */
  nodes_count = xmlXPathNodeSetGetLength(nodes);
  tickets = (flickcurl_ticket**)calloc(sizeof(flickcurl_ticket*), nodes_count+1);
  
  for(i = 0, ticket_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_ticket* t;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    t = (flickcurl_ticket*)calloc(sizeof(flickcurl_ticket), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id")) {
        t->id = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "complete")) {
        t->complete = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "photoid")) {
        t->photoid = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "invalid")) {
        t->invalid = atoi(attr_value);
        free(attr_value);
      }
    }

#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "ticket: label %s width %d height %d  source %s url %s\n",
            t->label, t->width, t->height, t->source, t->url);
#endif
    
    tickets[ticket_count++] = t;
  } /* for nodes */

  if(ticket_count_p)
    *ticket_count_p = ticket_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return tickets;
}
