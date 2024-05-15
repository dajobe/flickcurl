/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * gallery.c - Flickcurl gallery functions
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
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_free_gallery:
 * @gallery: gallery object
 *
 * Destructor for gallery object
 */
void
flickcurl_free_gallery(flickcurl_gallery *gallery)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(gallery, flickcurl_gallery);

  if(gallery->id)
    free(gallery->id);
  if(gallery->url)
    free(gallery->url);
  if(gallery->owner)
    free(gallery->owner);

  if(gallery->primary_photo)
    flickcurl_free_photo(gallery->primary_photo);

  if(gallery->title)
    free(gallery->title);
  if(gallery->description)
    free(gallery->description);


  free(gallery);
}


/**
 * flickcurl_free_galleries:
 * @galleries_object: gallery object array
 *
 * Destructor for array of gallery objects
 */
void
flickcurl_free_galleries(flickcurl_gallery **galleries_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(galleries_object,
                                         flickcurl_gallery_array);

  for(i = 0; galleries_object[i]; i++)
    flickcurl_free_gallery(galleries_object[i]);
  
  free(galleries_object);
}


flickcurl_gallery**
flickcurl_build_galleries(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                          const xmlChar* xpathExpr, int* gallery_count_p)
{
  flickcurl_gallery** galleries = NULL;
  int nodes_count;
  int gallery_count;
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
  galleries = (flickcurl_gallery**)calloc(nodes_count+1,
                                          sizeof(flickcurl_gallery*));
  
  for(i = 0, gallery_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_gallery* g;
    xmlNodePtr chnode;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    g = (flickcurl_gallery*)calloc(1, sizeof(flickcurl_gallery));
    
    /* initialise primary photo object */
    g->primary_photo = (flickcurl_photo*)calloc(1, sizeof(flickcurl_photo));
    /* assumes it is a photo */
#define PHOTO_STR_LEN 5
    g->primary_photo->media_type = (char*)malloc(PHOTO_STR_LEN + 1);
    memcpy(g->primary_photo->media_type, "photo", PHOTO_STR_LEN + 1);
    /* empty list of tags (1 NULL pointer) */
    g->primary_photo->tags = (flickcurl_tag**)calloc(1, sizeof(flickcurl_tag*));

    
    for(attr = node->properties; attr; attr = attr->next) {
      size_t attr_len = strlen((const char*)attr->children->content);
      const char *attr_name = (const char*)attr->name;
      char *attr_value;

      attr_value = (char*)malloc(attr_len + 1);
      memcpy(attr_value, attr->children->content, attr_len + 1);
      
      if(!strcmp(attr_name, "id"))
        g->id = attr_value;
      else if(!strcmp(attr_name, "url"))
        g->url = attr_value;
      else if(!strcmp(attr_name, "owner"))
        g->owner = attr_value;
      else if(!strcmp(attr_name, "date_create")) {
        g->date_create = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "date_update")) {
        g->date_update = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "primary_photo_id")) {
        g->primary_photo->id = attr_value;
      } else if(!strcmp(attr_name, "primary_photo_server")) {
        g->primary_photo->fields[PHOTO_FIELD_server].integer = (flickcurl_photo_field_type)atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "primary_photo_farm")) {
        g->primary_photo->fields[PHOTO_FIELD_farm].integer = (flickcurl_photo_field_type)atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "primary_photo_secret")) {
        g->primary_photo->fields[PHOTO_FIELD_secret].string = attr_value;
      } else if(!strcmp(attr_name, "count_photos")) {
        g->count_photos = atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "count_videos")) {
        g->count_videos = atoi(attr_value);
        free(attr_value);
      } else
        free(attr_value);
    } /* end attributes */


    /* Walk children nodes for <title> or <description> elements */
    for(chnode = node->children; chnode; chnode = chnode->next) {
      const char *chnode_name = (const char*)chnode->name;
      if(chnode->type == XML_ELEMENT_NODE) {
        if(!strcmp(chnode_name, "title")) {
          if(chnode->children) {
            size_t len = strlen((const char*)chnode->children->content);
            g->title = (char*)malloc(len + 1);
            memcpy(g->title, chnode->children->content, len + 1);
          }
        } else if(!strcmp(chnode_name, "description")) {
          if(chnode->children) {
            size_t len = strlen((const char*)chnode->children->content);
            g->description = (char*)malloc(len + 1);
            memcpy(g->description, chnode->children->content, len + 1);
          }
        }
      }
    } /* end for children of <item> */

#if FLICKCURL_DEBUG > 1
    fprintf(stderr,
            "gallery: id %s  url %s  owner %s\n"
            "  date create %d  date update %d\n"
            "  count of photos %d  count of videos %d\n"
            "  title '%s'\n"
            "  description '%s'\n"
            ,
            g->id, g->url, g->owner,
            g->date_create, g->date_update,
            g->count_photos, g->count_videos,
            g->title, g->description);
#endif
    
    galleries[gallery_count++] = g;
  } /* for nodes */

  if(gallery_count_p)
    *gallery_count_p = gallery_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return galleries;
}
