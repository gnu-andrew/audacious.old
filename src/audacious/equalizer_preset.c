/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2008  Audacious team
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

#include <glib.h>

#include "config.h"
#include "debug.h"
#include "i18n.h"
#include "interface.h"
#include "misc.h"

static EqualizerPreset * equalizer_preset_new (const gchar * name)
{
    EqualizerPreset *preset = g_new0(EqualizerPreset, 1);
    preset->name = g_strdup(name);
    return preset;
}

GList *
equalizer_read_presets(const gchar *basename)
{
    gchar *filename, *name;
    GKeyFile *rcfile;
    GError *error = NULL;
    GList *list = NULL;
    gint i, p = 0;
    EqualizerPreset *preset;

    filename = g_build_filename (get_path (AUD_PATH_USER_DIR), basename, NULL);

    rcfile = g_key_file_new();
    if (!g_key_file_load_from_file(rcfile, filename, G_KEY_FILE_NONE, &error))
    {
        g_free(filename);
        filename = g_build_filename (get_path (AUD_PATH_DATA_DIR), basename,
         NULL);

        error = NULL;
        if (!g_key_file_load_from_file(rcfile, filename, G_KEY_FILE_NONE, &error))
        {
           g_free(filename);
           return NULL;
        }
    }

    g_free(filename);

    for (;;)
    {
        gchar section[32];

        error = NULL;
        g_snprintf(section, sizeof(section), "Preset%d", p++);

        if ((name = g_key_file_get_string(rcfile, "Presets", section, &error)) != NULL)
        {
            error = NULL;

            preset = g_new0(EqualizerPreset, 1);
            preset->name = name;
            preset->preamp = g_key_file_get_double(rcfile, name, "Preamp", &error);

            for (i = 0; i < AUD_EQUALIZER_NBANDS; i++)
            {
                gchar band[16];
                g_snprintf(band, sizeof(band), "Band%d", i);

                error = NULL;

                preset->bands[i] = g_key_file_get_double(rcfile, name, band, &error);
            }

            list = g_list_prepend(list, preset);
        }
        else
            break;
    }

    list = g_list_reverse(list);
    g_key_file_free(rcfile);

    return list;
}

gboolean equalizer_write_preset_file (GList * list, const gchar * basename)
{
    gchar *filename, *tmp;
    gint i, p;
    EqualizerPreset *preset;
    GKeyFile *rcfile;
    GList *node;
    gchar *data;
    gsize len;
    GError *error = NULL;

    rcfile = g_key_file_new();
    p = 0;

    for (node = list; node; node = g_list_next(node))
    {
        preset = node->data;

        tmp = g_strdup_printf("Preset%d", p++);
        g_key_file_set_string(rcfile, "Presets", tmp, preset->name);
        g_free(tmp);

        g_key_file_set_double(rcfile, preset->name, "Preamp", preset->preamp);

        for (i = 0; i < 10; i++)
        {
            tmp = g_strdup_printf("Band%d", i);
            g_key_file_set_double(rcfile, preset->name, tmp,
                                  preset->bands[i]);
            g_free(tmp);
        }
    }

    filename = g_build_filename (get_path (AUD_PATH_USER_DIR), basename, NULL);

    data = g_key_file_to_data(rcfile, &len, &error);
    gboolean success = g_file_set_contents (filename, data, len, & error);
    g_free(data);

    g_key_file_free(rcfile);
    g_free(filename);
    return success;
}

GList *
import_winamp_eqf(VFSFile * file)
{
    gchar header[31];
    gchar bands[11];
    gint i = 0;
    EqualizerPreset *preset = NULL;
    GList *list = NULL;
    gchar *markup;
    gchar preset_name[0xb4];

    if (vfs_fread (header, 1, sizeof header, file) != sizeof header || strncmp
     (header, "Winamp EQ library file v1.1", 27))
        goto error;

    AUDDBG("The EQF header is OK\n");

    if (vfs_fseek(file, 0x1f, SEEK_SET) == -1) goto error;

    while (vfs_fread(preset_name, 1, 0xb4, file) == 0xb4) {
        AUDDBG("The preset name is '%s'\n", preset_name);
        if (vfs_fseek (file, 0x4d, SEEK_CUR)) /* unknown crap --asphyx */
            break;
        if (vfs_fread(bands, 1, 11, file) != 11) break;

        preset = equalizer_preset_new(preset_name);
        /*this was divided by 63, but shouldn't it be 64? --majeru*/
        preset->preamp = EQUALIZER_MAX_GAIN - ((bands[10] * EQUALIZER_MAX_GAIN * 2) / 64.0);

        for (i = 0; i < 10; i++)
            preset->bands[i] = EQUALIZER_MAX_GAIN - ((bands[i] * EQUALIZER_MAX_GAIN * 2) / 64.0);

        list = g_list_prepend(list, preset);
    }

    list = g_list_reverse(list);
    if (list == NULL) goto error;

    return list;

error:
    markup = g_strdup_printf (_("Error importing Winamp EQF file '%s'"),
     file->uri);
    interface_show_error(markup);

    g_free(markup);
    return NULL;
}

gboolean save_preset_file (EqualizerPreset * preset, const gchar * filename)
{
    GKeyFile *rcfile;
    gint i;
    gchar *data;
    gsize len;
    GError *error = NULL;

    rcfile = g_key_file_new();
    g_key_file_set_double(rcfile, "Equalizer preset", "Preamp", preset->preamp);

    for (i = 0; i < 10; i++) {
        gchar tmp[7];
        g_snprintf(tmp, sizeof(tmp), "Band%d", i);
        g_key_file_set_double(rcfile, "Equalizer preset", tmp,
                              preset->bands[i]);
    }

    data = g_key_file_to_data(rcfile, &len, &error);

    gboolean success = FALSE;

    VFSFile * file = vfs_fopen (filename, "w");
    if (file == NULL)
        goto DONE;
    if (vfs_fwrite (data, 1, strlen (data), file) == strlen (data))
        success = TRUE;
    vfs_fclose (file);

DONE:
    g_free(data);
    g_key_file_free(rcfile);
    return success;
}

static EqualizerPreset * equalizer_read_aud_preset (const gchar * filename)
{
    gint i;
    EqualizerPreset *preset;
    GKeyFile *rcfile;
    GError *error = NULL;

    preset = g_new0(EqualizerPreset, 1);
    preset->name = g_strdup("");

    rcfile = g_key_file_new();
    if (!g_key_file_load_from_file(rcfile, filename, G_KEY_FILE_NONE, &error))
    {
        g_key_file_free(rcfile);
        return NULL;
    }

    preset->preamp = g_key_file_get_double(rcfile, "Equalizer preset", "Preamp", &error);
    for (i = 0; i < 10; i++)
    {
        gchar tmp[7];
        g_snprintf(tmp, sizeof(tmp), "Band%d", i);

        preset->bands[i] = g_key_file_get_double(rcfile, "Equalizer preset", tmp, &error);
    }

    g_key_file_free(rcfile);
    return preset;
}

EqualizerPreset *
load_preset_file(const gchar *filename)
{
    if (filename) {
        EqualizerPreset *preset = equalizer_read_aud_preset(filename);
        return preset;
    }
    return NULL;
}
