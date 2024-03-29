/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2011  Audacious development team.
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

#include <gtk/gtk.h>

#include <audacious/gtk-compat.h>
#include <audacious/i18n.h>
#include <audacious/misc.h>
#include <audacious/playlist.h>
#include <libaudcore/hook.h>

#include "config.h"
#include "libaudgui.h"
#include "libaudgui-gtk.h"
#include "list.h"

static GtkWidget * playman_win = NULL;

static void save_position (GtkWidget * window)
{
    gint x, y, w, h;
    gtk_window_get_position ((GtkWindow *) window, & x, & y);
    gtk_window_get_size ((GtkWindow *) window, & w, & h);

    aud_set_int ("audgui", "playlist_manager_x", x);
    aud_set_int ("audgui", "playlist_manager_y", y);
    aud_set_int ("audgui", "playlist_manager_w", w);
    aud_set_int ("audgui", "playlist_manager_h", h);
}

static gboolean hide_cb (GtkWidget * window)
{
    save_position (window);
    gtk_widget_hide (window);
    return TRUE;
}

static void rename_cb (void)
{
    audgui_show_playlist_rename (aud_playlist_get_active ());
}

static void new_cb (GtkButton * button, void * unused)
{
    aud_playlist_insert (aud_playlist_get_active () + 1);
    aud_playlist_set_active (aud_playlist_get_active () + 1);
}

static void delete_cb (GtkButton * button, GtkWidget * list)
{
    audgui_confirm_playlist_delete (aud_playlist_get_active ());
}

static void save_config_cb (void * hook_data, void * user_data)
{
    if (gtk_widget_get_visible ((GtkWidget *) user_data))
        save_position ((GtkWidget *) user_data);
}

static void get_value (void * user, gint row, gint column, GValue * value)
{
    switch (column)
    {
    case 0:
        g_value_take_string (value, aud_playlist_get_title (row));
        break;
    case 1:
        g_value_set_int (value, aud_playlist_entry_count (row));
        break;
    }
}

static gboolean get_selected (void * user, gint row)
{
    return (row == aud_playlist_get_active ());
}

static void set_selected (void * user, gint row, gboolean selected)
{
    if (selected)
        aud_playlist_set_active (row);
}

static void select_all (void * user, gboolean selected)
{
}

static void activate_row (void * user, gint row)
{
    aud_playlist_set_active (row);

    if (aud_get_bool ("audgui", "playlist_manager_close_on_activate"))
        hide_cb (playman_win);
}

static void shift_rows (void * user, gint row, gint before)
{
    if (before < row)
        aud_playlist_reorder (row, before, 1);
    else if (before - 1 > row)
        aud_playlist_reorder (row, before - 1, 1);
}

static const AudguiListCallbacks callbacks = {
 .get_value = get_value,
 .get_selected = get_selected,
 .set_selected = set_selected,
 .select_all = select_all,
 .activate_row = activate_row,
 .right_click = NULL,
 .shift_rows = shift_rows,
 .data_type = NULL,
 .get_data = NULL,
 .receive_data = NULL};

static gboolean search_cb (GtkTreeModel * model, gint column, const gchar * key,
 GtkTreeIter * iter, void * user)
{
    GtkTreePath * path = gtk_tree_model_get_path (model, iter);
    g_return_val_if_fail (path, TRUE);
    gint row = gtk_tree_path_get_indices (path)[0];
    gtk_tree_path_free (path);

    gchar * temp = aud_playlist_get_title (row);
    g_return_val_if_fail (temp, TRUE);
    gchar * title = g_utf8_strdown (temp, -1);
    g_free (temp);

    temp = g_utf8_strdown (key, -1);
    gchar * * keys = g_strsplit (temp, " ", 0);
    g_free (temp);

    gboolean match = FALSE;

    for (gint i = 0; keys[i]; i ++)
    {
        if (! keys[i][0])
            continue;

        if (strstr (title, keys[i]))
            match = TRUE;
        else
        {
            match = FALSE;
            break;
        }
    }

    g_free (title);
    g_strfreev (keys);

    return ! match; /* TRUE == not matched, FALSE == matched */
}

static gboolean position_changed = FALSE;
static gboolean playlist_activated = FALSE;

static void update_hook (void * data, void * list)
{
    gint rows = aud_playlist_count ();

    if (GPOINTER_TO_INT (data) == PLAYLIST_UPDATE_STRUCTURE)
    {
        gint old_rows = audgui_list_row_count (list);

        if (rows < old_rows)
            audgui_list_delete_rows (list, rows, old_rows - rows);
        else if (rows > old_rows)
            audgui_list_insert_rows (list, old_rows, rows - old_rows);

        position_changed = TRUE;
        playlist_activated = TRUE;
    }

    if (GPOINTER_TO_INT (data) >= PLAYLIST_UPDATE_METADATA)
        audgui_list_update_rows (list, 0, rows);

    if (playlist_activated)
    {
        audgui_list_set_focus (list, aud_playlist_get_active ());
        audgui_list_update_selection (list, 0, rows);
        playlist_activated = FALSE;
    }

    if (position_changed)
    {
        audgui_list_set_highlight (list, aud_playlist_get_playing ());
        position_changed = FALSE;
    }
}

static void activate_hook (void * data, void * list)
{
    if (aud_playlist_update_pending ())
        playlist_activated = TRUE;
    else
    {
        audgui_list_set_focus (list, aud_playlist_get_active ());
        audgui_list_update_selection (list, 0, aud_playlist_count ());
    }
}

static void position_hook (void * data, void * list)
{
    if (aud_playlist_update_pending ())
        position_changed = TRUE;
    else
        audgui_list_set_highlight (list, aud_playlist_get_playing ());
}

