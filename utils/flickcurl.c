/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl utility - Invoke the Flickcurl library
 *
 * Copyright (C) 2007-2010, David Beckett http://www.dajobe.org/
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
 * auth_token=1234567-8901234567890123
 * api_key=0123456789abcdef0123456789abcdef
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


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static int verbose = 1;
static const char* program;
static FILE* output_fh;
static const char *output_filename = "<stdout>";


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

#ifdef FLICKCURL_MAINTAINER
#define GETOPT_STRING_MORE "m:"
#else
#define GETOPT_STRING_MORE
#endif

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"auth",    1, 0, 'a'},
  {"delay",   1, 0, 'd'},
  {"help",    0, 0, 'h'},
#ifdef FLICKCURL_MAINTAINER
  {"maintainer", 1, 0, 'm'},
#endif
  {"output",  0, 0, 'o'},
  {"quiet",   0, 0, 'q'},
  {"version", 0, 0, 'v'},
  {"verbose", 0, 0, 'V'},
  {NULL,      0, 0, 0}
};
#endif


typedef int (*command_handler)(flickcurl* fc, int argc, char *argv[]);


static int
command_test_echo(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_test_echo(fc, argv[1], argv[2]);
}


static int
command_test_login(flickcurl* fc, int argc, char *argv[])
{
  char* username;
  
  username = flickcurl_test_login(fc);
  if(username) {
    fprintf(stderr, "%s: Returned username '%s'\n", program, username);
    free(username);
  }
  
  return (username == NULL);
}


static int
command_test_null(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_test_null(fc);
}


static int
command_people_findByEmail(flickcurl* fc, int argc, char *argv[])
{
  char* nsid = NULL;
  char* email = argv[1];
  
  nsid = flickcurl_people_findByEmail(fc, email);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for user email %s\n", 
            program, nsid, email);
  
  return (nsid == NULL);
}


static int
command_people_findByUsername(flickcurl* fc, int argc, char *argv[])
{
  char* nsid = NULL;
  char* user_name = argv[1];
  
  nsid = flickcurl_people_findByUsername(fc, user_name);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for username %s\n", 
            program, nsid, user_name);
  
  return (nsid == NULL);
}


static void
command_print_person(flickcurl_person* person) 
{
  int i;
  
  fprintf(stderr, "Found person with ID %s\n", person->nsid);
  
  for(i = (int)PERSON_FIELD_FIRST; i <= (int)PERSON_FIELD_LAST; i++) {
    flickcurl_person_field_type field = (flickcurl_person_field_type)i;
    flickcurl_field_value_type datatype = person->fields[field].type;
    
    if(datatype == VALUE_TYPE_NONE)
      continue;
    
    fprintf(stderr, "field %s (%d) with %s value: '%s' / %d\n", 
            flickcurl_get_person_field_label(field), field,
            flickcurl_get_field_value_type_label(datatype),
            person->fields[field].string, person->fields[field].integer);
  }
}



static int
command_people_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_person* person;

  person = flickcurl_people_getInfo(fc, argv[1]);

  if(!person)
    return 1;
  
  command_print_person(person);

  flickcurl_free_person(person);
  
  return 0;
}


static void
command_print_tags(flickcurl_tag** tags, const char* label, const char* value)
{
  int i;
  if(!tags)
    return;
  
  if(label)
    fprintf(stderr, "%s: %s %s tags\n", program, label,
            (value ? value : "(none)"));
  else
    fprintf(stderr, "tags:\n");
  
  for(i = 0; tags[i]; i++) {
    flickcurl_tag* tag = tags[i];
    fprintf(stderr,
            "%d) %s tag: id %s author ID %s name %s raw '%s' cooked '%s' count %d\n",
            i, (tag->machine_tag ? "machine" : "regular"),
            tag->id, tag->author,
            (tag->authorname ? tag->authorname : "(Unknown)"),
            tag->raw, tag->cooked, tag->count);
  }
}


static int
command_print_location(flickcurl_location* location)
{
  const char* accuracy_label;
  accuracy_label = flickcurl_get_location_accuracy_label(location->accuracy);
  
  if(accuracy_label)
    fprintf(stderr, "latitude %f  longitude %f  accuracy %s(%d)\n",
            location->latitude, location->longitude, 
            accuracy_label, location->accuracy);
  else
    fprintf(stderr, "latitude %f  longitude %f  accuracy unknown\n",
            location->latitude, location->longitude);

  return 0;
}


static void
command_print_shape(flickcurl_shapedata* shape)
{
  fprintf(stderr,
          "created %d  alpha %2.2f  #points %d  #edges %d\n"
          "  is donuthole: %d  has donuthole: %d\n",
          shape->created, shape->alpha, shape->points, shape->edges,
          shape->is_donuthole, shape->has_donuthole);

  if(shape->data_length > 0) {
    int s;
#define MAX_XML 70
    s = (shape->data_length > MAX_XML ? MAX_XML : shape->data_length);
    fprintf(stderr, "  Shapedata (%d bytes):\n    ",
            (int)shape->data_length);
    fwrite(shape->data, 1, s, stderr);
    fputs("...\n", stderr);
  }

  if(shape->file_urls_count > 0) {
    int j;
    fprintf(stderr, "  Shapefile URLs: %d\n", shape->file_urls_count);
    for(j = 0; j < shape->file_urls_count; j++) {
      fprintf(stderr,"    URL %d: %s\n", j, shape->file_urls[j]);
    }
  }
}


static void
command_print_place(flickcurl_place* place,
                    const char* label, const char* value,
                    int print_locality)
{
  int i;
  if(label)
    fprintf(stderr, "%s: %s %s places\n", program, label,
            (value ? value : "(none)"));

  if(print_locality && place->type != FLICKCURL_PLACE_LOCATION)
    fprintf(stderr, "  Type %s (%d)\n",
            flickcurl_get_place_type_label(place->type), (int)place->type);
  
  if(place->location.accuracy != 0) {
    fputs("  Location: ", stderr);
    command_print_location(&place->location);
  }
  
  if(place->timezone)
    fprintf(stderr, "  Timezone: %s\n", place->timezone);

  if(place->shape)
    command_print_shape(place->shape);
  
  if(place->count >0)
    fprintf(stderr, "  Photos at Place: %d\n", place->count);
  
  for(i = (int)0; i <= (int)FLICKCURL_PLACE_LAST; i++) {
    char* name = place->names[i];
    char* id = place->ids[i];
    char* url = place->urls[i];
    char* woe_id = place->woe_ids[i];
    
    if(!name && !id && !url && !woe_id)
      continue;
    
    fprintf(stderr, "  %d) place %s:", i,
            flickcurl_get_place_type_label((flickcurl_place_type)i));
    if(name)
      fprintf(stderr," name '%s'", name);
    if(id)
      fprintf(stderr," id %s", id);
    if(woe_id)
      fprintf(stderr," woeid %s", woe_id);
    if(url)
      fprintf(stderr," url '%s'", url);
    fputc('\n', stderr);
  }

}


static void
command_print_video(flickcurl_video* v)
{
  fprintf(stderr,
          "video: ready %d  failed %d  pending %d  duration %d  width %d  height %d\n",
          v->ready, v->failed, v->pending, v->duration,
          v->width, v->height);
}


static void
command_print_notes(flickcurl_note** notes, const char* label,
                    const char* value)
{
  int i;

  if(!notes)
    return;
  
  if(label)
    fprintf(stderr, "%s: %s %s notes\n", program, label,
            (value ? value : "(none)"));
  else
    fprintf(stderr, "notes:\n");

  for(i = 0; notes[i]; i++) {
    flickcurl_note* note = notes[i];
    fprintf(stderr,
            "%d) id %d note: author ID %s name %s  x %d y %d w %d h %d text '%s'\n",
            i, note->id,
            note->author, (note->authorname ? note->authorname : "(Unknown)"),
            note->x, note->y, note->w, note->h,
            note->text);
  }
}


static void
command_print_photo(flickcurl_photo* photo)
{
  int i;
  
  fprintf(stderr, "%s with URI %s ID %s and %d tags\n",
          photo->media_type, 
          (photo->uri ? photo->uri : "(Unknown)"),
          photo->id, photo->tags_count);
  
  for(i = 0; i <= PHOTO_FIELD_LAST; i++) {
    flickcurl_photo_field_type field = (flickcurl_photo_field_type)i;
    flickcurl_field_value_type datatype = photo->fields[field].type;
    
    if(datatype == VALUE_TYPE_NONE)
      continue;
    
    fprintf(stderr, "    field %s (%d) with %s value: '%s' / %d\n", 
            flickcurl_get_photo_field_label(field), field,
            flickcurl_get_field_value_type_label(datatype),
            photo->fields[field].string, photo->fields[field].integer);
  }
  
  command_print_tags(photo->tags, NULL, NULL);

  if(photo->notes)
    command_print_notes(photo->notes, NULL, NULL);

  if(photo->place)
    command_print_place(photo->place, NULL, NULL, 1);

  if(photo->video)
    command_print_video(photo->video);
}


static int
command_photos_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_photo* photo;

  photo = flickcurl_photos_getInfo(fc, argv[1]);

  if(photo) {
    fprintf(stderr, "%s: ", program);
    command_print_photo(photo);
    flickcurl_free_photo(photo);
  }
  
  return (photo == NULL);
}


static int
command_photos_licenses_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_license** licenses;
  int i;
  
  licenses = flickcurl_photos_licenses_getInfo(fc);
  if(licenses) {

    if(verbose)
      fprintf(stderr, "%s: Found licenses\n", program);

    for(i = 0; licenses[i]; i++) {
      flickcurl_license* license = licenses[i];
      fprintf(stderr, "%d) license: id %d name '%s' url %s\n",
              i, license->id, license->name, 
              license->url ? license->url : "(none)");
      
    }
  }
  
  return (licenses == NULL);
}


static int
command_urls_lookupUser(flickcurl* fc, int argc, char *argv[])
{
  char* nsid = NULL;
  char* url = argv[1];
  
  nsid = flickcurl_urls_lookupUser(fc, url);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for user profile/photo URL %s\n", 
            program, nsid, url);
  
  return (nsid != NULL);
}


static void
command_contexts_print(FILE* fh, flickcurl_context** contexts)
{
  flickcurl_context* context;
  int i;
  
  for(i = 0; (context = contexts[i]); i++) {
    const char* label = flickcurl_get_context_type_field_label(context->type);
    fprintf(fh, "%d) context type '%s' id %s secret %s server %d farm %d\n  title: %s\n  url: %s\n  thumb: %s\n",
            i, label, 
            context->id,
            (context->secret ? context->secret : "NULL"),
            context->server, context->farm,
            (context->title ? context->title : "NULL"),
            (context->url ? context->url : "NULL"),
            (context->thumb ? context->thumb : "NULL")
            );
  }
}


static int
command_groups_pools_getContext(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_context** contexts;

  contexts = flickcurl_groups_pools_getContext(fc, argv[1], argv[2]);
  if(!contexts)
    return 1;
  if(verbose)
    fprintf(stderr, "%s: Pool context of photo %s in pool %s:\n", program,
            argv[1], argv[2]);
  command_contexts_print(stderr, contexts);
  
  flickcurl_free_contexts(contexts);

  return 0;
}

static int
command_photos_getAllContexts(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_context** contexts;
  
  contexts = flickcurl_photos_getAllContexts(fc, argv[1]);
  if(!contexts)
    return 1;
  if(verbose)
    fprintf(stderr, "%s: Photos %s all contexts:\n", program, argv[1]);
  command_contexts_print(stderr, contexts);
  
  flickcurl_free_contexts(contexts);

  return 0;
}

static int
command_photos_getContext(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_context** contexts;

  contexts = flickcurl_photos_getContext(fc, argv[1]);
  if(!contexts)
    return 1;
  if(verbose)
    fprintf(stderr, "%s: Photos %s context:\n", program, argv[1]);
  command_contexts_print(stderr, contexts);
  
  flickcurl_free_contexts(contexts);
  return 0;
}


static int
command_photos_getCounts(flickcurl* fc, int argc, char *argv[])
{
  char** dates_array = NULL;
  char** taken_dates_array = NULL;
  int** counts;
  
  if(argv[1]) {
    dates_array = flickcurl_array_split(argv[1], ',');
    if(argv[2])
      taken_dates_array = flickcurl_array_split(argv[2], ',');
  }

  counts = flickcurl_photos_getCounts(fc, (const char**)dates_array,
                                    (const char**)taken_dates_array);
  if(counts) {
    int i;
    
    for(i = 0; counts[i]; i++) {
      fprintf(stderr, "%s: photocount %i: count %d  fromdate %d  todate %d\n",
              program, i, counts[i][0], counts[i][1], counts[i][2]);
    }
    free(counts);
  }
  if(dates_array)
    flickcurl_array_free(dates_array);
  if(taken_dates_array)
    flickcurl_array_free(taken_dates_array);

  return (counts == NULL);
}


static int
command_photosets_getContext(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_context** contexts;

  contexts = flickcurl_photosets_getContext(fc, argv[1], argv[2]);
  if(!contexts)
    return 1;
  if(verbose)
    fprintf(stderr, "%s: Photo %s in photoset %s context:\n", program,
            argv[1], argv[2]);
  command_contexts_print(stderr, contexts);
  
  flickcurl_free_contexts(contexts);
  return 0;
}


static int
command_auth_getFrob(flickcurl* fc, int argc, char *argv[])
{
  char* frob;

  frob = flickcurl_auth_getFrob(fc);
  if(!frob)
    return 1;
  fprintf(stderr, "%s: Got frob: %s\n", program, frob);

  free(frob);
  return 0;
}


static int
command_auth_checkToken(flickcurl* fc, int argc, char *argv[])
{
  char* perms;

  perms = flickcurl_auth_checkToken(fc, argv[1]);
  if(!perms)
    return 1;
  fprintf(stderr, "%s: Checked token %s and got perms: %s\n", program, 
          argv[1], perms);

  free(perms);
  return 0;
}


static int
command_auth_getToken(flickcurl* fc, int argc, char *argv[])
{
  char* perms;

  perms = flickcurl_auth_getToken(fc, argv[1]);
  if(!perms)
    return 1;
  fprintf(stderr, "%s: Got token %s perms: %s\n", program, argv[1], perms);

  free(perms);
  return 0;
}


static int
command_auth_getFullToken(flickcurl* fc, int argc, char *argv[])
{
  char* perms;

  perms = flickcurl_auth_getFullToken(fc, argv[1]);
  if(!perms)
    return 1;
  fprintf(stderr, "%s: Got full token %s perms: %s\n", program, argv[1], perms);

  free(perms);
  return 0;
}


static int
command_tags_getListPhoto(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag** tags;
  char *photo_id = argv[1];

  tags = flickcurl_tags_getListPhoto(fc, photo_id);
  if(!tags)
    return 1;

  command_print_tags(tags, "Photo ID", photo_id);
  free(tags);

  return 0;
}


static int
command_tags_getListUser(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag** tags;
  char *user_id = argv[1];

  tags = flickcurl_tags_getListUser(fc, user_id);
  if(!tags)
    return 1;

  command_print_tags(tags, "User ID", user_id);
  free(tags);
  return 0;
}


static int
command_tags_getListUserPopular(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag** tags;
  char *user_id = NULL;
  int pop_count= -1;
  
  if(argv[1]) {
    user_id = argv[1];
    if(argv[2])
      pop_count = atoi(argv[2]);
  }
  
  tags = flickcurl_tags_getListUserPopular(fc, user_id, pop_count);
  if(!tags)
    return 1;

  command_print_tags(tags, "User ID", user_id);
  free(tags);
  return 0;
}


static int
command_tags_getListUserRaw(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag** tags;
  char *tag = argv[1];

  tags = flickcurl_tags_getListUserRaw(fc, tag);
  if(!tags)
    return 1;

  command_print_tags(tags, "Tag", tag);
  free(tags);
  return 0;
}


static int
command_tags_getRelated(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag** tags;
  char *tag = argv[1];

  tags = flickcurl_tags_getRelated(fc, tag);
  if(!tags)
    return 1;

  command_print_tags(tags, "Related to Tag", tag);
  free(tags);
  return 0;
}


static int
command_urls_getGroup(flickcurl* fc, int argc, char *argv[])
{
  char* nsid = NULL;
  char* url = argv[1];
  
  nsid = flickcurl_urls_getGroup(fc, url);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for group profile/photo URL %s\n", 
            program, nsid, url);
  
  return (nsid == NULL);
}


static int
command_urls_getUserPhotos(flickcurl* fc, int argc, char *argv[])
{
  char* url = NULL;
  char* user = argv[1];
  
  url = flickcurl_urls_getUserPhotos(fc, user);

  if(url)
    fprintf(stderr, "%s: photo URL %s for user %s\n", 
            program, url, user);
  
  return (url == NULL);
}


static int
command_urls_getUserProfile(flickcurl* fc, int argc, char *argv[])
{
  char* url = NULL;
  char* user = argv[1];
  
  url = flickcurl_urls_getUserProfile(fc, user);

  if(url)
    fprintf(stderr, "%s: photo URL %s for user %s\n", 
            program, url, user);
  
  return (url == NULL);
}


static int
command_urls_lookupGroup(flickcurl* fc, int argc, char *argv[])
{
  char* nsid = NULL;
  char* url = argv[1];
  
  nsid = flickcurl_urls_lookupGroup(fc, url);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for group profile/photo URL %s\n", 
            program, nsid, url);
  
  return (url == NULL);
}


static int
command_tags_getHotList(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag** tags;
  char *period = NULL;
  int count= -1;

  if(argv[1]) {
    period = argv[1];
    if(argv[2])
      count = atoi(argv[2]);
  }

  tags = flickcurl_tags_getHotList(fc, period, count);
  if(!tags)
    return 1;

  command_print_tags(tags, "Hot tags for period", 
                     (period ? period : "day"));
  free(tags);
  return 0;
}


static int
command_photos_addTags(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  const char *tags = argv[2];

  return flickcurl_photos_addTags(fc, photo_id, tags);
}


static int
command_photos_delete(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];

  return flickcurl_photos_delete(fc, photo_id);
}


static int
command_photos_removeTag(flickcurl* fc, int argc, char *argv[])
{
  const char *tag_id = argv[1];

  return flickcurl_photos_removeTag(fc, tag_id);
}


static int
command_photos_setTags(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  const char *tags = argv[2];

  return flickcurl_photos_setTags(fc, photo_id, tags);
}


