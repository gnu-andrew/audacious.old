/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2009  Audacious development team
 *
 *  Based on BMP:
 *  Copyright (C) 2003-2004  BMP development team
 *
 *  Based on XMMS:
 *  Copyright (C) 1998-2003  XMMS development team
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifndef SHARED_SUFFIX
# define SHARED_SUFFIX G_MODULE_SUFFIX
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gmodule.h>
#include <string.h>

#include "pluginenum.h"

#include "audconfig.h"
#include "credits.h"
#include "discovery.h"
#include "effect.h"
#include "general.h"
#include "input.h"
#include "main.h"
#include "chardet.h"
#include "output.h"
#include "playback.h"
#include "playlist-new.h"
#include "playlist-utils.h"
#include "audstrings.h"
#include "util.h"
#include "visualization.h"
#include "preferences.h"
#include "vfs_buffer.h"
#include "vfs_buffered_file.h"
#include "volumecontrol.h"
#include "equalizer_preset.h"

#include "ui_fileinfo.h"
#include "ui_fileinfopopup.h"
#include "ui_plugin_menu.h"
#include "ui_preferences.h"


const gchar *plugin_dir_list[] = {
    PLUGINSUBS,
    NULL
};

GHashTable *ext_hash = NULL;

static void set_pvt_data(Plugin * plugin, gpointer data);
static gpointer get_pvt_data(void);

/*****************************************************************/

static struct _AudaciousFuncTableV1 _aud_papi_v1 = {
    .vfs_fopen = vfs_fopen,
    .vfs_fclose = vfs_fclose,
    .vfs_dup = vfs_dup,
    .vfs_fread = vfs_fread,
    .vfs_fwrite = vfs_fwrite,
    .vfs_getc = vfs_getc,
    .vfs_ungetc = vfs_ungetc,
    .vfs_fgets = vfs_fgets,
    .vfs_fseek = vfs_fseek,
    .vfs_rewind = vfs_rewind,
    .vfs_ftell = vfs_ftell,
    .vfs_feof = vfs_feof,
    .vfs_file_test = vfs_file_test,
    .vfs_is_writeable = vfs_is_writeable,
    .vfs_truncate = vfs_truncate,
    .vfs_fsize = vfs_fsize,
    .vfs_get_metadata = vfs_get_metadata,
    .vfs_fprintf = vfs_fprintf,
    .vfs_register_transport = vfs_register_transport,
    .vfs_file_get_contents = vfs_file_get_contents,
    .vfs_is_remote = vfs_is_remote,
    .vfs_is_streaming = vfs_is_streaming,

    .vfs_buffer_new = vfs_buffer_new,
    .vfs_buffer_new_from_string = vfs_buffer_new_from_string,

    .vfs_buffered_file_new_from_uri = vfs_buffered_file_new_from_uri,
    .vfs_buffered_file_release_live_fd = vfs_buffered_file_release_live_fd,

    .vfs_fget_le16 = vfs_fget_le16,
    .vfs_fget_le32 = vfs_fget_le32,
    .vfs_fget_le64 = vfs_fget_le64,
    .vfs_fget_be16 = vfs_fget_be16,
    .vfs_fget_be32 = vfs_fget_be32,
    .vfs_fget_be64 = vfs_fget_be64,

    .vfs_fput_le16 = vfs_fput_le16,
    .vfs_fput_le32 = vfs_fput_le32,
    .vfs_fput_le64 = vfs_fput_le64,
    .vfs_fput_be16 = vfs_fput_be16,
    .vfs_fput_be32 = vfs_fput_be32,
    .vfs_fput_be64 = vfs_fput_be64,

    .cfg_db_open = cfg_db_open,
    .cfg_db_close = cfg_db_close,

    .cfg_db_get_string = cfg_db_get_string,
    .cfg_db_get_int = cfg_db_get_int,
    .cfg_db_get_bool = cfg_db_get_bool,
    .cfg_db_get_float = cfg_db_get_float,
    .cfg_db_get_double = cfg_db_get_double,

    .cfg_db_set_string = cfg_db_set_string,
    .cfg_db_set_int = cfg_db_set_int,
    .cfg_db_set_bool = cfg_db_set_bool,
    .cfg_db_set_float = cfg_db_set_float,
    .cfg_db_set_double = cfg_db_set_double,

    .cfg_db_unset_key = cfg_db_unset_key,

    .mime_get_plugin = mime_get_plugin,
    .mime_set_plugin = mime_set_plugin,

    .uri_get_plugin = uri_get_plugin,
    .uri_set_plugin = uri_set_plugin,

    .util_info_dialog = util_info_dialog,

    .smart_realloc = smart_realloc,
    .sadfmt_from_afmt = sadfmt_from_afmt,

