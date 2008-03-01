/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-licenses.c - Flickr flickr.photos.licenses.* API calls
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


static int
compare_licenses(const void *a, const void *b)
{
  flickcurl_license* l_a=*(flickcurl_license**)a;
  flickcurl_license* l_b=*(flickcurl_license**)b;
  return l_a->id - l_b->id;
}


/**
 * flickcurl_read_licenses:
 * @fc: flickcurl context
 * 
 * Internal - read licenses
 **/
static void
flickcurl_read_licenses(flickcurl *fc)
{
  const char * parameters[5][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  const xmlChar* xpathExpr=NULL;
  int i;
  int size;
  
  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.licenses.getInfo", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  xpathExpr=(const xmlChar*)"/rsp/licenses/license";
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed=1;
    goto tidy;
  }

  nodes=xpathObj->nodesetval;
  size=xmlXPathNodeSetGetLength(nodes);
  fc->licenses=(flickcurl_license**)calloc(1+size, sizeof(flickcurl_license*));

  for(i=0; i < size; i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    xmlAttr* attr;
    flickcurl_license* l;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    l=(flickcurl_license*)calloc(sizeof(flickcurl_license), 1);

    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;
      
      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id")) {
        l->id=atoi(attr_value);
        free(attr_value);
      } else if(!strcmp(attr_name, "name"))
        l->name=attr_value;
      else if(!strcmp(attr_name, "url")) {
        if(strlen(attr_value))
          l->url=attr_value;
        else
          free(attr_value);
      }
    }
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "license: id %d name '%s' url %s\n",
            l->id, l->name, (l->url ? l->url : "(none)"));
#endif
    
    fc->licenses[i]=l;
  } /* for nodes */

  qsort(fc->licenses, size, sizeof(flickcurl_license*), compare_licenses);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
}


/**
 * flickcurl_photos_licenses_getInfo - 
 * @fc: flickcurl context
 * 
 * Get a list of available photo licenses for Flickr.
 *
 * Implements flickr.photos.licenses.getInfo (0.6)
 * 
 * Return value: an array of #flickcurl_license or NULL on failure
 **/
flickcurl_license**
flickcurl_photos_licenses_getInfo(flickcurl *fc)
{
  if(!fc->licenses)
    flickcurl_read_licenses(fc);
  
  return fc->licenses;
}


/**
 * flickcurl_photos_licenses_getInfo_by_id - 
 * @fc: flickcurl context
 * @id: license ID
 * 
 * Get an individual photo license by ID
 *
 * Not part of the Flickr API.
 * 
 * Return value: a #flickcurl_license or NULL on failure
 **/
flickcurl_license*
flickcurl_photos_licenses_getInfo_by_id(flickcurl *fc, int id)
{
  int i;
  
  if(!fc->licenses)
    flickcurl_read_licenses(fc);
  if(!fc->licenses)
    return NULL;
  
  for(i=0; fc->licenses[i]; i++) {
    if(fc->licenses[i]->id == id)
      return fc->licenses[i];
    
    if(fc->licenses[i]->id > id)
      break;
  }
  return NULL;
}


/**
 * flickcurl_photos_licenses_setLicense:
 * @fc: flickcurl context
 * @photo_id: The photo to update the license for.
 * @license_id: The license to apply, or 0 (zero) to remove the current license.
 * 
 * Sets the license for a photo.
 *
 * Implements flickr.photos.licenses.setLicense (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_licenses_setLicense(flickcurl* fc, const char* photo_id,
                                     int license_id)
{
  const char* parameters[9][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  char license_id_s[5];
  
  if(!photo_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "license_id";
  sprintf(license_id_s, "%d", license_id);
  parameters[count++][1]= license_id_s;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.licenses.setLicense", parameters,
                       count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;

  return result;
}
