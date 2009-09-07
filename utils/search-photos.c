/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * search-photos - Search for my interesting photos about a tag
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
 *
 * USAGE: search-photos [OPTIONS] TAG
 *
 * This program is an example of how to use the search API to find
 * photos with Flickcurl.
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


static const char *title_format_string="search-photos - search for my interesting photos about a tag %s\n";

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
  char* tag = NULL;
  int request_delay= -1;
  flickcurl_photos_list_params list_params;
  flickcurl_search_params params;
  flickcurl_photos_list* photos_list = NULL;
  int i;
  
  program=my_basename(argv[0]);

  flickcurl_init();

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
    fprintf(stderr, "%s: No tag given\nTry `%s kitten' or a tag you have used for your photos.\n", program, program);
    usage=1;
    goto usage;
  }

  if(usage || help)
    goto usage;

  tag = argv[0];

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
    printf(title_format_string, flickcurl_version_string);
    puts("Search for my intersting Flickr photos about a tag.");
    printf("Usage: %s [OPTIONS] TAG\n\n", program);

    fputs(flickcurl_copyright_string, stdout);
    fputs("\nLicense: ", stdout);
    puts(flickcurl_license_string);
    fputs("Flickcurl home page: ", stdout);
    puts(flickcurl_home_url_string);

    fputs("\n", stdout);

    puts(HELP_TEXT("d", "delay DELAY     ", "Set delay between requests in milliseconds"));
    puts(HELP_TEXT("D", "debug           ", "Print debug messages"));
    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl library version"));

    rc=0;
    goto tidy;
  }


  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);
  
  /* Initialise the search parameters themselves
   *  user_id: "me" - Search only photos of the calling user.
   *  sort: "interestingness-desc" - return results with most interesting first
   *  tag: TAG - search for photos about the TAG given on the command line
   */
  flickcurl_search_params_init(&params);
  params.user_id = strdup("me");
  params.sort = strdup("interestingness-desc");
  params.tags = strdup(tag);

  /* Initialise the search result (list) parameters:
   *   per_page: 10 - ten results per-page
   *   page: 1 - return 1 page
   *   extras: "original_format" - want the returned photos to have the
   *      original secret and format fields.
   */
  flickcurl_photos_list_params_init(&list_params);
  list_params.per_page = 10;
  list_params.page = 1;
  list_params.extras = strdup("original_format");

  photos_list = flickcurl_photos_search_params(fc, &params, &list_params);
  if(!photos_list)
    goto tidy;

  if(debug)
    fprintf(stderr, "%s: Search returned %d photos\n", 
            program, photos_list->photos_count);

  for(i = 0; i < photos_list->photos_count; ++i) {
    fprintf(stdout, "Result #%d ", i);
    if(photos_list->photos[i]) { 
      char* uri = flickcurl_photo_as_source_uri(photos_list->photos[i], 'o');
      fprintf(stdout, "uri: %s\n", uri);
      free(uri);
    } else
      fputc('\n', stdout);
  }
  
 tidy:
  if(photos_list)
    flickcurl_free_photos_list(photos_list);

  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
