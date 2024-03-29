/*
 * misc-api.h
 * Copyright 2010-2011 John Lindgren
 *
 * This file is part of Audacious.
 *
 * Audacious is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 or version 3 of the License.
 *
 * Audacious is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Audacious. If not, see <http://www.gnu.org/licenses/>.
 *
 * The Audacious team does not consider modular code linking to Audacious or
 * using our public API to be a derived work.
 */

/* Do not include this file directly; use misc.h instead. */

/* CAUTION: Many of these functions are not thread safe. */

/* config.c */

AUD_FUNC1 (void, config_clear_section, const gchar *, section)
AUD_FUNC2 (void, config_set_defaults, const gchar *, section, const gchar * const *, entries)

AUD_FUNC3 (void, set_string, const gchar *, section, const gchar *, name, const gchar *, value)
AUD_FUNC2 (gchar *, get_string, const gchar *, section, const gchar *, name)
AUD_FUNC3 (void, set_bool, const gchar *, section, const gchar *, name, gboolean, value)
AUD_FUNC2 (gboolean, get_bool, const gchar *, section, const gchar *, name)
AUD_FUNC3 (void, set_int, const gchar *, section, const gchar *, name, gint, value)
AUD_FUNC2 (gint, get_int, const gchar *, section, const gchar *, name)
AUD_FUNC3 (void, set_double, const gchar *, section, const gchar *, name, gdouble, value)
AUD_FUNC2 (gdouble, get_double, const gchar *, section, const gchar *, name)

/* credits.c */
AUD_FUNC3 (void, get_audacious_credits, const gchar * *, brief,
 const gchar * * *, credits, const gchar * * *, translators)

/* equalizer.c */
AUD_FUNC1 (void, eq_set_bands, const gdouble *, values)
AUD_FUNC1 (void, eq_get_bands, gdouble *, values)
AUD_FUNC2 (void, eq_set_band, gint, band, gdouble, value)
AUD_FUNC1 (gdouble, eq_get_band, gint, band)

/* equalizer_preset.c */
AUD_FUNC1 (GList *, equalizer_read_presets, const gchar *, basename)
AUD_FUNC2 (gboolean, equalizer_write_preset_file, GList *, list, const gchar *,
 basename)
AUD_FUNC1 (EqualizerPreset *, load_preset_file, const gchar *, filename)
AUD_FUNC2 (gboolean, save_preset_file, EqualizerPreset *, preset, const gchar *,
 filename)
AUD_FUNC1 (GList *, import_winamp_eqf, VFSFile *, file)

/* history.c */
AUD_FUNC1 (const gchar *, history_get, gint, entry)
AUD_FUNC1 (void, history_add, const gchar *, path)

/* main.c */
AUD_FUNC1 (const gchar *, get_path, gint, path)

/* probe.c */
AUD_FUNC2 (PluginHandle *, file_find_decoder, const gchar *, filename, gboolean,
 fast)
AUD_FUNC2 (Tuple *, file_read_tuple, const gchar *, filename, PluginHandle *,
 decoder)
AUD_FUNC4 (gboolean, file_read_image, const gchar *, filename, PluginHandle *,
 decoder, void * *, data, gint *, size)
AUD_FUNC2 (gboolean, file_can_write_tuple, const gchar *, filename,
 PluginHandle *, decoder)
AUD_FUNC3 (gboolean, file_write_tuple, const gchar *, filename, PluginHandle *,
 decoder, const Tuple *, tuple)
AUD_FUNC2 (gboolean, custom_infowin, const gchar *, filename, PluginHandle *,
 decoder)

/* ui_albumart.c */
AUD_FUNC1 (gchar *, get_associated_image_file, const gchar *, filename)

/* ui_plugin_menu.c */
AUD_FUNC1 (/* GtkWidget * */ void *, get_plugin_menu, gint, id)
AUD_FUNC4 (void, plugin_menu_add, gint, id, MenuFunc, func, const gchar *, name,
 const gchar *, icon)
AUD_FUNC2 (void, plugin_menu_remove, gint, id, MenuFunc, func)

/* ui_preferences.c */
AUD_FUNC4 (void, create_widgets_with_domain, /* GtkWidget * */ void *, box,
 PreferencesWidget *, widgets, gint, count, const gchar *, domain)
AUD_FUNC0 (void, show_prefs_window)

/* util.c */
AUD_FUNC2 (gchar *, construct_uri, const gchar *, base, const gchar *, reference)

/* vis_runner.c */
AUD_FUNC2 (void, vis_runner_add_hook, VisHookFunc, func, void *, user)
AUD_FUNC1 (void, vis_runner_remove_hook, VisHookFunc, func)

/* visualization.c */
AUD_FUNC3 (void, calc_mono_freq, VisFreqData, buffer, const VisPCMData, data,
 gint, channels)
AUD_FUNC3 (void, calc_mono_pcm, VisPCMData, buffer, const VisPCMData, data,
 gint, channels)
AUD_FUNC3 (void, calc_stereo_pcm, VisPCMData, buffer, const VisPCMData, data,
 gint, channels)

/* New in 2.5-alpha2 */
AUD_FUNC1 (void, interface_install_toolbar, void *, button)
AUD_FUNC1 (void, interface_uninstall_toolbar, void *, button)
