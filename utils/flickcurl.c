/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl utility - Invoke the Flickcurl library
 *
 * Copyright (C) 2007-2012, David Beckett http://www.dajobe.org/
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
 * USAGE: flickcurl [OPTIONS] flickr-api-command args...
 *
 * ~/.flickcurl.conf should contain the authentication details in the form:
 * [flickr]
 * oauth_token=1234567-8901234567890123
 * oauth_token_secret=fedcba9876543210
 * oauth_client_key=0123456789abcdef0123456789abcdef
 * oauth_client_secret=fedcba9876543210
 *
 * If the old Flickr auth is used the values will be:
 * api_key=0123456789abcdef0123456789abcdef
 * auth_token=1234567-8901234567890123
 * secret=fedcba9876543210
 *
 * To authenticate from a FROB - to generate an auth_token from a FROB use:
 *   flickcurl -a FROB
 * FROB like 123-456-789
 * which will write a new ~/.flickcurl.conf with the auth_token received
 *
 * API calls are invoked like:
 *
 * flickcurl test.echo KEY VALUE
 *   This method does not require authentication.
 * Echoes back the KEY and VALUE received - an API test.
 *
 * flickcurl photo.getinfo PHOTO-ID
 *   PHOTO-ID like 123456789
 *   This method does not require authentication.
 *   -- http://www.flickr.com/services/api/flickr.photos.getInfo.html
 * Gets information about a photo including its tags
 *
 * See the help message for full list of supported Flickr API Calls.
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

#include <curl/curl.h>

#include <flickcurl_cmd.h>


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif

/* exported to commands.c */
int verbose = 1;
const char* program;
FILE* output_fh;
const char *output_filename = "<stdout>";


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


#ifdef HAVE_GETOPT_LONG
/* + makes GNU getopt_long() never permute the arguments */
#define GETOPT_STRING "+a:d:ho:qvV"
#else
#define GETOPT_STRING "a:d:ho:qvV"
#endif

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"auth",    1, 0, 'a'},
  {"delay",   1, 0, 'd'},
  {"help",    0, 0, 'h'},
  {"output",  0, 0, 'o'},
  {"quiet",   0, 0, 'q'},
  {"version", 0, 0, 'v'},
  {"verbose", 0, 0, 'V'},
  {NULL,      0, 0, 0}
};
#endif


static const char *title_format_string = "Flickr API utility %s\n";


static void
print_help_string(void)
{
  int i;
  printf(title_format_string, flickcurl_version_string);
  puts("Call the Flickr API to get information.");
  printf("Usage: %s [OPTIONS] command args...\n\n", program);

  fputs(flickcurl_copyright_string, stdout);
  fputs("\nLicense: ", stdout);
  puts(flickcurl_license_string);
  fputs("Flickcurl home page: ", stdout);
  puts(flickcurl_home_url_string);

  fputs("\n", stdout);

  puts(HELP_TEXT("a", "auth FROB       ", "Authenticate with a FROB and write auth config"));
  puts(HELP_TEXT("d", "delay DELAY     ", "Set delay between requests in milliseconds"));
  puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
  puts(HELP_TEXT("o", "output FILE     ", "Write format = FORMAT results to FILE"));
  puts(HELP_TEXT("q", "quiet           ", "Print less information while running"));
  puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));
  puts(HELP_TEXT("V", "verbose         ", "Print more information while running"));

  fputs("\nCommands:\n", stdout);
  for(i = 0; commands[i].name; i++)
    printf("    %-28s %s\n      %s\n", commands[i].name, commands[i].args,
	   commands[i].description);
  fputs("\nNSID is a user's Flickr ID, resembling the form 00000000@N00\n", stdout);
  fputs("\nA prefix of `flickr.' may be optionally given in all commands\n", stdout);

  fputs("\nParameters for API calls that return lists of photos:\n", stdout);

  fputs("\n  EXTRAS is a comma-separated list of optional fields to return from:\n", stdout);
  for(i = 0; 1; i++) {
    const char* name;
    const char* label;

    if(flickcurl_get_extras_format_info(i, &name, &label))
      break;
    printf("    %-16s %s\n", name, label);
  }

  fputs("\n  GROUP-EXTRAS is a comma-separated list of optional fields to return from:\n", stdout);
  fputs("    privacy\n", stdout);
  fputs("    throttle\n", stdout);
  fputs("    restrictions\n", stdout);

  fputs("\n  FORMAT is result syntax format:\n", stdout);
  for(i = 0; 1; i++) {
    const char* name;
    const char* label;

    if(flickcurl_get_feed_format_info(i, &name, &label, NULL))
      break;
    printf("    %-16s %s\n", name, label);
  }
  fputs("\n  PAGE is result page number or '-' for default (1 = first page)\n"
	"\n  PER-PAGE is photos per result page or '-' for default (10)\n",
	stdout
	);
  /* Extra space for neater distinctions in output */
  fputs("\n", stdout);
}

