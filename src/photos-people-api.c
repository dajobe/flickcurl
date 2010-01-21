/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-people-api.c - Flickr flickr.photos.people.* API calls
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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
#undef HAVE_STDLIB_H
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_photos_people_add:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to add a person to.
 * @user_id: The NSID of the user to add to the photo.
 * @person_x: The left-most pixel co-ordinate of the box around the person. (or < 0)
 * @person_y: The top-most pixel co-ordinate of the box around the person. (or < 0)
 * @person_w: The width (in pixels) of the box around the person. (or < 0)
 * @person_h: The height (in pixels) of the box around the person. (or < 0)
 * 
 * Add a person to a photo. Coordinates and sizes of boxes are optional; they are measured in pixels, based on the 500px image size shown on individual photo pages.
 *
 * Implements flickr.photos.people.add (1.17)
 * 
 * Announced 2010-01-21
 * http://code.flickr.com/blog/2010/01/21/people-in-photos-the-api-methods/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_people_add(flickcurl* fc, const char* photo_id,
                            const char* user_id,
                            int person_x, int person_y,
                            int person_w, int person_h)
{
  const char* parameters[13][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int rc = 0;
  char person_x_str[10];
  char person_y_str[10];
  char person_w_str[10];
  char person_h_str[10];
  
  if(!photo_id || !user_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  if(person_x >= 0) {
    sprintf(person_x_str, "%d", person_x);
    parameters[count][0]  = "person_x";
    parameters[count++][1]= person_x_str;
  }
  if(person_y >= 0) {
    sprintf(person_y_str, "%d", person_y);
    parameters[count][0]  = "person_y";
    parameters[count++][1]= person_y_str;
  }
  if(person_w >= 0) {
    sprintf(person_w_str, "%d", person_w);
    parameters[count][0]  = "person_w";
    parameters[count++][1]= person_w_str;
  }
  if(person_h >= 0) {
    sprintf(person_h_str, "%d", person_h);
    parameters[count][0]  = "person_h";
    parameters[count++][1]= person_h_str;
  }

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.people.add", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    rc = 1;

  return rc;
}


/**
 * flickcurl_photos_people_delete:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to remove a person from.
 * @user_id: The NSID of the person to remove from the photo.
 * 
 * Remove a person from a photo.
 *
 * Implements flickr.photos.people.delete (1.17)
 * 
 * Announced 2010-01-21
 * http://code.flickr.com/blog/2010/01/21/people-in-photos-the-api-methods/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_people_delete(flickcurl* fc, const char* photo_id,
                               const char* user_id)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  
  if(!photo_id || !user_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.people.delete", parameters, count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_photos_people_deleteCoords:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to edit a person in.
 * @user_id: The NSID of the person whose bounding box you want to remove.
 * 
 * Remove the bounding box from a person in a photo
 *
 * Implements flickr.photos.people.deleteCoords (1.17)
 * 
 * Announced 2010-01-21
 * http://code.flickr.com/blog/2010/01/21/people-in-photos-the-api-methods/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_people_deleteCoords(flickcurl* fc, const char* photo_id,
                                     const char* user_id)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  void* result = NULL;
  
  if(!photo_id || !user_id)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.people.deleteCoords", parameters,
                       count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    result = NULL;

  return (result == NULL);
}


/**
 * flickcurl_photos_people_editCoords:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to edit a person in.
 * @user_id: The NSID of the person to edit in a photo.
 * @person_x: The left-most pixel co-ordinate of the box around the person.
 * @person_y: The top-most pixel co-ordinate of the box around the person.
 * @person_w: The width (in pixels) of the box around the person.
 * @person_h: The height (in pixels) of the box around the person.
 * 
 * Edit the bounding box of an existing person on a photo.
 *
 * Implements flickr.photos.people.editCoords (1.17)
 * 
 * Announced 2010-01-21
 * http://code.flickr.com/blog/2010/01/21/people-in-photos-the-api-methods/
 *
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_people_editCoords(flickcurl* fc, const char* photo_id,
                                   const char* user_id,
                                   int person_x, int person_y,
                                   int person_w, int person_h)
{
  const char* parameters[13][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  int rc = 0;
  char person_x_str[10];
  char person_y_str[10];
  char person_w_str[10];
  char person_h_str[10];
  
  if(!photo_id || !user_id || 
     person_x < 0  || person_y < 0 || person_w <0 || person_h < 0)
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  parameters[count][0]  = "user_id";
  parameters[count++][1]= user_id;

  sprintf(person_x_str, "%d", person_x);
  parameters[count][0]  = "person_x";
  parameters[count++][1]= person_x_str;
  sprintf(person_y_str, "%d", person_y);
  parameters[count][0]  = "person_y";
  parameters[count++][1]= person_y_str;
  sprintf(person_w_str, "%d", person_w);
  parameters[count][0]  = "person_w";
  parameters[count++][1]= person_w_str;
  sprintf(person_h_str, "%d", person_h);
  parameters[count][0]  = "person_h";
  parameters[count++][1]= person_h_str;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.people.editCoords", parameters,
                       count))
    goto tidy;

  flickcurl_set_write(fc, 1);
  flickcurl_set_data(fc, (void*)"", 0);

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    rc = 1;

  return rc;
}


/**
 * flickcurl_photos_people_getList:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to get a list of people for.
 * 
 * Get a list of people in a given photo.
 *
 * Implements flickr.photos.people.getList (1.17)
 * 
 * Announced 2010-01-21
 * http://code.flickr.com/blog/2010/01/21/people-in-photos-the-api-methods/
 *
 * Return value: list of persons or NULL on failure
 **/
flickcurl_person**
flickcurl_photos_people_getList(flickcurl* fc, const char* photo_id)
{
  const char* parameters[8][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL; 
  flickcurl_person** people = NULL;
  
  if(!photo_id)
    return NULL;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.people.getList", parameters, count))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;


  xpathCtx = xmlXPathNewContext(doc);
  if(!xpathCtx) {
    flickcurl_error(fc, "Failed to create XPath context for document");
    fc->failed = 1;
    goto tidy;
  }

  people = flickcurl_build_persons(fc, xpathCtx,
                                   (const xmlChar*)"/rsp/people/person", NULL);

  tidy:
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed)
    people = NULL;

  return people;
}


