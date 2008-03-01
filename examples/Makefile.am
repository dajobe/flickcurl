#
# Flickcurl Example Makefile.am
#
# Copyright (C) 2008, David Beckett http://www.dajobe.org/
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

EXTRA_PROGRAMS = print-photo-info

CLEANFILES = $(EXTRA_PROGRAMS)

AM_CFLAGS=$(MEM)
AM_LDFLAGS=$(MEM_LIBS)
AM_CPPFLAGS=-I$(top_srcdir)/src

print_photo_info_SOURCES = print-photo-info.c
print_photo_info_LDADD=$(top_builddir)/src/libflickcurl.la
print_photo_info_DEPENDENCIES = $(top_builddir)/src/libflickcurl.la

$(top_builddir)/src/libflickcurl.la:
	cd $(top_builddir)/src && $(MAKE) libflickcurl.la
