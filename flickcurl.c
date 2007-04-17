/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * flickcurl utility - Invoke the Flickrcurl library
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


typedef int (*command_handler)(flickcurl* fc, int argc, char *argv[]);


static int
command_test_echo(flickcurl* fc, int argc, char *argv[])
{
  return flickcurl_test_echo(fc, argv[1], argv[2]);
}


static int
command_people_findByEmail(flickcurl* fc, int argc, char *argv[])
{
  char* nsid=NULL;
  char* email=argv[1];
  
  nsid=flickcurl_people_findByEmail(fc, email);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for user email %s\n", 
            program, nsid, email);
  
  return (nsid == NULL);
}


static int
command_people_findByUsername(flickcurl* fc, int argc, char *argv[])
{
  char* nsid=NULL;
  char* user_name=argv[1];
  
  nsid=flickcurl_people_findByUsername(fc, user_name);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for username %s\n", 
            program, nsid, user_name);
  
  return (nsid == NULL);
}


static int
command_people_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_person* person;

  person=flickcurl_people_getInfo(fc, argv[1]);

  if(person) {
    flickcurl_person_field_type field;
    
    fprintf(stderr, "Found person with ID %s\n", person->nsid);

    for(field=0; field <= PERSON_FIELD_LAST; field++) {
      flickcurl_field_value_type datatype=person->fields[field].type;

      if(datatype == VALUE_TYPE_NONE)
        continue;
      
      fprintf(stderr, "field %s (%d) with %s value: '%s' / %d\n", 
              flickcurl_get_person_field_label(field), field,
              flickcurl_get_field_value_type_label(datatype),
              person->fields[field].string, person->fields[field].integer);
    }
    
    flickcurl_free_person(person);
  }
  
  return (person == NULL);
}


static void
command_print_tags(flickcurl_tag** tags, const char* label, const char* value)
{
  int i;
  if(label)
    fprintf(stderr, "%s: %s %s tags\n", program, label,
            (value ? value : "(none)"));
  for(i=0; tags[i]; i++) {
    flickcurl_tag* tag=tags[i];
    fprintf(stderr,
            "%d) %s tag: id %s author ID %s name %s raw '%s' cooked '%s' count %d\n",
            i, (tag->machine_tag ? "machine" : "regular"),
            tag->id, tag->author,
            (tag->authorname ? tag->authorname : "(Unknown)"),
            tag->raw, tag->cooked, tag->count);
  }
}


static int
command_photos_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_photo* photo;

  photo=flickcurl_photos_getInfo(fc, argv[1]);

  if(photo) {
    flickcurl_photo_field_type field;
    
    fprintf(stderr, "%s: Found photo with URI %s ID %s and %d tags\n",
            program, photo->uri, photo->id, photo->tags_count);

    for(field=0; field <= PHOTO_FIELD_LAST; field++) {
      flickcurl_field_value_type datatype=photo->fields[field].type;

      if(datatype == VALUE_TYPE_NONE)
        continue;
      
      fprintf(stderr, "field %s (%d) with %s value: '%s' / %d\n", 
              flickcurl_get_photo_field_label(field), field,
              flickcurl_get_field_value_type_label(datatype),
              photo->fields[field].string, photo->fields[field].integer);
    }
    
    command_print_tags(photo->tags, NULL, NULL);

    flickcurl_free_photo(photo);
  }
  
  return (photo == NULL);
}


