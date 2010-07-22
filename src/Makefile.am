#
# Makefile.am
#
# Copyright (C) 2007-2010, David Beckett http://www.dajobe.org/
# 
# This file is licensed under the following three licenses as alternatives:
#   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
#   2. GNU General Public License (GPL) V2 or any newer version
#   3. Apache License, V2.0 or any newer version
# 
# You may not use this file except in compliance with at least one of
# the above three licenses.
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# complete terms and further detail along with the license texts for
# the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.

lib_LTLIBRARIES = libflickcurl.la

include_HEADERS = flickcurl.h

libflickcurl_la_SOURCES = \
activity.c \
args.c \
blog.c \
category.c \
collection.c \
common.c \
comments.c \
contacts.c \
context.c \
config.c \
exif.c \
gallery.c \
group.c \
institution.c \
md5.c \
location.c \
machinetags.c \
members.c \
method.c \
note.c \
person.c \
photo.c \
photoset.c \
place.c \
serializer.c \
shape.c \
size.c \
stat.c \
ticket.c \
user_upload_status.c \
tags.c \
video.c \
vsnprintf.c \
activity-api.c \
auth-api.c \
blogs-api.c \
commons-api.c \
collections-api.c \
contacts-api.c \
favorites-api.c \
interestingness-api.c \
galleries-api.c \
groups-api.c \
groups-members-api.c \
groups-pools-api.c \
machinetags-api.c \
people-api.c \
perms.c \
panda-api.c \
photos-api.c \
photos-comments-api.c \
photos-geo-api.c \
photos-licenses-api.c \
photos-notes-api.c \
photos-people-api.c \
photos-transform-api.c \
photos-upload-api.c \
photosets-api.c \
photosets-comments-api.c \
places-api.c \
prefs-api.c \
reflection-api.c \
stats-api.c \
tags-api.c \
test-api.c \
upload-api.c \
urls-api.c \
flickcurl_internal.h

AM_CFLAGS=
libflickcurl_la_LIBADD=

libflickcurl_la_LDFLAGS = -version-info @LIBFLICKCURL_LIBTOOL_VERSION@

if OFFLINE
if RAPTOR
AM_CFLAGS += @RAPTOR_CFLAGS@
libflickcurl_la_LIBADD += @RAPTOR_LIBS@
endif
endif
