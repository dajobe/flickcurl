/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * common.c - Flickcurl common functions
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

/* for access() and R_OK */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


const char* const flickcurl_short_copyright_string = "Copyright 2007 David Beckett.";

const char* const flickcurl_copyright_string = "Copyright (C) 2007 David Beckett - http://purl.org/net/dajobe/";

const char* const flickcurl_license_string = "LGPL 2.1 or newer, GPL 2 or newer, Apache 2.0 or newer.\nSee http://librdf.org/flickcurl/ for full terms.";

const char* const flickcurl_home_url_string = "http://librdf.org/flickcurl/";
const char* const flickcurl_version_string = VERSION;



static void
flickcurl_error_varargs(flickcurl* fc, const char *message, 
                        va_list arguments)
{
  if(fc->error_handler) {
    char *buffer=my_vsnprintf(message, arguments);
    if(!buffer) {
      fprintf(stderr, "flickcurl: Out of memory\n");
      return;
    }
    fc->error_handler(fc->error_data, buffer);
    free(buffer);
  } else {
    fprintf(stderr, "flickcurl error - ");
    vfprintf(stderr, message, arguments);
    fputc('\n', stderr);
  }
}

  
void
flickcurl_error(flickcurl* fc, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);
  flickcurl_error_varargs(fc, message, arguments);
  va_end(arguments);
}

  
static size_t
flickcurl_write_callback(void *ptr, size_t size, size_t nmemb, 
                         void *userdata) 
{
  flickcurl* fc=(flickcurl*)userdata;
  int len=size*nmemb;
  int rc=0;
  
  if(fc->failed)
    return 0;

  fc->total_bytes += len;
  
  if(!fc->xc) {
    xmlParserCtxtPtr xc;

    xc = xmlCreatePushParserCtxt(NULL, NULL,
                                 (const char*)ptr, len,
                                 (const char*)fc->uri);
    if(!xc)
      rc=1;
    else {
      xc->replaceEntities = 1;
      xc->loadsubset = 1;
    }
    fc->xc=xc;
  } else
    rc=xmlParseChunk(fc->xc, (const char*)ptr, len, 0);

#if FLICKCURL_DEBUG > 2
  fprintf(stderr, "Got >>%s<< (%d bytes)\n", (const char*)ptr, len);
#endif

  if(rc)
    flickcurl_error(fc, "XML Parsing failed");

#ifdef CAPTURE
  if(fc->fh)
    fwrite(ptr, size, nmemb, fc->fh);
#endif
  return len;
}


flickcurl*
flickcurl_new(void)
{
  flickcurl* fc;

  fc=(flickcurl*)calloc(1, sizeof(flickcurl));
  if(!fc)
    return NULL;

  /* DEFAULT delay between requests is 1000ms i.e 1 request/second max */
  fc->request_delay=1000;
  
  if(!fc->curl_handle) {
    fc->curl_handle=curl_easy_init();
    fc->curl_init_here=1;
  }

#ifndef CURLOPT_WRITEDATA
#define CURLOPT_WRITEDATA CURLOPT_FILE
#endif

  /* send all data to this function  */
  curl_easy_setopt(fc->curl_handle, CURLOPT_WRITEFUNCTION, 
                   flickcurl_write_callback);
  /* ... using this data pointer */
  curl_easy_setopt(fc->curl_handle, CURLOPT_WRITEDATA, fc);


  /* Make it follow Location: headers */
  curl_easy_setopt(fc->curl_handle, CURLOPT_FOLLOWLOCATION, 1);

#if FLICKCURL_DEBUG > 2
  curl_easy_setopt(fc->curl_handle, CURLOPT_VERBOSE, (void*)1);
#endif

  curl_easy_setopt(fc->curl_handle, CURLOPT_ERRORBUFFER, fc->error_buffer);

  return fc;
}


