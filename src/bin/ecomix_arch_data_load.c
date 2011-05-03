#define _GNU_SOURCE
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Eina.h>
#include <Eet.h>
#include <Ecore_File.h>

#include <archive.h>
#include <archive_entry.h>

#include "ecomix_arch_data_load.h"

#include "im.h"

#define ECOMIX_FILE_BUF_SIZE_MAX 16777216
#define ECOMIX_IMAGE_DATA_READ_BUF 10496
#define ECOMIX_FILTRE_ARCH_LIST 0

static Eina_Hash *mimetype_file;
static Eina_Hash *mimetype_buffer;

static  int sort_cb(const void *d1, const void *d2);

Eina_List *ecomix_arch_list_rar_decode(char *buf, size_t size);
Eina_List *ecomix_arch_list_lha_decode(char *buf, size_t size);

Eina_List *ecomix_arch_list_libevas_get(const char *archname);
Eina_List *ecomix_arch_list_libarch_get(const char *archname);
Ecomix_Arch_Info *ecomix_arch_list_pipe_get(Ecomix_Arch_Info *info);

Ecomix_File_Buffer *ecomix_arch_data_libevas_get(Ecomix_Arch_Info *info, char *filename);
Ecomix_File_Buffer *ecomix_arch_data_libarch_get(Ecomix_Arch_Info *info, char *filename);
Ecomix_File_Buffer *ecomix_arch_data_pipe_get(Ecomix_Arch_Info *info, char *filename);


static int sort_cb(const void *d1, const void *d2) 
{
   if(!d1) return(1);
   if(!d2) return(-1);
   
   return(strcmp((const char*)d1, (const char*)d2));
}

Eina_List *ecomix_arch_list_rar_decode(char *buf, size_t size)
{
   Eina_List *l = NULL;
   char *p = buf, *s = buf, *s1 = NULL, *s2 = NULL;
   int sep = 0, cnt = 0;
   const char *f;

   if(buf && size > 0) {
      while(p < (char *) (buf + size)) {
	 if(*p == '\n') {
	    *p = '\0';
	    s = p + 1;
	    if(strncmp("----------", s, 10) == 0) {
	       sep = (sep == 0) ? 1 : 0;
	       continue;
	    }
	    if(sep) {
	       if(cnt == 0) {
		  if(strncmp(s, "Data header type:", 17) == 0) continue;
                  s1 = s;
                  cnt++;
	       } else if(cnt == 1) {
		  s2 = s;
		  cnt++;
	       } else if(cnt == 2) {
		  if((strstr(s, "Unix") && s2[52] != 'd')
			|| (strstr(s, "Win95/NT") && s2[54] != 'd')
			|| s2[58] == 'A') {
                       if(strncmp(s1, " CMT", 4) != 0) {
                            f = eina_stringshare_add(s1 + 1);
                            l = eina_list_append(l, f);
                       }
		  }
		  cnt = 0;
	       }
	    }
	 }
	 p++;
      }
   }

   if(0) {
        Eina_List *n = NULL;
        EINA_LIST_FOREACH(l, n, f) {
             printf("%s\n", f);
        }
   }
   return l;
}

Eina_List *ecomix_arch_list_lha_decode(char *buf, size_t size)
{
   Eina_List *l = NULL;
   char *p = buf, *line = buf;
   const char *f;
   int sep = 0;
   int pos = 0;
   
   if(buf && size > 0) {
      while(p < (char *) (buf + size)) {
	 if(*p == '\n') {
	    *p = 0;
	    if(! strncmp(line, "----------", 10)) sep = (sep == 0) ? 1 : 0;
	    if(pos == 0 && *line == '-') {
	       pos = strlen(line);
	       while(pos > 0) {
		  if(line[pos] == ' ') {
		     pos++;
		     break;
		  }
		  pos--;
	       }
	    } else if (sep) {
	       char *s = line + pos;
	       if((s && strcmp(s, "CMT") != 0) && (ECOMIX_FILTRE_ARCH_LIST 
                  || eina_str_has_extension(s, "png") 
                  || eina_str_has_extension(s, "jpg") 
                  || eina_str_has_extension(s, "jpeg") 
                  || eina_str_has_extension(s, "gif") 
                  || eina_str_has_extension(s, "tiff")))
	       {
		  // printf("file : %s\n", s);
		  f = eina_stringshare_add(s);
		  l = eina_list_append(l, f);
	       }
	    }
	    line = p + 1;
	 }
	 p++;
      }
   }

   if(0 && l) {
      Eina_List *l2;
      EINA_LIST_FOREACH(l, l2, f) { printf("%s\n", f); }
   }
   return l;
}

