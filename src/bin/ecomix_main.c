#define _GNU_SOURCE
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Ecore_X.h>
#include <Ecore_X_Cursor.h>
#include <Evas.h>

#include <magic.h>
#ifdef HAVE_RSVG
#include <librsvg/rsvg.h>
#endif

#include "im.h"
#include "ecomix_arch_data_load.h"
#include "ecomix_image_rotate.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct _Ecomix_App
{
   Evas *evas;
   Ecore_Evas *ee;
   Ecore_Idler *idler;
   Evas_Object *image, *container, *text, *bg; /* image = o, container = r */
   int bg_r, bg_g, bg_b;
   int width, height;
   int verbose;
   char *bgimg;

   magic_t magic;
   Image_Entry ie;          /* Image Entry for fmemopen */
   Ecomix_Arch_Info *info;  
   char *dir;               /*  Directory of archive */
   Eina_Bool dir_have_arch;

   int current;             /* index of image in info->list */
   char *keyname;
   char *fill_mode, *fill_mode_previous; 
   Ecomix_Image_Orient orient, orient_previous;
   Eina_Bool fullscreen;

} Ecomix_App;

static void ecomix_usage(const char *cmd);
static Ecomix_App *ecomix_app_init(int argc, char **argv);
static void ecomix_app_finish(Ecomix_App *ecomix);
static void destroy_main_window(Ecore_Evas *ee);

static Eina_Bool ecomix_image_load(Ecomix_App *ecomix);
static void display_image(Ecomix_App *ecomix, const Ecomix_Buffer_Load *load, const char *file, const char *key, void *buf, size_t size);
static void image_fit(Ecomix_App *ecomix, char *data);
static int change_archive(Ecomix_App *ecomix, int z, char *keyname);

static void ecomix_key_down_callback(void *data, Evas *e, Evas_Object *o, void *einfo);
static void ecomix_mouse_down_callback(void *data, Evas *e, Evas_Object *o, void *einfo);
static void ecomix_mouse_wheel_callback(void *data, Evas *e, Evas_Object *o, void *einfo);

static void image_fit(Ecomix_App *ecomix, char *data)
{
   int nw, nh;
   double ow, oh;
   double dw, dh;

   evas_object_geometry_get(ecomix->container, NULL, NULL, &nw, &nh);

   ecomix_image_orient_set(ecomix->image, &ecomix->ie, ecomix->orient);

   if(ecomix->orient == ECOMIX_IMAGE_ROTATE_90_CW 
	 || ecomix->orient == ECOMIX_IMAGE_ROTATE_90_CCW) {
      ow = (double) (ecomix->ie.h);
      oh = (double) (ecomix->ie.w);
   } else {
      ow = (double) (ecomix->ie.w);
      oh = (double) (ecomix->ie.h);
   }

   if(data == NULL || strcmp(data, "full") == 0) {
      dw = ow;
      dh = oh;
   } else if(strcmp(data, "hfill") == 0) {
      dw = nw;
      dh = nw / (ow / oh);
   } else if(strcmp(data, "vfill") == 0) {
      dw = nh * (ow / oh);
      dh = nh; 
   } else if(strcmp(data, "fill") == 0) {
      if(ow > oh) {
	 dw = nw;
	 dh = nw / (ow / oh);
      } else {
	 dw = nh * (ow / oh);
	 dh = nh; 
      }
   } else {
      dw = ow;
      dh = oh;
   }

   ecomix->ie.dw = dw;
   ecomix->ie.dh = dh;

   evas_object_image_fill_set(ecomix->image, 0, 0, dw , dh);
   if(dh < nh)
      evas_object_move(ecomix->image, (nw - dw) / 2, (nh - dh) / 2);
   else
      evas_object_move(ecomix->image, (nw - dw) / 2, 0);
   evas_object_resize(ecomix->image, dw, dh);

}