void
flickcurl_free(flickcurl *fc)
{
  if(fc->xc) {
    if(fc->xc->myDoc) {
      xmlFreeDoc(fc->xc->myDoc);
      fc->xc->myDoc=NULL;
    }
    xmlFreeParserCtxt(fc->xc); 
  }

  if(fc->api_key)
    free(fc->api_key);
  if(fc->secret)
    free(fc->secret);
  if(fc->auth_token)
    free(fc->auth_token);
  if(fc->method)
    free(fc->method);

  /* only tidy up if we did all the work */
  if(fc->curl_init_here && fc->curl_handle) {
    curl_easy_cleanup(fc->curl_handle);
    fc->curl_handle=NULL;
  }

  if(fc->error_msg)
    free(fc->error_msg);

  if(fc->licenses) {
    int i;
    flickcurl_license *license;
    
    for(i=0; (license=fc->licenses[i]); i++) {
      free(license->name);
      if(license->url)
        free(license->url);
      free(license);
    }
    
    free(fc->licenses);
  }

  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
  }

  if(fc->param_fields) {
    int i;
    
    for(i=0; fc->param_fields[i]; i++) {
      free(fc->param_fields[i]);
      free(fc->param_values[i]);
    }
    free(fc->param_fields);
    free(fc->param_values);
    fc->param_fields=NULL;
    fc->param_values=NULL;
    fc->parameter_count=0;
  }
  if(fc->upload_field)
    free(fc->upload_field);
  if(fc->upload_value)
    free(fc->upload_value);

  free(fc);
}


void
flickcurl_init(void)
{
  curl_global_init(CURL_GLOBAL_ALL);
  xmlInitParser();
}


void
flickcurl_finish(void)
{
  xmlCleanupParser();
  curl_global_cleanup();
}


void
flickcurl_set_error_handler(flickcurl* fc, 
                            flickcurl_message_handler error_handler, 
                            void *error_data)
{
  fc->error_handler=error_handler;
  fc->error_data=error_data;
}


void
flickcurl_set_tag_handler(flickcurl* fc, 
                          flickcurl_tag_handler tag_handler, 
                          void *tag_data)
{
  fc->tag_handler=tag_handler;
  fc->tag_data=tag_data;
}


void
flickcurl_set_user_agent(flickcurl* fc, const char *user_agent)
{
  char *ua_copy=(char*)malloc(strlen(user_agent)+1);
  if(!ua_copy)
    return;
  strcpy(ua_copy, user_agent);
  
  fc->user_agent=ua_copy;
}


void
flickcurl_set_proxy(flickcurl* fc, const char *proxy)
{
  char *proxy_copy=(char*)malloc(strlen(proxy)+1);
  if(!proxy_copy)
    return;
  strcpy(proxy_copy, proxy);
  
  fc->proxy=proxy_copy;
}


void
flickcurl_set_http_accept(flickcurl* fc, const char *value)
{
  char *value_copy;
  size_t len=8; /* strlen("Accept:")+1 */
  
  if(value)
    len+=1+strlen(value); /* " "+value */
  
  value_copy=(char*)malloc(len);
  if(!value_copy)
    return;
  fc->http_accept=value_copy;

  strcpy(value_copy, "Accept:");
  value_copy+=7;
  if(value) {
    *value_copy++=' ';
    strcpy(value_copy, value);
  }

}


void
flickcurl_set_api_key(flickcurl* fc, const char *api_key)
{
#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "API Key: '%s'\n", api_key);
#endif
  if(fc->api_key)
    free(fc->api_key);
  fc->api_key=strdup(api_key);
}


const char*
flickcurl_get_api_key(flickcurl* fc)
{
  return fc->api_key;
}


void
flickcurl_set_shared_secret(flickcurl* fc, const char *secret)
{
#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "Secret: '%s'\n", secret);
#endif
  if(fc->secret)
    free(fc->secret);
  fc->secret=strdup(secret);
}


const char*
flickcurl_get_shared_secret(flickcurl* fc)
{
  return fc->secret;
}


void
flickcurl_set_auth_token(flickcurl *fc, const char* auth_token)
{
#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "Auth token: '%s'\n", auth_token);
#endif
  if(fc->auth_token)
    free(fc->auth_token);
  fc->auth_token=strdup(auth_token);
}


const char*
flickcurl_get_auth_token(flickcurl *fc)
{
  return fc->auth_token;
}


void
flickcurl_set_sign(flickcurl *fc)
{
  fc->sign=1;
}


void
flickcurl_set_request_delay(flickcurl *fc, long delay_msec)
{
  if(delay_msec >= 0)
    fc->request_delay=delay_msec;
}


static int
compare_args(const void *a, const void *b) 
{
  return strcmp(*(char**)a, *(char**)b);
}


static void
flickcurl_sort_args(flickcurl *fc, const char *parameters[][2], int count)
{
  qsort(parameters, count, sizeof(char*[2]), compare_args);
}


