/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * codegen utility - Make C code from Flickr API by reflection
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

#include <flickcurl_cmd.h>



#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


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


#define GETOPT_STRING "d:hv"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"delay",   1, 0, 'd'},
  {"help",    0, 0, 'h'},
  {"version", 0, 0, 'v'},
  {NULL,      0, 0, 0}
};
#endif

#define IS_INT_ARG(arg) (!strcmp(arg->name, "per_page") || !strcmp(arg->name, "page"))
#define IS_OPTIONAL_ARG(arg) (!strcmp(arg->name, "per_page") || !strcmp(arg->name, "page"))

static const char *title_format_string = "Skeleton code generator utility %s\n";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc = NULL;
  int rc = 0;
  int usage = 0;
  int help = 0;
  int read_auth = 1;
  int i;
  int request_delay= -1;
  char section[50];
  size_t section_len;
  char** methods = NULL;
  
  flickcurl_init();
  flickcurl_cmdline_init();
  
  program = flickcurl_cmdline_basename(argv[0]);

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
        
      case 'h':
        help = 1;
        break;

      case 'v':
        fputs(flickcurl_version_string, stdout);
        fputc('\n', stdout);

        flickcurl_cmdline_finish();
        exit(0);
    }
    
  }
  
  if(help)
    goto help;
  
  if(argc < 2) {
    fprintf(stderr, "%s: No API section given\n", program);
    usage = 1;
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
    rc = 1;
    goto tidy;
  }

  help:
  if(help) {
    printf(title_format_string, flickcurl_version_string);
    puts("Generate C code from Flickr API by reflection.");
    printf("Usage: %s [OPTIONS] command args...\n\n", program);

    fputs(flickcurl_copyright_string, stdout);
    fputs("\nLicense: ", stdout);
    puts(flickcurl_license_string);
    fputs("Flickcurl home page: ", stdout);
    puts(flickcurl_home_url_string);

    fputs("\n", stdout);

    puts(HELP_TEXT("d", "delay DELAY     ", "Set delay between requests in milliseconds"));
    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

    rc = 0;
    goto tidy;
  }


  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    rc = 1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(read_auth && !access((const char*)flickcurl_cmdline_config_path, R_OK)) {
    if(flickcurl_config_read_ini(fc, flickcurl_cmdline_config_path,
                                 flickcurl_cmdline_config_section, fc,
                                 flickcurl_config_var_handler)) {
      rc = 1;
      goto tidy;
    }
  }

  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);

  strcpy(section, "flickr.");
  strcpy(section+7, argv[1]);
  
  /* allow old format commands to work */
  for(i = 0; section[i]; i++) {
    if(section[i] == '-')
      section[i] = '.';
  }
  
  if(!strncmp(section, "flickr.flickr", 13))
    strcpy(section, section+7);
  section_len = strlen(section);
  
  fprintf(stderr, "%s: section '%s'\n", program, section);
    
  methods = flickcurl_reflection_getMethods(fc);
  if(!methods) {
    fprintf(stderr, "%s: getMethods failed\n", program);
    rc = 1;
    goto tidy;
  }

  fprintf(stdout,
"/* -*- Mode: c; c-basic-offset: 2 -*-\n"
" *\n"
" * %s-api.c - Flickr %s.* API calls\n"
" *\n",
          argv[1], section);

  fprintf(stdout,
" * Copyright (C) 2010-2012, David Beckett http://www.dajobe.org/\n"
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

  
  for(i = 0; methods[i]; i++) {
    flickcurl_method* method;
    char* method_name;
    char function_name[100];
    int c, j;
    int is_write = 0;
    
    if(strncmp(methods[i], section, section_len))
      continue;
    
    method = flickcurl_reflection_getMethodInfo(fc, methods[i]);
    if(!method) {
      fprintf(stderr, "%s: getMethodInfo(%s) failed\n", program, methods[i]);
      rc = 1;
      break;
    }

    method_name = method->name;

    if(
      strstr(method_name, ".add") ||
      strstr(method_name, ".create") ||
      strstr(method_name, ".delete") ||
      strstr(method_name, ".edit") ||
      strstr(method_name, ".remove") ||
      strstr(method_name, ".set")
       )
      is_write = 1;
    
    strcpy(function_name, "flickcurl_");
    for(j = 0; (c = methods[i][j+7]); j++) {
      if(c == '.')
        c = '_';
      function_name[j+10] = c;
    }
    function_name[j+10] = '\0';

    fprintf(stdout, "/**\n * %s:\n", function_name);

    /* fixed arguments */
    fprintf(stdout, " * @fc: flickcurl context\n");

    if(method->args_count) {
      int argi;
      for(argi = 0; method->args[argi]; argi++) {
        flickcurl_arg* arg = method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;

        fprintf(stdout, " * @%s: %s", arg->name, arg->description);
        if(arg->optional) {
          fputs((IS_INT_ARG(arg)? " (or < 0)" : " (or NULL)"), stdout);
        }
        fputc('\n', stdout);

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
      for(argi = 0; method->args[argi]; argi++) {
        flickcurl_arg* arg = method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;
        
        if(IS_INT_ARG(arg)) {
          fprintf(stdout, ", int %s", arg->name);
        } else {
          fprintf(stdout, ", const char* %s", arg->name);
        }
      }
    }
    fprintf(stdout, ")\n{\n");

    fprintf(stdout,
"  xmlDocPtr doc = NULL;\n"
"  xmlXPathContextPtr xpathCtx = NULL; \n"
"  void* result = NULL;\n");
    if(method->args_count) {
      int argi;
      for(argi = 0; method->args[argi]; argi++) {
        flickcurl_arg* arg = method->args[argi];
        if(IS_INT_ARG(arg)) {
          fprintf(stdout,
"  char %s_str[10];\n",
                  arg->name);
        }
      }
    }

    fputs(
"\n"
"  flickcurl_init_params(fc, 0);\n"
"\n",
    stdout);
    
    
    if(method->args_count) {
      int argi;
      int print_or = 0;
      
      fprintf(stdout, "  if(");
      for(argi = 0; method->args[argi]; argi++) {
        flickcurl_arg* arg = method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;
        
        if(!arg->optional) {
          if(print_or)
            fprintf(stdout, " || ");
          
          fprintf(stdout, "!%s", arg->name);
          print_or = 1;
        }
      }
      fprintf(stdout,
")\n"
"    return 1;\n"
"\n");
    }


    if(method->args_count) {
      int argi;
      for(argi = 0; method->args[argi]; argi++) {
        flickcurl_arg* arg = method->args[argi];
        if(!strcmp(arg->name, "api_key"))
          continue;
        
        if(arg->optional) {
          if(IS_INT_ARG(arg)) {
            fprintf(stdout,
"  if(%s >= 0) {\n"
"    sprintf(%s_str, \"%%d\", %s);\n"
"    flickcurl_add_param(fc, \"%s\", %s_str);\n"
"  }\n",
                    arg->name, arg->name, arg->name, arg->name, arg->name);
            continue;
          }
          fprintf(stdout,
"  if(%s)\n",
                  arg->name);
        }
        
        fprintf(stdout,
"  flickcurl_add_param(fc, \"%s\", %s);\n",
              arg->name, arg->name);

      }
    }

    fprintf(stdout,
"\n"
"  flickcurl_end_params(fc);\n"
"\n"
"  if(flickcurl_prepare(fc, \"%s\"))\n"
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
"  doc = flickcurl_invoke(fc);\n"
"  if(!doc)\n"
"    goto tidy;\n"
"\n"
"\n"
"  xpathCtx = xmlXPathNewContext(doc);\n"
"  if(!xpathCtx) {\n"
"    flickcurl_error(fc, \"Failed to create XPath context for document\");\n"
"    fc->failed = 1;\n"
"    goto tidy;\n"
"  }\n"
"\n"
"  result = NULL; /* your code here */\n"
"\n"
"  tidy:\n"
"  if(xpathCtx)\n"
"    xmlXPathFreeContext(xpathCtx);\n"
"\n"
"  if(fc->failed)\n"
"    result = NULL;\n"
"\n"
"  return (result == NULL);\n"
"}\n"
"\n"
"\n");

    flickcurl_free_method(method);
  }


 tidy:
  if(methods) {
    for(i = 0; methods[i]; i++)
      free(methods[i]);
    free(methods);
    methods = NULL;
  }

  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  flickcurl_cmdline_finish();

  return(rc);
}
