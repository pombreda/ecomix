MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I. \
-I$(top_srcdir)/ \
@ECORE_CFLAGS@ \
@LIBARCHIVE_CFLAGS@

if HAVE_LIBGRAPHICSMAGICK
INCLUDES += @GraphicsMagick_CFLAGS@
endif

if HAVE_LIBGRAPHICSMAGICKWAND
INCLUDES += @GraphicsMagickwand_CFLAGS@
endif

bin_PROGRAMS = ecomix

ecomix_SOURCES = \
ecomix_arch_data_load.c \
ecomix_image_rotate.c \
ecomix_main.c \
im.c 

ecomix_LDADD = @ECORE_LIBS@ @MAGIC_LIBS@ @LIBARCHIVE_LIBS@ @LIBS@

if HAVE_LIBGRAPHICSMAGICK
ecomix_LDADD += @GraphicsMagick_LIBS@
endif

if HAVE_LIBGRAPHICSMAGICKWAND
ecomix_LDADD += @GraphicsMagickwand_LIBS@
endif

