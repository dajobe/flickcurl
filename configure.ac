#                                               -*- Autoconf -*-
# Copyright (C) 2007-2011 David Beckett http://www.dajobe.org/

AC_PREREQ([2.68])
AC_INIT([Flickcurl], [1.26], [http://bugs.librdf.org/], [flickcurl])
AC_CONFIG_SRCDIR([src/flickcurl.h])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_MACRO_DIR(build)
AM_INIT_AUTOMAKE([1.11 check-news std-options -Wobsolete -Wportability -Wsyntax -Wunsupported -Wextra-portability])

AM_MAINTAINER_MODE

if test "$USE_MAINTAINER_MODE" = yes; then 
  CFLAGS=`echo $CFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
  CXXFLAGS=`echo $CXXFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
  CPPFLAGS=`echo $CPPFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
fi

dnl need to change quotes to allow square brackets
changequote(<<, >>)dnl
version="$PACKAGE_VERSION.0"
version_major=`echo $version | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\1/'`
version_minor=`echo $version | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\2/'`
version_release=`echo $version | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\3/'`
changequote([, ])dnl

version_decimal=`expr $version_major \* 10000 + $version_minor \* 100 + $version_release`
AC_DEFINE_UNQUOTED(FLICKCURL_VERSION_DECIMAL, $version_decimal, [Release version as a decimal])

# for flickcurl-config.in
FLICKCURL_VERSION_DECIMAL=$version_decimal
AC_SUBST(FLICKCURL_VERSION_DECIMAL)


# Libtool versioning
#
# CURRENT
#      The most recent interface number that this library implements.
# 
# REVISION
#      The implementation number of the CURRENT interface.
# 
# AGE
#      The difference between the newest and oldest interfaces that this
#      library implements.  In other words, the library implements all the
#      interface numbers in the range from number `CURRENT - AGE' to
#      `CURRENT'.
#
# Rules:
#  1. Start with version information of `0:0:0' for each libtool library.
#
#  2. Update the version information only immediately before a public
#     release of your software.  More frequent updates are unnecessary,
#     and only guarantee that the current interface number gets larger
#     faster.
#
#  3. If the library source code has changed at all since the last
#     update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#  4. If any interfaces have been added, removed, or changed since the
#     last update, increment CURRENT, and set REVISION to 0.
#
#  5. If any interfaces have been added since the last public release,
#     then increment AGE.
#
#  6. If any interfaces have been removed since the last public release,
#     then set AGE to 0.
#
# syntax: CURRENT[:REVISION[:AGE]]
LIBFLICKCURL_LIBTOOL_VERSION=0:0:0
AC_SUBST(LIBFLICKCURL_LIBTOOL_VERSION)


AC_CANONICAL_HOST
AM_SANITY_CHECK

# Checks for programs.
AM_PROG_AR
AM_PROG_CC_C_O
AC_MSG_CHECKING(whether $CC is clang)
CC_IS_CLANG=no
if $CC 2>&1 | grep clang >/dev/null 2>&1; then
  CC_IS_CLANG=yes
else
  :
fi
AC_MSG_RESULT($CC_IS_CLANG)

dnl Initialize libtool
LT_INIT

# Checks for libraries.

libxml_min_version=2.6.8
raptor_min_version=2.0.0
libcurl_min_version=7.10.0

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h getopt.h stdlib.h stdint.h time.h string.h unistd.h])
AC_HEADER_TIME

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_C_BIGENDIAN

AC_CHECK_TYPE([u32])
AC_CHECK_SIZEOF(unsigned int, 4)
AC_CHECK_SIZEOF(unsigned long, 4)

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([getopt getopt_long gettimeofday memset strdup usleep vsnprintf])
AC_SEARCH_LIBS(nanosleep, rt posix4, 
               AC_DEFINE(HAVE_NANOSLEEP, 1, [Define to 1 if you have the 'nanosleep' function.]),
               AC_MSG_WARN(nanosleep was not found))

AM_CONDITIONAL(GETOPT, test $ac_cv_func_getopt = no -a $ac_cv_func_getopt_long = no)

AC_MSG_CHECKING(whether need to declare optind)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif]], [[int x=optind;
]])],[AC_MSG_RESULT(no)],[AC_DEFINE(NEED_OPTIND_DECLARATION, 1, need 'extern int optind' declaration?)
            AC_MSG_RESULT(yes)])

