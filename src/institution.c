/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * institution.c - Flickr institution support calls
 *
 * Copyright (C) 2009, David Beckett http://www.dajobe.org/
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


/**
 * flickcurl_free_institution:
 * @institution: institution object
 *
 * Destructor for institution object
 */
void
flickcurl_free_institution(flickcurl_institution *institution)
{
  int i;

  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(institution, flickcurl_institution);

  if(institution->nsid)
    free(institution->nsid);

  if(institution->name)
    free(institution->name);

  if(institution->urls) {
    for(i = 0 ; i <= FLICKCURL_INSTITUTION_URL_LAST; i++)
      free(institution->urls[i]);
    free(institution->urls);
  }

  free(institution);
}


/**
 * flickcurl_free_institutions:
 * @institutions_object: institution object array
 *
 * Destructor for array of institution object
 */
void
flickcurl_free_institutions(flickcurl_institution **institutions_object)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(institutions_object,
                                         flickcurl_institution_array);

  for(i=0; institutions_object[i]; i++)
    flickcurl_free_institution(institutions_object[i]);
  
  free(institutions_object);
}


/* flickcurl_institution fields */
typedef enum {
  /* institution->nsid */
  INSTITUTION_NSID,
  /* institution->date_launch */
  INSTITUTION_DATE_LAUNCH,
  /* institution->name */
  INSTITUTION_NAME,
  /* institution->urls[{urls type}] */
  INSTITUTION_URL
} institution_field_type;


#define INSTITUTION_FIELDS_TABLE_SIZE 34

/*
 * The XPaths here are relative, such as prefixed by /rsp/institution
 */
static struct {
  const xmlChar* xpath;
  flickcurl_institution_url_type institution_url_type;
  institution_field_type institution_field;
} institution_fields_table[INSTITUTION_FIELDS_TABLE_SIZE+1]={
  {
    (const xmlChar*)"./@nsid",
    FLICKCURL_INSTITUTION_URL_NONE,
    INSTITUTION_NSID
  }
  ,
  {
    (const xmlChar*)"./@date_launch",
    FLICKCURL_INSTITUTION_URL_NONE,
    INSTITUTION_DATE_LAUNCH
  }
  ,
  {
    (const xmlChar*)"./name",
    FLICKCURL_INSTITUTION_URL_NONE,
    INSTITUTION_NAME
  }
  ,
  {
    (const xmlChar*)"./urls/url[@type='site']",
    FLICKCURL_INSTITUTION_URL_SITE,
    INSTITUTION_URL
  }
  ,
  {
    (const xmlChar*)"./urls/url[@type='license']",
    FLICKCURL_INSTITUTION_URL_LICENSE,
    INSTITUTION_URL
  }
  ,
  {
    (const xmlChar*)"./urls/url[@type='flickr']",
    FLICKCURL_INSTITUTION_URL_FLICKR,
    INSTITUTION_URL
  }
  ,
  { 
    NULL,
    FLICKCURL_INSTITUTION_URL_NONE,
    (unsigned short)0
  }
};



/* get shapedata from value */
flickcurl_institution**
flickcurl_build_institutions(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                             const xmlChar* xpathExpr,
                             int* institution_count_p)
{
  flickcurl_institution** institutions = NULL;
  int nodes_count;
  int institution_count;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  int i;
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed=1;
    goto tidy;
  }
  
  nodes=xpathObj->nodesetval;
  /* This is a max size - it can include nodes that are CDATA */
  nodes_count = xmlXPathNodeSetGetLength(nodes);
  institutions = (flickcurl_institution**)calloc(sizeof(flickcurl_institution*), nodes_count+1);

  for(i = 0, institution_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    int expri;
    xmlXPathContextPtr xpathNodeCtx = NULL;
    flickcurl_institution* institution;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    institution = (flickcurl_institution*)calloc(sizeof(flickcurl_institution), 1);
    institution->urls = (char**)calloc(FLICKCURL_INSTITUTION_URL_LAST+1,
                                       sizeof(char*));

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;

    for(expri = 0; expri <= FLICKCURL_INSTITUTION_URL_LAST; expri++) {
      if(institution->urls[expri]) {
        free(institution->urls[expri]);
        institution->urls[expri] = NULL;
      }
    }

    for(expri=0; institution_fields_table[expri].xpath; expri++) {
      flickcurl_institution_url_type institution_url_type=institution_fields_table[expri].institution_url_type;
      institution_field_type institution_field=institution_fields_table[expri].institution_field;
      const xmlChar* institution_xpathExpr = institution_fields_table[expri].xpath;
      char *value = NULL;
      
      value = flickcurl_xpath_eval(fc, xpathNodeCtx, institution_xpathExpr);
      if(!value)
        continue;

      switch(institution_field) {
        case INSTITUTION_NSID:
          institution->nsid = value;
          break;
          
        case INSTITUTION_DATE_LAUNCH:
          institution->date_launch = atoi(value);
          value = NULL;
          break;

        case INSTITUTION_NAME:
          institution->name = value;
          break;

        case INSTITUTION_URL:
          institution->urls[(int)institution_url_type] = value;
          break;
      }
      
      if(fc->failed)
        goto institutionstidy;
    } /* end for institution fields */

   institutionstidy:
    if(xpathNodeCtx)
      xmlXPathFreeContext(xpathNodeCtx);

    institutions[institution_count++] = institution;
  } /* for institutions */
  
  if(institution_count_p)
    *institution_count_p = institution_count;
  
 tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    institutions = NULL;

  return institutions;
}


flickcurl_institution*
flickcurl_build_institution(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                            const xmlChar* xpathExpr)
{
  flickcurl_institution** institutions;
  flickcurl_institution* result = NULL;

  institutions=flickcurl_build_institutions(fc, xpathCtx, xpathExpr, NULL);

  if(institutions) {
    result = institutions[0];
    free(institutions);
  }
  
  return result;
}


static const char* flickcurl_institution_url_type_label[FLICKCURL_INSTITUTION_URL_LAST+1]={
  "(none)",
  "site",
  "license",
  "flickr"
};


/**
 * flickcurl_get_institution_url_type_label:
 * @url_type: institution url enum
 *
 * Get label for institution url type
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_institution_url_type_label(flickcurl_institution_url_type url_type)
{
  if(url_type <= FLICKCURL_INSTITUTION_URL_LAST)
    return flickcurl_institution_url_type_label[(int)url_type];
  return NULL;
}
