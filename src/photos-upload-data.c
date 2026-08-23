/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photos-upload-data.c - Flickcurl flickr.photos.upload from a data buffer
 *
 * Copyright (C) 2024, David Beckett http://www.dajobe.org/
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
#include <errno.h>

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

#include <sys/types.h>

#include <flickcurl.h>
#include <flickcurl_upload_data.h>
#include <flickcurl_internal.h>


/**
 * flickcurl_photos_upload_data:
 * @fc: flickcurl context
 * @data: pointer to the photo data to upload
 * @data_length: length of the photo data, in bytes
 * @secret: (optional) the secret for the photo
 * @title: (optional) title for the photo
 * @description: (optional) description for the photo
 * @tags: (optional) space or comma separated tags for the photo
 * @async: (optional) whether the upload should be performed asynchronously
 *
 * Upload a photo from a pointer to an in-memory data buffer rather
 * than from a file name; see flickcurl_photos_upload() for the
 * file-name based variant of this call.
 *
 * Implements flickr.photos.upload (1.0)
 *
 * Return value: flickcurl_upload on success, NULL on failure
 **/
flickcurl_upload*
flickcurl_photos_upload_data(flickcurl* fc,
                             const char* data, size_t data_length,
                             const char* secret, const char* title,
                             const char* description,
                             const char* tags, const char* async)
{
  flickcurl_upload* upload = NULL;
  char tmpl[] = "flickcurl-upload.XXXXXX";
  char* filename = NULL;
  int fd = -1;
  size_t offset = 0;
  int success = 0;

  if(!fc)
    return NULL;

  if(!data || data_length == 0) {
    flickcurl_error(fc, "flickcurl_photos_upload_data: no photo data given");
    fc->failed = 1;
    return NULL;
  }

  /*
   * The existing flickr.photos.upload transport reads the photo from a
   * file name, so give the buffer a short-lived private temporary file
   * to upload from.  The file is unlinked again before returning, so
   * the caller does not need the photo on disk at all.
   */
  fd = mkstemp(tmpl);
  if(fd < 0) {
    flickcurl_error(fc, "flickcurl_photos_upload_data: failed to create temporary file");
    fc->failed = 1;
    return NULL;
  }

  filename = strdup(tmpl);
  if(!filename) {
    close(fd);
    unlink(tmpl);
    flickcurl_error(fc, "flickcurl_photos_upload_data: out of memory");
    fc->failed = 1;
    return NULL;
  }

  while(offset < data_length) {
    ssize_t written = write(fd, data + offset, data_length - offset);
    if(written < 0) {
      if(errno == EINTR)
        continue;
      break;
    }
    offset += (size_t)written;
  }
  close(fd);

  if(offset == data_length)
    success = 1;

  if(success)
    upload = flickcurl_photos_upload(fc, filename, secret, title,
                                     description, tags, async);
  else
    flickcurl_error(fc, "flickcurl_photos_upload_data: failed to write temporary file");

  unlink(filename);
  free(filename);

  if(!success)
    fc->failed = 1;

  return upload;
}
