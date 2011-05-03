#ifndef _ECOMIX_IMAGE_ENTRY_H
#define _ECOMIX_IMAGE_ENTRY_H

#include <Evas.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#undef BUILD_GRAPHICSMAGICK
#ifdef HAVE_LIBGRAPHICSMAGICK
#define BUILD_GRAPHICSMAGICK
#endif
#ifdef HAVE_LIBGRAPHICSMAGICKWAND
#define BUILD_GRAPHICSMAGICK
#endif

typedef unsigned long long              DATA64;
typedef unsigned int                    DATA32;
typedef unsigned short                  DATA16;
typedef unsigned char                   DATA8;

#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) ((DATA8 *)(p))[3]
#define R_VAL(p) ((DATA8 *)(p))[2]
#define G_VAL(p) ((DATA8 *)(p))[1]
#define B_VAL(p) ((DATA8 *)(p))[0]
#define AR_VAL(p) ((DATA16 *)(p)[1])
#define GB_VAL(p) ((DATA16 *)(p)[0])
#else
/* ppc */
#define A_VAL(p) ((DATA8 *)(p))[0]
#define R_VAL(p) ((DATA8 *)(p))[1]
#define G_VAL(p) ((DATA8 *)(p))[2]
#define B_VAL(p) ((DATA8 *)(p))[3]
#define AR_VAL(p) ((DATA16 *)(p)[0])
#define GB_VAL(p) ((DATA16 *)(p)[1])
#endif

typedef struct _RGBA_Image_Loadopts   RGBA_Image_Loadopts;
struct _RGBA_Image_Loadopts
{
   int                  scale_down_by; // if > 1 then use this
   double               dpi; // if > 0.0 use this
   int                  w, h; // if > 0 use this
};

typedef enum _RGBA_Image_Flags
{
   RGBA_IMAGE_NOTHING       = (0),
   /*    RGBA_IMAGE_HAS_ALPHA     = (1 << 0), */
   RGBA_IMAGE_IS_DIRTY      = (1 << 1),
   RGBA_IMAGE_INDEXED       = (1 << 2),
   RGBA_IMAGE_ALPHA_ONLY    = (1 << 3),
   RGBA_IMAGE_ALPHA_TILES   = (1 << 4),
   /*    RGBA_IMAGE_ALPHA_SPARSE  = (1 << 5), */
   /*    RGBA_IMAGE_LOADED        = (1 << 6), */
   /*    RGBA_IMAGE_NEED_DATA     = (1 << 7) */
} RGBA_Image_Flags;

typedef struct _Image_Entry_Flags
{
  Eina_Bool loaded       : 1;
  Eina_Bool dirty        : 1;
  Eina_Bool activ        : 1;
  Eina_Bool need_data    : 1;
  Eina_Bool lru_nodata   : 1;
  Eina_Bool cached       : 1;
  Eina_Bool alpha        : 1;
  Eina_Bool alpha_sparse : 1;
#ifdef BUILD_ASYNC_PRELOAD
  Eina_Bool preload      : 1;
#endif
} Image_Entry_Flags;

typedef struct _Image_Entry
{
   Evas *evas;
   int w;
   int h;

   double dw;
   double dh;
   int y;

   RGBA_Image_Loadopts    load_opts;
   unsigned char          scale;
   
   void *surface;
   
   Image_Entry_Flags      flags;
} Image_Entry;

int ecomix_cache_image_surface_alloc(Image_Entry *ie, int w, int h);

int ecomix_image_load_head_libevas(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size);
int ecomix_image_load_data_libevas(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size);

int ecomix_image_load_fmem_head_libgm(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size);
int ecomix_image_load_fmem_data_libgm(Image_Entry *ie, const char *file, const char *key, void *buf, size_t size);

#endif
