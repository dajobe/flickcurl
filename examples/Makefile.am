#
# Flickcurl examples Makefile
#
# Copyright (C) 2008-2009, David Beckett http://www.dajobe.org/
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

EXTRA_DIST= \
print-photo-info.c

example_programs = print-photo-info search-photos

EXTRA_PROGRAMS = $(example_programs)

CLEANFILES = $(EXTRA_PROGRAMS)

AM_CFLAGS=$(MEM)
AM_LDFLAGS=$(MEM_LIBS)
AM_CPPFLAGS=-I$(top_srcdir)/src

print_photo_info_SOURCES = print-photo-info.c
print_photo_info_LDADD=$(top_builddir)/src/libflickcurl.la
print_photo_info_DEPENDENCIES = $(top_builddir)/src/libflickcurl.la

search_photos_SOURCES = search-photos.c
if GETOPT
search_photos_SOURCES += $(srcdir)/../utils/getopt.c \
  $(srcdir)/../utils/flickcurl_getopt.h
search_photos_CPPFLAGS = -I$(srcdir)/../utils
endif
search_photos_LDADD= $(top_builddir)/src/libflickcurl.la

$(top_builddir)/src/libflickcurl.la:
	cd $(top_builddir)/src && $(MAKE) libflickcurl.la

examples: $(example_programs)