static int change_archive(Ecomix_App *ecomix, int z, char *keyname)
{
   Eina_List *ls;
   Ecomix_Arch_Info *new = NULL;
   const char *f, *n;
   char *str, *d, *p;
   int i = -1, j;

   if(!ecomix) return ecomix->current;

   if(ecomix->verbose)
      printf("CHANGE: current = %d, dir = %d, keyname = %s\n", ecomix->current, z, keyname);
   str = (char *) malloc(PATH_MAX);
   memset(str, 0, PATH_MAX);
   if(ecomix->info && ecomix->info->archname)
      p = ecore_file_realpath(ecomix->info->archname);
   else
      p = "";
   d = ecore_file_dir_get(p);
   if(! d || !strcmp(d, "")) d = ".";

   n = ecore_file_file_get(p);
   ls = ecore_file_ls(d);
   if(ecomix->info && eina_list_count(ecomix->info->list) == 1) {
      if(! strcmp(keyname, "Home") || !strcmp(keyname, "End")) {
	 i = 0;
	 z = 1;
	 if(! strcmp(keyname, "End")) ls = eina_list_reverse(ls);
      }
   }
   if(i == -1) {
      for(i = 0; i < eina_list_count(ls); i++) {
	 f = eina_list_nth(ls, i);
	 if(*n == '\0' || ! strcmp(f, n))
	    break;
      }
      if(ecomix->info && eina_list_count(ecomix->info->list) == 1) {
	 if(! strcmp(keyname, "Prior")) i -= 10;
	 if(! strcmp(keyname, "Next")) i += 10;
      }
   }
   if(i >= 0 && i < eina_list_count(ls)) {
      if(! strcmp(keyname, "Home") || ! strcmp(keyname, "End"))
	 j = i;
      else
	 j = (z > 0) ? i + 1 : i - 1;
      while(j >= 0 && j < eina_list_count(ls)) {
	 n = eina_list_nth(ls, j);
	 strncpy(str, d, PATH_MAX -1);
	 str[PATH_MAX -1] = '\0';
	 strncat(str, "/", PATH_MAX -1);
	 strncat(str, n, PATH_MAX -1);
	 f = str; // ecore_file_realpath(n);
	 if(ecomix->verbose)
	    printf("try : %s\n", str);
	 new = ecomix_arch_file_set(str, ecomix->magic);
	 if(new) {
	    ecomix_arch_list_get(new);
	    if(eina_list_count(new->list) > 0) {
	       break;
	    } else
	       new = ecomix_arch_file_del(new);
	 }
	 if(z > 0)
	    j++;
	 else
	    j--;
      }
   }

   if(d && strcmp(d, ".")) free(d);
   if(p && strcmp(p, "")) free(p);
   if(str) free(str);
   if(ls)
      EINA_LIST_FREE(ls, str) free(str);
   if(new) {
      ecomix_arch_file_del(ecomix->info);
      ecomix->info = new;
      if(ecomix->verbose)
	 printf("CHANGE ARCHIVE %s\n", ecomix->info->archname);
      ecomix->dir_have_arch = 1;
      return (z > 0) ? 0 : eina_list_count(ecomix->info->list) -1;
   }

   return 0;
}

static void ecomix_mouse_wheel_callback(void *data, Evas *e, Evas_Object *o, void *einfo)
{
   Evas_Event_Mouse_Wheel *mouse = einfo;
   Evas_Event_Key_Down key;
   Ecomix_App *ecomix = data;

   if(ecomix->idler) return;
   if(mouse) {
      // printf("MOUSE: dir: %d, z: %d\n", mouse->direction, mouse->z);
      memset(&key, 0, sizeof(Evas_Event_Key_Down));
      if(mouse->z < 0)
	 key.keyname = "Up";
      else if(mouse->z > 0)
	 key.keyname = "Down";

      if(key.keyname) {
	 ecomix_key_down_callback(data, e, o, &key);
	 // reload current or image wont display 
	 if(! strcmp(ecomix->fill_mode, "vfill") && ecomix->info && ecomix->info->type == ECOMIX_ARCH_TYPE_LIBARCH)
	    ecomix_image_load(ecomix);
      }
   }
}

static void ecomix_mouse_down_callback(void *data, Evas *e, Evas_Object *o, void *einfo)
{
   Evas_Event_Mouse_Down *mouse = einfo;
   Evas_Event_Key_Down key;
   Ecomix_App *ecomix = data;

   if(ecomix->idler) return;
   if(mouse) {
      memset(&key, 0, sizeof(Evas_Event_Key_Down));
      if(mouse->button == 3)
	 key.keyname = "Up";
      else if(mouse->button == 1)
	 key.keyname = "Down";

      if(key.keyname)
	 ecomix_key_down_callback(data, e, o, &key);
   }
}

