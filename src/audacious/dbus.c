/*
 * Audacious: A cross-platform multimedia player
 * Copyright (c) 2007 Ben Tucker
 * Copyright 2009-2011 Audacious development team
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

#include "config.h"

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>
#include "dbus.h"
#include "dbus-service.h"
#include "dbus-server-bindings.h"

#include <math.h>
#include <libaudcore/eventqueue.h>

#include "debug.h"
#include "drct.h"
#include "playback.h"
#include "playlist.h"
#include "interface.h"
#include "misc.h"

struct StatusRequest
{
    gboolean playing, paused;
    gint time, length;
    gint bitrate, samplerate, channels;
};

struct PositionRequest
{
    gint playlist;              /* -1 = active, -2 = playing */
    gint entry;                 /* -1 = current */
    gint entry_count, queue_count;
};

struct InfoRequest
{
    gint playlist;              /* -1 = active, -2 = playing */
    gint entry;                 /* -1 = current */
    gchar *filename, *title, *pltitle;
    gint length;
};

struct FieldRequest
{
    gint playlist;              /* -1 = active, -2 = playing */
    gint entry;                 /* -1 = current */
    const gchar *field;
    GValue *value;
};

struct AddRequest
{
    gint position;              /* -1 = at end */
    gchar *filename;
    gboolean play;
};

struct MprisMetadataRequest
{
    gint playlist;              /* -1 = active, -2 = playing */
    gint entry;                 /* -1 = current */
    GHashTable *metadata;
};

static DBusGConnection *dbus_conn = NULL;
static guint signals[LAST_SIG] = { 0 };
static guint tracklist_signals[LAST_TRACKLIST_SIG] = { 0 };

MprisPlayer * mpris = NULL;

static GThread *main_thread;
static GMutex *info_mutex;
static GCond *info_cond;

G_DEFINE_TYPE (RemoteObject, audacious_rc, G_TYPE_OBJECT)
G_DEFINE_TYPE (MprisRoot, mpris_root, G_TYPE_OBJECT)
G_DEFINE_TYPE (MprisPlayer, mpris_player, G_TYPE_OBJECT)
G_DEFINE_TYPE (MprisTrackList, mpris_tracklist, G_TYPE_OBJECT)

#define DBUS_TYPE_G_STRING_VALUE_HASHTABLE (dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE))

static void mpris_playlist_update_hook(gpointer unused, MprisTrackList *obj);

void audacious_rc_class_init(RemoteObjectClass * klass)
{
}

void mpris_root_class_init(MprisRootClass * klass)
{
}

