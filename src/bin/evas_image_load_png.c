/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// #include <Evas.h>

#include <stdio.h>
#include <png.h>
#include <jpeglib.h>
#include <setjmp.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef _WIN32_WCE
# define E_FOPEN(file, mode) evil_fopen_native((file), (mode))
# define E_FREAD(buffer, size, count, stream) evil_fread_native(buffer, size, count, stream)
# define E_FCLOSE(stream) evil_fclose_native(stream)
#else
# define E_FOPEN(file, mode) fopen((file), (mode))
# define E_FREAD(buffer, size, count, stream) fread(buffer, size, count, stream)
# define E_FCLOSE(stream) fclose(stream)
#endif

//#include "evas_common.h"
//#include "evas_private.h"

#define ALPHA_SPARSE_INV_FRACTION 3
#define PNG_BYTES_TO_CHECK 4
#include "im.h"

int ecomix_cache_image_surface_alloc(Image_Entry *ie, int w, int h)
{
   size_t        siz = 0;

   /*
   if (f & RGBA_IMAGE_ALPHA_ONLY)
      siz = w * h * sizeof(DATA8);
   else */
      siz = w * h * sizeof(DATA32);

   if (ie->surface) free(ie->surface);
   ie->surface= malloc(siz);
   if (ie->surface == NULL) return -1;

   return 0;
}

#ifdef HAVE_PNG 
void
_image_premul(Image_Entry * ie)
{
   DATA32 *s, *se;
   DATA32 nas = 0;

   if (!ie)
      return;
   if (!ie->surface)
      return;

   s = (DATA32 *) ie->surface;
   se = s + (ie->w * ie->h);
   while (s < se)
     {
	DATA32 a = 1 + (*s >> 24);

	*s = (*s & 0xff000000) + (((((*s) >> 8) & 0xff) * a) & 0xff00) +
	   (((((*s) & 0x00ff00ff) * a) >> 8) & 0x00ff00ff);
	s++;
	if ((a == 1) || (a == 256))
	   nas++;
     }

   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ie->w * ie->h))
      ie->flags.alpha_sparse = 1;
}

int
_ecomix_image_load_file_head_png(Image_Entry * ie, FILE * f, const char *key)
{
   png_uint_32 w32, h32;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   int bit_depth, color_type, interlace_type;
   unsigned char buf[PNG_BYTES_TO_CHECK];
   char hasa;

   hasa = 0;
   if (!f)
      return 0;

   /* if we havent read the header before, set the header data */
   if (E_FREAD(buf, PNG_BYTES_TO_CHECK, 1, f) != 1)
      goto close_file;

   if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
      goto close_file;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      goto close_file;

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	goto close_file;
     }
   if (setjmp(png_jmpbuf(png_ptr)))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	goto close_file;
     }
   png_init_io(png_ptr, f);
   png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   if ((w32 < 1) || (h32 < 1) || (w32 > 8192) || (h32 > 8192))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	goto close_file;
     }
   ie->w = (int)w32;
   ie->h = (int)h32;
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      hasa = 1;
   if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
      hasa = 1;
   if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      hasa = 1;
   if (hasa)
      ie->flags.alpha = 1;
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   return 1;

 close_file:
   return 0;

   key = 0;
}

