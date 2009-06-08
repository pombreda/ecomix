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

want_ecomix_image_loader_gif="yes"
want_ecomix_image_loader_jpeg="yes"
want_ecomix_image_loader_pmaps="yes"
want_ecomix_image_loader_png="yes"
want_ecomix_image_loader_svg="yes"
want_ecomix_image_loader_tiff="yes"
want_ecomix_image_loader_xpm="yes"

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

