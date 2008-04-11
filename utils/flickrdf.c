/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickrdf - Triples from Flickrs
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
 *
 * USAGE: flickrdf [OPTIONS] FLICKR-PHOTO-URI
 *
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
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

/* many places for getopt */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <flickcurl_getopt.h>
#endif

#include <flickcurl.h>

#ifdef HAVE_RAPTOR
#include <raptor.h>
#else
#define RAPTOR_IDENTIFIER_TYPE_LITERAL    0
#define RAPTOR_IDENTIFIER_TYPE_RESOURCE   1
#define RAPTOR_IDENTIFIER_TYPE_ANONYMOUS  2
#define raptor_identifier_type int
#endif


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static const char* program;

static int debug=0;



static const char*
my_basename(const char *name)
{
  char *p;
  if((p=strrchr(name, '/')))
    name=p+1;
  else if((p=strrchr(name, '\\')))
    name=p+1;

  return name;
}


static void
my_message_handler(void *user_data, const char *message)
{
  fprintf(stderr, "%s: ERROR: %s\n", program, message);
}


static void
my_set_config_var_handler(void* userdata, const char* key, const char* value)
{
  flickcurl *fc=(flickcurl *)userdata;
  
  if(!strcmp(key, "api_key"))
    flickcurl_set_api_key(fc, value);
  else if(!strcmp(key, "secret"))
    flickcurl_set_shared_secret(fc, value);
  else if(!strcmp(key, "auth_token"))
    flickcurl_set_auth_token(fc, value);
}


#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" short ", --" long "  " description
#define HELP_TEXT_LONG(long, description) "      --" long "  " description
#define HELP_ARG(short, long) "--" #long
#define HELP_PAD "\n                          "
#else
#define HELP_TEXT(short, long, description) "  -" short "  " description
#define HELP_TEXT_LONG(long, description)
#define HELP_ARG(short, long) "-" #short
#define HELP_PAD "\n      "
#endif


#define GETOPT_STRING "Dd:ho:v"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"debug",   1, 0, 'D'},
  {"delay",   1, 0, 'd'},
  {"help",    0, 0, 'h'},
  {"output",  1, 0, 'o'},
  {"version", 0, 0, 'v'},
  {NULL,      0, 0, 0}
};
#endif


#define DC_NS "http://purl.org/dc/elements/1.1/"
#define GEO_NS "http://www.w3.org/2003/01/geo/wgs84_pos#"
#define FOAF_NS "http://xmlns.com/foaf/0.1/#"
#define XSD_NS "http://www.w3.org/2001/XMLSchema#"
#define RDF_NS "http://www.w3.org/1999/02/22-rdf-syntax-ns#"

struct flickrdf_nspace_s
{
  char* prefix;
  char* uri;
  size_t prefix_len;
  size_t uri_len;
  int seen;
  struct flickrdf_nspace_s* next;
};
typedef struct flickrdf_nspace_s flickrdf_nspace;

flickrdf_nspace namespace_table[]={
  { (char*)"a",        (char*)"http://www.w3.org/2000/10/annotation-ns" },
  { (char*)"acl",      (char*)"http://www.w3.org/2001/02/acls#" },
  { (char*)"blue",     (char*)"http://machinetags.org/wiki/Blue#", },
  { (char*)"cell",     (char*)"http://machinetags.org/wiki/Cell#", },
  { (char*)"dc",       (char*)DC_NS },
  { (char*)"exif",     (char*)"http://nwalsh.com/rdf/exif#" },
  { (char*)"exifi",    (char*)"http://nwalsh.com/rdf/exif-intrinsic#" },
  { (char*)"flickr",   (char*)"http://machinetags.org/wiki/Flickr#" },
  { (char*)"filtr",    (char*)"http://machinetags.org/wiki/Filtr#", },
  { (char*)"foaf",     (char*)FOAF_NS },
  { (char*)"geo",      (char*)GEO_NS, },
  { (char*)"i",        (char*)"http://www.w3.org/2004/02/image-regions#" },
  { (char*)"ph"      , (char*)"http://machinetags.org/wiki/Ph#" },
  { (char*)"rdf",      (char*)RDF_NS },
  { (char*)"rdfs",     (char*)"http://www.w3.org/2000/01/rdf-schema#" },
  { (char*)"skos",     (char*)"http://www.w3.org/2004/02/skos/core" },
  { (char*)"upcoming", (char*)"http://machinetags.org/wiki/Upcoming#" },
  { (char*)"xsd",      (char*)XSD_NS, },
  { NULL, NULL }
};

