/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * serializer.c - Triples from photo metadata
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

/* Public namespaces */
#define DC_NS       "http://purl.org/dc/elements/1.1/"
#define DCTERMS_NS  "http://purl.org/dc/terms/"
/* predicates dc:creator dc:dateSubmitted dc:rights dc:modified dc:issued
 * dc:created dc:description dc:title */
#define FOAF_NS     "http://xmlns.com/foaf/0.1/"
/* predicates foaf:maker foaf:name foaf:nick
 * classes foaf:Person foaf:Image */
#define GEO_NS      "http://www.w3.org/2003/01/geo/wgs84_pos#"
/* predicates geo:lat geo:long */
#define RDFS_NS     "http://www.w3.org/2000/01/rdf-schema#"
/* predicates rdfs:label */
#define RDF_NS      "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
/* predicates rdf:type */
#define XSD_NS      "http://www.w3.org/2001/XMLSchema#"
/* XSD datatypes xsd:boolean xsd:dateTime xsd:double xsd:integer */

/* Flickr terminology namespaces */
#define FLICKR_NS   "http://machinetags.org/ns/Flickr#"
/* predicates flickr:image flickr:video flickr:width flickr:height
 * classes flickr:Video */
#define PLACES_NS   "http://machinetags.org/ns/Places#"
/* predicates places:place places:type places:name places:id places:placeid
 * places:url
 * class places:Place
 */

/* Machine tag namespaces */
#define BLUE_NS     "http://machinetags.org/ns/Blue#"
#define CELL_NS     "http://machinetags.org/ns/Cell#"
#define DOPPLR_NS   "http://machinetags.org/ns/Dopplr#"
#define FILTR_NS    "http://machinetags.org/ns/Filtr#"
#define GEONAMES_NS "http://machinetags.org/ns/Geonames#"
#define PH_NS       "http://machinetags.org/ns/Ph#"
#define UPCOMING_NS "http://machinetags.org/ns/Upcoming#"


struct flickrdf_nspace_s
{
  char* prefix;
  char* uri;
  size_t prefix_len;
  size_t uri_len;
  int seen;
  struct flickrdf_nspace_s* next;
};
typedef struct flickrdf_nspace_s flickrdf_nspace;

flickrdf_nspace namespace_table[]={
  { (char*)"a",        (char*)"http://www.w3.org/2000/10/annotation-ns" },
  { (char*)"acl",      (char*)"http://www.w3.org/2001/02/acls#" },
  { (char*)"blue",     (char*)BLUE_NS, },
  { (char*)"cell",     (char*)CELL_NS, },
  { (char*)"dc",       (char*)DC_NS },
  { (char*)"dcterms",  (char*)DCTERMS_NS },
  { (char*)"dopplr",   (char*)DOPPLR_NS },
  { (char*)"exif",     (char*)"http://nwalsh.com/rdf/exif#" },
  { (char*)"exifi",    (char*)"http://nwalsh.com/rdf/exif-intrinsic#" },
  { (char*)"flickr",   (char*)FLICKR_NS },
  { (char*)"filtr",    (char*)FILTR_NS, },
  { (char*)"foaf",     (char*)FOAF_NS },
  { (char*)"geo",      (char*)GEO_NS, },
  { (char*)"geonames", (char*)GEONAMES_NS, },
  { (char*)"i",        (char*)"http://www.w3.org/2004/02/image-regions#" },
  { (char*)"ph",       (char*)PH_NS },
  { (char*)"places",   (char*)PLACES_NS, },
  { (char*)"rdf",      (char*)RDF_NS },
  { (char*)"rdfs",     (char*)RDFS_NS },
  { (char*)"skos",     (char*)"http://www.w3.org/2004/02/skos/core" },
  { (char*)"upcoming", (char*)UPCOMING_NS },
  { (char*)"xsd",      (char*)XSD_NS, },
  { NULL, NULL }
};

#define FIELD_FLAGS_PERSON   1
#define FIELD_FLAGS_STRING   2
#define FIELD_FLAGS_SQL_DATE 4
#define FIELD_FLAGS_FLOAT    8