static int
command_reflection_getMethodInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_method* method;

  method = flickcurl_reflection_getMethodInfo(fc, argv[1]);

  if(method) {
    fprintf(stderr, "%s: Found method %s\n", program, method->name);
    fprintf(stderr, "  Needs Login? %s\n", (method->needslogin? "yes" : "no"));
    fprintf(stderr, "  Description: %s\n", method->description);
    fprintf(stderr, "  Response: '%s'\n", method->response);
    fprintf(stderr, "  Explanation of Response: %s\n", 
            method->explanation ? method->explanation : "(None)");

    if(method->args_count) {
      int i;
      
      fprintf(stderr, "%s: %d argument%s:\n", program, method->args_count,
              ((method->args_count != 1) ? "s" : ""));
    
      for(i = 0; method->args[i]; i++) {
        flickcurl_arg* arg = method->args[i];
        fprintf(stderr, "%d) argument '%s' %s description: '%s'\n",
                i, arg->name, (arg->optional? "" : "(required)"),
                arg->description);
      }
    } else
      fprintf(stderr, "%s: No arguments\n", program);


    flickcurl_free_method(method);
  }
  return (method == NULL);
}


static int
command_reflection_getMethods(flickcurl* fc, int argc, char *argv[])
{
  char** methods;
  
  methods = flickcurl_reflection_getMethods(fc);
  if(methods) {
    int i;
    fprintf(stderr, "%s: Found methods:\n", program);
    for(i = 0; methods[i]; i++)
      printf("%d) %s\n", i, methods[i]);

    for(i = 0; methods[i]; i++)
      free(methods[i]);
    free(methods);
  }
  
  return (methods == NULL);
}

static int
command_photos_comments_addComment(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  const char *comment_text = argv[2];
  char* id;
  
  id = flickcurl_photos_comments_addComment(fc, photo_id, comment_text);
  if(id) {
    fprintf(stderr,
            "%s: Added comment '%s' to photo %s giving comment ID %s\n", 
            program, photo_id, comment_text, id);
  }
  
  return (id == NULL);
}

static int
command_photos_comments_deleteComment(flickcurl* fc, int argc, char *argv[])
{
  const char *comment_id = argv[1];

  return flickcurl_photos_comments_deleteComment(fc, comment_id);
}


static int
command_photos_comments_editComment(flickcurl* fc, int argc, char *argv[])
{
  const char *comment_id = argv[1];
  const char *comment_text = argv[2];

  return flickcurl_photos_comments_editComment(fc, comment_id, comment_text);
}


static void
command_print_comments(flickcurl_comment** comments, const char* label,
                       const char* value)
{
  int i;
  if(label)
    fprintf(stderr, "%s: %s %s comments\n", program, label,
            (value ? value : "(none)"));
  for(i = 0; comments[i]; i++) {
    flickcurl_comment* comment_object = comments[i];
    fprintf(stderr,
            "%d) ID %s author %s authorname %s datecreate %d permalink %s text '%s'\n",
            i, comment_object->id, comment_object->author,
            comment_object->authorname, comment_object->datecreate,
            comment_object->permalink, comment_object->text);
  }
}


static int
command_photos_comments_getList(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  flickcurl_comment** comments;
  
  comments = flickcurl_photos_comments_getList(fc, photo_id);
  if(!comments)
    return 1;
  
  command_print_comments(comments, "Photo ID", photo_id);

  flickcurl_free_comments(comments);
  return 0;
}


static int
command_photosets_comments_addComment(flickcurl* fc, int argc, char *argv[])
{
  const char *photoset_id = argv[1];
  const char *comment_text = argv[2];
  char* id;
  
  id = flickcurl_photosets_comments_addComment(fc, photoset_id, comment_text);
  if(id) {
    fprintf(stderr,
            "%s: Added comment '%s' to photoset %s giving comment ID %s\n", 
            program, photoset_id, comment_text, id);
    free(id);
  }
  
  return (id == NULL);
}

static int
command_photosets_comments_deleteComment(flickcurl* fc, int argc, char *argv[])
{
  const char *comment_id = argv[1];

  return flickcurl_photosets_comments_deleteComment(fc, comment_id);
}


static int
command_photosets_comments_editComment(flickcurl* fc, int argc, char *argv[])
{
  const char *comment_id = argv[1];
  const char *comment_text = argv[2];

  return flickcurl_photosets_comments_editComment(fc, comment_id, comment_text);
}


static int
command_photosets_comments_getList(flickcurl* fc, int argc, char *argv[])
{
  const char *photoset_id = argv[1];
  flickcurl_comment** comments;
  
  comments = flickcurl_photosets_comments_getList(fc, photoset_id);
  if(!comments)
    return 1;
  
  command_print_comments(comments, "Photoset ID", photoset_id);

  flickcurl_free_comments(comments);
  return 0;
}


static void
print_upload_status(FILE* handle, flickcurl_upload_status* status,
                    const char* label)
{
  if(label)
    fprintf(handle, "%s: %s status\n", program, label);
  if(status->photoid)
    fprintf(handle, "  Photo ID: %s\n", status->photoid);
  if(status->secret)
    fprintf(handle, "  Secret: %s\n", status->secret);
  if(status->originalsecret)
    fprintf(handle, "  Original Secret: %s\n", status->originalsecret);
  if(status->ticketid)
    fprintf(handle, "  Ticket ID: %s\n", status->ticketid);
}


static const char* yn_strings[2] = {"no", "yes"};

static const char*
yesno(int v) 
{
  return yn_strings[(v ? 1 : 0)];
}


static void
print_upload_params(FILE* handle, flickcurl_upload_params* params,
                    const char* label)
{
  if(label)
    fprintf(handle, "%s: %s\n", program, label);

  fprintf(handle, "  File: %s\n", params->photo_file);
  if(params->title)
    fprintf(handle, "  Title: '%s'\n", params->title);
  else
    fprintf(handle, "  Title: none\n");
  if(params->description)
    fprintf(handle, "  Description: '%s'\n", params->description);
  else
    fprintf(handle, "  Description: none\n");
  fprintf(handle, "  Tags: %s\n", (params->tags ? params->tags : ""));
  fprintf(handle, "  Viewable by Public: %s  Friends: %s  Family: %s\n",
          yesno(params->is_public), yesno(params->is_friend),
          yesno(params->is_family));
  fprintf(handle, "  Safety level: %s (%d)\n",
          flickcurl_get_safety_level_label(params->safety_level),
          params->safety_level);
  fprintf(handle, "  Content type: %s (%d)\n",
          flickcurl_get_content_type_label(params->content_type),
          params->content_type);
}


static int
command_upload(flickcurl* fc, int argc, char *argv[])
{
  char *tags_string = NULL;
  flickcurl_upload_status* status = NULL;
  int usage = 0;
  flickcurl_upload_params params;
  
  memset(&params, '\0', sizeof(flickcurl_upload_params));
  params.safety_level = 1; /* default safe */
  params.content_type = 1; /* default photo */
  
  
  argv++; argc--;
  params.photo_file = argv[0];

  if(access((const char*)params.photo_file, R_OK)) {
    fprintf(stderr, "%s: Failed to read image filename '%s': %s\n",
            program, params.photo_file, strerror(errno));
    status = NULL;
    goto tidy;
  }

  argv++; argc--;
  while(!usage && argc) {
    char* field = argv[0];
    argv++; argc--;

    if(!strcmp(field, "description")) {
      params.description = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "title")) {
      params.title = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "safety_level")) {
      params.safety_level = flickcurl_get_safety_level_from_string(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "content_type")) {
      params.content_type = flickcurl_get_content_type_from_string(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "friend")) {
      params.is_friend = 1;
    } else if(!strcmp(field, "family")) {
      params.is_family = 1;
    } else if(!strcmp(field, "public")) {
      params.is_public = 1;
    } else if(!strcmp(field, "tags")) {
      size_t tags_len = 0;
      int i;
      char *p;

      /* tags absorb all remaining parameters */
      for(i = 0; i<argc; i++)
        tags_len+= strlen(argv[i])+1;
      tags_string = (char*)malloc(tags_len);
      
      p = tags_string;
      for(i = 0; i<argc; i++) {
        size_t tag_len = strlen(argv[i]);
        strncpy(p, argv[i], tag_len); p+= tag_len;
        *p++= ' ';
      }
      *(--p) = '\0';
      
      params.tags = tags_string;
      break;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, field);
      usage = 1;
    }
  }

  if(usage) {
    status = NULL;
    goto tidy;
  }

  if(verbose == 1)
    fprintf(stderr, "%s: Uploading file %s\n", program, params.photo_file);
  else if (verbose > 1)
    print_upload_params(stderr, &params, "Photo upload");

  status = flickcurl_photos_upload_params(fc, &params);
  if(status) {
    print_upload_status(stderr, status, "Photo upload");

    flickcurl_free_upload_status(status);
  }

  tidy:
  if(params.tags)
    free((char*)params.tags);
  
  return (status == NULL);
}


static int
command_replace(flickcurl* fc, int argc, char *argv[])
{
  const char *file = argv[1];
  const char *photo_id = argv[2];
  int async = 0;
  flickcurl_upload_status* status = NULL;
  
  if(access((const char*)file, R_OK)) {
    fprintf(stderr, "%s: Failed to read image filename '%s': %s\n",
            program, file, strerror(errno));
    status = NULL;
    goto tidy;
  }
  
  if(argc > 3 && !strcmp(argv[3], "async"))
    async = 1;

  status = flickcurl_photos_replace(fc, file, photo_id, async);
  if(status) {
    print_upload_status(stderr, status, "Photo replace");

    flickcurl_free_upload_status(status);
  }
  
  tidy:
  
  return (status == NULL);
}


static int
command_photos_setContentType(flickcurl* fc, int argc, char *argv[])
{
  const char* photo_id = argv[1];
  const char* content_type_str = argv[2];
  int content_type;

  content_type = flickcurl_get_content_type_from_string(content_type_str);
  if(content_type < 0) {
    fprintf(stderr, "%s: Bad content type '%s'\n", program, content_type_str);
    return 1;
  }

  content_type_str = flickcurl_get_content_type_label(content_type);
  if(verbose)
    fprintf(stderr, "%s: Setting photo %s to content type %d (%s)\n",
            program, photo_id, content_type, content_type_str);
  
  return flickcurl_photos_setContentType(fc, photo_id, content_type);
}


static int
command_photos_setDates(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  int date_posted= -1;
  int date_taken= -1;
  int date_taken_granularity= -1;

  date_posted = curl_getdate(argv[2], NULL);
  date_taken = curl_getdate(argv[3], NULL);
  date_taken_granularity = atoi(argv[4]);
  
  return flickcurl_photos_setDates(fc, photo_id, date_posted, date_taken,
                                   date_taken_granularity);
}


static int
command_photos_setMeta(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  const char* title = argv[2];
  const char* description = argv[3];
  
  return flickcurl_photos_setMeta(fc, photo_id, title, description);
}


static int
parse_bool_param(const char* param) 
{
  char *eptr;
  int b;
  
  if(!param ||
     (!strcmp(param, "false") || !strcmp(param, "no")))
    return 0;

  if(!strcmp(param, "true") || !strcmp(param, "yes"))
    return 1;

  eptr = NULL;
  b = (int)strtol(param, &eptr, 10);
  if(*eptr)
    return 0;

  return (b != 0);
}


static int
command_photos_setPerms(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  int is_public = parse_bool_param(argv[2]);
  int is_friend = parse_bool_param(argv[3]);
  int is_family = parse_bool_param(argv[4]);
  int perm_comment = atoi(argv[5]);
  int perm_addmeta = atoi(argv[6]);
  flickcurl_perms perms;

  memset(&perms, '\0', sizeof(flickcurl_perms));
  perms.is_public = is_public;
  perms.is_friend = is_friend;
  perms.is_family = is_family;
  perms.perm_comment = perm_comment;
  perms.perm_addmeta = perm_addmeta;

  return flickcurl_photos_setPerms(fc, photo_id, &perms);
}


static int
command_photos_setSafetyLevel(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  const char* safety_level_str = argv[2];
  int hidden = parse_bool_param(argv[3]);
  int safety_level;

  safety_level = flickcurl_get_safety_level_from_string(safety_level_str);
  if(safety_level < 0) {
    fprintf(stderr, "%s: Bad safety level '%s'\n", program, safety_level_str);
    return 1;
  }

  safety_level_str = flickcurl_get_safety_level_label(safety_level);
  if(verbose)
    fprintf(stderr, "%s: Setting photo %s safety level to %d (%s), hidden %d\n",
            program, photo_id, safety_level, safety_level_str, hidden);
  
  return flickcurl_photos_setSafetyLevel(fc, photo_id, safety_level, hidden);
}


static void
command_print_perms(flickcurl_perms* perms, int show_comment_metadata)
{
  static const char* perms_labels[4] = {"nobody", "friends and family", "contacts", "everybody" };

  fprintf(stderr,
          "view perms: public: %s  contact: %s  friend: %s  family: %s\n",
          yesno(perms->is_public), yesno(perms->is_contact),
          yesno(perms->is_friend), yesno(perms->is_family));

#define PERM_LABEL(x) (((x) >= 0 && (x) <= 3) ? perms_labels[(x)] : "?")
  if(show_comment_metadata)
    fprintf(stderr,
            "add comment: %s\nadd metadata: %s\n",
            PERM_LABEL(perms->perm_comment), PERM_LABEL(perms->perm_addmeta));
}


static int
command_photos_getPerms(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  flickcurl_perms* perms;

  perms = flickcurl_photos_getPerms(fc, photo_id);
  if(!perms)
    return 1;

  fprintf(stderr, "%s: Photo ID %s permissions\n", program, photo_id);
  command_print_perms(perms, 1);

  flickcurl_free_perms(perms);
  return 0;
}


static int
command_print_photos_list(flickcurl* fc, flickcurl_photos_list* photos_list,
                          FILE* fh, const char* label)
{
  int rc = 0;
  int i;
  
  if(photos_list->photos) {
    fprintf(stderr,
            "%s: %s returned %d photos out of %d, page %d per-page %d\n",
            program, label,
            photos_list->photos_count, photos_list->total_count,
            photos_list->page, photos_list->per_page);
    for(i = 0; photos_list->photos[i]; i++) {
      fprintf(stderr, "%s: %s photo %d\n", program, label, i);
      command_print_photo(photos_list->photos[i]);
    }
  } else if(photos_list->content) {
    size_t write_count;

    if(verbose)
      fprintf(stderr, "%s: %s returned %d bytes of %s content\n", 
              program, label,
              (int)photos_list->content_length, photos_list->format);
    write_count = fwrite(photos_list->content, 1, photos_list->content_length, fh);
    if(write_count < photos_list->content_length) {
      fprintf(stderr, "%s: writing to %s failed\n", program, output_filename);
      rc = 1;
    }
  } else {
    fprintf(stderr, "%s: %s returned neither photos nor raw content\n", 
            program, label);
    rc = 1;
  }

  return rc;
}


static int
command_photos_getContactsPhotos(flickcurl* fc, int argc, char *argv[])
{
  int contact_count = 10;
  int just_friends = 0;
  int single_photo = 1;
  int include_self = 0;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >1) {
    list_params.extras = argv[1];
    if(argc >2)
      list_params.format = argv[2];
  }

  photos_list = flickcurl_photos_getContactsPhotos_params(fc, contact_count,
                                                        just_friends,
                                                        single_photo,
                                                        include_self,
                                                        &list_params);
  if(!photos_list)
    return 1;

  rc = command_print_photos_list(fc, photos_list, output_fh, "Contact photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}


static int
parse_page_param(const char* param) 
{
  char *eptr;
  int i;
  
  if(!param || *param == '-')
    return -1;
  
  eptr = NULL;
  i = (int)strtol(param, &eptr, 10);
  if(*eptr)
    return -1;

  return i;
}


static int
command_photos_search(flickcurl* fc, int argc, char *argv[])
{
  char *tags_string = NULL;
  int usage = 0;
  flickcurl_photos_list_params list_params;
  flickcurl_search_params params;
  flickcurl_photos_list* photos_list = NULL;
  
  flickcurl_photos_list_params_init(&list_params);

  flickcurl_search_params_init(&params);

  argv++; argc--;
  while(!usage && argc) {
    char* field = argv[0];
    argv++; argc--;

    if(!strcmp(field, "user")) {
      params.user_id = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "tag-mode")) {
      /* "any" or "all" */
      params.tag_mode = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "text")) {
      params.text = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "min-upload-date")) {
      /* timestamp */
      params.min_upload_date = curl_getdate(argv[0], NULL);
      argv++; argc--;
    } else if(!strcmp(field, "max-upload-date")) {
      /* timestamp */
      params.max_upload_date = curl_getdate(argv[0], NULL);
      argv++; argc--;
    } else if(!strcmp(field, "min-taken-date")) {
      /* MYSQL datetime */
      params.min_taken_date = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "max-taken-date")) {
      /* MYSQL datetime */
      params.max_taken_date = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "license")) {
      params.license = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "sort")) {
      /* date-posted-asc, date-posted-desc (default), date-taken-asc,
       * date-taken-desc, interestingness-desc, interestingness-asc,
       * and relevance
       */
      params.sort = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "privacy")) {
      params.privacy_filter = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "bbox")) {
      /* "a,b,c,d" */
      params.bbox = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "accuracy")) {
      /* int 1-16 */
      params.accuracy = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "safe-search")) {
      /* int Safe search setting: 1 safe, 2 moderate, 3 restricted. */
      params.safe_search = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "type")) {
      /* int Content Type setting: 1 for photos only, 2 for screenshots
       * only, 3 for 'other' only, 4 for all types. */
      params.content_type = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "machine-tags")) {
      params.machine_tags = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "machine-tag-mode")) {
      /* any (default) or all */
      params.machine_tag_mode = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "group-id")) {
      params.group_id = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "extras")) {
      list_params.extras = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "per-page")) {
      /* int: default 100, max 500 */
      list_params.per_page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "page")) {
      /* int: default 1 */
      list_params.page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "place-id")) {
      params.place_id = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "media")) {
      /* "all" (default if missing) or "photos" or "video" */
      params.media = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "has-geo")) {
      params.has_geo = 1;
    } else if(!strcmp(field, "lat")) {
      /* double: */
      params.lat = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "lon")) {
      /* double: */
      params.lon = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "radius")) {
      /* double: */
      params.radius = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "radius-units")) {
      params.radius_units = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "contacts")) {
      params.contacts = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "format")) {
      list_params.format = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "woeid")) {
      /* int: */
      params.woe_id = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "geo-context")) {
      /* int: 0 (not defined)  1 (indoors)  2(outdoors) default 0 */
      params.geo_context = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "is-commons")) {
      params.is_commons = 1;
    } else if(!strcmp(field, "in-gallery")) {
      params.in_gallery = 1;
    } else if(!strcmp(field, "tags")) {
      size_t tags_len = 0;
      int j;
      char *p;

      /* tags absorb all remaining parameters */
      for(j = 0; j<argc; j++)
        tags_len+= strlen(argv[j])+1;
      tags_string = (char*)malloc(tags_len);
      
      p = tags_string;
      for(j = 0; j<argc; j++) {
        size_t tag_len = strlen(argv[j]);
        strncpy(p, argv[j], tag_len); p+= tag_len;
        *p++= ',';
      }
      *(--p) = '\0';
      
      params.tags = tags_string;
      break;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, argv[0]);
      usage = 1;
    }
  }

  if(usage) {
    photos_list = NULL;
    goto tidy;
  }
  
  photos_list = flickcurl_photos_search_params(fc, &params, &list_params);
  if(!photos_list) {
    fprintf(stderr, "%s: Searching failed\n", program);
  } else {
    int rc;
    
    rc = command_print_photos_list(fc, photos_list, output_fh, "Search result");
    flickcurl_free_photos_list(photos_list);
    if(rc)
      photos_list = NULL;
  }

  tidy:
  if(params.tags)
    free((char*)params.tags);
  
  return (photos_list == NULL);
}


