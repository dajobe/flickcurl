/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * photo.c - Flickcurl photo functions
 *
 * Copyright (C) 2007-2010, David Beckett http://www.dajobe.org/
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
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


static const char* flickcurl_photo_field_label[PHOTO_FIELD_LAST+1] = {
  "(none)",
  "dateuploaded",
  "farm",
  "isfavorite",
  "license",
  "originalformat",
  "rotation",
  "server",
  "dates_lastupdate",
  "dates_posted",
  "dates_taken",
  "dates_takengranularity",
  "description",
  "editability_canaddmeta",
  "editability_cancomment",
  "geoperms_iscontact",
  "geoperms_isfamily",
  "geoperms_isfriend",
  "geoperms_ispublic",
  "location_accuracy",
  "location_latitude",
  "location_longitude",
  "owner_location",
  "owner_nsid",
  "owner_realname",
  "owner_username",
  "title",
  "visibility_isfamily",
  "visibility_isfriend",
  "visibility_ispublic",
  "secret",
  "originalsecret",
  "location_neighbourhood",
  "location_locality",
  "location_county",
  "location_region",
  "location_country",
  "location_placeid",
  "neighbourhood_placeid",
  "locality_placeid",
  "county_placeid",
  "region_placeid",
  "country_placeid",
  "location_woeid",
  "neighbourhood_woeid",
  "locality_woeid",
  "county_woeid",
  "region_woeid",
  "country_woeid",
  "usage_candownload",
  "usage_canblog",
  "usage_canprint",
  "owner_iconserver",
  "owner_iconfarm",
  "original_width",
  "original_height",
  "views",
  "comments",
  "favorites",
  "gallery_comment"
};


/**
 * flickcurl_get_photo_field_label:
 * @field: field enum
 *
 * Get label for photo field.
 *
 * Return value: label string or NULL if none valid
 */
const char*
flickcurl_get_photo_field_label(flickcurl_photo_field_type field)
{
  if(field <= PHOTO_FIELD_LAST)
    return flickcurl_photo_field_label[(int)field];
  return NULL;
}


/**
 * flickcurl_free_photo:
 * @photo: photo object
 *
 * Destructor for photo object
 */
void
flickcurl_free_photo(flickcurl_photo *photo)
{
  int i;

  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(photo, flickcurl_photo);

  for(i = 0; i <= PHOTO_FIELD_LAST; i++) {
    if(photo->fields[i].string)
      free(photo->fields[i].string);
  }
  
  for(i = 0; i < photo->tags_count; i++)
    flickcurl_free_tag(photo->tags[i]);
  free(photo->tags);

  for(i = 0; i < photo->notes_count; i++)
    flickcurl_free_note(photo->notes[i]);
  free(photo->notes);

  if(photo->id)
    free(photo->id);
  
  if(photo->uri)
    free(photo->uri);
  
  if(photo->media_type)
    free(photo->media_type);
  
  if(photo->place)
    flickcurl_free_place(photo->place);
  
  if(photo->video)
    flickcurl_free_video(photo->video);
  
  free(photo);
}


/**
 * flickcurl_photo_as_source_uri:
 * @photo: photo object
 * @c: size s, m, t or b
 *
 * Get a photo's image source URIs
 *
 * @c can be s,m,t,b for sizes, o for original, otherwise default
 * http://www.flickr.com/services/api/misc.urls.html
 *
 * Return value: new source URI string or NULL on failure
 */
