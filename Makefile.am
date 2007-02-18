#
# Makefile.am
#
# Copyright (C) 2007, David Beckett http://purl.org/net/dajobe/
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

bin_PROGRAMS = flickcurl triplr

bin_SCRIPTS = flickcurl-config

lib_LTLIBRARIES = libflickcurl.la

include_HEADERS = flickcurl.h

flickcurl_SOURCES = main.c
if GETOPT
flickcurl_SOURCES += getopt.c flickcurl_getopt.h
endif
flickcurl_LDADD= libflickcurl.la

libflickcurl_la_SOURCES = core.c md5.c vsnprintf.c config.c \
api.c \
auth-api.c \
flickcurl_internal.h

triplr_SOURCES = triplr.c
if GETOPT
triplr_SOURCES += getopt.c flickcurl_getopt.h
endif
triplr_CFLAGS=
triplr_LDADD= libflickcurl.la
if RAPTOR
triplr_CFLAGS += @RAPTOR_CFLAGS@
triplr_LDADD += @RAPTOR_LIBS@
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = flickcurl.pc

EXTRA_DIST= \
ChangeLog \
README.html coverage.html README NEWS AUTHORS \
LICENSE.html LICENSE-2.0.txt NOTICE \
autogen.sh \
flickcurl-config.in flickcurl.pc.in \
flickcurl.rdf.in \
example.c


# Create some text files from HTML sources
LYNX=lynx
HTML_TO_TEXT=TERM=vt100 $(LYNX) -dump

SUFFIXES = .html .txt

.html.txt:
	$(HTML_TO_TEXT) $< > $@

README: README.html
	$(HTML_TO_TEXT) $< > $@