static int
command_photos_geo_getLocation(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  flickcurl_location* location;

  location = flickcurl_photos_geo_getLocation(fc, photo_id);
  if(!location)
    return 1;

  fprintf(stderr, "%s: Photo ID %s location\n  ", program, photo_id);
  command_print_location(location);

  flickcurl_free_location(location);
  return 0;
}


static int
command_photos_geo_getPerms(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  flickcurl_perms* perms;

  perms = flickcurl_photos_geo_getPerms(fc, photo_id);
  if(!perms)
    return 1;

  fprintf(stderr, "%s: Photo ID %s geo permissions:\n", program, photo_id);
  command_print_perms(perms, 0);

  flickcurl_free_perms(perms);
  return 0;
}


static int
command_photos_geo_removeLocation(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  
  return flickcurl_photos_geo_removeLocation(fc, photo_id);
}


static int
command_photos_geo_setLocation(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  double latitude = atof(argv[2]);
  double longitude = atof(argv[3]);
  int accuracy = atoi(argv[4]);
  flickcurl_location location;

  memset(&location, '\0', sizeof(flickcurl_location));
  location.latitude = latitude;
  location.longitude = longitude;
  location.accuracy = accuracy;

  return flickcurl_photos_geo_setLocation(fc, photo_id, &location);
}


static int
command_photos_geo_setPerms(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  int is_public = parse_bool_param(argv[2]);
  int is_contact = parse_bool_param(argv[3]);
  int is_friend = parse_bool_param(argv[4]);
  int is_family = parse_bool_param(argv[5]);
  flickcurl_perms perms;

  memset(&perms, '\0', sizeof(flickcurl_perms));
  perms.is_public = is_public;
  perms.is_contact = is_contact;
  perms.is_friend = is_friend;
  perms.is_family = is_family;

  return flickcurl_photos_geo_setPerms(fc, photo_id, &perms);
}


static int
command_photos_notes_add(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  int note_x = atoi(argv[2]);
  int note_y = atoi(argv[3]);
  int note_w = atoi(argv[4]);
  int note_h = atoi(argv[5]);
  char* note_text = argv[6];
  char *id;
  
  id = flickcurl_photos_notes_add(fc, photo_id,
                                note_x, note_y, note_w, note_h, note_text);
  if(id) {
    if(verbose)
      fprintf(stderr,
              "%s: Added note '%s' (x:%d y:%d w:%d h:%d) to photo ID %s giving note ID %s\n",
              program, note_text, note_x, note_y, note_w, note_h, photo_id, id);
    free(id);
  }
  
  return (id == NULL);
}

static int
command_photos_notes_delete(flickcurl* fc, int argc, char *argv[])
{
  const char *note_id = argv[1];

  return flickcurl_photos_notes_delete(fc, note_id);
}

static int
command_photos_notes_edit(flickcurl* fc, int argc, char *argv[])
{
  const char *note_id = argv[1];
  int note_x = atoi(argv[2]);
  int note_y = atoi(argv[3]);
  int note_w = atoi(argv[4]);
  int note_h = atoi(argv[5]);
  char* note_text = argv[6];

  return flickcurl_photos_notes_edit(fc, note_id,
                                     note_x, note_y, note_w, note_h,
                                     note_text);
}


static int
command_photos_licenses_setLicense(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id = argv[1];
  int license_id = atoi(argv[2]);

  return flickcurl_photos_licenses_setLicense(fc, photo_id, license_id);
}


static int
command_people_getPublicPhotos(flickcurl* fc, int argc, char *argv[])
{
  char *user_id = argv[1];
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >2) {
    list_params.per_page = parse_page_param(argv[2]);
    if(argc >3) {
      list_params.page = parse_page_param(argv[3]);
      if(argc >4) {
        list_params.format = argv[4];
      }
    }
  }

  photos_list = flickcurl_people_getPublicPhotos_params(fc, user_id,
                                                      &list_params);
  if(!photos_list)
    return 1;

  if(verbose)
    fprintf(stderr, "%s: User %s photos (per_page %d  page %d):\n",
            program, user_id, list_params.per_page, list_params.page);

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}


static int
command_groups_pools_add(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  char *group_id = argv[2];

  return flickcurl_groups_pools_add(fc,  photo_id, group_id);
}


static void
command_print_group(flickcurl_group* g)
{
  fprintf(stderr,
          "group: nsid %s  name '%s'  description '%s'  lang '%s'\n"
          "  admin %d  pool moderated %d  18+ %d  privacy %d\n"
          "  photos %d  iconserver %d  members %d\n"
          "  throttle count %d  mode '%s'  remaining %d\n",
          g->nsid, g->name, (g->description ? g->description : ""),
            (g->lang ? g->lang : ""),
          g->is_admin, g->is_pool_moderated, g->is_eighteenplus, g->privacy,
          g->photos, g->iconserver, g->members,
          g->throttle_count, (g->throttle_mode ? g->throttle_mode : ""), g->throttle_remaining);
}


static int
command_groups_pools_getGroups(flickcurl* fc, int argc, char *argv[])
{
  int per_page = 10;
  int page = 0;
  flickcurl_group** groups = NULL;

  if(argc >1) {
    per_page = parse_page_param(argv[1]);
    if(argc >2){
      page = parse_page_param(argv[2]);
    }
  }

  groups = flickcurl_groups_pools_getGroups(fc, page, per_page);
  if(groups) {
    int i;

    if(verbose)
      fprintf(stderr, "%s: Groups (page %d, per page %d)\n", program,
              page, per_page);
    for(i = 0; groups[i]; i++)
      command_print_group(groups[i]);
    flickcurl_free_groups(groups);
  }
  return (groups == NULL);
}


static int
command_groups_pools_getPhotos(flickcurl* fc, int argc, char *argv[])
{
  char *group_id = argv[1];
  char *tags = NULL;
  char *user_id = NULL;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >2) {
    list_params.per_page = parse_page_param(argv[2]);
    if(argc >3) {
      list_params.page = parse_page_param(argv[3]);
      if(argc >4) {
        list_params.format = argv[4];
      }
    }
  }
  
  photos_list = flickcurl_groups_pools_getPhotos_params(fc, group_id, tags,
                                                      user_id, &list_params);
  if(!photos_list)
    return 1;

  if(verbose)
    fprintf(stderr, "%s: Group %s photos (per_page %d  page %d):\n",
            program, group_id, list_params.per_page, list_params.page);

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}

static int
command_groups_pools_remove(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  char *group_id = argv[2];

  return flickcurl_groups_pools_remove(fc, photo_id, group_id);
}


static int
command_photos_getContactsPublicPhotos(flickcurl* fc, int argc, char *argv[])
{
  char* user_id = argv[1];
  int contact_count = 10;
  int just_friends = 0;
  int single_photo = 1;
  int include_self = 0;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >2) {
    list_params.format = argv[2];
  }
  
  photos_list = flickcurl_photos_getContactsPublicPhotos_params(fc,  user_id,
                                                              contact_count,
                                                              just_friends,
                                                              single_photo,
                                                              include_self,
                                                              &list_params);
  if(!photos_list)
    return 1;

  rc = command_print_photos_list(fc, photos_list, output_fh, "Contact Public Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}


static void
command_print_exif(flickcurl_exif* e)
{
  fprintf(stderr, "tagspace %s (%d) tag %d label '%s' raw '%s' clean '%s'\n",
          e->tagspace, e->tagspaceid, e->tag, e->label, e->raw, e->clean);
}


static int
command_photos_getExif(flickcurl* fc, int argc, char *argv[])
{
  const char* photo_id = argv[1];
  const char* secret = NULL;
  flickcurl_exif** exifs;
  int i;
  
  exifs = flickcurl_photos_getExif(fc, photo_id, secret);
  if(!exifs)
    return 1;

  for(i = 0; exifs[i]; i++)
    command_print_exif(exifs[i]);
  
  flickcurl_free_exifs(exifs);
  return 0;
}


static int
command_photos_getFavorites(flickcurl* fc, int argc, char *argv[])
{
  const char* photo_id = argv[1];
  int i;
  int per_page = 10;
  int page = 0;
  flickcurl_person** persons;
  
  if(argc >2) {
    per_page = parse_page_param(argv[2]);
    if(argc >3) {
      page = parse_page_param(argv[3]);
    }
  }
  
  persons = flickcurl_photos_getFavorites(fc, photo_id, page, per_page);
  if(!persons)
    return 1;

  for(i = 0; persons[i]; i++)
    command_print_person(persons[i]);
  
  flickcurl_free_persons(persons);
  return 0;
}


typedef flickcurl_photos_list* (*photoslist_fn)(flickcurl* fc, int min_upload_date, int max_upload_date, const char* min_taken_date, const char* max_taken_date, int privacy_filter, flickcurl_photos_list_params* list_params);


static int
command_photoslist(flickcurl* fc, int argc, char *argv[],
                   photoslist_fn api_fn, const char* label)
{
  int min_upload_date= -1;
  int max_upload_date= -1;
  char* min_taken_date= NULL;
  char* max_taken_date= NULL;
  int privacy_filter= -1;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >1) {
    list_params.per_page = parse_page_param(argv[1]);
    if(argc >2) {
      list_params.page = parse_page_param(argv[2]);
      if(argc >3) {
        list_params.format = argv[3];
      }
    }
  }

  photos_list = api_fn(fc, min_upload_date, max_upload_date, min_taken_date,
                     max_taken_date, privacy_filter, &list_params);
  if(!photos_list)
    return 1;

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}

static int
command_photos_getNotInSet(flickcurl* fc, int argc, char *argv[])
{
  return command_photoslist(fc, argc, argv,
                            flickcurl_photos_getNotInSet_params,
                            "Photo not in set");
}


static int
command_photos_getSizes(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_size** sizes = NULL;
  char* photo_id = argv[1];
  int i;
  
  sizes = flickcurl_photos_getSizes(fc, photo_id);
  if(!sizes)
    return 1;

  if(verbose)
    fprintf(stderr, "%s: Sizes for photo/video %s\n", program, photo_id);
  for(i = 0; sizes[i]; i++) {
    fprintf(stderr,
            "%d: type '%s' label '%s' width %d height %d\n  source %s\n  url %s\n",
            i, sizes[i]->media,
            sizes[i]->label, sizes[i]->width, sizes[i]->height,
            sizes[i]->source, sizes[i]->url);
  }
  
  flickcurl_free_sizes(sizes);

  return 0;
}
    

static int
command_photos_getRecent(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >1) {
    list_params.per_page = parse_page_param(argv[1]);
    if(argc >2) {
      list_params.page = parse_page_param(argv[2]);
      if(argc >3) {
        list_params.format = argv[3];
      }
    }
  }

  photos_list = flickcurl_photos_getRecent_params(fc, &list_params);
  if(!photos_list)
    return 1;

  rc = command_print_photos_list(fc, photos_list, output_fh, "Recent Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}

static int
command_photos_getUntagged(flickcurl* fc, int argc, char *argv[])
{
  return command_photoslist(fc, argc, argv,
                            flickcurl_photos_getUntagged_params,
                            "Untagged photo");
}

static int
command_photos_getWithGeoData(flickcurl* fc, int argc, char *argv[])
{
  return command_photoslist(fc, argc, argv,
                            flickcurl_photos_getWithGeoData_params,
                            "Photo with geo data");
}

static int
command_photos_getWithoutGeoData(flickcurl* fc, int argc, char *argv[])
{
  return command_photoslist(fc, argc, argv,
                            flickcurl_photos_getWithoutGeoData_params,
                            "Photo without geo data");
}

static int
command_photos_recentlyUpdated(flickcurl* fc, int argc, char *argv[])
{
  int min_date= -1;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  min_date = atoi(argv[1]);

  if(argc >2) {
    list_params.per_page = parse_page_param(argv[2]);
    if(argc >3) {
      list_params.page = parse_page_param(argv[3]);
      if(argc >4) {
        list_params.format = argv[4];
      }
    }
  }

  photos_list = flickcurl_photos_recentlyUpdated_params(fc, min_date,
                                                      &list_params);
  if(!photos_list)
    return 1;

  rc = command_print_photos_list(fc, photos_list, output_fh, "Recently Updated Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}


static
void command_print_photoset(flickcurl_photoset* photoset)
{
  fprintf(stderr, 
          "%s: Found photoset with ID %s primary photo: '%s' secret: %s server: %d farm: %d photos count: %d title: '%s' description: '%s'\n",
          program,
          photoset->id, photoset->primary, photoset->secret,
          photoset->server, photoset->farm,
          photoset->photos_count,
          photoset->title, 
          (photoset->description ? photoset->description : "(No description)"));
}


static int
command_photosets_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_photoset* photoset = NULL;
  const char* photoset_id = argv[1];
  
  photoset = flickcurl_photosets_getInfo(fc, photoset_id);
  if(photoset) {
    command_print_photoset(photoset);
    flickcurl_free_photoset(photoset);
  }
  
  return (photoset == NULL);
}


static int
command_photosets_getList(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_photoset** photoset_list = NULL;
  const char* user_id = argv[1];
  int i;
  
  photoset_list = flickcurl_photosets_getList(fc, user_id);
  if(!photoset_list)
    return 1;
  
  for(i = 0; photoset_list[i]; i++) {
    fprintf(stderr, "%s: Photoset %d\n", program, i);
    command_print_photoset(photoset_list[i]);
  }

  flickcurl_free_photosets(photoset_list);
  
  return 0;
}


static int
command_photosets_getPhotos(flickcurl* fc, int argc, char *argv[])
{
  const char* photoset_id = argv[1];
  int privacy_filter= -1;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc > 2) {
    list_params.extras = argv[2];
    if(argc > 3) {
      privacy_filter = atoi(argv[3]);
      if(argc >4) {
        list_params.per_page = parse_page_param(argv[4]);
        if(argc >5) {
          list_params.page = parse_page_param(argv[5]);
          if(argc >6) {
            list_params.format = argv[6];
          }
        }
      }
    }
  }
  
  photos_list = flickcurl_photosets_getPhotos_params(fc, photoset_id,
                                                   privacy_filter,
                                                   &list_params);
  if(!photos_list)
    return 1;

  if(verbose)
    fprintf(stderr, "%s: Photoset %s photos (per_page %d  page %d):\n",
            program, photoset_id, list_params.per_page, list_params.page);

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}


static int
command_photosets_addPhoto(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_photosets_addPhoto(fc, argv[1], argv[2]);
}


static int
command_photosets_create(flickcurl* fc, int argc, char *argv[])
{
  const char* title = argv[1];
  const char* description = argv[2];
  const char* primary_photo_id = argv[3];
  char* url = NULL;
  char* id;
  
  id = flickcurl_photosets_create(fc, title, description, primary_photo_id,
                                &url);
  if(!id)
    return 1;
  fprintf(stderr, "%s: Photoset %s created with URL %s\n", program, id, url);
  free(url);
  free(id);
  return 0;
}


static int
command_photosets_delete(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_photosets_delete(fc, argv[1]);
}


static int
command_photosets_editMeta(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_photosets_editMeta(fc, argv[1], argv[2], argv[3]);
}


static int
command_photosets_editPhotos(flickcurl* fc, int argc, char *argv[])
{
  const char* photoset_id = argv[1];
  const char* primary_photo_id = argv[2];
  char** photo_ids = flickcurl_array_split(argv[3], ',');
  int rc;
  
  rc = flickcurl_photosets_editPhotos(fc, photoset_id, primary_photo_id,
                                    (const char**)photo_ids);
  flickcurl_array_free(photo_ids);
  return rc;
}


static int
command_photosets_orderSets(flickcurl* fc, int argc, char *argv[])
{
  char** photoset_ids = flickcurl_array_split(argv[1], ',');
  int rc;
  
  rc = flickcurl_photosets_orderSets(fc, (const char**)photoset_ids);
  flickcurl_array_free(photoset_ids);
  return rc;
}


static int
command_photosets_removePhoto(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_photosets_removePhoto(fc, argv[1], argv[2]);
}


static int
command_photos_upload_checkTickets(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_ticket** tickets = NULL;
  char** tickets_ids = flickcurl_array_split(argv[1], ',');

  tickets = flickcurl_photos_upload_checkTickets(fc, (const char**)tickets_ids);

  if(tickets) {
    int i;
    
    for(i = 0; tickets[i]; i++) {
      fprintf(stderr,
              "%s: %d) ticket ID %d  photoID %d  complete %d  invalid %d\n",
              program, i, tickets[i]->id, tickets[i]->photoid,
              tickets[i]->complete, tickets[i]->invalid);
    }
    flickcurl_free_tickets(tickets);
  }

  if(tickets_ids)
    flickcurl_array_free(tickets_ids);
  
  return (tickets != NULL);
}


static void
command_print_category(flickcurl_category* c)
{
  fprintf(stderr, "category: id %s  name '%s'  path '%s'  count %d\n",
          c->id, c->name, c->path, c->count);
  if(c->categories) {
    int i;
    for(i = 0; c->categories[i]; i++) {
      fprintf(stderr, "%s: Category %d\n", program, i);
      command_print_category(c->categories[i]);
    }
  }
  if(c->groups) {
    int i;
    for(i = 0; c->groups[i]; i++) {
      fprintf(stderr, "%s: Group %d\n", program, i);
      command_print_group(c->groups[i]);
    }
  }
}


static int
command_groups_browse(flickcurl* fc, int argc, char *argv[])
{
  int cat_id= -1;
  flickcurl_category* category = NULL;
  
  if(argv[1])
    cat_id = atoi(argv[1]);
  
  category = flickcurl_groups_browse(fc, cat_id);
  if(category) {
    command_print_category(category);
    flickcurl_free_category(category);
  }
  return (category == NULL);
}


static int
command_groups_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_group* group = NULL;
  const char* group_id = argv[1];
  const char* lang = argv[2];
  
  group = flickcurl_groups_getInfo(fc, group_id, lang);
  if(group) {
    command_print_group(group);
    flickcurl_free_group(group);
  }
  
  return (group == NULL);
}


