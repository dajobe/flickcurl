/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_fake.c - Fake Raptor V2 - just enough for flickrdf.c
 *
 * Copyright (C) 2011-2012, David Beckett http://www.dajobe.org/
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


#include <raptor_fake.h>

raptor_world* raptor_new_world(void) { return NULL; }
int raptor_world_open(raptor_world* world) { return 0; }
void raptor_free_world(raptor_world* world) {  }

raptor_uri*
raptor_new_uri(raptor_world* world, const unsigned char *uri_string)
{
  size_t len;
  raptor_uri* u;

  if(!uri_string)
    return NULL;
  
  len = strlen((const char*)uri_string);
  u = (raptor_uri*)malloc(len + 1);
  memcpy(u, uri_string, len + 1);

  return u;
}


raptor_uri*
raptor_new_uri_from_uri_local_name(raptor_world* world,
                                   raptor_uri *uri,
                                   const unsigned char *local_name)
{
  size_t len1;
  size_t len2;
  raptor_uri* newu;

  if(!uri || !local_name)
    return NULL;
  
  len1 = strlen((const char*)uri);
  len2 = strlen((const char*)local_name);

  newu = (raptor_uri*)malloc(len1 + len2 + 1);
  memcpy(newu, uri, len1);
  memcpy(newu + len1, local_name, len2 + 1);

  return newu;
}


raptor_uri*
raptor_new_uri_from_uri(raptor_uri *uri)
{
  return raptor_new_uri(NULL, (const unsigned char *)uri);
}

void
raptor_free_uri(raptor_uri* u)
{
  free(u);
}


raptor_term*
raptor_new_term_from_blank(raptor_world* world, const unsigned char* blank)
{
  raptor_term* newt;
  size_t blank_len;
  
  if(!blank)
    return NULL;
  
  blank_len = strlen((const char*)blank);

  newt = (raptor_term*)calloc(sizeof(*newt), 1);
  newt->type = RAPTOR_TERM_TYPE_BLANK;
  newt->value.blank.string = malloc(blank_len + 1);

  memcpy(newt->value.blank.string, blank, blank_len + 1);

  return newt;
}

raptor_term*
raptor_new_term_from_uri_string(raptor_world* world,
                                const unsigned char *uri_string)
{
  raptor_term* newt;

  if(!uri_string)
    return NULL;
  
  newt = (raptor_term*)calloc(sizeof(*newt), 1);
  newt->type = RAPTOR_TERM_TYPE_URI;
  newt->value.uri = raptor_new_uri(world, uri_string);

  return newt;
}


raptor_term*
raptor_new_term_from_uri(raptor_world* world, raptor_uri* uri)
{
  return raptor_new_term_from_uri_string(world, (const unsigned char *)uri);
}


raptor_term*
raptor_new_term_from_literal(raptor_world* world, const unsigned char* literal, raptor_uri* datatype, const unsigned char* language)
{
  raptor_term* newt;
  size_t literal_len;

  if(!literal)
    return NULL;
  
  literal_len = strlen((const char*)literal);

  newt = (raptor_term*)calloc(sizeof(*newt), 1);
  newt->type = RAPTOR_TERM_TYPE_LITERAL;

  newt->value.literal.string = (unsigned char*)malloc(literal_len + 1);
  memcpy(newt->value.literal.string, literal, literal_len + 1);

  if(datatype)
    newt->value.literal.datatype = raptor_new_uri_from_uri(datatype);

  return newt;
}

void
raptor_free_term(raptor_term* term) 
{
  if(term->type == RAPTOR_TERM_TYPE_URI)
    raptor_free_uri(term->value.uri);
  
  if(term->type == RAPTOR_TERM_TYPE_BLANK)
    free(term->value.blank.string);
  if(term->type == RAPTOR_TERM_TYPE_LITERAL) {
    if(term->value.literal.datatype)
      raptor_free_uri(term->value.literal.datatype);
    free(term->value.literal.string);
  }
}


#define NSERIALIZERS 2
raptor_syntax_description serializers[NSERIALIZERS]= {
  { { "ntriples", NULL}, "N-Triples" },
  { { "turtle", NULL},   "Turtle",   },
};


int
raptor_world_is_serializer_name(raptor_world* world, const char* name)
{
  int i;
  
  for(i = 0; i < NSERIALIZERS; i++) {
    if(strcmp(serializers[i].names[0], name))
       return 1;
  }
  return 0;
}


const raptor_syntax_description*
raptor_world_get_serializer_description(raptor_world* world,
                                        unsigned int counter)
{
  if(counter > (NSERIALIZERS-1))
    return NULL;

  return &serializers[counter];
}


raptor_serializer*
raptor_new_serializer(raptor_world* world, const char* serializer_name)
{
  raptor_serializer* s;
  s = (raptor_serializer*)calloc(sizeof(raptor_serializer), 1);
  s->output_turtle = !strcmp((const char*)serializer_name, "turtle");
  return s;
}


void
raptor_free_serializer(raptor_serializer* s)
{
  free(s);
}


void
raptor_serializer_set_namespace(raptor_serializer* serializer,
                                raptor_uri* uri, const unsigned char* prefix)
{
  FILE* fh = serializer->fh;
  if(serializer->output_turtle)
    fprintf(fh, "@prefix %s: <%s> .\n", (const char*)prefix, (const char*)uri);
}


void
raptor_serializer_start_to_file_handle(raptor_serializer* serializer,
                                      raptor_uri* base_uri, FILE* fh)
{
  serializer->fh = fh;
  if(base_uri && serializer->output_turtle)
    fprintf(fh, "@base <%s>\n", (char*)base_uri);
}


void
raptor_serializer_serialize_statement(raptor_serializer* serializer,
                                      raptor_statement* s)
{
  FILE *fh = serializer->fh;

  if(s->subject->type == RAPTOR_TERM_TYPE_URI)
    fprintf(fh, "<%s>", (const char*)s->subject->value.uri);
  else /* blank node */
    fprintf(fh, "_:%s", (const char*)s->subject->value.blank.string);

  fprintf(fh, " <%s> ", (const char*)s->predicate->value.uri);

  if(s->object->type == RAPTOR_TERM_TYPE_LITERAL) {
    const char *p;
    char c;
    
    fputc('"', fh);
    for(p = (const char*)s->object->value.literal.string; (c = *p); p++) {
      if(c == '\n') {
        fputs("\\\n", fh);
        continue;
      } else if(c == '\t') {
        fputs("\\\t", fh);
        continue;
      } if(c == '\r') {
        fputs("\\\r", fh);
        continue;
      } else if(c == '"' || c == '\\')
        fputc('\\', fh);
      fputc(c, fh);
    }
    fputc('"', fh);
    if(s->object->value.literal.datatype)  {
      fputs("^^<", fh);
      fputs((const char*)s->object->value.literal.datatype, fh);
      fputc('>', fh);
    }
  } else if(s->object->type == RAPTOR_TERM_TYPE_URI)
    fprintf(fh, "<%s>", (const char*)s->object->value.uri);
  else /* blank node */
    fprintf(fh, "_:%s", (const char*)s->object->value.blank.string);
  
  fputs(" . \n", fh);
}


void
raptor_serializer_serialize_end(raptor_serializer* serializer)
{
  fflush(serializer->fh);
}

void
raptor_statement_init(raptor_statement *statement, raptor_world *world)
{
  memset(statement, '\0', sizeof(*statement));
  return;
}

void
raptor_statement_clear(raptor_statement *statement)
{
  raptor_free_term(statement->subject);
  raptor_free_term(statement->predicate);
  raptor_free_term(statement->object);
}