static Eina_Bool ecomix_on_idler(void *data) 
{
   Ecomix_App *ecomix = data;
   Eina_Bool renew = ECORE_CALLBACK_CANCEL, res = 0;

   if(!ecomix) return renew;

   if (ecomix->fill_mode_previous != ecomix->fill_mode 
	 || ecomix->orient_previous != ecomix->orient) {
      image_fit(ecomix, ecomix->fill_mode);
      ecomix->fill_mode_previous = ecomix->fill_mode;
      ecomix->orient_previous = ecomix->orient;
   } else {
      if(ecomix->current < 0) {
	 ecomix->current = change_archive(ecomix, -1, ecomix->keyname);
      } else if((!ecomix->info) || (ecomix->info && ecomix->current >= eina_list_count(ecomix->info->list))) {
	 ecomix->current = change_archive(ecomix, 1, ecomix->keyname);
      }

      if(ecomix->info) {
	 evas_event_freeze(ecomix->evas);
	 while(res == 0 && eina_list_count(ecomix->info->list) > 0) {
	    res = ecomix_image_load(ecomix);
	    if(!res && ecomix->info) {
	       char *filename = eina_list_nth(ecomix->info->list, ecomix->current);
	       ecomix->info->list = eina_list_remove(ecomix->info->list, filename);
	       if(ecomix->verbose)
		  printf("REMOVE FROM LIST: %d, %s\n", ecomix->current, filename);
	       eina_stringshare_del(filename);
	       if(ecomix->current >=  eina_list_count(ecomix->info->list))
		  ecomix->current = eina_list_count(ecomix->info->list) -1;
	    } 
	 }
	 if(! res) {
	    evas_object_text_text_set(ecomix->text, "No Image");
	    evas_object_move(ecomix->text, 10, 10);
	 }
	 evas_event_thaw(ecomix->evas);
      }
   }

   if(! renew)
      ecomix->idler = NULL;

   return renew;
}

