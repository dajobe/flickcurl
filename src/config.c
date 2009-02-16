/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * config.c - INI configuration file handling
 *
 * Copyright (C) 2007-2009, David Beckett http://www.dajobe.org/
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#include <flickcurl.h>


#undef CONFIG_DEBUG

/**
 * read_ini_config:
 * @filename: filename
 * @application: application key to look up
 * @user_data: user data pointer for handler
 * @handler: config variable handler function
 *
 * Read .INI config
 *
 * FIXME - where do you start.  This just needs a pile of error checking
 *
 * Return value: non-0 on failure
 */
int
read_ini_config(const char* filename, const char* application,
                void* user_data, set_config_var_handler handler)
{
  FILE* fh;
#define INI_BUF_SIZE 255
  char buf[INI_BUF_SIZE+1];
  int in_section = 0;
  int lineno = 1;
  
  if(access((const char*)filename, R_OK))
    return 1;
  
  fh=fopen(filename, "r");
  if(!fh)
    return 1;

  while(!feof(fh)) {
    size_t len;
    char *line;
    char *p;
    int warned = 0;
    int c = -1;
    int lastch = -1;

    for(line = buf, len = 0; !feof(fh); lastch = c) {
      c = fgetc(fh);
      if(c == '\n') {
        /* Line endings: \n or \r\n (\r is removed after this loop) */
        lineno++;
        break;
      } else if (lastch == '\r') {
        /* Line ending: \r - need to put back this character */
        ungetc(c, fh);
        line--;
        len--;

        lineno++;
        break;
      }

      if(len > INI_BUF_SIZE) {
        if(!warned++)
          fprintf(stderr, "read_ini_config(): line %d too long - truncated\n",
                  lineno);
        continue;
      }
      *line++ = c;
      len++;
    }
    *line = '\0';
    
    if(!len)
      continue;

#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 1 >>%s<<\n", line);
#endif
    
    /* remove leading spaces */
    for(line = buf; 
        *line && (*line == ' ' || *line == '\t');
        line++, len--)
      ;

    /* remove trailing \r, \n or \r\n */
    if(line[len-1] == '\n')
      line[(len--)-1] = '\0';

    if(line[len-1] == '\r')
      line[(len--)-1] = '\0';
    
#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 2 >>%s<<\n", line);
#endif

    /* skip if empty line or all white space OR starts with a comment */
    if(!*line || *line == '#')
      continue;
    
#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 3 >>%s<<\n", line);
#endif

    /* Wait for a line '['application']' */
    if(!in_section) {
      if(*line == '[' && line[len-1] == ']' &&
         (len-2) == strlen(application) &&
         !strncmp(line+1, application, len-2)
         )
        in_section = 1;
      continue;
    }

    /* End at a line starting with '[' */
    if(*line == '[')
      break;

#ifdef CONFIG_DEBUG    
    fprintf(stderr, "Line 4 >>%s<<\n", line);
#endif

    p = strchr(line, '=');
    if(p) {
      *p = '\0';
#ifdef CONFIG_DEBUG    
      fprintf(stderr, "Found key '%s' value '%s'\n", line, p+1);
#endif
      if(handler)
        handler(user_data, line, p+1);
    }
  }
  fclose(fh);

  return 0;
}