void mpris_player_class_init(MprisPlayerClass * klass)
{
    signals[CAPS_CHANGE_SIG] = g_signal_new("caps_change", G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
    signals[TRACK_CHANGE_SIG] =
        g_signal_new("track_change",
                     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL, g_cclosure_marshal_VOID__BOXED, G_TYPE_NONE, 1, DBUS_TYPE_G_STRING_VALUE_HASHTABLE);
    signals[STATUS_CHANGE_SIG] =
        g_signal_new("status_change", G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_VALUE_ARRAY);
}

void mpris_tracklist_class_init(MprisTrackListClass * klass)
{
    tracklist_signals[TRACKLIST_CHANGE_SIG] = g_signal_new("track_list_change", G_OBJECT_CLASS_TYPE(klass),
	G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

void audacious_rc_init(RemoteObject * object)
{
    GError *error = NULL;
    DBusGProxy *driver_proxy;
    guint request_ret;

    AUDDBG ("Registering remote D-Bus interfaces.\n");

    dbus_g_object_type_install_info(audacious_rc_get_type(), &dbus_glib_audacious_rc_object_info);

    // Register DBUS path
    dbus_g_connection_register_g_object(dbus_conn, AUDACIOUS_DBUS_PATH, G_OBJECT(object));

    // Register the service name, the constants here are defined in
    // dbus-glib-bindings.h
    driver_proxy = dbus_g_proxy_new_for_name(dbus_conn, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

    if (!org_freedesktop_DBus_request_name(driver_proxy, AUDACIOUS_DBUS_SERVICE, 0, &request_ret, &error))
    {
        g_warning("Unable to register service: %s", error->message);
        g_error_free(error);
    }

    if (!org_freedesktop_DBus_request_name(driver_proxy, AUDACIOUS_DBUS_SERVICE_MPRIS, 0, &request_ret, &error))
    {
        g_warning("Unable to register service: %s", error->message);
        g_error_free(error);
    }

    g_object_unref(driver_proxy);
}

void mpris_root_init(MprisRoot * object)
{
    dbus_g_object_type_install_info(mpris_root_get_type(), &dbus_glib_mpris_root_object_info);

    // Register DBUS path
    dbus_g_connection_register_g_object(dbus_conn, AUDACIOUS_DBUS_PATH_MPRIS_ROOT, G_OBJECT(object));
}

void mpris_player_init(MprisPlayer * object)
{
    dbus_g_object_type_install_info(mpris_player_get_type(), &dbus_glib_mpris_player_object_info);

    // Register DBUS path
    dbus_g_connection_register_g_object(dbus_conn, AUDACIOUS_DBUS_PATH_MPRIS_PLAYER, G_OBJECT(object));

    // Add signals
    DBusGProxy *proxy = object->proxy;
    if (proxy != NULL)
    {
        dbus_g_proxy_add_signal (proxy, "StatusChange", dbus_g_type_get_struct
         ("GValueArray", G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
         G_TYPE_INVALID), G_TYPE_INVALID);
        dbus_g_proxy_add_signal (proxy, "CapsChange", G_TYPE_INT, G_TYPE_INVALID);
        dbus_g_proxy_add_signal(proxy, "TrackChange", DBUS_TYPE_G_STRING_VALUE_HASHTABLE, G_TYPE_INVALID);
    }
    else
    {
        /* XXX / FIXME: Why does this happen? -- ccr */
        AUDDBG ("object->proxy == NULL; not adding some signals.\n");
    }
}

void mpris_tracklist_init(MprisTrackList * object)
{
    dbus_g_object_type_install_info(mpris_tracklist_get_type(), &dbus_glib_mpris_tracklist_object_info);

    // Register DBUS path
    dbus_g_connection_register_g_object(dbus_conn, AUDACIOUS_DBUS_PATH_MPRIS_TRACKLIST, G_OBJECT(object));

    // Add signals
    DBusGProxy *proxy = object->proxy;
    if (proxy != NULL)
    {
        dbus_g_proxy_add_signal(proxy, "TrackListChange", G_TYPE_INT, G_TYPE_INVALID);
    }
    else
    {
        /* XXX / FIXME: Why does this happen? -- ccr */
        AUDDBG ("object->proxy == NULL, not adding some signals.\n");
    }

    hook_associate("playlist update", (HookFunction) mpris_playlist_update_hook, object);
}

void init_dbus()
{
    GError *error = NULL;
    DBusConnection *local_conn;

    main_thread = g_thread_self();
    info_mutex = g_mutex_new();
    info_cond = g_cond_new();

    AUDDBG ("Trying to initialize D-Bus.\n");
    dbus_conn = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (dbus_conn == NULL)
    {
        g_warning("Unable to connect to dbus: %s", error->message);
        g_error_free(error);
        return;
    }

    g_type_init();
    g_object_new(audacious_rc_get_type(), NULL);
    g_object_new(mpris_root_get_type(), NULL);
    mpris = g_object_new(mpris_player_get_type(), NULL);
    g_object_new(mpris_tracklist_get_type(), NULL);

    local_conn = dbus_g_connection_get_connection(dbus_conn);
    dbus_connection_set_exit_on_disconnect(local_conn, FALSE);
}

static GValue *tuple_value_to_gvalue(const Tuple * tuple, const gchar * key)
{
    GValue *val;
    TupleValueType type = tuple_get_value_type((Tuple *) tuple, -1, key);

    if (type == TUPLE_STRING)
    {
        val = g_new0(GValue, 1);
        g_value_init(val, G_TYPE_STRING);
        g_value_take_string(val, g_strdup(tuple_get_string((Tuple *) tuple, -1, key)));
        return val;
    }
    else if (type == TUPLE_INT)
    {
        val = g_new0(GValue, 1);
        g_value_init(val, G_TYPE_INT);
        g_value_set_int(val, tuple_get_int((Tuple *) tuple, -1, key));
        return val;
    }
    return NULL;
}

/**
 * Retrieves value named tuple_key and inserts it inside hash table.
 *
 * @param[in,out] md GHashTable to insert into
 * @param[in] tuple Tuple to read data from
 * @param[in] tuple_key Tuple field key
 * @param[in] key key used for inserting into hash table.
 */
static void tuple_insert_to_hash_full(GHashTable * md, const Tuple * tuple,
                                      const gchar * tuple_key, const gchar *key)
{
    GValue *value = tuple_value_to_gvalue(tuple, tuple_key);
    if (value != NULL)
        g_hash_table_insert (md, (void *) key, value);
}

static void tuple_insert_to_hash(GHashTable * md, const Tuple * tuple,
                                 const gchar *key)
{
    tuple_insert_to_hash_full(md, tuple, key, key);
}

static void remove_metadata_value(gpointer value)
{
    g_value_unset((GValue *) value);
    g_free((GValue *) value);
}

static GHashTable *make_mpris_metadata(const gchar * filename, const Tuple * tuple)
{
    GHashTable *md = NULL;
    gpointer value;

    md = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, remove_metadata_value);

    value = g_malloc(sizeof(GValue));
    memset(value, 0, sizeof(GValue));
    g_value_init(value, G_TYPE_STRING);
    g_value_take_string(value, g_strdup(filename));
    g_hash_table_insert(md, "location", value);

    if (tuple != NULL)
    {
        tuple_insert_to_hash_full(md, tuple, "length", "mtime");
        tuple_insert_to_hash(md, tuple, "title");
        tuple_insert_to_hash(md, tuple, "artist");
        tuple_insert_to_hash(md, tuple, "album");
        tuple_insert_to_hash(md, tuple, "comment");
        tuple_insert_to_hash(md, tuple, "genre");
        tuple_insert_to_hash(md, tuple, "year");
        tuple_insert_to_hash(md, tuple, "codec");
        tuple_insert_to_hash(md, tuple, "quality");
        tuple_insert_to_hash_full(md, tuple, "track-number", "tracknumber");
        tuple_insert_to_hash_full(md, tuple, "bitrate", "audio-bitrate");
    }

    return md;
}

static void real_position(gint * playlist, gint * entry)
{
    if (*playlist == -2)
        *playlist = playlist_get_playing();
    if (*playlist == -1)
        *playlist = playlist_get_active();
    if (*entry == -1)
        *entry = playlist_get_position(*playlist);
}

static gboolean get_status_cb(void *data)
{
    struct StatusRequest *request = data;

    g_mutex_lock(info_mutex);

    memset (request, 0, sizeof (* request));
    request->playing = playback_get_playing();

    if (request->playing)
    {
        request->paused = playback_get_paused ();
        request->time = playback_get_time ();
        request->length = playback_get_length ();
        playback_get_info (& request->bitrate, & request->samplerate,
         & request->channels);
    }

    g_cond_signal(info_cond);
    g_mutex_unlock(info_mutex);
    return FALSE;
}

static void get_status(struct StatusRequest *request)
{
    if (g_thread_self() == main_thread)
        get_status_cb(request);
    else
    {
        g_mutex_lock(info_mutex);
        g_timeout_add(0, get_status_cb, request);
        g_cond_wait(info_cond, info_mutex);
        g_mutex_unlock(info_mutex);
    }
}

static gboolean get_position_cb(void *data)
{
    struct PositionRequest *request = data;

    g_mutex_lock(info_mutex);

    real_position(&request->playlist, &request->entry);
    request->entry_count = playlist_entry_count(request->playlist);
    request->queue_count = playlist_queue_count(request->playlist);

    g_cond_signal(info_cond);
    g_mutex_unlock(info_mutex);
    return FALSE;
}

static void get_position(struct PositionRequest *request)
{
    if (g_thread_self() == main_thread)
        get_position_cb(request);
    else
    {
        g_mutex_lock(info_mutex);
        g_timeout_add(0, get_position_cb, request);
        g_cond_wait(info_cond, info_mutex);
        g_mutex_unlock(info_mutex);
    }
}

static gboolean get_info_cb(void *data)
{
    struct InfoRequest *request = data;

    g_mutex_lock(info_mutex);

    real_position(&request->playlist, &request->entry);
    request->filename = playlist_entry_get_filename (request->playlist,
     request->entry);
    request->title = playlist_entry_get_title (request->playlist,
     request->entry, FALSE);
    request->length = playlist_entry_get_length (request->playlist,
     request->entry, FALSE);
    request->pltitle = playlist_get_title (request->playlist);

    g_cond_signal(info_cond);
    g_mutex_unlock(info_mutex);
    return FALSE;
}

static void get_info(struct InfoRequest *request)
{
    if (g_thread_self() == main_thread)
        get_info_cb(request);
    else
    {
        g_mutex_lock(info_mutex);
        g_timeout_add(0, get_info_cb, request);
        g_cond_wait(info_cond, info_mutex);
        g_mutex_unlock(info_mutex);
    }
}

static gboolean get_field_cb(void *data)
{
    struct FieldRequest *request = data;

    g_mutex_lock(info_mutex);

    real_position(&request->playlist, &request->entry);
    Tuple * tuple = playlist_entry_get_tuple (request->playlist, request->entry, FALSE);
    request->value = (tuple == NULL) ? NULL : tuple_value_to_gvalue(tuple, request->field);
    if (tuple)
        tuple_free (tuple)

    g_cond_signal(info_cond);
    g_mutex_unlock(info_mutex);
    return FALSE;
}

static void get_field(struct FieldRequest *request)
{
    if (g_thread_self() == main_thread)
        get_field_cb(request);
    else
    {
        g_mutex_lock(info_mutex);
        g_timeout_add(0, get_field_cb, request);
        g_cond_wait(info_cond, info_mutex);
        g_mutex_unlock(info_mutex);
    }
}

static gboolean play_cb(void *unused)
{
    /* Only the active playlist is visible through DBUS interface, so make sure
     * to play from it, not another playlist. --jlindgren */
    if (playlist_get_playing () != playlist_get_active ())
        playlist_set_playing (playlist_get_active ());

    drct_play();
    return FALSE;
}

static gboolean pause_cb(void *unused)
{
    playback_pause();
    return FALSE;
}

static gboolean play_pause_cb(void *unused)
{
    if (playback_get_playing())
        playback_pause();
    else
        playback_play (0, FALSE);

    return FALSE;
}

static gboolean seek_cb(void *data)
{
    playback_seek (GPOINTER_TO_INT (data));
    return FALSE;
}

static gboolean stop_cb(void *unused)
{
    playback_stop();
    return FALSE;
}

static gboolean prev_cb(void *unused)
{
    drct_pl_prev();
    return FALSE;
}

static gboolean next_cb(void *unused)
{
    drct_pl_next();
    return FALSE;
}

static gboolean jump_cb(void *data)
{
    drct_pl_set_pos(GPOINTER_TO_INT(data));
    return FALSE;
}

static gboolean add_cb(void *data)
{
    struct AddRequest *request = data;
    gint playlist = playlist_get_active();

    if (request->position < 0)
        request->position = playlist_entry_count (playlist);

    drct_pl_add (request->filename, request->position);

    if (request->play)
    {
        playlist_set_playing(playlist);
        playlist_set_position(playlist, request->position);
        playback_play (0, FALSE);
    }

    g_free(request);
    return FALSE;
}

static gboolean delete_cb(void *data)
{
    drct_pl_delete(GPOINTER_TO_INT(data));
    return FALSE;
}

static gboolean clear_cb(void *unused)
{
    drct_pl_clear();
    return FALSE;
}

static gboolean add_to_queue_cb(void *data)
{
    drct_pq_add(GPOINTER_TO_INT(data));
    return FALSE;
}

static gboolean remove_from_queue_cb(void *data)
{
    drct_pq_remove(GPOINTER_TO_INT(data));
    return FALSE;
}

static gboolean clear_queue_cb(void *unused)
{
    drct_pq_clear();
    return FALSE;
}

static gboolean queue_get_entry_cb(void *data)
{
    g_mutex_lock(info_mutex);

    * (gint *) data = drct_pq_get_entry (* (gint *) data);

    g_cond_signal(info_cond);
    g_mutex_unlock(info_mutex);
    return FALSE;
}

static gint queue_get_entry(gint position)
{
    if (g_thread_self() == main_thread)
        queue_get_entry_cb(&position);
    else
    {
        g_mutex_lock(info_mutex);
        g_timeout_add(0, queue_get_entry_cb, &position);
        g_cond_wait(info_cond, info_mutex);
        g_mutex_unlock(info_mutex);
    }

    return position;
}

static gboolean queue_find_entry_cb(void *data)
{
    g_mutex_lock(info_mutex);

    *(gint *) data = drct_pq_get_queue_position(*(gint *) data);

    g_cond_signal(info_cond);
    g_mutex_unlock(info_mutex);
    return FALSE;
}

static gint queue_find_entry(gint position)
{
    if (g_thread_self() == main_thread)
        queue_find_entry_cb(&position);
    else
    {
        g_mutex_lock(info_mutex);
        g_timeout_add(0, queue_find_entry_cb, &position);
        g_cond_wait(info_cond, info_mutex);
        g_mutex_unlock(info_mutex);
    }

    return position;
}

gboolean add_to_new_playlist_cb(void *data)
{
    drct_pl_open_temp (data);
    g_free(data);
    return FALSE;
}

static gboolean get_mpris_metadata_cb(void *data)
{
    struct MprisMetadataRequest *request = data;

    g_mutex_lock(info_mutex);

    real_position(&request->playlist, &request->entry);
    gchar * filename = playlist_entry_get_filename (request->playlist,
     request->entry);
    Tuple * tuple = playlist_entry_get_tuple (request->playlist, request->entry,
     FALSE);

    if (filename && tuple)
        request->metadata = make_mpris_metadata (filename, tuple);
    else
        request->metadata = NULL;

    g_free (filename);
    if (tuple)
        tuple_free (tuple);

    g_cond_signal(info_cond);
    g_mutex_unlock(info_mutex);
    return FALSE;
}

static void get_mpris_metadata(struct MprisMetadataRequest *request)
{
    if (g_thread_self() == main_thread)
        get_mpris_metadata_cb(request);
    else
    {
        g_mutex_lock(info_mutex);
        g_timeout_add(0, get_mpris_metadata_cb, request);
        g_cond_wait(info_cond, info_mutex);
        g_mutex_unlock(info_mutex);
    }
}

/* MPRIS API */
// MPRIS /
gboolean mpris_root_identity(MprisRoot * obj, gchar ** identity, GError ** error)
{
    *identity = g_strdup_printf("Audacious %s", VERSION);
    return TRUE;
}

gboolean mpris_root_quit(MprisPlayer * obj, GError ** error)
{
    event_queue("quit", NULL);
    return TRUE;
}

// MPRIS /Player

gboolean mpris_player_next(MprisPlayer * obj, GError * *error)
{
    g_timeout_add(0, next_cb, NULL);
    return TRUE;
}

gboolean mpris_player_prev(MprisPlayer * obj, GError * *error)
{
    g_timeout_add(0, prev_cb, NULL);
    return TRUE;
}

gboolean mpris_player_pause(MprisPlayer * obj, GError * *error)
{
    g_timeout_add(0, pause_cb, NULL);
    return TRUE;
}

gboolean mpris_player_stop(MprisPlayer * obj, GError * *error)
{
    g_timeout_add(0, stop_cb, NULL);
    return TRUE;
}

gboolean mpris_player_play(MprisPlayer * obj, GError * *error)
{
    g_timeout_add(0, play_cb, NULL);
    return TRUE;
}

gboolean mpris_player_repeat(MprisPlayer * obj, gboolean rpt, GError ** error)
{
    fprintf (stderr, "implement me\n");
    return TRUE;
}

static void append_int_value(GValueArray * ar, gint tmp)
{
    GValue value;
    memset(&value, 0, sizeof(value));
    g_value_init(&value, G_TYPE_INT);
    g_value_set_int(&value, tmp);
    g_value_array_append(ar, &value);
}

static gint get_playback_status(void)
{
    struct StatusRequest request;
    get_status(&request);

    return (!request.playing ? MPRIS_STATUS_STOP : request.paused ? MPRIS_STATUS_PAUSE : MPRIS_STATUS_PLAY);
}

gboolean mpris_player_get_status(MprisPlayer * obj, GValueArray * *status, GError * *error)
{
    *status = g_value_array_new(4);

    append_int_value(*status, (gint) get_playback_status());
    append_int_value (* status, get_bool (NULL, "shuffle"));
    append_int_value (* status, get_bool (NULL, "no_playlist_advance"));
    append_int_value (* status, get_bool (NULL, "repeat"));
    return TRUE;
}

gboolean mpris_player_get_metadata(MprisPlayer * obj, GHashTable * *metadata, GError * *error)
{
    struct MprisMetadataRequest request = {.playlist = -1,.entry = -1 };

    get_mpris_metadata(&request);
    *metadata = request.metadata;
    return TRUE;
}

gboolean mpris_player_get_caps(MprisPlayer * obj, gint * capabilities, GError ** error)
{
    *capabilities = MPRIS_CAPS_CAN_GO_NEXT | MPRIS_CAPS_CAN_GO_PREV | MPRIS_CAPS_CAN_PAUSE | MPRIS_CAPS_CAN_PLAY | MPRIS_CAPS_CAN_SEEK | MPRIS_CAPS_CAN_PROVIDE_METADATA | MPRIS_CAPS_PROVIDES_TIMING;
    return TRUE;
}

gboolean mpris_player_volume_set(MprisPlayer * obj, gint vol, GError ** error)
{
    drct_set_volume_main (vol);
    return TRUE;
}

gboolean mpris_player_volume_get(MprisPlayer * obj, gint * vol, GError ** error)
{
    drct_get_volume_main (vol);
    return TRUE;
}

gboolean mpris_player_position_set(MprisPlayer * obj, gint pos, GError * *error)
{
    g_timeout_add(0, seek_cb, GINT_TO_POINTER(pos));
    return TRUE;
}

gboolean mpris_player_position_get(MprisPlayer * obj, gint * pos, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *pos = request.time;
    return TRUE;
}

// MPRIS /Player signals
gboolean mpris_emit_caps_change(MprisPlayer * obj)
{
    g_signal_emit(obj, signals[CAPS_CHANGE_SIG], 0, 0);
    return TRUE;
}

gboolean mpris_emit_track_change(MprisPlayer * obj)
{
    gint playlist, entry;
    GHashTable *metadata;

    playlist = playlist_get_playing();
    entry = playlist_get_position(playlist);
    gchar * filename = playlist_entry_get_filename (playlist, entry);
    Tuple * tuple = playlist_entry_get_tuple (playlist, entry, FALSE);

    if (filename && tuple)
    {
        metadata = make_mpris_metadata (filename, tuple);
        g_signal_emit (obj, signals[TRACK_CHANGE_SIG], 0, metadata);
        g_hash_table_destroy (metadata);
    }

    g_free (filename);
    if (tuple)
        tuple_free (tuple);

    return (filename && tuple);
}

gboolean mpris_emit_status_change(MprisPlayer * obj, PlaybackStatus status)
{
    GValueArray *ar = g_value_array_new(4);

    if (status == MPRIS_STATUS_INVALID)
        status = get_playback_status ();

    append_int_value(ar, (gint) status);
    append_int_value (ar, get_bool (NULL, "shuffle"));
    append_int_value (ar, get_bool (NULL, "no_playlist_advance"));
    append_int_value (ar, get_bool (NULL, "repeat"));

    g_signal_emit(obj, signals[STATUS_CHANGE_SIG], 0, ar);
    g_value_array_free(ar);
    return TRUE;
}

// MPRIS /TrackList
gboolean mpris_emit_tracklist_change(MprisTrackList * obj, gint playlist)
{
    g_signal_emit(obj, tracklist_signals[TRACKLIST_CHANGE_SIG], 0, playlist_entry_count(playlist));
    return TRUE;
}

static void mpris_playlist_update_hook(gpointer unused, MprisTrackList * obj)
{
    gint playlist = playlist_get_active();

    mpris_emit_tracklist_change(obj, playlist);
}

gboolean mpris_tracklist_get_metadata(MprisTrackList * obj, gint pos, GHashTable * *metadata, GError * *error)
{
    struct MprisMetadataRequest request = {.playlist = -1,.entry = pos };

    get_mpris_metadata(&request);
    *metadata = request.metadata;
    return TRUE;
}

gboolean mpris_tracklist_get_current_track(MprisTrackList * obj, gint * pos, GError * *error)
{
    struct PositionRequest request = {.playlist = -1,.entry = -1 };

    get_position(&request);
    *pos = request.entry;
    return TRUE;
}

gboolean mpris_tracklist_get_length(MprisTrackList * obj, gint * length, GError * *error)
{
    struct PositionRequest request = {.playlist = -1,.entry = -1 };

    get_position(&request);
    *length = request.entry_count;
    return TRUE;
}

gboolean mpris_tracklist_add_track(MprisTrackList * obj, gchar * uri, gboolean play, GError * *error)
{
    struct AddRequest *request = g_malloc(sizeof(struct AddRequest));

    request->position = -1;
    request->filename = g_strdup(uri);
    request->play = play;

    g_timeout_add(0, add_cb, request);
    return TRUE;
}

gboolean mpris_tracklist_del_track(MprisTrackList * obj, gint pos, GError * *error)
{
    g_timeout_add(0, delete_cb, GINT_TO_POINTER(pos));
    return TRUE;
}

gboolean mpris_tracklist_loop (MprisTrackList * obj, gboolean loop, GError * *
 error)
{
    set_bool (NULL, "repeat", loop);
    return TRUE;
}

gboolean mpris_tracklist_random (MprisTrackList * obj, gboolean random,
 GError * * error)
{
    set_bool (NULL, "shuffle", random);
    return TRUE;
}

// Audacious General Information
gboolean audacious_rc_version(RemoteObject * obj, gchar ** version, GError ** error)
{
    *version = g_strdup(VERSION);
    return TRUE;
}

gboolean audacious_rc_quit(RemoteObject * obj, GError * *error)
{
    event_queue("quit", NULL);
    return TRUE;
}

gboolean audacious_rc_eject(RemoteObject * obj, GError ** error)
{
    interface_show_filebrowser (TRUE);
    return TRUE;
}

gboolean audacious_rc_main_win_visible (RemoteObject * obj,
 gboolean * visible, GError ** error)
{
    * visible = interface_is_shown ();
    return TRUE;
}

gboolean audacious_rc_show_main_win (RemoteObject * obj, gboolean show,
 GError * * error)
{
    interface_show (show);
    return TRUE;
}

gboolean audacious_rc_get_tuple_fields(RemoteObject * obj, gchar *** fields, GError ** error)
{
    gchar **res = g_new0(gchar *, FIELD_LAST + 1);
    gint i;
    for (i = 0; i < FIELD_LAST; i++)
    {
        res[i] = g_strdup(tuple_fields[i].name);
    }
    *fields = res;

    return TRUE;
}


// Playback Information/Manipulation

gboolean audacious_rc_play(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, play_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_pause(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, pause_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_stop(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, stop_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_playing(RemoteObject * obj, gboolean * is_playing, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *is_playing = request.playing;
    return TRUE;
}

gboolean audacious_rc_paused(RemoteObject * obj, gboolean * is_paused, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *is_paused = request.paused;
    return TRUE;
}

gboolean audacious_rc_stopped(RemoteObject * obj, gboolean * is_stopped, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *is_stopped = !request.playing;
    return TRUE;
}

gboolean audacious_rc_status(RemoteObject * obj, gchar * *status, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *status = g_strdup(!request.playing ? "stopped" : request.paused ? "paused" : "playing");
    return TRUE;
}

gboolean audacious_rc_info(RemoteObject * obj, gint * rate, gint * freq, gint * nch, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *rate = request.bitrate;
    *freq = request.samplerate;
    *nch = request.channels;
    return TRUE;
}

gboolean audacious_rc_time(RemoteObject * obj, gint * time, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *time = request.time;
    return TRUE;
}

gboolean audacious_rc_seek(RemoteObject * obj, guint pos, GError * *error)
{
    g_timeout_add(0, seek_cb, GINT_TO_POINTER(pos));
    return TRUE;
}

gboolean audacious_rc_volume(RemoteObject * obj, gint * vl, gint * vr, GError ** error)
{
    drct_get_volume (vl, vr);
    return TRUE;
}

gboolean audacious_rc_set_volume(RemoteObject * obj, gint vl, gint vr, GError ** error)
{
    drct_set_volume (vl, vr);
    return TRUE;
}

gboolean audacious_rc_balance(RemoteObject * obj, gint * balance, GError ** error)
{
    drct_get_volume_balance (balance);
    return TRUE;
}

// Playlist Information/Manipulation

gboolean audacious_rc_position(RemoteObject * obj, gint * pos, GError * *error)
{
    struct PositionRequest request = {.playlist = -1,.entry = -1 };

    get_position(&request);
    *pos = request.entry;
    return TRUE;
}

gboolean audacious_rc_advance(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, next_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_reverse(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, prev_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_length(RemoteObject * obj, gint * length, GError * *error)
{
    struct PositionRequest request = {.playlist = -1,.entry = -1 };

    get_position(&request);
    *length = request.entry_count;
    return TRUE;
}

gboolean audacious_rc_song_title(RemoteObject * obj, guint pos, gchar * *title, GError * *error)
{
    struct InfoRequest request = {.playlist = -1,.entry = pos };

    get_info(&request);
    g_free(request.filename);
    g_free(request.pltitle);
    *title = request.title;
    return TRUE;
}

gboolean audacious_rc_song_filename(RemoteObject * obj, guint pos, gchar * *filename, GError * *error)
{
    struct InfoRequest request = {.playlist = -1,.entry = pos };

    get_info(&request);
    *filename = request.filename;
    g_free(request.title);
    g_free(request.pltitle);
    return TRUE;
}

gboolean audacious_rc_song_length(RemoteObject * obj, guint pos, gint * length, GError * *error)
{
    audacious_rc_song_frames(obj, pos, length, error);
    *length /= 1000;
    return TRUE;
}

gboolean audacious_rc_song_frames(RemoteObject * obj, guint pos, gint * length, GError * *error)
{
    struct InfoRequest request = {.playlist = -1,.entry = pos };

    get_info(&request);
    g_free(request.filename);
    g_free(request.title);
    g_free(request.pltitle);
    *length = request.length;
    return TRUE;
}

gboolean audacious_rc_song_tuple(RemoteObject * obj, guint pos, gchar * field, GValue * value, GError * *error)
{
    struct FieldRequest request = {.playlist = -1,.entry = pos,.field = field };

    get_field(&request);

    if (request.value == NULL)
        return FALSE;

    memset(value, 0, sizeof(GValue));
    g_value_init(value, G_VALUE_TYPE(request.value));
    g_value_copy(request.value, value);
    g_value_unset(request.value);
    g_free(request.value);
    return TRUE;
}

gboolean audacious_rc_jump(RemoteObject * obj, guint pos, GError * *error)
{
    g_timeout_add(0, jump_cb, GINT_TO_POINTER(pos));
    return TRUE;
}

gboolean audacious_rc_add(RemoteObject * obj, gchar * file, GError * *error)
{
    return audacious_rc_playlist_ins_url_string(obj, file, -1, error);
}

gboolean audacious_rc_add_url(RemoteObject * obj, gchar * file, GError * *error)
{
    return audacious_rc_playlist_ins_url_string(obj, file, -1, error);
}

static GList * string_array_to_list (gchar * * strings)
{
    GList * list = NULL;

    while (* strings != NULL)
        list = g_list_prepend (list, * strings ++);

    return g_list_reverse (list);
}

gboolean audacious_rc_add_list (RemoteObject * obj, gchar * * filenames,
 GError * * error)
{
    GList * list = string_array_to_list (filenames);

    drct_pl_add_list (list, -1);
    g_list_free (list);
    return TRUE;
}

gboolean audacious_rc_open_list (RemoteObject * obj, gchar * * filenames,
 GError * * error)
{
    GList * list = string_array_to_list (filenames);

    drct_pl_open_list (list);
    g_list_free (list);
    return TRUE;
}

gboolean audacious_rc_open_list_to_temp (RemoteObject * obj, gchar * *
 filenames, GError * * error)
{
    GList * list = string_array_to_list (filenames);

    drct_pl_open_temp_list (list);
    g_list_free (list);
    return TRUE;
}

gboolean audacious_rc_delete(RemoteObject * obj, guint pos, GError * *error)
{
    g_timeout_add(0, delete_cb, GINT_TO_POINTER(pos));
    return TRUE;
}

gboolean audacious_rc_clear(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, clear_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_auto_advance(RemoteObject * obj, gboolean * is_advance, GError ** error)
{
    * is_advance = ! get_bool (NULL, "no_playlist_advance");
    return TRUE;
}

gboolean audacious_rc_toggle_auto_advance(RemoteObject * obj, GError ** error)
{
    set_bool (NULL, "no_playlist_advance", ! get_bool (NULL, "no_playlist_advance"));
    return TRUE;
}

gboolean audacious_rc_repeat(RemoteObject * obj, gboolean * is_repeating, GError ** error)
{
    *is_repeating = get_bool (NULL, "repeat");
    return TRUE;
}

gboolean audacious_rc_toggle_repeat (RemoteObject * obj, GError * * error)
{
    set_bool (NULL, "repeat", ! get_bool (NULL, "repeat"));
    return TRUE;
}

gboolean audacious_rc_shuffle(RemoteObject * obj, gboolean * is_shuffling, GError ** error)
{
    *is_shuffling = get_bool (NULL, "shuffle");
    return TRUE;
}

gboolean audacious_rc_toggle_shuffle (RemoteObject * obj, GError * * error)
{
    set_bool (NULL, "shuffle", ! get_bool (NULL, "shuffle"));
    return TRUE;
}

gboolean audacious_rc_stop_after (RemoteObject * obj, gboolean * is_stopping, GError * * error)
{
    * is_stopping = get_bool (NULL, "stop_after_current_song");
    return TRUE;
}

gboolean audacious_rc_toggle_stop_after (RemoteObject * obj, GError * * error)
{
    set_bool (NULL, "stop_after_current_song", ! get_bool (NULL, "stop_after_current_song"));
    return TRUE;
}

/* New on Oct 5 */
gboolean audacious_rc_show_prefs_box(RemoteObject * obj, gboolean show, GError ** error)
{
    event_queue("prefswin show", GINT_TO_POINTER(show));
    return TRUE;
}

gboolean audacious_rc_show_about_box(RemoteObject * obj, gboolean show, GError ** error)
{
    event_queue("aboutwin show", GINT_TO_POINTER(show));
    return TRUE;
}

gboolean audacious_rc_show_jtf_box(RemoteObject * obj, gboolean show, GError ** error)
{
    if (show)
        event_queue("interface show jump to track", NULL);
    else
        event_queue("interface hide jump to track", NULL);
    return TRUE;
}

gboolean audacious_rc_show_filebrowser(RemoteObject * obj, gboolean show, GError ** error)
{
    if (show)
        event_queue("filebrowser show", GINT_TO_POINTER(FALSE));
    else
        event_queue("filebrowser hide", NULL);
    return TRUE;
}

gboolean audacious_rc_play_pause(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, play_pause_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_get_info(RemoteObject * obj, gint * rate, gint * freq, gint * nch, GError * *error)
{
    struct StatusRequest request;

    get_status(&request);
    *rate = request.bitrate;
    *freq = request.samplerate;
    *nch = request.channels;
    return TRUE;
}

gboolean audacious_rc_toggle_aot(RemoteObject * obj, gboolean ontop, GError ** error)
{
    hook_call("mainwin set always on top", &ontop);
    return TRUE;
}

gboolean audacious_rc_playqueue_add(RemoteObject * obj, gint pos, GError * *error)
{
    g_timeout_add(0, add_to_queue_cb, GINT_TO_POINTER(pos));
    return TRUE;
}

gboolean audacious_rc_playqueue_remove(RemoteObject * obj, gint pos, GError * *error)
{
    g_timeout_add(0, remove_from_queue_cb, GINT_TO_POINTER(pos));
    return TRUE;
}

gboolean audacious_rc_playqueue_clear(RemoteObject * obj, GError * *error)
{
    g_timeout_add(0, clear_queue_cb, NULL);
    return TRUE;
}

gboolean audacious_rc_get_playqueue_length(RemoteObject * obj, gint * length, GError * *error)
{
    struct PositionRequest request = {.playlist = -1,.entry = -1 };

    get_position(&request);
    *length = request.queue_count;
    return TRUE;
}

gboolean audacious_rc_queue_get_list_pos(RemoteObject * obj, gint qpos, gint * pos, GError * *error)
{
    *pos = queue_get_entry(qpos);
    return TRUE;
}

gboolean audacious_rc_queue_get_queue_pos(RemoteObject * obj, gint pos, gint * qpos, GError * *error)
{
    *qpos = queue_find_entry(pos);
    return TRUE;
}

gboolean audacious_rc_playqueue_is_queued(RemoteObject * obj, gint pos, gboolean * is_queued, GError * *error)
{
    *is_queued = (queue_find_entry(pos) != -1);
    return TRUE;
}

gboolean audacious_rc_playlist_ins_url_string(RemoteObject * obj, gchar * url, gint pos, GError * *error)
{
    struct AddRequest *request = g_malloc(sizeof(struct AddRequest));

    request->position = pos;
    request->filename = g_strdup(url);
    request->play = FALSE;

    g_timeout_add(0, add_cb, request);
    return TRUE;
}

gboolean audacious_rc_playlist_add(RemoteObject * obj, void *list, GError * *error)
{
    return audacious_rc_playlist_ins_url_string(obj, list, -1, error);
}

gboolean audacious_rc_playlist_enqueue_to_temp(RemoteObject * obj, gchar * url, GError * *error)
{
    g_timeout_add(0, add_to_new_playlist_cb, g_strdup(url));
    return TRUE;
}

/* New on Nov 7: Equalizer */
gboolean audacious_rc_get_eq(RemoteObject * obj, gdouble * preamp, GArray ** bands, GError ** error)
{
    * preamp = get_double (NULL, "equalizer_preamp");
    * bands = g_array_new (FALSE, FALSE, sizeof (gdouble));
    g_array_set_size (* bands, AUD_EQUALIZER_NBANDS);
    eq_get_bands ((gdouble *) (* bands)->data);

    return TRUE;
}

gboolean audacious_rc_get_eq_preamp(RemoteObject * obj, gdouble * preamp, GError ** error)
{
    * preamp = get_double (NULL, "equalizer_preamp");
    return TRUE;
}

gboolean audacious_rc_get_eq_band(RemoteObject * obj, gint band, gdouble * value, GError ** error)
{
    * value = eq_get_band (band);
    return TRUE;
}

gboolean audacious_rc_set_eq(RemoteObject * obj, gdouble preamp, GArray * bands, GError ** error)
{
    set_double (NULL, "equalizer_preamp", preamp);
    eq_set_bands ((gdouble *) bands->data);
    return TRUE;
}

gboolean audacious_rc_set_eq_preamp(RemoteObject * obj, gdouble preamp, GError ** error)
{
    set_double (NULL, "equalizer_preamp", preamp);
    return TRUE;
}

gboolean audacious_rc_set_eq_band(RemoteObject * obj, gint band, gdouble value, GError ** error)
{
    eq_set_band (band, value);
    return TRUE;
}

gboolean audacious_rc_equalizer_activate(RemoteObject * obj, gboolean active, GError ** error)
{
    set_bool (NULL, "equalizer_active", active);
    return TRUE;
}

gboolean audacious_rc_get_active_playlist_name(RemoteObject * obj, gchar * *title, GError * *error)
{
    struct InfoRequest request = {.playlist = -2 };

    get_info(&request);
    g_free(request.title);
    g_free(request.filename);
    *title = request.pltitle;
    return TRUE;
}

DBusGProxy *audacious_get_dbus_proxy(void)
{
    DBusGConnection *connection = NULL;
    GError *error = NULL;
    connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    g_clear_error(&error);
    return dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE, AUDACIOUS_DBUS_PATH, AUDACIOUS_DBUS_INTERFACE);
}
