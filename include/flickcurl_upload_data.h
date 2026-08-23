/*
 * flickcurl_upload_data.h - Flickcurl public API for photo upload
 *                           from a data buffer (issue #42)
 *
 * Add-on public API that allows a photo to be uploaded from a
 * pointer to an in-memory data buffer rather than from a file
 * name.
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

#ifndef FLICKCURL_UPLOAD_DATA_H
#define FLICKCURL_UPLOAD_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <flickcurl.h>

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
flickcurl_upload* flickcurl_photos_upload_data(flickcurl* fc,
                                               const char* data,
                                               size_t data_length,
                                               const char* secret,
                                               const char* title,
                                               const char* description,
                                               const char* tags,
                                               const char* async);

#ifdef __cplusplus
}
#endif

#endif
