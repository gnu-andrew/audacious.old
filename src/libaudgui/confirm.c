/*
 * libaudgui/confirm.c
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

#include <gtk/gtk.h>

#include <audacious/gtk-compat.h>
#include <audacious/i18n.h>
#include <audacious/misc.h>
#include <audacious/playlist.h>

#include "config.h"
#include "libaudgui-gtk.h"

static void no_confirm_cb (GtkToggleButton * toggle)
{
    aud_set_bool ("audgui", "no_confirm_playlist_delete", gtk_toggle_button_get_active (toggle));
}

static void confirm_delete_cb (GtkButton * button, void * data)
{
    gint list = aud_playlist_by_unique_id (GPOINTER_TO_INT (data));
    if (list < 0)
        return;

    aud_playlist_delete (list);
    if (list > 0)
        aud_playlist_set_active (list - 1);
}

void audgui_confirm_playlist_delete (gint playlist)
{
    GtkWidget * window, * vbox, * hbox, * label, * button;
    gchar * message;

    if (aud_get_bool ("audgui", "no_confirm_playlist_delete"))
    {
        aud_playlist_delete (playlist);
        if (playlist > 0)
            aud_playlist_set_active (playlist - 1);
        return;
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint ((GtkWindow *) window,
     GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_resizable ((GtkWindow *) window, FALSE);
    gtk_container_set_border_width ((GtkContainer *) window, 6);
    audgui_destroy_on_escape (window);

    vbox = gtk_vbox_new (FALSE, 6);
    gtk_container_add ((GtkContainer *) window, vbox);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start ((GtkBox *) vbox, hbox, FALSE, FALSE, 0);

    gtk_box_pack_start ((GtkBox *) hbox, gtk_image_new_from_stock
     (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG), FALSE, FALSE, 0);

    gchar * title = aud_playlist_get_title (playlist);
    message = g_strdup_printf (_("Are you sure you want to close %s?  If you "
     "do, any changes made since the playlist was exported will be lost."),
     title);
    g_free (title);
    label = gtk_label_new (message);
    g_free (message);
    gtk_label_set_line_wrap ((GtkLabel *) label, TRUE);
    gtk_widget_set_size_request (label, 320, -1);
    gtk_box_pack_start ((GtkBox *) hbox, label, TRUE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start ((GtkBox *) vbox, hbox, FALSE, FALSE, 0);

    button = gtk_check_button_new_with_mnemonic (_("_Don't show this message again"));
    gtk_box_pack_start ((GtkBox *) hbox, button, FALSE, FALSE, 0);
    g_signal_connect (button, "toggled", (GCallback) no_confirm_cb, NULL);

    hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start ((GtkBox *) vbox, hbox, FALSE, FALSE, 0);

    button = gtk_button_new_from_stock (GTK_STOCK_NO);
    gtk_box_pack_end ((GtkBox *) hbox, button, FALSE, FALSE, 0);
    g_signal_connect_swapped (button, "clicked", (GCallback)
     gtk_widget_destroy, window);

    button = gtk_button_new_from_stock (GTK_STOCK_YES);
    gtk_box_pack_end ((GtkBox *) hbox, button, FALSE, FALSE, 0);
    gtk_widget_set_can_default (button, TRUE);
    gtk_widget_grab_default (button);
    gtk_widget_grab_focus (button);
    g_signal_connect ((GObject *) button, "clicked", (GCallback)
     confirm_delete_cb, GINT_TO_POINTER (aud_playlist_get_unique_id (playlist)));
    g_signal_connect_swapped ((GObject *) button, "clicked", (GCallback)
     gtk_widget_destroy, window);

    gtk_widget_show_all (window);
}

static void rename_cb (GtkDialog * dialog, gint resp, void * list)
{
    if (resp == GTK_RESPONSE_ACCEPT && GPOINTER_TO_INT (list) <
     aud_playlist_count ())
        aud_playlist_set_title (GPOINTER_TO_INT (list), gtk_entry_get_text
         ((GtkEntry *) g_object_get_data ((GObject *) dialog, "entry")));

    gtk_widget_destroy ((GtkWidget *) dialog);
}

void audgui_show_playlist_rename (gint playlist)
{
    GtkWidget * dialog = gtk_dialog_new_with_buttons (_("Rename Playlist"),
     NULL, 0, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL,
     GTK_RESPONSE_REJECT, NULL);
    gtk_dialog_set_default_response ((GtkDialog *) dialog, GTK_RESPONSE_ACCEPT);

    GtkWidget * entry = gtk_entry_new ();
    gchar * title = aud_playlist_get_title (playlist);
    gtk_entry_set_text ((GtkEntry *) entry, title);
    g_free (title);
    gtk_entry_set_activates_default ((GtkEntry *) entry, TRUE);
    gtk_box_pack_start ((GtkBox *) gtk_dialog_get_content_area ((GtkDialog *)
     dialog), entry, FALSE, FALSE, 0);
    g_object_set_data ((GObject *) dialog, "entry", entry);

    g_signal_connect (dialog, "response", (GCallback) rename_cb, GINT_TO_POINTER
     (playlist));
    gtk_widget_show_all (dialog);
}