static void ecomix_key_down_callback(void *data, Evas *e, Evas_Object *o, void *einfo)
{
   Evas_Event_Key_Down *key = einfo;
   Ecomix_App *ecomix = data;
   Ecomix_Image_Orient orient;
   char *mode;
   int ocur;
   int nw, nh;

   ocur = ecomix->current;
   mode = ecomix->fill_mode;
   orient = ecomix->orient;

   if(ecomix->idler) return;
   if(key) {
      if(! strcmp(key->keyname, "Up")) 
      {
	 evas_object_geometry_get(ecomix->container, NULL, NULL, &nw, &nh);
	 if(nh < ecomix->ie.dh && ecomix->ie.y > 0) {
	    ecomix->ie.y -= 20;
	    evas_object_move(ecomix->image, (nw - ecomix->ie.dw)/2, - ecomix->ie.y);
	 } else {
	    ecomix->current--;
	    ecomix->ie.y = 0;
	 }
      } 
      else if(! strcmp(key->keyname, "Down") || !strcmp(key->keyname, "space")) 
      {
	 evas_object_geometry_get(ecomix->container, NULL, NULL, &nw, &nh);
	 if(nh < ecomix->ie.dh && (ecomix->ie.dh - ecomix->ie.y) > nh) {
	    ecomix->ie.y += 20;
	    evas_object_move(ecomix->image, (nw - ecomix->ie.dw)/2, - ecomix->ie.y);
	 } else {
	    ecomix->current++;
	    ecomix->ie.y = 0;
	 }
      }
      else if(! strcmp(key->keyname, "Home"))
      {
	 evas_object_geometry_get(ecomix->container, NULL, NULL, &nw, &nh);
	 if(nh < ecomix->ie.dh && ecomix->ie.y != 0) {
	    ecomix->ie.y = 0;
	    evas_object_move(ecomix->image, (nw - ecomix->ie.dw)/2, - ecomix->ie.y);
	 } else
	    if(ecomix->info && eina_list_count(ecomix->info->list) > 1)
	       ecomix->current = 0;
	    else
	       ecomix->current--;
      }
      else if(! strcmp(key->keyname, "End"))
      {
	 evas_object_geometry_get(ecomix->container, NULL, NULL, &nw, &nh);
	 if(nh < ecomix->ie.dh && (ecomix->ie.dh - ecomix->ie.y) > nh) {
	    ecomix->ie.y = ecomix->ie.dh - nh + 20;
	    evas_object_move(ecomix->image, (nw - ecomix->ie.dw)/2, - ecomix->ie.y);
	 } else {
	    if(ecomix->info && eina_list_count(ecomix->info->list) > 1 )
	       ecomix->current = eina_list_count(ecomix->info->list) - 1;
	    else
	       ecomix->current++;
	    ecomix->ie.y = 0;
	 }
      }
      else if(! strcmp(key->keyname, "Prior"))
      {
	 ecomix->current -= 10;
	 if(ecomix->info && eina_list_count(ecomix->info->list) > 1 && ecomix->current < 0) ecomix->current = 0;
      }
      else if(! strcmp(key->keyname, "Next"))
      {
	 ecomix->current += 10;
	 if(ecomix->info && eina_list_count(ecomix->info->list) > 1 && ecomix->current > eina_list_count(ecomix->info->list))
	    ecomix->current = eina_list_count(ecomix->info->list) - 1;
      }
      else if(! strcmp(key->keyname, "Left"))
      {
	 ecomix->current = -1;
	 ecomix->ie.y = 0;
      }
      else if(! strcmp(key->keyname, "Right"))
      {
	 if(ecomix->info)
	    ecomix->current = eina_list_count(ecomix->info->list);
	 else
	    ecomix->current++;
	 ecomix->ie.y = 0;
      }
      else if(! strcmp(key->keyname, "n"))
      {
	 ecomix->fill_mode = "full";
      }
      else if(! strcmp(key->keyname, "h"))
      {
	 ecomix->fill_mode = "hfill";
      }
      else if(! strcmp(key->keyname, "v"))
      {
	 ecomix->fill_mode = "vfill";
      }
      else if(! strcmp(key->keyname, "f")) 
      {
	 ecomix->fullscreen = (ecomix->fullscreen == 1) ? 0 : 1;
	 ecore_evas_fullscreen_set(ecomix->ee, ecomix->fullscreen);
      }
      else if(! strcmp(key->keyname, "s"))
      {
	 if(evas_object_visible_get(ecomix->text))
	    evas_object_hide(ecomix->text);
	 else
	    evas_object_show(ecomix->text);
      }
      else if(! strcmp(key->key, "r"))
      {
	 if(ecomix->orient == ECOMIX_IMAGE_ORIENT_NONE)
	    ecomix->orient = ECOMIX_IMAGE_ROTATE_90_CW;
	 else if(ecomix->orient == ECOMIX_IMAGE_ROTATE_90_CW)
	    ecomix->orient = ECOMIX_IMAGE_ROTATE_180_CW;
	 else if(ecomix->orient == ECOMIX_IMAGE_ROTATE_180_CW)
	    ecomix->orient = ECOMIX_IMAGE_ROTATE_90_CCW;
	 else if(ecomix->orient == ECOMIX_IMAGE_ROTATE_90_CCW)
	    ecomix->orient = ECOMIX_IMAGE_ORIENT_NONE;
      }
      else if(! strcmp(key->key, "R"))
      {
	 if(ecomix->orient == ECOMIX_IMAGE_ORIENT_NONE)
	    ecomix->orient = ECOMIX_IMAGE_ROTATE_90_CCW;
	 else if(ecomix->orient == ECOMIX_IMAGE_ROTATE_90_CCW)
	    ecomix->orient = ECOMIX_IMAGE_ROTATE_180_CW;
	 else if(ecomix->orient == ECOMIX_IMAGE_ROTATE_180_CW)
	    ecomix->orient = ECOMIX_IMAGE_ROTATE_90_CW;
	 else if(ecomix->orient == ECOMIX_IMAGE_ROTATE_90_CW)
	    ecomix->orient = ECOMIX_IMAGE_ORIENT_NONE;
      }
      else if(! strcmp(key->keyname, "q"))
      {
	 destroy_main_window(ecomix->ee);
	 return;
      }

      if(ocur != ecomix->current) {
	 ecomix->keyname = key->keyname;
	 ecomix->idler = ecore_idler_add(ecomix_on_idler, ecomix);
      } else if (mode != ecomix->fill_mode || orient != ecomix->orient) {
	 ecomix->orient_previous = orient;
	 ecomix->fill_mode_previous = mode;
	 ecomix->idler = ecore_idler_add(ecomix_on_idler, ecomix);
      }

      if(ecomix->verbose)
	 printf("key : %s, %s, %s, %s\n", key->keyname, key->key, key->string, key->compose);
   }
}

