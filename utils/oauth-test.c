/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * Flickr OAuth test utility
 *
 * Copyright (C) 2011, David Beckett http://www.dajobe.org/
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
 * USAGE: oauth-test [OPTIONS]
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
#include <stdint.h>

/* many places for getopt */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <flickcurl_getopt.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>



#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static const char* program;

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


static int
oauth_prepare(flickcurl *fc,
              const char* method, const char* parameters[][2], int count)
{
  if(!method) {
    flickcurl_error(fc, "No method to prepare");
    return 1;
  }
  
  return flickcurl_oauth_prepare_common(fc,
                                        fc->service_uri,
                                        method,
                                        NULL, NULL,
                                        parameters, count,
                                        1, 1);
}


static int
oauth_test_echo(flickcurl* fc,
                const char* key, const char* value)
{
  const char * parameters[2 + FLICKCURL_MAX_OAUTH_PARAM_COUNT][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int rc = 0;
  
  parameters[count][0]  = key;
  parameters[count++][1]= value;

  parameters[count][0]  = NULL;

  if(oauth_prepare(fc, "flickr.test.echo", parameters, count)) {
    rc = 1;
    goto tidy;
  }

  doc = flickcurl_invoke(fc);
  if(!doc) {
    rc = 1;
    goto tidy;
  }

  fprintf(stderr, "Flickr echo returned %d bytes\n", fc->total_bytes);
  
  tidy:
  
  return rc;
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


#define GETOPT_STRING "hv"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"help",    0, 0, 'h'},
  {"version", 0, 0, 'v'},
  {NULL,      0, 0, 0}
};
#endif


static const char *title_format_string = "Flickr OAuth test utility %s\n";

static const char* config_filename = ".flickcurl.conf";
static const char* config_section = "flickr";


static void
print_help_string(void)
{
  printf(title_format_string, flickcurl_version_string);
  puts("Flickr OAuth test utility.");
  printf("Usage: %s [OPTIONS] COMMANDS\n\n", program);
  
  fputs(flickcurl_copyright_string, stdout);
  fputs("\nLicense: ", stdout);
  puts(flickcurl_license_string);
  fputs("Flickcurl home page: ", stdout);
  puts(flickcurl_home_url_string);
  
  fputs("\n", stdout);
  
  puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
  puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

  fputs("\nCOMMANDS\n", stdout);

  puts("  request_token\n    Ask for an OAuth request token and show the authorize url.\n");
  puts("  access_token   REQUEST_TOKEN REQUEST_TOKEN_SECRET VERIFIER\n    Use a request token with verifier to get an access token.\n");
  puts("  echo\n    Run the test.echo API call using OAuth.\n");
}



int
main(int argc, char *argv[]) 
{
  flickcurl *fc = NULL;
  int rc = 0;
  int usage = 0;
  int help = 0;
  int cmd_index = -1;
  int read_auth = 1;
  const char* home;
  char config_path[1024];
  char *command = NULL;

  flickcurl_init();
  
  program = my_basename(argv[0]);

  home = getenv("HOME");
  if(home)
    sprintf(config_path, "%s/%s", home, config_filename);
  else
    strcpy(config_path, config_filename);
  

  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    rc = 1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(read_auth && !access((const char*)config_path, R_OK)) {
    if(flickcurl_config_read_ini(fc, config_path, config_section, fc,
                                 flickcurl_config_var_handler)) {
      rc = 1;
      goto tidy;
    }
  }


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

      case 'h':
        help = 1;
        break;

      case 'v':
        fputs(flickcurl_version_string, stdout);
        fputc('\n', stdout);

        exit(0);
    }
    
  }
  
  argv += optind;
  argc -= optind;
  
  if(!help && !argc) {
    usage = 2; /* Title and usage */
    goto usage;
  }

  if(!help && !argc) {
    usage = 2; /* Title and usage */
    goto usage;
  }

  if(usage || help)
    goto usage;


  command = argv[0];

  if(!strncmp(command, "flickr.", 7))
    command += 7;
  
  if(!strcmp(command, "request_token")) {
    cmd_index = 0;
  } else if(!strcmp(command, "access_token")) {
    cmd_index = 1;
  } else if(!strcmp(command, "echo")) {
    cmd_index = 2;
  }

  if(cmd_index < 0) {
    fprintf(stderr, "%s: No such command `%s'\n", program, command);
    usage = 1;
    goto usage;
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
    print_help_string();
    rc = 0;
    goto tidy;
  }


  /* Request token */
  if(cmd_index == 0) {
    flickcurl_set_oauth_client_credentials(fc, fc->api_key, fc->secret);

    rc = flickcurl_oauth_create_request_token(fc);
    if(!rc) {
      char* uri;

      fprintf(stderr, "%s: Request token %s and request token secret %s\n",
              program,
              flickcurl_get_oauth_request_token(fc),
              flickcurl_get_oauth_request_token_secret(fc));
      
      uri = flickcurl_oauth_get_authorize_uri(fc);
      if(uri) {
        fprintf(stderr, "%s: Authorize uri is %s\n", program, uri);
        free(uri);
      }
    }
  }


  /* Access token */
  if(cmd_index == 1) {
    const char* request_token = argv[1];
    const char* request_token_secret = argv[2];
    const char* verifier = argv[3];

    flickcurl_set_oauth_client_credentials(fc, fc->api_key, fc->secret);
    flickcurl_set_oauth_request_credentials(fc, request_token, request_token_secret);
    rc = flickcurl_oauth_create_access_token(fc, verifier);

    if(!rc) {
      fprintf(stderr, 
              "OAuth access token returned token '%s' secret token '%s'\n",
              flickcurl_get_oauth_token(fc),
              flickcurl_get_oauth_token_secret(fc));
    }

  }


  if(cmd_index == 2) {
    flickcurl_oauth_data* od = &fc->od;

    memset(od, '\0', sizeof(od));

    rc = oauth_test_echo(fc, "hello", "world");
  }
  

 tidy:
  
  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
