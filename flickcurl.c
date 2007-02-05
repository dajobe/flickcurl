/*
 * flickcurl - Invoke Flickr API via CURL
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

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>


#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


#include <flickcurl.h>


#undef OFFLINE


extern char* MD5_string(char *string);
extern char* my_vsnprintf(const char *message, va_list arguments);

/* Prepare Flickr API request - form URI */
static int flickcurl_prepare(flickcurl *fc, const char* method, const char* parameters[][2], int count);

/* Invoke Flickr API at URi prepared above and get back an XML document */
static xmlDocPtr flickcurl_invoke(flickcurl *fc);

/* Debugging only */
#ifdef OFFLINE
void flickcurl_debug_set_uri(flickcurl* fc, const char* uri);
#endif


struct flickcurl_s {
  int total_bytes;

  /* Something failed */
  int failed;
  /* Flickr API error code */
  int error_code;
  /* Flickr API error message */
  char* error_msg;
  
  int status_code;

  char uri[2048];

  CURL* curl_handle;
  char error_buffer[CURL_ERROR_SIZE];
  int curl_init_here;

  char* user_agent;

  /* proxy URL string or NULL for none */
  char* proxy;
  
  void* error_data;
  flickcurl_message_handler error_handler;

  char *http_accept;

  /* XML parser */
  xmlParserCtxtPtr xc;

  /* The next three fields need to be set before authenticated
   * operations can be done (in most cases).
   */

  /* Flickr shared secret - flickcurl_set_shared_secret() */
  char* secret;

  /* Flickr application/api key  - flickcurl_set_api_key() */
  char* api_key;

  /* Flickr authentication token - flickcurl_set_auth_token() */
  char* auth_token;


  /* signature parameter (shared) for authenticated calls ("api_sig"
   * usually) or  NULL for where it is not needed - flickcurl_set_sig_key()
   */
  char* sig_key;

  /* Flickr API method to invoke - set by flickr_prepare */
  char* method;

  flickcurl_tag_handler tag_handler;
  void* tag_data;
};



const char* const flickcurl_short_copyright_string = "Copyright 2007 David Beckett.";

const char* const flickcurl_copyright_string = "Copyright (C) 2007 David Beckett - http://purl.org/net/dajobe/";

const char* const flickcurl_license_string = "LGPL 2.1 or newer, GPL 2 or newer, Apache 2.0 or newer.\nSee http://flickcurl.sf.net/LICENSE.html for full terms.";

const char* const flickcurl_home_url_string = "http://flickcurl.sf.net/";
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

  
static void
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

  return len;
}


