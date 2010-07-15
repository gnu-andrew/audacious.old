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

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>

#include <mowgli.h>

#include "input.h"
#include "output.h"
#include "plugin-registry.h"

static gboolean plugin_list_func (PluginHandle * plugin, GList * * list)
{
    InputPlugin * decoder = plugin_get_header (plugin);
    g_return_val_if_fail (decoder != NULL, TRUE);
    * list = g_list_prepend (* list, decoder);
    return TRUE;
}

GList * get_input_list (void)
{
    static GList * list = NULL;

    if (list == NULL)
    {
        plugin_for_each (PLUGIN_TYPE_INPUT, (PluginForEachFunc)
         plugin_list_func, & list);
        list = g_list_reverse (list);
    }

    return list;
}

void
input_get_volume(gint * l, gint * r)
{
    if (current_playback && current_playback->plugin->get_volume &&
     current_playback->plugin->get_volume (l, r))
        return;

    output_get_volume (l, r);
}

void
input_set_volume(gint l, gint r)
{
    gint h_vol[2] = {l, r};

    hook_call("volume set", h_vol);

    if (current_playback && current_playback->plugin->set_volume &&
     current_playback->plugin->set_volume (l, r))
        return;

    output_set_volume (l, r);
}
