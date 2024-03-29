/*
 * playlist-api.h
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

/* Do not include this file directly; use playlist.h instead. */

/* --- PLAYLIST CORE API --- */

/* Returns the number of playlists currently open.  There will always be at
 * least one playlist open.  The playlists are numbered starting from zero. */
AUD_FUNC0 (gint, playlist_count)

/* Adds a new playlist before the one numbered <at>.  If <at> is negative or
 * equal to the number of playlists, adds a new playlist after the last one. */
AUD_FUNC1 (void, playlist_insert, gint, at)

/* Moves a contiguous block of <count> playlists starting with the one numbered
 * <from> such that that playlist ends up at the position <to>. */
AUD_FUNC3 (void, playlist_reorder, gint, from, gint, to, gint, count)

/* Closes a playlist.  CAUTION: The playlist is not saved, and no confirmation
 * is presented to the user.  If <playlist> is the only playlist, a new playlist
 * is added.  If <playlist> is the active playlist, another playlist is marked
 * active.  If <playlist> is the one from which the last song played was taken,
 * playback is stopped.  In this case, calls to playlist_get_playing() will
 * return -1, and the behavior of drct_play() is unspecified. */
AUD_FUNC1 (void, playlist_delete, gint, playlist)

/* Returns a unique non-negative integer which can be used to identify a given
 * playlist even if its numbering changes (as when playlists are reordered).
 * On error, returns -1. */
AUD_FUNC1 (gint, playlist_get_unique_id, gint, playlist)

/* Returns the number of the playlist identified by a given integer ID as
 * returned by playlist_get_unique_id().  If the playlist no longer exists,
 * returns -1. */
AUD_FUNC1 (gint, playlist_by_unique_id, gint, id)

/* Sets the filename associated with a playlist.  (Audacious currently makes no
 * use of the filename.) */
AUD_FUNC2 (void, playlist_set_filename, gint, playlist, const gchar *, filename)

/* Returns the filename associated with a playlist.  The filename should be
 * freed when no longer needed. */
AUD_FUNC1 (gchar *, playlist_get_filename, gint, playlist)

/* Sets the title associated with a playlist. */
AUD_FUNC2 (void, playlist_set_title, gint, playlist, const gchar *, title)

/* Returns the title associated with a playlist.  The title should be freed when
 * no longer needed. */
AUD_FUNC1 (gchar *, playlist_get_title, gint, playlist)

/* Marks a playlist as active.  This is the playlist which the user will see and
 * on which most DRCT functions will take effect. */
AUD_FUNC1 (void, playlist_set_active, gint, playlist)

/* Returns the number of the playlist marked active. */
AUD_FUNC0 (gint, playlist_get_active)

/* Sets the playlist in which playback will begin when drct_play() is called.
 * This does not mark the playlist as active.  If a song is already playing, it
 * will be stopped.  If <playlist> is negative, calls to playlist_get_playing()
 * will return -1, and the behavior of drct_play() is unspecified. */
AUD_FUNC1 (void, playlist_set_playing, gint, playlist)

/* Returns the number of the playlist from which the last song played was taken,
 * or -1 if that cannot be determined.  Note that this playlist may not be
 * marked active. */
AUD_FUNC0 (gint, playlist_get_playing)

/* Returns the number of entries in a playlist.  The entries are numbered
 * starting from zero. */
AUD_FUNC1 (gint, playlist_entry_count, gint, playlist)

/* Adds a song file, playlist file, or folder to a playlist before the entry
 * numbered <at>.  If <at> is negative or equal to the number of entries, the
 * item is added after the last entry.  <tuple> may be NULL, in which case
 * Audacious will attempt to read metadata from the song file.  Audacious will
 * free the memory used by the filename and the tuple when they are no longer
 * needed.  Adding items to the playlist can be a slow process, and this
 * function may return before the process is complete.  Hence, the caller must
 * not assume that there will be new entries in the playlist immediately after
 * this function is called.  If <play> is nonzero, Audacious will begin playback
 * of the items once they have been added. */
AUD_FUNC5 (void, playlist_entry_insert, gint, playlist, gint, at, gchar *,
 filename, Tuple *, tuple, gboolean, play)

/* Similar to playlist_entry_insert, adds multiple song files, playlist files,
 * or folders to a playlist.  The filenames are stored as (gchar *) in an index
 * (see libaudcore/index.h).  Tuples are likewise stored as (Tuple *) in an
 * index of the same length.  <tuples> may be NULL, or individual pointers
 * within it may be NULL.  Audacious will free both indexes, the filenames, and
 * the tuples when they are no longer needed. */
AUD_FUNC5 (void, playlist_entry_insert_batch, gint, playlist, gint, at,
 struct index *, filenames, struct index *, tuples, gboolean, play)

/* Removes a contiguous block of <number> entries starting from the one numbered
 * <at> from a playlist.  If the last song played is in this block, playback is
 * stopped.  In this case, calls to playlist_get_position() will return -1, and
 * the behavior of drct_play() is unspecified. */
AUD_FUNC3 (void, playlist_entry_delete, gint, playlist, gint, at, gint, number)

