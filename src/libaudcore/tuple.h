/*
 * Audacious
 * Copyright (c) 2006-2007 Audacious team
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
/**
 * @file tuple.h
 * @brief Basic Tuple handling API.
 */

#ifndef AUDACIOUS_TUPLE_H
#define AUDACIOUS_TUPLE_H

#include <glib.h>
#include <mowgli.h>

G_BEGIN_DECLS

/** Ordered enum for basic #Tuple fields.
 * @sa TupleBasicType
 */
enum {
    FIELD_ARTIST = 0,
    FIELD_TITLE,        /**< Song title */
    FIELD_ALBUM,        /**< Album name */
    FIELD_COMMENT,      /**< Freeform comment */
    FIELD_GENRE,        /**< Song's genre */

    FIELD_TRACK,
    FIELD_TRACK_NUMBER,
    FIELD_LENGTH,       /**< Track length in milliseconds */
    FIELD_YEAR,         /**< Year of production/performance/etc */
    FIELD_QUALITY,      /**< String representing quality, such as
                             "lossy", "lossless", "sequenced"  */

    FIELD_CODEC,        /**< Codec name or similar */
    FIELD_FILE_NAME,    /**< File name part of the location URI */
    FIELD_FILE_PATH,    /**< Path part of the location URI */
    FIELD_FILE_EXT,     /**< Filename extension part of the location URI */
    FIELD_SONG_ARTIST,

    FIELD_MTIME,        /**< Playlist entry modification time for internal use */
    FIELD_FORMATTER,    /**< Playlist entry Tuplez formatting string */
    FIELD_PERFORMER,
    FIELD_COPYRIGHT,
    FIELD_DATE,

    FIELD_SUBSONG_ID,   /**< Index number of subsong/tune */
    FIELD_SUBSONG_NUM,  /**< Total number of subsongs in the file */
    FIELD_MIMETYPE,
    FIELD_BITRATE,      /**< Bitrate in kbps */

    FIELD_SEGMENT_START,
    FIELD_SEGMENT_END,

    /* Preserving replay gain information accurately is a challenge since there
     * are several differents formats around.  We use an integer fraction, with
     * the denominator stored in the *_UNIT fields.  For example, if ALBUM_GAIN
     * is 512 and GAIN_UNIT is 256, then the album gain is +2 dB.  If TRACK_PEAK
     * is 787 and PEAK_UNIT is 1000, then the peak volume is 0.787 in a -1.0 to
     * 1.0 range. */
    FIELD_GAIN_ALBUM_GAIN,
    FIELD_GAIN_ALBUM_PEAK,
    FIELD_GAIN_TRACK_GAIN,
    FIELD_GAIN_TRACK_PEAK,
    FIELD_GAIN_GAIN_UNIT,
    FIELD_GAIN_PEAK_UNIT,

    FIELD_COMPOSER,     /**< Composer of song, if different than artist. */
    FIELD_MBID,         /** MusicBrainz identifer for the song */

    /* Special field, must always be last */
    FIELD_LAST
};

typedef enum {
    TUPLE_STRING,
    TUPLE_INT,
    TUPLE_UNKNOWN
} TupleValueType;

typedef struct {
    gchar *name;
    TupleValueType type;
} TupleBasicType;

extern const TupleBasicType tuple_fields[FIELD_LAST];

#define TUPLE_NAME_MAX 20

typedef struct {
    gchar name[TUPLE_NAME_MAX]; /* for standard fields, the empty string */
    TupleValueType type;
    union {
        gchar *string;
        gint integer;
    } value;
} TupleValue;

/**
 * Structure for holding and passing around miscellaneous track
 * metadata. This is not the same as a playlist entry, though.
 */
typedef struct _Tuple {
    mowgli_object_t parent;
    mowgli_patricia_t *dict;        /**< Mowgli dictionary for holding other than basic values. */
    TupleValue *values[FIELD_LAST]; /**< Basic #Tuple values, entry is NULL if not set. */
    gint nsubtunes;                 /**< Number of subtunes, if any. Values greater than 0
                                         mean that there are subtunes and #subtunes array
                                         may be set. */
    gint *subtunes;                 /**< Array of gint containing subtune index numbers.
                                         Can be NULL if indexing is linear or if
                                         there are no subtunes. */
} Tuple;


Tuple *tuple_new(void);
Tuple *tuple_copy(const Tuple *);
void tuple_set_filename(Tuple *tuple, const gchar *filename);
Tuple *tuple_new_from_filename(const gchar *filename);
gboolean tuple_associate_string_rel(Tuple *tuple, const gint nfield, const gchar *field, gchar *string);
gboolean tuple_associate_string(Tuple *tuple, const gint nfield, const gchar *field, const gchar *string);
gboolean tuple_associate_int(Tuple *tuple, const gint nfield, const gchar *field, gint integer);
void tuple_disassociate(Tuple *tuple, const gint nfield, const gchar *field);
void tuple_disassociate_now(TupleValue *value);
TupleValueType tuple_get_value_type (const Tuple * tuple, gint nfield,
 const gchar * field);
const gchar * tuple_get_string (const Tuple * tuple, gint nfield, const gchar *
 field);
gint tuple_get_int (const Tuple * tuple, gint nfield, const gchar * field);
#define tuple_free(x) mowgli_object_unref(x);

/* Fills in format-related fields (specifically FIELD_CODEC, FIELD_QUALITY, and
 * FIELD_BITRATE.  Plugins should use this function instead of setting these
 * fields individually so that the style is consistent across file formats.
 * <format> should be a brief description such as "Microsoft WAV", "MPEG-1 layer
 * 3", "Audio CD", and so on.  <samplerate> is in Hertz.  <bitrate> is in 1000
 * bits per second. */
void tuple_set_format (Tuple * tuple, const gchar * format, gint channels, gint
 samplerate, gint bitrate);

G_END_DECLS

#endif /* AUDACIOUS_TUPLE_H */
