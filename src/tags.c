/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * tag.c - Flickcurl tag functions
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
 * flickcurl_free_tag:
 * @t: tag object
 *
 * Destructor for tag object
 */
void
flickcurl_free_tag(flickcurl_tag *t)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(t, flickcurl_tag);

  if(t->id)
    free(t->id);
  if(t->author)
    free(t->author);
  if(t->authorname)
    free(t->authorname);
  if(t->raw)
    free(t->raw);
  if(t->cooked)
    free(t->cooked);
  free(t);
}


flickcurl_tag**
flickcurl_build_tags(flickcurl* fc, flickcurl_photo* photo,
                     xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr,
                     int* tag_count_p)
{
  flickcurl_tag** tags = NULL;
  int nodes_count;
  int tag_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do tags */
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
  tags = (flickcurl_tag**)calloc(sizeof(flickcurl_tag*), nodes_count+1);
  
  for(i = 0, tag_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_tag* t;
    int saw_clean = 0;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    t = (flickcurl_tag*)calloc(sizeof(flickcurl_tag), 1);
    t->photo = photo;
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        t->id = attr_value;
      else if(!strcmp(attr_name, "author"))
        t->author = attr_value;
      else if(!strcmp(attr_name, "authorname"))
        t->authorname = attr_value;
      else if(!strcmp(attr_name, "raw"))
        t->raw = attr_value;
      else if(!strcmp(attr_name, "clean")) {
        t->cooked = attr_value;
        /* If we see @clean we are expecting
         * <tag clean = "cooked"><raw>raw</raw></tag>
         */
        saw_clean = 1;
      } else if(!strcmp(attr_name, "machine_tag")) {
        t->machine_tag = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "count")) {
        t->count = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "score")) {
        /* from tags.getHotList <tag score = "NN">TAG</tag> */
        t->count = atoi(attr_value);
        free(attr_value);
      }
    }

    /* Walk children nodes for <raw> element or text */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      const char *chnode_name = (const char*)chnode->name;
      if(chnode->type == XML_ELEMENT_NODE) {
        if(saw_clean && !strcmp(chnode_name, "raw")) {
          t->raw = (char*)malloc(strlen((const char*)chnode->children->content)+1);
          strcpy(t->raw, (const char*)chnode->children->content);
        }
      } else if(chnode->type == XML_TEXT_NODE) {
        if(!saw_clean) {
          t->cooked = (char*)malloc(strlen((const char*)chnode->content)+1);
          strcpy(t->cooked, (const char*)chnode->content);
        }
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "tag: id %s author ID %s name %s raw '%s' cooked '%s' count %d\n",
            t->id, t->author, t->authorname, t->raw, t->cooked, t->count);
#endif
    
    if(fc->tag_handler)
      fc->tag_handler(fc->tag_data, t);
    
    tags[tag_count++] = t;
  } /* for nodes */

  if(tag_count_p)
    *tag_count_p = tag_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return tags;
}


flickcurl_tag**
flickcurl_build_tags_from_string(flickcurl* fc, flickcurl_photo* photo,
                                 const char *string, int *tag_count_p)
{
  flickcurl_tag** tags = NULL;
  int nodes_count;
  int tag_count;
  int i;
  
  nodes_count = 0;
  for(i = 0; string[i]; i++) {
    if(string[i] == ' ')
      nodes_count++;
  }
  
  tags = (flickcurl_tag**)calloc(sizeof(flickcurl_tag*), nodes_count+1);
  
  for(i = 0, tag_count = 0; i < nodes_count; i++) {
    flickcurl_tag* t;
    const char *p = string;
    size_t len;
    
    t = (flickcurl_tag*)calloc(sizeof(flickcurl_tag), 1);
    t->photo = photo;

    while(*p && *p != ' ')
      p++;
    
    len = p-string;

    t->cooked = (char*)malloc(len+1);
    strncpy(t->cooked, string, len);
    t->cooked[len] = '\0';
    
    if(fc->tag_handler)
      fc->tag_handler(fc->tag_data, t);
    
    tags[tag_count++] = t;

    /* move past space */
    string = p+1;
  }

  if(tag_count_p)
    *tag_count_p = tag_count;
  
  return tags;
}


/*
 * flickcurl_free_tag_cluster:
 * @tc: tag cluster object
 *
 * INTERNAL - Destructor for tag cluster object
 */
static void
flickcurl_free_tag_cluster(flickcurl_tag_cluster *tc)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(tc, flickcurl_tag_cluster);

  if(tc->tags) {
    int i;
    for(i = 0; tc->tags[i]; i++)
      free(tc->tags[i]);
    free(tc->tags);
  }
  free(tc);
}


/**
 * flickcurl_free_tag_clusters:
 * @tcs: tag clusters object
 *
 * Destructor for tag clusters object
 */
void
flickcurl_free_tag_clusters(flickcurl_tag_clusters *tcs)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(tcs, flickcurl_tag_clusters);

  if(tcs->clusters) {
    int i;
    for(i = 0; tcs->clusters[i]; i++)
      flickcurl_free_tag_cluster(tcs->clusters[i]);
    free(tcs->clusters);
  }
  free(tcs);
}


flickcurl_tag_clusters*
flickcurl_build_tag_clusters(flickcurl* fc, 
                             xmlXPathContextPtr xpathCtx,
                             const xmlChar* xpathExpr)
{
  flickcurl_tag_clusters* tcs = NULL;
  int nodes_count;
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
  
  tcs = (flickcurl_tag_clusters*)calloc(sizeof(flickcurl_tag_clusters), 1);

  /* <cluster> XML element nodes */
  nodes = xpathObj->nodesetval;
  /* This is a max size - it can include nodes that are CDATA */
  nodes_count = xmlXPathNodeSetGetLength(nodes);

  tcs->clusters = (flickcurl_tag_cluster**)calloc(sizeof(flickcurl_tag_cluster*), nodes_count+1);
  
  for(i = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    xmlNodePtr chnode;
    flickcurl_tag_cluster* tc = NULL;
    int tags_count= -1;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }

    tc = (flickcurl_tag_cluster*)calloc(sizeof(flickcurl_tag_cluster), 1);
    if(!tc) {
      fc->failed = 1;
      break;
    }

    /* get <cluster @total> */
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      const char *attr_value = (const char*)attr->children->content;
      
      if(!strcmp(attr_name, "total"))
        tags_count = atoi(attr_value);
    }

    if(tags_count <= 0)
      continue;
    
    tc->tags = (char**)calloc(sizeof(char*), tags_count+1);

    /* Walk children nodes of <cluster> for <tag> elements */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      const char *chnode_name = (const char*)chnode->name;
      if(chnode->type == XML_ELEMENT_NODE && !strcmp(chnode_name, "tag")) {
        size_t len = strlen((const char*)chnode->children->content);
        char *tag_name = (char*)malloc(len+1);
        strcpy(tag_name, (const char*)chnode->children->content);
        tc->tags[tc->count++] = tag_name;

#if FLICKCURL_DEBUG > 1
        fprintf(stderr, "cluster #%d tag #%d: %s\n", tcs->count, tc->count,
                tag_name);
#endif
    
      }
    }
    tc->tags[tc->count] = NULL;
    
    tcs->clusters[tcs->count++] = tc;
  } /* for <cluster> nodes */

  tcs->clusters[tcs->count] = NULL;

 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return tcs;
}