static struct {
  flickcurl_photo_field_type field;
  const char* nspace_uri;
  const char* name;
  int flags;
} field_table[]={
  /* dc:available -- date that the resource will become/did become available.*/
  /* dc:dateSubmitted - Date of submission of resource (e.g. thesis, articles)*/
  { PHOTO_FIELD_dateuploaded,       DCTERMS_NS, "dateSubmitted" },
  { PHOTO_FIELD_license,            DCTERMS_NS, "rights" },
  /* dc:modified - date on which the resource was changed. */
  { PHOTO_FIELD_dates_lastupdate,   DCTERMS_NS, "modified" },
  /* dc:issued - date of formal issuance (e.g. publication of the resource */
  { PHOTO_FIELD_dates_posted,       DCTERMS_NS, "issued" },
  /* dc:created - date of creation of the resource */
  { PHOTO_FIELD_dates_taken,        DCTERMS_NS, "created", FIELD_FLAGS_SQL_DATE },
  { PHOTO_FIELD_description,        DCTERMS_NS, "description" },
  { PHOTO_FIELD_location_latitude,  GEO_NS,     "lat",  FIELD_FLAGS_FLOAT },
  { PHOTO_FIELD_location_longitude, GEO_NS,     "long", FIELD_FLAGS_FLOAT },
  { PHOTO_FIELD_owner_realname,     FOAF_NS,    "name", FIELD_FLAGS_PERSON },
  { PHOTO_FIELD_owner_username,     FOAF_NS,    "nick", FIELD_FLAGS_PERSON },
  { PHOTO_FIELD_title,              DCTERMS_NS, "title" },
  { PHOTO_FIELD_none, NULL, NULL }
};


void
flickcurl_serializer_init(void)
{
  int i;

  for(i=0; namespace_table[i].prefix != NULL; i++) {
    namespace_table[i].uri_len=strlen(namespace_table[i].uri);
    namespace_table[i].prefix_len=strlen(namespace_table[i].prefix);
  }
}


void
flickcurl_serializer_terminate(void)
{
}


/**
 * flickcurl_new_serializer:
 * @fc: flickcurl object
 * @data: user data for factory methods
 * @factory: triples serializer factory
 *
 * Create a new triples serializer for generating a RDF triples
 * representation of a Flickr photo.
 *
 * The factory must contain the methods and the correct factory API
 * version as described in #flickcurl_serializer_factory
 *
 * Return value: a new serializer object or NULL on failure
 *
*/
flickcurl_serializer*
flickcurl_new_serializer(flickcurl* fc, 
                         void* data, flickcurl_serializer_factory* factory)
{
  flickcurl_serializer* serializer;

  if(!factory || (factory && factory->version != 1))
    return NULL;
  
  serializer=(flickcurl_serializer*)malloc(sizeof(flickcurl_serializer));
  if(!serializer)
    return NULL;
  
  serializer->fc=fc;
  serializer->data=data;
  serializer->factory=factory;
  return serializer;
}


/**
 * flickcurl_free_serializer:
 * @serializer: serializer object
 *
 * Destructor for triples serializer object
 */
void
flickcurl_free_serializer(flickcurl_serializer* serializer)
{
  FLICKCURL_ASSERT_OBJECT_POINTER_RETURN(serializer, flickcurl_serializer);

  free(serializer);
}


static flickrdf_nspace*
nspace_add_new(flickrdf_nspace* list, char* prefix, char *uri)
{
  flickrdf_nspace* ns;

  ns=(flickrdf_nspace*)malloc(sizeof(flickrdf_nspace));
  ns->prefix_len=strlen(prefix);
  ns->uri_len=strlen(uri);

  ns->prefix=(char*)malloc(ns->prefix_len+1);
  strcpy(ns->prefix, prefix);

  ns->uri=(char*)malloc(ns->uri_len+1);
  strcpy(ns->uri, uri);

  ns->next=list;
  return ns;
}