flickcurl*
flickcurl_new(void)
{
  flickcurl* fc;

  fc=(flickcurl*)calloc(1, sizeof(flickcurl));
  if(!fc)
    return NULL;
  
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


static xmlDocPtr
flickcurl_invoke(flickcurl *fc)
{
  struct curl_slist *slist=NULL;
  xmlDocPtr doc=NULL;

  if(!fc->uri) {
    flickcurl_error(fc, "No Flickr URI prepared to invoke");
    return NULL;
  }
  
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
    fprintf(stderr, "Got %d bytes for uri '%s'\n", fc->total_bytes, fc->uri);
#endif

    doc=fc->xc->myDoc;
    if(!doc) {
      flickcurl_error(fc, "Failed to create XML DOM for document");
      fc->failed=1;
    }

#ifdef FLICKCURL_DEBUG
    fprintf(stderr, "Got XML DOM for uri '%s'\n", fc->uri);
#endif

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

  return doc;
}


void
free_flickr_tag(flickr_tag *t)
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
free_flickr_photo(flickr_photo *photo)
{
  int i;
  for(i=0; i < photo->tags_count; i++)
    free_flickr_tag(photo->tags[i]);
  free(photo);
}


static char*
unixtime_to_isotime(time_t unix_time)
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


static char*
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


#ifdef OFFLINE
static void
flickcurl_debug_set_uri(flickcurl* fc, const char* uri)
{
  strcpy(fc->uri, uri);
}
#endif


/*
 **********************************************************************
 * Flickr API Calls
 **********************************************************************
 */

#if 0
static struct {
  const char* prefix;
  const char* namespace_uri;
} namespace_table[]={
  { "a", "http://www.w3.org/2000/10/annotation-ns" },
  { "acl", "http://www.w3.org/2001/02/acls#" },
  { "dc", "http://purl.org/dc/elements/1.1/" },
  { "dcterms", "http://purl.org/dc/terms/" },
  { "exif", "http://nwalsh.com/rdf/exif#" },
  { "exifi", "http://nwalsh.com/rdf/exif-intrinsic#" },
  { "flickr", "x-urn:flickr:" },
  { "foaf", "http://xmlns.com/foaf/0.1/#" },
  { "geo", "http://www.w3.org/2003/01/geo/wgs84_pos#" },
  { "i", "http://www.w3.org/2004/02/image-regions#" },
  { "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#" },
  { "rdfs", "http://www.w3.org/2000/01/rdf-schema#" },
  { "skos", "http://www.w3.org/2004/02/skos/core" }
};
#endif


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


/* Flickr auth.getFullToken - turn a frob into an auth_token */
char*
flickcurl_auth_getFullToken(flickcurl* fc, const char* frob)
{
  const char * parameters[10][2];
  int count=0;
  char *auth_token=NULL;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  
  parameters[count][0]   = "mini_token";
  parameters[count++][1] = (char*)frob;

  parameters[count][0]   = NULL;

  flickcurl_set_sig_key(fc, "api_sig");

  if(flickcurl_prepare(fc, "flickr.auth.getFullToken", parameters, count))
    goto tidy;

#ifdef OFFLINE
  flickcurl_debug_set_uri(fc, "file:auth.getFullToken.xml");
#endif

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;
  
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx) {
    auth_token=flickcurl_xpath_eval(fc, xpathCtx,
                                    (const xmlChar*)"/rsp/auth/token");
    xmlXPathFreeContext(xpathCtx);
  }

  tidy:

  return auth_token;
}


#define VALUE_TYPE_URI       0
#define VALUE_TYPE_STRING    1
#define VALUE_TYPE_UNIXTIME  2 /* convered to VALUE_TYPE_DATE_TIME */
#define VALUE_TYPE_DATE_TIME 3
#define VALUE_TYPE_PHOTO_ID   4 /* Photo's identifier */
#define VALUE_TYPE_PHOTO_URI  5 /* Photo's page URI */

static const char* match_type_label[]={
  "uri",
  "string",
  "unix time",
  "dateTime",
  "photo id",
  "photo URI"
};


static struct {
  const xmlChar* xpath;
  const char* predicate_uri;
  int type;
} match_table[]={
  /* ID */
  {
    (const xmlChar*)"/rsp/photo/@id",
    NULL,
    VALUE_TYPE_PHOTO_ID,
  }
,
  /* photopage */
  {
    (const xmlChar*)"/rsp/photo/urls/url[@type=\"photopage\"]",
    "flickr:photopage",
    VALUE_TYPE_PHOTO_URI,
  }
,
  /* Owner name */
  {
    (const xmlChar*)"/rsp/photo/owner/@realname",
    "dc:creator",
    VALUE_TYPE_STRING,
  }
,
  /* Title */
  {
    (const xmlChar*)"/rsp/photo/title",
    "dc:title",
    VALUE_TYPE_STRING,
  }
,
  /* Description */
  {
    (const xmlChar*)"/rsp/photo/description",
    "dc:description",
    VALUE_TYPE_STRING,
  }
,
  /* Posted date */
  {
    (const xmlChar*)"/rsp/photo/dates/@posted",
    "flickr:postedDate",
    VALUE_TYPE_UNIXTIME,
  }
,
  /* Photo taken date */
  {
    (const xmlChar*)"/rsp/photo/dates/@taken",
    "flickr:takenDate",
    VALUE_TYPE_DATE_TIME,
  }
,
  /* Last update date */
  {
    (const xmlChar*)"/rsp/photo/dates/@lastupdate",
    "flickr:lastUpdate",
    VALUE_TYPE_UNIXTIME,
  }
,
  /* location latitude */
  {
    (const xmlChar*)"/rsp/photo/location/@latitude",
    "geo:latitude",
    VALUE_TYPE_STRING,
  }
,
  /* location longitude */
  {
    (const xmlChar*)"/rsp/photo/location/@longitude",
    "geo:longitude",
    VALUE_TYPE_STRING,
  }
,
  { 
    NULL,
    NULL,
    0
  }
};


