/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl command line utility functions
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <flickcurl.h>

#include <curl/curl.h>

#include <flickcurl_cmd.h>


/* private to this module */
#define CONFIG_FILENAME_LEN 15
const char* config_filename = ".flickcurl.conf";

/* external */
const char* flickcurl_cmdline_config_section = "flickr";
char* flickcurl_cmdline_config_path = NULL;


int
flickcurl_cmdline_init(void)
{
  size_t config_len = CONFIG_FILENAME_LEN;
  const char* home;

  if(flickcurl_cmdline_config_path)
    return 0;

  home = getenv("HOME");
  if(home)
    config_len += strlen(home) + 1;

  flickcurl_cmdline_config_path = malloc(config_len + 1);
  if(!flickcurl_cmdline_config_path)
    return 1;

  if(home)
    sprintf(flickcurl_cmdline_config_path, "%s/%s", home, config_filename);
  else
    memcpy(flickcurl_cmdline_config_path, config_filename, config_len + 1);

  return 0;
}



void
flickcurl_cmdline_finish(void)
{
  if(flickcurl_cmdline_config_path)
    free(flickcurl_cmdline_config_path);
}


const char*
flickcurl_cmdline_basename(const char *name)
{
  char *p;
  if((p = strrchr(name, '/')))
    name = p + 1;
  else if((p = strrchr(name, '\\')))
    name = p + 1;

  return name;
}