Eina_List *ecomix_arch_list_libevas_get(const char *archname) {
   Eina_List *l = NULL;
   Eet_File *ef = NULL;
   const char *name;

   ef = eet_open(archname, EET_FILE_MODE_READ);
   if(ef) {
      int cnt = 0;
      char **list = eet_list(ef, "*images/*", &cnt);
      if(list) {
	 int i;
	 for(i = 0; i < cnt; i++) {
	    name = eina_stringshare_add(list[i]);
	    l = eina_list_append(l, name);
	 }
	 free(list);
      }
      eet_close(ef);
   } else {
      name = eina_stringshare_add(archname);
      l = eina_list_append(l, name);
   }

   return l;
}

Eina_List *ecomix_arch_list_libarch_get(const char *archname) {
   struct archive *a;
   struct archive_entry *entry;
   Eina_List *l = NULL;
   int res = 0;

   a = archive_read_new();
   archive_read_support_compression_all(a);
   archive_read_support_format_all(a);
   if(archive_read_open_filename(a, archname, ECOMIX_IMAGE_DATA_READ_BUF) == ARCHIVE_OK) 
   {
      while (( res = archive_read_next_header(a, &entry)) == ARCHIVE_OK) 
      {
	 const char *str;
	 str = eina_stringshare_add(archive_entry_pathname(entry));
         if(str[strlen(str) - 1] != '/')
	    if(ECOMIX_FILTRE_ARCH_LIST || eina_str_has_extension(str, "png") || eina_str_has_extension(str, "jpg") || eina_str_has_extension(str, "jpeg"))
	       l = eina_list_append(l, str);
	    else
	       eina_stringshare_del(str);
         else
            eina_stringshare_del(str);
	 archive_read_data_skip(a);
      }
   } 

   archive_read_finish(a);

   if(l)
      l = eina_list_sort(l, eina_list_count(l), sort_cb);

   return l;
}

Ecomix_Arch_Info *ecomix_arch_list_pipe_get(Ecomix_Arch_Info *info)
{
   int pfd[2];
   pid_t cpid;

   if(! info || !info->archname) return info;
   
   if (pipe(pfd) == -1) { perror("pipe"); return info; }

   cpid = fork();
   if (cpid == -1) { perror("fork"); return info; }
   
   if (cpid == 0) {    /* Le fils lit dans le tube */
      close(pfd[0]);  /* Fermeture du descripteur en lecture inutilisé */
      
      dup2(pfd[1], STDOUT_FILENO);
      close(pfd[1]);
      
      if(info->type == ECOMIX_ARCH_TYPE_RAR)
	 execlp("unrar", "unrar", "vt", "-c-", info->archname, (char *) NULL);
      else if(info->type == ECOMIX_ARCH_TYPE_LHA)
	 execlp("lha", "lha", "l", info->archname, (char *) NULL);
      else if(info->type == ECOMIX_ARCH_TYPE_7Z)
	 execlp("7z", "7z", "l", info->archname, (char *) NULL);
      else
	 execlp("false", "false", (char *) NULL);
      
      _exit(EXIT_SUCCESS);
      
   } else {            /* Le père écrit argv[1] dans le tube */
      int n = 0, size = 0;
      char *buf = NULL, *p;
      
      close(pfd[1]);  /* Fermeture du descripteur en écriture inutilisé */
      
      buf = malloc(ECOMIX_IMAGE_DATA_READ_BUF);
      p = buf;
      while ((n = read(pfd[0], p, ECOMIX_IMAGE_DATA_READ_BUF)) > 0
	    && size < ECOMIX_FILE_BUF_SIZE_MAX) {
	 // printf("parent: %d : %s\n", n, buf);
	 size += n;
	 buf = realloc(buf, size + ECOMIX_IMAGE_DATA_READ_BUF);
	 p = buf + size;
      }
      close(pfd[0]);          /* Le lecteur verra EOF */
      waitpid(cpid, NULL, 0);             /* Attente du fils */
      
      if(buf) {
	 if(info->type == ECOMIX_ARCH_TYPE_RAR)
	    info->list = ecomix_arch_list_rar_decode(buf, size);
	 else if(info->type == ECOMIX_ARCH_TYPE_LHA || info->type == ECOMIX_ARCH_TYPE_7Z)
	    info->list = ecomix_arch_list_lha_decode(buf, size);
	 free(buf);
      }
   }

   if(info->list)
      info->list = eina_list_sort(info->list, eina_list_count(info->list), sort_cb);
   return info;
}

