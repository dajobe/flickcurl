/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * Flickr OAuth test utility
 *
 * Copyright (C) 2011, David Beckett http://www.dajobe.org/
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
 * USAGE: oauth-test [OPTIONS]
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
#include <stdint.h>

/* many places for getopt */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <flickcurl_getopt.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>



#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif


static const char* program;

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


#define GETOPT_STRING "hv"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"help",    0, 0, 'h'},
  {"version", 0, 0, 'v'},
  {NULL,      0, 0, 0}
};
#endif


static const char *title_format_string = "Flickr OAuth test utility %s\n";

static const char* config_filename = ".flickcurl.conf";
static const char* config_section = "flickr";


int
main(int argc, char *argv[]) 
{
  flickcurl *fc = NULL;
  int rc = 0;
  int usage = 0;
  int help = 0;
  int read_auth = 1;
  const char* home;
  char config_path[1024];
  
  flickcurl_init();
  
  program = my_basename(argv[0]);

  home = getenv("HOME");
  if(home)
    sprintf(config_path, "%s/%s", home, config_filename);
  else
    strcpy(config_path, config_filename);
  

  while (!usage && !help)
  {
    int c;
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
        usage = 1;
        break;

      case 'h':
        help = 1;
        break;

      case 'v':
        fputs(flickcurl_version_string, stdout);
        fputc('\n', stdout);

        exit(0);
    }
    
  }
  
  if(help)
    goto help;
  
  if(argc != 1) {
    fprintf(stderr, "%s: Extra arguments given\n", program);
    usage = 1;
  }

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

  help:
  if(help) {
    printf(title_format_string, flickcurl_version_string);
    puts("Flickr OAuth test utility.");
    printf("Usage: %s [OPTIONS]\n\n", program);

    fputs(flickcurl_copyright_string, stdout);
    fputs("\nLicense: ", stdout);
    puts(flickcurl_license_string);
    fputs("Flickcurl home page: ", stdout);
    puts(flickcurl_home_url_string);

    fputs("\n", stdout);

    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("v", "version         ", "Print the flickcurl version"));

    rc = 0;
    goto tidy;
  }


  /* Initialise the Flickcurl library */
  fc = flickcurl_new();
  if(!fc) {
    rc = 1;
    goto tidy;
  }

  flickcurl_set_error_handler(fc, my_message_handler, NULL);

  if(read_auth && !access((const char*)config_path, R_OK)) {
    if(read_ini_config(config_path, config_section, fc,
                       my_set_config_var_handler)) {
      fprintf(stderr, "%s: Failed to read config filename %s: %s\n",
              program, config_path, strerror(errno));
      rc = 1;
      goto tidy;
    }
  }

  if(1) {
    /* KEY fields */
    const char* client_shared_secret = "a9567d986a7539fe";
    const char* token_shared_secret  = NULL;

    /* DATA fields */
    const char* http_request_method = "GET";
    const char* uri_base_string     = "http://www.flickr.com/services/oauth/request_token";
    const char* request_parameters  = "oauth_callback=http%3A%2F%2Fwww.example.com&oauth_consumer_key=653e7a6ecc1d528c516cc8f92cf98611&oauth_nonce=95613465&oauth_signature_method=HMAC-SHA1&oauth_timestamp=1305586162&oauth_version=1.0";

    /*
     * KEY
     * http://tools.ietf.org/html/rfc5849#section-3.4.2
     * key = concat(client-shared-secret, '&', token-shared-secret) 
     */

    /* 
     * DATA
     * http://tools.ietf.org/html/rfc5849#section-3.4.1
     *
     * http-request-method:
     *   uppercase of method
     *
     * uri-base-string: scheme://authority/path (NO query or fragment)
     *   http://tools.ietf.org/html/rfc5849#section-3.4.1.2
     *
     * normalized-request-parameters:
     *   uri escaped
     *   http://tools.ietf.org/html/rfc5849#section-3.4.1.3.2
     *
     * data = concat(http-request-method, '&'
     *               uri-base-string, '&',
     *               normalized-request-parameters)
     */

    /* Expected results */
    const char* expected_key = 
      "a9567d986a7539fe"
      "&";
    const char* expected_data =
      "GET" "&"
      "http%3A%2F%2Fwww.flickr.com%2Fservices%2Foauth%2Frequest_token" "&"
      "oauth_callback%3Dhttp%253A%252F%252Fwww.example.com%26oauth_consumer_key%3D653e7a6ecc1d528c516cc8f92cf98611%26oauth_nonce%3D95613465%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D1305586162%26oauth_version%3D1.0";

    const char* expected_base64_hmac_sha1 = "7w18YS2bONDPL/zgyzP5XTr5af4=";
    const char* expected_url_encoded_base64_hmac_sha1 = "7w18YS2bONDPL%2FzgyzP5XTr5af4%3D";
    unsigned char *key = NULL;
    size_t key_len;
    unsigned char* data = NULL;
    size_t data_len;
    unsigned char *p;
    char *s = NULL;
    char *escaped_s = NULL;
    size_t s_len;
    size_t escaped_s_len;
    unsigned char* result;

    key_len = 1; /* & */
    if(client_shared_secret) {
      s_len = strlen(client_shared_secret);
      key_len += s_len;
    }
    if(token_shared_secret) {
      s_len = strlen(token_shared_secret);
      key_len += s_len;
    }
    
    key = malloc(key_len + 1); /* for NUL */

    data_len = strlen(http_request_method) +
      1 +
      (strlen(uri_base_string) * 3) + /* PESSIMAL; every char %-escaped */
      1 +
      (strlen(request_parameters) * 3); /* PESSIMAL */
    data = malloc(data_len + 1); /* for NUL */

    if(!key || !data) {
      fprintf(stderr, "%s: malloc() failed\n", program);
      rc = 1;
      goto tidy_oauth;
    }

    /* Prepare key */
    p = key;
    if(client_shared_secret) {
      s_len = strlen(client_shared_secret);
      memcpy(p, client_shared_secret, s_len);
      p += s_len;
    }
    *p++ = '&';
    if(token_shared_secret) {
      s_len = strlen(token_shared_secret);
      memcpy(p, token_shared_secret, s_len);
      p += s_len;
    }
    *p = '\0'; /* Not part of HMAC-SHA1 data */
    /* calculate actual key len */
    key_len = p - key;

    /* Prepare data */
    p = data;
    s_len = strlen(http_request_method);
    memcpy(p, http_request_method, s_len);
    p += s_len;

    *p++ = '&';

    escaped_s = curl_escape(uri_base_string, strlen(uri_base_string));
    s_len = strlen(escaped_s);
    memcpy(p, escaped_s, s_len);
    p += s_len;
    curl_free(escaped_s);

    *p++ = '&';

    escaped_s = curl_escape(request_parameters, strlen(request_parameters));
    s_len = strlen(escaped_s);
    memcpy(p, escaped_s, s_len);
    p += s_len;
    curl_free(escaped_s);

    *p = '\0'; /* Not part of HMAC-SHA1 data */
    /* calculate actual data len */
    data_len = p - data;

    fprintf(stderr, "%s: key is (%d bytes)\n  %s\n", program, (int)key_len, key);
    fprintf(stderr, "%s: expected key is\n  %s\n", program, expected_key);

    fprintf(stderr, "%s: data is (%d bytes)\n  %s\n", program, (int)data_len, data);
    fprintf(stderr, "%s: expected data is\n  %s\n", program, expected_data);

    result = flickcurl_hmac_sha1(data, data_len, key, key_len);
    if(!result) {
      fprintf(stderr, "%s: flickcurl_hmac_sha1() failed\n", program);
      rc = 1;
      goto tidy_oauth;
    }

    s = flickcurl_base64_encode(result, SHA1_DIGEST_LENGTH, NULL);
    if(!s) {
      fprintf(stderr, "%s: flickcurl_base64_encode() failed\n", program);
      rc = 1;
      goto tidy_oauth;
    }

    s_len = strlen(s);
    
    fprintf(stdout, "Result (%d bytes):\n  %s\n", (int)s_len, s);
    
    fprintf(stdout, "Expected result:\n  %s\n", expected_base64_hmac_sha1);
    
    escaped_s = curl_escape(s, s_len);
    if(!escaped_s) {
      fprintf(stderr, "%s: curl_escape(%s) failed\n", program, s);
      rc = 1;
      goto tidy_oauth;
    }

    escaped_s_len = strlen(escaped_s);
    fprintf(stdout, "URI Escaped result (%d bytes):\n   %s\n",
            (int)escaped_s_len, escaped_s);
    
    fprintf(stdout, "Expected URI escaped result\n   %s\n", 
            expected_url_encoded_base64_hmac_sha1);
    
    curl_free(escaped_s);

    tidy_oauth:
    if(s)
      free(s);
    if(result)
      free(result);
    if(data)
      free(data);
    if(key)
      free(key);
  }

 tidy:
  
  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
