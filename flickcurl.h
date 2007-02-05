/*
 *
 *
 */

#include <libxml/xpathInternals.h>


typedef struct flickcurl_s flickcurl;

struct flickcurl_photo_s;

typedef struct flickr_tag_s
{
  struct flickr_photo_s* photo;
  char* id;
  char* author;
  char* raw;
  char* cooked;
} flickr_tag;

typedef struct flickr_photo_s
{
  /* photo id */
  char *id;
  /* photo page uri */
  char *uri;
  
  flickr_tag* tags[20];
  int tags_count;
  
} flickr_photo;


typedef void (*flickcurl_message_handler)(void *user_data, const char *message);

typedef void (*set_config_var_handler)(void* userdata, const char* key, const char* value);

typedef void (*flickcurl_tag_handler)(void *user_data, flickr_tag* tag);


extern const char* const flickcurl_short_copyright_string;
extern const char* const flickcurl_copyright_string;
extern const char* const flickcurl_license_string;
extern const char* const flickcurl_home_url_string;
extern const char* const flickcurl_version_string;


/* constructor */
flickcurl* flickcurl_new(void);

/* destructor */
void flickcurl_free(flickcurl *fc);

/* error handler */
void flickcurl_set_error_handler(flickcurl* fc, flickcurl_message_handler error_handler,  void *error_data);
void flickcurl_set_tag_handler(flickcurl* fc,  flickcurl_tag_handler tag_handler, void *tag_data);

/* set methods */
void flickcurl_set_user_agent(flickcurl* fc, const char *user_agent);
void flickcurl_set_proxy(flickcurl* fc, const char *proxy);
void flickcurl_set_http_accept(flickcurl* fc, const char *value);
void flickcurl_set_api_key(flickcurl* fc, const char *api_key);
void flickcurl_set_shared_secret(flickcurl* fc, const char *secret);
void flickcurl_set_auth_token(flickcurl *fc, const char* auth_token);
void flickcurl_set_sig_key(flickcurl *fc, const char* sig_key);

/* get methods */
const char* flickcurl_get_api_key(flickcurl *fc);
const char* flickcurl_get_shared_secret(flickcurl *fc);
const char* flickcurl_get_auth_token(flickcurl *fc);

/* Flickr API calls */
char* flickcurl_auth_getFullToken(flickcurl* fc, const char* frob);

flickr_photo* flickcurl_photos_getInfo(flickcurl *fc, const char* photo_id);

int flickcurl_test_echo(flickcurl* fc, const char* key, const char* value);

/* helper */
void flickcurl_getInfo(flickcurl *fc, xmlDocPtr doc);


void free_flickr_tag(flickr_tag *t);
void free_flickr_photo(flickr_photo *photo);



/* config.c */
int read_ini_config(const char* filename, const char* application, void* user_data, set_config_var_handler handler);