    .util_add_url_history_entry = util_add_url_history_entry,

    .str_to_utf8 = cd_str_to_utf8,
    .chardet_to_utf8 = cd_chardet_to_utf8,

    .playlist_container_register = playlist_container_register,
    .playlist_container_unregister = playlist_container_unregister,
    .playlist_container_read = playlist_container_read,
    .playlist_container_write = playlist_container_write,
    .playlist_container_find = playlist_container_find,

    .playlist_count = playlist_count,
    .playlist_insert = playlist_insert,
    .playlist_delete = playlist_delete,

    .playlist_set_filename = playlist_set_filename,
    .playlist_get_filename = playlist_get_filename,
    .playlist_set_title = playlist_set_title,
    .playlist_get_title = playlist_get_title,

    .playlist_set_active = playlist_set_active,
    .playlist_get_active = playlist_get_active,
    .playlist_set_playing = playlist_set_playing,
    .playlist_get_playing = playlist_get_playing,

    .playlist_entry_count = playlist_entry_count,
    .playlist_entry_insert = playlist_entry_insert,
    .playlist_entry_insert_batch = playlist_entry_insert_batch,
    .playlist_entry_delete = playlist_entry_delete,

    .playlist_entry_get_filename = playlist_entry_get_filename,
    .playlist_entry_get_decoder = playlist_entry_get_decoder,
    .playlist_entry_set_tuple = playlist_entry_set_tuple,
    .playlist_entry_get_tuple = playlist_entry_get_tuple,
    .playlist_entry_get_title = playlist_entry_get_title,
    .playlist_entry_get_length = playlist_entry_get_length,

    .playlist_set_position = playlist_set_position,
    .playlist_get_position = playlist_get_position,

    .playlist_entry_set_selected = playlist_entry_set_selected,
    .playlist_entry_get_selected = playlist_entry_get_selected,
    .playlist_selected_count = playlist_selected_count,
    .playlist_select_all = playlist_select_all,

    .playlist_shift = playlist_shift,
    .playlist_shift_selected = playlist_shift_selected,
    .playlist_delete_selected = playlist_delete_selected,
    .playlist_reverse = playlist_reverse,
    .playlist_randomize = playlist_randomize,

    .playlist_sort_by_filename = playlist_sort_by_filename,
    .playlist_sort_by_tuple = playlist_sort_by_tuple,
    .playlist_sort_selected_by_filename = playlist_sort_selected_by_filename,
    .playlist_sort_selected_by_tuple = playlist_sort_selected_by_tuple,

    .playlist_rescan = playlist_rescan,

    .playlist_get_total_length = playlist_get_total_length,
    .playlist_get_selected_length = playlist_get_selected_length,

    .playlist_set_shuffle = playlist_set_shuffle,

    .playlist_queue_count = playlist_queue_count,
    .playlist_queue_insert = playlist_queue_insert,
    .playlist_queue_insert_selected = playlist_queue_insert_selected,
    .playlist_queue_get_entry = playlist_queue_get_entry,
    .playlist_queue_find_entry = playlist_queue_find_entry,
    .playlist_queue_delete = playlist_queue_delete,

    .playlist_prev_song = playlist_prev_song,
    .playlist_next_song = playlist_next_song,

    .playlist_update_pending = playlist_update_pending,

    .get_gentitle_format = get_gentitle_format,

    .playlist_sort_by_scheme = playlist_sort_by_scheme,
    .playlist_sort_selected_by_scheme = playlist_sort_selected_by_scheme,
    .playlist_remove_duplicates_by_scheme = playlist_remove_duplicates_by_scheme,
    .playlist_remove_failed = playlist_remove_failed,
    .playlist_select_by_patterns = playlist_select_by_patterns,

    .filename_is_playlist = filename_is_playlist,

    .playlist_insert_playlist = playlist_insert_playlist,
    .playlist_save = playlist_save,
    .playlist_add_folder = playlist_add_folder,

    .ip_state = &ip_data,
    ._cfg = &cfg,

    .hook_associate = hook_associate,
    .hook_dissociate = hook_dissociate,
    .hook_register = hook_register,
    .hook_call = hook_call,

    .open_ini_file = open_ini_file,
    .close_ini_file = close_ini_file,
    .read_ini_string = read_ini_string,
    .read_ini_array = read_ini_array,

    .menu_plugin_item_add = menu_plugin_item_add,
    .menu_plugin_item_remove = menu_plugin_item_remove,