char*
flickcurl_photo_as_source_uri(flickcurl_photo *photo, const char c)
{
  char buf[1024];
  char *result;
  size_t len;
  
  if(c == 'o') {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{o-secret}_o.(jpg|gif|png) */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s_o.%s", 
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_originalsecret].string,
            photo->fields[PHOTO_FIELD_originalformat].string);
  } else if (c == 'm' || c == 's' || c == 't' || c == 'b') {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}_[mstb].jpg */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s_%c.jpg",
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_secret].string,
            c);
  } else {
    /* http://farm{farm-id}.static.flickr.com/{server-id}/{id}_{secret}.jpg */
    sprintf(buf, "http://farm%s.static.flickr.com/%s/%s_%s.jpg",
            photo->fields[PHOTO_FIELD_farm].string,
            photo->fields[PHOTO_FIELD_server].string,
            photo->id,
            photo->fields[PHOTO_FIELD_secret].string);
  }
  len = strlen(buf);
  result = (char*)malloc(len+1);
  strcpy(result, buf);
  return result;
}


/**
 * flickcurl_photo_as_page_uri:
 * @photo: photo object
 *
 * Get a photo's page URI
 *
 * Return value: new source URI string or NULL on failure
 */
char*
flickcurl_photo_as_page_uri(flickcurl_photo *photo)
{
  char buf[1024];
  char *result;
  size_t len;
  
  /* http://www.flickr.com/photos/{owner}/{photo id}/ */
  sprintf(buf, "http://www.flickr.com/photos/%s/%s", 
          photo->fields[PHOTO_FIELD_owner_nsid].string, photo->id);

  len = strlen(buf);
  result = (char*)malloc(len+1);
  strncpy(result, buf, len+1);
  return result;
}


#define SHORT_URI_ALPHABET_SIZE 58
static const char short_uri_alphabet[SHORT_URI_ALPHABET_SIZE+1]=
  "123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ";

#define SHORT_URI_PREFIX_LEN 17
static const char short_uri_prefix[SHORT_URI_PREFIX_LEN+1] = 
  "http://flic.kr/p/";


/**
 * flickcurl_photo_id_as_short_uri:
 * @photo_id: photo ID
 *
 * Get a short URI for a photo ID
 *
 * Encoded based on description given in
 * http://www.flickr.com/groups/api/discuss/72157616713786392/
 *
 * Return value: new short URI string or NULL on failure
 */
char*
flickcurl_photo_id_as_short_uri(char *photo_id)
{
  char buf[SHORT_URI_ALPHABET_SIZE + 1];
  int base_count = SHORT_URI_ALPHABET_SIZE;
  char *p;
  char *r;
  long long num = atoll(photo_id);
  char *result;

  if(num <= 0)
    return NULL;
  
  /* http://flic.kr/p/{base58-photo id}/ */

  /* construct the encoding in reverse order into buf */
  p = buf;
  while(num >= base_count) {
    double divisor = num / base_count;
    long long modulus = (num - (base_count * (long long)divisor));
    *p++ = short_uri_alphabet[modulus];
    num = (long long)divisor;
  }
  if(num)
    *p++ = short_uri_alphabet[num];
    
  result = (char*)malloc((p-buf) + SHORT_URI_PREFIX_LEN + 1);
  if(!result)
    return NULL;
  
  r = result;
  strncpy(result, short_uri_prefix, SHORT_URI_PREFIX_LEN);
  r += SHORT_URI_PREFIX_LEN;
  /* now copy it backwards into new result string */
  while(p != buf)
    *r++ = *--p;
  *r = '\0';

  return result;
}


/**
 * flickcurl_photo_as_short_uri:
 * @photo: photo object
 *
 * Get a short URI for a photo
 *
 * Encoded based on description given in
 * http://www.flickr.com/groups/api/discuss/72157616713786392/
 *
 * Return value: new short URI string or NULL on failure
 */
char*
flickcurl_photo_as_short_uri(flickcurl_photo *photo)
{
  return flickcurl_photo_id_as_short_uri(photo->id);
}


#define SOURCE_URI_MATCH1_LENGTH 11
static const char source_uri_match1[SOURCE_URI_MATCH1_LENGTH+1] = "http://farm";
#define SOURCE_URI_MATCH2_LENGTH 19
static const char source_uri_match2[SOURCE_URI_MATCH2_LENGTH+1] = ".static.flickr.com/";

