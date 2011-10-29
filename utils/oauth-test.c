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


static int
oauth_prepare(flickcurl *fc, flickcurl_oauth_data* od,
              const char* method, const char* parameters[][2], int count)
{
  if(!method) {
    flickcurl_error(fc, "No method to prepare");
    return 1;
  }
  
  return flickcurl_oauth_prepare_common(fc, od,
                                        fc->service_uri,
                                        method,
                                        NULL, NULL,
                                        parameters, count,
                                        1, 1, 1);
}


#define REQUEST_TOKEN_URL "http://www.flickr.com/services/oauth/request_token"

static int
oauth_request_token(flickcurl* fc, flickcurl_oauth_data* od)
{
  const char * parameters[2 + MAX_OAUTH_PARAM_COUNT][2];
  int count = 0;
  char* tmp_token = NULL;
  char* tmp_token_secret = NULL;
  char* data = NULL;
  size_t data_len = 0;
  int rc = 0;
  const char* uri = REQUEST_TOKEN_URL;

  parameters[count][0]  = NULL;

  /* Require signature */
  flickcurl_set_sign(fc);

  if(flickcurl_oauth_prepare_common(fc, od,
                                    uri,
                                    /* method */ "GET",
                                    /* upload_field */ NULL,
                                    /* upload_value */ NULL,
                                    parameters, count,
                                    /* parameters_in_url */ 1,
                                    /* need_auth */ 1,
                                    /* is_request */ 1)) {
    rc = 1;
    goto tidy;
  }

  /* FIXME does not invoke it */
  rc = 1;
  goto tidy;

  data = flickcurl_invoke_get_content(fc, &data_len);
  if(!data) {
    rc = 1;
    goto tidy;
  }

  fprintf(stdout, "Data is '%s'\n", data);

  if(tmp_token && tmp_token_secret) {
    /* Now owned by od */
    od->tmp_token = tmp_token;
    od->tmp_token_len = strlen(od->tmp_token);
    tmp_token = NULL;
    od->tmp_token_secret = tmp_token_secret;
    od->tmp_token_secret_len = strlen(od->tmp_token_secret);
    tmp_token_secret = NULL;

    fprintf(stderr, "Request returned token '%s' secret token '%s'\n",
            od->tmp_token, od->tmp_token_secret);
  }
  
  tidy:
  if(tmp_token)
    free(tmp_token);
  if(tmp_token_secret)
    free(tmp_token_secret);
  
  return rc;
}


static int
oauth_access_token(flickcurl* fc, flickcurl_oauth_data* od)
{
  const char * parameters[2 + MAX_OAUTH_PARAM_COUNT][2];
  int count = 0;
  char* tmp_token = NULL;
  char* tmp_token_secret = NULL;
  char* data = NULL;
  size_t data_len = 0;
  int rc = 0;
  const char* uri = "UNKNOWN";

  parameters[count][0]  = NULL;

  /* Require signature */
  flickcurl_set_sign(fc);

  if(flickcurl_oauth_prepare_common(fc, od,
                                    uri,
                                    /* method */ "GET",
                                    /* upload_field */ NULL,
                                    /* upload_value */ NULL,
                                    parameters, count,
                                    /* parameters_in_url */ 1,
                                    /* need_auth */ 1,
                                    /* is_request */ 0)) {
    rc = 1;
    goto tidy;
  }

  /* FIXME does not invoke it */
  rc = 1;
  goto tidy;

  data = flickcurl_invoke_get_content(fc, &data_len);
  if(!data) {
    rc = 1;
    goto tidy;
  }

  fprintf(stdout, "Data is '%s'\n", data);

  if(tmp_token && tmp_token_secret) {
    /* Now owned by od */
    od->tmp_token = tmp_token;
    od->tmp_token_len = strlen(od->tmp_token);
    tmp_token = NULL;
    od->tmp_token_secret = tmp_token_secret;
    od->tmp_token_secret_len = strlen(od->tmp_token_secret);
    tmp_token_secret = NULL;

    fprintf(stderr, "Request returned token '%s' secret token '%s'\n",
            od->tmp_token, od->tmp_token_secret);
  }
  
  tidy:
  if(tmp_token)
    free(tmp_token);
  if(tmp_token_secret)
    free(tmp_token_secret);
  
  return rc;
}


static int
oauth_test_echo(flickcurl* fc, flickcurl_oauth_data* od,
                const char* key, const char* value)
{
  const char * parameters[2 + MAX_OAUTH_PARAM_COUNT][2];
  int count = 0;
  xmlDocPtr doc = NULL;
  int rc = 0;
  
  parameters[count][0]  = key;
  parameters[count++][1]= value;

  parameters[count][0]  = NULL;

  if(oauth_prepare(fc, od, "flickr.test.echo", parameters, count)) {
    rc = 1;
    goto tidy;
  }

  doc = flickcurl_invoke(fc);
  if(!doc) {
    rc = 1;
    goto tidy;
  }

  fprintf(stderr, "Flickr echo returned %d bytes\n", fc->total_bytes);
  
  tidy:
  
  return rc;
}