static void display_image(Ecomix_App *ecomix, const Ecomix_Buffer_Load *load, const char *file, const char *key, void *buf, size_t size)
{
   Image_Entry *ie;
   if(! ecomix) return;
   if(! file) return;
   if(! load) return;

   ie = &(ecomix->ie);

//   if(load->head(ie, file, key, buf, size))
      load->data(ie, file, key, buf, size);

   if(ie->surface && ie->w && ie->h) 
   {
      evas_object_image_alpha_set(ecomix->image, ie->flags.alpha);
      evas_object_image_size_set(ecomix->image, ie->w, ie->h);
      evas_object_image_data_copy_set(ecomix->image, ie->surface);
      if(evas_object_image_load_error_get(ecomix->image) != EVAS_LOAD_ERROR_NONE) {
	 printf("LOAD ERROR: %d, name : %s\n", evas_object_image_load_error_get(ecomix->image), file);
      }
      evas_object_image_smooth_scale_set(ecomix->image, 1);
      evas_object_image_data_update_add(ecomix->image, 0, 0, ie->w, ie->h);
      image_fit(ecomix, ecomix->fill_mode);
      evas_object_image_data_update_add(ecomix->image, 0, 0, ie->dw, ie->dh);
   }
}

static Eina_Bool ecomix_image_load(Ecomix_App *ecomix)
{
   Ecomix_File_Buffer *fbuf;
   Eina_Bool res = 0;
   char *filename;
   int tw, th, cw, ch;

   if(! ecomix || !ecomix->info) return 0;

   filename = eina_list_nth(ecomix->info->list, ecomix->current);
   if(filename) {
      fbuf = ecomix_arch_data_get(ecomix->info, filename);
      if(fbuf) {
	 const char *mime = NULL;
	 const Ecomix_Buffer_Load *load = NULL;
	 if(ecomix->info->type == ECOMIX_ARCH_TYPE_LIBEVAS)
	    mime = "libevas";
	 else
	    mime = magic_buffer(ecomix->magic, fbuf->buf, fbuf->size);
	 load = ecomix_buffer_type_check(mime);
	 if(load) {
	    char buf[PATH_MAX];
	    memset(buf, 0, PATH_MAX);
	    display_image(ecomix, load, ecomix->info->archname, filename, fbuf->buf, fbuf->size);
	    if(ecomix->verbose)
	       printf("LOAD: %s : %s\n", ecomix->info->archname, filename);
	    snprintf(buf, PATH_MAX -1, "%s [%d / %d]", filename, ecomix->current + 1,
		  eina_list_count(ecomix->info->list));
	    evas_object_text_text_set(ecomix->text, buf);
	    evas_object_geometry_get(ecomix->container, NULL, NULL, &cw, &ch);
	    evas_object_geometry_get(ecomix->text, NULL, NULL, &tw, &th);
	    evas_object_move(ecomix->text, cw - (tw + 10), 10);
	    res = 1;

	 } else if(ecomix->verbose)
	    printf("Not load: %s => %s\n", filename, mime);
	 ecomix_arch_data_del(fbuf);
      }
   }

   return res;
}


static void destroy_main_window(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
   return;
}

static void resize_window(Ecore_Evas *ee)
{
   Evas_Coord w, h;
   int tw, th;
   Ecomix_App *ecomix;

   ecomix = ecore_evas_data_get(ee, "ecomix_app");
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_geometry_get(ecomix->text, NULL, NULL, &tw, &th);
   evas_object_resize(ecomix->container, w, h);
   evas_object_move(ecomix->text, w - (tw + 10), 10);
   if(ecomix->bg) 
      evas_object_resize(ecomix->bg, w, h);
   image_fit(ecomix, ecomix->fill_mode);
}  