/**
 * flickcurl_source_uri_as_photo_id:
 * @uri: source uri
 *
 * Get a photo ID from an image source URI
 *
 * Turns an URL that points to the photo into a photo ID.
 * i.e. given an URI like these:
 * <code>http://farm{farm-id}.static.flickr.com/{server-id}/{photo-id}_{o-secret}_o.(jpg|gif|png)</code> or
 * <code>http://farm{farm-id}.static.flickr.com/{server-id}/{photo-id}_{secret}_[mstb].jpg</code>
 * <code>http://farm{farm-id}.static.flickr.com/{server-id}/{photo-id}_{secret}.jpg</code>
 * returns the {photo-id}
 *
 * Return value: new photo ID string or NULL on failure
 */
char*
flickcurl_source_uri_as_photo_id(const char *uri)
{
  const char* p = uri;
  const char* q = NULL;
  char *photo_id = NULL;
  size_t len = 0;
  
  if(!p)
    return NULL;

  if(memcmp(p, source_uri_match1, SOURCE_URI_MATCH1_LENGTH))
    return NULL;
  p+= SOURCE_URI_MATCH1_LENGTH;

  /* now at {farm-id}.static... */
  while(isdigit(*p))
    p++;

  if(memcmp(p, source_uri_match2, SOURCE_URI_MATCH2_LENGTH))
    return NULL;
  p+= SOURCE_URI_MATCH2_LENGTH;

  /* now at {server-id}/{photo_id}_... */
  while(isdigit(*p))
    p++;
  if(*p++ != '/')
    return NULL;

  /* now at {photo_id}_... */
  q = p;
  while(isdigit(*q))
    q++;
  if(*q != '_')
    return NULL;

  len = q-p;
  photo_id = (char*)malloc(len+1);
  if(!photo_id)
    return NULL;
  
  memcpy(photo_id, p, len);
  photo_id[len] = '\0';

  return photo_id;
}


/**
 * flickcurl_user_icon_uri:
 * @farm: user icon farm
 * @server: user icon server or 0
 * @nsid: user nsid
 *
 * Get the user's icon URI
 *
 * The icon URI returned is always a 48x48 pixel JPEG.
 *
 * If @server is 0 (or the other fields are NULL), the default icon URI of
 * http://www.flickr.com/images/buddyicon.jpg is returned.
 *
 * Defined by http://www.flickr.com/services/api/misc.buddyicons.html
 *
 * Return value: new icon URI string or NULL on failure
 */
char*
flickcurl_user_icon_uri(int farm, int server, char *nsid)
{
  char buf[1024];
  char *result;
  size_t len;
  
  if(server && farm && nsid)
  /* http://farm{icon-farm}.static.flickr.com/{icon-server}/buddyicons/{nsid}.jpg */
    sprintf(buf, "http://farm%d.static.flickr.com/%d/buddicons/%s.jpg", 
            farm, server, nsid);
  else
    strcpy(buf, "http://www.flickr.com/images/buddyicon.jpg");

  len = strlen(buf);
  result = (char*)malloc(len+1);
  strncpy(result, buf, len+1);
  return result;
}


/**
 * flickcurl_photo_as_user_icon_uri:
 * @photo: photo object
 *
 * Get the user's icon URI
 *
 * The icon URI returned is always a 48x48 pixel JPEG
 *
 * Return value: new icon URI string or NULL on failure
 */
char*
flickcurl_photo_as_user_icon_uri(flickcurl_photo *photo)
{
  return flickcurl_user_icon_uri(
            photo->fields[PHOTO_FIELD_owner_iconfarm].integer,
            photo->fields[PHOTO_FIELD_owner_iconserver].integer,
            photo->fields[PHOTO_FIELD_owner_nsid].string);
}


