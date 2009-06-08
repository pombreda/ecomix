#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wand/wand_api.h>
#include <magick/api.h>
#include "im.h"

static int libgm_initialized = 0;

static void libgm_init() {
   if(libgm_initialized == 0) {
      InitializeMagick(NULL);

      libgm_initialized = 1;
   }
}

#define ALPHA_SPARSE_INV_FRACTION 3
static void
evas_common_image_set_alpha_sparse(Image_Entry *ie)
{
   DATA32  *s, *se;
   DATA32  nas = 0;

   if (!ie) return;
   if (!ie->surface) return ;
   if (!ie->flags.alpha) return;

   s = ie->surface;
   se = s + (ie->w * ie->h);
   while (s < se)
   {
      DATA32  p = *s & 0xff000000;

      if (!p || (p == 0xff000000))
	 nas++;
      s++;
   }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ie->w * ie->h))
      ie->flags.alpha_sparse = 1;
}

static void
_unpremul_data(DATA32 *data, unsigned int len)
{
   DATA32  *de = data + len;

   while (data < de)
     {
	DATA32   a = A_VAL(data);
	
	if (a && (a < 255))
	  {
	     R_VAL(data) = (R_VAL(data) * 255) / a;
	     G_VAL(data) = (G_VAL(data) * 255) / a;
	     B_VAL(data) = (B_VAL(data) * 255) / a;
	  }
	data++;
     }
}

int ecomix_image_load_fmem_head_libgm(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size)
{
   MagickWand *wand = NULL;
   int ret = 0;
   if(!buf) return;

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
   return ret;
}

int ecomix_image_load_fmem_data_libgm(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size)
{
   MagickWand *wand = NULL;
   int ret = 0;
   Image *image;
   if(!buf) return;

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
   return ret;
}
