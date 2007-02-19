/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * api.c - Flickr API calls
 *
 * Copyright (C) 2007, David Beckett http://purl.org/net/dajobe/
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/*
 **********************************************************************
 * Flickr API Calls
 **********************************************************************
 */

/* Flickr test echo */
int
flickcurl_test_echo(flickcurl* fc, const char* key, const char* value)
{
  const char * parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int rc=0;
  
  parameters[count][0]  = key;
  parameters[count++][1]= value;

  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

  if(flickcurl_prepare(fc, "flickr.test.echo", parameters, count)) {
    rc=1;
    goto tidy;
  }

  doc=flickcurl_invoke(fc);
  if(!doc) {
    rc=1;
    goto tidy;
  }

  fprintf(stderr, "Flickr echo returned %d bytes\n", fc->total_bytes);
  
  tidy:
  
  return rc;
}


static const char* flickcurl_photo_field_label[PHOTO_FIELD_LAST+1]={
  "(none)",
  "dateuploaded",
  "farm",
  "isfavorite",
  "license",
  "originalformat",
  "rotation",
  "server",
  "dates_lastupdate",
  "dates_posted",
  "dates_taken",
  "dates_takengranularity",
  "description",
  "editability_canaddmeta",
  "editability_cancomment",
  "geoperms_iscontact",
  "geoperms_isfamily",
  "geoperms_isfriend",
  "geoperms_ispublic",
  "location_accuracy",
  "location_latitude",
  "location_longitude",
  "owner_location",
  "owner_nsid",
  "owner_realname",
  "owner_username",
  "title",
  "visibility_isfamily",
  "visibility_isfriend",
  "visibility_ispublic",
  "secret",
  "originalsecret"
};


const char*
flickcurl_get_photo_field_label(flickcurl_photo_field field)
{
  if(field <= PHOTO_FIELD_LAST)
    return flickcurl_photo_field_label[(int)field];
  return NULL;
}


static const char* flickcurl_field_value_type_label[VALUE_TYPE_LAST+1]={
  "(none)",
  "photo id",
  "photo URI",
  "unix time",
  "boolean",
  "dateTime",
  "float",
  "integer",
  "string",
  "uri"
};


const char*
flickcurl_get_field_value_type_label(flickcurl_field_value_type datatype)
{
  if(datatype <= VALUE_TYPE_LAST)
    return flickcurl_field_value_type_label[(int)datatype];
  return NULL;
}


static int compare_licenses(const void *a, const void *b)
{
  flickcurl_license* l_a=*(flickcurl_license**)a;
  flickcurl_license* l_b=*(flickcurl_license**)b;
  return l_a->id - l_b->id;
}


static void flickcurl_read_licenses(flickcurl *fc)
{
  const char * parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  const xmlChar* xpathExpr=NULL;
  int i;
  int size;
  
  /* does not require authentication */
  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

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
      
      if(!strcmp(attr_name, "id"))
        l->id=atoi(attr_value);
      else if(!strcmp(attr_name, "name"))
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


flickcurl_license** flickcurl_photos_licenses_getInfo(flickcurl *fc)
{
  if(!fc->licenses)
    flickcurl_read_licenses(fc);
  
  return fc->licenses;
}


flickcurl_license* flickcurl_photos_licenses_getInfo_by_id(flickcurl *fc, int id)
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


static const char* flickcurl_person_field_label[PERSON_FIELD_LAST+1]={
  "(none)",
  "isadmin",
  "ispro",
  "iconserver",
  "iconfarm",
  "username",
  "realname",
  "mbox_sha1sum",
  "location",
  "photosurl",
  "profileurl",
  "mobileurl",
  "photos_firstdate",
  "photos_firstdatetaken",
  "photos_count",
};


const char*
flickcurl_get_person_field_label(flickcurl_person_field field)
{
  if(field <= PERSON_FIELD_LAST)
    return flickcurl_person_field_label[(int)field];
  return NULL;
}


/* This is the element name and the label - lazy */
const char* flickcurl_context_type_element[FLICKCURL_CONTEXT_LAST+2]={
  "---",
  "set",
  "pool",
  "prevphoto",
  "nextphoto",
  NULL
};


const char*
flickcurl_get_context_type_field_label(flickcurl_context_type type)
{
  if(type > FLICKCURL_CONTEXT_NONE && type <= FLICKCURL_CONTEXT_LAST)
    return flickcurl_context_type_element[(int)type];
  return NULL;
}


/**
 * flickcurl_groups_pools_getContext:
 * @fc: flickcurl context
 * @id: photo ID
 * 
 * Returns an array of size 3 [prev, next, NULL] flickcurl_context*
 * or NULL on error
 * 
 * Return value: prev, next contexts or NULL
 **/
flickcurl_context**
flickcurl_groups_pools_getContext(flickcurl* fc, const char* photo_id,
                                  const char* group_id)
{
  const char * parameters[5][2];
  int count=0;
  xmlDocPtr doc=NULL;
  flickcurl_context** contexts=NULL;
  
  if(!photo_id || !group_id)
    return NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "group_id";
  parameters[count++][1]= group_id;

  /* does not require authentication */
  if(fc->auth_token) {
    parameters[count][0]  = "token";
    parameters[count++][1]= fc->auth_token;
  }

  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

  if(flickcurl_prepare(fc, "flickr.groups.pools.getContext", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  contexts=flickcurl_build_contexts(fc, doc);

 tidy:
  if(fc->failed)
    contexts=NULL;

  return contexts;
}


/**
 * flickcurl_photosets_getContext:
 * @fc: flickcurl context
 * @id: photo ID
 * 
 * Returns an array of size 3 [prev, next, NULL] flickcurl_context*
 * or NULL on error
 * 
 * Return value: prev, next contexts or NULL
 **/
flickcurl_context**
flickcurl_photosets_getContext(flickcurl* fc, const char* photo_id,
                               const char* photoset_id)
{
  const char * parameters[5][2];
  int count=0;
  xmlDocPtr doc=NULL;
  flickcurl_context** contexts=NULL;
  
  if(!photo_id || !photoset_id)
    return NULL;
  
  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "photoset_id";
  parameters[count++][1]= photoset_id;

  /* does not require authentication */
  if(fc->auth_token) {
    parameters[count][0]  = "token";
    parameters[count++][1]= fc->auth_token;
  }

  parameters[count][0]  = NULL;

  flickcurl_set_sig_key(fc, NULL);

  if(flickcurl_prepare(fc, "flickr.photosets.getContext", parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  contexts=flickcurl_build_contexts(fc, doc);

 tidy:
  if(fc->failed)
    contexts=NULL;

  return contexts;
}