static flickrdf_nspace*
nspace_add_if_not_declared(flickrdf_nspace* list, 
                           const char* prefix, const char* nspace_uri)
{
  int n;
  flickrdf_nspace* ns;
  size_t prefix_len=prefix ? strlen(prefix) : 0;
  size_t uri_len=nspace_uri ? strlen(nspace_uri) : 0;
  
  for(ns=list; ns; ns=ns->next) {
    if(nspace_uri && ns->uri_len == uri_len && !strcmp(ns->uri, nspace_uri))
      break;
    if(prefix && ns->prefix_len == prefix_len && !strcmp(ns->prefix, prefix))
      break;
  }
  if(ns)
    return list;

  ns=NULL;
  for(n=0; namespace_table[n].uri; n++) {
    if(prefix && namespace_table[n].prefix_len == prefix_len && 
       !strcmp(namespace_table[n].prefix, prefix)) {
      ns=&namespace_table[n];
      break;
    }
    if(nspace_uri && namespace_table[n].uri_len == uri_len && 
       !strcmp(namespace_table[n].uri, nspace_uri)) {
      ns=&namespace_table[n];
      break;
    }
  }
  if(!ns)
    return list;

  /* ns was not found, copy it and add it to the list */
  return nspace_add_new(list, ns->prefix, ns->uri);
}


static flickrdf_nspace*
nspace_get_by_prefix(flickrdf_nspace* list, const char *prefix)
{
  flickrdf_nspace* ns;
  size_t prefix_len=strlen(prefix);
  
  for(ns=list; ns; ns=ns->next) {
    if(ns->prefix_len == prefix_len && !strcmp(ns->prefix, prefix))
      break;
  }
  return ns;
}


#if FLICKCURL_DEBUG > 1
static void
print_nspaces(FILE* fh, const char* label, flickrdf_nspace* list)
{
  flickrdf_nspace* ns;

  for(ns=list; ns; ns=ns->next) {
    fprintf(fh, "%s: Declaring namespace prefix %s URI %s\n",
            label, (ns->prefix ? ns->prefix : ":"),
            (ns->uri ? ns->uri : "\"\""));

  }
}
#endif    

static void
free_nspaces(flickrdf_nspace* list)
{
  flickrdf_nspace* next;

  for(; list; list=next) {
    next=list->next;
    if(list->prefix)
      free(list->prefix);
    free(list->uri);
    free(list);
  }
}
    

/**
 * flickcurl_serialize_photo:
 * @fcs: flickcurl serializer object
 * @photo: photo object
 *
 * Serialize photo description to RDF triples
 *
 * Return value: non-0 on failure
 */
