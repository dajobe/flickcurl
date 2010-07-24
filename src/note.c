/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * note.c - Flickcurl note functions
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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
 * flickcurl_free_note:
 * @note: note object
 *
 * Destructor for note object
 */
void
flickcurl_free_note(flickcurl_note *note)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(note, flickcurl_note);

  if(note->author)
    free(note->author);
  if(note->authorname)
    free(note->authorname);
  if(note->text)
    free(note->text);
  free(note);
}


flickcurl_note**
flickcurl_build_notes(flickcurl* fc, flickcurl_photo* photo,
                      xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr,
                      int* note_count_p)
{
  flickcurl_note** notes = NULL;
  int nodes_count;
  int note_count;
  int i;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  
  /* Now do notes */
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
  notes = (flickcurl_note**)calloc(sizeof(flickcurl_note*), nodes_count+1);
  
  for(i = 0, note_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_note* n;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    n = (flickcurl_note*)calloc(sizeof(flickcurl_note), 1);
    
    for(attr = node->properties; attr; attr = attr->next) {
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id")) {
        n->id = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "author"))
        n->author = attr_value;
      else if(!strcmp(attr_name, "authorname"))
        n->authorname = attr_value;
      else if(!strcmp(attr_name, "x")) {
        n->x = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "y")) {
        n->y = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "w")) {
        n->w = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "h")) {
        n->h = atoi(attr_value);
        free(attr_value);
      }
    }

    /* Walk children nodes for text */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      if(chnode->type == XML_TEXT_NODE) {
        n->text = (char*)malloc(strlen((const char*)chnode->content)+1);
        strcpy(n->text, (const char*)chnode->content);
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "note: id %d author ID %s name %s x %d y %d w %d h %d text '%s'\n",
            n->id, n->author, n->authorname, n->x, n->y, n->w, n->h, n->text);
#endif
    
    notes[note_count++] = n;
  } /* for nodes */

  if(note_count_p)
    *note_count_p = note_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return notes;
}