static int
command_photos_licenses_getInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_license** licenses;
  int i;
  
  licenses=flickcurl_photos_licenses_getInfo(fc);
  if(licenses) {

    fprintf(stderr, "%s: Found licenses\n", program);

    for(i=0; licenses[i]; i++) {
      flickcurl_license* license=licenses[i];
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
  char* nsid=NULL;
  char* url=argv[1];
  
  nsid=flickcurl_urls_lookupUser(fc, url);

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
  
  for(i=0; (context=contexts[i]); i++) {
    const char* label=flickcurl_get_context_type_field_label(context->type);
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

  contexts=flickcurl_groups_pools_getContext(fc, argv[1], argv[2]);
  if(!contexts)
    return 1;
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
  
  contexts=flickcurl_photos_getAllContexts(fc, argv[1]);
  if(!contexts)
    return 1;
  fprintf(stderr, "%s: Photos %s all contexts:\n", program, argv[1]);
  command_contexts_print(stderr, contexts);
  
  flickcurl_free_contexts(contexts);

  return 0;
}

static int
command_photos_getContext(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_context** contexts;

  contexts=flickcurl_photos_getContext(fc, argv[1]);
  if(!contexts)
    return 1;
  fprintf(stderr, "%s: Photos %s context:\n", program, argv[1]);
  command_contexts_print(stderr, contexts);
  
  flickcurl_free_contexts(contexts);
  return 0;
}

static int
command_photosets_getContext(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_context** contexts;

  contexts=flickcurl_photosets_getContext(fc, argv[1], argv[2]);
  if(!contexts)
    return 1;
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

  frob=flickcurl_auth_getFrob(fc);
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

  perms=flickcurl_auth_checkToken(fc, argv[1]);
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

  perms=flickcurl_auth_getToken(fc, argv[1]);
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

  perms=flickcurl_auth_getFullToken(fc, argv[1]);
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
  char *photo_id=argv[1];

  tags=flickcurl_tags_getListPhoto(fc, photo_id);
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
  char *user_id=argv[1];

  tags=flickcurl_tags_getListUser(fc, user_id);
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
  char *user_id=NULL;
  int pop_count= -1;
  
  if(argv[1]) {
    user_id=argv[1];
    if(argv[2])
      pop_count=atoi(argv[2]);
  }
  
  tags=flickcurl_tags_getListUserPopular(fc, user_id, pop_count);
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
  char *tag=argv[1];

  tags=flickcurl_tags_getListUserRaw(fc, tag);
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
  char *tag=argv[1];

  tags=flickcurl_tags_getRelated(fc, tag);
  if(!tags)
    return 1;

  command_print_tags(tags, "Related to Tag", tag);
  free(tags);
  return 0;
}


static int
command_urls_getGroup(flickcurl* fc, int argc, char *argv[])
{
  char* nsid=NULL;
  char* url=argv[1];
  
  nsid=flickcurl_urls_getGroup(fc, url);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for group profile/photo URL %s\n", 
            program, nsid, url);
  
  return (nsid == NULL);
}


static int
command_urls_getUserPhotos(flickcurl* fc, int argc, char *argv[])
{
  char* url=NULL;
  char* user=argv[1];
  
  url=flickcurl_urls_getUserPhotos(fc, user);

  if(url)
    fprintf(stderr, "%s: photo URL %s for user %s\n", 
            program, url, user);
  
  return (url == NULL);
}


static int
command_urls_getUserProfile(flickcurl* fc, int argc, char *argv[])
{
  char* url=NULL;
  char* user=argv[1];
  
  url=flickcurl_urls_getUserProfile(fc, user);

  if(url)
    fprintf(stderr, "%s: photo URL %s for user %s\n", 
            program, url, user);
  
  return (url == NULL);
}


static int
command_urls_lookupGroup(flickcurl* fc, int argc, char *argv[])
{
  char* nsid=NULL;
  char* url=argv[1];
  
  nsid=flickcurl_urls_lookupGroup(fc, url);

  if(nsid)
    fprintf(stderr, "%s: NSID %s for group profile/photo URL %s\n", 
            program, nsid, url);
  
  return (url == NULL);
}


static int
command_tags_getHotList(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_tag** tags;
  char *period=NULL;
  int count= -1;

  if(argv[1]) {
    period=argv[1];
    if(argv[2])
      count=atoi(argv[2]);
  }

  tags=flickcurl_tags_getHotList(fc, period, count);
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
  const char *photo_id=argv[1];
  const char *tags=argv[2];

  return flickcurl_photos_addTags(fc, photo_id, tags);
}


static int
command_photos_delete(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id=argv[1];

  return flickcurl_photos_delete(fc, photo_id);
}


static int
command_photos_removeTag(flickcurl* fc, int argc, char *argv[])
{
  const char *tag_id=argv[1];

  return flickcurl_photos_removeTag(fc, tag_id);
}


static int
command_photos_setTags(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id=argv[1];
  const char *tags=argv[2];

  return flickcurl_photos_setTags(fc, photo_id, tags);
}


static int
command_reflection_getMethodInfo(flickcurl* fc, int argc, char *argv[])
{
  flickcurl_method* method;

  method=flickcurl_reflection_getMethodInfo(fc, argv[1]);

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
    
      for(i=0; method->args[i]; i++) {
        flickcurl_arg* arg=method->args[i];
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
  
  methods=flickcurl_reflection_getMethods(fc);
  if(methods) {
    int i;
    fprintf(stderr, "%s: Found methods:\n", program);
    for(i=0; methods[i]; i++)
      printf("%d) %s\n", i, methods[i]);

    for(i=0; methods[i]; i++)
      free(methods[i]);
    free(methods);
  }
  
  return (methods == NULL);
}

static int
command_photos_comments_addComment(flickcurl* fc, int argc, char *argv[])
{
  const char *photo_id=argv[1];
  const char *comment_text=argv[2];
  char* id;
  
  id=flickcurl_photos_comments_addComment(fc, photo_id, comment_text);
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
  const char *comment_id=argv[1];

  return flickcurl_photos_comments_deleteComment(fc, comment_id);
}


static int
command_photos_comments_editComment(flickcurl* fc, int argc, char *argv[])
{
  const char *comment_id=argv[1];
  const char *comment_text=argv[2];

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
  for(i=0; comments[i]; i++) {
    flickcurl_comment* comment_object=comments[i];
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
  const char *photo_id=argv[1];
  flickcurl_comment** comments;
  
  comments=flickcurl_photos_comments_getList(fc, photo_id);
  if(!comments)
    return 1;
  
  command_print_comments(comments, "Photo ID", photo_id);

  flickcurl_free_comments(comments);
  return 0;
}


static int
command_photosets_comments_addComment(flickcurl* fc, int argc, char *argv[])
{
  const char *photoset_id=argv[1];
  const char *comment_text=argv[2];
  char* id;
  
  id=flickcurl_photosets_comments_addComment(fc, photoset_id, comment_text);
  if(id) {
    fprintf(stderr,
            "%s: Added comment '%s' to photoset %s giving comment ID %s\n", 
            program, photoset_id, comment_text, id);
  }
  
  return (id == NULL);
}

static int
command_photosets_comments_deleteComment(flickcurl* fc, int argc, char *argv[])
{
  const char *comment_id=argv[1];

  return flickcurl_photosets_comments_deleteComment(fc, comment_id);
}


static int
command_photosets_comments_editComment(flickcurl* fc, int argc, char *argv[])
{
  const char *comment_id=argv[1];
  const char *comment_text=argv[2];

  return flickcurl_photosets_comments_editComment(fc, comment_id, comment_text);
}


static int
command_photosets_comments_getList(flickcurl* fc, int argc, char *argv[])
{
  const char *photoset_id=argv[1];
  flickcurl_comment** comments;
  
  comments=flickcurl_photosets_comments_getList(fc, photoset_id);
  if(!comments)
    return 1;
  
  command_print_comments(comments, "Photoset ID", photoset_id);

  flickcurl_free_comments(comments);
  return 0;
}


static void
print_upload_status(flickcurl_upload_status* status, const char* label)
{
  if(label)
    fprintf(stderr, "%s: %s status\n", program, label);
  if(status->photoid)
    fprintf(stderr, "  Photo ID: %s\n", status->photoid);
  if(status->secret)
    fprintf(stderr, "  Secret: %s\n", status->secret);
  if(status->originalsecret)
    fprintf(stderr, "  Original Secret: %s\n", status->originalsecret);
  if(status->ticketid)
    fprintf(stderr, "  Ticket ID: %s\n", status->ticketid);
}


static int
command_upload(flickcurl* fc, int argc, char *argv[])
{
  const char *file=argv[1];
  const char *title=NULL;
  const char *description=NULL;
  char *tags_string=NULL;
  flickcurl_upload_status* status=NULL;
  int is_public=0;
  int is_friend=0;
  int is_family=0;
  int usage=0;

  argv++; argc--;
  file=argv[0];

  if(access((const char*)file, R_OK)) {
    fprintf(stderr, "%s: Failed to read image filename '%s': %s\n",
            program, file, strerror(errno));
    status=NULL;
    goto tidy;
  }
  
  while(!usage && argc) {
    argv++; argc--;
    if(!strcmp(argv[0], "description")) {
      argv++; argc--;
      description=argv[0];
    } else if(!strcmp(argv[0], "title")) {
      argv++; argc--;
      title=argv[0];
    } else if(!strcmp(argv[0], "friend")) {
      is_friend=1;
    } else if(!strcmp(argv[0], "family")) {
      is_family=1;
    } else if(!strcmp(argv[0], "public")) {
      is_public=1;
    } else if(!strcmp(argv[0], "tags")) {
      size_t tags_len=0;
      int i;
      char *p;

      /* tags absorb all remaining parameters */
      for(i=1; i<argc; i++)
        tags_len+=strlen(argv[i])+1;
      tags_string=(char*)malloc(tags_len);
      
      p=tags_string;
      for(i=1; i<argc; i++) {
        size_t tag_len=strlen(argv[i]);
        strncpy(p, argv[i], tag_len); p+= tag_len;
        *p++=' ';
      }
      *(--p)='\0';
      
      fprintf(stderr, "%s: Setting tags: '%s'\n", program, tags_string);
      break;
    } else {
      fprintf(stderr, "%s: Unknown parameter: '%s'\n", program, argv[0]);
      usage=1;
    }
  }

  if(usage) {
    status=NULL;
    goto tidy;
  }
  
  fprintf(stderr, "%s: Uploading file %s\n", program, file);
  
  status=flickcurl_photos_upload(fc, file, title, description, tags_string,
                                 is_public, is_friend, is_family);
  if(status) {
    print_upload_status(status, "Photo upload");

    flickcurl_upload_status_free(status);
  }

  tidy:
  if(tags_string)
    free(tags_string);
  
  return (status == NULL);
}


static int
command_replace(flickcurl* fc, int argc, char *argv[])
{
  const char *file=argv[1];
  const char *photo_id=argv[2];
  int async=0;
  flickcurl_upload_status* status=NULL;
  
  if(access((const char*)file, R_OK)) {
    fprintf(stderr, "%s: Failed to read image filename '%s': %s\n",
            program, file, strerror(errno));
    status=NULL;
    goto tidy;
  }
  
  if(argc > 3 && !strcmp(argv[3], "async"))
    async=1;

  status=flickcurl_photos_replace(fc, file, photo_id, async);
  if(status) {
    print_upload_status(status, "Photo replace");

    flickcurl_upload_status_free(status);
  }
  
  tidy:
  
  return (status == NULL);
}



static struct {
  const char*     name;
  const char*     args;
  const char*     description;
  command_handler handler;
  int             min;
  int             max;
} commands[] = {
  /* name, min, handler */
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
  {"groups.pools.getContext",
   "PHOTO-ID GROUP-ID", "Get next and previous photos for PHOTO-ID in GROUP-ID pool.",
   command_groups_pools_getContext, 2, 2},
  {"people.findByEmail",
   "EMAIL", "get a user's NSID from their EMAIL address", 
   command_people_findByEmail,  1, 1},
  {"people.findByUsername",
   "USERNAME", "get a user's NSID from their USERNAME", 
   command_people_findByUsername,  1, 1},
  {"people.getInfo",
   "USER-ID", "Get information about one person with id USER-ID", 
   command_people_getInfo,  1, 1},
  {"photos.addTags",
   "PHOTO-ID TAGS", "Add TAGS to a PHOTO-ID.",
   command_photos_addTags, 2, 2},
  {"photos.delete",
   "PHOTO-ID", "Delete a PHOTO-ID.",
   command_photos_delete, 1, 1},
  {"photos.getAllContexts",
   "PHOTO-ID", "Get all visible sets and pools the PHOTO-ID belongs to.",
   command_photos_getAllContexts, 1, 1},
  {"photos.getContext",
   "PHOTO-ID", "Get next and previous photos for a PHOTO-ID in a photostream.",
   command_photos_getContext, 1, 1},
  {"photos.getInfo",
   "PHOTO-ID", "Get information about one photo with id PHOTO-ID", 
   command_photos_getInfo,  1, 1},
  {"photos.licenses.getInfo",
   "", "Get list of available photo licenses", 
   command_photos_licenses_getInfo,  0, 0},
  {"photos.removeTag",
   "TAG-ID", "Remove a tag TAG-ID from a photo.",
   command_photos_removeTag, 1, 1},
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
  {"photosets.getContext",
   "PHOTO-ID PHOTOSET-ID", "Get next and previous photos for PHOTO-ID in PHOTOSET-ID.",
   command_photosets_getContext, 2, 2},
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
  {"reflection.getMethods",
   "", "Get API methods",
   command_reflection_getMethods, 0, 0},
  {"reflection.getMethodInfo",
   "NAME", "Get information about an API method NAME",
   command_reflection_getMethodInfo, 1, 1},
  {"tags.getHotList",
   "[PERIOD [COUNT]]", "Get the list of hot tags for the given PERIOD (day, week)",
   command_tags_getHotList, 0, 2},
  {"tags.getListPhoto",
   "PHOTO-ID", "Get the tag list for a PHOTO-ID.",
   command_tags_getListPhoto, 1, 1},
  {"tags.getListUser",
   "[USER-ID]", "Get the tag list for a USER-ID (or current user).",
   command_tags_getListUser, 0, 1},
  {"tags.getListUserPopular",
   "[USER-ID [COUNT]]", "Get the popular tag list for a USER-ID (or current user).",
   command_tags_getListUserPopular, 0, 2},
  {"tags.getListUserRaw",
   "[TAG]", "Get the raw versions of a TAG (or all tags) for the current user.",
   command_tags_getListUserRaw, 0, 1},
  {"tags.getRelated",
   "TAG", "Get a list of tags 'related' to TAG based on clustered usage analysis.",
   command_tags_getRelated, 1, 1},
  {"test.echo",
   "KEY VALUE", "Test echo of KEY VALUE",
   command_test_echo,  2, 2},
  {"urls.getGroup",
   "GROUP-ID", "Get the url of the group page for GROUP-ID.", 
   command_urls_getGroup,  1, 1},
  {"urls.getUserPhotos",
   "USER-ID", "Get the url of the photo page for USER-ID.", 
   command_urls_getUserPhotos,  1, 1},
  {"urls.getUserProfile",
   "USER-ID", "Get the url of the profile page for USER-ID.", 
   command_urls_getUserProfile,  1, 1},
  {"urls.lookupGroup",
   "URL", "Get a group NSID from the URL to a group's page or photo pool.", 
   command_urls_lookupGroup,  1, 1},
  {"urls.lookupUser",
   "URL", "Get a user NSID from the URL to a user's photo", 
   command_urls_lookupUser,  1, 1},

  {"upload",
   "FILE [PARAMS...]", "Upload a photo FILE with optional parameters PARAM or PARAM VALUE\n      title TITLE | description DESC | tags TAGS... | friend | public | family", 
   command_upload,  1, 0},

  {"replace",
   "FILE PHOTO-ID [async]", "Replace a photo PHOTO-ID with a new FILE (async)", 
   command_replace,  2, 3},

  {NULL, NULL, NULL, 0, 0}
};  
  


static const char *title_format_string="Flickr API utility %s\n";

static const char* config_filename=".flickcurl.conf";
static const char* config_section="flickr";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc=NULL;
  int rc=0;
  int usage=0;
  int help=0;
  int cmd_index= -1;
  int read_auth=1;
  int i;
  const char* home;
  char config_path[1024];
  int request_delay= -1;
  char *command=NULL;
  
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

  argv+=optind;
  argc-=optind;
  
  if(!help && argc < 1)
    usage=2; /* Title and usage */

  if(!help && !argc) {
    fprintf(stderr, "%s: No command given\n", program);
    usage=1;
    goto usage;
  }

  if(usage || help)
    goto usage;


  if(request_delay >= 0)
    flickcurl_set_request_delay(fc, request_delay);

  command=argv[0];
  
  /* allow old format commands to work */
  for(i=0; command[i]; i++) {
    if(command[i] == '-')
      command[i]='.';
  }
  
  if(!strncmp(command, "flickr.", 7))
    command+=7;
  
  for(i=0; commands[i].name; i++)
    if(!strcmp(command, commands[i].name)) {
      cmd_index=i;
      break;
    }
  if(cmd_index < 0) {
    fprintf(stderr, "%s: No such command `%s'\n", program, command);
    usage=1;
    goto usage;
  }

  if((argc-1) < commands[cmd_index].min) {
    fprintf(stderr, "%s: Need min %d arguments for command `%s'\n", program,
            commands[cmd_index].min, command);
    usage=1;
    goto usage;
  }
  
  if(commands[cmd_index].max > 0 && 
     (argc-1) > commands[cmd_index].max) {
    fprintf(stderr, "%s: Need max %d arguments for command `%s'\n", program,
            commands[cmd_index].max, command);
    usage=1;
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
    rc=1;
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
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

    fputs("\nCommands:\n", stdout);
    for(i=0; commands[i].name; i++)
      printf("    %-28s %s\n      %s\n", commands[i].name, commands[i].args,
             commands[i].description);
    fputs("  A prefix of `flickr.' may be optionally given\n", stdout);

    rc=0;
    goto tidy;
  }


  /* Perform the API call */
  rc=commands[cmd_index].handler(fc, argc, argv);
  if(rc)
    fprintf(stderr, "%s: Command %s failed\n", program, argv[0]);
  
 tidy:
  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
