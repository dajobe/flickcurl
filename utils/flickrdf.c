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
#endif


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static const char* program;

static int debug = 0;



static const char*
my_basename(const char *name)
{
  char *p;
  if((p = strrchr(name, '/')))
    name = p+1;
  else if((p = strrchr(name, '\\')))
    name = p+1;

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
  flickcurl *fc = (flickcurl *)userdata;
  
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


#ifndef HAVE_RAPTOR
typedef enum {
  RAPTOR_IDENTIFIER_TYPE_UNKNOWN,
  RAPTOR_IDENTIFIER_TYPE_RESOURCE,
  RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
  RAPTOR_IDENTIFIER_TYPE_PREDICATE,
  RAPTOR_IDENTIFIER_TYPE_ORDINAL,
  RAPTOR_IDENTIFIER_TYPE_LITERAL,
  RAPTOR_IDENTIFIER_TYPE_XML_LITERAL
} raptor_identifier_type;
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

  len = strlen((const char*)uri);
  u = (raptor_uri*)malloc(len+1);
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
  
  len1 = strlen((const char*)u);
  len2 = strlen((const char*)name);

  newu = (raptor_uri*)malloc(len1+len2+1);
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
  
  for(i = 0; i < NSERIALIZERS; i++) {
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
    *name_p = serializers[counter].name;
  if(label_p)
    *label_p = serializers[counter].label;
  return 0;
}


static raptor_serializer*
raptor_new_serializer(const char* serializer_name)
{
  raptor_serializer* s;
  s = (raptor_serializer*)calloc(sizeof(raptor_serializer), 1);
  s->output_turtle = !strcmp((const char*)serializer_name, "turtle");
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
  FILE* fh = serializer->fh;
  if(serializer->output_turtle)
    fprintf(fh, "@prefix %s: <%s> .\n", (const char*)prefix, (const char*)uri);
}


static void
raptor_serialize_start_to_file_handle(raptor_serializer* serializer,
                                      raptor_uri* base_uri, FILE* fh)
{
  serializer->fh = fh;
  if(base_uri && serializer->output_turtle)
    fprintf(fh, "@base <%s>\n", (char*)base_uri);
}


static void
raptor_serialize_statement(raptor_serializer* serializer, raptor_statement* s)
{
  FILE *fh = serializer->fh;

  if(s->subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    fprintf(fh, "<%s>", (const char*)s->subject);
  else /* blank node */
    fprintf(fh, "_:%s", (const char*)s->subject);

  fprintf(fh, " <%s> ", (const char*)s->predicate);

  if(s->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    const char *p;
    char c;
    
    fputc('"', fh);
    for(p = (const char*)s->object; (c = *p); p++) {
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
ser_emit_namespace(void* user_data,
                   const char *prefix, size_t prefix_len,
                   const char* uri, size_t uri_len)
{
  raptor_serializer* serializer = (raptor_serializer*)user_data;
  raptor_uri *ns_uri = NULL;

  ns_uri = raptor_new_uri((const unsigned char*)uri);
  raptor_serialize_set_namespace(serializer, ns_uri, 
                                 (const unsigned char*)prefix);
  raptor_free_uri(ns_uri);
}


static void
ser_emit_triple(void* user_data,
                const char* subject, int subject_type,
                const char* predicate_nspace, const char* predicate_name,
                const char *object, int object_type,
                const char *datatype_uri)
{
  raptor_serializer* serializer = (raptor_serializer*)user_data;
  raptor_statement s;
  raptor_uri* predicate_ns_uri;
  
  s.subject_type = (raptor_identifier_type)subject_type;
  if((flickcurl_term_type)s.subject_type == FLICKCURL_TERM_TYPE_RESOURCE)
    s.subject = (void*)raptor_new_uri((const unsigned char*)subject);
  else /* blank node */
    s.subject = (void*)subject;

  predicate_ns_uri = raptor_new_uri((const unsigned char*)predicate_nspace);
  s.predicate = (void*)raptor_new_uri_from_uri_local_name(predicate_ns_uri,
                                                        (const unsigned char*)predicate_name);
  raptor_free_uri(predicate_ns_uri);
  s.predicate_type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;

  s.object_type = (raptor_identifier_type)object_type;
  if((flickcurl_term_type)s.object_type == FLICKCURL_TERM_TYPE_RESOURCE)
    s.object = (void*)raptor_new_uri((const unsigned char*)object);
  else /* literal or blank node */
    s.object = (void*)object;
  if(datatype_uri)
    s.object_literal_datatype = raptor_new_uri((const unsigned char*)datatype_uri);
  else
    s.object_literal_datatype = NULL;
  s.object_literal_language = NULL;

  raptor_serialize_statement(serializer, &s);

  if((flickcurl_term_type)s.subject_type == FLICKCURL_TERM_TYPE_RESOURCE)
    raptor_free_uri((raptor_uri*)s.subject);
  raptor_free_uri((raptor_uri*)s.predicate);
  if((flickcurl_term_type)s.object_type == FLICKCURL_TERM_TYPE_RESOURCE)
    raptor_free_uri((raptor_uri*)s.object);
  if(datatype_uri)
    raptor_free_uri(s.object_literal_datatype);
}


static void
ser_emit_finish(void* user_data)
{
  raptor_serializer* serializer = (raptor_serializer*)user_data;
  raptor_serialize_end(serializer);
}


static flickcurl_serializer_factory flickrdf_serializer_factory = {
  1, ser_emit_namespace, ser_emit_triple, ser_emit_finish
};


static const char *title_format_string = "Flickrdf - triples from flickrs %s\n";

static const char* config_filename = ".flickcurl.conf";
static const char* config_section = "flickr";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc = NULL;
  int rc = 0;
  int usage = 0;
  int help = 0;
  const char* home;
  char config_path[1024];
  char* photo_id = NULL;
  const char* prefix_uri = "http://www.flickr.com/photos/";
  size_t prefix_uri_len = strlen(prefix_uri);
  const char *serializer_syntax_name = "ntriples";
  raptor_uri* base_uri = NULL;
  raptor_serializer* serializer = NULL;
  int request_delay= -1;
  flickcurl_serializer* fs = NULL;
  flickcurl_photo* photo = NULL;

  program = my_basename(argv[0]);

  flickcurl_init();

  raptor_init();

  home = getenv("HOME");
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
        usage = 1;
        break;

      case 'd':
        if(optarg)
          request_delay = atoi(optarg);
        break;
        
      case 'D':
        debug = 1;
        break;
        
      case 'h':
        help = 1;
        break;

      case 'o':
        if(optarg) {
          if(raptor_serializer_syntax_name_check(optarg))
            serializer_syntax_name = optarg;
          else {
            int i;
            
            fprintf(stderr,
                    "%s: invalid argument `%s' for `" HELP_ARG(o, output) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n");
            for(i = 0; 1; i++) {
              const char *help_name;
              const char *help_label;
              if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
                break;
              printf("  %-12s for %s\n", help_name, help_label);
            }
            usage = 1;
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

  argv+= optind;
  argc-= optind;
  
  if(!help && argc < 1)
    usage = 2; /* Title and usage */

  if(!help && !argc) {
    fprintf(stderr, "%s: No photo URI given\n", program);
    usage = 1;
    goto usage;
  }

  if(usage || help)
    goto usage;

  photo_id = argv[0];
  if(strncmp(photo_id, prefix_uri, prefix_uri_len))
    usage = 1;
  else {
    size_t len;

    photo_id+= prefix_uri_len;
    len = strlen(photo_id);
    if(!len)
      usage = 1;
    else {
      if(photo_id[len-1] == '/')
        photo_id[--len] = '\0';
      
      while(*photo_id && *photo_id != '/')
        photo_id++;
      if(!*photo_id)
        usage = 1;
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


  serializer = raptor_new_serializer(serializer_syntax_name);
  if(!serializer) {
    fprintf(stderr, 
            "%s: Failed to create raptor serializer type %s\n", program,
            serializer_syntax_name);
    return(1);
  }

  /* base_uri = raptor_new_uri((const unsigned char*)argv[0]); */

  raptor_serialize_start_to_file_handle(serializer, base_uri, stdout);


  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    rc = 1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(!access((const char*)config_path, R_OK)) {
    if(read_ini_config(config_path, config_section, fc,
                       my_set_config_var_handler)) {
      fprintf(stderr, "%s: Failed to read config filename %s: %s\n",
              program, config_path, strerror(errno));
      rc = 1;
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
    rc = 1;
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
    for(i = 0; 1; i++) {
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

    rc = 0;
    goto tidy;
  }


  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);
  
  fs = flickcurl_new_serializer(fc, serializer, &flickrdf_serializer_factory);
  if(!fs) {
    fprintf(stderr, "%s: Failed to create Flickcurl serializer\n", program);
    goto tidy;
  }
  
  photo = flickcurl_photos_getInfo(fc, photo_id);

  if(!photo)
    goto tidy;

  if(debug)
    fprintf(stderr, "%s: Photo with URI %s ID %s has %d tags\n",
            program, photo->uri, photo->id, photo->tags_count);

  rc = flickcurl_serialize_photo(fs, photo);

 tidy:
  if(photo)
    flickcurl_free_photo(photo);

  if(fs)
    flickcurl_free_serializer(fs);

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
