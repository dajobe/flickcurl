/*
 * upload-photo-from-data.c - Flickcurl example code to upload a photo
 *                           from a pointer to a data buffer
 *
 * This is example code and not complete because the OAuth client
 * key, secret and the token and token secret are not configured.
 *
 * The flickcurl utility in utils/flickcurl.c contains code that
 * fully uses the API and read/writes configuration from a file
 * using flickcurl_config_var_handler()
 *
 * This file is in the Public Domain
 *
 * Compile it like this:
 *   gcc -o upload-photo-from-data upload-photo-from-data.c `flickcurl-config --cflags` `flickcurl-config --libs`
 * or use
 *   make upload-photo-from-data
 *
 * USAGE: upload-photo-from-data PHOTO-FILE
 *
 * The photo is read into an in-memory buffer first and then uploaded
 * with flickcurl_photos_upload_data() so that only a pointer to the
 * buffer (and its length) is used for the upload itself - the file
 * name is never passed to the Flickcurl API (issue #42).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <flickcurl.h>
#include <flickcurl_upload_data.h>

int main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  flickcurl *fc;
  flickcurl_upload* upload = NULL;
  FILE* fh = NULL;
  char* data = NULL;
  const char* filename = NULL;
  long file_size = 0;
  int rc = 0;

  flickcurl_init(); /* optional static initialising of resources */
  fc = flickcurl_new();

  /* Set configuration explicitly: ... */
  flickcurl_set_oauth_client_key(fc, "...");
  flickcurl_set_oauth_client_secret(fc, "...");
  flickcurl_set_oauth_token(fc, "...");
  flickcurl_set_oauth_token_secret(fc, "...");

  /* or could read from an INI config file like this: */
  /*
  flickcurl_config_read_ini(fc, "/home/user/.flickcurl.conf", "flickr",
                            fc, flickcurl_config_var_handler);
  */

  if(argc != 2) {
    fprintf(stderr, "Usage: upload-photo-from-data PHOTO-FILE\n");
    rc = 1;
    goto tidy;
  }

  filename = argv[1];

  fh = fopen(filename, "rb");
  if(!fh) {
    fprintf(stderr, "fopen(%s) failed: %s\n", filename, strerror(errno));
    rc = 1;
    goto tidy;
  }

  if(fseek(fh, 0, SEEK_END) != 0 ||
     (file_size = ftell(fh)) < 0 ||
     fseek(fh, 0, SEEK_SET) != 0) {
    fprintf(stderr, "Failed to determine size of %s\n", filename);
    rc = 1;
    goto tidy;
  }

  data = (char*)malloc((size_t)file_size + 1);
  if(!data) {
    fprintf(stderr, "Failed to allocate %ld bytes for %s\n", file_size, filename);
    rc = 1;
    goto tidy;
  }

  if(fread(data, 1, (size_t)file_size, fh) != (size_t)file_size) {
    fprintf(stderr, "Failed to read %s: %s\n", filename, strerror(errno));
    rc = 1;
    goto tidy;
  }
  data[file_size] = '\0';
  fclose(fh);
  fh = NULL;

  /*
   * Upload the photo from the in-memory buffer: only a pointer to the
   * data and its length are passed, not a file name.
   */
  upload = flickcurl_photos_upload_data(fc, data, (size_t)file_size,
                                        NULL, "Example photo",
                                        "Uploaded from a data buffer",
                                        "example,test", NULL);
  free(data);
  data = NULL;

  if(!upload) {
    fprintf(stderr, "flickcurl_photos_upload_data(%s) failed\n", filename);
    rc = 1;
  } else {
    fprintf(stderr, "Uploaded photo with ID %s\n", upload->photoid);
    flickcurl_free_upload(upload);
    upload = NULL;
  }

tidy:
  if(fh)
    fclose(fh);

  if(data)
    free(data);

  if(upload)
    flickcurl_free_upload(upload);

  if(fc)
    flickcurl_free(fc);

  flickcurl_finish(); /* optional static free of resources */

  return rc;
}
