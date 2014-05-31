/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl utility commands
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

typedef int (*command_handler)(flickcurl* fc, int argc, char *argv[]);

typedef struct {
  const char*     name;
  const char*     args;
  const char*     description;
  command_handler handler;
  int             min;
  int             max;
} flickcurl_cmd;

#define FLICKCURL_CMD_COUNT 184

extern int verbose;
extern FILE* output_fh;
extern const char *output_filename;
extern flickcurl_cmd commands[FLICKCURL_CMD_COUNT];

/* cmdline.c */
extern char* flickcurl_cmdline_config_path;
extern const char* flickcurl_cmdline_config_section;

int flickcurl_cmdline_init(void);
void flickcurl_cmdline_finish(void);
const char* flickcurl_cmdline_basename(const char *name);
