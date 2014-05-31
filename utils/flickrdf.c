/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickrdf - Triples from Flickrs
 *
 * Copyright (C) 2011-2012, David Beckett http://www.dajobe.org/
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
#include <raptor2.h>
#else
#include <raptor_fake.h>
#endif

#include <flickcurl_cmd.h>


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static int debug = 0;

const char* program;


static void
my_message_handler(void *user_data, const char *message)
{
  fprintf(stderr, "%s: ERROR: %s\n", program, message);
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



static raptor_world* rworld;


static void
ser_emit_namespace(void* user_data,
                   const char *prefix, size_t prefix_len,
                   const char* uri, size_t uri_len)
{
  raptor_serializer* serializer = (raptor_serializer*)user_data;
  raptor_uri *ns_uri = NULL;

  ns_uri = raptor_new_uri(rworld, (const unsigned char*)uri);
  raptor_serializer_set_namespace(serializer, ns_uri, 
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
  raptor_statement s; /* static */
  raptor_uri* predicate_ns_uri;
  raptor_uri* predicate_uri;
  
  raptor_statement_init(&s, rworld);
  
  if(subject_type == FLICKCURL_TERM_TYPE_RESOURCE)
    s.subject = raptor_new_term_from_uri_string(rworld, (const unsigned char*)subject);
  else /* blank node */
    s.subject = raptor_new_term_from_blank(rworld, (const unsigned char*)subject);

  predicate_ns_uri = raptor_new_uri(rworld, (const unsigned char*)predicate_nspace);
  predicate_uri = raptor_new_uri_from_uri_local_name(rworld, predicate_ns_uri,
                                                     (const unsigned char*)predicate_name);
  s.predicate = raptor_new_term_from_uri(rworld, predicate_uri);
  raptor_free_uri(predicate_uri);
  raptor_free_uri(predicate_ns_uri);

  if(object_type == FLICKCURL_TERM_TYPE_RESOURCE)
    s.object = raptor_new_term_from_uri_string(rworld, (const unsigned char*)object);
  else if(object_type == FLICKCURL_TERM_TYPE_BLANK)
    s.object = raptor_new_term_from_blank(rworld, (const unsigned char*)subject);
  else {
    /* literal */
    raptor_uri* raptor_datatype_uri = NULL;

    if(datatype_uri)
      raptor_datatype_uri = raptor_new_uri(rworld, (const unsigned char*)datatype_uri);

    s.object = raptor_new_term_from_literal(rworld, (const unsigned char*)object, raptor_datatype_uri, NULL /* language */);

    raptor_free_uri(raptor_datatype_uri);
  }

  raptor_serializer_serialize_statement(serializer, &s);

raptor_statement_clear(&s);

}


static void
ser_emit_finish(void* user_data)
{
  raptor_serializer* serializer = (raptor_serializer*)user_data;
  raptor_serializer_serialize_end(serializer);
}


static flickcurl_serializer_factory flickrdf_serializer_factory = {
  1, ser_emit_namespace, ser_emit_triple, ser_emit_finish
};


static const char *title_format_string = "Flickrdf - triples from flickrs %s\n";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc = NULL;
  int rc = 0;
  int usage = 0;
  int help = 0;
  char* photo_id = NULL;
  const char* prefix_uri = "http://www.flickr.com/photos/";
  size_t prefix_uri_len = strlen(prefix_uri);
  const char *serializer_syntax_name = "ntriples";
  raptor_uri* base_uri = NULL;
  raptor_serializer* serializer = NULL;
  int request_delay= -1;
  flickcurl_serializer* fs = NULL;
  flickcurl_photo* photo = NULL;

  flickcurl_init();
  flickcurl_cmdline_init();

  program = flickcurl_cmdline_basename(argv[0]);

  rworld = raptor_new_world();
  raptor_world_open(rworld);

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
          if(raptor_world_is_serializer_name(rworld, optarg))
            serializer_syntax_name = optarg;
          else {
            int i;
            
            fprintf(stderr,
                    "%s: invalid argument `%s' for `" HELP_ARG(o, output) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n");
            for(i = 0; 1; i++) {
              const raptor_syntax_description *d;

              d = raptor_world_get_serializer_description(rworld, i);
              if(!d)
                break;
              printf("  %-12s for %s\n", d->names[0], d->label);
            }
            usage = 1;
            break;
            
          }
        }
        break;
        
      case 'v':
        fputs(flickcurl_version_string, stdout);
        fputc('\n', stdout);

        flickcurl_cmdline_finish();
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


  serializer = raptor_new_serializer(rworld, serializer_syntax_name);
  if(!serializer) {
    fprintf(stderr, 
            "%s: Failed to create raptor serializer type %s\n", program,
            serializer_syntax_name);
    return(1);
  }

  base_uri = raptor_new_uri(rworld, (const unsigned char*)argv[0]);

  raptor_serializer_start_to_file_handle(serializer, base_uri, stdout);


  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    rc = 1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(!access((const char*)flickcurl_cmdline_config_path, R_OK)) {
    if(flickcurl_config_read_ini(fc, flickcurl_cmdline_config_path,
                                 flickcurl_cmdline_config_section, fc,
                                 flickcurl_config_var_handler)) {
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
      const raptor_syntax_description* d;

      d = raptor_world_get_serializer_description(rworld, i);
      if(!d)
        break;

      if(!strcmp(d->names[0], serializer_syntax_name))
        printf("      %-15s %s (default)\n", d->names[0], d->label);
      else
        printf("      %-15s %s\n", d->names[0], d->label);
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
  
  photo = flickcurl_photos_getInfo2(fc, photo_id, NULL);

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

  if(rworld)
    raptor_free_world(rworld);

  flickcurl_finish();

  flickcurl_cmdline_finish();

  return(rc);
}
