/*  Audacious
 *  Copyright (C) 2005-2007  Audacious development team.
 *
 *  BMP - Cross-platform multimedia player
 *  Copyright (C) 2003-2004  BMP development team.
 *
 *  Based on XMMS:
 *  Copyright (C) 1998-2003  XMMS development team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; under version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses>.
 *
 *  The Audacious team does not consider modular code linking to
 *  Audacious or using our public API to be a derived work.
 */

#ifndef AUDACIOUS_STRINGS_H
#define AUDACIOUS_STRINGS_H

#include <stdlib.h>
#include <glib.h>

G_BEGIN_DECLS

gchar *str_append(gchar * str, const gchar * add_str);
gchar *str_replace(gchar * str, gchar * new_str);
void str_replace_in(gchar ** str, gchar * new_str);

gboolean str_has_prefix_nocase(const gchar * str, const gchar * prefix);
gboolean str_has_suffix_nocase(const gchar * str, const gchar * suffix);
gboolean str_has_suffixes_nocase(const gchar * str, gchar * const *suffixes);

gchar *str_assert_utf8(const gchar *str);

void str_set_utf8_impl (gchar * (* stu_impl) (const gchar *),
 gchar * (* stuf_impl) (const gchar *, gssize, gsize *, gsize *, GError * *));
gchar * str_to_utf8 (const gchar * str);
gchar * str_to_utf8_full (const gchar * str, gssize len, gsize * bytes_read,
 gsize * bytes_written, GError * * err);

const gchar *str_skip_chars(const gchar * str, const gchar * chars);

gchar *convert_dos_path(gchar * text);

gchar *filename_get_subtune(const gchar * filename, gint * track);
gchar *filename_split_subtune(const gchar * filename, gint * track);

void string_replace_char (gchar * string, gchar old_str, gchar new_str);
void string_decode_percent (gchar * string);
gchar * string_encode_percent (const gchar * string, gboolean is_filename);

gboolean uri_is_utf8 (const gchar * uri, gboolean warn);
gchar * uri_to_utf8 (const gchar * uri);
void uri_check_utf8 (gchar * * uri, gboolean warn);
gchar * filename_to_uri (const gchar * filename);
gchar * uri_to_filename (const gchar * uri);
gchar * uri_to_display (const gchar * uri);

gchar * uri_get_extension (const gchar * uri);

void string_cut_extension(gchar *string);
gint string_compare (const gchar * a, const gchar * b);
gint string_compare_encoded (const gchar * a, const gchar * b);

const void * memfind (const void * mem, gint size, const void * token, gint
 length);

gchar *str_replace_fragment(gchar *s, gint size, const gchar *old_str, const gchar *new_str);

void string_canonize_case(gchar *string);

gboolean string_to_int (const gchar * string, gint * addr);
gboolean string_to_double (const gchar * string, gdouble * addr);
gchar * int_to_string (gint val);
gchar * double_to_string (gdouble val);

gboolean string_to_double_array (const gchar * string, gdouble * array, gint count);
gchar * double_array_to_string (const gdouble * array, gint count);

G_END_DECLS

#endif /* AUDACIOUS_STRINGS_H */
