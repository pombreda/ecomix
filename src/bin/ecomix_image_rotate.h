#ifndef _ECOMIX_IMAGE_ROTATE_H
#define _ECOMIX_IMAGE_ROTATE_H

#include <Evas.h>
#include "im.h"

typedef enum _Ecomix_Image_Orient
{
   ECOMIX_IMAGE_ORIENT_NONE,
   ECOMIX_IMAGE_ROTATE_90_CW,
   ECOMIX_IMAGE_ROTATE_180_CW,
   ECOMIX_IMAGE_ROTATE_90_CCW,
   ECOMIX_IMAGE_FLIP_HORIZONTAL,
   ECOMIX_IMAGE_FLIP_VERTICAL,
   ECOMIX_IMAGE_FLIP_TRANSPOSE,
   ECOMIX_IMAGE_FLIP_TRANSVERSE
} Ecomix_Image_Orient;

void ecomix_image_orient_set(Evas_Object *obj, Image_Entry *ie, Ecomix_Image_Orient orient);


#endif