/* Test KEY fields */
static const char* test_client_secret = "a9567d986a7539fe";
static const char* test_token_secret  = NULL;

/* Test DATA fields */
static const char* test_http_request_method = "GET";
static const char* test_uri_base_string     = "http://www.flickr.com/services/oauth/request_token";
static const char* test_oauth_callback_url  = "http://www.example.com";
static const char* test_oauth_consumer_key  = "653e7a6ecc1d528c516cc8f92cf98611";
static const char* test_oauth_nonce         = "95613465";
static const time_t test_oauth_timestamp     = (time_t)1305586162;

static const char* test_request_parameters  = "oauth_callback=http%3A%2F%2Fwww.example.com&oauth_consumer_key=653e7a6ecc1d528c516cc8f92cf98611&oauth_nonce=95613465&oauth_signature_method=HMAC-SHA1&oauth_timestamp=1305586162&oauth_version=1.0";

/* Expected results */
static const char* expected_key = 
      "a9567d986a7539fe"
      "&";
static const char* expected_data =
      "GET" "&"
      "http%3A%2F%2Fwww.flickr.com%2Fservices%2Foauth%2Frequest_token" "&"
      "oauth_callback%3Dhttp%253A%252F%252Fwww.example.com%26oauth_consumer_key%3D653e7a6ecc1d528c516cc8f92cf98611%26oauth_nonce%3D95613465%26oauth_signature_method%3DHMAC-SHA1%26oauth_timestamp%3D1305586162%26oauth_version%3D1.0";

static const char* expected_signature = "7w18YS2bONDPL%2FzgyzP5XTr5af4%3D";

static void
oauth_init_test_secrets(flickcurl_oauth_data *od)
{
  /* Set up test data */
  od->client_secret = test_client_secret;
  if(od->client_secret)
    od->client_secret_len = strlen(od->client_secret);

  od->token_secret = test_token_secret;
  if(od->token_secret)
    od->token_secret_len = strlen(od->token_secret);
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
    if(flickcurl_config_read_ini(fc, config_path, config_section, fc,
                                 flickcurl_config_var_handler)) {
      rc = 1;
      goto tidy;
    }
  }


  /* FIXME */
  srand(getpid());

  /* Request token */
  if(0) {
    flickcurl_oauth_data od;

    memset(&od, '\0', sizeof(od));

#if 0
    od.callback = test_oauth_callback_url;
    od.client_key = test_oauth_consumer_key;
    od.nonce = test_oauth_nonce;
    od.timestamp = test_oauth_timestamp;

    oauth_init_test_secrets(&od);
#endif
#if 0
    od.client_key = fc->api_key;
    od.client_secret = fc->secret;
    od.client_secret_len = strlen(od.client_secret);
#endif

    od.client_len = strlen(od.client_key);
    
    rc = oauth_request_token(fc, &od);
  }


  /* Access token */
  if(1) {
    flickcurl_oauth_data od;

    memset(&od, '\0', sizeof(od));

#if 0
    od.callback = test_oauth_callback_url;
    od.client_key = test_oauth_consumer_key;
    od.nonce = test_oauth_nonce;
    od.timestamp = test_oauth_timestamp;

    oauth_init_test_secrets(&od);
#endif
#if 0
    od.client_key = fc->api_key;
    od.client_secret = fc->secret;
    od.client_secret_len = strlen(od.client_secret);
#endif

    od.client_len = strlen(od.client_key);
    
    rc = oauth_access_token(fc, &od);
  }


  if(0) {
    flickcurl_oauth_data od;

    memset(&od, '\0', sizeof(od));

    rc = oauth_test_echo(fc, &od, "hello", "world");
  }
  
  if(0) {
    char *s = NULL;
    char *escaped_s = NULL;
    size_t escaped_s_len;
    char* signature;
    flickcurl_oauth_data od;

    memset(&od, '\0', sizeof(od));
  
    oauth_init_test_secrets(&od);
    
    rc = flickcurl_oauth_build_key_data(&od, test_http_request_method,
                                        test_uri_base_string, 
                                        test_request_parameters);
    
    fprintf(stderr, "%s: key is (%d bytes)\n  %s\n", program, (int)od.key_len, od.key);
    fprintf(stderr, "%s: expected key is\n  %s\n", program, expected_key);

    fprintf(stderr, "%s: data is (%d bytes)\n  %s\n", program, (int)od.data_len, od.data);
    fprintf(stderr, "%s: expected data is\n  %s\n", program, expected_data);

    signature = flickcurl_oauth_compute_signature(&od, &escaped_s_len);

    escaped_s = curl_escape((char*)signature, 0);
    free(signature);
    escaped_s_len = strlen(escaped_s);

    fprintf(stdout, "URI Escaped result (%d bytes):\n   %s\n",
            (int)escaped_s_len, escaped_s);
    
    fprintf(stdout, "Expected URI escaped result\n   %s\n", 
            expected_signature);
    
    curl_free(escaped_s);

    if(s)
      free(s);
    if(od.data)
      free(od.data);
    if(od.key)
      free(od.key);
  }

 tidy:
  
  if(fc)
    flickcurl_free(fc);

  flickcurl_finish();

  return(rc);
}
