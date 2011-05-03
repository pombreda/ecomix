#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "im.h"
#ifdef BUILD_GRAPHICSMAGICK

#ifdef HAVE_LIBGRAPHICSMAGICKWAND
#include <wand/wand_api.h>
#endif
#include <magick/api.h>

static int libgm_initialized = 0;
#ifdef HAVE_LIBGRAPHICSMAGICK
static ExceptionInfo exception;
static ImageInfo *image_info;
#endif

static void libgm_init() {
   if(libgm_initialized == 0) {
      InitializeMagick(NULL);
#ifdef HAVE_LIBGRAPHICSMAGICK
      GetExceptionInfo(&exception);
      image_info = CloneImageInfo((ImageInfo *) NULL);
#endif

      libgm_initialized = 1;
   }
}

static void libgm_reset() {
   DestroyExceptionInfo(&exception);
   GetExceptionInfo(&exception);
}

int ecomix_image_load_head_libevas(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size)
{
   int ret = 0;

   return ret;
}

int ecomix_image_load_data_libevas(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size)
{
   Evas_Object *o = NULL;
   int ret = 0;
   
   if(! ie) return ret;
   if(! ie->evas) return ret;
   if(! file) return ret;

   ie->w = 0;
   ie->h = 0;
   if(ie->surface) {
      free(ie->surface);
      ie->surface = NULL;
   }

   o = evas_object_image_add(ie->evas);
   if(o) {
      evas_object_image_file_set(o, file, key);
      if(evas_object_image_load_error_get(o) == EVAS_LOAD_ERROR_NONE) {
	 evas_object_image_size_get(o, &ie->w, &ie->h);

	 ie->surface = malloc(ie->w * ie->h * sizeof(DATA32));
	 if(ie->surface) {
	    memcpy(ie->surface, evas_object_image_data_get(o, EINA_FALSE),
		  ie->w * ie->h * sizeof(DATA32));
	    ret = 1;
	 }
      }

      evas_object_del(o);
   }
   return ret;
}

int ecomix_image_load_fmem_head_libgm(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size)
{
   int ret = 0;
#ifdef HAVE_LIBGRAPHICSMAGICK
   Image *image = NULL;
   if(! buf) return ret;

   libgm_init();
   // image = PingBlob(image_info, buf, size, &exception);
   image = BlobToImage(image_info, buf, size, &exception);
   if(image) {
      ie->w = image->columns;
      ie->h = image->rows;
      DestroyImage(image);
      ret = 1;
   }

#else
   MagickWand *wand = NULL;
   if(!buf) return ret;

   libgm_init();
   wand = NewMagickWand();
   if(! wand) return ret;

   ret = MagickReadImageBlob(wand, buf, size);
   if(ret) {
      ie->w = MagickGetImageWidth(wand); 
      ie->h = MagickGetImageHeight(wand);
      ret = 1;
   }

   DestroyMagickWand(wand);
#endif
   return ret;
}

int ecomix_image_load_fmem_data_libgm(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size)
{
   int ret = 0;
#ifdef HAVE_LIBGRAPHICSMAGICK
   Image *image;
   if(! ie) return ret;
   if(! buf) return ret;
   if(! size) return ret;

   libgm_init();
   image = BlobToImage(image_info, buf, size, &exception);
   if(image) {
      size_t siz;
      const PixelPacket *pix;
      ExceptionInfo ex;
      magick_uint64_t mem_limit = GetMagickResourceLimit(MemoryResource);

      GetExceptionInfo(&ex);

      ie->w = 0;
      ie->h = 0;
      if(ie->surface) {
           free(ie->surface);
           ie->surface = NULL;
      }

      siz = image->columns * image->rows * sizeof(DATA32);
      if(siz > (mem_limit / 10)) {
           Image *img;
           img = ScaleImage(image, image->columns / 10, image->rows / 10, &ex);
           if(img) {
                DestroyImage(image);
                image = img;
                siz = image->columns * image->rows * sizeof(DATA32);
           }

      }

      pix = AcquireImagePixels(image, 0, 0, image->columns, image->rows, &ex);
      if(pix) {
           ie->surface = malloc(siz);
           ie->flags.alpha = (image->matte) ? 1 : 0;
           if(ie->surface) {
                ie->w = image->columns;
                ie->h = image->rows;
                memcpy(ie->surface, pix, siz);
           } else {
	      printf("No surface from libgm for %s\n", file);
	   }
      } else {
	 printf("No pix from libgm for %s\n", file);
      }
      DestroyImage(image);
      ret = 1;
   } else {
      printf("No image from libgm for %s\n", file);
      libgm_reset();
   }

#else
   MagickWand *wand = NULL;
   Image *image;
   if(! buf) return ret;

   libgm_init();
   wand = NewMagickWand();
   if(! wand) return ret;

   ret = MagickReadImageBlob(wand, buf, size);
   if(ret) {
      size_t siz;
      ie->w = MagickGetImageWidth(wand);
      ie->h = MagickGetImageHeight(wand);

      if(ie->surface) free(ie->surface);
      siz = ie->w * ie->h * sizeof(DATA32);
      ie->surface = malloc(siz);
      if(ie->surface) {
	 MagickGetImagePixels(wand, 0, 0, ie->w, ie->h, "BGRA", CharPixel, ie->surface);
      }
   }

   DestroyMagickWand(wand);
#endif
   return ret;
}

#endif