static int
flickcurl_prepare_common(flickcurl *fc, 
                         const char* url,
                         const char* method,
                         const char* upload_field, const char* upload_value,
                         const char* parameters[][2], int count,
                         int parameters_in_url)
{
  int i;
  char *md5_string=NULL;
  size_t* values_len=NULL;

  if(!url || !method || !parameters)
    return 1;
  
  /* If one is given, both are required */
  if((upload_field || upload_value) && (!upload_field || !upload_value))
    return 1;
  
  fc->failed=0;
  fc->error_code=0;
  if(fc->error_msg) {
    free(fc->error_msg);
    fc->error_msg=NULL;
  }
  /* Default to read */
  fc->is_write=0;
  /* Default to no data */
  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
    fc->data=NULL;
    fc->data_length=0;
    fc->data_is_xml=0;
  }
  if(fc->param_fields) {
    for(i=0; fc->param_fields[i]; i++) {
      free(fc->param_fields[i]);
      free(fc->param_values[i]);
    }
    free(fc->param_fields);
    free(fc->param_values);
    fc->param_fields=NULL;
    fc->param_values=NULL;
    fc->parameter_count=0;
  }
  if(fc->upload_field) {
    free(fc->upload_field);
    fc->upload_field=NULL;
  }
  if(fc->upload_value) {
    free(fc->upload_value);
    fc->upload_value=NULL;
  }
  
  if(!method) {
    flickcurl_error(fc, "No method to prepare");
    return 1;
  }
  if(!fc->secret) {
    flickcurl_error(fc, "No shared secret");
    return 1;
  }
  if(!fc->api_key) {
    flickcurl_error(fc, "No API key");
    return 1;
  }

  
  if(fc->method)
    free(fc->method);
  fc->method=strdup(method);
  
  parameters[count][0]  = "method";
  parameters[count++][1]= fc->method;

  parameters[count][0]  = "api_key";
  parameters[count++][1]= fc->api_key;

  if(fc->auth_token) {
    parameters[count][0]  = "auth_token";
    parameters[count++][1]= fc->auth_token;
  }

  parameters[count][0]  = NULL;

  /* +1 for api_sig */
  fc->param_fields=(char**)calloc(count+1, sizeof(char*));
  fc->param_values=(char**)calloc(count+1, sizeof(char*));
  values_len=(size_t*)calloc(count+1, sizeof(size_t));

  if(fc->auth_token || fc->sign)
    flickcurl_sort_args(fc, parameters, count);

  /* Save away the parameters and calculate the value lengths */
  for(i=0; parameters[i][0]; i++) {
    size_t param_len=strlen(parameters[i][0]);

    values_len[i]=strlen(parameters[i][1]);

    fc->param_fields[i]=(char*)malloc(param_len+1);
    strcpy(fc->param_fields[i], parameters[i][0]);
    fc->param_values[i]=(char*)malloc(values_len[i]+1);
    strcpy(fc->param_values[i], parameters[i][1]);
  }

  if(upload_field) {
    fc->upload_field=(char*)malloc(strlen(upload_field)+1);
    strcpy(fc->upload_field, upload_field);

    fc->upload_value=(char*)malloc(strlen(upload_value)+1);
    strcpy(fc->upload_value, upload_value);
  }

  if(fc->auth_token || fc->sign) {
    size_t buf_len=0;
    char *buf;
    
    buf_len=strlen(fc->secret);
    for(i=0; parameters[i][0]; i++)
      buf_len += strlen(parameters[i][0]) + values_len[i];

    buf=(char*)malloc(buf_len+1);
    strcpy(buf, fc->secret);
    for(i=0; parameters[i][0]; i++) {
      strcat(buf, parameters[i][0]);
      strcat(buf, parameters[i][1]);
    }
    
#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "MD5 Buffer '%s'\n", buf);
#endif
    md5_string=MD5_string(buf);
    
    parameters[count][0]  = "api_sig";
    parameters[count][1]= md5_string;
    values_len[count]=32; /* MD5 is always 32 */
    count++;
    
#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "Signature: '%s'\n", parameters[count-1][1]);
#endif
    
    free(buf);
    
    parameters[count][0] = NULL;
  }

  strcpy(fc->uri, url);

  if(parameters_in_url) {
    for(i=0; parameters[i][0]; i++) {
      char *value=(char*)parameters[i][1];
      char *escaped_value=NULL;

      if(!parameters[i][1])
        continue;

      strcat(fc->uri, parameters[i][0]);
      strcat(fc->uri, "=");
      if(!strcmp(parameters[i][0], "method")) {
        /* do not touch method name */
      } else
        escaped_value=curl_escape(value, values_len[i]);

      if(escaped_value) {
        strcat(fc->uri, escaped_value);
        curl_free(escaped_value);
      } else
        strcat(fc->uri, value);
      strcat(fc->uri, "&");
    }

    /* zap last & */
    fc->uri[strlen(fc->uri)-1]= '\0';
  }