#define FIELD_FLAGS_PERSON 1
#define FIELD_FLAGS_STRING 2

static struct {
  flickcurl_photo_field_type field;
  const char* nspace_uri;
  const char* name;
  int flags;
} field_table[]={
  /* dc:available -- date that the resource will become/did become available.*/
  /* dc:dateSubmitted - Date of submission of resource (e.g. thesis, articles)*/
  { PHOTO_FIELD_dateuploaded,       DC_NS,   "dateSubmitted" },
  { PHOTO_FIELD_license,            DC_NS,   "rights" },
  /* dc:modified - date on which the resource was changed. */
  { PHOTO_FIELD_dates_lastupdate,   DC_NS,   "modified" },
  /* dc:issued - date of formal issuance (e.g. publication of the resource */
  { PHOTO_FIELD_dates_posted,       DC_NS,   "issued" },
  /* dc:created - date of creation of the resource */
  { PHOTO_FIELD_dates_taken,        DC_NS,   "created" },
  { PHOTO_FIELD_description,        DC_NS,   "description" },
  { PHOTO_FIELD_location_latitude,  GEO_NS,  "lat", FIELD_FLAGS_STRING },
  { PHOTO_FIELD_location_longitude, GEO_NS,  "long", FIELD_FLAGS_STRING },
  { PHOTO_FIELD_owner_realname,     FOAF_NS, "name", FIELD_FLAGS_PERSON },
  { PHOTO_FIELD_owner_username,     FOAF_NS, "nick", FIELD_FLAGS_PERSON },
  { PHOTO_FIELD_title,              DC_NS,   "title" },
  { PHOTO_FIELD_none, NULL, NULL }
};


static void
flickrdf_init(void)
{
  int i;

  for(i=0; namespace_table[i].prefix != NULL; i++) {
    namespace_table[i].uri_len=strlen(namespace_table[i].uri);
    namespace_table[i].prefix_len=strlen(namespace_table[i].prefix);
  }
}


struct flickrdf_context_s
{
  void *data;
  void (*emit_start)(struct flickrdf_context_s* frc, const char* base_uri_string, FILE* handle);
  void (*emit_namespace)(struct flickrdf_context_s* frc, flickrdf_nspace* ns);
  void (*emit_triple)(struct flickrdf_context_s* frc,
                      const char* subject, raptor_identifier_type subject_type,
                      const char* predicate_nspace, const char* predicate_name,
                      const char *object, raptor_identifier_type object_type,
                      const char *datatype_uri);
  void (*emit_finish)(struct flickrdf_context_s* frc);
};

typedef struct flickrdf_context_s flickrdf_context;

#ifndef HAVE_RAPTOR
typedef char raptor_uri;
typedef struct {
  const void *subject;
  raptor_identifier_type subject_type;
  const void *predicate;
  raptor_identifier_type predicate_type;
  const void *object;
  raptor_identifier_type object_type;
  raptor_uri *object_literal_datatype;
  const unsigned char *object_literal_language;
} raptor_statement;
typedef struct {
  FILE* fh;
  int output_turtle;
} raptor_serializer;


static void raptor_init(void) {}
static void raptor_finish(void) {}