static Ecomix_App *ecomix_app_init(int argc, char **argv)
{
   Ecomix_App *ecomix = NULL;
   char *file = NULL;
   int i = 1, w, h;
   int r, g, b;

   ecomix = malloc(sizeof(Ecomix_App));
   if(! ecomix) return NULL;
   memset(ecomix, 0, sizeof(Ecomix_App));

   ecomix->width = 300;
   ecomix->height = 300;
   while(i < argc) {
      if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")
	    || !strcmp(argv[i], "-?") || !strcmp(argv[i], "--?")) {
	 ecomix_usage(argv[0]);
	 exit(0);
      } else if(!strcmp(argv[i], "-bg") && (i + 1) < argc) {
	 int n = sscanf(argv[i+1], "%d,%d,%d", &r, &g, &b);
	 if(n == 3) {
	    ecomix->bg_r = r;
	    ecomix->bg_g = g;
	    ecomix->bg_b = b;
	 } else {
              struct stat info;
              if(stat(argv[i+1], &info) == 0 && S_ISREG(info.st_mode))
                 ecomix->bgimg = argv[i+1];
	 }
	 i++;
      } else if(!strcmp(argv[i], "-g") && (i + 1) < argc) {
	 int n = sscanf(argv[i+1], "%dx%d", &r, &g);
	 if(n == 2) {
	    ecomix->width = r;
	    ecomix->height = g;
	 }
	 i++;
      } else if(! strcmp(argv[i], "-v")) {
	 ecomix->verbose = 1;
      } else if (! file) {
	 file = argv[i];
      }
      i++;
   }

   w = ecomix->width; 
   h = ecomix->width;
   ecomix->magic = magic_open(MAGIC_MIME_TYPE);
   magic_load(ecomix->magic, NULL);

   ecomix->fill_mode = "vfill";
   ecomix->fill_mode_previous = ecomix->fill_mode;
   ecomix->current = 0;

   ecomix->ee = ecore_evas_software_x11_new(NULL, 0, 10, 10, w, h);
   ecore_evas_title_set(ecomix->ee, "ecomix");
   ecore_evas_name_class_set(ecomix->ee, "ecomix", "ecomix");
   ecore_evas_callback_delete_request_set(ecomix->ee, destroy_main_window);
   ecore_evas_callback_resize_set(ecomix->ee, resize_window);
   ecore_evas_size_min_set(ecomix->ee, 210, 230);
   ecore_evas_show(ecomix->ee);
   ecore_evas_data_set(ecomix->ee, "ecomix_app", ecomix);

   ecomix->evas = ecore_evas_get(ecomix->ee);
   evas_font_hinting_set(ecomix->evas, EVAS_FONT_HINTING_AUTO);

   memset(&ecomix->ie, 0, sizeof(Image_Entry));
   ecomix->ie.evas = ecomix->evas;

   if(ecomix->bgimg) {
      int bw, bh;
      ecomix->bg = evas_object_image_add(ecomix->evas);
      evas_object_image_file_set(ecomix->bg, ecomix->bgimg, NULL);
      evas_object_image_size_get(ecomix->bg, &bw, &bh);
      evas_object_resize(ecomix->bg, w, h);
      evas_object_image_fill_set(ecomix->bg, 0, 0, bw, bh);
   }

   ecomix->container = evas_object_rectangle_add(ecomix->evas);
   evas_object_color_set(ecomix->container, ecomix->bg_r, ecomix->bg_g, ecomix->bg_b, 255);
   evas_object_resize(ecomix->container, w, h);

   ecomix->image = evas_object_image_add(ecomix->evas);
   evas_object_image_smooth_scale_set(ecomix->image, 1);

   ecomix->text = evas_object_text_add(ecomix->evas);
   evas_object_color_set(ecomix->text, 255, 255, 255, 255);
   evas_object_text_outline_color_set(ecomix->text, 0, 0, 0, 255);
   evas_object_text_font_set(ecomix->text, "Vera", 20);
   evas_object_move(ecomix->text, 10, 10);
   evas_object_text_style_set(ecomix->text, EVAS_TEXT_STYLE_OUTLINE);
   evas_object_text_text_set(ecomix->text, "No image");

   evas_object_layer_set(ecomix->container, 0);
   evas_object_layer_set(ecomix->bg, 1);
   evas_object_layer_set(ecomix->image, 1);
   evas_object_layer_set(ecomix->text, 1);

   evas_object_focus_set(ecomix->container, 1);

   evas_object_propagate_events_set(ecomix->image, 1);
   evas_object_pass_events_set(ecomix->bg, 1);
   evas_object_pass_events_set(ecomix->image, 1);
   evas_object_pass_events_set(ecomix->text, 1);
   evas_object_event_callback_add(ecomix->container, EVAS_CALLBACK_KEY_DOWN, ecomix_key_down_callback, ecomix);
   evas_object_event_callback_add(ecomix->container, EVAS_CALLBACK_MOUSE_WHEEL, ecomix_mouse_wheel_callback, ecomix);
   evas_object_event_callback_add(ecomix->container, EVAS_CALLBACK_MOUSE_DOWN, ecomix_mouse_down_callback, ecomix);

   evas_object_show(ecomix->container);
   evas_object_show(ecomix->bg);
   evas_object_show(ecomix->image);
   evas_object_show(ecomix->text);

   ecomix_arch_init();

   if(file) {
      ecomix->info = ecomix_arch_file_set(file, ecomix->magic);
      if(ecomix->info) {
	 ecomix_arch_list_get(ecomix->info);
	 if(eina_list_count(ecomix->info->list) > 0) {
	    ecomix->dir_have_arch = ecomix_image_load(ecomix);
	    ecomix->dir_have_arch = 1;
	 }
      } else { // TODO: Read directory
      }
   }

   return ecomix;
}