Ecomix_Arch_Info *ecomix_arch_list_get(Ecomix_Arch_Info *info)
{
   if(! info || !info->archname) return NULL;

   switch(info->type) {
      case ECOMIX_ARCH_TYPE_LIBEVAS:
	 info->list = ecomix_arch_list_libevas_get(info->archname);
	 break;
      case ECOMIX_ARCH_TYPE_LIBARCH:
	 info->list = ecomix_arch_list_libarch_get(info->archname);
	 break;
      case ECOMIX_ARCH_TYPE_RAR:
      case ECOMIX_ARCH_TYPE_LHA:
      case ECOMIX_ARCH_TYPE_7Z:
	 info = ecomix_arch_list_pipe_get(info);
	 break;
      default:
	 printf("ArchType Unknow\n");
   }

   return info;

}

Ecomix_File_Buffer *ecomix_arch_data_libevas_get(Ecomix_Arch_Info *info, char *filename)
{
   Ecomix_File_Buffer *fbuf = NULL;

   if(!info || !filename) return NULL;
   if(info->type != ECOMIX_ARCH_TYPE_LIBEVAS) return NULL;

   fbuf = (Ecomix_File_Buffer *) calloc(1, sizeof(Ecomix_File_Buffer));
   if(fbuf) {
      fbuf->filename = eina_stringshare_add(filename);
   }

   return fbuf;
}

Ecomix_File_Buffer *ecomix_arch_data_libarch_get(Ecomix_Arch_Info *info, char *filename)
{
   struct archive *a;
   struct archive_entry *entry;
   Ecomix_File_Buffer *fbuf = NULL;

   if(!info || !filename) return NULL;
   if(info->type != ECOMIX_ARCH_TYPE_LIBARCH) return NULL;

   a = archive_read_new();
   archive_read_support_compression_all(a);
   archive_read_support_format_all(a);
   if(archive_read_open_filename(a, info->archname, ECOMIX_IMAGE_DATA_READ_BUF) == ARCHIVE_OK) 
   {
      while (archive_read_next_header(a, &entry) == ARCHIVE_OK) 
      {
	 if(! strcmp(filename, archive_entry_pathname(entry)))
	 {
	    char *buf;
	    size_t size = archive_entry_size(entry);

	    if(size < ECOMIX_FILE_BUF_SIZE_MAX) {
	       buf = malloc(size);
	       archive_read_data(a, buf, size);
	       if(buf) {
		  fbuf = (Ecomix_File_Buffer *) malloc(sizeof(Ecomix_File_Buffer));
		  if(fbuf) {
		     fbuf->filename = eina_stringshare_add(filename);
		     fbuf->buf = buf;
		     fbuf->size = size;
		  } else
		     free(buf);
	       } else {
		  archive_read_data_skip(a);
	       }
	    }
	 } 
	 else
	    archive_read_data_skip(a);
      }
   } 

   archive_read_finish(a);

   return fbuf;
}

