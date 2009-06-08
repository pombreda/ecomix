/* disable librsv because
 * - memory leak
 * - rendering need to be improve
 * - fail on some svg
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "im.h"
#ifdef HAVE_SVG

#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>


static int  rsvg_initialized = 0;
static void svg_loader_unpremul_data(DATA32 *data, unsigned int len);

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
svg_loader_unpremul_data(DATA32 *data, unsigned int len)
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

int
_ecomix_image_load_file_head_svg(Image_Entry *ie, RsvgHandle *rsvg)
{
   RsvgDimensionData   dim;
   int                 w, h;
   
   if (!rsvg)
     {
	return 0;
     }

   rsvg_handle_get_dimensions(rsvg, &dim);
   w = dim.width;
   h = dim.height;
   if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
     {
//	rsvg_handle_close(rsvg, NULL);
	g_object_unref(rsvg);
//	rsvg_handle_free(rsvg);
	return 0;
     }
   if (ie->load_opts.scale_down_by > 1)
     {
	w /= ie->load_opts.scale_down_by;
	h /= ie->load_opts.scale_down_by;
     }
   else if (ie->load_opts.dpi > 0.0)
     {
	w = (w * ie->load_opts.dpi) / 90.0;
	h = (h * ie->load_opts.dpi) / 90.0;
     }
   else if ((ie->load_opts.w > 0) &&
	    (ie->load_opts.h > 0))
     {
	int w2, h2;
	
	w2 = ie->load_opts.w;
	h2 = (ie->load_opts.w * h) / w;
	if (h2 > ie->load_opts.h)
	  {
	     h2 = ie->load_opts.h;
	     w2 = (ie->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ie->w = w;
   ie->h = h;
   ie->flags.alpha = 1;
//   rsvg_handle_close(rsvg, NULL);
   g_object_unref(rsvg);
//   rsvg_handle_free(rsvg);
   return 1;
}

/** FIXME: All evas loaders need to be tightened up **/
int
_ecomix_image_load_file_data_svg(Image_Entry *ie, RsvgHandle *rsvg)
{
   DATA32             *pixels;
   RsvgDimensionData   dim;
   int                 w, h;
   cairo_surface_t    *surface;
   cairo_t            *cr;

   if (!rsvg)
     {
	return 0;
     }

   rsvg_handle_get_dimensions(rsvg, &dim);
   w = dim.width;
   h = dim.height;
   if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
     {
//	rsvg_handle_close(rsvg, NULL);
	g_object_unref(rsvg);
//	rsvg_handle_free(rsvg);
	return 0;
     }
   if (ie->load_opts.scale_down_by > 1)
     {
	w /= ie->load_opts.scale_down_by;
	h /= ie->load_opts.scale_down_by;
     }
   else if (ie->load_opts.dpi > 0.0)
     {
	w = (w * ie->load_opts.dpi) / 90.0;
	h = (h * ie->load_opts.dpi) / 90.0;
     }
   else if ((ie->load_opts.w > 0) &&
	    (ie->load_opts.h > 0))
     {
	int w2, h2;
	
	w2 = ie->load_opts.w;
	h2 = (ie->load_opts.w * h) / w;
	if (h2 > ie->load_opts.h)
	  {
	     h2 = ie->load_opts.h;
	     w2 = (ie->load_opts.h * w) / h;
	  }
	w = w2;
	h = h2;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ie->flags.alpha = 1;
   ecomix_cache_image_surface_alloc(ie, w, h);
   pixels = ie->surface;
   if (!pixels)
     {
//	rsvg_handle_close(rsvg, NULL);
	g_object_unref(rsvg);
//	rsvg_handle_free(rsvg);
	return 0;
     }

   memset(pixels, 0, w * h * sizeof(DATA32));
   
   surface = cairo_image_surface_create_for_data((unsigned char *)pixels, CAIRO_FORMAT_ARGB32,
						 w, h, w * sizeof(DATA32));
   if (!surface)
     {
//	rsvg_handle_close(rsvg, NULL);
	g_object_unref(rsvg);
//	rsvg_handle_free(rsvg);
	return 0;
     }
   cr = cairo_create(surface);
   if (!cr)
     {
	cairo_surface_destroy(surface);
//	rsvg_handle_close(rsvg, NULL);
	g_object_unref(rsvg);
//	rsvg_handle_free(rsvg);
	return 0;
     }
   
   cairo_scale(cr, 
	       (double)ie->w / dim.em, 
	       (double)ie->h / dim.ex);
   rsvg_handle_render_cairo(rsvg, cr);
   cairo_surface_destroy(surface);
   /* need to check if this is required... */
   cairo_destroy(cr);
//   rsvg_handle_close(rsvg, NULL);
   g_object_unref(rsvg);
//   rsvg_handle_free(rsvg);
   evas_common_image_set_alpha_sparse(ie);
   return 1;
}

int ecomix_image_load_file_head_svg(Image_Entry *ie, const char *file, const char *key) {
   int ret = 0;
   char               cwd[PATH_MAX], pcwd[PATH_MAX], *p;
   RsvgHandle         *rsvg;
   char               *ext;

   if (!file) return 0;

   /* ignore all files not called .svg or .svg.gz - because rsvg has a leak
    * where closing the handle doesn't free mem */
   ext = strrchr(file, '.');
   if (!ext) return 0;
   if (!strcasecmp(ext, ".gz"))
     {
	if (p > file)
	  {
	     ext = p - 1;
	     while ((*p != '.') && (p > file))
	       {
		  p--;
	       }
	     if (p <= file) return 0;
	     if (strcasecmp(p, ".svg.gz")) return 0;
	  }
	else
	  return 0;
     }
   else if (strcasecmp(ext, ".svg")) return 0;

   getcwd(pcwd, sizeof(pcwd));
   strncpy(cwd, file, sizeof(cwd) - 1);
   cwd[sizeof(cwd) - 1] = 0;
   p = strrchr(cwd, '/');
   if (p) *p = 0;
   chdir(cwd);
   
   if(rsvg_initialized == 0) {
      rsvg_init();
      rsvg_initialized = 1;
   }

   rsvg = rsvg_handle_new_from_file(file, NULL);

   ret = _ecomix_image_load_file_head_svg(ie, rsvg);

   chdir(pcwd);
   return ret;
}

int ecomix_image_load_file_data_svg(Image_Entry *ie, const char *file, const char *key) {
   int ret = 0;
   char               cwd[PATH_MAX], pcwd[PATH_MAX], *p;
   RsvgHandle         *rsvg;
   char               *ext;

   if (!file) return 0;

   /* ignore all files not called .svg or .svg.gz - because rsvg has a leak
    * where closing the handle doesn't free mem */
   ext = strrchr(file, '.');
   if (!ext) return 0;
   if (!strcasecmp(ext, ".gz"))
     {
	if (p > file)
	  {
	     ext = p - 1;
	     while ((*p != '.') && (p > file))
	       {
		  p--;
	       }
	     if (p <= file) return 0;
	     if (strcasecmp(p, ".svg.gz")) return 0;
	  }
	else
	  return 0;
     }
   else if (strcasecmp(ext, ".svg")) return 0;

   getcwd(pcwd, sizeof(pcwd));
   strncpy(cwd, file, sizeof(cwd) - 1);
   cwd[sizeof(cwd) - 1] = 0;
   p = strrchr(cwd, '/');
   if (p) *p = 0;
   chdir(cwd);
   
   if(rsvg_initialized == 0) {
      rsvg_init();
      rsvg_initialized = 1;
   }

   rsvg = rsvg_handle_new_from_file(file, NULL);

   ret = _ecomix_image_load_file_data_svg(ie, rsvg);

   chdir(pcwd);
   return ret;
}

int ecomix_image_load_fmem_head_svg(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size) {
   int ret = 0;
   RsvgHandle         *rsvg;

   if(!buf) return 0;

   if(rsvg_initialized == 0) {
      rsvg_init();
      rsvg_initialized = 1;
   }

   rsvg = rsvg_handle_new_from_data(buf, size, NULL);
   ret = _ecomix_image_load_file_head_svg(ie, rsvg);

   return ret;
}

int ecomix_image_load_fmem_data_svg(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size) {
   int ret = 0;
   RsvgHandle         *rsvg;

   if(rsvg_initialized == 0) {
      rsvg_init();
      rsvg_initialized = 1;
   }

   rsvg = rsvg_handle_new_from_data(buf, size, NULL);
   ret = _ecomix_image_load_file_data_svg(ie, rsvg);

   return ret;
}

#endif
