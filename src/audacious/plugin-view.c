/*
 * plugin-view.c
 * Copyright 2010 John Lindgren
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

#include "plugin.h"
#include "plugins.h"
#include "ui_preferences.h"

enum {
 PVIEW_COL_NODE,
 PVIEW_COL_ENABLED,
 PVIEW_COL_NAME,
 PVIEW_COL_PATH,
 PVIEW_COLS
};

typedef struct {
    PluginHandle * p;
    GtkTreeModel * model;
    GtkTreePath * path;
} Node;

static PluginHandle * get_selected_plugin (GtkTreeView * tree)
{
    Node * n = NULL;

    GtkTreeSelection * sel = gtk_tree_view_get_selection (tree);
    GtkTreeModel * model;
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected (sel, & model, & iter))
        gtk_tree_model_get (model, & iter, PVIEW_COL_NODE, & n, -1);

    return n == NULL ? NULL : n->p;
}

static Plugin * get_selected_header (GtkTreeView * tree)
{
    PluginHandle * p = get_selected_plugin (tree);
    g_return_val_if_fail (p != NULL, NULL);
    g_return_val_if_fail (plugin_get_enabled (p), NULL);
    return plugin_get_header (p);
}

static void do_enable (GtkCellRendererToggle * cell, const gchar * path_str,
 GtkTreeModel * model)
{
    GtkTreePath * path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter;
    gtk_tree_model_get_iter (model, & iter, path);
    gtk_tree_path_free (path);

    Node * n = NULL;
    gboolean enabled;
    gtk_tree_model_get (model, & iter, PVIEW_COL_NODE, & n,
     PVIEW_COL_ENABLED, & enabled, -1);
    g_return_if_fail (n != NULL);

    plugin_enable (n->p, ! enabled);
}

static gboolean list_watcher (PluginHandle * p, Node * n)
{
    GtkTreeIter iter;
    gtk_tree_model_get_iter (n->model, & iter, n->path);
    gtk_list_store_set ((GtkListStore *) n->model, & iter, PVIEW_COL_ENABLED,
     plugin_get_enabled (n->p), -1);
    return TRUE;
}

static gboolean fill_cb (PluginHandle * p, GtkTreeModel * model)
{
    Node * n = g_slice_new (Node);

    GtkTreeIter iter;
    gtk_list_store_append ((GtkListStore *) model, & iter);
    gtk_list_store_set ((GtkListStore *) model, & iter, PVIEW_COL_NODE, n,
     PVIEW_COL_ENABLED, plugin_get_enabled (p), PVIEW_COL_NAME, plugin_get_name
     (p), PVIEW_COL_PATH, plugin_get_filename (p), -1);

    n->p = p;
    n->model = model;
    n->path = gtk_tree_model_get_path (model, & iter);

    plugin_add_watch (p, (PluginForEachFunc) list_watcher, n);

    return TRUE;
}

static void list_fill (GtkTreeView * tree, void * type)
{
    GtkTreeModel * model = (GtkTreeModel *) gtk_list_store_new (PVIEW_COLS,
     G_TYPE_POINTER, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model (tree, model);

    GtkTreeViewColumn * col = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
    gtk_tree_view_column_set_resizable (col, FALSE);
    gtk_tree_view_append_column (tree, col);

    GtkCellRenderer * rend = gtk_cell_renderer_toggle_new ();
    g_signal_connect (rend, "toggled", (GCallback) do_enable, model);
    gtk_tree_view_column_pack_start (col, rend, FALSE);
    gtk_tree_view_column_set_attributes (col, rend, "active", PVIEW_COL_ENABLED,
     NULL);

    for (gint i = PVIEW_COL_NAME; i <= PVIEW_COL_PATH; i ++)
    {
        col = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
        gtk_tree_view_column_set_resizable (col, FALSE);
        gtk_tree_view_append_column (tree, col);

        rend = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_pack_start (col, rend, FALSE);
        gtk_tree_view_column_set_attributes (col, rend, "text", i, NULL);
    }

    plugin_for_each (GPOINTER_TO_INT (type), (PluginForEachFunc) fill_cb, model);
}

static void list_destroy (GtkTreeView * tree)
{
    GtkTreeModel * model = gtk_tree_view_get_model (tree);
    if (model == NULL)
        return;

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first (model, & iter))
    {
        do
        {
            Node * n = NULL;
            gtk_tree_model_get (model, & iter, PVIEW_COL_NODE, & n, -1);
            g_return_if_fail (n != NULL);

            plugin_remove_watch (n->p, (PluginForEachFunc) list_watcher, n);
            gtk_tree_path_free (n->path);
            g_slice_free (Node, n);
        }
        while (gtk_tree_model_iter_next (model, & iter));
    }

    g_object_unref ((GObject *) model);
}

static gboolean config_watcher (PluginHandle * p, GtkWidget * config)
{
    gtk_widget_set_sensitive (config, plugin_has_configure (p) &&
     plugin_get_enabled (p));
    return TRUE;
}

static gboolean about_watcher (PluginHandle * p, GtkWidget * about)
{
    gtk_widget_set_sensitive (about, plugin_has_about (p) && plugin_get_enabled
     (p));
    return TRUE;
}

static void button_update (GtkTreeView * tree, GtkWidget * b)
{
    PluginForEachFunc watcher = g_object_get_data ((GObject *) b, "watcher");
    g_return_if_fail (watcher != NULL);

    PluginHandle * p = g_object_steal_data ((GObject *) b, "plugin");
    if (p != NULL)
        plugin_remove_watch (p, watcher, b);

    p = get_selected_plugin (tree);
    if (p != NULL)
    {
        g_object_set_data ((GObject *) b, "plugin", p);
        watcher (p, b);
        plugin_add_watch (p, watcher, b);
    }
    else
        gtk_widget_set_sensitive (b, FALSE);
}

static void do_config (GtkTreeView * tree)
{
    Plugin * header = get_selected_header (tree);
    g_return_if_fail (header != NULL);

    if (header->configure != NULL)
        header->configure ();
    else if (header->settings != NULL)
        plugin_preferences_show (header->settings);
}

static void do_about (GtkTreeView * tree)
{
    Plugin * header = get_selected_header (tree);
    g_return_if_fail (header != NULL);

    if (header->about != NULL)
        header->about ();
}

static void button_destroy (GtkWidget * b)
{
    PluginForEachFunc watcher = g_object_get_data ((GObject *) b, "watcher");
    g_return_if_fail (watcher != NULL);

    PluginHandle * p = g_object_steal_data ((GObject *) b, "plugin");
    if (p != NULL)
        plugin_remove_watch (p, watcher, b);
}

GtkWidget * plugin_view_new (gint type)
{
    GtkWidget * vbox = gtk_vbox_new (FALSE, 6);
    gtk_container_set_border_width ((GtkContainer *) vbox, 6);

    GtkWidget * scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_box_pack_start ((GtkBox *) vbox, scrolled, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy ((GtkScrolledWindow *) scrolled,
     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type ((GtkScrolledWindow *) scrolled,
     GTK_SHADOW_IN);

    GtkWidget * tree = gtk_tree_view_new ();
    gtk_container_add ((GtkContainer *) scrolled, tree);
    gtk_tree_view_set_headers_visible ((GtkTreeView *) tree, FALSE);
    g_signal_connect (tree, "realize", (GCallback) list_fill, GINT_TO_POINTER
     (type));
    g_signal_connect (tree, "destroy", (GCallback) list_destroy, NULL);

    GtkWidget * hbox = gtk_hbox_new (FALSE, 6);
    gtk_box_pack_start ((GtkBox *) vbox, hbox, FALSE, FALSE, 0);

    GtkWidget * config = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
    gtk_box_pack_start ((GtkBox *) hbox, config, FALSE, FALSE, 0);
    gtk_widget_set_sensitive (config, FALSE);
    g_object_set_data ((GObject *) config, "watcher", config_watcher);
    g_signal_connect (tree, "cursor-changed", (GCallback) button_update, config);
    g_signal_connect_swapped (config, "clicked", (GCallback)
     do_config, tree);
    g_signal_connect (config, "destroy", (GCallback) button_destroy, NULL);

    GtkWidget * about = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
    gtk_box_pack_start ((GtkBox *) hbox, about, FALSE, FALSE, 0);
    gtk_widget_set_sensitive (about, FALSE);
    g_object_set_data ((GObject *) about, "watcher", about_watcher);
    g_signal_connect (tree, "cursor-changed", (GCallback) button_update, about);
    g_signal_connect_swapped (about, "clicked", (GCallback) do_about, tree);
    g_signal_connect (about, "destroy", (GCallback) button_destroy, NULL);

    return vbox;
}