/* Returns the filename of an entry.  The filename should be freed when no
 * longer needed. */
AUD_FUNC2 (gchar *, playlist_entry_get_filename, gint, playlist, gint, entry)

/* Returns a handle to the decoder plugin associated with an entry, or NULL if
 * none can be found.  If <fast> is nonzero, returns NULL if no decoder plugin
 * has yet been found. */
AUD_FUNC3 (PluginHandle *, playlist_entry_get_decoder, gint, playlist, gint,
 entry, gboolean, fast)

/* Returns the tuple associated with an entry, or NULL if one is not available.
 * The reference count of the tuple is incremented.  If <fast> is nonzero,
 * returns NULL if metadata for the entry has not yet been read from the song
 * file. */
AUD_FUNC3 (Tuple *, playlist_entry_get_tuple, gint, playlist, gint, entry,
 gboolean, fast)

/* Returns a formatted title string for an entry.  This may include information
 * such as the filename, song title, and/or artist.  The string should be freed
 * when no longer needed.  If <fast> is nonzero, returns the entry's filename if
 * metadata for the entry has not yet been read. */
AUD_FUNC3 (gchar *, playlist_entry_get_title, gint, playlist, gint, entry,
 gboolean, fast)

/* Returns three strings (title, artist, and album) describing an entry.  The
 * strings should be freed when no longer needed.  If <fast> is nonzero, returns
 * the entry's filename for <title> and NULL for <artist> and <album> if
 * metadata for the entry has not yet been read. */
AUD_FUNC6 (void, playlist_entry_describe, gint, playlist, gint, entry,
 gchar * *, title, gchar * *, artist, gchar * *, album, gboolean, fast)

/* Returns the length in milliseconds of an entry, or -1 if the length is not
 * known.  <fast> is as in playlist_entry_get_tuple(). */
AUD_FUNC3 (gint, playlist_entry_get_length, gint, playlist, gint, entry,
 gboolean, fast)

/* Sets the entry which will be played (if this playlist is selected with
 * playlist_set_playing()) when drct_play() is called.  If a song from this
 * playlist is already playing, it will be stopped.  If <position> is negative,
 * calls to playlist_get_position() will return -1, and the behavior of
 * drct_play() is unspecified. */
AUD_FUNC2 (void, playlist_set_position, gint, playlist, gint, position)

/* Returns the number of the entry which was last played from this playlist, or
 * -1 if that cannot be determined. */
AUD_FUNC1 (gint, playlist_get_position, gint, playlist)

/* Sets whether an entry is selected. */
AUD_FUNC3 (void, playlist_entry_set_selected, gint, playlist, gint, entry,
 gboolean, selected)

/* Returns whether an entry is selected. */
AUD_FUNC2 (gboolean, playlist_entry_get_selected, gint, playlist, gint, entry)

/* Returns the number of selected entries in a playlist. */
AUD_FUNC1 (gint, playlist_selected_count, gint, playlist)

/* Selects all (or none) of the entries in a playlist. */
AUD_FUNC2 (void, playlist_select_all, gint, playlist, gboolean, selected)

/* Moves a selected entry within a playlist by an offset of <distance> entries.
 * Other selected entries are gathered around it.  Returns the offset by which
 * the entry was actually moved, which may be less in absolute value than the
 * requested offset. */
AUD_FUNC3 (gint, playlist_shift, gint, playlist, gint, position, gint, distance)

/* Removes the selected entries from a playlist.  If the last song played is one
 * of these entries, playback is stopped.  In this case, calls to
 * playlist_get_position() will return -1, and the behavior of drct_play() is
 * unspecified. */
AUD_FUNC1 (void, playlist_delete_selected, gint, playlist)

/* Sorts the entries in a playlist based on filename.  The callback function
 * should return negative if the first filename comes before the second,
 * positive if it comes after, or zero if the two are indistinguishable. */
AUD_FUNC2 (void, playlist_sort_by_filename, gint, playlist,
 PlaylistStringCompareFunc, compare)

/* Sorts the entries in a playlist based on tuple. */
AUD_FUNC2 (void, playlist_sort_by_tuple, gint, playlist,
 PlaylistTupleCompareFunc, compare)

/* Sorts the entries in a playlist based on formatted title string. */
AUD_FUNC2 (void, playlist_sort_by_title, gint, playlist,
 PlaylistStringCompareFunc, compare)

/* Sorts only the selected entries in a playlist based on filename. */
AUD_FUNC2 (void, playlist_sort_selected_by_filename, gint, playlist,
 PlaylistStringCompareFunc, compare)

/* Sorts only the selected entries in a playlist based on tuple. */
AUD_FUNC2 (void, playlist_sort_selected_by_tuple, gint, playlist,
 PlaylistTupleCompareFunc, compare)

/* Sorts only the selected entries in a playlist based on formatted title string. */
AUD_FUNC2 (void, playlist_sort_selected_by_title, gint, playlist,
 PlaylistStringCompareFunc, compare)

/* Reverses the order of the entries in a playlist. */
AUD_FUNC1 (void, playlist_reverse, gint, playlist)

