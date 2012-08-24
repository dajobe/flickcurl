/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * topic.c - Flickr topic/reply support calls
 *
 * Copyright (C) 2012, David Beckett http://www.dajobe.org/
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


static const char* flickcurl_topic_field_label[TOPIC_FIELD_LAST + 1] = {
  "(none)",
  "subject",
  "group NSID",
  "group icon server",
  "group icon farm",
  "group name",
  "message",
  "author NSID",
  "author name",
  "author role",
  "author icon server",
  "author icon farm",
  "author can edit",
  "author can delete",
  "date created",
  "last edited",
  "reply to topic NSID"
};


/**
 * flickcurl_get_topic_field_label:
 * @field: field enum
 *
 * Get label for topic field
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_topic_field_label(flickcurl_topic_field_type field)
{
  if(field <= TOPIC_FIELD_LAST)
    return flickcurl_topic_field_label[(int)field];

  return NULL;
}


/**
 * flickcurl_free_topic:
 * @topic: topic object
 *
 * Destructor for topic object
 */
void
flickcurl_free_topic(flickcurl_topic *topic)
{
  int i;

  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(topic, flickcurl_topic);

  for(i = 0; i <= TOPIC_FIELD_LAST; i++) {
    if(topic->fields[i].string)
      free(topic->fields[i].string);
  }
  
  if(topic->nsid)
    free(topic->nsid);
  
  free(topic);
}


/*
 * The XPaths here are relative, such as prefixed by /rsp/topic
 */
static struct {
  const xmlChar* xpath;
  flickcurl_topic_field_type field;
  flickcurl_field_value_type type;
} topic_fields_table[PHOTO_FIELD_LAST + 4] = {
  {
    (const xmlChar*)"./@topic_id",
    TOPIC_FIELD_none,
    VALUE_TYPE_TOPIC_ID,
  }
  ,
  {
    (const xmlChar*)"./@subject",
    TOPIC_FIELD_subject,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@group_id",
    TOPIC_FIELD_group_nsid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@iconserver",
    TOPIC_FIELD_group_iconserver,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@iconfarm",
    TOPIC_FIELD_group_iconfarm,
    VALUE_TYPE_INTEGER
  }
  ,
  { 
    NULL,
    (flickcurl_topic_field_type)0,
    (flickcurl_field_value_type)0
  }
};



flickcurl_topic_list*
flickcurl_build_topic_list(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                           const xmlChar* xpathExpr)
{
  flickcurl_topic_list* topic_list = NULL;
  flickcurl_topic** topic_array = NULL;
  int nodes_count;
  int topic_count;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  xmlChar full_xpath[512];
  size_t xpathExpr_len;
  int i;
  
  xpathExpr_len = strlen((const char*)xpathExpr);
  strncpy((char*)full_xpath, (const char*)xpathExpr, xpathExpr_len+1);
  
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
  topic_array = (flickcurl_topic**)calloc(sizeof(flickcurl_topic*),
                                             nodes_count + 1);
  topic_list = (flickcurl_topic_list*)calloc(sizeof(*topic_list), 1);
  topic_list->topics = topic_array;

  for(i = 0, topic_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    flickcurl_topic* topic;
    int expri;
    xmlXPathContextPtr xpathNodeCtx = NULL;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    topic = (flickcurl_topic*)calloc(sizeof(flickcurl_topic), 1);

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;

    for(expri = 0; expri <= TOPIC_FIELD_LAST; expri++) {
      if(topic->fields[expri].string)
        free(topic->fields[expri].string);
      topic->fields[expri].string = NULL;
      topic->fields[expri].integer= (flickcurl_topic_field_type)-1;
      topic->fields[expri].type   = VALUE_TYPE_NONE;
    }

    for(expri = 0; topic_fields_table[expri].xpath; expri++) {
      flickcurl_topic_field_type field = topic_fields_table[expri].field;
      flickcurl_field_value_type datatype = topic_fields_table[expri].type;
      char *string_value;
      int int_value= -1;
      time_t unix_time;
      
      string_value = flickcurl_xpath_eval(fc, xpathNodeCtx,
                                          topic_fields_table[expri].xpath);
      if(!string_value)
        continue;
      
      switch(datatype) {
        case VALUE_TYPE_TOPIC_ID:
          topic->nsid = string_value;
          string_value = NULL;
          datatype = VALUE_TYPE_NONE;
          break;
          
        case VALUE_TYPE_UNIXTIME:
        case VALUE_TYPE_DATETIME:
          
          if(datatype == VALUE_TYPE_UNIXTIME)
            unix_time = atoi(string_value);
          else
            unix_time = curl_getdate((const char*)string_value, NULL);
          
          if(unix_time >= 0) {
            char* new_value = flickcurl_unixtime_to_isotime(unix_time);
#if FLICKCURL_DEBUG > 1
            fprintf(stderr, "  date from: '%s' unix time %ld to '%s'\n",
                    value, (long)unix_time, new_value);
#endif
            free(string_value);
            string_value= new_value;
            int_value= (int)unix_time;
            datatype = VALUE_TYPE_DATETIME;
          } else
            /* failed to convert, make it a string */
            datatype = VALUE_TYPE_STRING;
          break;
          
        case VALUE_TYPE_INTEGER:
        case VALUE_TYPE_BOOLEAN:
          int_value = atoi(string_value);
          break;
          
        case VALUE_TYPE_NONE:
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_FLOAT:
        case VALUE_TYPE_URI:
          break;
          
        case VALUE_TYPE_PERSON_ID:
        case VALUE_TYPE_PHOTO_ID:
        case VALUE_TYPE_PHOTO_URI:
        case VALUE_TYPE_MEDIA_TYPE:
        case VALUE_TYPE_TAG_STRING:
        case VALUE_TYPE_COLLECTION_ID:
        case VALUE_TYPE_ICON_PHOTOS:
          abort();
      }
      
      topic->fields[field].string = string_value;
      topic->fields[field].integer= (flickcurl_topic_field_type)int_value;
      topic->fields[field].type   = datatype;
      
#if FLICKCURL_DEBUG > 1
      fprintf(stderr, "field %d with %s value: '%s' / %d\n",
              field, flickcurl_field_value_type_label[datatype], 
              string_value, int_value);
#endif
      
      if(fc->failed)
        goto tidy;
    }

    topic_list->topics[topic_count++] = topic;
  } /* for topics */
  

  topic_list->topics_count = topic_count;

 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    topic_list = NULL;

  return topic_list;
}


flickcurl_topic*
flickcurl_build_topic(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                      const xmlChar* root_xpathExpr)
{
  flickcurl_topic_list* topic_list;
  flickcurl_topic* result = NULL;

  topic_list = flickcurl_build_topic_list(fc, xpathCtx, root_xpathExpr);
  if(topic_list) {
    result = topic_list->topics[0];
    topic_list->topics[0] = NULL;
    flickcurl_free_topic_list(topic_list);
  }
  
  return result;
}


/**
 * flickcurl_free_topics:
 * @topics: topic object array
 *
 * Destructor for array of topic object
 */
void
flickcurl_free_topic_list(flickcurl_topic_list* topic_list)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(topic_list, flickcurl_topic_list);

  if(topic_list->topics) {
    for(i = 0; topic_list->topics[i]; i++)
      flickcurl_free_topic(topic_list->topics[i]);
    free(topic_list->topics);
  }

  if(topic_list->group_nsid)
    free(topic_list->group_nsid);
  if(topic_list->name)
    free(topic_list->name);
  if(topic_list->lang)
    free(topic_list->lang);
  free(topic_list);
}