    .drct_quit = drct_quit,
    .drct_eject = drct_eject,
    .drct_jtf_show = drct_jtf_show,
    .drct_main_win_is_visible = drct_main_win_is_visible,
    .drct_main_win_toggle = drct_main_win_toggle,
    .drct_eq_win_is_visible = drct_eq_win_is_visible,
    .drct_eq_win_toggle = drct_eq_win_toggle,
    .drct_pl_win_is_visible = drct_pl_win_is_visible,
    .drct_pl_win_toggle = drct_pl_win_toggle,
    .drct_activate = drct_activate,

    .drct_initiate = drct_initiate,
    .drct_play = drct_play,
    .drct_pause = drct_pause,
    .drct_stop = drct_stop,
    .drct_get_playing = drct_get_playing,
    .drct_get_paused = drct_get_paused,
    .drct_get_stopped = drct_get_stopped,
    .drct_get_info = playback_get_info,
    .drct_get_time = playback_get_time,
    .drct_get_length = playback_get_length,
    .drct_seek = drct_seek,
    .drct_get_volume = drct_get_volume,
    .drct_set_volume = drct_set_volume,
    .drct_get_volume_main = drct_get_volume_main,
    .drct_set_volume_main = drct_set_volume_main,
    .drct_get_volume_balance = drct_get_volume_balance,
    .drct_set_volume_balance = drct_set_volume_balance,

    .drct_pl_next = drct_pl_next,
    .drct_pl_prev = drct_pl_prev,
    .drct_pl_repeat_is_enabled = drct_pl_repeat_is_enabled,
    .drct_pl_repeat_toggle = drct_pl_repeat_toggle,
    .drct_pl_repeat_is_shuffled = drct_pl_repeat_is_shuffled,
    .drct_pl_shuffle_toggle = drct_pl_shuffle_toggle,
    .drct_pl_get_title = drct_pl_get_title,
    .drct_pl_get_time = drct_pl_get_time,
    .drct_pl_get_pos = drct_pl_get_pos,
    .drct_pl_get_file = drct_pl_get_file,
    .drct_pl_add = drct_pl_add,
    .drct_pl_clear = drct_pl_clear,
    .drct_pl_get_length = drct_pl_get_length,
    .drct_pl_delete = drct_pl_delete,
    .drct_pl_set_pos = drct_pl_set_pos,
    .drct_pl_ins_url_string = drct_pl_ins_url_string,
    .drct_pl_add_url_string = drct_pl_add_url_string,
    .drct_pl_enqueue_to_temp = drct_pl_enqueue_to_temp,

    .drct_pq_get_length = drct_pq_get_length,
    .drct_pq_add = drct_pq_add,
    .drct_pq_remove = drct_pq_remove,
    .drct_pq_clear = drct_pq_clear,
    .drct_pq_is_queued = drct_pq_is_queued,
    .drct_pq_get_position = drct_pq_get_position,
    .drct_pq_get_queue_position = drct_pq_get_queue_position,

    .prefswin_page_new = prefswin_page_new,
    .prefswin_page_destroy = prefswin_page_destroy,

    .fileinfopopup_create = fileinfopopup_create,
    .fileinfopopup_destroy = fileinfopopup_destroy,
    .fileinfopopup_show_from_title = fileinfopopup_show_from_title,
    .fileinfopopup_show_from_tuple = fileinfopopup_show_from_tuple,
    .fileinfopopup_hide = fileinfopopup_hide,

    .util_get_localdir = util_get_localdir,

    .input_check_file = input_check_file,

    .flow_new = flow_new,
    .flow_execute = flow_execute,
    .flow_link_element = flow_link_element,
    .flow_unlink_element = flow_unlink_element,
    .effect_flow = effect_flow,
    .volumecontrol_flow = volumecontrol_flow,

    .get_output_list = get_output_list,

    .input_get_volume = input_get_volume,
    .construct_uri = construct_uri,
    .uri_to_display_basename = uri_to_display_basename,
    .uri_to_display_dirname = uri_to_display_dirname,

    .get_pvt_data = get_pvt_data,
    .set_pvt_data = set_pvt_data,

    .event_queue = event_queue,
    .event_queue_with_data_free = event_queue_with_data_free,

    .calc_mono_freq = calc_mono_freq,
    .calc_mono_pcm = calc_mono_pcm,
    .calc_stereo_pcm = calc_stereo_pcm,

    .create_widgets = create_widgets,

    .equalizer_read_presets = equalizer_read_presets,
    .equalizer_write_preset_file = equalizer_write_preset_file,
    .import_winamp_eqf = import_winamp_eqf,
    .save_preset_file = save_preset_file,
    .equalizer_read_aud_preset = equalizer_read_aud_preset,
    .load_preset_file = load_preset_file,
    .output_plugin_cleanup = output_plugin_cleanup,
    .output_plugin_reinit = output_plugin_reinit,