static
raptor_uri* raptor_new_uri(const unsigned char* uri)
{
  size_t len;
  raptor_uri* u;

  len=strlen((const char*)uri);
  u=(raptor_uri*)malloc(len+1);
  strncpy((char*)u, (const char*)uri, len+1);
  return u;
}

static
raptor_uri* raptor_new_uri_from_uri_local_name(raptor_uri* u,
                                               const unsigned char* name)
{
  size_t len1;
  size_t len2;
  raptor_uri* newu;
  
  len1=strlen((const char*)u);
  len2=strlen((const char*)name);

  newu=(raptor_uri*)malloc(len1+len2+1);
  strncpy((char*)newu, (const char*)u, len1);
  strncpy((char*)(newu+len1), (const char*)name, len2+1);

  return newu;
}


static
void raptor_free_uri(raptor_uri* u)
{
  free(u);
}

#define NSERIALIZERS 2
static struct 
{
  const char *name;
  const char *label;
} serializers[NSERIALIZERS]= {
  { "ntriples",  "N-Triples" },
  { "turtle",    "Turtle" }
};

static int
raptor_serializer_syntax_name_check(const char* name)
{
  int i;
  
  for(i=0; i < NSERIALIZERS; i++) {
    if(strcmp(serializers[i].name, name))
       return 1;
  }
  return 0;
}

static int
raptor_serializers_enumerate(const unsigned int counter,
                             const char **name_p, const char **label_p,
                             const char **mime_type,
                             const unsigned char **uri_string)
{
  if(counter > (NSERIALIZERS-1))
    return 1;
  if(name_p)
    *name_p=serializers[counter].name;
  if(label_p)
    *label_p=serializers[counter].label;
  return 0;
}


static raptor_serializer*
raptor_new_serializer(const char* serializer_name)
{
  raptor_serializer* s;
  s=(raptor_serializer*)calloc(sizeof(raptor_serializer), 1);
  s->output_turtle=!strcmp((const char*)serializer_name, "turtle");
  return s;
}

static void
raptor_free_serializer(raptor_serializer* s)
{
  free(s);
}

static void
raptor_serialize_set_namespace(raptor_serializer* serializer,
                               raptor_uri* uri, const unsigned char* prefix)
{
  FILE* fh=serializer->fh;
  if(serializer->output_turtle)
    fprintf(fh, "@prefix %s: <%s> .\n", (const char*)prefix, (const char*)uri);
}


static void
raptor_serialize_start_to_file_handle(raptor_serializer* serializer,
                                      raptor_uri* base_uri, FILE* fh)
{
  serializer->fh=fh;
  if(base_uri)
    fprintf(fh, "@base <%s>\n", (char*)base_uri);
}

