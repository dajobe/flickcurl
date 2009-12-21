/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * method.c - Flickcurl method functions
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
 * flickcurl_free_method:
 * @method: method object
 *
 * Destructor for method object
 */
void
flickcurl_free_method(flickcurl_method *method)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(method, flickcurl_method);

  if(method->name)
    free(method->name);
  if(method->description)
    free(method->description);
  if(method->response)
    free(method->response);
  if(method->explanation)
    free(method->explanation);
  
  for(i = 0; i < method->args_count; i++)
    flickcurl_free_arg(method->args[i]);

  free(method);
}

typedef enum 
{
  METHOD_FIELD_name,
  METHOD_FIELD_needslogin,
  METHOD_FIELD_description,
  METHOD_FIELD_response,
  METHOD_FIELD_explanation,
  METHOD_FIELD_LAST = METHOD_FIELD_explanation
} flickcurl_method_field_type;

static struct {
  const xmlChar* xpath;
  flickcurl_method_field_type field;
} method_fields_table[METHOD_FIELD_LAST + 3] = {
  {
    (const xmlChar*)"/rsp/method/@name",
    METHOD_FIELD_name
  }
  ,
  {
    (const xmlChar*)"/rsp/method/@needslogin",
    METHOD_FIELD_needslogin
  }
  ,
  {
    (const xmlChar*)"/rsp/method/description",
    METHOD_FIELD_description
  }
  ,
  {
    (const xmlChar*)"/rsp/method/response",
    METHOD_FIELD_response
  }
  ,
  {
    (const xmlChar*)"/rsp/method/explanation",
    METHOD_FIELD_explanation,
  }
  ,
  { 
    NULL,
    (flickcurl_method_field_type)0
  }
};


flickcurl_method*
flickcurl_build_method(flickcurl* fc, xmlXPathContextPtr xpathCtx)
{
  int expri;
  flickcurl_method* method = NULL;
  
  method = (flickcurl_method*)calloc(sizeof(flickcurl_method), 1);
  
  for(expri = 0; method_fields_table[expri].xpath; expri++) {
    char *string_value = flickcurl_xpath_eval(fc, xpathCtx, 
                                            method_fields_table[expri].xpath);
    switch(method_fields_table[expri].field) {
      case METHOD_FIELD_name:
        method->name = string_value;
        break;
        
      case METHOD_FIELD_needslogin:
        method->needslogin = atoi(string_value);
        break;
        
      case METHOD_FIELD_description:
        method->description = string_value;
        break;
        
      case METHOD_FIELD_response:
        method->response = string_value;
        break;
        
      case METHOD_FIELD_explanation:
        method->explanation = string_value;
        break;

      default:
        abort();
    }
      
    if(fc->failed)
      goto tidy;
  }

  /* As of 2007-04-15 - the response is different from the docs
   * There is no /method/arguments element
   */
  method->args = flickcurl_build_args(fc, xpathCtx, 
                                    (xmlChar*)"/rsp/arguments/argument", 
                                    &method->args_count);

  tidy:
  if(fc->failed)
    method = NULL;

  return method;
}


