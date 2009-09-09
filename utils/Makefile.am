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

EXTRA_PROGRAMS = codegen list-methods

CLEANFILES=$(EXTRA_PROGRAMS)

AM_CPPFLAGS= -I$(top_srcdir)/src 

flickcurl_SOURCES = flickcurl.c
if GETOPT
flickcurl_SOURCES += getopt.c flickcurl_getopt.h
endif
flickcurl_CFLAGS=
if MAINTAINER_MODE
flickcurl_CFLAGS += -DFLICKCURL_MAINTAINER
endif
flickcurl_LDADD= $(top_builddir)/src/libflickcurl.la

flickrdf_SOURCES = flickrdf.c
if GETOPT
flickrdf_SOURCES += getopt.c flickcurl_getopt.h
endif
flickrdf_CFLAGS=
flickrdf_LDADD= $(top_builddir)/src/libflickcurl.la
if RAPTOR
flickrdf_CFLAGS += @RAPTOR_CFLAGS@
flickrdf_LDADD += @RAPTOR_LIBS@
endif


codegen_SOURCES = codegen.c
if GETOPT
codegen_SOURCES += getopt.c flickcurl_getopt.h
endif
codegen_LDADD= $(top_builddir)/src/libflickcurl.la

list_methods_SOURCES = list-methods.c
if GETOPT
list_methods_SOURCES += getopt.c flickcurl_getopt.h
endif
list_methods_LDADD= $(top_builddir)/src/libflickcurl.la

$(top_builddir)/src/libflickcurl.la:
	cd $(top_builddir)/src && $(MAKE) libflickcurl.la