static int
command_groups_search(flickcurl* fc, int argc, char *argv[])
{
  const char* text = argv[1];
  int per_page= -1;
  int page= -1;
  flickcurl_group** groups = NULL;
  int i;
  
  if(argc >2) {
    per_page = parse_page_param(argv[2]);
    if(argc >3) {
      page = parse_page_param(argv[3]);
    }
  }
  
  groups = flickcurl_groups_search(fc, text, per_page, page);
  if(!groups)
    return 1;

  for(i = 0; groups[i]; i++) {
    fprintf(stderr, "%s: Group %d\n", program, i);
    command_print_group(groups[i]);
  }

  flickcurl_free_groups(groups);

  return 0;
}


static int
command_people_getPublicGroups(flickcurl* fc, int argc, char *argv[])
{
  char* user_id = argv[1];
  flickcurl_group** groups = NULL;
  int i;
  
  groups = flickcurl_people_getPublicGroups(fc, user_id);
  if(!groups)
    return 1;

  for(i = 0; groups[i]; i++) {
    fprintf(stderr, "%s: Group %d\n", program, i);
    command_print_group(groups[i]);
  }

  flickcurl_free_groups(groups);

  return 0;
}


static int
command_people_getUploadStatus(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_user_upload_status* u;
  
  u = flickcurl_people_getUploadStatus(fc);
  if(!u)
    return 1;
  
  fprintf(stderr, "user upload status for %s:\n"
          "  bandwidth max %d/%d K  used %d/%d K  remaining %d/%d K\n"
          "  max filesize %d/%d K  sets created %d remaining %s\n",
            u->username,
            u->bandwidth_maxbytes, u->bandwidth_maxkb,
            u->bandwidth_usedbytes, u->bandwidth_usedkb,
            u->bandwidth_remainingbytes, u->bandwidth_remainingkb,
            u->filesize_maxbytes, u->filesize_maxkb,
            u->sets_created, (u->sets_remaining ? u->sets_remaining : ""));

  flickcurl_free_user_upload_status(u);
  return 0;
}


static int
command_photos_transform_rotate(flickcurl* fc, int argc, char *argv[])
{
  const char* photo_id = argv[1];
  int degrees = atoi(argv[2]);
  
  return flickcurl_photos_transform_rotate(fc, photo_id, degrees);
}


static int
command_interestingness_getList(flickcurl* fc, int argc, char *argv[])
{
  int usage = 0;
  char* date = NULL;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  argv++; argc--;
  
  while(!usage && argc) {
    char* field = argv[0];
    argv++; argc--;

    if(!strcmp(field, "date")) {
      date = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "extras")) {
      list_params.extras = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "per-page")) {
      /* int: default 100, max 500 */
      list_params.per_page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "page")) {
      /* int: default 1 */
      list_params.page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "format")) {
      list_params.format = argv[0];
      argv++; argc--;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, field);
      usage = 1;
    }
  }

  if(usage) {
    photos_list = NULL;
    goto tidy;
  }
  
  photos_list = flickcurl_interestingness_getList_params(fc, date, &list_params);
  if(!photos_list)
    return 1;

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);
  if(rc)
    photos_list = NULL;  

  tidy:
  
  return (photos_list == NULL);
}


static int
command_places_resolvePlaceId(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place* place = NULL;
  char* place_id = argv[1];

  place = flickcurl_places_resolvePlaceId(fc, place_id);
  if(place) {
    command_print_place(place, NULL, NULL, 1);
    flickcurl_free_place(place);
  }
  
  return (place == NULL);
}

static int
command_places_resolvePlaceURL(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place* place = NULL;
  char* place_url = argv[1];

  place = flickcurl_places_resolvePlaceURL(fc, place_url);
  if(place) {
    command_print_place(place, NULL, NULL, 1);
    flickcurl_free_place(place);
  }
  
  return (place == NULL);
}


static int
command_favorites_add(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];

  return flickcurl_favorites_add(fc, photo_id);
}


static int
command_favorites_getList(flickcurl* fc, int argc, char *argv[])
{
  char *user_id = argv[1];
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;

  flickcurl_photos_list_params_init(&list_params);

  if(argc >2) {
    list_params.per_page = parse_page_param(argv[2]);
    if(argc >3) {
      list_params.page = parse_page_param(argv[3]);
      if(argc >4)
        list_params.format = argv[4];
    }
  }
  
  photos_list = flickcurl_favorites_getList_params(fc, user_id, &list_params);
  if(!photos_list) {
    fprintf(stderr, "%s: Getting favorites failed\n", program);
  } else {
    int rc;
    if(verbose)
      fprintf(stderr,
              "%s: User %s has %d favorite photos (per_page %d  page %d):\n",
              program, user_id, photos_list->photos_count,
              list_params.per_page, list_params.page);
    rc = command_print_photos_list(fc, photos_list, output_fh, "Favorite photos");

    flickcurl_free_photos_list(photos_list);
    if(rc)
      photos_list = NULL;
  }

  return (photos_list == NULL);
}


static int
command_favorites_getPublicList(flickcurl* fc, int argc, char *argv[])
{
  char *user_id = argv[1];
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >2) {
    list_params.per_page = parse_page_param(argv[2]);
    if(argc >3) {
      list_params.page = parse_page_param(argv[3]);
      if(argc >4) {
        list_params.format = argv[4];
      }
    }
  }
  
  photos_list = flickcurl_favorites_getPublicList_params(fc, user_id,
                                                       &list_params);
  if(!photos_list)
    return 1;

  if(verbose)
    fprintf(stderr,
            "%s: User %s public favorite photos (per_page %d  page %d):\n",
            program, user_id, list_params.per_page, list_params.page);
  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}


static int
command_favorites_remove(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];

  return flickcurl_favorites_remove(fc, photo_id);
}


static int
command_blogs_getList(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_blog** blogs = NULL;
  
  blogs = flickcurl_blogs_getList(fc);
  if(blogs) {
    int i;
    
    for(i = 0; blogs[i]; i++) {
      fprintf(stderr,
              "Blog %d) id %s  name '%s'  needs password '%d'  url '%s'\n",
              i,
              blogs[i]->id, blogs[i]->name, blogs[i]->needs_password, 
              blogs[i]->url);
    }

    flickcurl_free_blogs(blogs);
  }
  return (blogs == NULL);
}


static int
command_blogs_postPhoto(flickcurl* fc, int argc, char *argv[])
{
  const char* blog_id = argv[1];
  const char* photo_id = argv[2];
  const char* title = argv[3];
  const char* description = argv[4];
  const char* blog_password = argv[5];

  return flickcurl_blogs_postPhoto(fc, blog_id, photo_id, title,
                                   description, blog_password);
}


static void
command_print_activity(flickcurl_activity* a)
{
  fprintf(stderr,
          "  type %s  id %s  owner %s name '%s'  primary %s\n"
          "  secret %s  server %d farm %d\n"
          "  comments %d old/new %d/%d  notes old/new %d/%d\n"
          "  views %d  photos %d  faves %d  more %d\n"
          "  title '%s'\n"
          ,
          a->type, a->id, a->owner, a->owner_name,
          (a->primary ? a->primary : ""),
          a->secret, a->server, a->farm,
          a->comments, a->comments_old, a->comments_new,
          a->notes_old, a->notes_new,
          a->views, a->photos, a->faves, a->more,
          a->title);
  if(a->events) {
    int i;
    
    for(i = 0; a->events[i]; i++) {
      flickcurl_activity_event* ae = a->events[i];
      
      fprintf(stderr,
              "    activity event %i) type %s  user %s  username %s\n      datetime %d\n      value '%s'\n",
              i, ae->type, ae->user, ae->username, 
              ae->date_added,
              ae->value);
    }
  }
}


static int
command_activity_userComments(flickcurl* fc, int argc, char *argv[])
{
  int per_page = 10;
  int page = 0;
  flickcurl_activity** activities = NULL;
  int i;

  if(argc >1) {
    per_page = parse_page_param(argv[1]);
    if(argc >2) {
      page = parse_page_param(argv[2]);
    }
  }

  activities = flickcurl_activity_userComments(fc, per_page, page);
  if(!activities)
    return 1;

  if(verbose)
    fprintf(stderr, 
            "%s: Comments on the caller's photos (per_page %d  page %d):\n",
            program, per_page, page);
  for(i = 0; activities[i]; i++) {
    fprintf(stderr, "%s: Activity %d\n", program, i);
    command_print_activity(activities[i]);
  }
  
  flickcurl_free_activities(activities);

  return 0;
}


static int
command_activity_userPhotos(flickcurl* fc, int argc, char *argv[])
{
  char* timeframe = argv[1];
  int per_page = 10;
  int page = 0;
  flickcurl_activity** activities = NULL;
  int i;

  if(argc >2) {
    per_page = parse_page_param(argv[2]);
    if(argc >3) {
      page = parse_page_param(argv[3]);
    }
  }

  activities = flickcurl_activity_userPhotos(fc, timeframe, per_page, page);
  if(!activities)
    return 1;

  if(verbose)
    fprintf(stderr, 
            "%s: Recent activity on the caller's photos (timeframe %s  per_page %d  page %d):\n",
            program, timeframe, per_page, page);
  for(i = 0; activities[i]; i++) {
    fprintf(stderr, "%s: Activity %d\n", program, i);
    command_print_activity(activities[i]);
  }
  
  flickcurl_free_activities(activities);

  return 0;
}


static int
command_places_find(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place** places = NULL;
  char* query = argv[1];

  places = flickcurl_places_find(fc, query);
  if(places) {
    int i;
    for(i = 0; places[i]; i++) {
      fprintf(stderr, "Place Result #%d\n", i);
      command_print_place(places[i], NULL, NULL, 1);
    }
    flickcurl_free_places(places);
  }
  
  return (places == NULL);
}


static int
command_places_findByLatLon(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place* place = NULL;
  double lat = atof(argv[1]);
  double lon = atof(argv[2]);
  int accuracy = atoi(argv[3]);

  place = flickcurl_places_findByLatLon(fc, lat, lon, accuracy);
  if(place) {
    command_print_place(place, NULL, NULL, 1);
    flickcurl_free_place(place);
  }
  
  return (place == NULL);
}


static int
command_prefs_getContentType(flickcurl* fc, int argc, char *argv[])
{
  int content_type;

  content_type = flickcurl_prefs_getContentType(fc);
  if(content_type) {
    fprintf(stderr, "%s: Content type preference is %d\n", program,
            content_type);
  }

  return (content_type <0);
}


static int
command_prefs_getHidden(flickcurl* fc, int argc, char *argv[])
{
  int hidden;

  hidden = flickcurl_prefs_getHidden(fc);
  if(hidden >= 0) {
    fprintf(stderr, "%s: Hidden preference is %d\n", program, hidden);
  }

  return (hidden < 0);
}


static int
command_prefs_getPrivacy(flickcurl* fc, int argc, char *argv[])
{
  int privacy;

  privacy = flickcurl_prefs_getPrivacy(fc);
  if(privacy >= 0) {
    fprintf(stderr, "%s: Privacy preference is %d\n", program, privacy);
  }

  return (privacy < 0);
}


static int
command_prefs_getSafetyLevel(flickcurl* fc, int argc, char *argv[])
{
  int safety_level;

  safety_level = flickcurl_prefs_getSafetyLevel(fc);
  if(safety_level >= 0) {
    fprintf(stderr, "%s: Safety level preference is %d\n", program,
            safety_level);
  }

  return (safety_level < 0);
}


static int
command_prefs_getGeoPerms(flickcurl* fc, int argc, char *argv[])
{
  int geo_perms;

  geo_perms = flickcurl_prefs_getGeoPerms(fc);
  if(geo_perms >= 0) {
    fprintf(stderr, "%s: Geographic information preference is %d\n", program, geo_perms);
  }

  return (geo_perms < 0);
}


static int
command_tags_getClusters(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag_clusters* clusters;
  flickcurl_tag_cluster* cluster;
  const char *tag = argv[1];
  int clusteri;
  
  clusters = flickcurl_tags_getClusters(fc, tag);
  if(!clusters)
    return 1;

  fprintf(stderr, "%s: Tag %s returned %d clusters\n", program, tag,
          clusters->count);

  for(clusteri = 0; (cluster = clusters->clusters[clusteri]); clusteri++) {
    const char* tag_name;
    int tagi;
    fprintf(stderr, "%s: Cluster #%d - %d tags\n", program, clusteri,
            cluster->count);
    for(tagi = 0; (tag_name = cluster->tags[tagi]); tagi++)
      fprintf(stderr, "  %s\n", tag_name);
  }

  flickcurl_free_tag_clusters(clusters);
  return 0;
}


static int
command_places_placesForUser(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place** places = NULL;
  int woe_id= -1;
  int threshold= -1;
  const char* place_id = NULL;
  flickcurl_place_type place_type;
  
  place_type = flickcurl_get_place_type_by_label(argv[1]);
  
  if(argc > 2) {
    if(strcmp(argv[2], "-"))
       woe_id = atoi(argv[2]);
    if(argc > 3) {
      place_id = argv[3];
      if(argc > 4) {
        threshold = atoi(argv[4]);
      }
    }
  }

  places = flickcurl_places_placesForUser(fc, place_type, woe_id, place_id,
                                        threshold);
  if(places) {
    int i;
    for(i = 0; places[i]; i++) {
      fprintf(stderr, "Place Result #%d\n", i);
      command_print_place(places[i], NULL, NULL, 0);
    }
    flickcurl_free_places(places);
  }
  
  return (places == NULL);
}


static int
command_places_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place* place = NULL;
  const char* place_id = NULL;
  int woe_id = -1;

  if(strcmp(argv[1], "-"))
    place_id = argv[1];

  if(argc > 2) {
    if(strcmp(argv[2], "-"))
      woe_id = atoi(argv[2]);
  }

  if(!place_id && !woe_id)
    return 1;
  
  place = flickcurl_places_getInfo2(fc, place_id, woe_id);
  if(place) {
    command_print_place(place, NULL, NULL, 1);
    flickcurl_free_place(place);
  }
  
  return (place == NULL);
}


static int
command_places_getInfoByUrl(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place* place = NULL;
  const char* url = argv[1];

  place = flickcurl_places_getInfoByUrl(fc, url);
  if(place) {
    command_print_place(place, NULL, NULL, 1);
    flickcurl_free_place(place);
  }
  
  return (place == NULL);
}


static int
command_places_getChildrenWithPhotosPublic(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place** places = NULL;
  const char* place_id = NULL;
  int woe_id = -1;

  if(strcmp(argv[1], "-"))
    place_id = argv[1];

  if(argc > 2) {
    if(strcmp(argv[2], "-"))
      woe_id = atoi(argv[2]);
  }

  if(!place_id && !woe_id)
    return 1;
  
  places = flickcurl_places_getChildrenWithPhotosPublic2(fc, place_id, woe_id);
  if(places) {
    int i;
    for(i = 0; places[i]; i++) {
      fprintf(stderr, "Place Result #%d\n", i);
      command_print_place(places[i], NULL, NULL, 0);
    }
    flickcurl_free_places(places);
  }
  
  return (places == NULL);
}


static int
command_machinetags_getNamespaces(flickcurl* fc, int argc, char *argv[])
{
  const char* predicate = NULL;
  int per_page = 10;
  int page = 0;
  flickcurl_tag_namespace** tag_namespaces = NULL;

  if(argc >1) {
    predicate = argv[1];
    if(argc >2) {
      per_page = parse_page_param(argv[2]);
      if(argc >3) {
        page = parse_page_param(argv[3]);
      }
    }
  }

  tag_namespaces = flickcurl_machinetags_getNamespaces(fc, predicate,
                                                       per_page, page);
  if(tag_namespaces) {
    int i;
    for(i = 0; tag_namespaces[i]; i++) {
      flickcurl_tag_namespace *tn = tag_namespaces[i];
      
      fprintf(stderr,
              "Namespace #%d: name %s usage %d predicates count %d\n",
              i, tn->name, tn->usage_count, tn->predicates_count);
    }
    flickcurl_free_tag_namespaces(tag_namespaces);
  }

  return (tag_namespaces != NULL);
}


static void
command_print_predicate_values(flickcurl_tag_predicate_value **tag_pvs,
                               const char* label)
{
  int i;
  if(label)
    fprintf(stderr, "%s: %s\n", program, label);

  for(i = 0; tag_pvs[i]; i++) {
    flickcurl_tag_predicate_value* tpv = tag_pvs[i];
    fprintf(stderr, "  #%d) ", i);
    if(tpv->predicate) {
      fputs("predicate ", stderr);
      fputs(tpv->predicate, stderr);
      fputc(' ', stderr);
    }
    if(tpv->value) {
      fputs("value ", stderr);
      fputs(tpv->value, stderr);
      fputc(' ', stderr);
    }
    if(tpv->usage_count > 0)
      fprintf(stderr, "usage %d", tpv->usage_count);
    if(tpv->used_in_namespace_count > 0)
      fprintf(stderr, "used in %d namespaces", tpv->used_in_namespace_count);
      fputc('\n', stderr);
  }
}


static int
command_machinetags_getPairs(flickcurl* fc, int argc, char *argv[])
{
  const char *nspace = NULL;
  const char* predicate = NULL;
  int per_page = 10;
  int page = 0;
  flickcurl_tag_predicate_value** tag_pvs = NULL;

  if(argc >1) {
    nspace = argv[1];
    if(argc >2) {
      predicate = argv[2];
      if(argc >3) {
        per_page = parse_page_param(argv[3]);
        if(argc >4) {
          page = parse_page_param(argv[4]);
        }
      }
    }
  }

  tag_pvs = flickcurl_machinetags_getPairs(fc, nspace, predicate, per_page,
                                           page);
  if(tag_pvs) {
    command_print_predicate_values(tag_pvs, "getPairs returned");
    flickcurl_free_tag_predicate_values(tag_pvs);
  }

  return (tag_pvs == NULL);
}


