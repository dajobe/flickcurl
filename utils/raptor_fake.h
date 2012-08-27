/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_fake.h - Fake Raptor V2 API - just enough for flickrdf.c
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

/* Fake Raptor does not use world */
typedef void raptor_world;

typedef enum {
  RAPTOR_TERM_TYPE_UNKNOWN,
  RAPTOR_TERM_TYPE_URI,
  RAPTOR_TERM_TYPE_LITERAL,
  RAPTOR_TERM_TYPE_BLANK
} raptor_term_type;

/* Fake Raptor uses char* for uris */
typedef char raptor_uri;

typedef struct {
  unsigned char *string;
  unsigned int string_len;
  raptor_uri *datatype;
  unsigned char *language;
  unsigned char language_len;
} raptor_term_literal_value;

typedef struct {
  unsigned char *string;
  unsigned int string_len;
} raptor_term_blank_value;

typedef union {
  raptor_uri *uri;
  raptor_term_literal_value literal;
  raptor_term_blank_value blank;
} raptor_term_value;

typedef struct {
  raptor_world* world;
  raptor_term_type type;
  raptor_term_value value;
} raptor_term;

typedef struct {
  raptor_world* world;
  /* int usage; */
  raptor_term* subject;
  raptor_term* predicate;
  raptor_term* object;
  /* raptor_term* graph; */
} raptor_statement;

typedef struct {
  FILE* fh;
  int output_turtle;
} raptor_serializer;


typedef struct {
  const char* const names[2];
  const char* label;
  /* omitting fields unused in this program */
} raptor_syntax_description;


raptor_world* raptor_new_world(void);
int raptor_world_open(raptor_world* world);
void raptor_free_world(raptor_world* world);
int raptor_world_is_serializer_name(raptor_world* world, const char* name);
const raptor_syntax_description* raptor_world_get_serializer_description(raptor_world* world, unsigned int counter);

raptor_uri* raptor_new_uri(raptor_world* world, const unsigned char *uri_string);
raptor_uri* raptor_new_uri_from_uri_local_name(raptor_world* world, raptor_uri *uri, const unsigned char *local_name);
raptor_uri* raptor_new_uri_from_uri(raptor_uri *uri);
void raptor_free_uri(raptor_uri* u);

raptor_term* raptor_new_term_from_blank(raptor_world* world, const unsigned char* blank);
raptor_term* raptor_new_term_from_uri_string(raptor_world* world, const unsigned char *uri_string);
raptor_term* raptor_new_term_from_uri(raptor_world* world, raptor_uri* uri);
raptor_term*
raptor_new_term_from_literal(raptor_world* world, const unsigned char* literal, raptor_uri* datatype, const unsigned char* language);
void raptor_free_term(raptor_term* term);

raptor_serializer* raptor_new_serializer(raptor_world* world, const char* serializer_name);
void raptor_free_serializer(raptor_serializer* s);
void raptor_serializer_set_namespace(raptor_serializer* serializer, raptor_uri* uri, const unsigned char* prefix);
void raptor_serializer_start_to_file_handle(raptor_serializer* serializer, raptor_uri* base_uri, FILE* fh);
void raptor_serializer_serialize_statement(raptor_serializer* serializer, raptor_statement* s);
void raptor_serializer_serialize_end(raptor_serializer* serializer);


void raptor_statement_init(raptor_statement *statement, raptor_world *world);
void raptor_statement_clear(raptor_statement *statement);
