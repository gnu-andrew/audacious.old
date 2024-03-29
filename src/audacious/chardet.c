/*  Audacious
 *  Copyright (C) 2005-2007  Audacious development team.
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

#include <string.h>
#include <libaudcore/audstrings.h>

#include "config.h"
#include "debug.h"
#include "i18n.h"
#include "main.h"
#include "misc.h"

#ifdef USE_CHARDET
#  include <libguess.h>
#endif

static gchar * cd_chardet_to_utf8 (const gchar * str, gssize len,
 gsize * arg_bytes_read, gsize * arg_bytes_write, GError ** error);

static gchar * str_to_utf8_fallback (const gchar * str)
{
    gchar * out = g_strconcat (str, _("  (invalid UTF-8)"), NULL);

    for (gchar * c = out; * c; c ++)
    {
        if (* c & 0x80)
            * c = '?';
    }

    return out;
}

static gchar * cd_str_to_utf8 (const gchar * str)
{
    gchar *out_str;

    if (str == NULL)
        return NULL;

    /* Note: Currently, playlist calls this function repeatedly, even
     * if the string is already converted into utf-8.
     * chardet_to_utf8() would convert a valid utf-8 string into a
     * different utf-8 string, if fallback encodings were supplied and
     * the given string could be treated as a string in one of
     * fallback encodings. To avoid this, g_utf8_validate() had been
     * used at the top of evaluation.
     */

    /* Note 2: g_utf8_validate() has so called encapsulated utf-8
     * problem, thus chardet_to_utf8() took the place of that.
     */

    /* Note 3: As introducing madplug, the problem of conversion from
     * ISO-8859-1 to UTF-8 arose. This may be coped with g_convert()
     * located near the end of chardet_to_utf8(), but it requires utf8
     * validation guard where g_utf8_validate() was. New
     * dfa_validate_utf8() employs libguess' DFA engine to validate
     * utf-8 and can properly distinguish examples of encapsulated
     * utf-8. It is considered to be safe to use as a guard.
     */

    /* Already UTF-8? */
#ifdef USE_CHARDET
    if (libguess_validate_utf8(str, strlen(str)))
        return g_strdup(str);
#else
    if (g_utf8_validate(str, strlen(str), NULL))
        return g_strdup(str);
#endif

    /* chardet encoding detector */
    if ((out_str = cd_chardet_to_utf8(str, strlen(str), NULL, NULL, NULL)) != NULL)
        return out_str;

    /* all else fails, we mask off character codes >= 128, replace with '?' */
    return str_to_utf8_fallback(str);
}

static gchar * cd_chardet_to_utf8 (const gchar * str, gssize len,
 gsize * arg_bytes_read, gsize * arg_bytes_write, GError ** error)
{
    if (error)
        * error = NULL;

    gchar *ret = NULL;
    gsize *bytes_read, *bytes_write;
    gsize my_bytes_read, my_bytes_write;

    bytes_read = arg_bytes_read != NULL ? arg_bytes_read : &my_bytes_read;
    bytes_write = arg_bytes_write != NULL ? arg_bytes_write : &my_bytes_write;

    g_return_val_if_fail(str != NULL, NULL);

#ifdef USE_CHARDET
    if (libguess_validate_utf8(str, len))
#else
    if (g_utf8_validate(str, len, NULL))
#endif
    {
        if (len < 0)
            len = strlen (str);

        ret = g_malloc (len + 1);
        memcpy (ret, str, len);
        ret[len] = 0;

        if (arg_bytes_read != NULL)
            * arg_bytes_read = len;
        if (arg_bytes_write != NULL)
            * arg_bytes_write = len;

        return ret;
    }

#ifdef USE_CHARDET
    gchar * det = get_string (NULL, "chardet_detector");

    if (det[0])
    {
        AUDDBG("guess encoding (%s) %s\n", det, str);
        const gchar * encoding = libguess_determine_encoding (str, len, det);
        AUDDBG("encoding = %s\n", encoding);
        if (encoding)
            ret = g_convert (str, len, "UTF-8", encoding, bytes_read,
             bytes_write, (error && * error) ? NULL : error);
    }

    g_free (det);
#endif

    /* If detection failed or was not enabled, try fallbacks (if there are any) */
    if (! ret)
    {
        gchar * fallbacks = get_string (NULL, "chardet_fallback");
        gchar * * split = g_strsplit_set (fallbacks, " ,:;|/", -1);

        for (gchar * * enc = split; * enc; enc ++)
        {
            ret = g_convert (str, len, "UTF-8", * enc, bytes_read, bytes_write,
             (error && * error) ? NULL : error);
            if (len == *bytes_read)
                break;
            else {
                g_free(ret);
                ret = NULL;
            }
        }

        g_strfreev (split);
        g_free (fallbacks);
    }

    /* First fallback: locale (duh!) */
    if (ret == NULL)
        ret = g_locale_to_utf8 (str, len, bytes_read, bytes_write,
         (error && * error) ? NULL : error);

    /* The final fallback is ISO-8859-1, if no other is specified or conversions fail */
    if (ret == NULL)
        ret = g_convert (str, len, "UTF-8", "ISO-8859-1", bytes_read,
         bytes_write, (error && * error) ? NULL : error);

    if (ret != NULL)
    {
        if (g_utf8_validate(ret, -1, NULL))
            return ret;
        else
        {
            g_warning("g_utf8_validate() failed for converted string in cd_chardet_to_utf8: '%s'", ret);
            g_free(ret);
            return NULL;
        }
    }

    return NULL; /* If we have no idea, return NULL. */
}

void chardet_init (void)
{
    str_set_utf8_impl (cd_str_to_utf8, cd_chardet_to_utf8);
}
