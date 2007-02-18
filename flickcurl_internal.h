/* internal functions */

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>


#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


/* Prepare Flickr API request - form URI */
int flickcurl_prepare(flickcurl *fc, const char* method, const char* parameters[][2], int count);

/* Invoke Flickr API at URi prepared above and get back an XML document */
xmlDocPtr flickcurl_invoke(flickcurl *fc);

/* Convert a unix timestamp into an ISO string */
char* flickcurl_unixtime_to_isotime(time_t unix_time);

/* Evaluate an XPath to get the string value */
char* flickcurl_xpath_eval(flickcurl *fc, xmlXPathContextPtr xpathCtx, const xmlChar* xpathExpr);

/* invoke an error */
void flickcurl_error(flickcurl* fc, const char *message, ...);

extern const char* flickcurl_context_type_element[FLICKCURL_CONTEXT_LAST+2];

flickcurl_context** flickcurl_build_contexts(flickcurl* fc, xmlDocPtr doc);

/* MD5 as hex string */
extern char* MD5_string(char *string);

/* my_vsnprintf */
extern char* my_vsnprintf(const char *message, va_list arguments);



struct flickcurl_s {
  int total_bytes;

  /* Something failed */
  int failed;

  /* Flickr API error code */
  int error_code;

  /* Flickr API error message */
  char* error_msg;
  
  int status_code;

  char uri[2048];

  CURL* curl_handle;
  char error_buffer[CURL_ERROR_SIZE];
  int curl_init_here;

  char* user_agent;

  /* proxy URL string or NULL for none */
  char* proxy;
  
  void* error_data;
  flickcurl_message_handler error_handler;

  char *http_accept;

  /* XML parser */
  xmlParserCtxtPtr xc;

  /* The next three fields need to be set before authenticated
   * operations can be done (in most cases).
   */

  /* Flickr shared secret - flickcurl_set_shared_secret() */
  char* secret;

  /* Flickr application/api key  - flickcurl_set_api_key() */
  char* api_key;

  /* Flickr authentication token - flickcurl_set_auth_token() */
  char* auth_token;


  /* signature parameter (shared) for authenticated calls ("api_sig"
   * usually) or  NULL for where it is not needed - flickcurl_set_sig_key()
   */
  char* sig_key;

  /* Flickr API method to invoke - set by flickcurl_prepare */
  char* method;

  flickcurl_tag_handler tag_handler;
  void* tag_data;

  /* licenses returned by flickr.photos.licenses.getInfo 
   * as initialised by flickcurl_read_licenses() 
   */
  flickcurl_license** licenses;

  /* Time the last request was made */
  struct timeval last_request_time;
  
  /* Delay between HTTP requests in microseconds - default is none (0) */
  long request_delay;
};