    .get_plugin_menu = get_plugin_menu,
    .playback_get_title = playback_get_title,
    .fileinfo_show = ui_fileinfo_show,
    .fileinfo_show_current = ui_fileinfo_show_current,
    .save_all_playlists = save_all_playlists,

    .interface_get_current = interface_get_current,
    .interface_toggle_visibility = interface_toggle_visibility,
    .interface_show_error = interface_show_error_message,

    .get_audacious_credits = get_audacious_credits,
};

/*****************************************************************/

GList *lowlevel_list = NULL;
extern GList *vfs_transports;

static mowgli_dictionary_t *plugin_dict = NULL;

static GStaticPrivate cur_plugin_key = G_STATIC_PRIVATE_INIT;
static mowgli_dictionary_t *pvt_data_dict = NULL;

static mowgli_list_t *headers_list = NULL;

void plugin_set_current(Plugin *plugin)
{
    g_static_private_set(&cur_plugin_key, plugin, NULL);
}

static Plugin *plugin_get_current(void)
{
    return g_static_private_get(&cur_plugin_key);
}

static void set_pvt_data(Plugin * plugin, gpointer data)
{
    mowgli_dictionary_elem_t *elem;

    elem = mowgli_dictionary_find(pvt_data_dict, g_path_get_basename(plugin->filename));
    if (elem == NULL)
        mowgli_dictionary_add(pvt_data_dict, g_path_get_basename(plugin->filename), data);
    else
        elem->data = data;
}

static gpointer get_pvt_data(void)
{
    Plugin *cur_p = plugin_get_current();

    return mowgli_dictionary_retrieve(pvt_data_dict, g_path_get_basename(cur_p->filename));
}

static gint
inputlist_compare_func(gconstpointer a, gconstpointer b)
{
    const InputPlugin *ap = a, *bp = b;
    if(ap->description && bp->description)
        return strcasecmp(ap->description, bp->description);
    else
        return 0;
}

static gint
outputlist_compare_func(gconstpointer a, gconstpointer b)
{
    const OutputPlugin *ap = a, *bp = b;
    return (bp->probe_priority - ap->probe_priority);
}

static gint
effectlist_compare_func(gconstpointer a, gconstpointer b)
{
    const EffectPlugin *ap = a, *bp = b;
    if(ap->description && bp->description)
        return strcasecmp(ap->description, bp->description);
    else
        return 0;
}

static gint
generallist_compare_func(gconstpointer a, gconstpointer b)
{
    const GeneralPlugin *ap = a, *bp = b;
    if(ap->description && bp->description)
        return strcasecmp(ap->description, bp->description);
    else
        return 0;
}

static gint
vislist_compare_func(gconstpointer a, gconstpointer b)
{
    const VisPlugin *ap = a, *bp = b;
    if(ap->description && bp->description)
        return strcasecmp(ap->description, bp->description);
    else
        return 0;
}

static gint
discoverylist_compare_func(gconstpointer a, gconstpointer b)
{
    const DiscoveryPlugin *ap = a, *bp = b;
    if(ap->description && bp->description)
        return strcasecmp(ap->description, bp->description);
    else
        return 0;
}

static gboolean
plugin_is_duplicate(const gchar * filename)
{
    gchar *base_filename = g_path_get_basename(filename);

    if (mowgli_dictionary_retrieve(plugin_dict, base_filename) != NULL)
    {
        g_free(base_filename);
        return TRUE;
    }

    g_free(base_filename);

    return FALSE;
}

gboolean
plugin_is_enabled(const gchar *filename)
{
    Plugin *plugin = mowgli_dictionary_retrieve(plugin_dict, filename);

    if (!plugin)
        return FALSE;

    return plugin->enabled;
}

void
plugin_set_enabled(const gchar *filename, gboolean enabled)
{
    Plugin *plugin = mowgli_dictionary_retrieve(plugin_dict, filename);

    if (!plugin)
        return;

    plugin->enabled = enabled;
}

Plugin *
plugin_get_plugin(const gchar *filename)
{
    return mowgli_dictionary_retrieve(plugin_dict, filename);
}

static void
input_plugin_init(Plugin * plugin)
{
    InputPlugin *p = INPUT_PLUGIN(plugin);

    p->set_info = playback_set_info;
    p->set_info_text = playback_set_title;

    ip_data.input_list = g_list_append(ip_data.input_list, p);

    p->enabled = TRUE;

    /* XXX: we need something better than p->filename if plugins
       will eventually provide multiple plugins --nenolod */
    mowgli_dictionary_add(plugin_dict, g_path_get_basename(p->filename), p);

    /* build the extension hash table */
    gint i;
    if(p->vfs_extensions) {
        for(i = 0; p->vfs_extensions[i] != NULL; i++) {
            GList *hdr = NULL;
            GList **handle = NULL; //allocated as auto in stack.
            GList **handle2 = g_malloc(sizeof(GList **));

            handle = g_hash_table_lookup(ext_hash, p->vfs_extensions[i]);
            if(handle)
                hdr = *handle;
            hdr = g_list_append(hdr, p); //returned hdr is non-volatile
            *handle2 = hdr;
            g_hash_table_replace(ext_hash, g_strdup(p->vfs_extensions[i]), handle2);
        }
    }
}