int
_ecomix_image_load_file_data_png(Image_Entry * ie, FILE * f, const char *key)
{
   png_uint_32 w32, h32;
   int w, h;
   png_structp png_ptr = NULL;
   png_infop info_ptr = NULL;
   int bit_depth, color_type, interlace_type;
   unsigned char buf[PNG_BYTES_TO_CHECK];
   unsigned char **lines;
   char hasa;
   int i;

   hasa = 0;
   if (!f)
      return 0;

   /* if we havent read the header before, set the header data */
   E_FREAD(buf, 1, PNG_BYTES_TO_CHECK, f);
   if (!png_check_sig(buf, PNG_BYTES_TO_CHECK))
      goto close_file;
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      goto close_file;

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	goto close_file;
     }
   if (setjmp(png_jmpbuf(png_ptr)))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	goto close_file;
     }
   png_init_io(png_ptr, f);
   png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *) (&w32),
		(png_uint_32 *) (&h32), &bit_depth, &color_type,
		&interlace_type, NULL, NULL);
   // evas_cache_image_surface_alloc(ie, w32, h32);
   ecomix_cache_image_surface_alloc(ie, w32, h32);
   // surface = (unsigned char *) evas_cache_image_pixels(ie);
   // ie->surface = (unsigned char *)calloc(w32 * h32, sizeof(DATA32));
   if (!ie->surface)
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	goto close_file;
     }
   if ((w32 != ie->w) || (h32 != ie->h))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	goto close_file;
     }
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      hasa = 1;
   if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
      hasa = 1;
   if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      hasa = 1;
   if (hasa)
      ie->flags.alpha = 1;

   /* Prep for transformations...  ultimately we want ARGB */
   /* expand palette -> RGB if necessary */
   if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(png_ptr);
   /* expand gray (w/reduced bits) -> 8-bit RGB if necessary */
   if ((color_type == PNG_COLOR_TYPE_GRAY) ||
       (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
     {
	png_set_gray_to_rgb(png_ptr);
	if (bit_depth < 8)
	   png_set_expand_gray_1_2_4_to_8(png_ptr);
     }
   /* expand transparency entry -> alpha channel if present */
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(png_ptr);
   /* reduce 16bit color -> 8bit color if necessary */
   if (bit_depth > 8)
      png_set_strip_16(png_ptr);
   /* pack all pixels to byte boundaries */
   png_set_packing(png_ptr);

   w = ie->w;
   h = ie->h;
   /* we want ARGB */
#ifdef WORDS_BIGENDIAN
   png_set_swap_alpha(png_ptr);
   if (!hasa)
      png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);
#else
   png_set_bgr(png_ptr);
   if (!hasa)
      png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
#endif
   lines = (unsigned char **)alloca(h * sizeof(unsigned char *));

   for (i = 0; i < h; i++)
      lines[i] = ie->surface + (i * w * sizeof(DATA32));
   png_read_image(png_ptr, lines);
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   _image_premul(ie);

   return 1;

 close_file:
   return 0;

   key = 0;
}

int
ecomix_image_load_file_head_png(Image_Entry * ie, const char *file,
				const char *key)
{
   FILE *f;
   int ret;

   if ((!file))
      return 0;
   f = E_FOPEN(file, "rb");
   if (!f)
      return 0;

   ret = _ecomix_image_load_file_head_png(ie, f, key);

   E_FCLOSE(f);
   return ret;

   key = 0;
}

int
ecomix_image_load_file_data_png(Image_Entry * ie, const char *file,
				const char *key)
{
   FILE *f;
   int ret;

   if ((!file))
      return 0;
   f = E_FOPEN(file, "rb");
   if (!f)
      return 0;

   ret = _ecomix_image_load_file_data_png(ie, f, key);

   E_FCLOSE(f);
   return ret;

   key = 0;
}

int
ecomix_image_load_fmem_head_png(Image_Entry * ie, const char *file,
				    const char *key, void *buf, size_t size)
{
   FILE *f;
   int ret;

   if ((!file))
      return 0;
   if(!buf) return 0;
   if(!size) return 0;

   f = fmemopen(buf, size, "rb");
   if (!f)
      return 0;

   ret = _ecomix_image_load_file_head_png(ie, f, key);

   E_FCLOSE(f);
   return ret;

   key = 0;
}

int
ecomix_image_load_fmem_data_png(Image_Entry * ie, const char *file,
				    const char *key, void *buf, size_t size)
{
   FILE *f;
   int ret;

   if ((!file))
      return 0;
   if(!buf) return 0;
   if(!size) return 0;

   f = fmemopen(buf, size, "rb");
   if (!f)
      return 0;

   ret = _ecomix_image_load_file_data_png(ie, f, key);

   E_FCLOSE(f);
   return ret;

   key = 0;
}
#endif

