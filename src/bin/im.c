#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <magick/api.h>
#include "im.h"

static int libgm_initialized = 0;
static ExceptionInfo exception;
static ImageInfo *image_info;

static void libgm_init() {
   if(libgm_initialized == 0) {
      InitializeMagick(NULL);
      GetExceptionInfo(&exception);
      image_info = CloneImageInfo((ImageInfo *) NULL);

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
   int ret = 0;
   Image *image;
   if(!buf) return;

   libgm_init();
   // image = PingBlob(image_info, buf, size, &exception);
   image = BlobToImage(image_info, buf, size, &exception);
   if(image) {
      ie->w = image->columns;
      ie->h = image->rows;
      DestroyImage(image);
      ret = 1;
   }

   return ret;
}

int ecomix_image_load_fmem_data_libgm(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size)
{
   int ret = 0;
   Image *image;
   if(!buf) return;

   libgm_init();
   image = BlobToImage(image_info, buf, size, &exception);
   if(image) {
      size_t siz;
      PixelPacket *pix;
      ie->w = image->columns;
      ie->h = image->rows;

      if(ie->surface) free(ie->surface);
      siz = ie->w * ie->h * sizeof(DATA32);
      ie->surface = malloc(siz);
      ie->flags.alpha = (image->matte) ? 1 : 0;
      if(ie->surface) {
	 pix = GetImagePixels(image, 0, 0, image->columns, image->rows);
	 memcpy(ie->surface, pix, siz);
      }
      DestroyImage(image);
      ret = 1;
   }

   return ret;
}
