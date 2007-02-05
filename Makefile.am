bin_PROGRAMS = flickcurl

bin_SCRIPTS = flickcurl-config

lib_LTLIBRARIES = libflickcurl.la

include_HEADERS = flickcurl.h

flickcurl_SOURCES = main.c
if GETOPT
flickcurl_SOURCES += getopt.c flickcurl_getopt.h
endif
flickcurl_LDADD= libflickcurl.la

libflickcurl_la_SOURCES = flickcurl.c md5.c vsnprintf.c config.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = flickcurl.pc

EXTRA_DIST= \
ChangeLog README NEWS AUTHORS \
autogen.sh \
flickcurl-config.in flickcurl.pc.in
