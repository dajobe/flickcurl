#
# Makefile.am - makefile for utilities
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

bin_PROGRAMS = flickcurl flickrdf

EXTRA_PROGRAMS = codegen list-methods mangen

CLEANFILES=$(EXTRA_PROGRAMS)

AM_CPPFLAGS= -I$(top_srcdir)/src -DMTWIST_CONFIG -I$(top_srcdir)/libmtwist

flickcurl_SOURCES = flickcurl.c commands.c flickcurl_cmd.h cmdline.c
flickcurl_CPPFLAGS = $(AM_CPPFLAGS)
flickcurl_LDADD= $(top_builddir)/src/libflickcurl.la
if GETOPT
flickcurl_CPPFLAGS += -I$(top_srcdir)/getopt
flickcurl_LDADD += $(top_builddir)/getopt/libgetopt.la
endif
flickcurl_CFLAGS=

flickrdf_SOURCES = flickrdf.c flickcurl_cmd.h cmdline.c
flickrdf_CPPFLAGS = $(AM_CPPFLAGS)
flickrdf_LDADD = $(top_builddir)/src/libflickcurl.la
if GETOPT
flickrdf_CPPFLAGS += -I$(top_srcdir)/getopt
flickrdf_LDADD += $(top_builddir)/getopt/libgetopt.la
endif
flickrdf_CFLAGS=
if RAPTOR
flickrdf_CFLAGS += @RAPTOR_CFLAGS@
flickrdf_LDADD += @RAPTOR_LIBS@
endif
if !RAPTOR
flickrdf_SOURCES += raptor_fake.c raptor_fake.h
endif

codegen_SOURCES = codegen.c flickcurl_cmd.h cmdline.c
codegen_CPPFLAGS = $(AM_CPPFLAGS)
codegen_LDADD = $(top_builddir)/src/libflickcurl.la
if GETOPT
codegen_CPPFLAGS += -I$(top_srcdir)/getopt
codegen_LDADD += $(top_builddir)/getopt/libgetopt.la
endif

list_methods_SOURCES = list-methods.c flickcurl_cmd.h cmdline.c
list_methods_CPPFLAGS = $(AM_CPPFLAGS)
list_methods_LDADD= $(top_builddir)/src/libflickcurl.la
if GETOPT
list_methods_CPPFLAGS += -I$(top_srcdir)/getopt
list_methods_LDADD += $(top_builddir)/getopt/libgetopt.la
endif

mangen_SOURCES = mangen.c commands.c flickcurl_cmd.h cmdline.c
mangen_CPPFLAGS = $(AM_CPPFLAGS)
mangen_LDADD = $(top_builddir)/src/libflickcurl.la


$(top_builddir)/src/libflickcurl.la:
	cd $(top_builddir)/src && $(MAKE) libflickcurl.la

$(top_builddir)/libgetopt/libgetopt.la:
	cd $(top_builddir)/libgetopt && $(MAKE) libgetopt.la

all-programs: $(bin_PROGRAMS) $(EXTRA_PROGRAMS)
