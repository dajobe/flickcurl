/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-transform-api.c - Flickr flickr.photos.transform.* API calls
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
 * flickcurl_photos_transform_rotate:
 * @fc: flickcurl context
 * @photo_id: The id of the photo to rotate.
 * @degrees: The amount of degrees by which to rotate the photo (clockwise) from it's current orientation. Valid values are 90, 180 and 270.
 * 
 * Rotate a photo.
 *
 * Implements flickr.photos.transform.rotate (0.13)
 * 
 * Return value: non-0 on failure
 **/
int
flickcurl_photos_transform_rotate(flickcurl* fc, const char* photo_id,
                                  int degrees)
{
  const char* parameters[9][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  char degrees_str[4];
  int result = 0;
  
  if(!photo_id || !(degrees == 90 || degrees == 180 || degrees == 270))
    return 1;

  parameters[count][0]  = "photo_id";
  parameters[count++][1]= photo_id;
  sprintf(degrees_str, "%d", degrees);
  parameters[count][0]  = "degrees";
  parameters[count++][1]= degrees_str;

  parameters[count][0]  = NULL;

  if(flickcurl_prepare(fc, "flickr.photos.transform.rotate", parameters, count))
    goto tidy;

  doc = flickcurl_invoke(fc);
  if(!doc)
    goto tidy;

  result = 0;

  tidy:
  if(fc->failed)
    result = 1;

  return result;
}