static int
command_machinetags_getPredicates(flickcurl* fc, int argc, char *argv[])
{
  const char *nspace = NULL;
  int per_page = 10;
  int page = 0;
  flickcurl_tag_predicate_value** tag_pvs = NULL;

  if(argc >1) {
    nspace = argv[1];
    if(argc >2) {
      per_page = parse_page_param(argv[2]);
      if(argc >3) {
        page = parse_page_param(argv[3]);
      }
    }
  }

  tag_pvs = flickcurl_machinetags_getPredicates(fc, nspace, per_page, page);

  if(tag_pvs) {
    command_print_predicate_values(tag_pvs, "getPredicates returned");
    flickcurl_free_tag_predicate_values(tag_pvs);
  }

  return (tag_pvs == NULL);
}


static int
command_machinetags_getValues(flickcurl* fc, int argc, char *argv[])
{
  const char *nspace = argv[1];
  const char* predicate = argv[2];
  int per_page = 10;
  int page = 0;
  flickcurl_tag_predicate_value** tag_pvs = NULL;

  if(argc >3) {
    per_page = parse_page_param(argv[3]);
    if(argc >4) {
      page = parse_page_param(argv[4]);
    }
  }

  tag_pvs = flickcurl_machinetags_getValues(fc, nspace, predicate,
                                            per_page, page);

  if(tag_pvs) {
    command_print_predicate_values(tag_pvs, "getValues returned");
    flickcurl_free_tag_predicate_values(tag_pvs);
  }

  return (tag_pvs == NULL);
}


static int
command_places_getPlaceTypes(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place_type_info** place_types;

  place_types = flickcurl_places_getPlaceTypes(fc);
  if(place_types) {
    int i;
    for(i = 0; place_types[i]; i++) {
      flickcurl_place_type_info *pt = place_types[i];
      fprintf(stderr, "place type %d): id %d  type %d  name %s\n",
              i, pt->id, pt->type, pt->name);
    }
    flickcurl_free_place_type_infos(place_types);
  }

  return (place_types == NULL);
}



static int
command_places_placesForBoundingBox(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place** places = NULL;
  double minimum_longitude;
  double minimum_latitude;
  double maximum_longitude;
  double maximum_latitude;
  flickcurl_place_type place_type = FLICKCURL_PLACE_LOCATION;
  
  place_type = flickcurl_get_place_type_by_label(argv[1]);
  
  minimum_longitude = atof(argv[2]);
  minimum_latitude = atof(argv[3]);
  maximum_longitude = atof(argv[4]);
  maximum_latitude = atof(argv[5]);

  places = flickcurl_places_placesForBoundingBox(fc,
                                                 place_type, 
                                                 minimum_longitude,
                                                 minimum_latitude,
                                                 maximum_longitude,
                                                 maximum_latitude);
  if(places) {
    int i;
    for(i = 0; places[i]; i++) {
      fprintf(stderr, "Place Result #%d\n", i);
      command_print_place(places[i], NULL, NULL, 0);
    }
    flickcurl_free_places(places);
  }
  
  return (places == NULL);
}


static int
command_places_placesForContacts(flickcurl* fc, int argc, char *argv[])
{
  int usage = 0;
  flickcurl_place** places = NULL;
  flickcurl_place_type place_type = FLICKCURL_PLACE_LOCATION;
  int woe_id = -1;
  char* place_id = NULL;
  int threshold = -1;
  char* contacts = NULL;
  int min_upload_date = -1;
  int max_upload_date = -1;
  int min_taken_date = -1;
  int max_taken_date = -1;
  
  place_type = flickcurl_get_place_type_by_label(argv[1]);
  if(strcmp(argv[2], "-"))
     woe_id = atoi(argv[2]);
  if(strcmp(argv[3], "-"))
     place_id = argv[3];
  threshold = atoi(argv[4]);
  argv+= 5; argc-= 5;

  while(!usage && argc) {
    char* field = argv[0];
    argv++; argc--;

    if(!strcmp(field, "contacts")) {
      contacts = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "min-upload")) {
      min_upload_date = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "max-upload")) {
      max_upload_date = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "min-taken")) {
      min_taken_date = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "max-taken")) {
      max_taken_date = atoi(argv[0]);
      argv++; argc--;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, argv[0]);
      usage = 1;
    }
  }

  if(usage) {
    places = NULL;
    goto tidy;
  }

  places = flickcurl_places_placesForContacts(fc,
                                              place_type, 
                                              woe_id, place_id, threshold,
                                              contacts,
                                              min_upload_date,
                                              max_upload_date,
                                              min_taken_date,
                                              max_taken_date);
  if(places) {
    int i;
    for(i = 0; places[i]; i++) {
      fprintf(stderr, "Place Result #%d\n", i);
      command_print_place(places[i], NULL, NULL, 0);
    }
    flickcurl_free_places(places);
  }

  tidy:
  return (places == NULL);
}


static void
command_print_contact(flickcurl_contact* contact, int i)
{
  fprintf(stderr,
          "contact %d: NSID %s username %s iconserver %d realname %s friend %d family %d ignored %d upload count %d\n",
          i,
          contact->nsid, contact->username,
          contact->iconserver, contact->realname,
          contact->is_friend, contact->is_family,
          contact->ignored, contact->uploaded);
}


static int
command_contacts_getList(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_contact **contacts = NULL;
  const char* filter = NULL;
  int page = -1;
  int per_page = -1;
  
  if(argc >1) {
    filter = argv[1];
    if(argc >2) {
      per_page = parse_page_param(argv[2]);
      if(argc >3) {
        page = parse_page_param(argv[3]);
      }
    }
  }

  contacts = flickcurl_contacts_getList(fc, filter, page, per_page);
  if(contacts) {
    int i;
    for(i = 0; contacts[i]; i++)
      command_print_contact(contacts[i], i);

    flickcurl_free_contacts(contacts);
  }

  return (contacts == NULL);
}


static int
command_contacts_getListRecentlyUploaded(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_contact **contacts = NULL;
  int date_lastupload = 1;
  const char* filter = NULL;
  
  if(argc >1) {
    date_lastupload = atoi(argv[1]);
    if(argc >2) {
      filter = argv[2];
    }
  }

  contacts = flickcurl_contacts_getListRecentlyUploaded(fc, date_lastupload,
                                                        filter);
  if(contacts) {
    int i;
    for(i = 0; contacts[i]; i++)
      command_print_contact(contacts[i], i);

    flickcurl_free_contacts(contacts);
  }

  return (contacts == NULL);
}


static int
command_places_getShapeHistory(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_shapedata **shapes = NULL;
  int woe_id = -1;
  char* place_id = NULL;

  if(strcmp(argv[1], "-"))
    place_id = argv[1];
  if(argc > 2) {
    if(strcmp(argv[2], "-"))
     woe_id = atoi(argv[2]);
  }

  if(!woe_id < 0 && !place_id)
    return 1;

  shapes = flickcurl_places_getShapeHistory(fc, place_id, woe_id);
  if(shapes) {
    int i;
    for(i = 0; shapes[i]; i++) {
      fprintf(stderr, "Shape %d: ", i);
      command_print_shape(shapes[i]);
      fputc('\n', stderr);
    }

    flickcurl_free_shapes(shapes);
  }

  return (shapes == NULL);
}


static int
command_places_tagsForPlace(flickcurl* fc, int argc, char *argv[])
{
  int usage = 0;
  flickcurl_tag **tags = NULL;
  int woe_id = -1;
  const char* place_id = NULL;
  int min_upload_date = -1;
  int max_upload_date = -1;
  int min_taken_date = -1;
  int max_taken_date = -1;
    
  argv++; argc--;
  if(strcmp(argv[0], "-"))
    place_id = argv[0];
  argv++; argc--;
  if(argc) {
    if(strcmp(argv[0], "-"))
     woe_id = atoi(argv[0]);
  }

  while(!usage && argc > 0) {
    char* field = argv[0];
    argv++; argc--;

    if(!strcmp(field, "min-upload")) {
      min_upload_date = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "max-upload")) {
      max_upload_date = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "min-taken")) {
      min_taken_date = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "max-taken")) {
      max_taken_date = atoi(argv[0]);
      argv++; argc--;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, argv[0]);
      usage = 1;
    }
  }

  if(usage) {
    tags = NULL;
    goto tidy;
  }

  tags = flickcurl_places_tagsForPlace(fc, woe_id, place_id,
                                       min_upload_date, max_upload_date,
                                       min_taken_date, max_taken_date);
  if(tags) {
    fprintf(stderr, "%s: Tags for WOE ID %d / place ID %s\n",
            program, woe_id, place_id);
    command_print_tags(tags, NULL, NULL);
    free(tags);
  }

  tidy:
  return (tags == NULL);
}


static void
command_print_institution(flickcurl_institution* institution, int ix)
{
  int i;

  fprintf(stderr, "Institution %d:\n"
          "  NSID: %s\n  Date launch: %d\n  Name: %s\n",
          ix, institution->nsid, institution->date_launch,
          institution->name);

  for(i = 0 ; i <= FLICKCURL_INSTITUTION_URL_LAST; i++) {
    if(institution->urls[i])
      fprintf(stderr, "  URL %s: %s\n",
              flickcurl_get_institution_url_type_label((flickcurl_institution_url_type)i),
              institution->urls[i]);
  }
  
}


static int
command_commons_getInstitutions(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_institution** institutions = NULL;
  
  institutions = flickcurl_commons_getInstitutions(fc);
  if(institutions) {
    int i;
    for(i = 0; institutions[i]; i++)
      command_print_institution(institutions[i], i);

    flickcurl_free_institutions(institutions);
  }

  return (institutions == NULL);
}


static void
command_print_member(flickcurl_member* member, int i)
{
    fprintf(stderr,
            "member %d: NSID %s username %s iconserver %d iconfarm %d member type %d\n",
            i,
            member->nsid, member->username,
            member->iconserver, member->iconfarm, member->member_type);
}


static int
command_groups_members_getList(flickcurl* fc, int argc, char *argv[])
{
  const char *group_id = argv[1];
  const char *membertypes = NULL;
  int per_page = -1;
  int page = -1;
  flickcurl_member** members;

  if(argc > 1) {
    membertypes = argv[2];
    if(argc > 2) {
      per_page = parse_page_param(argv[3]);
      if(argc > 3) {
        page = parse_page_param(argv[4]);
      }
    }
  }

  members = flickcurl_groups_members_getList(fc, group_id, membertypes,
                                             per_page, page);
  if(members) {
    int i;
    for(i = 0; members[i]; i++)
      command_print_member(members[i], i);

    flickcurl_free_members(members);
  }

  return (members == NULL);
}


static int
command_panda_getList(flickcurl* fc, int argc, char *argv[])
{
  int i;
  char **pandas;
  
  pandas = flickcurl_panda_getList(fc);
  if(!pandas)
    return 1;
  
  for(i = 0; pandas[i]; i++)
    fprintf(stderr, "%s: panda %d: %s\n", program, i, pandas[i]);
  free(pandas);

  return 0;
}

static int
command_panda_getPhotos(flickcurl* fc, int argc, char *argv[])
{
  char *panda = argv[1];
  flickcurl_photo **photos = NULL;
  int i;
  
  photos = flickcurl_panda_getPhotos(fc, panda);
  if(!photos)
    return 1;

  fprintf(stderr, "%s: Panda %s returned photos!\n", program, panda);
  for(i = 0; photos[i]; i++) {
    fprintf(stderr, "%s: %s photo %d\n", program, panda, i);
    command_print_photo(photos[i]);
  }
  flickcurl_free_photos(photos);

  return 0;
}


static void
command_print_collection(flickcurl_collection *collection)
{
  fprintf(stderr, "Collection id %s  secret %s  server %d\n"
                  "  Title %s\n"
                  "  Description %s\n"
                  "  Large icon %s\n"
                  "  Small Icon %s\n",
          collection->id, collection->secret, collection->server,
          collection->title, 
          (collection->description ? collection->description : "(None)"),
          collection->iconlarge, collection->iconsmall);

  if(collection->photos) {
    int i;
    
    for(i = 0; collection->photos[i]; i++) {
      fprintf(stderr, "  icon photo %d) ", i);
      command_print_photo(collection->photos[i]);
    }
  }
  
  if(collection->collections) {
    int i;
  
    for(i = 0; collection->collections[i]; i++) {
      fprintf(stderr, "  Sub-Collection %d)", i);
      command_print_collection(collection->collections[i]);
    }
  }
  
}


static int
command_collections_getInfo(flickcurl* fc, int argc, char *argv[])
{
  char *collection_id = argv[1];
  flickcurl_collection *collection = NULL;
  
  collection = flickcurl_collections_getInfo(fc, collection_id);
  if(collection) {
    command_print_collection(collection);
    flickcurl_free_collection(collection);
  }

  return (collection == NULL);
}


static int
command_collections_getTree(flickcurl* fc, int argc, char *argv[])
{
  char *collection_id = NULL;
  char *user_id = NULL;
  flickcurl_collection *collection = NULL;
  
  if(strcmp(argv[1], "-"))
    collection_id = argv[1];
  if(strcmp(argv[2], "-"))
    user_id = argv[2];

  collection = flickcurl_collections_getTree(fc, collection_id, user_id);
  if(collection) {
    command_print_collection(collection);
    flickcurl_free_collection(collection);
  }

  return (collection == NULL);
}


static int
command_machinetags_getRecentValues(flickcurl* fc, int argc, char *argv[])
{
  const char *nspace = NULL;
  const char *predicate = NULL;
  int added_since = 0;
  flickcurl_tag_predicate_value** tag_pvs = NULL;

  if(argc >1) {
    if(strcmp("-", argv[1]))
      nspace = argv[1];
    if(argc >2) {
      if(strcmp("-", argv[2]))
        nspace = argv[2];
      if(argc >3) {
        added_since = atoi(argv[3]);
      }
    }
  }

  tag_pvs = flickcurl_machinetags_getRecentValues(fc, nspace, predicate,
                                                  added_since);

  if(tag_pvs) {
    command_print_predicate_values(tag_pvs, "getRecentValues returned");
    flickcurl_free_tag_predicate_values(tag_pvs);
  }

  return (tag_pvs == NULL);
}


static int
command_comments_getRecentForContacts(flickcurl* fc, int argc, char *argv[])
{
  int usage = 0;
  int date_lastcomment = -1;
  const char *filter = NULL;
  flickcurl_photos_list_params list_params;
  flickcurl_photos_list* photos_list = NULL;
  int rc = 0;
  
  flickcurl_photos_list_params_init(&list_params);

  argv++; argc--;
  while(!usage && argc) {
    char* field = argv[0];
    argv++; argc--;

    if(!strcmp(field, "since")) {
      date_lastcomment = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "filter")) {
      filter = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "per-page")) {
      /* int: default 100, max 500 */
      list_params.per_page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "page")) {
      /* int: default 1 */
      list_params.page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "format")) {
      list_params.format = argv[0];
      argv++; argc--;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, argv[0]);
      usage = 1;
    }
  }

  if(usage)
    goto tidy;
  
  photos_list = flickcurl_photos_comments_getRecentForContacts_params(fc,
                                                                      date_lastcomment,
                                                                      filter,
                                                                      &list_params);
  if(!photos_list) {
    fprintf(stderr, "%s: Getting recent comments for contacts failed\n",
            program);
    return 1;
  }
  
  rc = command_print_photos_list(fc, photos_list, output_fh, "Recent content photos");
  flickcurl_free_photos_list(photos_list);

  tidy:
  
  return rc;
}


static int
command_short_uri(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  char *uri;

  uri = flickcurl_photo_id_as_short_uri(photo_id);
  if(uri) {
    fprintf(stderr, "%s: Short URI for photo ID %s is %s\n",
            program, photo_id, uri);
    free(uri);
  } else {
    fprintf(stderr, "%s: Failed to get short URI for photo ID %s\n",
            program, photo_id);
    return 1;
  }
  return 0;
  
}


static int
command_blogs_getServices(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_blog_service** blog_services;
  
  blog_services = flickcurl_blogs_getServices(fc);
  if(blog_services) {
    int i;
    for (i = 0; blog_services[i]; i++) {
      flickcurl_blog_service* blog_service = blog_services[i];
      fprintf(stderr, "%d) blog service: id %s  name '%s'\n", i,
              blog_service->id, blog_service->name);
    }
    flickcurl_free_blog_services(blog_services);
  }
  return 0;
}


static int
command_places_getTopPlacesList(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_place** places = NULL;
  flickcurl_place_type place_type;
  const char* date;
  int woe_id = -1;
  const char* place_id = NULL;
  
  place_type = flickcurl_get_place_type_by_label(argv[1]);
  date = argv[2];

  if(argc > 3) {
    if(strcmp(argv[3], "-"))
       woe_id = atoi(argv[3]);
    if(argc > 4) {
      place_id = argv[4];
    }
  }

  places = flickcurl_places_getTopPlacesList(fc, place_type, date, woe_id,
                                             place_id);
  if(places) {
    int i;
    for(i = 0; places[i]; i++) {
      fprintf(stderr, "Place Result #%d\n", i);
      command_print_place(places[i], NULL, NULL, 1);
    }
    flickcurl_free_places(places);
  }
  
  return (places == NULL);
}


static int
command_source_uri_to_photoid(flickcurl* fc, int argc, char *argv[])
{
  char *uri = argv[1];
  char *photo_id;

  photo_id = flickcurl_source_uri_as_photo_id(uri);
  if(!photo_id) {
    fprintf(stderr, "%s: Failed to get photo ID for source URI %s\n",
            program, uri);
    return 1;
  }

  fprintf(stderr, "%s: Photo ID for source URI %s is %s\n",
          program, uri, photo_id);
  free(photo_id);

  return 0;
}


static int
command_people_getPhotosOf(flickcurl* fc, int argc, char *argv[])
{
  char *user_id = argv[1];
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc >1) {
    list_params.per_page = parse_page_param(argv[2]);
    if(argc >2) {
      list_params.page = parse_page_param(argv[3]);
      if(argc >3) {
        list_params.format = argv[4];
      }
    }
  }

  photos_list = flickcurl_people_getPhotosOf_params(fc, user_id,
                                                    &list_params);
  if(!photos_list)
    return 1;

  if(verbose)
    fprintf(stderr, "%s: Photos of user %s (per_page %d  page %d):\n",
            program, user_id, list_params.per_page, list_params.page);

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}


static int
command_photos_people_add(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  char *user_id = argv[2];
  int person_x = atoi(argv[3]);
  int person_y = atoi(argv[4]);
  int person_w = atoi(argv[5]);
  int person_h = atoi(argv[6]);

  return flickcurl_photos_people_add(fc, photo_id, user_id,
                                     person_x, person_y, person_w, person_h);
}


