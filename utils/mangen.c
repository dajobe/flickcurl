/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * mangen utility - Generate utility manpage
 *
 * Copyright (C) 2014, David Beckett http://www.dajobe.org/
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

#include <flickcurl.h>
#include <flickcurl_cmd.h>


/* These are exported for use by the commands.c module but mostly
 * unused (except for program) since we only read the commands array
 * and never invoke the commands
 */
int verbose = 1;
const char* program;
FILE* output_fh;
const char *output_filename = "fake";


/* Not using getopt here */
#define HELP_TEXT(short, long, description) "  -" short "  " description
#define HELP_TEXT_LONG(long, description)
#define HELP_ARG(short, long) "-" #short
#define HELP_PAD "\n      "


static const char *title_format_string = "Manpage code generator utility %s\n";


static int
mangen_cmd_compare(const void *a, const void *b)
{
  flickcurl_cmd* a_cmd = (flickcurl_cmd*)a;
  flickcurl_cmd* b_cmd = (flickcurl_cmd*)b;

  return strcmp(a_cmd->name, b_cmd->name);
}


static void
mangen_print_man_page(FILE* fh)
{
  int i;

  fputs(".LP\n"
        "In the following list of commands:\n"
        ".br\n"
        "\\fIPER-PAGE\\fR is photos per result page or '-' for default (10)\n"
        ".br\n"
        "\\fIPAGE\\fR is result page number or '-' for default (1 = first page)\n",
        fh);

  qsort(commands, FLICKCURL_CMD_COUNT-1,
        sizeof(flickcurl_cmd), mangen_cmd_compare);

  for(i = 0; commands[i].name; i++) {
    int d, dc, nl = 1, lastdc= -1;
    fprintf(fh, ".IP \"\\fB%s\\fP \\fI%s\\fP\"\n",
            commands[i].name, commands[i].args);
    for(d = 0; (dc = commands[i].description[d]); d++) {
      if(nl && dc == ' ') {
        lastdc = dc;
        continue;
      }

      if(dc == ' ' && lastdc == ' ') {
        fputs("\n.br\n", fh);
        do {
          d++;
          dc = commands[i].description[d];
        } while(dc == ' ');
      }

      nl = 0;
      if(dc == '\n') {
        fputs("\n.br\n", fh);
        nl = 1;
      } else
        fputc(dc, fh);
      lastdc = dc;
    }
    fputc('\n', fh);
  }

  fputs(".SH Extras Fields\n", fh);
  fputs("The \\fBEXTRAS\\fP parameter can take a comma-separated set of the following values\n", fh);
  for(i = 0; 1; i++) {
    const char* name;
    const char* label;

    if(flickcurl_get_extras_format_info(i, &name, &label))
      break;

    fprintf(fh, ".TP\n\\fB%s\\fP\n%s\n", name, label);
  }

  fputs(".SH Photos List Feed Formats\n", fh);
  fputs("The \\fBFORMAT\\fP parameter can take any of the following values\n", fh);
  for(i = 0; 1; i++) {
    const char* name;
    const char* label;

    if(flickcurl_get_feed_format_info(i, &name, &label, NULL))
      break;

    fprintf(fh, ".TP\n\\fB%s\\fP\n%s\n", name, label);
  }

}


static void
mangen_print_extras(FILE* fh)
{
  int i;

  fputs("<variablelist>\n", fh);

  for(i = 0; 1; i++) {
    const char* name;
    const char* label;

    if(flickcurl_get_extras_format_info(i, &name, &label))
      break;

    fprintf(fh,
            "  <varlistentry>\n"
            "    <term>%s</term>\n"
            "    <listitem><simpara>%s</simpara></listitem>\n"
            "  </varlistentry>\n", name, label);
  }

  fputs("</variablelist>\n", fh);
}


int
main(int argc, char *argv[])
{
  flickcurl *fc = NULL;
  int rc = 0;
  int usage = 0;
  int help = 0;
  int mode = 0;

  flickcurl_init();
  flickcurl_cmdline_init();

  program = flickcurl_cmdline_basename(argv[0]);

  if(argc > 1 && *argv[1] == '-') {
    if(!strcmp(argv[1], "-v")) {
      fputs(flickcurl_version_string, stdout);
      fputc('\n', stdout);

      flickcurl_cmdline_finish();
      exit(0);
    }

    if(!strcmp(argv[1], "-h"))
      help = 1;
    else {
      fprintf(stderr, "%s: Invalid option `%s'\n", program, argv[1]);
      usage = 1;
    }
    goto usage;
  }

  if(argc < 2) {
    usage = 2; /* Title and usage */
    goto usage;
  }

  if(!strcmp(argv[1], "manpage"))
    mode = 0;
  else if(!strcmp(argv[1], "extras"))
    mode = 1;
  else {
    fprintf(stderr, "%s: Invalid mode `%s'\n", program, argv[1]);
    usage = 1;
  }

 usage:
  if(usage) {
    if(usage > 1) {
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
    printf(title_format_string, flickcurl_version_string);
    puts("Generate manpage for Flickcurl utility.");
    printf("Usage: %s [OPTIONS] manual | extras\n\n", program);

    fputs(flickcurl_copyright_string, stdout);
    fputs("\nLicense: ", stdout);
    puts(flickcurl_license_string);
    fputs("Flickcurl home page: ", stdout);
    puts(flickcurl_home_url_string);

    fputs("\n", stdout);

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

  if(mode == 0) {
    /* mode 0 : man page */
    mangen_print_man_page(stdout);
  } else {
    /* mode 1: extras */
    mangen_print_extras(stdout);
  }

 tidy:
  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
