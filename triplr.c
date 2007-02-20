/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * triplr - Triples from Flickrs
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
 *
 * USAGE: triplr [OPTIONS] FLICKR-PHOTO-URI
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
#endif


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static const char* program;

static int debug=0;

#ifdef HAVE_RAPTOR
static raptor_serializer* serializer=NULL;
#else
static int output_turtle=0;
#endif


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

static struct {
  const char* prefix;
  const char* nspace_uri;
  int active;
  int prefix_len;
  int nspace_uri_len;
} namespace_table[]={
  { "a",        "http://www.w3.org/2000/10/annotation-ns" },
  { "acl",      "http://www.w3.org/2001/02/acls#" },
  { "blue",     "x-urn:blue:#", 1 }, /* active */
  { "cell",     "http://www.machinetags.org/wiki/Cell#", 1 }, /* active */
  { "dc",       DC_NS },
  { "dcterms",  "http://purl.org/dc/terms/" },
  { "exif",     "http://nwalsh.com/rdf/exif#" },
  { "exifi",    "http://nwalsh.com/rdf/exif-intrinsic#" },
  { "flickr",   "x-urn:flickr:" },
  { "filtr",    "x-urn:filtr:", 1 }, /* active */
  { "foaf",     FOAF_NS },
  { "geo",      GEO_NS, 1 }, /* active */
  { "i",        "http://www.w3.org/2004/02/image-regions#" },
  { "rdf",      RDF_NS },
  { "rdfs",     "http://www.w3.org/2000/01/rdf-schema#" },
  { "skos",     "http://www.w3.org/2004/02/skos/core" },
  { "upcoming", "http://www.machinetags.org/wiki/Upcoming#" },
  { NULL, NULL }
};

#define FIELD_FLAGS_PERSON 1

static struct {
  flickcurl_photo_field field;
  const char* nspace_uri;
  const char* name;
  int flags;
} field_table[]={
  { PHOTO_FIELD_dateuploaded,       DC_NS   , "date" },
  { PHOTO_FIELD_license,            DC_NS   , "rights" },
  { PHOTO_FIELD_dates_lastupdate,   DC_NS   , "date" },
  { PHOTO_FIELD_dates_posted,       DC_NS   , "date" },
  { PHOTO_FIELD_dates_taken,        DC_NS   , "date" },
  { PHOTO_FIELD_description,        DC_NS   , "description" },
  { PHOTO_FIELD_location_latitude,  GEO_NS  , "lat" },
  { PHOTO_FIELD_location_longitude, GEO_NS  , "long" },
  { PHOTO_FIELD_owner_realname,     FOAF_NS , "name", FIELD_FLAGS_PERSON },
  { PHOTO_FIELD_owner_username,     FOAF_NS , "nick", FIELD_FLAGS_PERSON },
  { PHOTO_FIELD_title,              DC_NS   , "title" },
  { PHOTO_FIELD_none, NULL, NULL }
};


static void
triplr_init(void)
{
  int i;

  for(i=0; namespace_table[i].prefix != NULL; i++) {
    namespace_table[i].nspace_uri_len=strlen(namespace_table[i].nspace_uri);
    namespace_table[i].prefix_len=strlen(namespace_table[i].prefix);
  }
}


static void
emit_namespace(FILE* fh,
               const char* prefix, const char* uri)
{
#ifdef HAVE_RAPTOR
  raptor_uri *ns_uri=NULL;

  ns_uri=raptor_new_uri((const unsigned char*)uri);
  raptor_serialize_set_namespace(serializer, ns_uri, 
                                 (const unsigned char*)prefix);
  raptor_free_uri(ns_uri);
#else
  if(output_turtle)
    fprintf(fh, "@prefix %s: <%s> .\n", prefix, uri);
#endif
}


#ifdef HAVE_RAPTOR
#else
#define RAPTOR_IDENTIFIER_TYPE_LITERAL    0
#define RAPTOR_IDENTIFIER_TYPE_RESOURCE   1
#define RAPTOR_IDENTIFIER_TYPE_ANONYMOUS  2
#define raptor_identifier_type int
#endif