static int
command_photos_people_delete(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  char *user_id = argv[2];

  return flickcurl_photos_people_delete(fc, photo_id, user_id);
}


static int
command_photos_people_deleteCoords(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  char *user_id = argv[2];

  return flickcurl_photos_people_deleteCoords(fc, photo_id, user_id);
}


static int
command_photos_people_editCoords(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  char *user_id = argv[2];
  int person_x = atoi(argv[3]);
  int person_y = atoi(argv[4]);
  int person_w = atoi(argv[5]);
  int person_h = atoi(argv[6]);

  return flickcurl_photos_people_editCoords(fc, photo_id, user_id,
                                            person_x, person_y,
                                            person_w, person_h);
}


static int
command_photos_people_getList(flickcurl* fc, int argc, char *argv[])
{
  char *photo_id = argv[1];
  int i;
  flickcurl_person** persons;

  persons = flickcurl_photos_people_getList(fc, photo_id);

  if(!persons)
    return 1;

  for(i = 0; persons[i]; i++)
    command_print_person(persons[i]);
  
  flickcurl_free_persons(persons);
  return 0;
}


static int
command_galleries_addPhoto(flickcurl* fc, int argc, char *argv[])
{
  const char* gallery_id = argv[1];
  const char* photo_id = argv[2];
  const char* comment_text = argv[3];

  return flickcurl_galleries_addPhoto(fc, gallery_id, photo_id, comment_text);
}


static void
command_print_gallery(flickcurl_gallery* g)
{
    fprintf(stderr,
            "id %s  url %s  owner %s\n"
            "  date create %d  date update %d\n"
            "  count of photos %d  count of videos %d\n"
            "  title '%s'\n"
            "  description '%s'\n"
            ,
            g->id, g->url, g->owner,
            g->date_create, g->date_update,
            g->count_photos, g->count_videos,
            g->title, g->description);
    fputs("  primary ", stderr);
    command_print_photo(g->primary_photo);
}


static int
command_galleries_getList(flickcurl* fc, int argc, char *argv[])
{
  const char* user_id = argv[1];
  int per_page = 10;
  int page = 0;
  flickcurl_gallery** galleries;
  int i;
  
  if(argc > 1) {
    per_page = parse_page_param(argv[2]);
    if(argc > 2) {
      page = parse_page_param(argv[3]);
    }
  }

  galleries = flickcurl_galleries_getList(fc, user_id, per_page, page);
  if(!galleries)
    return 1;
  for(i = 0; galleries[i]; i++) {
    fprintf(stderr, "%s: Gallery %d\n", program, i);
    command_print_gallery(galleries[i]);
  }

  flickcurl_free_galleries(galleries);
  return 0;
}


static int
command_galleries_getListForPhoto(flickcurl* fc, int argc, char *argv[])
{
  const char* photo_id = argv[1];
  int per_page = 10;
  int page = 0;
  flickcurl_gallery** galleries;
  int i;

  if(argc > 2) {
    per_page = parse_page_param(argv[2]);
    if(argc > 3){
      page = parse_page_param(argv[3]);
    }
  }

  galleries = flickcurl_galleries_getListForPhoto(fc, photo_id, per_page, page);
  if(!galleries)
    return 1;
  for(i = 0; galleries[i]; i++) {
    fprintf(stderr, "%s: Gallery %d\n", program, i);
    command_print_gallery(galleries[i]);
  }

  flickcurl_free_galleries(galleries);
  return 0;
};


static void
command_print_stat(flickcurl_stat* s)
{
  fprintf(stderr, "  Views %d  Comments %d  Favorites %d", 
          s->views, s->comments, s->favorites);
  if(s->name)
    fprintf(stderr, "  Name %s", s->name);
  if(s->url)
    fprintf(stderr, "  Url %s", s->url);
  if(s->searchterms)
    fprintf(stderr, "  Search terms %s", s->searchterms);
  fputc('\n', stderr);
}

static void
command_print_stats(flickcurl_stat** stats)
{
  int i;

  for(i = 0; stats[i]; i++) {
    fprintf(stderr, "%s: Statistic %d\n", program, i);
    command_print_stat(stats[i]);
  }

}


static int
command_stats_getCollectionDomains(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* collection_id = NULL;
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 1) {
    collection_id = argv[2];
    if(argc > 2) {
      per_page = parse_page_param(argv[3]);
      if(argc > 3) {
        page = parse_page_param(argv[4]);
      }
    }
  }

  stats = flickcurl_stats_getCollectionDomains(fc, date, collection_id,
                                               per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Collection domain stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getCollectionReferrers(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* domain = argv[2];
  const char* collection_id = NULL;
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 2) {
    collection_id = argv[3];
    if(argc > 3) {
      per_page = parse_page_param(argv[4]);
      if(argc > 4) {
        page = parse_page_param(argv[5]);
      }
    }
  }

  stats = flickcurl_stats_getCollectionReferrers(fc, date, domain,
                                                 collection_id, per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Collection referrers stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getCollectionStats(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* collection_id = argv[2];
  int views;

  views = flickcurl_stats_getCollectionStats(fc, date, collection_id);
  if(views < 0)
    return 1;

  fprintf(stderr, "%s: Collection view stats: %d\n", program, views);
  
  return 0;
}


static int
command_stats_getPhotoDomains(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* photo_id = NULL;
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 1) {
    photo_id = argv[2];
    if(argc > 2) {
      per_page = parse_page_param(argv[3]);
      if(argc > 3) {
        page = parse_page_param(argv[4]);
      }
    }
  }

  stats = flickcurl_stats_getPhotoDomains(fc, date, photo_id, per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Photo domains stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getPhotoReferrers(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* domain = argv[2];
  const char* photo_id = NULL;
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 2) {
    photo_id = argv[3];
    if(argc > 3) {
      per_page = parse_page_param(argv[4]);
      if(argc > 4) {
        page = parse_page_param(argv[5]);
      }
    }
  }

  stats = flickcurl_stats_getPhotoReferrers(fc, date, domain, photo_id,
                                            per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Photo referrers stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getPhotoStats(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* photo_id = argv[2];
  flickcurl_stat* stat1;

  stat1 = flickcurl_stats_getPhotoStats(fc, date, photo_id);
  if(!stat1)
    return 1;
  
  fprintf(stderr, "%s: Photo %s on date %s statistics:\n", program,
          photo_id, date);
  command_print_stat(stat1);
  flickcurl_free_stat(stat1);

  return 0;
}


static int
command_stats_getPhotosetDomains(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* photoset_id = NULL;
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 1) {
    photoset_id = argv[2];
    if(argc > 2) {
      per_page = parse_page_param(argv[3]);
      if(argc > 3) {
        page = parse_page_param(argv[4]);
      }
    }
  }

  stats = flickcurl_stats_getPhotosetDomains(fc, date, photoset_id, 
                                             per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Photoset domains stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getPhotosetReferrers(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* domain = argv[2];
  const char* photoset_id = NULL;
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 2) {
    photoset_id = argv[3];
    if(argc > 3) {
      per_page = parse_page_param(argv[4]);
      if(argc > 4) {
        page = parse_page_param(argv[5]);
      }
    }
  }

  stats = flickcurl_stats_getPhotosetReferrers(fc, date, domain, photoset_id,
                                               per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Photoset referrers stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getPhotosetStats(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* photoset_id = argv[2];
  int views;

  views = flickcurl_stats_getPhotosetStats(fc, date, photoset_id);
  if(views < 0)
    return 1;

  fprintf(stderr, "%s: Photoset view stats: %d\n", program, views);
  
  return 0;
}


static int
command_stats_getPhotostreamDomains(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 1) {
    per_page = parse_page_param(argv[2]);
    if(argc > 2) {
      page = parse_page_param(argv[3]);
    }
  }

  stats = flickcurl_stats_getPhotostreamDomains(fc, date, per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Photostream domains stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getPhotostreamReferrers(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* domain = argv[2];
  int per_page = -1;
  int page = 0;
  flickcurl_stat** stats;

  if(argc > 2) {
    per_page = parse_page_param(argv[3]);
    if(argc > 3) {
      page = parse_page_param(argv[4]);
    }
  }

  stats = flickcurl_stats_getPhotostreamReferrers(fc, date, domain,
                                                  per_page, page);
  if(!stats)
    return 1;
  
  if(verbose)
    fprintf(stderr, "%s: Photostream referrers stats (per_page %d  page %d):\n",
            program, per_page, page);

  command_print_stats(stats);
  flickcurl_free_stats(stats);

  return 0;
}


static int
command_stats_getPhotostreamStats(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  int views;

  views = flickcurl_stats_getPhotostreamStats(fc, date);
  if(views < 0)
    return 1;
  
  fprintf(stderr, "%s: Photostream view stats: %d\n", program, views);
  
  return 0;
}


static int
command_stats_getPopularPhotos(flickcurl* fc, int argc, char *argv[])
{
  char* date = argv[1];
  const char* sort = NULL;
  int per_page = -1;
  int page = 0;
  const char *extras = NULL;
  flickcurl_photo** photos;
  int i;
  
  if(argc > 1) {
    sort = argv[2];
    if(argc > 2) {
      per_page = parse_page_param(argv[3]);
      if(argc > 3) {
        page = parse_page_param(argv[4]);
        if(argc > 4) {
          extras = argv[5];
        }
      }
    }
  }

  photos = flickcurl_stats_getPopularPhotos(fc, date, sort,
                                            per_page, page, extras);
  if(!photos)
    return 1;

  fprintf(stderr, "%s: Popular photos:\n", program);
  for(i = 0; photos[i]; i++) {
    fprintf(stderr, "%s: popular photo %d\n", program, i);
    command_print_photo(photos[i]);
  }
  flickcurl_free_photos(photos);

  return 0;
}


static int
command_stats_getTotalViews(flickcurl* fc, int argc, char *argv[])
{
  char* date = NULL;
  flickcurl_view_stats* view_stats;

  if(argc > 0)
    date = argv[1];
  
  view_stats = flickcurl_stats_getTotalViews(fc, date);
  if(!view_stats)
    return 1;
  
  fprintf(stderr, "%s: Total view stats\n", program);
  fprintf(stderr,
          "  Total: %d\n  Photos: %d\n  Photostreams: %d\n  Sets: %d\n  Collections: %d\n",
          view_stats->total, view_stats->photos, view_stats->photostreams,
          view_stats->sets, view_stats->collections);
  
  flickcurl_free_view_stats(view_stats);
  return 0;
}



static int
command_people_getPhotos(flickcurl* fc, int argc, char *argv[])
{
  int usage = 0;
  char *user_id;
  int safe_search = -1;
  const char* min_upload_date = NULL;
  const char* max_upload_date = NULL;
  const char* min_taken_date = NULL;
  const char* max_taken_date = NULL;
  int content_type = -1;
  int privacy_filter = -1;
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc = 0;
  
  flickcurl_photos_list_params_init(&list_params);

  argv++; argc--;
  user_id = argv[0];
  argv++; argc--;

  while(!usage && argc) {
    char* field = argv[0];
    argv++; argc--;

    if(!strcmp(field, "safe-search")) {
      safe_search = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "min-upload-date")) {
      min_upload_date = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "max-upload-date")) {
      max_upload_date = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "min-taken-date")) {
      min_taken_date = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "max-taken-date")) {
      max_taken_date = argv[0];
      argv++; argc--;
    } else if(!strcmp(field, "content-type")) {
      content_type = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "privacy-filter")) {
      privacy_filter = atoi(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "per-page")) {
      /* int: default 100, max 500 */
      list_params.per_page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "page")) {
      /* int: default 1 */
      list_params.page = parse_page_param(argv[0]);
      argv++; argc--;
    } else if(!strcmp(field, "format")) {
      list_params.format = argv[0];
      argv++; argc--;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, argv[0]);
      usage = 1;
    }
  }

  if(usage)
    goto tidy;
  
  photos_list = flickcurl_people_getPhotos_params(fc, user_id,
                                                  safe_search,
                                                  min_upload_date,
                                                  max_upload_date,
                                                  min_taken_date,
                                                  max_taken_date,
                                                  content_type,
                                                  privacy_filter,
                                                  &list_params);
  if(!photos_list)
    return 1;

  if(verbose)
    fprintf(stderr, "%s: Photos of user %s (per_page %d  page %d):\n",
            program, user_id, list_params.per_page, list_params.page);

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  tidy:
  return rc;
}


static int
command_galleries_create(flickcurl* fc, int argc, char *argv[])
{
  const char* title = argv[1];
  const char* description = NULL;
  const char* primary_photo_id = NULL;
  char* url = NULL;
  char* id;

  if(argc > 1) {
    description = argv[2];
    if(argc > 2)
      primary_photo_id = argv[3];
  }
  
  id = flickcurl_galleries_create(fc, title, description, primary_photo_id,
                                  &url);
  if(!id)
    return 1;
  fprintf(stderr, "%s: Gallery %s created with URL %s\n", program, id, url);
  free(url);
  free(id);
  return 0;
}


static int
command_galleries_editMeta(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_galleries_editMeta(fc, argv[1], argv[2], argv[3]);
}


static int
command_galleries_editPhoto(flickcurl* fc, int argc, char *argv[])
{
  const char* gallery_id = argv[1];
  const char* primary_photo_id = argv[2];
  const char* new_comment = argv[3];
  
  return flickcurl_galleries_editPhoto(fc, gallery_id, primary_photo_id,
                                       new_comment);
}


static int
command_galleries_editPhotos(flickcurl* fc, int argc, char *argv[])
{
  const char* photoset_id = argv[1];
  const char* primary_photo_id = argv[2];
  char** photo_ids = flickcurl_array_split(argv[3], ',');
  int rc;
  
  rc = flickcurl_galleries_editPhotos(fc, photoset_id, primary_photo_id,
                                    (const char**)photo_ids);
  flickcurl_array_free(photo_ids);
  return rc;
}


static int
command_galleries_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_gallery* gallery = NULL;
  const char* gallery_id = argv[1];
  
  gallery = flickcurl_galleries_getInfo(fc, gallery_id);
  if(gallery) {
    command_print_gallery(gallery);
    flickcurl_free_gallery(gallery);
  }
  
  return (gallery == NULL);
}


static int
command_galleries_getPhotos(flickcurl* fc, int argc, char *argv[])
{
  const char* gallery_id = argv[1];
  flickcurl_photos_list* photos_list = NULL;
  flickcurl_photos_list_params list_params;
  int rc;
  
  flickcurl_photos_list_params_init(&list_params);

  if(argc > 2) {
    list_params.extras = argv[2];
    if(argc > 3) {
      list_params.per_page = parse_page_param(argv[3]);
      if(argc > 4) {
        list_params.page = parse_page_param(argv[4]);
        if(argc > 5) {
          list_params.format = argv[5];
        }
      }
    }
  }
  
  photos_list = flickcurl_galleries_getPhotos_params(fc, gallery_id,
                                                     &list_params);
  if(!photos_list)
    return 1;

  if(verbose)
    fprintf(stderr, "%s: Gallery %s photos (per_page %d  page %d):\n",
            program, gallery_id, list_params.per_page, list_params.page);

  rc = command_print_photos_list(fc, photos_list, output_fh, "Photo");
  flickcurl_free_photos_list(photos_list);

  return rc;
}




static int
command_photosets_removePhotos(flickcurl* fc, int argc, char *argv[])
{
  char** photoset_ids = flickcurl_array_split(argv[2], ',');
  int rc;
  
  rc = flickcurl_photosets_removePhotos(fc, argv[1],
                                        (const char**)photoset_ids);
  flickcurl_array_free(photoset_ids);
  return rc;
}


static int
command_photosets_reorderPhotos(flickcurl* fc, int argc, char *argv[])
{
  char** photoset_ids = flickcurl_array_split(argv[2], ',');
  int rc;
  
  rc = flickcurl_photosets_reorderPhotos(fc, argv[1],
                                         (const char**)photoset_ids);
  flickcurl_array_free(photoset_ids);
  return rc;
}


static int
command_photosets_setPrimaryPhoto(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_photosets_setPrimaryPhoto(fc, argv[1], argv[2]);
}


typedef struct {
  const char*     name;
  const char*     args;
  const char*     description;
  command_handler handler;
  int             min;
  int             max;
} flickcurl_cmd;


#ifdef FLICKCURL_MAINTAINER
static int flickcurl_cmd_compare(const void *a, const void *b)
{
  flickcurl_cmd* a_cmd = (flickcurl_cmd*)a;
  flickcurl_cmd* b_cmd = (flickcurl_cmd*)b;
  return strcmp(a_cmd->name, b_cmd->name);
}

#endif