Ecomix_File_Buffer *ecomix_arch_data_pipe_get(Ecomix_Arch_Info *info, char *filename)
{
   int pfd[2];
   pid_t cpid;
   Ecomix_File_Buffer *fbuf = NULL;

   if(!info || !info->archname || !filename) return fbuf;
   // if(!(info->type == ECOMIX_ARCH_TYPE_RAR || info->type == ECOMIX_ARCH_TYPE_LHA)) return fbuf;

   if (pipe(pfd) == -1) { perror("pipe"); return fbuf; }
   
   cpid = fork();
   if (cpid == -1) { perror("fork"); return fbuf; }
   
   if (cpid == 0) {    /* Le fils écrit dans le tube */
      close(pfd[0]);  /* Fermeture du descripteur en lecture inutilisé */
      
      dup2(pfd[1], STDOUT_FILENO);
      close(pfd[1]);
      close(STDERR_FILENO);
   
      if(info->type == ECOMIX_ARCH_TYPE_RAR)
	 execlp("unrar", "unrar", "p", "-inul", "-p-", info->archname, filename, (char *) NULL);
      else if(info->type == ECOMIX_ARCH_TYPE_LHA)
	 execlp("lha", "lha", "-pq", info->archname, filename, (char *) NULL);
      else if(info->type == ECOMIX_ARCH_TYPE_7Z)
	 execlp("7z", "7z", "e", "-so", info->archname, filename, (char *) NULL);
      else if(info->type == ECOMIX_ARCH_TYPE_CHECK_COMMAND)
	 execlp("which", "which", "unrar", "lha", "7z", (char *) NULL);
      else
	 execlp("false", "false", (char *) NULL);
      
      _exit(EXIT_SUCCESS);

   } else {            /* Le père les données dans le tube */
      char *buf = NULL, *p;
      int n = 0, size = 0;
      
      close(pfd[1]);  /* Fermeture du descripteur en écriture inutilisé */
      
      buf = malloc(ECOMIX_IMAGE_DATA_READ_BUF);
      p = buf;
      while ((n = read(pfd[0], p, ECOMIX_IMAGE_DATA_READ_BUF)) > 0
	    && size < ECOMIX_FILE_BUF_SIZE_MAX) {
	 // printf("parent: %d %d \n", size, n);
	 size += n;
	 buf = realloc(buf, (size + ECOMIX_IMAGE_DATA_READ_BUF));
	 p = buf + size;
      }
      close(pfd[0]);          
      waitpid(cpid, NULL, 0);
      
      if(buf) {
	 fbuf = (Ecomix_File_Buffer *) malloc(sizeof(Ecomix_File_Buffer));
	 if(fbuf) {
	    fbuf->filename = eina_stringshare_add(filename);
	    fbuf->buf = buf;
	    fbuf->size = size;
	 } else
	    free(buf);
      }
   }

   return fbuf;
}

Ecomix_File_Buffer *ecomix_arch_data_get(Ecomix_Arch_Info *info, char *filename)
{
   Ecomix_File_Buffer *buf = NULL;

   if(! info || !filename) return NULL;

   switch(info->type) {
      case ECOMIX_ARCH_TYPE_LIBEVAS:
	 buf = ecomix_arch_data_libevas_get(info, filename);
	 break;
      case ECOMIX_ARCH_TYPE_LIBARCH:
	 buf = ecomix_arch_data_libarch_get(info, filename);
	 break;
      case ECOMIX_ARCH_TYPE_RAR:
      case ECOMIX_ARCH_TYPE_LHA:
      case ECOMIX_ARCH_TYPE_7Z:
	 buf = ecomix_arch_data_pipe_get(info, filename);
	 break;
      default:
	 printf("ArchTYpe Unknow\n");
   }

   return buf;
}

Ecomix_File_Buffer * ecomix_arch_data_del(Ecomix_File_Buffer *buf)
{
   if(buf) {
      if(buf->filename) eina_stringshare_del(buf->filename);
      if(buf->buf) free(buf->buf);
      free(buf);
   }

   return NULL;
}

