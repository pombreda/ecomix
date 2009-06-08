MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I. \
-I$(top_srcdir)/ \
@ECORE_CFLAGS@ \
@GraphicsMagick_CFLAGS@

bin_PROGRAMS = ecomix

ecomix_SOURCES = \
ecomix_arch_data_load.c \
ecomix_image_rotate.c \
ecomix_main.c \
im.c

ecomix_LDADD = @ECORE_LIBS@ @GraphicsMagick_LIBS@ @LIBS@

