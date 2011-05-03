rm -f config.cache

AC_INIT([ecomix], [0.1], [saitmoh@gmail.com])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(1.6 dist-bzip2 silent-rules)
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AC_C_BIGENDIAN

define([AC_LIBTOOL_LANG_CXX_CONFIG], [:])dnl
define([AC_LIBTOOL_LANG_F77_CONFIG], [:])dnl
AC_PROG_LIBTOOL

AC_ARG_ENABLE([graphicsmagick],
   [AC_HELP_STRING([--enable-graphicsmagick], [build ecomix with libgraphicsmagick])],
   [
    if test "x${enableval}" = "xyes" ; then
       _enable_ecomix_graphicsmagick="yes"
    else
       _enable_ecomix_graphicsmagick="no"
    fi
   ],
   [_enable_ecomix_graphicsmagick="yes"]
)

AC_MSG_CHECKING([build ecomix with libgraphicsmagick])
AC_MSG_RESULT([${_enable_ecomix_graphicsmagick}])

if test "x${_enable_ecomix_graphicsmagick}" = "xno" ; then
    AC_ARG_ENABLE([graphicsmagickwand],
       [AC_HELP_STRING([--enable-graphicsmagickwand], [build ecomix with libgraphicsmagickwand])],
       [
       if test "x${enableval}" = "xyes" ; then
           _enable_ecomix_graphicsmagickwand="yes"
       else
           _enable_ecomix_graphicsmagickwand="no"
       fi
       ],
       [_enable_ecomix_graphicsmagickwand="no"]
   )
fi
AC_MSG_CHECKING([build ecomix with libgraphicsmagickwand])
AC_MSG_RESULT([${_enable_ecomix_graphicsmagickwand}])

PKG_CHECK_MODULES([ECORE], [
	 eina
	 eet
	 evas
	 ecore
	 ecore-evas
	 ecore-file
])

PKG_CHECK_MODULES([LIBARCHIVE], [
	libarchive
])

if test "x${_enable_ecomix_graphicsmagick}" = "xyes" ; then
    PKG_CHECK_MODULES([GraphicsMagick], [
	GraphicsMagick
    ])
    have_libgraphicsmagick="yes"
    AC_DEFINE(HAVE_LIBGRAPHICSMAGICK, 1, [build with libgraphicsmagick])
fi

if test "x${_enable_ecomix_graphicsmagickwand}" = "xyes" ; then
    PKG_CHECK_MODULES([GraphicsMagickwand], [
	GraphicsMagickWand
    ])
    have_libgraphicsmagickwand="yes"
    AC_DEFINE(HAVE_LIBGRAPHICSMAGICKWAND, 1, [build with libgraphicsmagickwand])
fi

AM_CONDITIONAL(HAVE_LIBGRAPHICSMAGICK, [test "x${_enable_ecomix_graphicsmagick}" = "xyes"])
AM_CONDITIONAL(HAVE_LIBGRAPHICSMAGICKWAND, [test "x${_enable_ecomix_graphicsmagickwand}" = "xyes"])

AC_CHECK_HEADER(magic.h, magic=yes, magic=no)
if test "$magic" = "no" ; then
	AC_MSG_ERROR([Cannot find libmagic-dev])
fi
AC_CHECK_LIB([magic], [magic_file, magic_buffer], magic=yes, magic=no)
if test "$magic" = "no" ; then
	AC_MSG_ERROR([Cannot find libmagic-dev])
else
	MAGIC_LIBS=-lmagic
fi
AC_SUBST([MAGIC_LIBS])

AC_OUTPUT([
Makefile
src/Makefile
src/bin/Makefile
])