static void
output_plugin_init(Plugin * plugin)
{
    OutputPlugin *p = OUTPUT_PLUGIN(plugin);
    op_data.output_list = g_list_append(op_data.output_list, p);

    mowgli_dictionary_add(plugin_dict, g_path_get_basename(p->filename), p);
}

static void
effect_plugin_init(Plugin * plugin)
{
    EffectPlugin *p = EFFECT_PLUGIN(plugin);
    ep_data.effect_list = g_list_append(ep_data.effect_list, p);

    mowgli_dictionary_add(plugin_dict, g_path_get_basename(p->filename), p);
}

static void
general_plugin_init(Plugin * plugin)
{
    GeneralPlugin *p = GENERAL_PLUGIN(plugin);
    gp_data.general_list = g_list_append(gp_data.general_list, p);

    mowgli_dictionary_add(plugin_dict, g_path_get_basename(p->filename), p);
}

static void
vis_plugin_init(Plugin * plugin)
{
    VisPlugin *p = VIS_PLUGIN(plugin);
    p->disable_plugin = vis_disable_plugin;
    vp_data.vis_list = g_list_append(vp_data.vis_list, p);

    mowgli_dictionary_add(plugin_dict, g_path_get_basename(p->filename), p);
}

static void
discovery_plugin_init(Plugin * plugin)
{
    DiscoveryPlugin *p = DISCOVERY_PLUGIN(plugin);
    dp_data.discovery_list = g_list_append(dp_data.discovery_list, p);

    mowgli_dictionary_add(plugin_dict, g_path_get_basename(p->filename), p);
}

/*******************************************************************/

static void
plugin2_dispose(GModule *module, const gchar *str, ...)
{
    gchar *buf;
    va_list va;

    va_start(va, str);
    buf = g_strdup_vprintf(str, va);
    va_end(va);

    g_message("*** %s\n", buf);
    g_free(buf);

    g_module_close(module);
}

void
plugin2_process(PluginHeader *header, GModule *module, const gchar *filename)
{
    gint i, n;
    mowgli_node_t *hlist_node;

    if (header->magic != PLUGIN_MAGIC)
        return plugin2_dispose(module, "plugin <%s> discarded, invalid module magic", filename);

    if (header->api_version != __AUDACIOUS_PLUGIN_API__)
        return plugin2_dispose(module, "plugin <%s> discarded, wanting API version %d, we implement API version %d",
                               filename, header->api_version, __AUDACIOUS_PLUGIN_API__);

    hlist_node = mowgli_node_create();
    mowgli_node_add(header, hlist_node, headers_list);

    if (header->init)
        header->init();

    header->priv_assoc = g_new0(Plugin, 1);
    header->priv_assoc->handle = module;
    header->priv_assoc->filename = g_strdup(filename);

    n = 0;

    if (header->ip_list)
    {
        for (i = 0; (header->ip_list)[i] != NULL; i++, n++)
        {
            PLUGIN((header->ip_list)[i])->filename = g_strdup_printf("%s (#%d)", filename, n);
            input_plugin_init(PLUGIN((header->ip_list)[i]));
        }
    }

    if (header->op_list)
    {
        for (i = 0; (header->op_list)[i] != NULL; i++, n++)
        {
            PLUGIN((header->op_list)[i])->filename = g_strdup_printf("%s (#%d)", filename, n);
            output_plugin_init(PLUGIN((header->op_list)[i]));
        }
    }

    if (header->ep_list)
    {
        for (i = 0; (header->ep_list)[i] != NULL; i++, n++)
        {
            PLUGIN((header->ep_list)[i])->filename = g_strdup_printf("%s (#%d)", filename, n);
            effect_plugin_init(PLUGIN((header->ep_list)[i]));
        }
    }


    if (header->gp_list)
    {
        for (i = 0; (header->gp_list)[i] != NULL; i++, n++)
        {
            PLUGIN((header->gp_list)[i])->filename = g_strdup_printf("%s (#%d)", filename, n);
            general_plugin_init(PLUGIN((header->gp_list)[i]));
        }
    }

    if (header->vp_list)
    {
        for (i = 0; (header->vp_list)[i] != NULL; i++, n++)
        {
            PLUGIN((header->vp_list)[i])->filename = g_strdup_printf("%s (#%d)", filename, n);
            vis_plugin_init(PLUGIN((header->vp_list)[i]));
        }
    }

    if (header->dp_list)
    {
        for (i = 0; (header->dp_list)[i] != NULL; i++, n++)
        {
            PLUGIN((header->dp_list)[i])->filename = g_strdup_printf("%s (#%d)", filename, n);
            discovery_plugin_init(PLUGIN((header->dp_list)[i]));
        }
    }

    if (header->interface)
        interface_register(header->interface);
}

