bin_PROGRAMS = flickcurl

bin_SCRIPTS = flickcurl-config

lib_LTLIBRARIES = libflickcurl.la

include_HEADERS = flickcurl.h

flickcurl_SOURCES = main.c
if GETOPT
flickcurl_SOURCES += getopt.c flickcurl_getopt.h flickcurl_internal.h
endif
flickcurl_LDADD= libflickcurl.la

libflickcurl_la_SOURCES = flickcurl.c md5.c vsnprintf.c config.c api.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = flickcurl.pc

EXTRA_DIST= \
ChangeLog \
README NEWS AUTHORS \
LICENSE.html LICENSE-2.0.txt NOTICE \
autogen.sh \
flickcurl-config.in flickcurl.pc.in \
flickcurl.rdf.in


# Create some text files from HTML sources
LYNX=lynx
HTML_TO_TEXT=TERM=vt100 $(LYNX) -dump -nolist

SUFFIXES = .html .txt

.html.txt:
	$(HTML_TO_TEXT) $< > $@

