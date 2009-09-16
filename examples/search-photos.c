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
 * USAGE: search-photos TAG
 *
 * This program is an example of how to use the search API to find
 * photos with Flickcurl.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <flickcurl.h>

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


static void
my_set_config_var_handler(void* userdata, const char* key, const char* value)
{
  flickcurl *fc = (flickcurl *)userdata;
  
  if(!strcmp(key, "api_key"))
    flickcurl_set_api_key(fc, value);
  else if(!strcmp(key, "secret"))
    flickcurl_set_shared_secret(fc, value);
  else if(!strcmp(key, "auth_token"))
    flickcurl_set_auth_token(fc, value);
}


static const char* config_filename = ".flickcurl.conf";
static const char* config_section = "flickr";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc = NULL;
  int rc = 0;
  const char* home;
  char config_path[1024];
  char* tag = NULL;
  flickcurl_photos_list_params list_params;
  flickcurl_search_params params;
  flickcurl_photos_list* photos_list = NULL;
  int i;
  
  program = my_basename(argv[0]);

  flickcurl_init();

  home = getenv("HOME");
  if(home)
    sprintf(config_path, "%s/%s", home, config_filename);
  else
    strcpy(config_path, config_filename);
  
  if(argc != 2) {
    fprintf(stderr, "%s: No tag given\n"
                    "Try `%s -h for more information.\n", program, program);
    rc = 1;
    goto tidy;
  }

  if(!strcmp(argv[1], "-h")) {
    printf("%s - search for my interesting photos about a tag\n"
           "Usage: %s TAG\n\n", program, program);
    
    fputs("Flickcurl home page: ", stdout);
    puts(flickcurl_home_url_string);
    puts(flickcurl_copyright_string);
    fputs("License: ", stdout);
    puts(flickcurl_license_string);
    rc = 1;
    goto tidy;
  }
  
  tag = argv[1];

  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    rc = 1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(!access((const char*)config_path, R_OK)) {
    if(read_ini_config(config_path, config_section, fc,
                       my_set_config_var_handler)) {
      fprintf(stderr, "%s: Failed to read config filename %s: %s\n",
              program, config_path, strerror(errno));
      rc = 1;
      goto tidy;
    }
  }
  

  /* Initialise the search parameters themselves
   *  user_id: "me" - Search only photos of the calling user.
   *  sort: "interestingness-desc" - return results with most interesting first
   *  tag: TAG - search for photos about the TAG given on the command line
   */
  flickcurl_search_params_init(&params);
  /* these strings are shared and not strdup()ed since they are stored 
   * in 'params" on the stack */
  params.user_id = (char*)"me";
  params.sort = (char*)"interestingness-desc";
  params.tags = tag;

  /* Initialise the search result (list) parameters:
   *   per_page: 10 - ten results per-page
   *   page: 1 - return 1 page
   *   extras: "original_format" - want the returned photos to have the
   *      original secret and format fields.
   */
  flickcurl_photos_list_params_init(&list_params);
  list_params.per_page = 10;
  list_params.page = 1;
  /* this string is shared and not strdup()ed since it is stored 
   * in 'list_params" on the stack */
  list_params.extras = "original_format";

  photos_list = flickcurl_photos_search_params(fc, &params, &list_params);
  if(!photos_list)
    goto tidy;

  fprintf(stderr, "%s: Search returned %d photos\n", 
          program, photos_list->photos_count);

  for(i = 0; i < photos_list->photos_count; ++i)
    printf("  Result #%d has ID %s\n", i, photos_list->photos[i]->id);
  
 tidy:
  if(photos_list)
    flickcurl_free_photos_list(photos_list);

  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