void
plugin2_unload(PluginHeader *header, mowgli_node_t *hlist_node)
{
    GModule *module;

    g_return_if_fail(header->priv_assoc != NULL);

    if (header->interface)
        interface_deregister(header->interface);

    module = header->priv_assoc->handle;

    g_free(header->priv_assoc->filename);
    g_free(header->priv_assoc);

    if (header->fini)
        header->fini();

    mowgli_node_delete(hlist_node, headers_list);
    mowgli_node_free(hlist_node);

    g_module_close(module);
}

/******************************************************************/

static void
add_plugin(const gchar * filename)
{
    GModule *module;
    gpointer func;

    if (plugin_is_duplicate(filename))
        return;

    g_message("Loaded plugin (%s)", filename);

    if (!(module = g_module_open(filename, G_MODULE_BIND_LAZY |
     G_MODULE_BIND_LOCAL)))
    {
        printf("Failed to load plugin (%s): %s\n",
                  filename, g_module_error());
        return;
    }

    /* v2 plugin loading */
    if (g_module_symbol(module, "get_plugin_info", &func))
    {
        PluginHeader *(*header_func_p)(struct _AudaciousFuncTableV1 *) = func;
        PluginHeader *header;

        /* this should never happen. */
        g_return_if_fail((header = header_func_p(&_aud_papi_v1)) != NULL);

        plugin2_process(header, module, filename);
        return;
    }

    printf("Invalid plugin (%s)\n", filename);
    g_module_close(module);
}

static gboolean
scan_plugin_func(const gchar * path, const gchar * basename, gpointer data)
{
    if (!str_has_suffix_nocase(basename, SHARED_SUFFIX))
        return FALSE;

    if (!g_file_test(path, G_FILE_TEST_IS_REGULAR))
        return FALSE;

    add_plugin(path);

    return FALSE;
}

static void
scan_plugins(const gchar * path)
{
    dir_foreach(path, scan_plugin_func, NULL, NULL);
}

