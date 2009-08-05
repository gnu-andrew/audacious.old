/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2009  Audacious development team.
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

#ifndef LIBAUDGUI_H
#define LIBAUDGUI_H

#include <glib.h>

/* macro defines for Audacious stock icons */
#define AUD_STOCK_PLAYLIST    "aud-playlist"
#define AUD_STOCK_PLUGIN      "aud-plugin"
#define AUD_STOCK_QUEUETOGGLE "aud-queuetoggle"
#define AUD_STOCK_RANDOMIZEPL "aud-randomizepl"

void audgui_register_stock_icons(void);

void audgui_show_add_url_window(void);

void audgui_jump_to_track(void);
void audgui_jump_to_track_hide(void);

void audgui_set_default_icon(void);

void audgui_run_filebrowser(gboolean clear_pl_on_ok);
void audgui_hide_filebrowser(void);

void audgui_show_about_window(void);
void audgui_hide_about_window(void);

#endif /* LIBAUDGUI_H */