if test $ac_cv_func_vsnprintf = yes; then
  AC_MSG_CHECKING(vsnprintf has C99 compatible return value)
  AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdarg.h>
int is_c99(char *s, ...) {
  char buffer[32];
  va_list args;
  int r;
  va_start(args, s);
  r = vsnprintf(buffer, 5, s, args);
  va_end(args);

  return (r == 7);
}

int main(int argc, char* argv) {
  return is_c99("1234567");
}]])],[AC_MSG_RESULT(no)],[AC_DEFINE(HAVE_C99_VSNPRINTF, 1, vsnprint has C99 compatible return value)
    AC_MSG_RESULT(yes)], [ ], [ ])
fi


AC_CHECK_PROGS(RECHO, echo)
RECHO_C=
RECHO_N=
case `$RECHO -n x` in
  -n*)
    case `$RECHO 'xy\c'` in
      *c*)
         ;;
      xy)
         RECHO_C='\c'
         ;;
    esac;;
  *)
    RECHO_N='-n'
    ;;
esac

AC_CHECK_PROGS(PKG_CONFIG, pkg-config)

PKG_CONFIG_REQUIRES=


dnl libxml - required
LIBXML_MIN_VERSION=$libxml_min_version
AC_SUBST(LIBXML_MIN_VERSION)

AC_ARG_WITH(xml2-config, [  --with-xml2-config=PATH   Location of libxml xml2-config []], xml2_config="$withval", xml2_config="")

if test "X$xml2_config" != "Xno" ; then
  if test "X$xml2_config" != "X" ; then
    AC_MSG_CHECKING(for $xml2_config)

    if test -f $xml2_config ; then
      XML_CONFIG=$xml2_config
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no - searching PATH)
    fi
  fi

  if test "X$XML_CONFIG" = "X" ; then
    AC_CHECK_PROGS(XML_CONFIG, xml2-config)
  fi
fi

libxml_source=no
if test "X$XML_CONFIG" != "X"; then
  oCPPFLAGS="$CPPFLAGS"
  oLIBS="$LIBS"

  CPPFLAGS="`$XML_CONFIG --cflags` $CPPFLAGS"
  LIBS="$LIBS `$XML_CONFIG --libs`"
  AC_CHECK_FUNC(xmlCreatePushParserCtxt, have_xmlCreatePushParserCtxt=yes, have_xmlCreatePushParserCtxt=no)

  AC_MSG_CHECKING(for libxml via xml2-config)
  if test $have_xmlCreatePushParserCtxt = yes; then
    libxml_source="xml2-config"
    LIBXML_VERSION=`$XML_CONFIG --version`
  fi
  CPPFLAGS="$oCPPFLAGS"
  LIBS="$oLIBS"
else
  AC_MSG_CHECKING(for libxml via pkg-config)
  XML_CONFIG="$PKG_CONFIG libxml-2.0"
  if $XML_CONFIG --exists; then
    LIBXML_VERSION=`$XML_CONFIG --modversion 2>/dev/null`
    libxml_source="pkg-config"
  fi
fi

if test "$libxml_source" != "no"; then
  AC_MSG_RESULT(yes - $LIBXML_VERSION via $libxml_source)
else
  AC_MSG_RESULT(no - not found)
  AC_MSG_ERROR(Please install version $libxml_min_version or newer)
fi

libxml_min_version_dec=`echo $libxml_min_version | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
libxml_version_dec=`echo $LIBXML_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
if test $libxml_version_dec -lt $libxml_min_version_dec; then
  AC_MSG_ERROR(libxml $LIBXML_VERSION is too old - $libxml_min_version or newer required.)
fi

LIBS="$LIBS `$XML_CONFIG --libs`"
CFLAGS="`$XML_CONFIG --cflags` $CFLAGS"

if test $libxml_source = "pkg-config"; then
  if test "X$PKG_CONFIG_REQUIRES" != X; then
    PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
  fi
  PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES libxml-2.0 >= $LIBXML_MIN_VERSION"
fi

dnl curl - required
LIBCURL_MIN_VERSION=$libcurl_min_version
AC_SUBST(LIBCURL_MIN_VERSION)

AC_ARG_WITH(curl-config, [  --with-curl-config=PATH   Location of libcurl curl-config []], curl_config="$withval", curl_config="")

