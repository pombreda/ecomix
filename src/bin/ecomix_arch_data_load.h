#ifndef _ECOMIX_ARCH_DATA_LOAD_H
#define _ECOMIX_ARCH_DATA_LOAD_H

#include <Eina.h>
#include <magic.h>

#include "im.h"

typedef enum _Ecomix_Arch_Type {
   ECOMIX_ARCH_TYPE_UNKNOW = 0,
   ECOMIX_ARCH_TYPE_CHECK_COMMAND = 2,
   ECOMIX_ARCH_TYPE_LIBARCH = 4,
   ECOMIX_ARCH_TYPE_LIBEVAS = 8,
   ECOMIX_ARCH_TYPE_RAR = 16,
   ECOMIX_ARCH_TYPE_LHA = 32,
   ECOMIX_ARCH_TYPE_7Z = 64
} Ecomix_Arch_Type;

typedef struct _Ecomix_Arch_Info {
   const char *archname;
   Ecomix_Arch_Type type;
   Eina_List *list;
} Ecomix_Arch_Info;

typedef struct _Ecomix_File_Buffer {
   const char *filename;
   char *buf;
   size_t size;
} Ecomix_File_Buffer;

typedef struct _Ecomix_Buffer_Load {
   int (*head) (Image_Entry *, const char *, const char *, void *, size_t);
   int (*data) (Image_Entry *, const char *, const char *, void *, size_t);
} Ecomix_Buffer_Load;

void ecomix_arch_init();
void ecomix_arch_finish();

const char*ecomix_file_type_check(const char*mimetype);
const Ecomix_Buffer_Load *ecomix_buffer_type_check(const char*mimetype);

Ecomix_Arch_Info *ecomix_arch_file_set(char *archname, magic_t magic);
Ecomix_Arch_Info *ecomix_arch_file_del(Ecomix_Arch_Info *info);

Ecomix_Arch_Info *ecomix_arch_list_get(Ecomix_Arch_Info *info);

Ecomix_File_Buffer *ecomix_arch_data_get(Ecomix_Arch_Info *info, char *filename);
Ecomix_File_Buffer * ecomix_arch_data_del(Ecomix_File_Buffer *buf);

#endif
