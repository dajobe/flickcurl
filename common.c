/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl.c - Flickcurl core functions for the Flickr API
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

  /* DEFAULT delay between requests is 100ms i.e 10 requests/second max */
  fc->request_delay=100;
  
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
  if(fc->sig_key)
    free(fc->sig_key);
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
  
  free(fc);
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
flickcurl_set_sig_key(flickcurl *fc, const char* sig_key)
{
  if(fc->sig_key)
    free(fc->sig_key);
  fc->sig_key=sig_key ? strdup(sig_key) : NULL;
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


int
flickcurl_prepare(flickcurl *fc, const char* method,
                  const char* parameters[][2], int count)
{
  int i;
  char *md5_string=NULL;

  fc->failed=0;
  fc->error_code=0;
  if(fc->error_msg) {
    free(fc->error_msg);
    fc->error_msg=NULL;
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

  parameters[count][0]  = NULL;

  if(fc->sig_key) {
    size_t buf_len=0;
    char *buf;
    
    flickcurl_sort_args(fc, parameters, count);

    buf_len=strlen(fc->secret);
    for(i=0; parameters[i][0]; i++) {
      buf_len += strlen(parameters[i][0]) + strlen(parameters[i][1]);
    }

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
    
    parameters[count][0]  = fc->sig_key;
    parameters[count++][1]= md5_string;

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "Signature: '%s'\n", parameters[count-1][1]);
#endif
    
    free(buf);
    
    parameters[count][0] = NULL;
  }

  strcpy(fc->uri, "http://www.flickr.com/services/rest/?");

  for(i=0; parameters[i][0]; i++) {
    if(!parameters[i][1])
      continue;
    
    strcat(fc->uri, parameters[i][0]);
    strcat(fc->uri, "=");
    strcat(fc->uri, parameters[i][1]);
    strcat(fc->uri, "&");
  }

  /* zap last & */
  fc->uri[strlen(fc->uri)-1]= '\0';
#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "URI is '%s'\n", fc->uri);
#endif

  if(md5_string)
    free(md5_string);

  return 0;
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

  /* Insert HTTP Accept: header only */
  if(fc->http_accept) {
    slist=curl_slist_append(slist, (const char*)fc->http_accept);
    curl_easy_setopt(fc->curl_handle, CURLOPT_HTTPHEADER, slist);
  }

  /* specify URL to get */
  curl_easy_setopt(fc->curl_handle, CURLOPT_URL, fc->uri);

  fc->total_bytes=0;

#ifdef FLICKCURL_DEBUG
  fprintf(stderr, "Retrieving URI '%s'\n", fc->uri);
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
    }

    xnp = xmlDocGetRootElement(doc);
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
      doc=NULL;
    }
  }

#ifdef CAPTURE
  if(1) {
    if(fc->fh)
      fclose(fc->fh);
    if(fc->failed)
      remove(filename);
  }
#endif

  return doc;
}


void
flickcurl_free_tag(flickcurl_tag *t)
{
  if(t->id)
    free(t->id);
  if(t->author)
    free(t->author);
  if(t->raw)
    free(t->raw);
  if(t->cooked)
    free(t->cooked);
  free(t);
}


void
flickcurl_free_photo(flickcurl_photo *photo)
{
  int i;
  for(i=0; i <= PHOTO_FIELD_LAST; i++) {
    if(photo->fields[i].string)
      free(photo->fields[i].string);
  }
  
  for(i=0; i < photo->tags_count; i++)
    flickcurl_free_tag(photo->tags[i]);

  if(photo->id)
    free(photo->id);
  
  if(photo->uri)
    free(photo->uri);
  
  free(photo);
}


void
flickcurl_free_person(flickcurl_person *person)
{
  int i;
  for(i=0; i <= PERSON_FIELD_LAST; i++) {
    if(person->fields[i].string)
      free(person->fields[i].string);
  }
  
  if(person->nsid)
    free(person->nsid);
  
  free(person);
}


void
flickcurl_free_context(flickcurl_context *context)
{
  if(context->id)
    free(context->id);
  if(context->secret)
    free(context->secret);
  if(context->title)
    free(context->title);
  if(context->url)
    free(context->url);
  if(context->thumb)
    free(context->thumb);
  free(context);
}


void
flickcurl_free_contexts(flickcurl_context** contexts)
{
  int i;
  for(i=0; contexts[i]; i++)
    flickcurl_free_context(contexts[i]);
  free(contexts);
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



/*
 * Get a photo's image source URIs
 * @c can be s,m,t,b for sizes, o for original, otherwise default
 * http://www.flickr.com/services/api/misc.urls.html
 */
char*
flickcurl_photo_as_source_uri(flickcurl_photo *photo, const char c)
{
  char buf[1024];
  char *result;
  size_t len;
  
  if(c == 'o') {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{o-secret}_o.(jpg|gif|png) */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s_o.%s", 
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_originalsecret].string,
            photo->fields[PHOTO_FIELD_originalformat].string);
  } else if (c == 'm' || c == 's' || c == 't' || c == 'b') {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}_[mstb].jpg */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s_%c.jpg",
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_secret].string,
            c);
  } else {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}.jpg */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s.jpg",
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_secret].string);
  }
  len=strlen(buf);
  result=(char*)malloc(len+1);
  strcpy(result, buf);
  return result;
}


flickcurl_context**
flickcurl_build_contexts(flickcurl* fc, xmlDocPtr doc)
{
  flickcurl_context** contexts=NULL;
  xmlNodePtr xnp;
  xmlNodePtr node;
  int i;
  int count=0;
  
  contexts=(flickcurl_context**)calloc(sizeof(flickcurl_context**), 3);

  /* walk children elements of root element */
  xnp = xmlDocGetRootElement(doc);
  for(i=0, node=xnp->children;
      node;
      i++, node=node->next) {
    xmlAttr* attr;
    flickcurl_context* context;
    flickcurl_context_type type=FLICKCURL_CONTEXT_NONE;
    int j;
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "XML node name %s XML type %d\n", node->name, node->type);
#endif
    
    if(node->type != XML_ELEMENT_NODE)
      continue;

    for(j=0; j <= FLICKCURL_CONTEXT_LAST; j++) {
      if(!strcmp((const char*)node->name, flickcurl_context_type_element[j])) {
        type=(flickcurl_context_type)j;
        break;
      }
    }
    if(type == FLICKCURL_CONTEXT_NONE)
      continue;
    
    context=(flickcurl_context*)calloc(sizeof(flickcurl_context), 1);
    context->type=type;

    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;
      
      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        context->id=attr_value;
      else if(!strcmp(attr_name, "secret"))
        context->secret=attr_value;
      else if(!strcmp(attr_name, "server"))
        context->server=atoi(attr_value);
      else if(!strcmp(attr_name, "farm"))
        context->farm=atoi(attr_value);
      else if(!strcmp(attr_name, "title"))
        context->title=attr_value;
      else if(!strcmp(attr_name, "url"))
        context->url=attr_value;
      else if(!strcmp(attr_name, "thumb"))
        context->thumb=attr_value;
    } /* for attributes */
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "context: id %s secret %s server %d farm %d title '%s'\n  url '%s'\n  thumb '%s'\n",
            context->id, 
            (context->secret ? context->secret : "NULL"),
            context->server, context->farm,
            (context->title ? context->title : "NULL"),
            context->url,
            context->thumb
            );
#endif
    
    contexts[count++]=context;
  } /* for nodes */

  contexts[count]=NULL;

  return contexts;
}
