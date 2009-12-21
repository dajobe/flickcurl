/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * activity.c - Flickcurl activity functions
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
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


static void
flickcurl_free_activity_event(flickcurl_activity_event *activity_event)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(activity_event, flickcurl_activity_event);
  
  if(activity_event->id)
    free(activity_event->id);
  if(activity_event->type)
    free(activity_event->type);
  if(activity_event->user)
    free(activity_event->user);
  if(activity_event->username)
    free(activity_event->username);
  if(activity_event->value)
    free(activity_event->value);
  free(activity_event);
}

static void
flickcurl_free_activity(flickcurl_activity *activity)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(activity, flickcurl_activity);

  if(activity->type)
    free(activity->type);
  if(activity->id)
    free(activity->id);
  if(activity->owner)
    free(activity->owner);
  if(activity->owner_name)
    free(activity->owner_name);
  if(activity->primary)
    free(activity->primary);
  if(activity->secret)
    free(activity->secret);
  if(activity->title)
    free(activity->title);
  if(activity->events) {
    int i;
    for(i = 0; activity->events[i]; i++) 
      flickcurl_free_activity_event(activity->events[i]);
  }
  free(activity);
}


/**
 * flickcurl_free_activities:
 * @activities_object: activity object array
 *
 * Destructor for array of activity objects
 */
void
flickcurl_free_activities(flickcurl_activity **activities_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(activities_object, flickcurl_activity_array);

  for(i = 0; activities_object[i]; i++)
    flickcurl_free_activity(activities_object[i]);
  
  free(activities_object);
}


static flickcurl_activity_event*
flickcurl_build_activity_event(flickcurl* fc, xmlNodePtr node)
{
  flickcurl_activity_event* ae;
  xmlAttr* attr;
  xmlNodePtr chnode;

  ae = (flickcurl_activity_event*)calloc(sizeof(flickcurl_activity_event), 1);
  if(!ae)
    return NULL;
    
  for(attr = node->properties; attr; attr = attr->next) {
    const char *attr_name = (const char*)attr->name;
    char *attr_value;
    
    attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
    strcpy(attr_value, (const char*)attr->children->content);
      
    if(!strcmp(attr_name, "type"))
      ae->type = attr_value;
    else if(!strcmp(attr_name, "commentid"))
      ae->id = attr_value;
    else if(!strcmp(attr_name, "user"))
      ae->user = attr_value;
    else if(!strcmp(attr_name, "username"))
      ae->username = attr_value;
    else if(!strcmp(attr_name, "dateadded")) {
      ae->date_added = atoi(attr_value);
      free(attr_value);
    } else
      free(attr_value);
  } /* end attributes */

  /* Walk children nodes for value text */
  for(chnode = node->children; chnode; chnode = chnode->next) {
    if(chnode->type == XML_TEXT_NODE) {
      ae->value = (char*)malloc(strlen((const char*)chnode->content)+1);
      strcpy(ae->value, (const char*)chnode->content);
      break;
    }
  }

  return ae;
}


flickcurl_activity**
flickcurl_build_activities(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                           const xmlChar* xpathExpr, int* activity_count_p)
{
  flickcurl_activity** activities = NULL;
  int nodes_count;
  int activity_count;
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
  activities = (flickcurl_activity**)calloc(sizeof(flickcurl_activity*), nodes_count+1);
  
  for(i = 0, activity_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_activity* a;
    xmlNodePtr chnode;
    int events_count = 0;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    a = (flickcurl_activity*)calloc(sizeof(flickcurl_activity), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "type"))
        a->type = attr_value;
      else if(!strcmp(attr_name, "id"))
        a->id = attr_value;
      else if(!strcmp(attr_name, "owner"))
        a->owner = attr_value;
      else if(!strcmp(attr_name, "ownername"))
        a->owner_name = attr_value;
      else if(!strcmp(attr_name, "primary"))
        a->primary = attr_value;
      else if(!strcmp(attr_name, "secret"))
        a->secret = attr_value;
      else if(!strcmp(attr_name, "server")) {
        a->server = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "farm")) {
        a->farm = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "commentsold")) {
        a->comments_old = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "commentsnew")) {
        a->comments_new = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "notesold")) {
        a->notes_old = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "notesnew")) {
        a->notes_new = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "views")) {
        a->views = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "photos")) {
        a->photos = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "faves")) {
        a->faves = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "comments")) {
        a->comments = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "more")) {
        a->more = atoi(attr_value);
        free(attr_value);
      } else
        free(attr_value);
    } /* end attributes */


    /* Walk children nodes for <title> or <activity> elements */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      const char *chnode_name = (const char*)chnode->name;
      if(chnode->type == XML_ELEMENT_NODE) {
        if(!strcmp(chnode_name, "title")) {
          a->title = (char*)malloc(strlen((const char*)chnode->children->content)+1);
          strcpy(a->title, (const char*)chnode->children->content);
        } else if(!strcmp(chnode_name, "activity")) {
          xmlNodePtr chnode2;
          for(chnode2 = chnode->children; chnode2; chnode2 = chnode2->next) {
            const char *chnode2_name = (const char*)chnode2->name;
            if(chnode2->type == XML_ELEMENT_NODE &&
               !strcmp(chnode2_name, "event")) {
              if(events_count < FLICKCURL_MAX_ACTIVITY_EVENTS) {
                flickcurl_activity_event *ae = NULL;
                ae = flickcurl_build_activity_event(fc, chnode2);
                if(ae)
                  a->events[events_count++] = ae;
              }
            }
          } /* end for children of <activity> */

        }
      }
    } /* end for children of <item> */

    a->events[events_count] = 0;
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr,
            "activity: type %s  id %s  owner %s name %s  primary %s\n"
            "  secret %s  server %d farm %d\n"
            "  comments %d old/new %d/%d  notes old/new %d/%d\n"
            "  views %d  photos %d  faves %d  more %d\n"
            "  title '%s'\n"
            ,
            a->type, a->id, a->owner, a->owner_name, a->primary,
            a->secret, a->server, a->farm,
            a->comments, a->comments_old, a->comments_new, a->notes_old, a->notes_new,
            a->views, a->photos, a->faves, a->more,
            a->title);
#endif
    
    activities[activity_count++] = a;
  } /* for nodes */

  if(activity_count_p)
    *activity_count_p = activity_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return activities;
}
