#                                               -*- Autoconf -*-
# Copyright (C) 2007-2010 David Beckett http://www.dajobe.org/

AC_PREREQ(2.62)
AC_INIT(Flickcurl, 1.20, http://bugs.librdf.org/, flickcurl)
AC_CONFIG_SRCDIR([src/flickcurl.h])
AM_CONFIG_HEADER([src/config.h])
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_MACRO_DIR(build)
AM_INIT_AUTOMAKE([1.11 check-news std-options -Wall])

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


# Checks for programs.
AM_PROG_CC_C_O

m4_undefine([AC_PROG_CXX])
m4_defun([AC_PROG_CXX],[])
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])
AC_PROG_LIBTOOL

# Checks for libraries.

libxml_min_version=2.6.8
raptor_min_version=1.4.16
libcurl_min_version=7.10.0
libcurl_min_vernum=070A00

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h getopt.h stdlib.h string.h unistd.h])
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
AC_TRY_LINK([#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif], [int x=optind;],
            AC_MSG_RESULT(no),
            AC_DEFINE(NEED_OPTIND_DECLARATION, 1, [need 'extern int optind' declaration?])
            AC_MSG_RESULT(yes))

if test $ac_cv_func_vsnprintf = yes; then
  AC_MSG_CHECKING(vsnprintf has C99 compatible return value)
  AC_TRY_RUN([#include <stdarg.h>
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
}], AC_MSG_RESULT(no),
    AC_DEFINE(HAVE_C99_VSNPRINTF, 1, [vsnprint has C99 compatible return value])
    AC_MSG_RESULT(yes))
fi


AC_CHECK_PROGS(PKG_CONFIG, pkg-config)
if test "X$PKG_CONFIG" = X; then
  AC_MSG_ERROR(Please install pkg-config from http://pkg-config.freedesktop.org/)
fi

PKG_CONFIG_REQUIRES=


dnl libxml - required
LIBXML_MIN_VERSION=$libxml_min_version
AC_SUBST(LIBXML_MIN_VERSION)

AC_ARG_WITH(xml2-config, [  --with-xml2-config=PATH   Location of libxml xml2-config []], xml2_config="$withval", xml2_config="")

if test "X$xml2_config" != "X" ; then
  AC_MSG_WARN(Ignoring --with-xml2-config parameter: using pkg-config now)
fi

AC_MSG_CHECKING(for libxml)
if $PKG_CONFIG libxml-2.0 --exists; then
  :
else
  AC_MSG_RESULT(no - not found)
  AC_MSG_ERROR(Please install version $LIBXML_MIN_VERSION or newer)
fi

LIBXML_VERSION=`$PKG_CONFIG libxml-2.0 --modversion 2>/dev/null`
AC_MSG_RESULT(yes - $LIBXML_VERSION)
if $PKG_CONFIG libxml-2.0 --atleast-version=$LIBXML_MIN_VERSION; then
  :
else
  AC_MSG_ERROR(libxml $LIBXML_VERSION is too old - $libxml_min_version or newer required.)
fi

LIBS="$LIBS `$PKG_CONFIG libxml-2.0 --libs`"
CFLAGS="`$PKG_CONFIG libxml-2.0 --cflags` $CFLAGS"

if test "X$PKG_CONFIG_REQUIRES" != X; then
  PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
fi
PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES libxml-2.0 >= $LIBXML_MIN_VERSION"


dnl curl - required
LIBCURL_MIN_VERSION=$libcurl_min_version
AC_SUBST(LIBCURL_MIN_VERSION)

AC_ARG_WITH(curl-config, [  --with-curl-config=PATH   Location of libcurl curl-config []], curl_config="$withval", curl_config="")

if test "X$curl_config" != "X" ; then
  AC_MSG_WARN(Ignoring --with-curl-config parameter: using pkg-config now)
fi

AC_MSG_CHECKING(for libcurl)
if $PKG_CONFIG libcurl --exists; then
:
else
  AC_MSG_RESULT(no - not found)
  AC_MSG_ERROR(Please install version $LIBCURL_MIN_VERSION or newer)
fi

LIBCURL_VERSION=`$PKG_CONFIG libcurl --modversion 2>/dev/null`
AC_MSG_RESULT(yes - $LIBCURL_VERSION)
if $PKG_CONFIG libxml-2.0 --atleast-version=$LIBXML_MIN_VERSION; then
  :
else
  AC_MSG_ERROR(curl $LIBCURL_VERSION is too old - $libcurl_min_version or newer required.)
fi

LIBS="$LIBS `$PKG_CONFIG libcurl --libs`"
CFLAGS="`$PKG_CONFIG libcurl --cflags` $CFLAGS"

if test "X$PKG_CONFIG_REQUIRES" != X; then
  PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
fi
PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES libcurl >= $LIBCURL_MIN_VERSION"


dnl raptor - optional
RAPTOR_MIN_VERSION=$raptor_min_version
AC_SUBST(RAPTOR_MIN_VERSION)

AC_ARG_WITH(raptor, [  --with-raptor    Use raptor to serialize (default=auto)], with_raptor="$withval", with_raptor="auto")

# resolve autodetecting raptor
if test "$with_raptor" = auto; then
  if $PKG_CONFIG raptor --exists; then
      with_raptor=yes
    else
      with_raptor=no
  fi
fi

# disable raptor if the version is too old
RAPTOR_VERSION=
if test "$with_raptor" = yes; then
  RAPTOR_VERSION=`$PKG_CONFIG raptor --modversion 2>/dev/null`

  if $PKG_CONFIG raptor --atleast-version=$RAPTOR_MIN_VERSION; then
    with_raptor=yes
  else
    with_raptor=no
    AC_MSG_WARN(Installed raptor $RAPTOR_VERSION is too old - need $RAPTOR_MIN_VERSION)
  fi
fi

AM_CONDITIONAL(RAPTOR, test $with_raptor = yes)
AC_MSG_CHECKING(whether to use raptor)
if test "$with_raptor" = yes; then
  AC_MSG_RESULT(yes - $RAPTOR_VERSION)
  RAPTOR_LIBS="`$PKG_CONFIG raptor --libs`"
  RAPTOR_CFLAGS="`$PKG_CONFIG raptor --cflags`"

  LIBS="$LIBS $RAPTOR_LIBS"
  CPPFLAGS="$CPPFLAGS $RAPTOR_CFLAGS"

  if test "X$PKG_CONFIG_REQUIRES" != X; then
    PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
  fi
  PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES raptor >= $RAPTOR_MIN_VERSION"
  AC_SUBST(RAPTOR_LIBS)
  AC_SUBST(RAPTOR_CFLAGS)
  AC_DEFINE(HAVE_RAPTOR, 1, [have raptor RDF parser library])
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
src/Makefile
utils/Makefile
examples/Makefile
docs/Makefile
docs/version.xml
flickcurl.rdf
flickcurl.pc])

AC_CONFIG_FILES([flickcurl-config flickcurl.spec], [chmod +x flickcurl-config])

dnl Check for gtk-doc and docbook
GTK_DOC_CHECK([1.3])

AC_OUTPUT

AC_MSG_RESULT([
Flickcurl build summary:
  libxml  $LIBXML_VERSION
  curl    $LIBCURL_VERSION
  raptor  $RAPTOR_VERSION
])
