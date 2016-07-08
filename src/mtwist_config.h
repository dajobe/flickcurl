/* Flickcurl configure wrapper for libmtwist
 *
 * Includes configuration file
 * Adjusts symbols to be all flickcurl_ prefixed
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#define mtwist_new flickcurl_mtwist_new
#define mtwist_free flickcurl_mtwist_free
#define mtwist_init flickcurl_mtwist_init
#define mtwist_u32rand flickcurl_mtwist_u32rand
#define mtwist_drand flickcurl_mtwist_drand
#define mtwist_seed_from_system flickcurl_mtwist_seed_from_system