static flickcurl_cmd commands[] = {
  /* {fn name, 
   *  args desc, fn description
   *  handler, min args, max args },
   */
  {"auth.checkToken",
   "TOKEN", "Get the credentials attached to an authentication TOKEN.",
   command_auth_checkToken, 1, 1},
  {"auth.getFrob",
   "", "Get a frob to be used during authentication.",
   command_auth_getFrob, 0, 0},
  {"auth.getFullToken",
   "MINI-TOKEN", "Get the full authentication token for MINI-TOKEN.",
   command_auth_getFullToken, 0, 0},
  {"auth.getToken",
   "TOKEN", "Get the auth token for the FROB, if one has been attached.",
   command_auth_getToken, 0, 0},

  {"activity.userComments",
   "[PER-PAGE [PAGE]]", "Get photos commented on by the caller.",
   command_activity_userComments, 0, 2},
  {"activity.userPhotos",
   "[TIMEFRAME [PER-PAGE [PAGE]]]", "Get recent activity on the caller's photos.",
   command_activity_userPhotos, 0, 3},

  {"blogs.getList",
   "", "Get a list of configured blogs for the calling user.",
   command_blogs_getList, 0, 0},
  {"blogs.getServices",
   "", "Get a list of blog services.",
   command_blogs_getServices, 0, 0},
  {"blogs.postPhoto",
   "BLOG-ID PHOTO-ID TITLE DESCRIPTION [BLOG-PASSWORD]", "Post PHOTO-ID to blog BLOG-ID with TITLE, DESCRIPTION and optional password.",
   command_blogs_postPhoto, 4, 5},

  {"commons.getInstitutions",
   "", "Get list of institutions", 
   command_commons_getInstitutions,  0, 0},

  {"collections.getInfo",
   "COLLECTION-ID", "Get information on collection COLLECTION-ID", 
   command_collections_getInfo, 1, 1},
  {"collections.getTree",
   "[COLLECTION-ID|- [USER-ID|-]]", "Get tree of collections COLLECTION-ID for USER-ID", 
   command_collections_getTree, 0, 2},

  {"contacts.getList",
   "[FILTER [PER-PAGE [PAGE]]]", "Get a list of contacts with optional FILTER", 
   command_contacts_getList, 0, 3},
  {"contacts.getListRecentlyUploaded",
   "[DATE-LAST-UPLOAD [FILTER]]", "Get a list of recent uploading contacts since DATE-LAST-UPLOAD with optional FILTER", 
   command_contacts_getListRecentlyUploaded, 0, 2},

  {"favorites.add",
   "PHOTO-ID", "Adds PHOTO-ID to the current user's favorites.",
   command_favorites_add, 1, 1},
  {"favorites.getList",
   "USER-NSID [[PER-PAGE] [PAGE [FORMAT]]]", "Get a list of USER-NSID's favorite photos.",
   command_favorites_getList, 1, 4},
  {"favorites.getPublicList",
   "USER-NSID [[PER-PAGE] [PAGE [FORMAT]]]", "Get a list of USER-NSID's favorite public photos.",
   command_favorites_getPublicList, 1, 4},
  {"favorites.remove",
   "PHOTO-ID", "Removes PHOTO-ID to the current user's favorites.",
   command_favorites_remove, 1, 1},

  {"galleries.addPhoto",
   "GALLERY-ID PHOTO-ID TEXT", "Add photo PHOTO-ID to galleries GALLERY-ID with TEXT",
   command_galleries_addPhoto, 3, 3},
  {"galleries.create",
   "TITLE [DESCRIPTION [PRIMARY-PHOTO-ID]", "Create a new galleries with TITlE, DESCRIPTION and PRIMARY-PHOTO_ID.",
   command_galleries_create, 1, 3},
  {"galleries.editMeta",
   "GALLERY-ID TITLE [DESCRIPTION]", "Set the TITLE and/or DESCRIPTION of a GALLERY-ID.",
   command_galleries_editMeta, 2, 3},
  {"galleries.editPhoto",
   "GALLERY-ID PHOTO-ID COMMENT", "Set the COMMENT for PHOTO-ID in GALLERY-ID.",
   command_galleries_editPhoto, 3, 3},
  {"galleries.editPhotos",
   "GALLERY-ID PRIMARY-PHOTO-ID PHOTO-IDS,...", "Set the PHOTO-IDs of a GALLERY-ID and PRIMARY-PHOTO-ID.",
   command_galleries_editPhotos, 3, 3},
  {"galleries.getInfo",
   "GALLERY-ID", "Get information about GALLERY-ID.",
   command_galleries_getInfo, 1, 1},
  {"galleries.getList",
   "USER-ID [PER-PAGE [PAGE]]", "Get list of galleries for a USER-ID with optional paging",
   command_galleries_getList, 1, 3},
  {"galleries.getListForPhoto",
   "PHOTO-ID [PER-PAGE [PAGE]]", "Get list of galleries PHOTO-ID appears in with optional paging",
   command_galleries_getListForPhoto, 1, 3},
  {"galleries.getPhotos",
   "GALLERY-ID [EXTRAS [PER-PAGE [PAGE [FORMAT]]]]", "Get the list of photos in GALLERY-ID with options.",
   command_galleries_getPhotos, 1, 5},

  {"groups.browse",
   "[CAT-ID]", "Browse groups below category CAT-ID (or root).",
   command_groups_browse, 0, 1},
  {"groups.getInfo",
   "GROUP-ID [LANG]", "Get information on group GROUP-ID with language LANG.",
   command_groups_getInfo, 1, 2},
  {"groups.search",
   "TEXT [PER-PAGE [PAGE]]", "Search for groups matching TEXT paging PER-PAGE and PAGE.",
   command_groups_search, 1, 3},

  {"groups.members.getList",
   "GROUP-ID [MEMBER-TYPES [PER-PAGE [PAGE]]]", "Get list of MEMBER-TYPES types members of group GROUP-ID.",
   command_groups_members_getList, 1, 4},

  {"groups.pools.add",
   "PHOTO-ID GROUP-ID", "Add PHOTO-ID in GROUP-ID pool.",
   command_groups_pools_add, 2, 2},
  {"groups.pools.getContext",
   "PHOTO-ID GROUP-ID", "Get next and previous photos for PHOTO-ID in GROUP-ID pool.",
   command_groups_pools_getContext, 2, 2},
  {"groups.pools.getGroups",
   "[PAGE [PER-PAGE]]", "Get list of groups a user can add to.",
   command_groups_pools_getGroups, 0, 2},
  {"groups.pools.getPhotos",
   "GROUP-ID [PAGE [PER-PAGE [FORMAT]]]", "Get list of photos in GROUP-ID.",
   command_groups_pools_getPhotos, 1, 4},
  {"groups.pools.remove",
   "PHOTO-ID GROUP-ID", "Remove PHOTO-ID from group GROUP-ID.",
   command_groups_pools_remove, 2, 2},

  {"interestingness.getList",
   "[PARAMS]", "Get interesting photos with optional parameters\n  date DATE  extras EXTRAS  per-page PER-PAGE  page PAGE  format FORMAT", 
   command_interestingness_getList,  1, 0},

  {"machinetags.getNamespaces",
   "[PREDICATE [PER-PAGE [PAGE]]]", "Get a list of namespaces with optional PREDICATE", 
   command_machinetags_getNamespaces, 0, 3},
  {"machinetags.getPairs",
   "[NAMESPACE [PREDICATE [PER-PAGE [PAGE]]]]", "Get a list of unique NAMESPACE and PREDICATE pairs", 
   command_machinetags_getPairs, 0, 4},
  {"machinetags.getPredicates",
   "[NAMESPACE [PER-PAGE [PAGE]]]", "Get a list of unique predicates optionally by NAMESPACE", 
   command_machinetags_getPredicates, 0, 3},
  {"machinetags.getValues",
   "NAMESPACE PREDICATE [PER-PAGE [PAGE]]", "Get a list of unique values for a NAMESPACE and PREDICATE", 
   command_machinetags_getValues, 2, 4},
  {"machinetags.getRecentValues",
   "[NAMESPACE|- [PREDICATE|- [ADDED-SINCE]]]", "Get a list of recent machinetags for NAMESPACE and PREDICATE since ADDED-SINCE", 
   command_machinetags_getRecentValues, 0, 3},

  {"panda.getList",
   "", "get the current list of pandas", 
   command_panda_getList,  0, 0},
  {"panda.getPhotos",
   "PANDA", "ask a PANDA for a list of recent public and safe photos", 
   command_panda_getPhotos,  1, 1},

  {"people.findByEmail",
   "EMAIL", "get a user's NSID from their EMAIL address", 
   command_people_findByEmail,  1, 1},
  {"people.findByUsername",
   "USERNAME", "get a user's NSID from their USERNAME", 
   command_people_findByUsername,  1, 1},
  {"people.getInfo",
   "USER-NSID", "Get information about one person with id USER-NSID", 
   command_people_getInfo,  1, 1},
  {"people.getPhotos",
   "USER-NSID", "Get photos from user USER-NSID with optional parameters\n  safe-search 1-3  min-upload-date DATE  max-upload-date DATE\n  min-taken date DATE  max-taken-date DATE  content-type 1-7\n  privacy-filter 1-5  per-page PER-PAGE  page PAGE  format FORMAT", 
   command_people_getPhotos,  1, 0},
  {"people.getPhotosOf",
   "USER-NSID [PER-PAGE [PAGE [FORMAT]]]", "Get public photos of a user USER-NSID", 
   command_people_getPhotosOf,  1, 4},
  {"people.getPublicGroups",
   "USER-NSID", "Get list of public groups a user is amember of", 
   command_people_getPublicGroups,  1, 1},
  {"people.getPublicPhotos",
   "USER-NSID [PER-PAGE [PAGE [FORMAT]]]", "Get public photos for a user USER-NSID", 
   command_people_getPublicPhotos,  1, 4},
  {"people.getUploadStatus",
   "", "Get calling user upload status", 
   command_people_getUploadStatus,  0, 0},

  {"photos.addTags",
   "PHOTO-ID TAGS", "Add TAGS to a PHOTO-ID.",
   command_photos_addTags, 2, 2},
  {"photos.delete",
   "PHOTO-ID", "Delete a PHOTO-ID.",
   command_photos_delete, 1, 1},
  {"photos.getAllContexts",
   "PHOTO-ID", "Get all visible sets and pools the PHOTO-ID belongs to.",
   command_photos_getAllContexts, 1, 1},
  {"photos.getContactsPhotos",
   "", "Get a list of recent photos from the calling users' contacts",
   command_photos_getContactsPhotos, 0, 0},
  {"photos.getContactsPublicPhotos",
   "USER-NSID [FORMAT]", "Get a list of recent public photos from USER-NSID's contacts",
   command_photos_getContactsPublicPhotos, 1, 2},
  {"photos.getContext",
   "PHOTO-ID", "Get next and previous photos for a PHOTO-ID in a photostream.",
   command_photos_getContext, 1, 1},
  {"photos.getCounts",
   "DATES TAKEN-DATES", "Get the counts for a set of DATES or TAKEN-DATES.",
   command_photos_getCounts, 0, 2},
  {"photos.getExif",
   "PHOTO-ID", "Get EXIF information about one photo with id PHOTO-ID", 
   command_photos_getExif,  1, 1},
  {"photos.getFavorites",
   "PHOTO-ID [PER-PAGE [PAGE]]", "Get favourites information about one photo with id PHOTO-ID", 
   command_photos_getFavorites,  1, 3},
  {"photos.getInfo",
   "PHOTO-ID", "Get information about one photo with id PHOTO-ID", 
   command_photos_getInfo,  1, 1},
  {"photos.getNotInSet",
   "[PER-PAGE [PAGE [FORMAT]]]", "Get list of photos that are not in any set", 
   command_photos_getNotInSet, 0, 3},
  {"photos.getPerms",
   "PHOTO-ID", "Get a photo viewing and commenting permissions",
   command_photos_getPerms, 1, 1},
  {"photos.getRecent",
   "[PER-PAGE [PAGE [FORMAT]]]", "Get list of recent photos", 
   command_photos_getRecent, 0, 3},
  {"photos.getSizes",
   "PHOTO-ID", "Get sizes of a PHOTO-ID", 
   command_photos_getSizes, 1, 1},
  {"photos.getUntagged",
   "[PER-PAGE [PAGE [FORMAT]]]", "Get list of photos that are not tagged", 
   command_photos_getUntagged, 0, 3},
  {"photos.getWithGeoData",
   "[PER-PAGE [PAGE [FORMAT]]]", "Get list of photos that have geo data", 
   command_photos_getWithGeoData, 0, 3},
  {"photos.getWithoutGeoData",
   "[PER-PAGE [PAGE [FORMAT]]]", "Get list of photos that do not have geo data", 
   command_photos_getWithoutGeoData, 0, 3},
  {"photos.recentlyUpdated",
   "MIN-DATE [PER-PAGE [PAGE [FORMAT]]]", "Get list of photos that were recently updated", 
   command_photos_recentlyUpdated, 1, 4},
  {"photos.removeTag",
   "PHOTO-ID TAG-ID", "Remove a tag TAG-ID from a photo.",
   command_photos_removeTag, 2, 2},
  {"photos.search",
   "[PARAMS] tags TAGS...", "Search for photos/videos with many optional parameters\n        user USER  tag-mode any|all  text TEXT\n        (min|max)-(upload|taken)-date DATE\n        license LICENSE  privacy PRIVACY  bbox a,b,c,d\n        sort date-(posted|taken)-(asc|desc)|interestingness-(desc|asc)|relevance\n        accuracy 1-16  safe-search 1-3  type 1-4\n        machine-tags TAGS  machine-tag-mode any|all\n        group-id ID  place-id ID  extras EXTRAS\n        per-page PER-PAGE  page PAGES\n        media all|photos|videos  has-geo\n        lat LAT lon LON radius RADIUS radius-units km|mi\n        contacts (all|ff)\n        format FORMAT  woeid WOEID\n        geo-context 1-2\n        in-commons  in-gallery",
   command_photos_search, 1, 0},
  {"photos.setContentType",
   "PHOTO-ID TYPE", "Set photo TYPE to one of 'photo', 'screenshot' or 'other'",
   command_photos_setContentType, 2, 2},
  {"photos.setDates",
   "PHOTO-ID POSTED TAKEN GRANULARITY", "Set a photo POSTED date, TAKEN date with GRANULARITY",
   command_photos_setDates, 4, 4},
  {"photos.setMeta",
   "PHOTO-ID TITLE DESCRIPTION", "Set a photo TITLE and DESCRIPTION",
   command_photos_setMeta, 3, 3},
  {"photos.setPerms",
   "PHOTO-ID IS-PUBLIC IS-FRIEND IS-FAMILY PERM-COMMENT PERM-ADDMETA", "Set a photo viewing and commenting permissions",
   command_photos_setPerms, 6, 6},
  {"photos.setSafetyLevel",
   "PHOTO-ID SAFETY-LEVEL HIDDEN", "Set a photo's SAFETY-LEVEL and HIDDEN flag",
   command_photos_setSafetyLevel, 3, 3},
  {"photos.setTags",
   "PHOTO-ID TAGS", "Set the tags for a PHOTO-ID to TAGS.",
   command_photos_setTags, 2, 2},

  {"photos.comments.addComment",
   "PHOTO-ID TEXT", "Add a photo comment TEXT to PHOTO-ID.",
   command_photos_comments_addComment, 2, 2},
  {"photos.comments.deleteComment",
   "COMMENT-ID", "Delete a photo comment COMMENT-ID.",
   command_photos_comments_deleteComment, 1, 1},
  {"photos.comments.editComment",
   "COMMENT-ID TEXT", "Edit a photo comment COMMENT-ID to have new TEXT.",
   command_photos_comments_editComment, 2, 2},
  {"photos.comments.getList",
   "PHOTO-ID", "Get the comments for a photo PHOTO-ID.",
   command_photos_comments_getList, 1, 1},
  {"photos.comments.getRecentForContacts",
   "[PARAMS]", "Get the list of photos for user contacts with recent comments\n      since DATE-LAST-COMMENT filter CONTACTS-FILTER\n      per-page PER-PAGE page PAGE format FORMAT",
   command_comments_getRecentForContacts, 0, 0},

  {"photos.geo.getLocation",
   "PHOTO-ID", "Get the geo location for a photo PHOTO-ID.",
   command_photos_geo_getLocation, 1, 1},
  {"photos.geo.getPerms",
   "PHOTO-ID", "Get the geo perms for a photo PHOTO-ID.",
   command_photos_geo_getPerms, 1, 1},
  {"photos.geo.removeLocation",
   "PHOTO-ID", "Remove the location for a photo PHOTO-ID.",
   command_photos_geo_removeLocation, 1, 1},
  {"photos.geo.setLocation",
   "PHOTO-ID LAT LONG ACCURACY", "Set the location for a photo PHOTO-ID.",
   command_photos_geo_setLocation, 4, 4},
  {"photos.geo.setPerms",
   "PHOTO-ID IS-PUBLIC IS-CONTACT IS-FRIEND IS-FAMILY", "Set the geo perms for a photo PHOTO-ID.",
   command_photos_geo_setPerms, 5, 5},

  {"photos.licenses.getInfo",
   "", "Get list of available photo licenses", 
   command_photos_licenses_getInfo,  0, 0},
  {"photos.licenses.setLicense",
   "PHOTO-ID LICENSE-ID", "Get photo PHOTO-ID license to LICENSE-ID", 
   command_photos_licenses_setLicense,  2, 2},

  {"photos.notes.add",
   "PHOTO-ID X Y W H TEXT", "Add a note (X, Y, W, H, TEXT) to a photo with id PHOTO-ID", 
   command_photos_notes_add,  6, 6},
  {"photos.notes.delete",
   "NOTE-ID", "Delete a note with id NOTE-ID", 
   command_photos_notes_delete,  1, 1},
  {"photos.notes.edit",
   "NOTE-ID X Y W H TEXT", "Edit note NOTE-ID to (X, Y, W, H, TEXT)", 
   command_photos_notes_edit,  6, 6},

  {"photos.people.add",
   "PHOTO-ID USER-ID X Y W H", "Mark USER-ID appearing in PHOTO-ID at (X, Y, W, H)", 
   command_photos_people_add,  6, 6},
  {"photos.people.delete",
   "PHOTO-ID USER-ID", "Mark USER-ID as not appearing in PHOTO-ID", 
   command_photos_people_delete,  2, 2},
  {"photos.people.deleteCoords",
   "PHOTO-ID USER-ID", "Mark USER-ID as not appearing at coordinates in PHOTO-ID", 
   command_photos_people_deleteCoords,  2, 2},
  {"photos.people.editCoords",
   "PHOTO-ID USER-ID X Y W H", "Update USER-ID appearing in PHOTO-ID to coords (X, Y, W, H)", 
   command_photos_people_editCoords,  6, 6},
  {"photos.people.getList",
   "PHOTO-ID", "Get list of users appearing in PHOTO-ID", 
   command_photos_people_getList,  1, 1},

  {"photos.transform.rotate",
   "PHOTO-ID DEGREES", "Rotate PHOTO-ID by 90/180/270 DEGREES", 
   command_photos_transform_rotate,  2, 2},

  {"photos.upload.checkTickets",
   "TICKET-IDS...", "Get the status of upload TICKET-IDS", 
   command_photos_upload_checkTickets,  1, 1},

  {"photosets.addPhoto",
   "PHOTOSET-ID PHOTO-ID", "Add PHOTO-ID to a PHOTOSET-ID.",
   command_photosets_addPhoto, 2, 2},
  {"photosets.create",
   "TITLE DESCRIPTION PRIMARY-PHOTO-ID", "Create a photoset with TITLE, DESCRIPTION and PRIMARY-PHOTO-ID.",
   command_photosets_create, 3, 3},
  {"photosets.delete",
   "PHOTOSET-ID", "Delete a photoset with PHOTOSET-ID.",
   command_photosets_delete, 1, 1},
  {"photosets.editMeta",
   "PHOTOSET-ID TITLE DESCRIPTION", "Set the TITLE and/or DESCRIPTION of a PHOTOSET-ID.",
   command_photosets_editMeta, 3, 3},
  {"photosets.editPhotos",
   "PHOTOSET-ID PRIMARY-PHOTO-ID PHOTO-IDS,...", "Set the PHOTO-IDs of a PHOTOSET-ID and PRIMARY-PHOTO-ID.",
   command_photosets_editPhotos, 3, 3},
  {"photosets.getContext",
   "PHOTO-ID PHOTOSET-ID", "Get next and previous photos for PHOTO-ID in PHOTOSET-ID.",
   command_photosets_getContext, 2, 2},
  {"photosets.getInfo",
   "PHOTOSET-ID", "Get information about PHOTOSET-ID.",
   command_photosets_getInfo, 1, 1},
  {"photosets.getList",
   "[USER-NSID]", "Get the list of photosets for the USER-NSID.",
   command_photosets_getList, 0, 1},
  {"photosets.getPhotos",
   "PHOTOSET-ID [EXTRAS [PRIVACY [PER-PAGE [PAGE [FORMAT]]]]]", "Get the list of photos in PHOTOSET-ID with options.",
   command_photosets_getPhotos, 1, 6},
  {"photosets.orderSets",
   "PHOTOSET-IDS...", "Set the order of sets PHOTOSET-IDS.",
   command_photosets_orderSets, 1, 1},
  {"photosets.removePhoto",
   "PHOTOSET-ID PHOTO-ID", "Remove PHOTO-ID from PHOTOSET-ID.",
   command_photosets_removePhoto, 2, 2},
  {"photosets.removePhotos",
   "PHOTOSET-ID PHOTO-IDS...", "Remove PHOTO-IDS from PHOTOSET-ID.",
   command_photosets_removePhotos, 2, 2},
  {"photosets.reorderPhotos",
   "PHOTOSET-ID PHOTO-IDS...", "Reorder PHOTO-IDS from PHOTOSET-ID.",
   command_photosets_reorderPhotos, 2, 2},
  {"photosets.setPrimaryPhoto",
   "PHOTOSET-ID PHOTO-ID", "Set photoset PHOTOSET-ID primary photo to PHOTO-ID.",
   command_photosets_setPrimaryPhoto, 2, 2},

  {"photosets.comments.addComment",
   "PHOTOSET-ID TEXT", "Add a comment TEXT to photoset PHOTOSET-ID.",
   command_photosets_comments_addComment, 2, 2},
  {"photosets.comments.deleteComment",
   "COMMENT-ID", "Delete a photoset comment COMMENT-ID.",
   command_photosets_comments_deleteComment, 1, 1},
  {"photosets.comments.editComment",
   "COMMENT-ID TEXT", "Edit a photoset comment COMMENT-ID to have new TEXT.",
   command_photosets_comments_editComment, 2, 2},
  {"photosets.comments.getList",
   "PHOTOSET-ID", "Get the comments for a photoset PHOTOSET-ID.",
   command_photosets_comments_getList, 1, 1},

  {"places.find",
   "TEXT", "Find places by TEXT query.",
   command_places_find, 1, 1},
  {"places.findByLatLon",
   "LAT LON ACCURACY", "Find places by LAT and LON with ACCURACY 1-16.",
   command_places_findByLatLon, 3, 3},
  {"places.getChildrenWithPhotosPublic",
   "PLACE-ID|- [WOE-ID|-]", "Find child places with public photos by PLACE-ID or WOE-ID",
   command_places_getChildrenWithPhotosPublic, 1, 2},
  {"places.getInfo",
   "PLACE-ID|- [WOE-ID|-]", "Find place by PLACE-ID or WOE-ID",
   command_places_getInfo, 1, 2},
  {"places.getInfoByUrl",
   "URL", "Find place by place URL",
   command_places_getInfoByUrl, 1, 1},
  {"places.getPlaceTypes",
   "URL", "Get a list of available place types",
   command_places_getPlaceTypes, 0, 0},
  {"places.getShapeHistory",
   "PLACE-ID|- [WOE-ID|-]", "Get history of shapes for a place by PLACE-ID or WOE-ID",
   command_places_getShapeHistory, 1, 0},
  {"places.getTopPlacesList",
   "PLACE-TYPE [DATE [WOE-ID|- [PLACE-ID]]]", "Get the top 100 most geotagged places for a DATE (or yesterday).",
   command_places_getTopPlacesList, 1, 4},
  {"places.placesForBoundingBox",
   "PLACE-TYPE MIN-LONG MIN-LAT MAX-LONG MAX-LAT", "Find user places of PLACE-TYPE in bbox.",
   command_places_placesForBoundingBox, 5, 5},
  {"places.placesForContacts",
   "PLACE-TYPE WOE-ID|- PLACE-ID|- THRESHOLD [PARAMS]", "Find top 100 unique places clustered by a given PLACE-TYPE for a\nuser's contacts with optional parameters\n  contacts CONTACTS  min-upload MIN-UPLOAD-DATE  max-upload MAX-UPLOAD-DATE\n  min-taken MIN-TAKEN-DATE  max-taken MAX-TAKEN-DATE",
   command_places_placesForContacts, 4, 0},
  {"places.placesForUser",
   "PLACE-TYPE [WOE-ID] [PLACE-ID [THRESHOLD]]]", "Find user places of PLACE-TYPE.",
   command_places_placesForUser, 1, 4},
  {"places.resolvePlaceId",
   "PLACE-ID  / WOE-ID", "Find places information by PLACE-ID or WOE-ID (number).",
   command_places_resolvePlaceId, 1, 1},
  {"places.resolvePlaceURL",
   "PLACE-URL", "Find places information by PLACE-URL.",
   command_places_resolvePlaceURL, 1, 1},
  {"places.tagsForPlace",
   "PLACE-ID|- [WOE-ID|-]", "Get tags for a place by PLACE-ID or WOE-ID with optional parameters\n  min-upload MIN-UPLOAD-DATE  max-upload MAX-UPLOAD-DATE\n  min-taken MIN-TAKEN-DATE  max-taken MAX-TAKEN-DATE",
   command_places_tagsForPlace, 1, 0},

  {"prefs.getContentType",
   "", "Get default content type preference for user.",
   command_prefs_getContentType, 0, 0},
  {"prefs.getGeoPerms",
   "", "Get default privacy level for geographic info for user.",
   command_prefs_getGeoPerms, 0, 0},
  {"prefs.getHidden",
   "", "Get default hidden preference for user.",
   command_prefs_getHidden, 0, 0},
  {"prefs.getPrivacy",
   "", "Get default privacy preference for user.",
   command_prefs_getPrivacy, 0, 0},
  {"prefs.getSafetyLevel",
   "", "Get default safety level for user.",
   command_prefs_getSafetyLevel, 0, 0},

  {"reflection.getMethods",
   "", "Get API methods",
   command_reflection_getMethods, 0, 0},
  {"reflection.getMethodInfo",
   "NAME", "Get information about an API method NAME",
   command_reflection_getMethodInfo, 1, 1},


  {"stats.getCollectionDomains",
   "DATE [COLLECTION-ID [PER-PAGE [PAGE]]]", "Get collection domains stats",
  command_stats_getCollectionDomains, 1, 4},
  {"stats.getCollectionReferrers",
   "DATE DOMAIN [COLLECTION-ID [PER-PAGE [PAGE]]]", "Get collection referrers stats", 
  command_stats_getCollectionReferrers, 2, 5},
  {"stats.getCollectionStats",
   "DATE COLLECTION-ID", "Get collection view count stats", 
  command_stats_getCollectionStats, 2, 2},
  {"stats.getPhotoDomains",
   "DATE [PHOTO-ID [PER-PAGE [PAGE]]]", "Get photo domains stats", 
  command_stats_getPhotoDomains, 1, 4},
  {"stats.getPhotoReferrers",
   "DATE DOMAIN [PHOTO-ID [PER-PAGE [PAGE]]]", "Get photo referrers stats", 
  command_stats_getPhotoReferrers, 2, 5},
  {"stats.getPhotoStats",
   "DATE PHOTO-ID", "Get photo view count stats", 
  command_stats_getPhotoStats, 2, 2},
  {"stats.getPhotosetDomains",
   "DATE [PHOTOSET-ID [PER-PAGE [PAGE]]]", "Get photoset domains stats", 
  command_stats_getPhotosetDomains, 1, 4},
  {"stats.getPhotosetReferrers",
   "DATE DOMAIN [PHOTOSET-ID [PER-PAGE [PAGE]]]", "Get photoset referrers stats", 
  command_stats_getPhotosetReferrers, 2, 5},
  {"stats.getPhotosetStats",
   "DATE PHOTOSET-ID", "Get photoset view count stats", 
  command_stats_getPhotosetStats, 2, 2},
  {"stats.getPhotostreamDomains",
   "DATE [PER-PAGE [PAGE]]", "Get photostream domains stats", 
  command_stats_getPhotostreamDomains, 1, 3},
  {"stats.getPhotostreamReferrers",
   "DATE DOMAIN [PER-PAGE [PAGE]]", "Get photostream referrers stats", 
  command_stats_getPhotostreamReferrers, 2, 4},
  {"stats.getPhotostreamStats",
   "DATE", "Get photostream view count stats", 
  command_stats_getPhotostreamStats, 1, 1},
  {"stats.getPopularPhotos",
   "[DATE [SORT [PER-PAGE [PAGE [EXTRAS]]]]]", "Get popular photos stats", 
  command_stats_getPopularPhotos, 0, 5},
  {"stats.getTotalViews",
   "[DATE]", "Get total stats", 
  command_stats_getTotalViews, 0, 1},

  {"tags.getClusters",
   "TAG", "Get list of tag clusters for TAG",
   command_tags_getClusters, 1, 1},
  {"tags.getHotList",
   "[PERIOD [COUNT]]", "Get the list of hot tags for the given PERIOD (day, week)",
   command_tags_getHotList, 0, 2},
  {"tags.getListPhoto",
   "PHOTO-ID", "Get the tag list for a PHOTO-ID.",
   command_tags_getListPhoto, 1, 1},
  {"tags.getListUser",
   "[USER-NSID]", "Get the tag list for a USER-NSID (or current user).",
   command_tags_getListUser, 0, 1},
  {"tags.getListUserPopular",
   "[USER-NSID [COUNT]]", "Get the popular tag list for a USER-NSID (or current user).",
   command_tags_getListUserPopular, 0, 2},
  {"tags.getListUserRaw",
   "[TAG]", "Get the raw versions of a TAG (or all tags) for the current user.",
   command_tags_getListUserRaw, 0, 1},
  {"tags.getRelated",
   "TAG", "Get a list of tags 'related' to TAG based on clustered usage analysis.",
   command_tags_getRelated, 1, 1},

  {"test.echo",
   "KEY VALUE", "Test echo API call; echos KEY VALUE",
   command_test_echo,  2, 2},
  {"test.login",
   "", "Test login API call: returns username",
   command_test_login,  0, 0},
  {"test.null",
   "KEY VALUE", "Test null API call: no return",
   command_test_null,  0, 0},

  {"urls.getGroup",
   "GROUP-ID", "Get the url of the group page for GROUP-ID.", 
   command_urls_getGroup,  1, 1},
  {"urls.getUserPhotos",
   "USER-NSID", "Get the url of the photo page for USER-NSID.", 
   command_urls_getUserPhotos,  1, 1},
  {"urls.getUserProfile",
   "USER-NSID", "Get the url of the profile page for USER-NSID.", 
   command_urls_getUserProfile,  1, 1},
  {"urls.lookupGroup",
   "URL", "Get a group NSID from the URL to a group's page or photo pool.", 
   command_urls_lookupGroup,  1, 1},
  {"urls.lookupUser",
   "URL", "Get a user NSID from the URL to a user's photo", 
   command_urls_lookupUser,  1, 1},

  {"upload",
   "FILE [PARAMS...]", "Upload a photo FILE with optional parameters PARAM or PARAM VALUE\n      title TITLE  description DESC  tags TAGS...  friend  public  family", 
   command_upload,  1, 0},

  {"replace",
   "FILE PHOTO-ID [async]", "Replace a photo PHOTO-ID with a new FILE (async)", 
   command_replace,  2, 3},

  {"shorturi",
   "PHOTO-ID", "Get the http://flic.kr short uri for PHOTO-ID", 
   command_short_uri,  1, 1},

  {"getphotoid",
   "IMAGE-URL", "Get the photo id from a raw flickr farm IMAGE-URL", 
   command_source_uri_to_photoid,  1, 1},

  {NULL, 
   NULL, NULL,
   NULL, 0, 0}
};  
  


