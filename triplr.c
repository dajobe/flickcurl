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



#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static const char* program;

static int output_turtle=0;

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


#define GETOPT_STRING "dho:v"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"debug",   0, 0, 'd'},
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
} namespace_table[]={
  { "a", "http://www.w3.org/2000/10/annotation-ns" },
  { "acl", "http://www.w3.org/2001/02/acls#" },
  { "blue", "http://example.org/blue#", 1 }, /* active */
  { "cell", "http://www.machinetags.org/wiki/Cell#", 1 }, /* active */
  { "dc", DC_NS },
  { "dcterms", "http://purl.org/dc/terms/" },
  { "exif", "http://nwalsh.com/rdf/exif#" },
  { "exifi", "http://nwalsh.com/rdf/exif-intrinsic#" },
  { "flickr", "x-urn:flickr:" },
  { "filtr", "http://example.org/filtr#", 1 }, /* active */
  { "foaf", FOAF_NS },
  { "geo", GEO_NS, 1 }, /* active */
  { "i", "http://www.w3.org/2004/02/image-regions#" },
  { "rdf", RDF_NS },
  { "rdfs", "http://www.w3.org/2000/01/rdf-schema#" },
  { "skos", "http://www.w3.org/2004/02/skos/core" },
  { "upcomin", "http://www.machinetags.org/wiki/Upcoming#" },
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
emit_namespace(FILE* fh,
               const char* prefix, const char* uri)
{
  if(output_turtle)
    fprintf(fh, "@prefix %s: <%s> .\n", prefix, uri);
}


/* subject/object type: 0 literal, 1 uri, 2 blank */
static void
emit_triple(FILE* fh,
            const char* subject, int subject_type,
            const char* predicate_nspace, const char* predicate_name,
            const char *object, int object_type,
            const char *datatype_uri)
{
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
}


static int
triplr(flickcurl* fc, const char* photo_id)
{
  flickcurl_photo* photo;
  flickcurl_photo_field field;
  int i;
  FILE* fh=stdout;
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
    emit_triple(fh, photo->uri, 1,
                DC_NS, "creator",
                "person", 2,
                NULL);
    emit_triple(fh, "person", 2,
                RDF_NS, "type",
                FOAF_NS "Person", 1,
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
      int type=0;
      
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
          type=1;
          break;
          
        case VALUE_TYPE_NONE:
        case VALUE_TYPE_PHOTO_ID:
        case VALUE_TYPE_PHOTO_URI:
        case VALUE_TYPE_UNIXTIME:
        default:
          break;
      }

      if(field_table[f].flags & FIELD_FLAGS_PERSON)
        emit_triple(fh, "person", 2,
                    field_table[f].nspace_uri, field_table[f].name,
                    object, type,
                    datatype_uri);
      else
        emit_triple(fh, photo->uri, 1,
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
      size_t namespace_uri_len;
      
      len=strlen(namespace_table[n].prefix);
      p=tag->raw;
      if(tag_len < len+1 || p[len] != ':')
        continue;

      if(!namespace_table[n].active)
        continue;

      namespace_uri_len=strlen(namespace_table[n].nspace_uri);

      f=p+len;
      *f++='\0';
      
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
                  photo->uri, 1,
                  namespace_table[n].nspace_uri, f,
                  v, 0, 
                  NULL);
      
      break;
    }
  }
  
  free_flickcurl_photo(photo);

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

  program=my_basename(argv[0]);

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
        debug=1;
        break;
        
      case 'h':
        help=1;
        break;

      case 'o':
        if(optarg) {
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
    printf(title_format_string, flickcurl_version_string);
    puts("Get Triples from Flickr photos.");
    printf("Usage: %s [OPTIONS] FLICKR-PHOTO-URI\n\n", program);

    fputs(flickcurl_copyright_string, stdout);
    fputs("\nLicense: ", stdout);
    puts(flickcurl_license_string);
    fputs("Flickcurl home page: ", stdout);
    puts(flickcurl_home_url_string);

    fputs("\n", stdout);

    puts(HELP_TEXT("d", "debug           ", "Print lots of output"));
    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("o", "output FORMAT   ", "Choose output format 'ntriples' or 'turtle'"));
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

    exit(0);
  }


  /* Perform the API call */
  rc=triplr(fc, photo_id);

 tidy:
  if(fc)
    flickcurl_free(fc);

  return(rc);
}
