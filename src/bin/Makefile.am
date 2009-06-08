MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
-I. \
-I$(top_srcdir)/ \
@ECORE_CFLAGS@

if HAVE_PNG
INCLUDES += @ecomix_image_loader_png_cflags@
endif

if HAVE_GIF
INCLUDES += @ecomix_image_loader_gif_cflags@
endif

if HAVE_TIFF
INCLUDES += @ecomix_image_loader_tiff_cflags@
endif

if HAVE_SVG
INCLUDES += @ecomix_image_loader_svg_cflags@
endif

if HAVE_JPEG
INCLUDES += @ecomix_image_loader_jpeg_cflags@
endif


bin_PROGRAMS = ecomix

ecomix_SOURCES = \
ecomix_arch_data_load.c \
ecomix_image_rotate.c \
ecomix_main.c \
evas_image_load_gif.c \
evas_image_load_png.c \
evas_image_load_jpeg.c \
evas_image_load_tiff.c \
evas_image_load_pmaps.c \
evas_image_load_svg.c \
evas_image_load_xpm.c

ecomix_LDADD = @ECORE_LIBS@ @LIBS@

if HAVE_PNG
ecomix_LDADD += @ecomix_image_loader_png_libs@
endif

if HAVE_GIF
ecomix_LDADD += @ecomix_image_loader_gif_libs@
endif

if HAVE_SVG
ecomix_LDADD += @ecomix_image_loader_svg_libs@
endif

if HAVE_TIFF
ecomix_LDADD += @ecomix_image_loader_tiff_libs@
endif

if HAVE_JPEG
ecomix_LDADD += @ecomix_image_loader_jpeg_libs@
endif

