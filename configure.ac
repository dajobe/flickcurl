#                                               -*- Autoconf -*-
# Copyright (C) 2007-2009 David Beckett http://www.dajobe.org/

AC_PREREQ(2.59)
AC_INIT(Flickcurl, 1.11, http://bugs.librdf.org/, flickcurl)
AC_CONFIG_SRCDIR([src/flickcurl.h])
AM_CONFIG_HEADER([src/config.h])
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_MACRO_DIR(build)
AM_INIT_AUTOMAKE([1.7 check-news std-options -Wall])

AC_REVISION($Revision: $)dnl

AM_MAINTAINER_MODE

if test "$USE_MAINTAINER_MODE" = yes; then 
  CFLAGS=`echo $CFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
  CXXFLAGS=`echo $CXXFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
  CPPFLAGS=`echo $CPPFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
fi

dnl need to change quotes to allow square brackets
changequote(<<, >>)dnl
VERSION="$PACKAGE_VERSION.0"
version_major=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\1/'`
version_minor=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\2/'`
version_release=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\3/'`
changequote([, ])dnl

version_decimal=`expr $version_major \* 10000 + $version_minor \* 100 + $version_release`
AC_DEFINE_UNQUOTED(FLICKCURL_VERSION_DECIMAL, $version_decimal, [Release version as a decimal])

# for flickcurl-config.in
FLICKCURL_VERSION_DECIMAL=$version_decimal
AC_SUBST(FLICKCURL_VERSION_DECIMAL)


AM_INIT_AUTOMAKE([1.7 check-news std-options -Wobsolete -Wportability -Wsyntax -Wunsupported])

# Checks for programs.
AM_PROG_CC_C_O

m4_undefine([AC_PROG_CXX])
m4_defun([AC_PROG_CXX],[])
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])
AC_PROG_LIBTOOL

# Checks for libraries.

libxml_min_version=2.6.8
raptor_min_version=1.4.0
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

AC_ARG_WITH(xml2-config, [  --with-xml2-config=PATH   Location of libxml xml2-config []], xml2_config="$withval", xml2_config="")

if test "X$xml2_config" != "X" ; then
  AC_MSG_CHECKING(for $xml2_config)

  if test -f $xml2_config ; then
    XML_CONFIG=$xml2_config
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no - searching PATH)
  fi
fi
if test "X$XML_CONFIG" = "X"; then
  AC_CHECK_PROGS(XML_CONFIG, xml2-config)
fi

if test "X$XML_CONFIG" = "X"; then
  AC_MSG_ERROR(xml-config not found - please install libxml development headers and ibraries)
fi

LIBXML_VERSION=`$XML_CONFIG --version`
libxml_version_dec=`echo $LIBXML_VERSION | awk -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
libxml_min_version_dec=`echo $libxml_min_version | awk -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
if test $libxml_version_dec -lt $libxml_min_version_dec; then
  AC_MSG_ERROR(libxml $LIBXML_VERSION is too old - $libxml_min_version or newer required.)
fi


LIBS="$LIBS `$XML_CONFIG --libs`"
CPPFLAGS="`$XML_CONFIG --cflags` $CPPFLAGS"

AC_ARG_WITH(curl-config, [  --with-curl-config=PATH   Location of libcurl curl-config []], curl_config="$withval", curl_config="")

if test "X$curl_config" != "X" ; then
  AC_MSG_CHECKING(for $curl_config)

  if test -f $curl_config ; then
    CURL_CONFIG=$curl_config
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no - searching PATH)
  fi
fi
if test "X$CURL_CONFIG" = "X"; then
  AC_CHECK_PROGS(CURL_CONFIG, curl-config)
fi

if test "X$CURL_CONFIG" = "X"; then
  AC_MSG_ERROR(curl-config not found - please install curl development headers and ibraries)
fi

LIBS="$LIBS `$CURL_CONFIG --libs`"
CPPFLAGS="`$CURL_CONFIG --cflags` $CPPFLAGS"

LIBCURL_VERSION=`$CURL_CONFIG --version | sed -e 's/^libcurl *//'`
LIBCURL_VERNUM=`$CURL_CONFIG --vernum`
if test $LIBCURL_VERNUM '<' $libcurl_min_vernum; then
  AC_MSG_ERROR(curl $LIBCURL_VERSION is too old - $libcurl_min_version or newer required.)
fi


AC_CHECK_PROGS(PKG_CONFIG, pkg-config)

PKG_CONFIG_REQUIRES=


RAPTOR_MIN_VERSION=1.4.16
AC_SUBST(RAPTOR_MIN_VERSION)

AC_ARG_WITH(raptor, [  --with-raptor    Use raptor to serialize (default=auto)], with_raptor="$withval", with_raptor="auto")

RAPTOR_VERSION=
if test "$with_raptor" != no; then
  if $PKG_CONFIG raptor --exists; then
    RAPTOR_VERSION=`$PKG_CONFIG raptor --modversion 2>/dev/null`

    if $PKG_CONFIG raptor --atleast-version=$RAPTOR_MIN_VERSION; then
      with_raptor=yes
    else
      with_raptor=no
      AC_MSG_WARN(Installed raptor $RAPTOR_VERSION is too old - need $RAPTOR_MIN_VERSION)
    fi
  fi
fi

AM_CONDITIONAL(RAPTOR, test $with_raptor = yes)
AC_MSG_CHECKING(whether to use raptor)

if test "$with_raptor" = yes; then
  AC_MSG_RESULT(yes)
  RAPTOR_LIBS="`$PKG_CONFIG raptor --libs`"
  RAPTOR_CFLAGS="`$PKG_CONFIG raptor --cflags`"

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

if test "X$PKG_CONFIG_REQUIRES" != X; then
  PKG_CONFIG_REQUIRES="Requires:$PKG_CONFIG_REQUIRES"
fi
AC_SUBST(PKG_CONFIG_REQUIRES)


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
  AC_COMPILE_IFELSE([ ], [flickcurl_cc_flag=yes], [flickcurl_cc_flag=no])
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

dnl Shave "make autotools output sane" - disabled by default
dnl http://git.lespiau.name/cgit/shave/  git clone git://git.lespiau.name/shave
SHAVE_INIT([build], [disable])

AC_CONFIG_FILES([Makefile
build/shave
build/shave-libtool
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