#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "URI is '%s'\n", fc->uri);
#endif

  if(md5_string)
    free(md5_string);

  if(values_len)
    free(values_len);

  return 0;
}


int
flickcurl_prepare(flickcurl *fc, const char* method,
                  const char* parameters[][2], int count)
{
  return flickcurl_prepare_common(fc,
                                  "http://www.flickr.com/services/rest/?",
                                  method,
                                  NULL, NULL,
                                  parameters, count,
                                  1);
}


int
flickcurl_prepare_upload(flickcurl *fc, 
                         const char* url,
                         const char* method,
                         const char* upload_field, const char* upload_value,
                         const char* parameters[][2], int count)
{
  int rc;
  
  rc=flickcurl_prepare_common(fc,
                              url,
                              method,
                              upload_field, upload_value,
                              parameters, count,
                              0);
  if(!rc)
    flickcurl_set_write(fc, 1);
  return rc;
}


xmlDocPtr
flickcurl_invoke(flickcurl *fc)
{
  struct curl_slist *slist=NULL;
  xmlDocPtr doc=NULL;
  struct timeval now;
#if defined(OFFLINE) || defined(CAPTURE)
  char filename[200];
#endif

#if defined(OFFLINE) || defined(CAPTURE)
  if(1)
    sprintf(filename, "%s.xml", fc->method+7); /* skip "flickr." */
#endif

#ifdef OFFLINE
  if(1) {
    if(access(filename, R_OK)) {
      fprintf(stderr, "Method %s cannot run offline - no %s XML result available\n",
              fc->method, filename);
      return NULL;
    }
    sprintf(fc->uri, "file:%s", filename);
    fprintf(stderr, "Method %s: running offline using result from %s\n", 
            fc->method, filename);
  }
#endif

  if(!fc->uri) {
    flickcurl_error(fc, "No Flickr URI prepared to invoke");
    return NULL;
  }
  
  gettimeofday(&now, NULL);
#ifndef OFFLINE
  if(fc->last_request_time.tv_sec) {
    /* If there was a previous request, check it's not too soon to
     * do another
     */
    struct timeval uwait;

    memcpy(&uwait, &fc->last_request_time, sizeof(struct timeval));

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "Previous request was at %lu.N%lu\n",
            (unsigned long)uwait.tv_sec, (unsigned long)1000*uwait.tv_usec);
#endif

    /* Calculate in micro-seconds */
    uwait.tv_usec += 1000 * fc->request_delay;
    if(uwait.tv_usec >= 1000000) {
      uwait.tv_sec+= uwait.tv_usec / 1000000;
      uwait.tv_usec= uwait.tv_usec % 1000000;
    }

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "Next request is no earlier than %lu.N%lu\n",
            (unsigned long)uwait.tv_sec, (unsigned long)1000*uwait.tv_usec);
    fprintf(stderr, "Now is %lu.N%lu\n",
            (unsigned long)now.tv_sec, (unsigned long)1000*now.tv_usec);
#endif
    
    if(now.tv_sec > uwait.tv_sec ||
       (now.tv_sec == uwait.tv_sec && now.tv_usec > uwait.tv_usec)) {
      /* No need to delay */
    } else {
      struct timespec nwait;
      /* Calculate in nano-seconds */
      nwait.tv_sec= uwait.tv_sec - now.tv_sec;
      nwait.tv_nsec= 1000*(uwait.tv_usec - now.tv_usec);
      if(nwait.tv_nsec < 0) {
        nwait.tv_sec--;
        nwait.tv_nsec+= 1000000000;
      }
      
      /* Wait until timeval 'wait' happens */
#ifdef FLICKCURL_DEBUG
      fprintf(stderr, "Waiting for %lu sec N%lu nsec period\n",
              (unsigned long)nwait.tv_sec, (unsigned long)nwait.tv_nsec);
#endif
      while(1) {
        struct timespec rem;
        if(nanosleep(&nwait, &rem) < 0 && errno == EINTR) {
          memcpy(&nwait, &rem, sizeof(struct timeval));
#ifdef FLICKCURL_DEBUG
          fprintf(stderr, "EINTR - waiting for %lu sec N%lu nsec period\n",
                  (unsigned long)nwait.tv_sec, (unsigned long)nwait.tv_nsec);
#endif
          continue;
        }
        break;
      }
    }
  }