Ecomix_Arch_Info *ecomix_arch_file_set(char *archname, magic_t magic) {
   Ecomix_Arch_Info *info = NULL;


   if(archname && magic) {
      const char *res, *mtype = NULL;
      Ecomix_Arch_Type type = ECOMIX_ARCH_TYPE_UNKNOW;

      res = magic_file(magic, archname);
      if(res)
	 mtype = ecomix_file_type_check(res);
      if(mtype) {
	 if(! strcmp(mtype, "libevas")) 
	    type = ECOMIX_ARCH_TYPE_LIBEVAS;
	 else if(! strcmp(mtype, "libarch")) 
	    type = ECOMIX_ARCH_TYPE_LIBARCH;
	 else if(! strcmp(mtype, "rar"))
	    type = ECOMIX_ARCH_TYPE_RAR;
	 else if(! strcmp(mtype, "lha"))
	    type = ECOMIX_ARCH_TYPE_LHA;
	 else if(! strcmp(mtype, "7z"))
	    type = ECOMIX_ARCH_TYPE_7Z;
      } else {
	 char *p = rindex(archname, '.');
	 if(p) {
	    if(strcmp(p, ".edj") == 0 || strcmp(p, ".eet") == 0)
	       type = ECOMIX_ARCH_TYPE_LIBEVAS;
	 }
      }

      if(type != ECOMIX_ARCH_TYPE_UNKNOW) {
	 info = (Ecomix_Arch_Info *) malloc(sizeof(Ecomix_Arch_Info));
	 if(info) {
	    info->archname = eina_stringshare_add(archname);
	    info->type = type;
	 }
      }
   }

   return info;
}

Ecomix_Arch_Info *ecomix_arch_file_del(Ecomix_Arch_Info *info) {
   if(info) {
      char *d;
      if(info->archname) info->archname = eina_stringshare_add(info->archname);
      if(info->list) EINA_LIST_FREE(info->list, d) eina_stringshare_del(d);

      free(info);
   }

   return NULL;
}

const char*ecomix_file_type_check(const char*mimetype) {
   const char *ret = NULL;
   
   if(mimetype_file && mimetype)
      ret = eina_hash_find(mimetype_file, mimetype);

   return ret;
}

const Ecomix_Buffer_Load *ecomix_buffer_type_check(const char*mimetype) {
   const Ecomix_Buffer_Load *ret = NULL;
   
   if(mimetype_buffer)
      ret = eina_hash_find(mimetype_buffer, mimetype);

   return ret;
}


void ecomix_arch_finish() {
   if(mimetype_file)
      eina_hash_free(mimetype_file);

   if(mimetype_buffer)
      eina_hash_free(mimetype_buffer);
}

Ecomix_Arch_Type ecomix_arch_check_command(void) {
   Ecomix_Arch_Type type = ECOMIX_ARCH_TYPE_UNKNOW;
   Ecomix_Arch_Info info;
   Ecomix_File_Buffer *fbuf;

   info.archname = "test command";
   info.type = ECOMIX_ARCH_TYPE_CHECK_COMMAND;

   fbuf = ecomix_arch_data_pipe_get(&info, "test command");
   if(fbuf) {
      char *p = fbuf->buf, *s = fbuf->buf;
      const char *cmd;
      while(p < fbuf->buf + fbuf->size) {
	 if(*p == '\n') {
	    *p = '\0';
	    cmd = ecore_file_file_get(s);
	    if(! strcmp(cmd, "unrar"))
	       type |= ECOMIX_ARCH_TYPE_RAR;
	    else if(! strcmp(cmd, "lha"))
	       type |= ECOMIX_ARCH_TYPE_LHA;
	    else if(! strcmp(cmd, "7z"))
	       type |= ECOMIX_ARCH_TYPE_7Z;
	    s = p + 1;
	 }
	 p++;
      }
      ecomix_arch_data_del(fbuf);
   }

   return type;
}