static void close_on_activate_cb (GtkToggleButton * toggle)
{
    aud_set_bool ("audgui", "playlist_manager_close_on_activate",
     gtk_toggle_button_get_active (toggle));
}

void audgui_playlist_manager (void)
{
    GtkWidget *playman_vbox;
    GtkWidget * playman_pl_lv, * playman_pl_lv_sw;
    GtkWidget *playman_bbar_hbbox;
    GtkWidget * rename_button, * new_button, * delete_button;
    GtkWidget * hbox, * button;
    GdkGeometry playman_win_hints;

    if ( playman_win != NULL )
    {
        gtk_window_present( GTK_WINDOW(playman_win) );
        return;
    }

    playman_win = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_type_hint( GTK_WINDOW(playman_win), GDK_WINDOW_TYPE_HINT_DIALOG );
    gtk_window_set_title( GTK_WINDOW(playman_win), _("Playlist Manager") );
    gtk_container_set_border_width ((GtkContainer *) playman_win, 6);
    playman_win_hints.min_width = 400;
    playman_win_hints.min_height = 250;
    gtk_window_set_geometry_hints( GTK_WINDOW(playman_win) , GTK_WIDGET(playman_win) ,
                                   &playman_win_hints , GDK_HINT_MIN_SIZE );

    gint x = aud_get_int ("audgui", "playlist_manager_x");
    gint y = aud_get_int ("audgui", "playlist_manager_y");
    gint w = aud_get_int ("audgui", "playlist_manager_w");
    gint h = aud_get_int ("audgui", "playlist_manager_h");

    if (w && h)
    {
        gtk_window_move ((GtkWindow *) playman_win, x, y);
        gtk_window_set_default_size ((GtkWindow *) playman_win, w, h);
    }

    g_signal_connect ((GObject *) playman_win, "delete-event", (GCallback)
     hide_cb, NULL);
    audgui_hide_on_escape (playman_win);

    playman_vbox = gtk_vbox_new (FALSE, 6);
    gtk_container_add( GTK_CONTAINER(playman_win) , playman_vbox );

    playman_pl_lv = audgui_list_new (& callbacks, NULL, aud_playlist_count ());
    audgui_list_add_column (playman_pl_lv, _("Title"), 0, G_TYPE_STRING, TRUE);
    audgui_list_add_column (playman_pl_lv, _("Entries"), 1, G_TYPE_INT, FALSE);
    audgui_list_set_highlight (playman_pl_lv, aud_playlist_get_playing ());
    gtk_tree_view_set_search_equal_func ((GtkTreeView *) playman_pl_lv,
     search_cb, NULL, NULL);
    hook_associate ("playlist update", update_hook, playman_pl_lv);
    hook_associate ("playlist activate", activate_hook, playman_pl_lv);
    hook_associate ("playlist position", position_hook, playman_pl_lv);

    playman_pl_lv_sw = gtk_scrolled_window_new( NULL , NULL );
    gtk_scrolled_window_set_shadow_type ((GtkScrolledWindow *) playman_pl_lv_sw,
     GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy ((GtkScrolledWindow *) playman_pl_lv_sw,
     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add( GTK_CONTAINER(playman_pl_lv_sw) , playman_pl_lv );
    gtk_box_pack_start ((GtkBox *) playman_vbox, playman_pl_lv_sw, TRUE, TRUE, 0);

    /* button bar */
    playman_bbar_hbbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout( GTK_BUTTON_BOX(playman_bbar_hbbox) , GTK_BUTTONBOX_END );
    gtk_box_set_spacing ((GtkBox *) playman_bbar_hbbox, 6);

    rename_button = gtk_button_new_with_mnemonic (_("_Rename"));
    gtk_button_set_image ((GtkButton *) rename_button, gtk_image_new_from_stock
     (GTK_STOCK_EDIT, GTK_ICON_SIZE_BUTTON));
    new_button = gtk_button_new_from_stock (GTK_STOCK_NEW);
    delete_button = gtk_button_new_from_stock (GTK_STOCK_DELETE);

    gtk_container_add ((GtkContainer *) playman_bbar_hbbox, rename_button);
    gtk_button_box_set_child_secondary ((GtkButtonBox *) playman_bbar_hbbox,
     rename_button, TRUE);
    gtk_container_add ((GtkContainer *) playman_bbar_hbbox, new_button);
    gtk_container_add ((GtkContainer *) playman_bbar_hbbox, delete_button);

    gtk_box_pack_start( GTK_BOX(playman_vbox) , playman_bbar_hbbox , FALSE , FALSE , 0 );

    g_signal_connect ((GObject *) rename_button, "clicked", (GCallback)
     rename_cb, playman_pl_lv);
    g_signal_connect ((GObject *) new_button, "clicked", (GCallback) new_cb,
     playman_pl_lv);
    g_signal_connect ((GObject *) delete_button, "clicked", (GCallback)
     delete_cb, playman_pl_lv);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start ((GtkBox *) playman_vbox, hbox, FALSE, FALSE, 0);
    button = gtk_check_button_new_with_mnemonic
     (_("_Close dialog on activating playlist"));
    gtk_box_pack_start ((GtkBox *) hbox, button, FALSE, FALSE, 0);
    gtk_toggle_button_set_active ((GtkToggleButton *) button, aud_get_bool
     ("audgui", "playlist_manager_close_on_activate"));
    g_signal_connect (button, "toggled", (GCallback) close_on_activate_cb, NULL);

    gtk_widget_show_all( playman_win );

    hook_associate ("config save", save_config_cb, playman_win);
}