/* subject/object type: 0 literal, 1 uri, 2 blank */
static void
raptor_serialize_statement(raptor_serializer* serializer,
                           raptor_statement* s)
{
  FILE *fh=serializer->fh;

  if(s->subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    fprintf(fh, "<%s>", (const char*)s->subject);
  else /* blank node */
    fprintf(fh, "_:%s", (const char*)s->subject);

  fprintf(fh, " <%s> ", (const char*)s->predicate);

  if(s->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    const char *p;
    char c;
    
    fputc('"', fh);
    for(p=(const char*)s->object; (c=*p); p++) {
      if(c == '\n') {
        fputs("\\\n", fh);
        continue;
      } else if(c == '\t') {
        fputs("\\\t", fh);
        continue;
      } if(c == '\r') {
        fputs("\\\r", fh);
        continue;
      } else if(c == '"' || c == '\\')
        fputc('\\', fh);
      fputc(c, fh);
    }
    fputc('"', fh);
    if(s->object_literal_datatype)  {
      fputs("^^<", fh);
      fputs((const char*)s->object_literal_datatype, fh);
      fputc('>', fh);
    }
  } else if(s->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    fprintf(fh, "<%s>", (const char*)s->object);
  else /* blank node */
    fprintf(fh, "_:%s", (const char*)s->object);
  
  fputs(" . \n", fh);
}

static void
raptor_serialize_end(raptor_serializer* serializer)
{
  fflush(serializer->fh);
}

#endif


static void
ser_emit_namespace(flickrdf_context* frc, flickrdf_nspace* ns)
{
  raptor_serializer* serializer=(raptor_serializer*)frc->data;
  raptor_uri *ns_uri=NULL;

  ns_uri=raptor_new_uri((const unsigned char*)ns->uri);
  raptor_serialize_set_namespace(serializer, ns_uri, 
                                 (const unsigned char*)ns->prefix);
  raptor_free_uri(ns_uri);
}

/* subject/object type: 0 literal, 1 uri, 2 blank */
static void
ser_emit_triple(flickrdf_context* frc,
                const char* subject, raptor_identifier_type subject_type,
                const char* predicate_nspace, const char* predicate_name,
                const char *object, raptor_identifier_type object_type,
                const char *datatype_uri)
{
  raptor_serializer* serializer=(raptor_serializer*)frc->data;
  raptor_statement s;
  raptor_uri* predicate_ns_uri;
  
  s.subject_type=subject_type;
  if(s.subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    s.subject=(void*)raptor_new_uri((const unsigned char*)subject);
  else /* blank node */
    s.subject=(void*)subject;

  predicate_ns_uri=raptor_new_uri((const unsigned char*)predicate_nspace);
  s.predicate=(void*)raptor_new_uri_from_uri_local_name(predicate_ns_uri,
                                                        (const unsigned char*)predicate_name);
  raptor_free_uri(predicate_ns_uri);
  s.predicate_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;

  s.object_type=object_type;
  if(s.object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    s.object=(void*)raptor_new_uri((const unsigned char*)object);
  else /* literal or blank node */
    s.object=(void*)object;
  if(datatype_uri)
    s.object_literal_datatype=raptor_new_uri((const unsigned char*)datatype_uri);
  else
    s.object_literal_datatype=NULL;
  s.object_literal_language=NULL;

  raptor_serialize_statement(serializer, &s);

  if(s.subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    raptor_free_uri((raptor_uri*)s.subject);
  raptor_free_uri((raptor_uri*)s.predicate);
  if(s.object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    raptor_free_uri((raptor_uri*)s.object);
  if(datatype_uri)
    raptor_free_uri(s.object_literal_datatype);
}

static void
ser_emit_finish(flickrdf_context* frc)
{
  raptor_serializer* serializer=(raptor_serializer*)frc->data;
  raptor_serialize_end(serializer);
}



static flickrdf_nspace*
nspace_add_new(flickrdf_nspace* list, char* prefix, char *uri)
{
  flickrdf_nspace* ns;

  ns=(flickrdf_nspace*)malloc(sizeof(flickrdf_nspace));
  ns->prefix_len=strlen(prefix);
  ns->uri_len=strlen(uri);

  ns->prefix=(char*)malloc(ns->prefix_len+1);
  strcpy(ns->prefix, prefix);

  ns->uri=(char*)malloc(ns->uri_len+1);
  strcpy(ns->uri, uri);

  ns->next=list;
  return ns;
}


static flickrdf_nspace*
nspace_add_if_not_declared(flickrdf_nspace* list, 
                           const char* prefix, const char* nspace_uri)
{
  int n;
  flickrdf_nspace* ns;
  size_t prefix_len=prefix ? strlen(prefix) : 0;
  size_t uri_len=nspace_uri ? strlen(nspace_uri) : 0;
  
  for(ns=list; ns; ns=ns->next) {
    if(nspace_uri && ns->uri_len == uri_len && !strcmp(ns->uri, nspace_uri))
      break;
    if(prefix && ns->prefix_len == prefix_len && !strcmp(ns->prefix, prefix))
      break;
  }
  if(ns)
    return list;

  ns=NULL;
  for(n=0; namespace_table[n].uri; n++) {
    if(prefix && namespace_table[n].prefix_len == prefix_len && 
       !strcmp(namespace_table[n].prefix, prefix)) {
      ns=&namespace_table[n];
      break;
    }
    if(nspace_uri && namespace_table[n].uri_len == uri_len && 
       !strcmp(namespace_table[n].uri, nspace_uri)) {
      ns=&namespace_table[n];
      break;
    }
  }
  if(!ns)
    return list;

  /* ns was not found, copy it and add it to the list */
  return nspace_add_new(list, ns->prefix, ns->uri);
}


static flickrdf_nspace*
nspace_get_by_prefix(flickrdf_nspace* list, const char *prefix)
{
  flickrdf_nspace* ns;
  size_t prefix_len=strlen(prefix);
  
  for(ns=list; ns; ns=ns->next) {
    if(ns->prefix_len == prefix_len && !strcmp(ns->prefix, prefix))
      break;
  }
  return ns;
}


static void
print_nspaces(flickrdf_nspace* list)
{
  flickrdf_nspace* ns;

  for(ns=list; ns; ns=ns->next) {
    fprintf(stderr, "%s: Declaring namespace prefix %s URI %s\n",
            program, (ns->prefix ? ns->prefix : ":"),
            (ns->uri ? ns->uri : "\"\""));

  }
}
    

static void
free_nspaces(flickrdf_nspace* list)
{
  flickrdf_nspace* next;

  for(; list; list=next) {
    next=list->next;
    if(list->prefix)
      free(list->prefix);
    free(list->uri);
    free(list);
  }
}
    

static int
flickrdf(flickrdf_context* frc, flickcurl* fc, const char* photo_id)
{
  flickcurl_photo* photo;
  int i;
  int need_person=0;
  flickrdf_nspace* nspaces=NULL;
  flickrdf_nspace* ns;

  photo=flickcurl_photos_getInfo(fc, photo_id);

  if(!photo)
    return 1;
  
  if(debug)
    fprintf(stderr, "%s: Photo with URI %s ID %s has %d tags\n",
            program, photo->uri, photo->id, photo->tags_count);

  /* Always add XSD */
  nspaces=nspace_add_if_not_declared(nspaces, NULL, XSD_NS);

  /* mark namespaces used in fields */
  for(i=PHOTO_FIELD_FIRST; i <= PHOTO_FIELD_LAST; i++) {
    flickcurl_photo_field_type field=(flickcurl_photo_field_type)i;
    flickcurl_field_value_type datatype=photo->fields[field].type;
    int f;

    if(datatype == VALUE_TYPE_NONE)
      continue;

    for(f=0; field_table[f].field != PHOTO_FIELD_none; f++) {
      if(field_table[f].field != field) 
        continue;

      if(field_table[f].flags & FIELD_FLAGS_PERSON)
        need_person=1;

      nspaces=nspace_add_if_not_declared(nspaces, NULL, field_table[f].nspace_uri);
      break;
    }

  }
  

  /* in tags look for xmlns:PREFIX="URI" otherwise look for PREFIX: */
  for(i=0; i < photo->tags_count; i++) {
    char* prefix;
    char *p;
    flickcurl_tag* tag=photo->tags[i];

    if(!strncmp(tag->raw, "xmlns:", 6)) {
      prefix=&tag->raw[6];
      for(p=prefix; *p && *p != '='; p++)
        ;
      if(!*p) /* "xmlns:PREFIX" seen */
        continue;

      /* "xmlns:PREFIX=" seen */
      *p='\0';
      nspaces=nspace_add_new(nspaces, prefix, p+1);
      if(debug)
        fprintf(stderr,
                "%s: Found declaration of namespace prefix %s uri %s in tag '%s'\n",
                program, prefix, p+1, tag->raw);
      *p='=';
      continue;
    }

    prefix=tag->raw;
    for(p=prefix; *p && *p != ':'; p++)
      ;
    if(!*p) /* "PREFIX:" seen */
      continue;

    *p='\0';
    nspaces=nspace_add_if_not_declared(nspaces, prefix, NULL);
    *p=':';
  }


  if(debug)
    print_nspaces(nspaces);

  /* generate seen namespace declarations */
  for(ns=nspaces; ns; ns=ns->next)
    frc->emit_namespace(frc, ns);
  
  if(need_person) {
    frc->emit_triple(frc, photo->uri, RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                     DC_NS, "creator",
                     "person", RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                     NULL);
    frc->emit_triple(frc, "person", RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                     RDF_NS, "type",
                     FOAF_NS "Person", RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                     NULL);
    frc->emit_triple(frc, "person", RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                     FOAF_NS, "maker",
                     photo->uri, RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                     NULL);
  }
  

  /* generate triples from fields */
  for(i=PHOTO_FIELD_FIRST; i <= PHOTO_FIELD_LAST; i++) {
    flickcurl_photo_field_type field=(flickcurl_photo_field_type)i;
    flickcurl_field_value_type datatype=photo->fields[field].type;
    int f;

    if(datatype == VALUE_TYPE_NONE)
      continue;

    for(f=0; field_table[f].field != PHOTO_FIELD_none; f++) {
      const char* datatype_uri=NULL;
      char* object=NULL;
      raptor_identifier_type type= RAPTOR_IDENTIFIER_TYPE_LITERAL;
      
      if(field_table[f].field != field) 
        continue;

      if(debug)
        fprintf(stderr,
                "%s: field %s (%d) with %s value: '%s' has predicate %s%s\n", 
                program,
                flickcurl_get_photo_field_label(field), field,
                flickcurl_get_field_value_type_label(datatype),
                photo->fields[field].string,
                field_table[f].nspace_uri, field_table[f].name);

      object=photo->fields[field].string;

      if(field_table[f].flags & FIELD_FLAGS_STRING)
        datatype=VALUE_TYPE_STRING;

      if(field == PHOTO_FIELD_license) {
        flickcurl_license* license;
        license=flickcurl_photos_licenses_getInfo_by_id(fc, 
                                                        photo->fields[field].integer);
        if(!license)
          continue;

        if(license->url) {
          datatype=VALUE_TYPE_URI;
          object=license->url;
        } else {
          datatype=VALUE_TYPE_STRING;
          object=license->name;
        }
      }
      
      switch(datatype) {
        case VALUE_TYPE_BOOLEAN:
          datatype_uri= XSD_NS "boolean";
          break;

        case VALUE_TYPE_DATETIME:
          datatype_uri= XSD_NS "dateTime";
          break;
          
        case VALUE_TYPE_FLOAT:
          datatype_uri= XSD_NS "double";
          break;
          
        case VALUE_TYPE_INTEGER:
          datatype_uri= XSD_NS "integer";
          break;
          
        case VALUE_TYPE_STRING:
          break;
          
        case VALUE_TYPE_URI:
          type= RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          break;
          
        case VALUE_TYPE_NONE:
        case VALUE_TYPE_PHOTO_ID:
        case VALUE_TYPE_PHOTO_URI:
        case VALUE_TYPE_UNIXTIME:
        case VALUE_TYPE_PERSON_ID:
        case VALUE_TYPE_MEDIA_TYPE:
        default:
          break;
      }

      if(field_table[f].flags & FIELD_FLAGS_PERSON)
        frc->emit_triple(frc, "person", RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                         field_table[f].nspace_uri, field_table[f].name,
                         object, type,
                         datatype_uri);
      else
        frc->emit_triple(frc, photo->uri, RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                         field_table[f].nspace_uri, field_table[f].name,
                         object, type,
                         datatype_uri);
      break;
    }
  }
  

  /* generate triples from tags */
  for(i=0; i < photo->tags_count; i++) {
    flickcurl_tag* tag=photo->tags[i];
    char* prefix;
    char *p;
    char *f;
    char *v;
    size_t value_len;
    
    prefix=&tag->raw[0];

    if(!strncmp(prefix, "xmlns:", 6))
      continue;
    
    for(p=prefix; *p && *p != ':'; p++)
      ;
    /* ":" seen */
    *p='\0';

    f=p+1;
    
    for(v=f; *v && *v != '='; v++)
      ;
    if(!*v) /* "prefix:name" seen with no value */
      continue;
    /* zap = */
    *v++='\0';

    value_len=strlen(v);
    if(*v == '"') {
      v++;
      if(v[value_len-1]=='"')
        v[--value_len]='\0';
    }
        
    ns=nspace_get_by_prefix(nspaces, prefix);

    if(debug)
      fprintf(stderr,
              "%s: prefix '%s' field '%s' value '%s' namespace uri %s\n",
              program, p, f, v, 
              ns->uri);
    
    frc->emit_triple(frc, photo->uri, RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                     ns->uri, f,
                     v, RAPTOR_IDENTIFIER_TYPE_LITERAL, 
                     NULL);
    
  }
  
  if(nspaces)
    free_nspaces(nspaces);
  
  flickcurl_free_photo(photo);

  frc->emit_finish(frc);
  
  return 0;
}


static const char *title_format_string="Flickrdf - triples from flickrs %s\n";

static const char* config_filename=".flickcurl.conf";
static const char* config_section="flickr";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc=NULL;
  int rc=0;
  int usage=0;
  int help=0;
  const char* home;
  char config_path[1024];
  char* photo_id=NULL;
  const char* prefix_uri="http://www.flickr.com/photos/";
  size_t prefix_uri_len=strlen(prefix_uri);
  const char *serializer_syntax_name="ntriples";
  raptor_uri* base_uri=NULL;
  raptor_serializer* serializer=NULL;
  int request_delay= -1;
  flickrdf_context frc;

  program=my_basename(argv[0]);

  flickcurl_init();

  flickrdf_init();

  raptor_init();

  home=getenv("HOME");
  if(home)
    sprintf(config_path, "%s/%s", home, config_filename);
  else
    strcpy(config_path, config_filename);
  

  while (!usage && !help)
  {
    int c;
    
#ifdef HAVE_GETOPT_LONG
    int option_index = 0;

    c = getopt_long (argc, argv, GETOPT_STRING, long_options, &option_index);
#else
    c = getopt (argc, argv, GETOPT_STRING);
#endif
    if (c == -1)
      break;

    switch (c) {
      case 0:
      case '?': /* getopt() - unknown option */
        usage=1;
        break;

      case 'd':
        if(optarg)
          request_delay=atoi(optarg);
        break;
        
      case 'D':
        debug=1;
        break;
        
      case 'h':
        help=1;
        break;

      case 'o':
        if(optarg) {
          if(raptor_serializer_syntax_name_check(optarg))
            serializer_syntax_name=optarg;
          else {
            int i;
            
            fprintf(stderr,
                    "%s: invalid argument `%s' for `" HELP_ARG(o, output) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n");
            for(i=0; 1; i++) {
              const char *help_name;
              const char *help_label;
              if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
                break;
              printf("  %-12s for %s\n", help_name, help_label);
            }
            usage=1;
            break;
            
          }
        }
        break;
        
      case 'v':
        fputs(flickcurl_version_string, stdout);
        fputc('\n', stdout);

        exit(0);
    }
    
  }

  argv+=optind;
  argc-=optind;
  
  if(!help && argc < 1)
    usage=2; /* Title and usage */

  if(!help && !argc) {
    fprintf(stderr, "%s: No photo URI given\n", program);
    usage=1;
    goto usage;
  }

  if(usage || help)
    goto usage;

  photo_id=argv[0];
  if(strncmp(photo_id, prefix_uri, prefix_uri_len))
    usage=1;
  else {
    size_t len;

    photo_id+=prefix_uri_len;
    len=strlen(photo_id);
    if(!len)
      usage=1;
    else {
      if(photo_id[len-1] == '/')
        photo_id[--len]='\0';
      
      while(*photo_id && *photo_id != '/')
        photo_id++;
      if(!*photo_id)
        usage=1;
      else
        photo_id++;
    }
  }

  if(usage) {
    fprintf(stderr,
            "%s: Argument is not a Flickr photo URI like\n"
            "  http://www.flickr.com/photos/USER/PHOTO/\n", 
            program);
    goto usage;
  }


  serializer=raptor_new_serializer(serializer_syntax_name);
  if(!serializer) {
    fprintf(stderr, 
            "%s: Failed to create raptor serializer type %s\n", program,
            serializer_syntax_name);
    return(1);
  }

  /* base_uri=raptor_new_uri((const unsigned char*)argv[0]); */

  raptor_serialize_start_to_file_handle(serializer, base_uri, stdout);


  /* Initialise the Flickcurl library */
  fc=flickcurl_new();
  if(!fc) {
    rc=1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(!access((const char*)config_path, R_OK)) {
    if(read_ini_config(config_path, config_section, fc,
                       my_set_config_var_handler)) {
      fprintf(stderr, "%s: Failed to read config filename %s: %s\n",
              program, config_path, strerror(errno));
      rc=1;
      goto tidy;
    }
  }
  
 usage:
  if(usage) {
    if(usage>1) {
      fprintf(stderr, title_format_string, flickcurl_version_string);
      fputs("Flickcurl home page: ", stderr);
      fputs(flickcurl_home_url_string, stderr);
      fputc('\n', stderr);
      fputs(flickcurl_copyright_string, stderr);
      fputs("\nLicense: ", stderr);
      fputs(flickcurl_license_string, stderr);
      fputs("\n\n", stderr);
    }
    fprintf(stderr, "Try `%s " HELP_ARG(h, help) "' for more information.\n",
            program);
    rc=1;
    goto tidy;
  }

  if(help) {
    int i;

    printf(title_format_string, flickcurl_version_string);
    puts("Get Triples from Flickr photos.");
    printf("Usage: %s [OPTIONS] FLICKR-PHOTO-URI\n\n", program);

    fputs(flickcurl_copyright_string, stdout);
    fputs("\nLicense: ", stdout);
    puts(flickcurl_license_string);
    fputs("Flickcurl home page: ", stdout);
    puts(flickcurl_home_url_string);

    fputs("\n", stdout);

    puts(HELP_TEXT("d", "delay DELAY     ", "Set delay between requests in milliseconds"));
    puts(HELP_TEXT("D", "debug           ", "Print lots of output"));
    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("o", "output FORMAT   ", "Set output format to one of:"));
    for(i=0; 1; i++) {
      const char *help_name;
      const char *help_label;
      if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
        break;
      if(!strcmp(help_name, serializer_syntax_name))
        printf("      %-15s %s (default)\n", help_name, help_label);
      else
        printf("      %-15s %s\n", help_name, help_label);
    }
#ifdef HAVE_RAPTOR
    printf("    via Raptor %s serializers\n", raptor_version_string);
#else
    puts("    via internal RDF serializer");
#endif
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

    rc=0;
    goto tidy;
  }


  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);

  memset(&frc, sizeof(flickrdf_context), '\0');
  
  /* Perform the API call */
  frc.data=serializer;
  frc.emit_namespace=ser_emit_namespace;
  frc.emit_triple=ser_emit_triple;
  frc.emit_finish=ser_emit_finish;

  rc=flickrdf(&frc, fc, photo_id);

 tidy:
  if(fc)
    flickcurl_free(fc);

  if(serializer)
    raptor_free_serializer(serializer);
  if(base_uri)
    raptor_free_uri(base_uri);
  
  raptor_finish();

  flickcurl_finish();

  return(rc);
}