#endif
  memcpy(&fc->last_request_time, &now, sizeof(struct timeval));

#ifdef CAPTURE
  if(1) {
    fc->fh=fopen(filename, "wb");
    if(!fc->fh)
      flickcurl_error(fc, "Capture failed to write to %s - %s",
                      filename, strerror(errno));
  }
#endif

  if(fc->xc) {
    if(fc->xc->myDoc) {
      xmlFreeDoc(fc->xc->myDoc);
      fc->xc->myDoc=NULL;
    }
    xmlFreeParserCtxt(fc->xc); 
    fc->xc=NULL;
  }

  if(fc->proxy)
    curl_easy_setopt(fc->curl_handle, CURLOPT_PROXY, fc->proxy);

  if(fc->user_agent)
    curl_easy_setopt(fc->curl_handle, CURLOPT_USERAGENT, fc->user_agent);

  /* Insert HTTP Accept: header */
  if(fc->http_accept)
    slist=curl_slist_append(slist, (const char*)fc->http_accept);

  /* specify URL to call */
  curl_easy_setopt(fc->curl_handle, CURLOPT_URL, fc->uri);

  fc->total_bytes=0;

  if(fc->is_write)
    curl_easy_setopt(fc->curl_handle, CURLOPT_POST, 1); /* Set POST */
  else
    curl_easy_setopt(fc->curl_handle, CURLOPT_POST, 0);  /* Set GET */

  if(fc->data) {
    /* write is POST */
    curl_easy_setopt(fc->curl_handle, CURLOPT_POSTFIELDS, fc->data);
    curl_easy_setopt(fc->curl_handle, CURLOPT_POSTFIELDSIZE, fc->data_length);
    /* Replace default POST content type 'application/x-www-form-urlencoded' */
    slist=curl_slist_append(slist, (const char*)"Content-Type: application/xml");
    /* curl_easy_setopt(fc->curl_handle, CURLOPT_CUSTOMREQUEST, fc->verb); */
  }

  if(slist)
    curl_easy_setopt(fc->curl_handle, CURLOPT_HTTPHEADER, slist);

  if(fc->upload_field) {
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;
    int i;
    
    /* Main parameters */
    for(i=0; fc->param_fields[i]; i++) {
      curl_formadd(&post, &last, CURLFORM_PTRNAME, fc->param_fields[i],
                   CURLFORM_PTRCONTENTS, fc->param_values[i],
                   CURLFORM_END);
    }
    
    /* Upload parameter */
    curl_formadd(&post, &last, CURLFORM_COPYNAME, fc->upload_field,
                 CURLFORM_FILE, fc->upload_value, CURLFORM_END);

    /* Set the form info */
    curl_easy_setopt(fc->curl_handle, CURLOPT_HTTPPOST, post);
  }
  

#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "Resolving URI '%s' with method %s\n", 
          fc->uri, (fc->is_write ? "POST" : "GET"));
#endif
  
  if(curl_easy_perform(fc->curl_handle)) {
    /* failed */
    fc->failed=1;
    flickcurl_error(fc, fc->error_buffer);
  } else {
    long lstatus;

#ifndef CURLINFO_RESPONSE_CODE
#define CURLINFO_RESPONSE_CODE CURLINFO_HTTP_CODE
#endif

    /* Requires pointer to a long */
    if(CURLE_OK == 
       curl_easy_getinfo(fc->curl_handle, CURLINFO_RESPONSE_CODE, &lstatus) )
      fc->status_code=lstatus;

  }

  if(slist)
    curl_slist_free_all(slist);

  if(!fc->failed) {
    xmlNodePtr xnp;
    xmlAttr* attr;
    int failed=0;
    
    xmlParseChunk(fc->xc, NULL, 0, 1);

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "Got %d bytes content from URI '%s'\n",
            fc->total_bytes, fc->uri);
