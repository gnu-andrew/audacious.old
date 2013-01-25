#ifndef TAGUTIL_H

#define TAGUTIL_H

#include <glib.h>
#include "libaudcore/tuple.h"
#include "libaudcore/vfs.h"

#define WMA_DEBUG 1

#if WMA_DEBUG
#define DEBUG(...) printf("TAG %25s:\t", __func__); printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif


/* defines functions for WMA file handling */

#ifdef TEST /* this should only be defined by the tests, so that VFS won't be used */
#define VFSFile FILE
#define vfs_fopen(...)  fopen(__VA_ARGS__)
#define vfs_fread(...)  bla = fread(__VA_ARGS__)
#define vfs_fwrite(...) bla = fwrite(__VA_ARGS__)
#define vfs_fseek(...)  bla = fseek(__VA_ARGS__)
#define vfs_fclose(...) bla = fclose(__VA_ARGS__)
#define vfs_feof(...)   feof(__VA_ARGS__)
#define vfs_ftell(...)  ftell(__VA_ARGS__)
#endif



#ifndef BLA
#define BLA
int bla;
#endif

time_t unix_time(guint64 win_time);

guint16 get_year(guint64 win_time);

void printTuple(Tuple *tuple);
Tuple *makeTuple(Tuple *tuple, const gchar* title, const gchar* artist,
        const gchar* comment, const gchar* album,
        const gchar * genre, const gchar* year,
        const gchar* filePath, int tracnr);


const gchar* get_complete_filepath(Tuple *tuple);
#endif		   