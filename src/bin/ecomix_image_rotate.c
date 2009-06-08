#include <string.h>
#include <Evas.h>
#include "im.h"
#include "ecomix_image_rotate.h"

static void _els_smart_icon_flip_horizontal(Evas_Object *obj);
static void _els_smart_icon_flip_vertical(Evas_Object *obj);
static void _els_smart_icon_rotate_180(Evas_Object *obj, Image_Entry *ie);

void
ecomix_image_orient_set(Evas_Object *obj, Image_Entry *ie, Ecomix_Image_Orient orient)
{
   unsigned int   *data, *data2, *to = NULL, *from = NULL;
   int size;
   int             x, y, w, hw, iw, ih;
   
   if(!ie || !ie->surface) return;

   switch (orient)
     {
      case ECOMIX_IMAGE_FLIP_HORIZONTAL:
         _els_smart_icon_flip_horizontal(obj);
         return;
      case ECOMIX_IMAGE_FLIP_VERTICAL:
         _els_smart_icon_flip_vertical(obj);
	 return;
      case ECOMIX_IMAGE_ROTATE_180_CW:
         _els_smart_icon_rotate_180(obj, ie);
         return;
      default:
	 break;
     }

   // evas_object_image_size_get(obj, &iw, &ih);
   iw = ie->w;
   ih = ie->h;
   size = ie->w * ie->h * sizeof(DATA32);
   data2 = malloc(size);
   memcpy(data2, ie->surface, size); 

   w = ih;
   ih = iw;
   iw = w;
   hw = w * ih;

   // iw = ie->w;
   // ih = ie->h;
   if(orient == ECOMIX_IMAGE_ROTATE_90_CW || orient == ECOMIX_IMAGE_ROTATE_90_CCW)
      evas_object_image_size_set(obj, iw, ih);
   else
      evas_object_image_size_set(obj, ie->w, ie->h);
   size = ie->w * ie->h * sizeof(DATA32);
   data = malloc(size);
   memcpy(data, ie->surface, size); 

   switch (orient)
     {
      case ECOMIX_IMAGE_FLIP_TRANSPOSE:
	 to = data;
	 hw = -hw + 1;
	 break;
      case ECOMIX_IMAGE_FLIP_TRANSVERSE:
	 to = data + hw - 1;
	 w = -w;
	 hw = hw - 1;
	 break;
      case ECOMIX_IMAGE_ROTATE_90_CW:
	 to = data + w - 1;
	 hw = -hw - 1;
	 break;
      case ECOMIX_IMAGE_ROTATE_90_CCW:
	 to = data + hw - w;
	 w = -w;
	 hw = hw + 1;
	 break;
      default:
	 break;
     }
   if(orient) {
      from = data2;
      for (x = iw; --x >= 0;)
      {
	 for (y = ih; --y >= 0;)
       	 {
	    // printf("%d, %d\n", x, y);
      	    *to = *from;
     	    from++;
    	    to += w;
   	 }
    	 to += hw;
      }
   }
   free(data2);
   evas_object_image_data_copy_set(obj, data);
   evas_object_image_data_update_add(obj, 0, 0, iw, ih);
   free(data);
}

/* local subsystem globals */
/*
static void
_smart_reconfigure(Smart_Data *sd)
{
   int iw, ih;
   Evas_Coord x, y, w, h;
   
   if (!sd->obj) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     {
	w = sd->w;
	h = sd->h;
	x = sd->x;
	y = sd->y;
	evas_object_move(sd->obj, x, y);
	evas_object_resize(sd->obj, w, h);
     }
   else
     {
	iw = 0;
	ih = 0;
	evas_object_image_size_get(sd->obj, &iw, &ih);
	
	iw = ((double)iw) * sd->scale;
	ih = ((double)ih) * sd->scale;
	
	if (iw < 1) iw = 1;
	if (ih < 1) ih = 1;
	
	if (sd->fill_inside)
	  {
	     w = sd->w;
	     h = ((double)ih * w) / (double)iw;
	     if (h > sd->h)
	       {
		  h = sd->h;
		  w = ((double)iw * h) / (double)ih;
	       }
	  }
	else
	  {
	     w = sd->w;
	     h = ((double)ih * w) / (double)iw;
	     if (h < sd->h)
	       {
		  h = sd->h;
		  w = ((double)iw * h) / (double)ih;
	       }	
	  }
	if (!sd->scale_up)
	  {
	     if ((w > iw) || (h > ih))
	       {
		  w = iw;
		  h = ih;
	       }
	  }
	if (!sd->scale_down)
	  {
	     if ((w < iw) || (h < ih))
	       {
		  w = iw;
		  h = ih;
	       }
	  }
	x = sd->x + ((sd->w - w) / 2);
	y = sd->y + ((sd->h - h) / 2);
	evas_object_move(sd->obj, x, y);
	evas_object_image_fill_set(sd->obj, 0, 0, w, h);
	evas_object_resize(sd->obj, w, h);
     }
}
*/

static void
_els_smart_icon_flip_horizontal(Evas_Object *obj)
{
   unsigned int   *data;
   unsigned int   *p1, *p2, tmp;
   int             x, y, iw, ih;
   
   evas_object_image_size_get(obj, &iw, &ih);
   data = evas_object_image_data_get(obj, 1);

   for (y = 0; y < ih; y++)
     {
        p1 = data + (y * iw);
        p2 = data + ((y + 1) * iw) - 1;
        for (x = 0; x < (iw >> 1); x++)
          {
             tmp = *p1;
             *p1 = *p2;
             *p2 = tmp;
             p1++;
             p2--;
          }
     }

   evas_object_image_data_set(obj, data);
   evas_object_image_data_update_add(obj, 0, 0, iw, ih);
}

static void
_els_smart_icon_flip_vertical(Evas_Object *obj)
{
   unsigned int   *data;
   unsigned int   *p1, *p2, tmp;
   int             x, y, iw, ih;
   
   evas_object_image_size_get(obj, &iw, &ih);
   data = evas_object_image_data_get(obj, 1);

   for (y = 0; y < (ih >> 1); y++)
     {
        p1 = data + (y * iw);
        p2 = data + ((ih - 1 - y) * iw);
        for (x = 0; x < iw; x++)
          {
             tmp = *p1;
             *p1 = *p2;
             *p2 = tmp;
             p1++;
             p2++;
          }
     }

   evas_object_image_data_set(obj, data);
   evas_object_image_data_update_add(obj, 0, 0, iw, ih);
}

static void
_els_smart_icon_rotate_180(Evas_Object *obj, Image_Entry *ie)
{
   unsigned int   *data;
   unsigned int   *p1, *p2, tmp;
   int             size, x, hw, iw, ih;
   
   // evas_object_image_size_get(obj, &iw, &ih);
   // data = evas_object_image_data_get(obj, 1);
   data = ie->surface;
   iw = ie->w;
   ih = ie->h;
   evas_object_image_size_set(obj, iw, ih);
   size = ie->w * ie->h * sizeof(DATA32);
   data = malloc(size);
   memcpy(data, ie->surface, size); 


   hw = iw * ih;
   x = (hw / 2);
   p1 = data;
   p2 = data + hw - 1;
   for (; --x > 0;)
     {
        tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
     }
   evas_object_image_data_copy_set(obj, data);
   evas_object_image_data_update_add(obj, 0, 0, iw, ih);
   free(data);
}