if test "X$curl_config" != "Xno" ; then
  if test "X$curl_config" != "X" ; then
    AC_MSG_CHECKING(for $curl_config)

    if test -f $curl_config ; then
      CURL_CONFIG=$curl_config
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no - searching PATH)
    fi
  fi

  if test "X$CURL_CONFIG" = "X" ; then
    AC_CHECK_PROGS(CURL_CONFIG, curl-config)
  fi
fi

libcurl_source=no
if test "X$CURL_CONFIG" != "X"; then
  oCPPFLAGS="$CPPFLAGS"
  oLIBS="$LIBS"

  CPPFLAGS="`$CURL_CONFIG --cflags` $CPPFLAGS"
  LIBS="$LIBS `$CURL_CONFIG --libs`"
  AC_CHECK_HEADER(curl/curl.h)
  AC_CHECK_FUNC(curl_easy_init, have_curl_easy_init=yes, have_curl_easy_init=no)

  AC_MSG_CHECKING(for libcurl via curl-config)
  if test $have_curl_easy_init = yes; then
    libcurl_source="curl-config"
    LIBCURL_VERSION=`$CURL_CONFIG --version | sed -e 's/^libcurl *//'`
  fi
  CPPFLAGS="$oCPPFLAGS"
  LIBS="$oLIBS"
else
  AC_MSG_CHECKING(for libcurl via pkg-config)
  CURL_CONFIG="$PKG_CONFIG libcurl"
  if $CURL_CONFIG --exists; then
    LIBCURL_VERSION=`$CURL_CONFIG --modversion 2>/dev/null`
    libcurl_source="pkg-config"
  fi
fi

if test "$libcurl_source" != "no"; then
  AC_MSG_RESULT(yes - $LIBCURL_VERSION via $libcurl_source)
else
  AC_MSG_RESULT(no - not found)
  AC_MSG_ERROR(Please install version $libcurl_min_version or newer)
fi

libcurl_min_version_dec=`echo $libcurl_min_version | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
libcurl_version_dec=`echo $LIBCURL_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
if test $libcurl_version_dec -lt $libcurl_min_version_dec; then
  AC_MSG_ERROR(libcurl $LIBCURL_VERSION is too old - $libcurl_min_version or newer required.)
fi

LIBS="$LIBS `$CURL_CONFIG --libs`"
CFLAGS="`$CURL_CONFIG --cflags` $CFLAGS"

if test $libcurl_source = "pkg-config"; then
  if test "X$PKG_CONFIG_REQUIRES" != X; then
    PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
  fi
  PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES libcurl >= $LIBCURL_MIN_VERSION"
fi


dnl raptor - optional
AC_ARG_WITH(raptor, [  --with-raptor=yes|no|2    Use Raptor to serialize to triples (default=auto)], with_raptor="$withval", with_raptor="auto")

# resolve autodetecting raptor
if test "$with_raptor" = auto; then
  if $PKG_CONFIG raptor2 --exists; then
    with_raptor=yes
  else
    with_raptor=no
  fi
fi

# Check raptor version and use it if new enough
RAPTOR_VERSION=
if test "$with_raptor" = yes -o "$with_raptor" = 2; then
  RAPTOR_VERSION=`$PKG_CONFIG raptor2 --modversion 2>/dev/null`

  if $PKG_CONFIG raptor2 --atleast-version=$raptor_min_version; then
    with_raptor=yes
  else
    with_raptor=no
    AC_MSG_WARN(Installed Raptor $RAPTOR_VERSION is too old - need $raptor_min_version)
  fi
else
  with_raptor="no"
fi
RAPTOR_MIN_VERSION=$raptor_min_version
AC_SUBST(RAPTOR_MIN_VERSION)


AM_CONDITIONAL(RAPTOR, test $with_raptor != no)
AC_MSG_CHECKING(whether to use Raptor)
if test "$with_raptor" = yes; then
  AC_MSG_RESULT(yes - $RAPTOR_VERSION)
  RAPTOR_LIBS="`$PKG_CONFIG raptor2 --libs`"
  RAPTOR_CFLAGS="`$PKG_CONFIG raptor2 --cflags`"

  LIBS="$LIBS $RAPTOR_LIBS"
  CPPFLAGS="$CPPFLAGS $RAPTOR_CFLAGS"

  if test "X$PKG_CONFIG_REQUIRES" != X; then
    PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
  fi
  PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES raptor2 >= $raptor_min_version"
  AC_SUBST(RAPTOR_LIBS)
  AC_SUBST(RAPTOR_CFLAGS)
  AC_DEFINE(HAVE_RAPTOR, 1, [have Raptor RDF syntax library])