int
main(int argc, char *argv[]) 
{
  flickcurl *fc = NULL;
  int rc = 0;
  int usage = 0;
  int help = 0;
  int cmd_index= -1;
  int read_auth = 1;
  int i;
  int request_delay= -1;
  char *command = NULL;

  output_fh = stdout;
  
  flickcurl_init();  
  flickcurl_cmdline_init();

  program = flickcurl_cmdline_basename(argv[0]);

  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    rc = 1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(read_auth) {
    if(!access((const char*)flickcurl_cmdline_config_path, R_OK)) {
      if(flickcurl_config_read_ini(fc, flickcurl_cmdline_config_path,
                                   flickcurl_cmdline_config_section, fc,
                                   flickcurl_config_var_handler)) {
        rc = 1;
        goto tidy;
      }
    } else {
      /* Check if the user has requested to see the help message */
      for (i = 0; i < argc; ++i) {
	if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
	  print_help_string();
	}
      }

      fprintf(stderr, "%s: Configuration file %s not found.\n\n"
"1. Visit http://www.flickr.com/services/api/keys/ to get an <API Key>\n"
"    and <Shared Secret>.\n"
"\n"
"2. Create %s in this format:\n"
"[flickr]\n"
"oauth_client_key=<Client key / API Key>\n"
"oauth_client_secret=<Client secret / Shared Secret>\n"
"\n"
"3. Call this program with:\n"
"  %s oauth.create\n"
"  (or %s oauth.create <Callback URL> if you understand and need that)\n"
"This gives a <Request Token> <Request Token Secret> and <Authentication URL>\n"
"\n"
"4. Visit the <Authentication URL> and approve the request to get a <Verifier>\n"
"\n"
"5. Call this program with the <Request Token>, <Request Token Secret>\n"
"    and <Verifier>:\n"
"  %s oauth.verify <Request Token> <Request Token Secret> <Verifier>\n"
"\n"
"This will write the configuration file with the OAuth access tokens.\n"
"See http://librdf.org/flickcurl/api/flickcurl-auth.html for full instructions.\n",
	      program, flickcurl_cmdline_config_path,
              flickcurl_cmdline_config_path,
              program, program,
              program);
      rc = 1;
      goto tidy;
    }
  }

  while (!usage && !help)
  {
    int c;
    char* auth_token = NULL;
    
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

      case 'a':
        /* Exchange the frob for a full token */
        auth_token = flickcurl_auth_getFullToken(fc, optarg);
        if(!auth_token) {
          fprintf(stderr, 
                  "%s: Could not find auth_token in getFullToken response\n",
                  program);
          rc = 1;
        } else {
          fprintf(stdout, 
                  "%s: Successfully exchanged frob %s for authentication token\n",
                  program, optarg);
          
          flickcurl_set_auth_token(fc, auth_token);

          rc = flickcurl_config_write_ini(fc, flickcurl_cmdline_config_path,
                                          flickcurl_cmdline_config_section);
          if(!rc)
            fprintf(stdout,
                  "%s: Updated configuration file %s with authentication token\n",
                    program, flickcurl_cmdline_config_path);
        }
        goto tidy;

      case 'd':
        if(optarg)
          request_delay = atoi(optarg);
        break;
        
      case 'h':
        help = 1;
        break;

      case 'o':
        if(optarg) {
          output_filename = optarg;
          output_fh = fopen(output_filename, "w");
          if(!output_fh) {
            fprintf(stderr, "%s: Failed to write to output file %s: %s\n",
                    program, output_filename, strerror(errno));
            rc = 1;
            goto tidy;
          }
        }
        break;

      case 'q':
        verbose = 0;
        break;

      case 'v':
        fputs(flickcurl_version_string, stdout);
        fputc('\n', stdout);

        flickcurl_cmdline_finish();
        exit(0);

      case 'V':
        verbose = 2;
        break;
    }
    
  }

  argv += optind;
  argc -= optind;
  
  if(!help && !argc) {
    usage = 2; /* Title and usage */
    goto usage;
  }

  if(usage || help)
    goto usage;


  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);

  command = argv[0];
  
  /* allow old format commands to work */
  for(i = 0; command[i]; i++) {
    if(command[i] == '-')
      command[i] = '.';
  }
  
  if(!strncmp(command, "flickr.", 7))
    command+= 7;

  if(!strcmp(command, "places.forUser"))
    command = (char*)"places.placesForUser";
  
  for(i = 0; commands[i].name; i++)
    if(!strcmp(command, commands[i].name)) {
      cmd_index = i;
      break;
    }
  if(cmd_index < 0) {
    fprintf(stderr, "%s: No such command `%s'\n", program, command);
    usage = 1;
    goto usage;
  }

  if((argc-1) < commands[cmd_index].min) {
    fprintf(stderr,
            "%s: Minimum of %d arguments for command `%s'\n  USAGE: %s %s %s\n",
            program,
            commands[cmd_index].min, command,
            program, command, commands[cmd_index].args);
    usage = 1;
    goto usage;
  }
  
  if(commands[cmd_index].max > 0 && 
     (argc-1) > commands[cmd_index].max) {
    fprintf(stderr,
            "%s: Maxiumum of %d arguments for command `%s'\n  USAGE: %s %s %s\n",
            program,
            commands[cmd_index].max, command,
            program, command, commands[cmd_index].args);
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


  /* Perform the API call */
  rc = commands[cmd_index].handler(fc, argc, argv);
  if(rc)
    fprintf(stderr, "%s: Command %s failed\n", program, argv[0]);
  
 tidy:
  if(output_fh) {
    fclose(output_fh);
    output_fh = NULL;
  }
  
  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  flickcurl_cmdline_finish();

  return(rc);
}
