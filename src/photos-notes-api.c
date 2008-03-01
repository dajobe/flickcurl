/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-notes-api.c - Flickr flickr.photos.notes.* API calls
 *
 * Copyright (C) 2007-2008, David Beckett http://www.dajobe.org/
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

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_photos_notes_add:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to add a note to
 * @note_x: The left coordinate of the note
 * @note_y: The top coordinate of the note
 * @note_w: The width of the note
 * @note_h: The height of the note
 * @note_text: The description of the note
 * 
 * Add a note to a photo.
 *
 * Coordinates and sizes are in pixels, based on the 500px image size
 * shown on individual photo pages.
 *
 * Implements flickr.photos.notes.add (0.12)
 * 
 * Return value: note ID or NULL on failure
 **/
char*
flickcurl_photos_notes_add(flickcurl* fc, const char* photo_id,
                           int note_x, int note_y, int note_w, int note_h,
                           const char* note_text)
{
  const char* parameters[13][2];
  int count=0;
  xmlDocPtr doc=NULL;
  xmlXPathContextPtr xpathCtx=NULL; 
  char *id=NULL;
  char note_x_s[10];
  char note_y_s[10];
  char note_w_s[10];
  char note_h_s[10];
  
  if(!photo_id || !note_text)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "note_x";
  sprintf(note_x_s, "%d", note_x);
  parameters[count++][1]= note_x_s;
  parameters[count][0]  = "note_y";
  sprintf(note_y_s, "%d", note_y);
  parameters[count++][1]= note_y_s;
  parameters[count][0]  = "note_w";
  sprintf(note_w_s, "%d", note_w);
  parameters[count++][1]= note_w_s;
  parameters[count][0]  = "note_h";
  sprintf(note_h_s, "%d", note_h);
  parameters[count++][1]= note_h_s;
  parameters[count][0]  = "note_text";
  parameters[count++][1]= note_text;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.notes.add", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed=1;
    goto tidy;
  }

  id=flickcurl_xpath_eval(fc, xpathCtx, (const xmlChar*)"/rsp/note/@id");

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    id=NULL;

  return id;
}


/**
 * flickcurl_photos_notes_delete:
 * @fc: flickcurl context
 * @note_id: The id of the note to delete
 * 
 * Delete a note from a photo.
 *
 * Implements flickr.photos.notes.delete (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_notes_delete(flickcurl* fc, const char* note_id)
{
  const char* parameters[8][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  
  if(!note_id)
    return 1;

  parameters[count][0]  = "note_id";
  parameters[count++][1]= note_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.notes.delete", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;

  return result;
}


/**
 * flickcurl_photos_notes_edit:
 * @fc: flickcurl context
 * @note_id: The id of the note to edit
 * @note_x: The left coordinate of the note
 * @note_y: The top coordinate of the note
 * @note_w: The width of the note
 * @note_h: The height of the note
 * @note_text: The description of the note
 * 
 * Edit a note on a photo. Coordinates and sizes are in pixels, based on the 500px image size shown on individual photo pages.

 *
 * Implements flickr.photos.notes.edit (0.12)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_notes_edit(flickcurl* fc,
                            const char* note_id,
                            int note_x, int note_y, int note_w, int note_h,
                            const char* note_text)
{
  const char* parameters[13][2];
  int count=0;
  xmlDocPtr doc=NULL;
  int result=1;
  char note_x_s[10];
  char note_y_s[10];
  char note_w_s[10];
  char note_h_s[10];
  
  if(!note_id || !note_text)
    return 1;

  parameters[count][0]  = "note_id";
  parameters[count++][1]= note_id;
  parameters[count][0]  = "note_x";
  sprintf(note_x_s, "%d", note_x);
  parameters[count++][1]= note_x_s;
  parameters[count][0]  = "note_y";
  sprintf(note_y_s, "%d", note_y);
  parameters[count++][1]= note_y_s;
  parameters[count][0]  = "note_w";
  sprintf(note_w_s, "%d", note_w);
  parameters[count++][1]= note_w_s;
  parameters[count][0]  = "note_h";
  sprintf(note_h_s, "%d", note_h);
  parameters[count++][1]= note_h_s;
  parameters[count][0]  = "note_text";
  parameters[count++][1]= note_text;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.notes.edit", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc=flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result=0;

  tidy:
  if(fc->failed)
    result=1;

  return result;
}


