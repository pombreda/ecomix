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

PKG_CHECK_MODULES([ECORE], [
	 eina-0
	 eet
	 evas
	 ecore
	 ecore-evas
	 ecore-file
])

PKG_CHECK_MODULES([GraphicsMagick], [
	GraphicsMagickWand
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