static struct {
  const xmlChar* xpath;
  flickcurl_photo_field_type field;
  flickcurl_field_value_type type;
} photo_fields_table[] = {
  {
    (const xmlChar*)"./@id",
    PHOTO_FIELD_none,
    VALUE_TYPE_PHOTO_ID,
  }
  ,
  {
    (const xmlChar*)"./urls/url[@type = \"photopage\"]",
    PHOTO_FIELD_none,
    VALUE_TYPE_PHOTO_URI
  }
  ,
  {
    (const xmlChar*)"./@media",
    PHOTO_FIELD_none,
    VALUE_TYPE_MEDIA_TYPE
  }
  ,
  {
    (const xmlChar*)"./@dateuploaded",
    PHOTO_FIELD_dateuploaded,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./@dateupload",
    PHOTO_FIELD_dateuploaded,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./@farm",
    PHOTO_FIELD_farm,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@isfavorite",
    PHOTO_FIELD_isfavorite,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@license",
    PHOTO_FIELD_license,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@originalformat",
    PHOTO_FIELD_originalformat,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@rotation",
    PHOTO_FIELD_rotation,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@server",
    PHOTO_FIELD_server,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@tags",
    PHOTO_FIELD_none,
    VALUE_TYPE_TAG_STRING
  }
  ,
  {
    (const xmlChar*)"./@owner",
    PHOTO_FIELD_owner_nsid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@ownername",
    PHOTO_FIELD_owner_realname,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@place_id",
    PHOTO_FIELD_location_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@woeid",
    PHOTO_FIELD_location_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@accuracy",
    PHOTO_FIELD_location_accuracy,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@latitude",
    PHOTO_FIELD_location_latitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"./@longitude",
    PHOTO_FIELD_location_longitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"./@datetaken",
    PHOTO_FIELD_dates_taken,
    VALUE_TYPE_DATETIME
  }
  ,
  {
    (const xmlChar*)"./@lastupdate",
    PHOTO_FIELD_dates_lastupdate,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./@datetakengranularity",
    PHOTO_FIELD_dates_takengranularity,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./dates/@lastupdate",
    PHOTO_FIELD_dates_lastupdate,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./dates/@posted",
    PHOTO_FIELD_dates_posted,
    VALUE_TYPE_UNIXTIME
  }
  ,
  {
    (const xmlChar*)"./dates/@taken",
    PHOTO_FIELD_dates_taken,
    VALUE_TYPE_DATETIME
  }
  ,
  {
    (const xmlChar*)"./dates/@takengranularity",
    PHOTO_FIELD_dates_takengranularity,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./description",
    PHOTO_FIELD_description,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./editability/@canaddmeta",
    PHOTO_FIELD_editability_canaddmeta,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./editability/@cancomment",
    PHOTO_FIELD_editability_cancomment,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@iscontact",
    PHOTO_FIELD_geoperms_iscontact,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@isfamily",
    PHOTO_FIELD_geoperms_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@isfriend",
    PHOTO_FIELD_geoperms_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./geoperms/@ispublic",
    PHOTO_FIELD_geoperms_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./location/@accuracy",
    PHOTO_FIELD_location_accuracy,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./location/@latitude",
    PHOTO_FIELD_location_latitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"./location/@longitude",
    PHOTO_FIELD_location_longitude,
    VALUE_TYPE_FLOAT
  }
  ,
  {
    (const xmlChar*)"./location/neighborhood",
    PHOTO_FIELD_location_neighbourhood,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighbourhood",
    PHOTO_FIELD_location_neighbourhood,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/locality",
    PHOTO_FIELD_location_locality,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighborhood",
    PHOTO_FIELD_location_neighbourhood,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighbourhood",
    PHOTO_FIELD_location_neighbourhood,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/region",
    PHOTO_FIELD_location_region,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/country",
    PHOTO_FIELD_location_country,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@location",
    PHOTO_FIELD_owner_location,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@nsid",
    PHOTO_FIELD_owner_nsid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@realname",
    PHOTO_FIELD_owner_realname,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./owner/@username",
    PHOTO_FIELD_owner_username,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./title",
    PHOTO_FIELD_title,
    VALUE_TYPE_STRING
  }
  ,
  /* title can also appear as an attribute in a photo summary */
  {
    (const xmlChar*)"./@title",
    PHOTO_FIELD_title,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./visibility/@isfamily",
    PHOTO_FIELD_visibility_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./visibility/@isfriend",
    PHOTO_FIELD_visibility_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./visibility/@ispublic",
    PHOTO_FIELD_visibility_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  /* these can also appear in an attribute in a photo summary */
  {
    (const xmlChar*)"./@isfamily",
    PHOTO_FIELD_visibility_isfamily,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@isfriend",
    PHOTO_FIELD_visibility_isfriend,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@ispublic",
    PHOTO_FIELD_visibility_ispublic,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@secret",
    PHOTO_FIELD_secret,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./@originalsecret",
    PHOTO_FIELD_originalsecret,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/@place_id",
    PHOTO_FIELD_location_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/@woeid",
    PHOTO_FIELD_location_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighborhood/@place_id",
    PHOTO_FIELD_neighbourhood_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighbourhood/@place_id",
    PHOTO_FIELD_neighbourhood_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighborhood/@woeid",
    PHOTO_FIELD_neighbourhood_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/neighbourhood/@woeid",
    PHOTO_FIELD_neighbourhood_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/locality/@place_id",
    PHOTO_FIELD_locality_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/locality/@woeid",
    PHOTO_FIELD_locality_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/region/@place_id",
    PHOTO_FIELD_region_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/region/@woeid",
    PHOTO_FIELD_region_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/county/@place_id",
    PHOTO_FIELD_county_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/county/@woeid",
    PHOTO_FIELD_county_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/country/@place_id",
    PHOTO_FIELD_country_placeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./location/country/@woeid",
    PHOTO_FIELD_country_woeid,
    VALUE_TYPE_STRING
  }
  ,
  {
    (const xmlChar*)"./usage/@candownload",
    PHOTO_FIELD_usage_candownload,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./usage/@canblog",
    PHOTO_FIELD_usage_canblog,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./usage/@canprint",
    PHOTO_FIELD_usage_canprint,
    VALUE_TYPE_BOOLEAN
  }
  ,
  {
    (const xmlChar*)"./@iconserver",
    PHOTO_FIELD_owner_iconserver,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@iconfarm",
    PHOTO_FIELD_owner_iconfarm,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@o_width",
    PHOTO_FIELD_original_width,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@o_height",
    PHOTO_FIELD_original_height,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./@views",
    PHOTO_FIELD_views,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./stats/@views",
    PHOTO_FIELD_views,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./stats/@comments",
    PHOTO_FIELD_comments,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./stats/@favorites",
    PHOTO_FIELD_favorites,
    VALUE_TYPE_INTEGER
  }
  ,
  {
    (const xmlChar*)"./comment",
    PHOTO_FIELD_gallery_comment,
    VALUE_TYPE_STRING
  }
  ,
  { 
    NULL,
    (flickcurl_photo_field_type)0,
    (flickcurl_field_value_type)0
  }
};


flickcurl_photo**
flickcurl_build_photos(flickcurl* fc, xmlXPathContextPtr xpathCtx,
                       const xmlChar* xpathExpr, int* photo_count_p)
{
  flickcurl_photo** photos = NULL;
  int nodes_count;
  int photo_count;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlNodeSetPtr nodes;
  xmlChar full_xpath[512];
  size_t xpathExpr_len;
  int i;
  
  xpathExpr_len = strlen((const char*)xpathExpr);
  strncpy((char*)full_xpath, (const char*)xpathExpr, xpathExpr_len+1);
  
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(!xpathObj) {
    flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                    xpathExpr);
    fc->failed = 1;
    goto tidy;
  }
  
  nodes = xpathObj->nodesetval;
  /* This is a max size - it can include nodes that are CDATA */
  nodes_count = xmlXPathNodeSetGetLength(nodes);
  photos = (flickcurl_photo**)calloc(sizeof(flickcurl_photo*), nodes_count+1);

  for(i = 0, photo_count = 0; i < nodes_count; i++) {
    xmlNodePtr node = nodes->nodeTab[i];
    flickcurl_photo* photo;
    int expri;
    xmlXPathContextPtr xpathNodeCtx = NULL;
    
    if(node->type != XML_ELEMENT_NODE) {
      flickcurl_error(fc, "Got unexpected node type %d", node->type);
      fc->failed = 1;
      break;
    }
    
    photo = (flickcurl_photo*)calloc(sizeof(flickcurl_photo), 1);

    /* set up a new XPath context relative to the current node */
    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    xpathNodeCtx->node = node;
    
    for(expri = 0; expri <= PHOTO_FIELD_LAST; expri++) {
      if(photo->fields[expri].string)
        free(photo->fields[expri].string);
      photo->fields[expri].string = NULL;
      photo->fields[expri].integer= (flickcurl_photo_field_type)-1;
      photo->fields[expri].type   = VALUE_TYPE_NONE;
    }

    for(expri = 0; photo_fields_table[expri].xpath; expri++) {
      char *string_value;
      flickcurl_field_value_type datatype = photo_fields_table[expri].type;
      int int_value= -1;
      flickcurl_photo_field_type field = photo_fields_table[expri].field;
      time_t unix_time;
      int special = 0;
      
      string_value = flickcurl_xpath_eval(fc, xpathNodeCtx,
                                        photo_fields_table[expri].xpath);
      if(!string_value)
        continue;

#if FLICKCURL_DEBUG > 1
        fprintf(stderr, "  type %d  string value '%s'\n", datatype,
                string_value);
#endif
      switch(datatype) {
        case VALUE_TYPE_PHOTO_ID:
          photo->id = string_value;
          string_value = NULL;
          datatype = VALUE_TYPE_NONE;
          break;

        case VALUE_TYPE_PHOTO_URI:
          photo->uri = string_value;
          string_value = NULL;
          datatype = VALUE_TYPE_NONE;
          break;

        case VALUE_TYPE_MEDIA_TYPE:
          photo->media_type = string_value;
          string_value = NULL;
          datatype = VALUE_TYPE_NONE;
          break;

        case VALUE_TYPE_UNIXTIME:
        case VALUE_TYPE_DATETIME:

          if(datatype == VALUE_TYPE_UNIXTIME)
            unix_time = atoi(string_value);
          else
            unix_time = curl_getdate((const char*)string_value, NULL);

          if(unix_time >= 0) {
            char* new_value = flickcurl_unixtime_to_isotime(unix_time);
#if FLICKCURL_DEBUG > 1
            fprintf(stderr, "  date from: '%s' unix time %ld to '%s'\n",
                    string_value, (long)unix_time, new_value);
#endif
            free(string_value);
            string_value= new_value;
            int_value= (int)unix_time;
            datatype = VALUE_TYPE_DATETIME;
          } else
            /* failed to convert, make it a string */
            datatype = VALUE_TYPE_STRING;
          break;

        case VALUE_TYPE_INTEGER:
        case VALUE_TYPE_BOOLEAN:
          if(!*string_value && datatype == VALUE_TYPE_BOOLEAN) {
            /* skip setting field with a boolean value '' */
            special = 1;
            break;
          }

          int_value = atoi(string_value);
          break;

        case VALUE_TYPE_TAG_STRING:
          /* A space-separated list of tags */
          photo->tags = flickcurl_build_tags_from_string(fc, photo,
                                                         (const char*)string_value,
                                                         &photo->tags_count);
          special = 1;
          break;


        case VALUE_TYPE_NONE:
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_FLOAT:
        case VALUE_TYPE_URI:
          break;

        case VALUE_TYPE_PERSON_ID:
        case VALUE_TYPE_COLLECTION_ID:
        case VALUE_TYPE_ICON_PHOTOS:
          abort();
      }

      if(special)
        continue;

      photo->fields[field].string = string_value;
      photo->fields[field].integer= (flickcurl_photo_field_type)int_value;
      photo->fields[field].type   = datatype;

  #if FLICKCURL_DEBUG > 1
      fprintf(stderr, "field %d with %s value: '%s' / %d\n",
              field, flickcurl_get_field_value_type_label(datatype), 
              string_value, int_value);
  #endif

      if(fc->failed)
        goto tidy;
    } /* end for */

    if(!photo->tags)
      photo->tags = flickcurl_build_tags(fc, photo, xpathNodeCtx, 
                                       (const xmlChar*)"./tags/tag",
                                       &photo->tags_count);

    if(!photo->place)
      photo->place = flickcurl_build_place(fc, xpathNodeCtx,
                                         (const xmlChar*)"./location");

    photo->video = flickcurl_build_video(fc, xpathNodeCtx,
                                       (const xmlChar*)"./video");
    
    photo->notes = flickcurl_build_notes(fc, photo, xpathNodeCtx, 
                                         (const xmlChar*)"./notes/note",
                                         &photo->notes_count);

    if(!photo->media_type) {
      photo->media_type = (char*)malloc(6);
      strncpy(photo->media_type, "photo", 6);
    }

    if(xpathNodeCtx)
      xmlXPathFreeContext(xpathNodeCtx);

    photos[photo_count++] = photo;
  } /* for photos */
  
  if(photo_count_p)
    *photo_count_p = photo_count;

  tidy:
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  if(fc->failed)
    photos = NULL;

  return photos;
}