else
  AC_MSG_RESULT(no)
fi

AC_ARG_ENABLE(offline, [  --enable-offline        Turn on offline use of captured XML web service responses (default disabled).  ], offline="yes", offline="no")	
AC_MSG_CHECKING(using offline mode)
AC_MSG_RESULT($offline);
if test $offline = yes; then
  AC_DEFINE(OFFLINE, 1, [Offline mode])
fi
AM_CONDITIONAL(OFFLINE, test $offline = yes)


AC_ARG_ENABLE(capture, [  --enable-capture        Turn on capturing web service responses (default disabled).  ], capture="yes", capture="no")	
AC_MSG_CHECKING(capturing web service responses)
AC_MSG_RESULT($capture);
if test $capture = yes; then
  AC_DEFINE(CAPTURE, 1, [Capture web service responses])
fi

if test $offline = yes -a $capture = yes; then
  AC_MSG_ERROR(Cannot use both offline mode and capturing web service responses)
fi


dnl compiler checks

AC_DEFUN([FLICKCURL_CC_TRY_FLAG], [
  AC_MSG_CHECKING([whether $CC supports $1])
  flickcurl_save_CFLAGS="$CFLAGS"
  if test $CC_IS_CLANG = yes; then
    # Make unknown warnings into errors for clang so the test below works
    CFLAGS="-Werror $CFLAGS"
  fi
  CFLAGS="$CFLAGS $1"
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([ ])], [flickcurl_cc_flag=yes], [flickcurl_cc_flag=no])
  CFLAGS="$flickcurl_save_CFLAGS"
  if test "X$flickcurl_cc_flag" = "Xyes"; then
    ifelse([$2], , :, [$2])
  else
    ifelse([$3], , :, [$3])
  fi
  AC_MSG_RESULT($flickcurl_cc_flag)
])

# Just too annoying:
#  -Wmissing-format-attribute
possible_warnings="-Wall -Wextra \
-Wshadow -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes \
-Wmissing-declarations -Wnested-externs -Wredundant-decls -Wswitch-enum \
-Wsign-compare -Werror-implicit-function-declaration -Wwrite-strings
-Wpacked -Wpointer-arith -Wcast-align -Winit-self \
-Wunsafe-loop-optimizations -Wdeclaration-after-statement \
-Wold-style-definition \
-Wno-missing-field-initializers -Wno-unused-parameter"

warning_cflags=
if test "$USE_MAINTAINER_MODE" = yes; then
  AC_MSG_CHECKING(for supported $CC warning flags)
  AC_MSG_RESULT($warning_cflags)
  for warning in $possible_warnings; do
    FLICKCURL_CC_TRY_FLAG([$warning], [warning_cflags="$warning_cflags $warning"])
  done
  AC_MSG_CHECKING($CC supports warning flags)
  AC_MSG_RESULT($warning_cflags)
fi


CPPFLAGS="-DFLICKCURL_INTERNAL=1 $CPPFLAGS"

STANDARD_CFLAGS=
MAINTAINER_CFLAGS="$warning_cflags"

STANDARD_CFLAGS="$STANDARD_CFLAGS $CFLAGS"
if test "$USE_MAINTAINER_MODE" = yes; then
  CPPFLAGS="-g -DFLICKCURL_DEBUG=1 $CPPFLAGS"
  CFLAGS="$MAINTAINER_CFLAGS $CFLAGS"
fi

AC_SUBST(STANDARD_CFLAGS)

AC_SUBST(PKG_CONFIG_REQUIRES)

AC_SUBST(ECHO)

dnl automake 1.11
AM_SILENT_RULES([no])

AC_CONFIG_FILES([Makefile
libmtwist/Makefile
getopt/Makefile
src/Makefile
utils/Makefile
examples/Makefile
docs/Makefile
docs/version.xml
flickcurl.spec
flickcurl.rdf
flickcurl.pc])

AC_CONFIG_FILES([flickcurl-config], [chmod +x flickcurl-config])

dnl Check for gtk-doc and docbook
GTK_DOC_CHECK([1.3])

AC_OUTPUT

AC_MSG_RESULT([
Flickcurl build summary:
  libxml  $LIBXML_VERSION via $libxml_source
  curl    $LIBCURL_VERSION via $libcurl_source
  raptor  $RAPTOR_VERSION
])