void ecomix_arch_init() {
   Ecomix_Arch_Type type = ecomix_arch_check_command();

   if(! mimetype_file) {
      mimetype_file = eina_hash_string_superfast_new(NULL);
      eina_hash_add(mimetype_file, "image/gif",                "libevas");
      eina_hash_add(mimetype_file, "image/png",                "libevas");
      eina_hash_add(mimetype_file, "image/jpeg",               "libevas");
      eina_hash_add(mimetype_file, "image/tiff",               "libevas");
      eina_hash_add(mimetype_file, "image/svg+xml",            "libevas");
      eina_hash_add(mimetype_file, "image/x-xpixmap",          "libevas");
      // eina_hash_add(mimetype_file, "text/x-c",                 "libevas");
      eina_hash_add(mimetype_file, "image/x-portable-anymap",  "libevas");
      eina_hash_add(mimetype_file, "image/x-portable-bitmap",  "libevas");
      eina_hash_add(mimetype_file, "image/x-portable-graymap", "libevas");
      eina_hash_add(mimetype_file, "image/x-portable-greymap", "libevas");
      eina_hash_add(mimetype_file, "image/x-portable-pixmap",  "libevas");
      eina_hash_add(mimetype_file, "application/x-eet",   "libevas");

      eina_hash_add(mimetype_file, "application/x-tar",   "libarch");
      eina_hash_add(mimetype_file, "application/x-gtar",  "libarch");
      eina_hash_add(mimetype_file, "application/zip",     "libarch");
      eina_hash_add(mimetype_file, "application/x-bzip2", "libarch");
      eina_hash_add(mimetype_file, "application/x-bcpio", "libarch");
      eina_hash_add(mimetype_file, "application/x-cpio",  "libarch");

      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.chart", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.database", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.formula", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.graphics", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.graphics-template", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.image", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.presentation", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.presentation-template", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.spreadsheet", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.spreadsheet-template", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.text", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.text-master", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.text-template", "libarch");
      eina_hash_add(mimetype_file, "application/vnd.oasis.opendocument.text-web", "libarch");

      eina_hash_add(mimetype_file, "", "libarch");

      if(type & ECOMIX_ARCH_TYPE_RAR) {
   	 eina_hash_add(mimetype_file, "application/x-rar", "rar");
      }

      if(type & ECOMIX_ARCH_TYPE_LHA) {
	 eina_hash_add(mimetype_file, "application/x-lharc", "lha");
	 eina_hash_add(mimetype_file, "application/x-lha",   "lha");
      }

      if(type & ECOMIX_ARCH_TYPE_7Z) {
	 if(! (type & ECOMIX_ARCH_TYPE_LHA)) {
	    eina_hash_add(mimetype_file, "application/x-lharc", "7z");
	    eina_hash_add(mimetype_file, "application/x-lha",   "7z");
	 }
	 if(! (type & ECOMIX_ARCH_TYPE_LIBARCH)) {
	    eina_hash_add(mimetype_file, "application/x-tar",   "7z");
	    eina_hash_add(mimetype_file, "application/x-gtar",  "7z");
	    eina_hash_add(mimetype_file, "application/zip",     "7z");
	    eina_hash_add(mimetype_file, "application/x-bzip2", "7z");
	    eina_hash_add(mimetype_file, "application/x-bcpio", "7z");
	    eina_hash_add(mimetype_file, "application/x-cpio",  "7z");
	 }
	 eina_hash_add(mimetype_file, "application/x-7z-compressed",  "7z");
	 eina_hash_add(mimetype_file, "application/x-gzip",  "7z");
	 eina_hash_add(mimetype_file, "application/vnd.ms-cab-compressed",  "7z");
      }
   }

   if(! mimetype_buffer) {
      static Ecomix_Buffer_Load libgm, libevas;

      mimetype_buffer = eina_hash_string_superfast_new(NULL);

      libevas.head = ecomix_image_load_head_libevas;
      libevas.data = ecomix_image_load_data_libevas;

      libgm.head = ecomix_image_load_fmem_head_libgm;
      libgm.data = ecomix_image_load_fmem_data_libgm;

      eina_hash_add(mimetype_buffer, "libevas",  &libevas);
      eina_hash_add(mimetype_buffer, "image/gif",  &libgm);
      eina_hash_add(mimetype_buffer, "image/png",  &libgm);
      eina_hash_add(mimetype_buffer, "image/jpeg", &libgm);
      eina_hash_add(mimetype_buffer, "image/tiff", &libgm);
      eina_hash_add(mimetype_buffer, "image/x-xcf", &libgm);
      eina_hash_add(mimetype_buffer, "image/svg+xml", &libgm);
      eina_hash_add(mimetype_buffer, "image/x-xpixmap", &libgm);
      eina_hash_add(mimetype_buffer, "image/x-portable-anymap",  &libgm);
      eina_hash_add(mimetype_buffer, "image/x-portable-bitmap",  &libgm);
      eina_hash_add(mimetype_buffer, "image/x-portable-graymap", &libgm);
      eina_hash_add(mimetype_buffer, "image/x-portable-greymap", &libgm);
      eina_hash_add(mimetype_buffer, "image/x-portable-pixmap",  &libgm);
      // eina_hash_add(mimetype_buffer, "text/x-c",        &xpm);
   }
}