void
plugin_system_init(void)
{
    gchar *dir, **disabled;
    GList *node;
    OutputPlugin *op;
    InputPlugin *ip;
    LowlevelPlugin *lp;
    DiscoveryPlugin *dp;
    GtkWidget *dialog;
    gint dirsel = 0, i = 0;
    gint prio;

    if (!g_module_supported()) {
        dialog =
            gtk_message_dialog_new (NULL,
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    _("Module loading not supported! Plugins will not be loaded.\n"));
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
        return;
    }

    plugin_dict = mowgli_dictionary_create(g_ascii_strcasecmp);
    pvt_data_dict = mowgli_dictionary_create(g_ascii_strcasecmp);

    headers_list = mowgli_list_create();

    /* make extension hash */
    ext_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

#ifndef DISABLE_USER_PLUGIN_DIR
    scan_plugins(aud_paths[BMP_PATH_USER_PLUGIN_DIR]);
    /*
     * This is in a separate lo
     * DiscoveryPlugin *dpop so if the user puts them in the
     * wrong dir we'll still get them in the right order (home dir
     * first)                                                - Zinx
     */
    while (plugin_dir_list[dirsel]) {
        dir = g_build_filename(aud_paths[BMP_PATH_USER_PLUGIN_DIR],
                               plugin_dir_list[dirsel++], NULL);
        scan_plugins(dir);
        g_free(dir);
    }
    dirsel = 0;
#endif

    while (plugin_dir_list[dirsel]) {
        dir = g_build_filename(PLUGIN_DIR, plugin_dir_list[dirsel++], NULL);
        scan_plugins(dir);
        g_free(dir);
    }

    op_data.output_list = g_list_sort(op_data.output_list, outputlist_compare_func);
    if (!op_data.current_output_plugin
        && g_list_length(op_data.output_list)) {
        op_data.current_output_plugin = op_data.output_list->data;
    }

    ip_data.input_list = g_list_sort(ip_data.input_list, inputlist_compare_func);

    ep_data.effect_list = g_list_sort(ep_data.effect_list, effectlist_compare_func);
    ep_data.enabled_list = NULL;

    gp_data.general_list = g_list_sort(gp_data.general_list, generallist_compare_func);
    gp_data.enabled_list = NULL;

    vp_data.vis_list = g_list_sort(vp_data.vis_list, vislist_compare_func);
    vp_data.enabled_list = NULL;

    dp_data.discovery_list = g_list_sort(dp_data.discovery_list, discoverylist_compare_func);
    dp_data.enabled_list = NULL;


    general_enable_from_stringified_list(cfg.enabled_gplugins);
    vis_enable_from_stringified_list(cfg.enabled_vplugins);
    effect_enable_from_stringified_list(cfg.enabled_eplugins);
    discovery_enable_from_stringified_list(cfg.enabled_dplugins);

    g_free(cfg.enabled_gplugins);
    cfg.enabled_gplugins = NULL;

    g_free(cfg.enabled_vplugins);
    cfg.enabled_vplugins = NULL;

    g_free(cfg.enabled_eplugins);
    cfg.enabled_eplugins = NULL;

    g_free(cfg.enabled_dplugins);
    cfg.enabled_dplugins = NULL;

    if (!cfg.outputplugin) {
        for (prio = 10; prio >= 0; prio--) {
            for (node = op_data.output_list; node; node = g_list_next(node)) {
                op = OUTPUT_PLUGIN(node->data);

                if (op->probe_priority != prio)
                    continue;

		if (op->init)
		{
		    OutputPluginInitStatus ret = op->init();
		    if (ret == OUTPUT_PLUGIN_INIT_NO_DEVICES)
		    {
		        g_message("Plugin %s reports no devices. Attempting to avert disaster, trying others.\n",
                                  g_path_get_basename(op->filename));
                    }
                    else if (ret == OUTPUT_PLUGIN_INIT_FAIL)
                    {
		        g_message("Plugin %s was unable to initialise. Attemping to avert disaster, trying others.\n",
		                  g_path_get_basename(op->filename));
                    }
                    else if (ret == OUTPUT_PLUGIN_INIT_FOUND_DEVICES)
                    {
                        if (!op_data.current_output_plugin)
                            op_data.current_output_plugin = op;
                    }
                    else if (!op_data.current_output_plugin)
                    {
		        g_message("Plugin %s did not report status, and no plugin has worked yet. Do you still need to convert it? Selecting for now...\n",
		                  g_path_get_basename(op->filename));

                        if (!op_data.current_output_plugin)
                            op_data.current_output_plugin = op;
                    }
		}
            }
        }
    }
    else
    {
        for (node = op_data.output_list; node; node = g_list_next(node)) {
            op = OUTPUT_PLUGIN(node->data);

            if (op->init) {
                plugin_set_current((Plugin *)op);
                op->init();
            }

            if (!g_ascii_strcasecmp(g_path_get_basename(cfg.outputplugin), g_path_get_basename(op->filename)))
                op_data.current_output_plugin = op;
        }
    }

    for (node = ip_data.input_list; node; node = g_list_next(node)) {
        ip = INPUT_PLUGIN(node->data);
        if (ip->init)
	{
	    plugin_set_current((Plugin *)ip);
            ip->init();
	}
    }

    for (node = dp_data.discovery_list; node; node = g_list_next(node)) {
        dp = DISCOVERY_PLUGIN(node->data);
        if (dp->init)
	{
	    plugin_set_current((Plugin *)dp);
            dp->init();
	}
    }


    for (node = lowlevel_list; node; node = g_list_next(node)) {
        lp = LOWLEVEL_PLUGIN(node->data);
        if (lp->init)
	{
	    plugin_set_current((Plugin *)lp);
            lp->init();
	}
    }

    if (cfg.disabled_iplugins) {
        disabled = g_strsplit(cfg.disabled_iplugins, ":", 0);

        while (disabled[i]) {
            Plugin *plugintmp = plugin_get_plugin(disabled[i]);
            g_free(disabled[i]);
            if (plugintmp)
                INPUT_PLUGIN(plugintmp)->enabled = FALSE;
            i++;
        }

        g_free(disabled);

        g_free(cfg.disabled_iplugins);
        cfg.disabled_iplugins = NULL;
    }
}

static void
remove_list(gpointer key, gpointer value, gpointer data)
{
    g_list_free(*(GList **)value);
}

