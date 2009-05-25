#
# Makefile.am - Top level automakefile for Flickcurl
#
# Copyright (C) 2007-2009, David Beckett http://www.dajobe.org/
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

SUBDIRS = src utils docs examples

ACLOCAL_AMFLAGS = -I build

bin_SCRIPTS = flickcurl-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = flickcurl.pc

EXTRA_DIST= \
ChangeLog \
README.html NEWS.html coverage.html README NEWS AUTHORS \
LICENSE.html LICENSE-2.0.txt NOTICE \
autogen.sh \
flickcurl-config.in flickcurl.pc.in \
flickcurl.rdf.in \
$(man_MANS) \
flickcurl.spec \
flickcurl.conf.example

# Allows 'make distcheck' to work
DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

# Create some text files from HTML sources
LYNX=lynx
HTML_TO_TEXT=TERM=vt100 $(LYNX) -dump

SUFFIXES = .html .txt

.html.txt:
	$(HTML_TO_TEXT) $< > $@

README: README.html
	$(HTML_TO_TEXT) $< > $@

NEWS: NEWS.html
	$(HTML_TO_TEXT) $< > $@