/* Get information about a photo */
flickr_photo*
flickcurl_photos_getInfo(flickcurl* fc, const char* photo_id)
{
  const char * parameters[10][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  int expri;
  xmlXPathObjectPtr xpathObj=NULL;
  xmlNodeSetPtr nodes;
  const xmlChar* xpathExpr=NULL;
  flickr_photo* photo=NULL;
  int i;
  
  if(!fc->auth_token) {
    flickcurl_error(fc, "No auth_token for method flickr.photos.getInfo");
    return NULL;
  }


  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = "token";
  parameters[count++][1]= fc->auth_token;

  parameters[count][0]  = NULL;


  flickcurl_set_sig_key(fc, "api_sig");

  if(flickcurl_prepare(fc, "flickr.photos.getInfo", parameters, count))
    goto tidy;

#ifdef OFFLINE
  flickcurl_debug_set_uri(fc, "file:photos_getInfo.xml");
#endif

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  photo=(flickr_photo*)calloc(sizeof(flickr_photo), 1);
  
  for(expri=0; match_table[expri].xpath; expri++) {
    char *value=flickcurl_xpath_eval(fc, xpathCtx, match_table[expri].xpath);
    int datatype=match_table[expri].type;
    
    if(!value)
      continue;

    
    if(VALUE_TYPE_PHOTO_ID == datatype) {
      photo->id=strdup(value);
      continue;
    }

    if(VALUE_TYPE_PHOTO_URI == datatype) {
      photo->uri=strdup(value);
      continue;
    }

    if(datatype == VALUE_TYPE_UNIXTIME || datatype == VALUE_TYPE_DATE_TIME) {
      time_t unix_time;
      
      if(datatype == VALUE_TYPE_UNIXTIME)
        unix_time=atoi(value);
      else
        unix_time=curl_getdate((const char*)value, NULL);
      
      if(unix_time >= 0) {
        char* new_value=unixtime_to_isotime(unix_time);
#if FLICKCURL_DEBUG > 1
        fprintf(stderr, "  date from: '%s' unix time %ld to '%s'\n",
                value, (long)unix_time, new_value);
#endif
        free(value);
        value=new_value;
        datatype=VALUE_TYPE_DATE_TIME;
      } else
        /* failed to convert, make it a string */
        datatype=VALUE_TYPE_STRING;
    }

    fprintf(stderr, "predicate URI: %s with %s value: '%s'\n",
            match_table[expri].predicate_uri, 
            match_type_label[datatype],
            value);
      
    free(value);
    
    if(fc->failed)
      goto tidy;
  }


  /* Now do tags */
  xpathExpr=(const xmlChar*)"/rsp/photo/tags/tag";
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
    xmlAttr* attr;
    flickr_tag* t;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed=1;
      break;
    }
    
    t=(flickr_tag*)calloc(sizeof(flickr_tag), 1);
    t->photo=photo;
    
    for(attr=node->properties; attr; attr=attr->next) {
      const char *attr_name=(const char*)attr->name;
      char *attr_value;
      
      attr_value=(char*)malloc(strlen((const char*)attr->children->content)+1);
      strcpy(attr_value, (const char*)attr->children->content);
      
      if(!strcmp(attr_name, "id"))
        t->id=attr_value;
      else if(!strcmp(attr_name, "author"))
        t->author=attr_value;
      else if(!strcmp(attr_name, "raw"))
        t->raw=attr_value;
    }
    
    t->cooked=(char*)malloc(strlen((const char*)node->children->content)+1);
    strcpy(t->cooked, (const char*)node->children->content);
    
#if FLICKCURL_DEBUG > 1
    fprintf(stderr, "tag: id %s author %s raw '%s' cooked '%s'\n",
            t->id, t->author, t->raw, t->cooked);
#endif
    
    if(fc->tag_handler)
      fc->tag_handler(fc->tag_data, t);
    
    photo->tags[photo->tags_count++]=t;
  } /* for nodes */
  


 tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  
  if(fc->failed)
    doc=NULL;

  return photo;
}