/* Reorders the entries in a playlist randomly. */
AUD_FUNC1 (void, playlist_randomize, gint, playlist)

/* Discards the metadata stored for all the entries in a playlist and starts
 * reading it afresh from the song files in the background. */
AUD_FUNC1 (void, playlist_rescan, gint, playlist)

/* Like playlist_rescan, but applies only to the selected entries in a playlist. */
AUD_FUNC1 (void, playlist_rescan_selected, gint, playlist)

/* Discards the metadata stored for all the entries that refer to a particular
 * song file, in whatever playlist they appear, and starts reading it afresh
 * from that file in the background. */
AUD_FUNC1 (void, playlist_rescan_file, const gchar *, filename)

/* Calculates the total length in milliseconds of all the entries in a playlist.
 * <fast> is as in playlist_entry_get_tuple(). */
AUD_FUNC2 (gint64, playlist_get_total_length, gint, playlist, gboolean, fast)

/* Calculates the total length in milliseconds of only the selected entries in a
 * playlist.  <fast> is as in playlist_entry_get_tuple(). */
AUD_FUNC2 (gint64, playlist_get_selected_length, gint, playlist, gboolean, fast)

/* Returns the number of entries in a playlist queue.  The entries are numbered
 * starting from zero, lower numbers being played first. */
AUD_FUNC1 (gint, playlist_queue_count, gint, playlist)

/* Adds an entry to a playlist's queue before the entry numbered <at> in the
 * queue.  If <at> is negative or equal to the number of entries in the queue,
 * adds the entry after the last one in the queue.  The same entry cannot be
 * added to the queue more than once. */
AUD_FUNC3 (void, playlist_queue_insert, gint, playlist, gint, at, gint, entry)

/* Adds the selected entries in a playlist to the queue, if they are not already
 * in it. */
AUD_FUNC2 (void, playlist_queue_insert_selected, gint, playlist, gint, at)

/* Returns the position in the playlist of the entry at a given position in the
 * queue. */
AUD_FUNC2 (gint, playlist_queue_get_entry, gint, playlist, gint, at)

/* Returns the position in the queue of the entry at a given position in the
 * playlist.  If it is not in the queue, returns -1. */
AUD_FUNC2 (gint, playlist_queue_find_entry, gint, playlist, gint, entry)

/* Removes a contiguous block of <number> entries starting with the one numbered
 * <at> from the queue. */
AUD_FUNC3 (void, playlist_queue_delete, gint, playlist, gint, at, gint, number)

/* Removes the selected entries in a playlist from the queue, if they are in it. */
AUD_FUNC1 (void, playlist_queue_delete_selected, gint, playlist)

/* Returns nonzero if any playlist has been changed since the last call of the
 * "playlist update" hook.  If called from within the hook, returns nonzero. */
AUD_FUNC0 (gboolean, playlist_update_pending)

/* May be called within the "playlist update" hook to determine what range of
 * entries must be updated.  If all entries in all playlists must be updated,
 * returns zero.  If a limited range in a single playlist must be updated,
 * returns nonzero.  In this case, stores the number of that playlist at
 * <playlist>, the number of the first entry to be updated at <at>, and the
 * number of contiguous entries to be updated at <count>.  Note that entries may
 * have been added or removed within this range. */
AUD_FUNC3 (gboolean, playlist_update_range, gint *, playlist, gint *, at,
 gint *, count)

/* --- PLAYLIST UTILITY API --- */

/* Sorts the entries in a playlist according to one of the schemes listed in
 * playlist.h. */
AUD_FUNC2 (void, playlist_sort_by_scheme, gint, playlist, gint, scheme)

/* Sorts only the selected entries in a playlist according to one of those
 * schemes. */
AUD_FUNC2 (void, playlist_sort_selected_by_scheme, gint, playlist, gint, scheme)

/* Removes duplicate entries in a playlist according to one of those schemes.
 * As currently implemented, first sorts the playlist. */
AUD_FUNC2 (void, playlist_remove_duplicates_by_scheme, gint, playlist, gint,
 scheme)

/* Removes all entries referring to unavailable files in a playlist.  ("Remove
 * failed" is something of a misnomer for the current behavior.)  As currently
 * implemented, only works for file:// URIs. */
AUD_FUNC1 (void, playlist_remove_failed, gint, playlist)

/* Selects all the entries in a playlist that match regular expressions stored
 * in the fields of a tuple.  Does not free the memory used by the tuple.
 * Example: To select all the songs whose title starts with the letter "A",
 * create a blank tuple and set its title field to "^A". */
AUD_FUNC2 (void, playlist_select_by_patterns, gint, playlist, const Tuple *,
 patterns)

/* Returns nonzero if <filename> refers to a playlist file. */
AUD_FUNC1 (gboolean, filename_is_playlist, const gchar *, filename)

/* Saves the entries in a playlist to a playlist file.  The format of the file
 * is determined from the file extension.  Returns nonzero on success. */
AUD_FUNC2 (gboolean, playlist_save, gint, playlist, const gchar *, filename)
