/*
 * Audacious2
 * Copyright (c) 2008 William Pitcock <nenolod@dereferenced.org>
 * Copyright (c) 2008-2009 Tomasz Moń <desowin@gmail.com>
 * Copyright (c) 2011 John Lindgren <john.lindgren@tds.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; under version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses>.
 *
 * The Audacious team does not consider modular code linking to
 * Audacious or using our public API to be a derived work.
 */

#ifndef __AUDACIOUS2_INTERFACE_H__
#define __AUDACIOUS2_INTERFACE_H__

#include <gtk/gtk.h>
#include <audacious/plugins.h>

gboolean interface_load (PluginHandle * plugin);
void interface_unload (void);

/* These functions have to be called from main thread
   Use event_queue if you need to call those from other threads */

void interface_show (gboolean show);
gboolean interface_is_shown (void);
gboolean interface_is_focused (void);

void interface_show_error (const gchar * markup);
void interface_show_filebrowser (gboolean play_button);
void interface_show_jump_to_track (void);

void interface_add_plugin_widget (PluginHandle * plugin, GtkWidget * widget);
void interface_remove_plugin_widget (PluginHandle * plugin, GtkWidget * widget);

void register_interface_hooks (void);

PluginHandle * iface_plugin_probe (void);
PluginHandle * iface_plugin_get_current (void);
gboolean iface_plugin_set_current (PluginHandle * plugin);

#endif