static const char *title_format_string = "Flickr API utility %s\n";

static const char* config_filename = ".flickcurl.conf";
static const char* config_section = "flickr";


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
  const char* home;
  char config_path[1024];
  int request_delay= -1;
  char *command = NULL;

  output_fh = stdout;
  
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

  if(read_auth) {
    if(!access((const char*)config_path, R_OK)) {
      if(read_ini_config(config_path, config_section, fc,
                         my_set_config_var_handler)) {
        fprintf(stderr, "%s: Failed to read configuration filename %s: %s\n",
                program, config_path, strerror(errno));
        rc = 1;
        goto tidy;
      }
    } else {
        fprintf(stderr, "%s: Configuration file %s not found.\n\n"
"1. Visit http://www.flickr.com/services/api/keys/ and obtain a\n"
"mobile application <API Key>, <Shared Secret> and <Authentication URL>.\n"
"\n"
"2. Create %s in this format:\n"
"[flickr]\n"
"api_key=<API Key>\n"
"secret=<Shared Secret>\n"
"\n"
"3. Visit the <Authentication URL> in a browser to get a <FROB>\n"
"\n"
"4. Call this program with the frob:\n"
"  %s -a <FROB>\n"
"to update the configuration file with the authentication token.\n"
"See http://librdf.org/flickcurl/api/flickcurl-auth.html for full instructions.\n",
                program, config_path, config_path, program);
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

    c = getopt_long (argc, argv, GETOPT_STRING GETOPT_STRING_MORE,
                     long_options, &option_index);
#else
    c = getopt (argc, argv, GETOPT_STRING GETOPT_STRING_MORE);
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
          FILE* fh;
          
          fprintf(stdout, 
                  "%s: Successfully exchanged frob %s for authentication token\n",
                  program, optarg);
          
          flickcurl_set_auth_token(fc, auth_token);
          
          fh = fopen(config_path, "w");
          if(!fh) {
            fprintf(stderr,
                    "%s: Failed to write to configuration file %s: %s\n",
                    program, config_path, strerror(errno));
            rc = 1;
          } else {
            fputs("[flickr]\nauth_token=", fh);
            fputs(flickcurl_get_auth_token(fc), fh);
            fputs("\napi_key=", fh);
            fputs(flickcurl_get_api_key(fc), fh);
            fputs("\nsecret=", fh);
            fputs(flickcurl_get_shared_secret(fc), fh);
            fputs("\n", fh);
            fclose(fh);
            fprintf(stdout, 
                  "%s: Updated configuration file %s with authentication token\n",
                    program, config_path);
            rc = 0;
          }
        }
        goto tidy;

      case 'd':
        if(optarg)
          request_delay = atoi(optarg);
        break;
        
      case 'h':
        help = 1;
        break;

#ifdef FLICKCURL_MAINTAINER
      case 'm':
        /* reusing rc since we set it and return it anyway */
        rc = atoi(optarg);

        if(rc == 0) {
          qsort(commands, (sizeof(commands) / sizeof(flickcurl_cmd))-1,
                sizeof(flickcurl_cmd), flickcurl_cmd_compare);

          for(i = 0; commands[i].name; i++) {
            int d, dc, nl = 1, lastdc= -1;
            printf(".IP \"\\fB%s\\fP \\fI%s\\fP\"\n",
                   commands[i].name, commands[i].args);
            for(d = 0; (dc = commands[i].description[d]); d++) {
              if(nl && dc == ' ') {
                lastdc = dc;
                continue;
              }

              if(dc == ' ' && lastdc == ' ') {
                puts("\n.br");
                do {
                  d++;
                  dc = commands[i].description[d];
                } while(dc == ' ');
              }

              nl = 0;
              if(dc == '\n') {
                puts("\n.br");
                nl = 1;
              } else
                putchar(dc);
              lastdc = dc;
            }
            putchar('\n');
          }

          puts(".SH Extras Fields");
          puts("The \\fBEXTRAS\\fP parameter can take a comma-separated set of the following values");
          for(i = 0; 1; i++) {
            const char* name;
            const char* label;

            if(flickcurl_get_extras_format_info(i, &name, &label))
              break;
            printf(".TP\n\\fB%s\\fP\n%s\n", name, label);
          }

          puts(".SH Photos List Feed Formats");
          puts("The \\fBFORMAT\\fP parameter can take any of the following values");
          for(i = 0; 1; i++) {
            const char* name;
            const char* label;

            if(flickcurl_get_feed_format_info(i, &name, &label, NULL))
              break;
            printf(".TP\n\\fB%s\\fP\n%s\n", name, label);
          }
          rc = 0;
        } else if (rc == 1) {
          puts("<variablelist>");

          for(i = 0; 1; i++) {
            const char* name;
            const char* label;
            
            if(flickcurl_get_extras_format_info(i, &name, &label))
              break;
            printf("  <varlistentry>\n"
                   "    <term>%s</term>\n"
                   "    <listitem><simpara>%s</simpara></listitem>\n"
                   "  </varlistentry>\n", name, label);
          }
          
          puts("</variablelist>");
          rc = 0;
        } else {
          fprintf(stderr, "%s: Unknown maintainer info flag %s / %d", program, 
                  optarg, rc);
          rc = 1;
        }
        goto tidy;
#endif

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
#ifdef FLICKCURL_MAINTAINER
    puts(HELP_TEXT("m", "maintainer TYPE ", "Print formatted fragments for maintainer use, then exit"));
#endif
    puts(HELP_TEXT("o", "output FILE     ", "Write format = FORMAT results to FILE"));
    puts(HELP_TEXT("q", "quiet           ", "Print less information while running"));
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));
    puts(HELP_TEXT("V", "verbose         ", "Print more information while running"));

    fputs("\nCommands:\n", stdout);
    for(i = 0; commands[i].name; i++)
      printf("    %-28s %s\n      %s\n", commands[i].name, commands[i].args,
             commands[i].description);
    fputs("  A prefix of `flickr.' may be optionally given\n", stdout);

    fputs("\nParameters for API calls that return lists of photos:\n", stdout);
    
    fputs("  EXTRAS is a comma-separated list of optional fields to return from:\n", stdout);
    for(i = 0; 1; i++) {
      const char* name;
      const char* label;

      if(flickcurl_get_extras_format_info(i, &name, &label))
        break;
      printf("    %-16s %s\n", name, label);
    }

    fputs("  FORMAT is result syntax format:\n", stdout);
    for(i = 0; 1; i++) {
      const char* name;
      const char* label;

      if(flickcurl_get_feed_format_info(i, &name, &label, NULL))
        break;
      printf("    %-16s %s\n", name, label);
    }
    fputs("  PAGE is result page number or '-' for default (1 = first page)\n"
          "  PER-PAGE is photos per result page or '-' for default (10)\n",
          stdout
          );

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

  return(rc);
}
