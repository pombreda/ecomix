rm -f config.cache

AC_INIT([ecomix], [0.1], [saitmoh@gmail.com])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
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
   [_enable_ecomix_graphicsmagick="no"]
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

if test "x${_enable_ecomix_graphicsmagick}" = "xyes" || test "x${_enable_ecomix_graphicsmagickwand}" = "xyes" ; then
   want_ecomix_image_loader_gif="no"
   want_ecomix_image_loader_jpeg="no"
   want_ecomix_image_loader_pmaps="no"
   want_ecomix_image_loader_png="no"
   want_ecomix_image_loader_svg="no"
   want_ecomix_image_loader_tiff="no"
   want_ecomix_image_loader_xpm="no"
else 
   want_ecomix_image_loader_gif="yes"
   want_ecomix_image_loader_jpeg="yes"
   want_ecomix_image_loader_pmaps="yes"
   want_ecomix_image_loader_png="yes"
   want_ecomix_image_loader_svg="no"
   want_ecomix_image_loader_tiff="yes"
   want_ecomix_image_loader_xpm="yes"
fi

ECOMIX_CHECK_IMAGE_LOADER([Gif], [${want_ecomix_image_loader_gif}])

have_ecomix_image_saver_jpeg="no"
ECOMIX_CHECK_IMAGE_LOADER([Jpeg], [${want_ecomix_image_loader_jpeg}])
ECOMIX_CHECK_IMAGE_LOADER([PMAPS], [${want_ecomix_image_loader_pmaps}])

ECOMIX_CHECK_IMAGE_LOADER([PNG], [${want_ecomix_image_loader_png}])

ECOMIX_CHECK_IMAGE_LOADER([SVG], [${want_ecomix_image_loader_svg}])

ECOMIX_CHECK_IMAGE_LOADER([Tiff], [${want_ecomix_image_loader_tiff}])

ECOMIX_CHECK_IMAGE_LOADER([XPM], [${want_ecomix_image_loader_xpm}])

PKG_CHECK_MODULES([ECORE], [
	 eina-0
	 eet
	 evas
	 ecore
	 ecore-evas
	 ecore-file
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

AC_CHECK_HEADER(archive.h)
AC_CHECK_HEADER(archive_entry.h)
AC_CHECK_HEADER(magic.h)

AC_CHECK_LIB([archive], [archive_read_new])
AC_CHECK_LIB([magic], [magic_file, magic_buffer])

AC_OUTPUT([
Makefile
src/Makefile
src/bin/Makefile
])