static void ecomix_app_finish(Ecomix_App *ecomix)
{
   if(! ecomix) return;

   if(ecomix->info) ecomix_arch_file_del(ecomix->info);
   magic_close(ecomix->magic);
   if(ecomix->ie.surface) free(ecomix->ie.surface);

   ecomix_arch_finish();
#ifdef HAVE_RSVG
   rsvg_term();
#endif
}

static void ecomix_usage(const char *cmd) 
{
   const char *prog;
   if(!cmd) return;

   prog = basename(cmd);
   printf("A minimalist image viewer for archive (tar, zip, cpio, rar, 7z) without extracting file to disk.\n\
\n\
Usage: %s [-h] [-bg (r,g,b|image)] [-g wxh] [-v] file|dir\n\
\n\
Options:\n\
-h or --help or -? or --?	Show this help\n\
-bg (r,g,b|image)               Set background color or image\n\
-g widthxheight                 Set window size\n\
-v                              verbose\n\
\n\
Basic keys:\n\
q         quit\n\
s         show/hide filename / position\n\
r or R    rotate image by 90° clockwise or anti-clockwise\n\
f         (un)fullscreen\n\
v         scale image to max vertical\n\
h         scale image to max horizontal\n\
n         original image size\n\
left      go to prev file in dir\n\
right     go to next file in dir\n\
up        prev image or prev file if first image or up image if top not seen\n\
down      next image or next file if last image or down image if bottom not seen\n\
Home      go to fisrt of archive or top of image or first file in dir\n\
End       go to end of archive or bottom of image or last file in dir\n\
PgUp      skip 10 image/file backwrad\n\
PgDown    skip 10 image/file forward\n\
\n\
Note:\n\
The command line tools 7z, unrar, lha need to be installed to view content of archive not supported by libarchive.\n\
\n\
\n", prog);

}

int main(int argc, char **argv)
{
   Ecomix_App *ecomix;
   int i = 1;

   while(i < argc) {
      if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")
	    || !strcmp(argv[i], "-?") || !strcmp(argv[i], "--?")) {
	 ecomix_usage(argv[0]);
	 exit(0);
      }
      i++;
   }

   evas_init();
   ecore_evas_init();

   ecomix = ecomix_app_init(argc, argv);

   ecore_main_loop_begin();

   ecomix_app_finish(ecomix);

   ecore_shutdown();

   return 0;
}
