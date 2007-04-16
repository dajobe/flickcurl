/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * codegen utility - Make C code from Flickr API by reflection
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
 * USAGE: codegen [OPTIONS] flickr.section
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


#define GETOPT_STRING "a:d:hv"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"auth",    1, 0, 'a'},
  {"delay",   1, 0, 'd'},
  {"help",    0, 0, 'h'},
  {"version", 0, 0, 'v'},
  {NULL,      0, 0, 0}
};
#endif


static const char *title_format_string="Code gen utility %s\n";

static const char* config_filename=".flickcurl.conf";
static const char* config_section="flickr";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc=NULL;
  int rc=0;
  int usage=0;
  int help=0;
  int read_auth=1;
  int i;
  const char* home;
  char config_path[1024];
  int request_delay= -1;
  char section[50];
  size_t section_len;
  char** methods=NULL;
  
  flickcurl_init();
  
  program=my_basename(argv[0]);

  home=getenv("HOME");
  if(home)
    sprintf(config_path, "%s/%s", home, config_filename);
  else
    strcpy(config_path, config_filename);
  

  /* Initialise the Flickcurl library */
  fc=flickcurl_new();
  if(!fc) {
    rc=1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(read_auth && !access((const char*)config_path, R_OK)) {
    if(read_ini_config(config_path, config_section, fc,
                       my_set_config_var_handler)) {
      fprintf(stderr, "%s: Failed to read config filename %s: %s\n",
              program, config_path, strerror(errno));
      rc=1;
      goto tidy;
    }
  }

  while (!usage && !help)
  {
    int c;
    char* auth_token=NULL;
    
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

      case 'a':
        /* Exchange the mini-token for a full token */
        auth_token=flickcurl_auth_getFullToken(fc, optarg);
        if(!auth_token) {
          fprintf(stderr, 
                  "%s: Could not find auth_token in getFullToken response\n",
                  program);
        } else {
          FILE* fh;

          flickcurl_set_auth_token(fc, auth_token);
          
          fh=fopen(config_path, "w");
          if(!fh) {
            fprintf(stderr, "%s: Failed to write to config filename %s: %s\n",
                    program, config_path, strerror(errno));
          } else {
            fputs("[flickr]\nauth_token=", fh);
            fputs(flickcurl_get_auth_token(fc), fh);
            fputs("\napi_key=", fh);
            fputs(flickcurl_get_api_key(fc), fh);
            fputs("\nsecret=", fh);
            fputs(flickcurl_get_shared_secret(fc), fh);
            fputs("\n", fh);
            fclose(fh);
            read_auth=0;
          }
        }
        
        break;

      case 'd':
        if(optarg)
          request_delay=atoi(optarg);
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

  if(!help && argc < 1)
    usage=2; /* Title and usage */

  if(!help && !argc) {
    fprintf(stderr, "%s: No API section given\n", program);
    usage=1;
  }

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
    puts("Make C code from Flickr API by reflection.");
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
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

    rc=0;
    goto tidy;
  }


  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);

  strcpy(section, "flickr.");
  strcpy(section+7, argv[1]);
  
  /* allow old format commands to work */
  for(i=0; section[i]; i++) {
    if(section[i] == '-')
      section[i]='.';
  }
  
  if(!strncmp(section, "flickr.flickr", 13))
    strcpy(section, section+7);
  section_len=strlen(section);
  
  fprintf(stderr, "%s: section '%s'\n", program, section);
    
  methods=flickcurl_reflection_getMethods(fc);
  if(!methods) {
    fprintf(stderr, "%s: getMethods failed\n", program);
    rc=1;
    goto tidy;
  }

  fprintf(stdout,
"/* -*- Mode: c; c-basic-offset: 2 -*-\n"
" *\n"
" * %s.c - Flickr %s.* API calls\n"
" *\n",
          argv[1], section);

  fprintf(stdout,
" * Copyright (C) 2007, David Beckett http://purl.org/net/dajobe/\n"
" * \n"
" * This file is licensed under the following three licenses as alternatives:\n"
" *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version\n"
" *   2. GNU General Public License (GPL) V2 or any newer version\n"
" *   3. Apache License, V2.0 or any newer version\n"
" * \n"
" * You may not use this file except in compliance with at least one of\n"
" * the above three licenses.\n"
" * \n"
" * See LICENSE.html or LICENSE.txt at the top of this package for the\n"
" * complete terms and further detail along with the license texts for\n"
" * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.\n"
" * \n"
" */\n"
"\n"
"#include <stdio.h>\n"
"#include <string.h>\n"
"#include <stdarg.h>\n"
"\n"
"#ifdef HAVE_CONFIG_H\n"
"#include <config.h>\n"
"#endif\n"
"\n"
"#ifdef HAVE_STDLIB_H\n"
"#include <stdlib.h>\n"
"#undef HAVE_STDLIB_H\n"
"#endif\n"
"#ifdef HAVE_UNISTD_H\n"
"#include <unistd.h>\n"
"#endif\n"
"\n"
"#include <flickcurl.h>\n"
"#include <flickcurl_internal.h>\n"
"\n"
"\n"
          );

  
  for(i=0; methods[i]; i++) {
    flickcurl_method* method;
    char* method_name;
    char function_name[100];
    int c, j;
    int is_write=0;
    
    if(strncmp(methods[i], section, section_len))
      continue;
    
    method=flickcurl_reflection_getMethodInfo(fc, methods[i]);
    if(!method) {
      fprintf(stderr, "%s: getMethodInfo(%s) failed\n", program, methods[i]);
      rc=1;
      break;
    }

    method_name=method->name;

    if(
      strstr(method_name, ".add") ||
      strstr(method_name, ".create") ||
      strstr(method_name, ".delete") ||
      strstr(method_name, ".edit") ||
      strstr(method_name, ".remove") ||
      strstr(method_name, ".set")
       )
      is_write=1;
    
    strcpy(function_name, "flickcurl_");
    for(j=0; (c=methods[i][j+7]); j++) {
      if(c=='.')
        c='_';
      function_name[j+10]=c;
    }
    function_name[j+10]='\0';

    fprintf(stdout, "/**\n * %s:\n", function_name);

    /* fixed arguments */
    fprintf(stdout, " * @fc: flickcurl context\n");

    if(method->args_count) {
      int argi;
      for(argi=0; method->args[argi]; argi++) {
        flickcurl_arg* arg=method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;

        fprintf(stdout, " * @%s: %s%s\n", arg->name, arg->description,
                (arg->optional? " (or NULL)" : ""));
      }
    }

    fprintf(stdout,
" * \n"
" * %s\n"
" *\n"
" * Implements %s (%s)\n",
            method->description, method->name, flickcurl_version_string
           );

    fprintf(stdout,
" * \n"
" * Return value: non-0 on failure\n"
" **/\n"
           );
      
    fprintf(stdout, "int\n%s(flickcurl* fc", function_name);
    if(method->args_count) {
      int argi;
      for(argi=0; method->args[argi]; argi++) {
        flickcurl_arg* arg=method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;
        
        fprintf(stdout, ", const char* %s", arg->name);
      }
    }
    fprintf(stdout, ")\n{\n");

    fprintf(stdout,
"  const char* parameters[%d][2];\n"
"  int count=0;\n"
"  xmlDocPtr doc=NULL;\n"
"  xmlXPathContextPtr xpathCtx=NULL; \n"
"  void* result=NULL;\n"
"  \n",
  6+method->args_count);

    if(method->args_count) {
      int argi;
      int print_or=0;
      
      fprintf(stdout, "  if(");
      for(argi=0; method->args[argi]; argi++) {
        flickcurl_arg* arg=method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;
        
        if(!arg->optional) {
          if(print_or)
            fprintf(stdout, " || ");
          
          fprintf(stdout, "!%s", arg->name);
          print_or=1;
        }
      }
      fprintf(stdout,
")\n"
"    return 1;\n"
"\n");
    }


    if(method->args_count) {
      int argi;
      for(argi=0; method->args[argi]; argi++) {
        flickcurl_arg* arg=method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;
        
        fprintf(stdout,
"  parameters[count][0]  = \"%s\";\n"
"  parameters[count++][1]= %s;\n",
              arg->name, arg->name);
      }
    }

    fprintf(stdout,
"\n"
"  parameters[count][0]  = NULL;\n"
"\n"
            );

    fprintf(stdout,
"  if(flickcurl_prepare(fc, \"%s\", parameters, count))\n"
"    goto tidy;\n"
"\n",
    method->name);

    if(is_write)
      fprintf(stdout, 
"  flickcurl_set_write(fc, 1);\n"
"  flickcurl_set_data(fc, (void*)\"\", 0);\n"
"\n"
);

    fprintf(stdout,
"  doc=flickcurl_invoke(fc);\n"
"  if(!doc)\n"
"    goto tidy;\n"
"\n"
"\n"
"  xpathCtx = xmlXPathNewContext(doc);\n"
"  if(!xpathCtx) {\n"
"    flickcurl_error(fc, \"Failed to create XPath context for document\");\n"
"    fc->failed=1;\n"
"    goto tidy;\n"
"  }\n"
"\n"
"  result=NULL; /* your code here */\n"
"\n"
"  tidy:\n"
"  if(xpathCtx)\n"
"    xmlXPathFreeContext(xpathCtx);\n"
"\n"
"  if(fc->failed)\n"
"    result=NULL;\n"
"\n"
"  return (result == NULL);\n"
"}\n"
"\n"
"\n");

    flickcurl_free_method(method);
  }


 tidy:
  if(methods) {
    for(i=0; methods[i]; i++)
      free(methods[i]);
    free(methods);
    methods=NULL;
  }

  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
