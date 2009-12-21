/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * comments.c - Flickcurl method comment functions
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
 * flickcurl_free_comment:
 * @comment_object: comment object
 *
 * Destructor for comment object
 */
void
flickcurl_free_comment(flickcurl_comment *comment_object)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(comment_object, flickcurl_comment);

  if(comment_object->id)
    free(comment_object->id);
  if(comment_object->author)
    free(comment_object->author);
  if(comment_object->authorname)
    free(comment_object->authorname);
  if(comment_object->permalink)
    free(comment_object->permalink);
  if(comment_object->text)
    free(comment_object->text);

  free(comment_object);
}


/**
 * flickcurl_free_comments:
 * @comments_object: comment object array
 *
 * Destructor for array of comment object
 */
void
flickcurl_free_comments(flickcurl_comment **comments_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(comments_object, flickcurl_comment_array);

  for(i = 0; comments_object[i]; i++)
    flickcurl_free_comment(comments_object[i]);
  
  free(comments_object);
}


flickcurl_comment**
flickcurl_build_comments(flickcurl* fc, 
                         xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr,
                         int* comment_count_p)
{
  flickcurl_comment** comments = NULL;
  int nodes_count;
  int comment_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do comments */
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
  comments = (flickcurl_comment**)calloc(sizeof(flickcurl_comment*), nodes_count+1);
  
  for(i = 0, comment_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_comment* comment_object;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    comment_object = (flickcurl_comment*)calloc(sizeof(flickcurl_comment), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;
      
      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        comment_object->id = attr_value;
      else if(!strcmp(attr_name, "author"))
        comment_object->author = attr_value;
      else if(!strcmp(attr_name, "authorname"))
        comment_object->authorname = attr_value;
      else if(!strcmp(attr_name, "datecreate")) {
        comment_object->datecreate = atoi((const char*)attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "permalink"))
        comment_object->permalink = attr_value;
      else
        free(attr_value);
    }

    /* Walk children nodes for comment text */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        comment_object->text = (char*)malloc(strlen((const char*)chnode->content)+1);
        strcpy(comment_object->text, (const char*)chnode->content);
        break;
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "comment: ID %s author %s authorname %s datecreate %d permalink %s text '%s'\n",
            comment_object->id,
            comment_object->author,
            comment_object->authorname,
            comment_object->datecreate,
            comment_object->permalink,
            comment_object->text);
#endif
    
    comments[comment_count++] = comment_object;
  } /* for nodes */

  if(comment_count_p)
    *comment_count_p = comment_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return comments;
}