#endif

    doc=fc->xc->myDoc;
    if(!doc) {
      flickcurl_error(fc, "Failed to create XML DOM for document");
      fc->failed=1;
      goto tidy;
    }

    xnp = xmlDocGetRootElement(doc);
    if(!xnp) {
      flickcurl_error(fc, "Failed to parse XML");
      fc->failed=1;
      goto tidy;
    }

    for(attr=xnp->properties; attr; attr=attr->next) {
      if(!strcmp((const char*)attr->name, "stat")) {
        const char *attr_value=(const char*)attr->children->content;
#ifdef FLICKCURL_DEBUG
        fprintf(stderr, "Request returned stat '%s'\n", attr_value);
#endif
        if(strcmp(attr_value, "ok"))
          failed=1;
        break;
      }
    }

    if(failed) {
      xmlNodePtr err=xnp->children->next;
      for(attr=err->properties; attr; attr=attr->next) {
        const char *attr_name=(const char*)attr->name;
        const char *attr_value=(const char*)attr->children->content;
        if(!strcmp(attr_name, "code"))
          fc->error_code=atoi(attr_value);
        else if(!strcmp(attr_name, "msg"))
          fc->error_msg=strdup(attr_value);
      }
      flickcurl_error(fc, "Method %s failed with error %d - %s", 
                      fc->method, fc->error_code, fc->error_msg);
      fc->failed=1;
    }
  }

  tidy:
  if(fc->failed)
    doc=NULL;
  
#ifdef CAPTURE
  if(1) {
    if(fc->fh)
      fclose(fc->fh);
  }
#endif

  /* reset special flags */
  fc->sign=0;
  
  return doc;
}


char*
flickcurl_unixtime_to_isotime(time_t unix_time)
{
  struct tm* structured_time;
#define ISO_DATE_FORMAT "%Y-%m-%dT%H:%M:%SZ"
#define ISO_DATE_LEN 20
  static char date_buffer[ISO_DATE_LEN + 1];
  size_t len;
  char *value=NULL;
  
  structured_time=gmtime(&unix_time);
  len=ISO_DATE_LEN;
  strftime(date_buffer, len+1, ISO_DATE_FORMAT, structured_time);
  
  value=(char*)malloc(len + 1);
  strncpy((char*)value, date_buffer, len+1);
  return value;
}


char*
flickcurl_xpath_eval(flickcurl *fc, xmlXPathContextPtr xpathCtx,
                     const xmlChar* xpathExpr) 
{
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  int i;
  char* value=NULL;
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed=1;
    goto tidy;
  }
    
  nodes=xpathObj->nodesetval;
  for(i=0; i < xmlXPathNodeSetGetLength(nodes); i++) {
    xmlNodePtr node=nodes->nodeTab[i];
    
    if(node->type != XML_ATTRIBUTE_NODE &&
       node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    if(node->children)
      value=strdup((char*)node->children->content);
    break;
  }

  tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);

  return value;
}


void
flickcurl_set_write(flickcurl *fc, int is_write)
{
  fc->is_write=is_write;
}


void
flickcurl_set_data(flickcurl *fc, void* data, size_t data_length)
{
  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
  }
  
  fc->data=data;
  fc->data_length=data_length;
  fc->data_is_xml=0;
}


void
flickcurl_set_xml_data(flickcurl *fc, xmlDocPtr doc)
{
  xmlChar* mem;
  int size;

  if(fc->data) {
    if(fc->data_is_xml)
      xmlFree(fc->data);
  }

  xmlDocDumpFormatMemory(doc, &mem, &size, 1); /* format 1 means indent */
  
  fc->data=mem;
  fc->data_length=(size_t)size;
  fc->data_is_xml=1;
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


char*
flickcurl_call_get_one_string_field(flickcurl* fc, 
                                    const char* key, const char* value,
                                    const char* method,
                                    const xmlChar* xpathExpr)
{
  const char * parameters[6][2];
  int count=0;
  char *result=NULL;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 

  if(!value)
    return NULL;
  
  parameters[count][0]  = key;
  parameters[count++][1]= value;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, method, parameters, count))
    goto tidy;

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx)
    result=flickcurl_xpath_eval(fc, xpathCtx, xpathExpr);
  
  xmlXPathFreeContext(xpathCtx);

  tidy:

  return result;
}