void
plugin_system_cleanup(void)
{
    InputPlugin *ip;
    OutputPlugin *op;
    EffectPlugin *ep;
    GeneralPlugin *gp;
    VisPlugin *vp;
    LowlevelPlugin *lp;
    DiscoveryPlugin *dp;
    GList *node;
    mowgli_node_t *hlist_node;

    g_message("Shutting down plugin system");

    if (playback_get_playing()) {
        ip_data.stop = TRUE;
        playback_stop();
        ip_data.stop = FALSE;
    }

    /* FIXME: race condition -nenolod */
    op_data.current_output_plugin = NULL;

    for (node = get_input_list(); node; node = g_list_next(node)) {
        ip = INPUT_PLUGIN(node->data);
        if (ip) {
	    plugin_set_current((Plugin *)ip);

            if (ip->cleanup)
                ip->cleanup();

            GDK_THREADS_LEAVE();
            while (g_main_context_iteration(NULL, FALSE));
            GDK_THREADS_ENTER();
        }
    }

    if (ip_data.input_list != NULL)
    {
        g_list_free(ip_data.input_list);
        ip_data.input_list = NULL;
    }

    for (node = get_output_list(); node; node = g_list_next(node)) {
        op = OUTPUT_PLUGIN(node->data);
        if (op) {
	    plugin_set_current((Plugin *)op);

            if (op->cleanup)
                op->cleanup();

            GDK_THREADS_LEAVE();
            while (g_main_context_iteration(NULL, FALSE));
            GDK_THREADS_ENTER();
        }
    }

    if (op_data.output_list != NULL)
    {
        g_list_free(op_data.output_list);
        op_data.output_list = NULL;
    }

    for (node = get_effect_list(); node; node = g_list_next(node)) {
        ep = EFFECT_PLUGIN(node->data);
        if (ep) {
	    plugin_set_current((Plugin *)ep);

            if (ep->cleanup)
                ep->cleanup();

            GDK_THREADS_LEAVE();
            while (g_main_context_iteration(NULL, FALSE));
            GDK_THREADS_ENTER();
        }
    }

    if (ep_data.effect_list != NULL)
    {
        g_list_free(ep_data.effect_list);
        ep_data.effect_list = NULL;
    }

    for (node = get_general_list(); node; node = g_list_next(node)) {
        gp = GENERAL_PLUGIN(node->data);
        if (gp) {
	    plugin_set_current((Plugin *)gp);

            if (gp->cleanup)
                gp->cleanup();

            GDK_THREADS_LEAVE();
            while (g_main_context_iteration(NULL, FALSE));
            GDK_THREADS_ENTER();
        }
    }

    if (gp_data.general_list != NULL)
    {
        g_list_free(gp_data.general_list);
        gp_data.general_list = NULL;
    }

    for (node = get_vis_list(); node; node = g_list_next(node)) {
        vp = VIS_PLUGIN(node->data);
        if (vp) {
	    plugin_set_current((Plugin *)vp);

            if (vp->cleanup)
                vp->cleanup();

            GDK_THREADS_LEAVE();
            while (g_main_context_iteration(NULL, FALSE));
            GDK_THREADS_ENTER();
        }
    }

    if (vp_data.vis_list != NULL)
    {
        g_list_free(vp_data.vis_list);
        vp_data.vis_list = NULL;
    }


    for (node = get_discovery_list(); node; node = g_list_next(node)) {
        dp = DISCOVERY_PLUGIN(node->data);
        if (dp) {
	    plugin_set_current((Plugin *)dp);

            if (dp->cleanup)
                dp->cleanup();

            GDK_THREADS_LEAVE();
            while (g_main_context_iteration(NULL, FALSE));
            GDK_THREADS_ENTER();
        }
    }

    if (dp_data.discovery_list != NULL)
    {
        g_list_free(dp_data.discovery_list);
        dp_data.discovery_list = NULL;
    }

    for (node = lowlevel_list; node; node = g_list_next(node)) {
        lp = LOWLEVEL_PLUGIN(node->data);
        if (lp) {
	    plugin_set_current((Plugin *)lp);

            if (lp->cleanup)
                lp->cleanup();

            GDK_THREADS_LEAVE();
            while (g_main_context_iteration(NULL, FALSE));
            GDK_THREADS_ENTER();
        }
    }

    if (lowlevel_list != NULL)
    {
        g_list_free(lowlevel_list);
        lowlevel_list = NULL;
    }

    /* XXX: vfs will crash otherwise. -nenolod */
    if (vfs_transports != NULL)
    {
        g_list_free(vfs_transports);
        vfs_transports = NULL;
    }

    MOWGLI_LIST_FOREACH(hlist_node, headers_list->head)
    	plugin2_unload(hlist_node->data, hlist_node);

    mowgli_dictionary_destroy(plugin_dict, NULL, NULL);
    mowgli_dictionary_destroy(pvt_data_dict, NULL, NULL);

    mowgli_list_free(headers_list);

    g_hash_table_foreach(ext_hash, remove_list, NULL);
    g_hash_table_remove_all(ext_hash);
}