flickcurl_photo*
flickcurl_build_photo(flickcurl* fc, xmlXPathContextPtr xpathCtx)
{
  flickcurl_photo** photos;
  flickcurl_photo* result = NULL;

  photos = flickcurl_build_photos(fc, xpathCtx,
                                (const xmlChar*)"/rsp/photo", NULL);
  if(photos) {
    result = photos[0];
    free(photos);
  }
  
  return result;
}


/**
 * flickcurl_free_photos:
 * @photos: photo object array
 *
 * Destructor for array of photo objects
 */
void
flickcurl_free_photos(flickcurl_photo** photos)
{
  int i;
  
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(photos, flickcurl_photo_array);

  for(i = 0; photos[i]; i++)
    flickcurl_free_photo(photos[i]);
  free(photos);
}


/*
 * flickcurl_invoke_photos_list:
 * @fc: Flickcurl context
 * @xpathExpr: Xpath to the list of photos e.g. '/rsp/photos' or '/rsp/gallery'.  The /photos suffix is added internally.
 * @format: result format wanted
 *
 * INTERNAL - Build photos list from XML or get format content result from web service response document
 *
 * Return value: new photos list or NULL on failure
 */
flickcurl_photos_list*
flickcurl_invoke_photos_list(flickcurl* fc, const xmlChar* xpathExpr,
                             const char* format)
{
  flickcurl_photos_list* photos_list = NULL;
  xmlXPathContextPtr xpathCtx = NULL;
  xmlXPathObjectPtr xpathObj = NULL;
  xmlXPathContextPtr xpathNodeCtx = NULL;
  const char *nformat;
  size_t format_len;

  photos_list = (flickcurl_photos_list*)calloc(1, sizeof(*photos_list));
  if(!photos_list) {
    fc->failed = 1;
    goto tidy;
  }

  photos_list->page = -1;
  photos_list->per_page = -1;
  photos_list->total_count = -1;
  
  if(format) {
    nformat = format;
    format_len = strlen(format);
  
    photos_list->content = flickcurl_invoke_get_content(fc,
                                                        &photos_list->content_length);
    if(!photos_list->content) {
      fc->failed = 1;
      goto tidy;
    }

  } else {
    xmlDocPtr doc = NULL;
    xmlNodePtr photos_node;
    size_t xpathExprLen = strlen((const char*)xpathExpr);
    char* value;
    xmlChar* photosXpathExpr;
#define SUFFIX "/photo"
#define SUFFIX_LEN 6

    nformat = "xml";
    format_len = 3;
    
    doc = flickcurl_invoke(fc);
    if(!doc)
      goto tidy;

    xpathCtx = xmlXPathNewContext(doc);
    if(!xpathCtx) {
      flickcurl_error(fc, "Failed to create XPath context for document");
      fc->failed = 1;
      goto tidy;
    }

    /* set up a new XPath context for the top level list-of-photos
     * XML element.  It may be <photos> or <gallery> or ... - the
     * code does not care.
     */

    xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
    if(!xpathObj) {
      flickcurl_error(fc, "Unable to evaluate XPath expression \"%s\"", 
                      xpathExpr);
      fc->failed = 1;
      goto tidy;
    }

    if(!xpathObj->nodesetval || !xpathObj->nodesetval->nodeTab) {
      /* No <photo> elements found in content - not a failure */
      goto tidy;
    }

    photos_node = xpathObj->nodesetval->nodeTab[0];

    xpathNodeCtx = xmlXPathNewContext(xpathCtx->doc);
    if(!xpathNodeCtx) {
      flickcurl_error(fc, "Unable to create XPath context for XPath \"%s\"", 
                      xpathExpr);
      fc->failed = 1;
      goto tidy;
    }
    
    xpathNodeCtx->node = photos_node;

    value = flickcurl_xpath_eval(fc, xpathNodeCtx,
                                 (const xmlChar*)"./@page");
    if(value) {
      photos_list->page = atoi(value);
      free(value);
    }

    value = flickcurl_xpath_eval(fc, xpathNodeCtx,
                                 (const xmlChar*)"./@perpage");
    if(value) {
      photos_list->per_page = atoi(value);
      free(value);
    }

    value = flickcurl_xpath_eval(fc, xpathNodeCtx,
                                 (const xmlChar*)"./@total");
    if(value) {
      photos_list->total_count = atoi(value);
      free(value);
    }

    /* finished with these */
    xmlXPathFreeContext(xpathNodeCtx);
    xpathNodeCtx = NULL;
    xmlXPathFreeObject(xpathObj);
    xpathObj = NULL;


    photosXpathExpr = (xmlChar*)malloc(xpathExprLen + SUFFIX_LEN + 1);
    memcpy(photosXpathExpr, xpathExpr, xpathExprLen);
    memcpy(photosXpathExpr + xpathExprLen, SUFFIX, SUFFIX_LEN + 1);
  
    photos_list->photos = flickcurl_build_photos(fc, xpathCtx, photosXpathExpr,
                                                 &photos_list->photos_count);
    if(!photos_list->photos) {
      fc->failed = 1;
      goto tidy;
    }

  }


  photos_list->format = (char*)malloc(format_len+1);
  if(!photos_list->format) {
    fc->failed = 1;
    goto tidy;
  }
  memcpy(photos_list->format, nformat, format_len+1);

  tidy:
  if(xpathNodeCtx)
    xmlXPathFreeContext(xpathNodeCtx);
  if(xpathObj)
    xmlXPathFreeObject(xpathObj);
  if(xpathCtx)
    xmlXPathFreeContext(xpathCtx);

  if(fc->failed) {
    if(photos_list)
      flickcurl_free_photos_list(photos_list);
    photos_list = NULL;
  }

  return photos_list;
}


/**
 * flickcurl_free_photos_list:
 * @photos_list: photos list object
 *
 * Destructor for photos list
 */
void
flickcurl_free_photos_list(flickcurl_photos_list* photos_list)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(photos_list, flickcurl_photos_list);

  if(photos_list->format)
    free(photos_list->format);
  if(photos_list->photos)
    flickcurl_free_photos(photos_list->photos);
  if(photos_list->content)
    free(photos_list->content);
  free(photos_list);
}