/* subject/object type: 0 literal, 1 uri, 2 blank */
static void
emit_triple(FILE* fh,
            const char* subject, raptor_identifier_type subject_type,
            const char* predicate_nspace, const char* predicate_name,
            const char *object, raptor_identifier_type object_type,
            const char *datatype_uri)
{
#ifdef HAVE_RAPTOR
  raptor_statement s;
  raptor_uri* predicate_ns_uri;
  
  s.subject_type=subject_type;
  if(s.subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    s.subject=(void*)raptor_new_uri((const unsigned char*)subject);
  else
    s.subject=(void*)subject;

  predicate_ns_uri=raptor_new_uri((const unsigned char*)predicate_nspace);
  s.predicate=(void*)raptor_new_uri_from_uri_local_name(predicate_ns_uri,
                                                        (const unsigned char*)predicate_name);
  raptor_free_uri(predicate_ns_uri);
  s.predicate_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;

  s.object_type=object_type;
  if(s.object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    s.object=(void*)raptor_new_uri((const unsigned char*)object);
  else
    s.object=(void*)object;
  if(datatype_uri)
    s.object_literal_datatype=raptor_new_uri((const unsigned char*)datatype_uri);
  else
    s.object_literal_datatype=NULL;
  s.object_literal_language=NULL;

  raptor_serialize_statement(serializer, &s);

  if(s.subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    raptor_new_uri(s.subject);
  raptor_new_uri(s.predicate);
  if(s.object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    raptor_free_uri((raptor_uri*)s.object);
  if(datatype_uri)
    raptor_free_uri(s.object_literal_datatype);
#else
  if(subject_type == 1)
    fprintf(fh, "<%s>", subject);
  else
    fprintf(fh, "_:%s", subject);

  fprintf(fh, " <%s%s> ", predicate_nspace, predicate_name);

  if(object_type == 0) {
    fprintf(fh, "\"%s\"", object);
    if(datatype_uri)  {
      fputs("^^<", fh);
      fputs(datatype_uri, fh);
      fputc('>', fh);
    }
  } else if(object_type == 1)
    fprintf(fh, "<%s>", object);
  else
    fprintf(fh, "_:%s", object);
  
  fputs(" . \n", fh);
#endif
}


static int
triplr(FILE* fh, flickcurl* fc, const char* photo_id)
{
  flickcurl_photo* photo;
  flickcurl_photo_field field;
  int i;
  int need_person=0;
  
  photo=flickcurl_photos_getInfo(fc, photo_id);

  if(!photo)
    return 1;
  
  if(debug)
    fprintf(stderr, "%s: Photo with URI %s ID %s has %d tags\n",
            program, photo->uri, photo->id, photo->tags_count);

  /* mark namespaces used in fields */
  for(field=0; field <= PHOTO_FIELD_LAST; field++) {
    flickcurl_field_value_type datatype=photo->fields[field].type;
    int f;

    if(datatype == VALUE_TYPE_NONE)
      continue;

    for(f=0; field_table[f].field != PHOTO_FIELD_none; f++) {
      int n;
      
      if(field_table[f].field != field) 
        continue;

      if(field_table[f].flags & FIELD_FLAGS_PERSON)
        need_person=1;

      for(n=0; namespace_table[n].prefix != NULL; n++) {
        if(strcmp(namespace_table[n].nspace_uri, field_table[f].nspace_uri))
          continue;
        namespace_table[n].active=1;
        break;
      }

      break;
    }

  }
  

  /* in tags, look for xmlns:PREFIX="URI" and mark namespaces used */
  for(i=0; i < photo->tags_count; i++) {
    int n;
    char* prefix;
    char *p;
    flickcurl_tag* tag=photo->tags[i];

    if(!tag->machine_tag)
      continue;
    
    if(strncmp(tag->raw, "xmlns:", 6))
      continue;

    prefix=&tag->raw[6];
    for(p=prefix; *p && *p != '='; p++)
      ;
    if(!*p) /* "xmlns:PREFIX" seen */
      continue;
    
    /* "xmlns:PREFIX=" seen */
    *p='\0';

    for(n=0; namespace_table[n].prefix != NULL; n++) {
      if(strcmp(namespace_table[n].prefix, prefix))
        continue;
      *p='=';
      if(debug)
        fprintf(stderr,
                "%s: Found declaration of namespace with prefix %s in tag '%s'\n",
              program, namespace_table[n].prefix, tag->raw);
      namespace_table[n].active=1;
      break;
    }
  }


  /* generate seen namespace declarations */
  for(i=0; namespace_table[i].prefix != NULL; i++)
    if(namespace_table[i].active)
      emit_namespace(fh, namespace_table[i].prefix, 
                     namespace_table[i].nspace_uri);
  
  if(need_person) {
    emit_triple(fh, photo->uri, RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                DC_NS, "creator",
                "person", RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                NULL);
    emit_triple(fh, "person", RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                RDF_NS, "type",
                FOAF_NS "Person", RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                NULL);
  }
  

  /* generate triples from fields */
  for(field=0; field <= PHOTO_FIELD_LAST; field++) {
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

      if(field == PHOTO_FIELD_license) {
        flickcurl_license* license;
        license=flickcurl_photos_licenses_getInfo_by_id(fc, 
                                                        photo->fields[field].integer);
        if(license) {
          datatype=VALUE_TYPE_URI;
          object=license->url;
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
        default:
          break;
      }

      if(field_table[f].flags & FIELD_FLAGS_PERSON)
        emit_triple(fh, "person", RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                    field_table[f].nspace_uri, field_table[f].name,
                    object, type,
                    datatype_uri);
      else
        emit_triple(fh, photo->uri, RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                    field_table[f].nspace_uri, field_table[f].name,
                    object, type,
                    datatype_uri);
      break;
    }
  }
  

  /* generate triples from tags */
  for(i=0; i < photo->tags_count; i++) {
    int n;
    flickcurl_tag* tag=photo->tags[i];
    size_t tag_len;

    if(!tag->machine_tag)
      continue;


    tag_len=strlen(tag->raw);
    for(n=0; namespace_table[n].prefix != NULL; n++) {
      char *p;
      char *f;
      char *v;
      int value_len;
      size_t len;
      
      if(!namespace_table[n].active)
        continue;

      len=namespace_table[n].prefix_len;
      p=tag->raw;
      if(tag_len < len+1 || p[len] != ':')
        continue;

      f=p+len;
      *f++='\0';
      
      if(strcmp(namespace_table[n].prefix, p))
        continue;

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
        
      if(debug)
        fprintf(stderr,
                "%s: prefix '%s' field '%s' value '%s' namespace uri %s\n",
                program, p, f, v, 
                namespace_table[n].nspace_uri);

      emit_triple(fh, 
                  photo->uri, RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                  namespace_table[n].nspace_uri, f,
                  v, RAPTOR_IDENTIFIER_TYPE_LITERAL, 
                  NULL);
      
      break;
    }
  }
  
  flickcurl_free_photo(photo);

  return 0;
}


static const char *title_format_string="Triplr - triples from flickrs %s\n";

static const char* config_filename=".flickcurl.conf";
static const char* config_section="flickr";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc;
  int rc=0;
  int usage=0;
  int help=0;
  const char* home;
  char config_path[1024];
  char* photo_id;
  const char* prefix_uri="http://www.flickr.com/photos/";
  size_t prefix_uri_len=strlen(prefix_uri);
#ifdef HAVE_RAPTOR
  const char *serializer_syntax_name="ntriples";
  raptor_uri* base_uri=NULL;
#endif
  int request_delay= -1;

  program=my_basename(argv[0]);

  triplr_init();
#ifdef HAVE_RAPTOR
  raptor_init();
#endif

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
#ifdef HAVE_RAPTOR
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
#else
          if(!strcmp(optarg, "ntriples"))
            output_turtle=0;
          else if(!strcmp(optarg, "turtle"))
            output_turtle=1;
          else {
            fprintf(stderr,
                    "%s: invalid argument `%s' for `" HELP_ARG(o, output) "'\nUse either 'ntriples' or 'turtle'\n",
                    program, optarg);
            usage=1;

          }
#endif
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


#ifdef HAVE_RAPTOR
  serializer=raptor_new_serializer(serializer_syntax_name);
  if(!serializer) {
    fprintf(stderr, 
            "%s: Failed to create raptor serializer type %s\n", program,
            serializer_syntax_name);
    return(1);
  }

  /* base_uri=raptor_new_uri((const unsigned char*)argv[0]); */

  raptor_serialize_start_to_file_handle(serializer, base_uri, stdout);
#endif


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
    exit(1);
  }

  if(help) {
#ifdef HAVE_RAPTOR
    int i;
#endif

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
#ifdef HAVE_RAPTOR
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
    printf("    via Raptor %s serializers\n", raptor_version_string);
#else
    puts(HELP_TEXT("o", "output FORMAT   ", "Set output format to one of 'ntriples' or 'turtle'"));
#endif
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

    exit(0);
  }


  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);

  /* Perform the API call */
  rc=triplr(stdout, fc, photo_id);

 tidy:
  if(fc)
    flickcurl_free(fc);

#ifdef HAVE_RAPTOR
  if(serializer) {
    raptor_serialize_end(serializer);
    raptor_free_serializer(serializer);
  }
  if(base_uri)
    raptor_free_uri(base_uri);
  
  raptor_finish();
#endif

  return(rc);
}
