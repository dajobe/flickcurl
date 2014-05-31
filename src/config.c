/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * config.c - INI configuration file handling
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
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif


#include <flickcurl.h>
#include <flickcurl_internal.h>


#undef CONFIG_DEBUG

/**
 * flickcurl_config_read_ini:
 * @fc: flickcurl config
 * @filename: filename
 * @section: section name to use
 * @user_data: user data pointer for handler (usually the @fc )
 * @handler: config variable handler function (usually flickcurl_config_var_handler() )
 *
 * Read flickcurl library configuration in .INI format from a filename
 *
 * Return value: non-0 on failure
 */
int
flickcurl_config_read_ini(flickcurl* fc,
                          const char* filename,
                          const char* section,
                          void* user_data, set_config_var_handler handler)
{
  FILE* fh;
#define INI_BUF_SIZE 255
  char buf[INI_BUF_SIZE+1];
  int in_section = 0;
  int lineno = 1;
  size_t section_len;

  if(!fc || !filename || !section || !handler)
    return 1;

  fh = fopen(filename, "r");
  if(!fh) {
    flickcurl_error(fc, "Failed to open %s for reading - %s", filename,
                    strerror(errno));
    return 1;
  }

  section_len = strlen(section);

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
          fprintf(stderr, 
                  "flickcurl_config_read_ini(): line %d too long - truncated\n",
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
         (len - 2) == section_len &&
         !strncmp(line + 1, section, len - 2)
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
      char *kchar;
      
      *p = '\0';

      /* remove trailing spaces at end of key */
      kchar = (p-1);
      while(kchar >= line && isspace(*kchar))
        *kchar-- = '\0';

      /* remove leading spaces at start of value */
      p++;
      while(*p && isspace(*p))
        p++;

#ifdef CONFIG_DEBUG    
      fprintf(stderr, "Found key '%s' value '%s'\n", line, p);
#endif
      if(handler)
        handler(user_data, line, p);
    }
  }
  fclose(fh);

  return 0;
}

/**
 * read_ini_config:
 * @filename: filename
 * @section: section name to use
 * @user_data: user data pointer for handler
 * @handler: config variable handler function
 *
 * @Deprecated for flickcurl_config_read_ini()
 * 
 * Read .INI config
 *
 * Return value: non-0 on failure
 */
int
read_ini_config(const char* filename, const char* section,
                void* user_data, set_config_var_handler handler)
{
  return flickcurl_config_read_ini(NULL, filename, section, user_data,
                                   handler);
}


/**
 * flickcurl_config_var_handler:
 * @userdata: user data pointing to #flickcurl object
 * @key: var key
 * @value: var value
 *
 * Standard handler for flickcurl_config_read_ini()
 *
 */
void
flickcurl_config_var_handler(void* userdata, 
                             const char* key, const char* value)
{
  flickcurl *fc = (flickcurl *)userdata;
  
  if(!strcmp(key, "api_key"))
    flickcurl_set_api_key(fc, value);
  else if(!strcmp(key, "secret"))
    flickcurl_set_shared_secret(fc, value);
  else if(!strcmp(key, "auth_token"))
    flickcurl_set_auth_token(fc, value);
  else if(!strcmp(key, "oauth_client_key"))
    flickcurl_set_oauth_client_key(fc, value);
  else if(!strcmp(key, "oauth_client_secret"))
    flickcurl_set_oauth_client_secret(fc, value);
  else if(!strcmp(key, "oauth_token"))
    flickcurl_set_oauth_token(fc, value);
  else if(!strcmp(key, "oauth_token_secret"))
    flickcurl_set_oauth_token_secret(fc, value);
}


/**
 * flickcurl_config_write_ini:
 * @fc: #flickcurl object
 * @filename: filename
 * @section: section name to use
 *
 * Write flickcurl library configuration in INI file format to the given file
 *
 * Return value: non-0 on failure
 *
 */
int
flickcurl_config_write_ini(flickcurl *fc,
                           const char* filename,
                           const char* section)
{
  const char* s;
  FILE *fh;
  
  if(!fc || !filename || !section)
    return 1;
  
  fh = fopen(filename, "w");
  if(!fh) {
    flickcurl_error(fc, "Failed to write to configuration file %s - %s",
                    filename, strerror(errno));
    return 1;
  }

  fputc('[', fh);
  fputs(section, fh);
  fputc(']', fh);

  s = flickcurl_get_oauth_token(fc);
  if(s) {
    /* OAuth token and secret */
    fputs("\noauth_token=", fh);
    fputs(s, fh);
    s = flickcurl_get_oauth_token_secret(fc);
    if(s) {
      fputs("\noauth_token_secret=", fh);
      fputs(s, fh);
    }
    s = flickcurl_get_oauth_client_key(fc);
    if(s) {
      fputs("\noauth_client_key=", fh);
      fputs(s, fh);
    }
    s = flickcurl_get_oauth_client_secret(fc);
    if(s) {
      fputs("\noauth_client_secret=", fh);
      fputs(s, fh);
    }
  } else {
    /* Legacy Flickr auth */
    s = flickcurl_get_auth_token(fc);
    if(s) {
      fputs("\noauth_token=", fh);
      fputs(s, fh);
    }
    s = flickcurl_get_shared_secret(fc);
    if(s) {
      fputs("\noauth_secret=", fh);
      fputs(s, fh);
    }
    s = flickcurl_get_api_key(fc);
    if(s) {
      fputs("\napi_key=", fh);
      fputs(s, fh);
    }
  }
  
  fputs("\n", fh);

  fclose(fh);

  return 0;
}