int
flickcurl_serialize_photo(flickcurl_serializer* fcs, flickcurl_photo* photo)
{
  int i;
  int need_person=0;
  int need_foaf=0;
  int need_rdfs=0;
  flickrdf_nspace* nspaces=NULL;
  flickrdf_nspace* ns;
  flickcurl_serializer_factory* fsf=fcs->factory;
  flickcurl* fc=fcs->fc;
#if FLICKCURL_DEBUG > 1
  FILE* fh=stderr;
  const char* label="libflickcurl";
#endif
  flickcurl_size** sizes=NULL;

  if(!photo)
    return 1;

  /* Always add XSD, RDF and Flickr namespaces */
  nspaces=nspace_add_if_not_declared(nspaces, NULL, XSD_NS);
  nspaces=nspace_add_if_not_declared(nspaces, "rdf", RDF_NS);
  nspaces=nspace_add_if_not_declared(nspaces, "flickr", FLICKR_NS);

  if(photo->place)
    nspaces=nspace_add_if_not_declared(nspaces, "places", PLACES_NS);

  sizes=flickcurl_photos_getSizes(fc, photo->id);
  if(sizes) {
    need_foaf=1;
    need_rdfs=1;
  }

  /* mark namespaces used in fields */
  for(i=PHOTO_FIELD_FIRST; i <= PHOTO_FIELD_LAST; i++) {
    flickcurl_photo_field_type field=(flickcurl_photo_field_type)i;
    flickcurl_field_value_type datatype=photo->fields[field].type;
    int f;

    if(datatype == VALUE_TYPE_NONE)
      continue;

    for(f=0; field_table[f].field != PHOTO_FIELD_none; f++) {
      if(field_table[f].field != field) 
        continue;

      if(field_table[f].flags & FIELD_FLAGS_PERSON)
        need_person=1;

      nspaces=nspace_add_if_not_declared(nspaces, NULL, field_table[f].nspace_uri);
      break;
    }

  }
  

  /* in tags look for xmlns:PREFIX="URI" otherwise look for PREFIX: */
  for(i=0; i < photo->tags_count; i++) {
    char* prefix;
    char *p;
    flickcurl_tag* tag=photo->tags[i];

    if(!strncmp(tag->raw, "xmlns:", 6)) {
      prefix=&tag->raw[6];
      for(p=prefix; *p && *p != '='; p++)
        ;
      if(!*p) /* "xmlns:PREFIX" seen */
        continue;

      /* "xmlns:PREFIX=" seen */
      *p='\0';
      nspaces=nspace_add_new(nspaces, prefix, p+1);
#if FLICKCURL_DEBUG > 1
        fprintf(fh,
                "%s: Found declaration of namespace prefix %s uri %s in tag '%s'\n",
                label, prefix, p+1, tag->raw);
#endif
      *p='=';
      continue;
    }

    prefix=tag->raw;
    for(p=prefix; *p && *p != ':'; p++)
      ;
    if(!*p) /* "PREFIX:" seen */
      continue;

    *p='\0';
    nspaces=nspace_add_if_not_declared(nspaces, prefix, NULL);
    *p=':';
  }


  if(need_person) {
    need_foaf=1;
    nspaces=nspace_add_if_not_declared(nspaces, "dc", DCTERMS_NS);
  }
  
  if(need_foaf)
    nspaces=nspace_add_if_not_declared(nspaces, "foaf", FOAF_NS);

  if(need_rdfs)
    nspaces=nspace_add_if_not_declared(nspaces, "rdfs", RDFS_NS);


#if FLICKCURL_DEBUG > 1
  print_nspaces(fh, label, nspaces);
#endif

  /* generate seen namespace declarations */
  for(ns=nspaces; ns; ns=ns->next)
    fsf->emit_namespace(fcs->data,
                        ns->prefix, ns->prefix_len, ns->uri, ns->uri_len);
  

  if(need_person) {
    fsf->emit_triple(fcs->data,
                     photo->uri, FLICKCURL_TERM_TYPE_RESOURCE,
                     DCTERMS_NS, "creator",
                     "person", FLICKCURL_TERM_TYPE_BLANK,
                     NULL);
    fsf->emit_triple(fcs->data,
                     "person", FLICKCURL_TERM_TYPE_BLANK,
                     RDF_NS, "type",
                     FOAF_NS "Person", FLICKCURL_TERM_TYPE_RESOURCE,
                     NULL);
    fsf->emit_triple(fcs->data,
                     "person", FLICKCURL_TERM_TYPE_BLANK,
                     FOAF_NS, "maker",
                     photo->uri, FLICKCURL_TERM_TYPE_RESOURCE,
                     NULL);

  }
  

  /* generate triples from fields */
  for(i=PHOTO_FIELD_FIRST; i <= PHOTO_FIELD_LAST; i++) {
    flickcurl_photo_field_type field=(flickcurl_photo_field_type)i;
    flickcurl_field_value_type datatype=photo->fields[field].type;
    int f;

    if(datatype == VALUE_TYPE_NONE)
      continue;

    for(f=0; field_table[f].field != PHOTO_FIELD_none; f++) {
      const char* datatype_uri=NULL;
      char* object=NULL;
      char* new_object=NULL;
      int type= FLICKCURL_TERM_TYPE_LITERAL;
      
      if(field_table[f].field != field) 
        continue;

#if FLICKCURL_DEBUG > 1
        fprintf(fh,
                "%s: field %s (%d) with %s value: '%s' has predicate %s%s\n", 
                label,
                flickcurl_get_photo_field_label(field), field,
                flickcurl_get_field_value_type_label(datatype),
                photo->fields[field].string,
                field_table[f].nspace_uri, field_table[f].name);
#endif

      object=photo->fields[field].string;

      if(field_table[f].flags & FIELD_FLAGS_STRING) {
        datatype=VALUE_TYPE_STRING;
      } else if(field_table[f].flags & FIELD_FLAGS_FLOAT) {
        datatype=VALUE_TYPE_FLOAT;
      } else if(field_table[f].flags & FIELD_FLAGS_SQL_DATE) {
        new_object=flickcurl_sqltimestamp_to_isotime(object);
        object=new_object;
        datatype=VALUE_TYPE_DATETIME;
      }

      if(field == PHOTO_FIELD_license) {
        flickcurl_license* license;
        license=flickcurl_photos_licenses_getInfo_by_id(fc, 
                                                        photo->fields[field].integer);
        if(!license)
          continue;

        if(license->url) {
          datatype=VALUE_TYPE_URI;
          object=license->url;
        } else {
          datatype=VALUE_TYPE_STRING;
          object=license->name;
        }
      }
      
      switch(datatype) {
        case VALUE_TYPE_BOOLEAN:
          datatype_uri= XSD_NS "boolean";
          break;

        case VALUE_TYPE_DATETIME:
          datatype_uri= XSD_NS "dateTime";
          break;
          
        case VALUE_TYPE_FLOAT:
          datatype_uri= XSD_NS "double";
          break;
          
        case VALUE_TYPE_INTEGER:
          datatype_uri= XSD_NS "integer";
          break;
          
        case VALUE_TYPE_STRING:
          break;
          
        case VALUE_TYPE_URI:
          type= FLICKCURL_TERM_TYPE_RESOURCE;
          break;
          
        case VALUE_TYPE_NONE:
        case VALUE_TYPE_PHOTO_ID:
        case VALUE_TYPE_PHOTO_URI:
        case VALUE_TYPE_UNIXTIME:
        case VALUE_TYPE_PERSON_ID:
        case VALUE_TYPE_MEDIA_TYPE:
        default:
          break;
      }

      if(field_table[f].flags & FIELD_FLAGS_PERSON)
        fsf->emit_triple(fcs->data,
                         "person", FLICKCURL_TERM_TYPE_BLANK,
                         field_table[f].nspace_uri, field_table[f].name,
                         object, type,
                         datatype_uri);
      else
        fsf->emit_triple(fcs->data,
                         photo->uri, FLICKCURL_TERM_TYPE_RESOURCE,
                         field_table[f].nspace_uri, field_table[f].name,
                         object, type,
                         datatype_uri);

      if(new_object)
        free(new_object);
      
      break;
    }
  }
  

  /* generate triples from tags */
  for(i=0; i < photo->tags_count; i++) {
    flickcurl_tag* tag=photo->tags[i];
    char* prefix;
    char *p;
    char *f;
    char *v;
    size_t value_len;
    
    prefix=&tag->raw[0];

    if(!strncmp(prefix, "xmlns:", 6))
      continue;
    
    for(p=prefix; *p && *p != ':'; p++)
      ;
    if(!*p) /* No ':' found */
      continue;
    
    /* ":" seen */
    *p='\0';

    f=p+1;
    
    for(v=f; *v && *v != '='; v++)
      ;
    if(!*v) /* "prefix:name" seen with no value */
      continue;
    /* zap = */
    *v++='\0';

    value_len=strlen(v);
    if(*v == '"') {
      v++;
      if(v[value_len-1]=='"')
        v[--value_len]='\0';
    }
        
    ns=nspace_get_by_prefix(nspaces, prefix);
    
#if FLICKCURL_DEBUG > 1
      fprintf(fh,
              "%s: tag with prefix '%s' field '%s' value '%s' namespace uri %s\n",
              label, prefix, f, v, 
              ns ? ns->uri : "(No namespace)");
#endif
    if(!ns)
      continue;
    
    fsf->emit_triple(fcs->data,
                     photo->uri, FLICKCURL_TERM_TYPE_RESOURCE,
                     ns->uri, f,
                     v, FLICKCURL_TERM_TYPE_LITERAL, 
                     NULL);
    
  }


  /* generate triples from places */
  if(photo->place) {
    char place_bnode[7]={'p', 'l', 'a', 'c', 'e', 'X', '\0'};
    flickcurl_place* place=photo->place;
    
    for(i=(int)0; i <= (int)FLICKCURL_PLACE_LAST; i++) {
      char* name=place->names[i];
      char* id=place->ids[i];
      char* url=place->urls[i];
      char* woe_id=place->woe_ids[i];
      
      if(!name && !id && !url && !woe_id)
        continue;
      
      place_bnode[5]='0'+i;
      
      fsf->emit_triple(fcs->data,
                       photo->uri, FLICKCURL_TERM_TYPE_RESOURCE,
                       PLACES_NS, "place",
                       place_bnode, FLICKCURL_TERM_TYPE_BLANK,
                       NULL);
      fsf->emit_triple(fcs->data,
                       place_bnode, FLICKCURL_TERM_TYPE_BLANK,
                       RDF_NS, "type",
                       PLACES_NS "Place", FLICKCURL_TERM_TYPE_RESOURCE,
                       NULL);
      
      fsf->emit_triple(fcs->data,
                       place_bnode, FLICKCURL_TERM_TYPE_BLANK,
                       PLACES_NS, "type",
                       flickcurl_get_place_type_label(i),
                       FLICKCURL_TERM_TYPE_LITERAL,
                       NULL);

      if(name)
        fsf->emit_triple(fcs->data,
                         place_bnode, FLICKCURL_TERM_TYPE_BLANK,
                         PLACES_NS, "name",
                         name, FLICKCURL_TERM_TYPE_LITERAL,
                         NULL);
      if(id)
        fsf->emit_triple(fcs->data,
                         place_bnode, FLICKCURL_TERM_TYPE_BLANK,
                         PLACES_NS, "id",
                         id, FLICKCURL_TERM_TYPE_LITERAL,
                         NULL);
      if(woe_id)
        fsf->emit_triple(fcs->data,
                         place_bnode, FLICKCURL_TERM_TYPE_BLANK,
                         PLACES_NS, "placeid",
                         woe_id, FLICKCURL_TERM_TYPE_LITERAL,
                         NULL);
      if(url)
        fsf->emit_triple(fcs->data,
                         place_bnode, FLICKCURL_TERM_TYPE_BLANK,
                         PLACES_NS, "url",
                         url, FLICKCURL_TERM_TYPE_RESOURCE,
                         NULL);
    }
  }


  /* generate triples from sizes */
  if(sizes) {
    for(i=0; sizes[i]; i++) {
      flickcurl_size* size=sizes[i];
      char buf[10];
      int is_photo;
      const char* sizePredicate;
      const char* sizeClass;

      is_photo=(!strcmp(size->media, "photo"));
      sizePredicate=is_photo ? "photo" : "video";
      sizeClass=is_photo ? FOAF_NS "Image" : FLICKR_NS "Video";

      fsf->emit_triple(fcs->data,
                       photo->uri, FLICKCURL_TERM_TYPE_RESOURCE,
                       FLICKR_NS, sizePredicate,
                       size->source, FLICKCURL_TERM_TYPE_RESOURCE,
                       NULL);
      fsf->emit_triple(fcs->data,
                       size->source, FLICKCURL_TERM_TYPE_RESOURCE,
                       RDF_NS, "type",
                       sizeClass, FLICKCURL_TERM_TYPE_RESOURCE,
                       NULL);
      
      if(size->label)
        fsf->emit_triple(fcs->data,
                         size->source, FLICKCURL_TERM_TYPE_RESOURCE,
                         RDFS_NS, "label",
                         size->label, FLICKCURL_TERM_TYPE_LITERAL,
                         NULL);
      sprintf(buf, "%d", size->width);
      fsf->emit_triple(fcs->data,
                       size->source, FLICKCURL_TERM_TYPE_RESOURCE,
                       FLICKR_NS, "width",
                       buf, FLICKCURL_TERM_TYPE_LITERAL,
                       XSD_NS "integer");
      sprintf(buf, "%d", size->height);
      fsf->emit_triple(fcs->data,
                       size->source, FLICKCURL_TERM_TYPE_RESOURCE,
                       FLICKR_NS, "height",
                       buf, FLICKCURL_TERM_TYPE_LITERAL,
                       XSD_NS "integer");

    }
    flickcurl_free_sizes(sizes);
  }


  if(nspaces)
    free_nspaces(nspaces);
  
  if(fsf->emit_finish)
    fsf->emit_finish(fcs->data);
  
  return 0;
}
