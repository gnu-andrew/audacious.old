/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2008  Audacious team
 *
 *  Based on BMP:
 *  Copyright (C) 2003-2004  BMP development team.
 *
 *  Based on XMMS:
 *  Copyright (C) 1998-2003  XMMS development team.
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

/*#define AUD_DEBUG*/

#include <stdint.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "audconfig.h"
#include "equalizer.h"
#include "output.h"
#include "main.h"
#include "input.h"
#include "playback.h"

#include "playlist.h"
#include "configdb.h"

#include "flow.h"

#include "pluginenum.h"

#include "effect.h"
#include "volumecontrol.h"
#include "visualization.h"

#include "libSAD.h"
#include "util.h"
#include "equalizer_flow.h"

#include <math.h>

#ifdef USE_SAMPLERATE
# include "src_flow.h"
#endif

OutputPluginData op_data = {
    NULL,
    NULL
};

OutputPluginState op_state = {
    0,
    0,
    0
};

static gint decoder_srate = 0;
static gboolean bypass_dsp = FALSE;

OutputPlugin psuedo_output_plugin = {
    .description = "XMMS reverse compatibility output plugin",
    .get_volume = output_get_volume,
    .set_volume = output_set_volume,

    .open_audio = output_open_audio,
    .write_audio = output_write_audio,
    .close_audio = output_close_audio,

    .flush = output_flush,
    .pause = output_pause,

    .buffer_free = output_buffer_free,
    .buffer_playing = output_buffer_playing,
    .output_time = get_output_time,
    .written_time = get_written_time,
};

static void apply_replaygain_info (ReplayGainInfo *rg_info);

OutputPlugin *
get_current_output_plugin(void)
{
    return op_data.current_output_plugin;
}

void
set_current_output_plugin(gint i)
{
    char playing;
    Playlist * playlist;
    int time, position;

    playing = playback_get_playing ();

    if (playing)
    {
        playlist = playlist_get_active ();
        position = playlist_get_position (playlist);
        time = playback_get_time ();
        playback_stop();
    }

    op_data.current_output_plugin = g_list_nth(op_data.output_list, i)->data;

    if (playing)
    {
        playlist_set_position (playlist, position);
        playback_initiate ();
        playback_seek (time / 1000);
    }
}

GList *
get_output_list(void)
{
    return op_data.output_list;
}

void
output_about(gint i)
{
    OutputPlugin *out = g_list_nth(op_data.output_list, i)->data;
    if (out && out->about)
    {
	plugin_set_current((Plugin *)out);
        out->about();
    }
}

void
output_configure(gint i)
{
    OutputPlugin *out = g_list_nth(op_data.output_list, i)->data;
    if (out && out->configure)
    {
	plugin_set_current((Plugin *)out);
        out->configure();
    }
}

void
output_get_volume(gint * l, gint * r)
{
    if (cfg.software_volume_control)
    {
        * l = cfg.sw_volume_left;
        * r = cfg.sw_volume_right;
    }
    else if (op_data.current_output_plugin &&
     op_data.current_output_plugin->get_volume)
        op_data.current_output_plugin->get_volume (l, r);
    else
    {
        * l = -1;
        * r = -1;
    }
}

void
output_set_volume(gint l, gint r)
{
    if (cfg.software_volume_control)
    {
        cfg.sw_volume_left = l;
        cfg.sw_volume_right = r;
    }
    else if (op_data.current_output_plugin &&
     op_data.current_output_plugin->set_volume)
        op_data.current_output_plugin->set_volume (l, r);
}

void
output_set_eq(gboolean active, gfloat pre, gfloat * bands)
{
    AUDDBG("preamp: %f, bands: %f:%f:%f:%f:%f:%f:%f:%f:%f:%f\n", pre, bands[0], bands[1], bands[2], bands[3], bands[4],
            bands[5], bands[6], bands[7], bands[8], bands[9]);

    equalizer_flow_set_bands(pre, bands);
}

/* called by input plugin to peek at the output plugin's write progress */
gint
get_written_time(void)
{
    OutputPlugin *op = get_current_output_plugin();

    plugin_set_current((Plugin *)op);
    return op->written_time();
}

/* called by input plugin to peek at the output plugin's output progress */
gint
get_output_time(void)
{
    OutputPlugin *op = get_current_output_plugin();

    plugin_set_current((Plugin *)op);
    return op->output_time();
}

static SAD_dither_t *sad_state_to_float = NULL;
static SAD_dither_t *sad_state_from_float = NULL;

static ReplayGainInfo replay_gain_info = {
    .track_gain = 0.0,
    .track_peak = 0.0,
    .album_gain = 0.0,
    .album_peak = 0.0,
};

static void
freeSAD()
{
    if (sad_state_from_float != NULL) {SAD_dither_free(sad_state_from_float); sad_state_from_float = NULL;}
    if (sad_state_to_float != NULL)   {SAD_dither_free(sad_state_to_float);   sad_state_to_float = NULL;}
}

static gboolean
reopen_audio(AFormat fmt, gint rate, gint nch)
{
    OutputPlugin *op = get_current_output_plugin();

    if (op == NULL)
        return FALSE;

    /* Is our output port already open? */
    if ((op_state.rate != 0 && op_state.nch != 0) &&
        (op_state.rate == rate && op_state.nch == nch && op_state.fmt == fmt))
    {
        /* Yes, and it's the correct sampling rate. Reset the counter and go. */
	AUDDBG("flushing output instead of reopening\n");
	plugin_set_current((Plugin *)op);
        op->flush(0);
        return TRUE;
    }
    else if (op_state.rate != 0 && op_state.nch != 0)
    {
        plugin_set_current((Plugin *)op);
        op->close_audio();
	op_state.fmt = 0;
	op_state.rate = 0;
	op_state.nch = 0;
    }

    plugin_set_current((Plugin *)op);
    gint ret = op->open_audio(fmt, rate, nch);

    if (ret == 1)            /* Success? */
    {
        AUDDBG("opened audio: fmt=%d, rate=%d, nch=%d\n", fmt, rate, nch);
        op_state.fmt = fmt;
        op_state.rate = rate;
        op_state.nch = nch;

        output_set_eq (cfg.equalizer_active, cfg.equalizer_preamp,
         cfg.equalizer_bands);
        return TRUE;
    } else {
        return FALSE;
    }
}

gint
output_open_audio(AFormat fmt, gint rate, gint nch)
{
    gint ret;
    AUDDBG("requested: fmt=%d, rate=%d, nch=%d\n", fmt, rate, nch);

    AFormat output_fmt;
    int bit_depth;
    SAD_buffer_format input_sad_fmt;
    SAD_buffer_format output_sad_fmt;

    decoder_srate = rate;
    bypass_dsp = cfg.bypass_dsp;

    if (bypass_dsp) {
        AUDDBG("trying to open audio in native format\n");
        bypass_dsp = reopen_audio(fmt, rate, nch);
        AUDDBG("opening in native fmt %s\n", bypass_dsp ? "succeeded" : "failed");
    }

    if (bypass_dsp) {
        return TRUE;
    } else {
#ifdef USE_SAMPLERATE
        rate = src_flow_init(rate, nch); /* returns sample rate unchanged if resampling switched off */
#endif

        bit_depth = cfg.output_bit_depth;

        AUDDBG("bit depth: %d\n", bit_depth);
        switch (bit_depth) {
            case 32:
                output_fmt = FMT_S32_NE;
                break;
            case 24:
                output_fmt = FMT_S24_NE;
                break;
            case 16:
                output_fmt = FMT_S16_NE;
                break;
            default:
                AUDDBG("unsupported bit depth requested %d\n", bit_depth);
                output_fmt = FMT_S16_NE;
                break;
        }

        freeSAD();

        AUDDBG("initializing dithering engine for 2 stage conversion: fmt%d --> float -->fmt%d\n", fmt, output_fmt);
        input_sad_fmt.sample_format = sadfmt_from_afmt(fmt);
        if (input_sad_fmt.sample_format < 0) return FALSE;
        input_sad_fmt.fracbits = FMT_FRACBITS(fmt);
        input_sad_fmt.channels = nch;
        input_sad_fmt.channels_order = SAD_CHORDER_INTERLEAVED;
        input_sad_fmt.samplerate = 0;

        output_sad_fmt.sample_format = SAD_SAMPLE_FLOAT;
        output_sad_fmt.fracbits = 0;
        output_sad_fmt.channels = nch;
        output_sad_fmt.channels_order = SAD_CHORDER_INTERLEAVED;
        output_sad_fmt.samplerate = 0;

        sad_state_to_float = SAD_dither_init(&input_sad_fmt, &output_sad_fmt, &ret);
        if (sad_state_to_float == NULL) {
            AUDDBG("ditherer init failed (decoder's native --> float)\n");
            return FALSE;
        }
        SAD_dither_set_dither (sad_state_to_float, FALSE);

        input_sad_fmt.sample_format = SAD_SAMPLE_FLOAT;
        input_sad_fmt.fracbits = 0;
        input_sad_fmt.channels = nch;
        input_sad_fmt.channels_order = SAD_CHORDER_INTERLEAVED;
        input_sad_fmt.samplerate = 0;

        output_sad_fmt.sample_format = sadfmt_from_afmt(output_fmt);
        if (output_sad_fmt.sample_format < 0) return FALSE;
        output_sad_fmt.fracbits = FMT_FRACBITS(output_fmt);
        output_sad_fmt.channels = nch;
        output_sad_fmt.channels_order = SAD_CHORDER_INTERLEAVED;
        output_sad_fmt.samplerate = 0;

        sad_state_from_float = SAD_dither_init(&input_sad_fmt, &output_sad_fmt, &ret);
        if (sad_state_from_float == NULL) {
            SAD_dither_free(sad_state_to_float);
            AUDDBG("ditherer init failed (float --> output)\n");
            return FALSE;
        }
        SAD_dither_set_dither (sad_state_from_float, TRUE);

        fmt = output_fmt;

        if(replay_gain_info.album_peak == 0.0 && replay_gain_info.track_peak == 0.0) {
            AUDDBG("RG info isn't set yet. Filling replay_gain_info with default values.\n");
            replay_gain_info.track_gain = cfg.default_gain;
            replay_gain_info.track_peak = 0.01;
            replay_gain_info.album_gain = cfg.default_gain;
            replay_gain_info.album_peak = 0.01;
        }
        apply_replaygain_info(&replay_gain_info);

        return reopen_audio(fmt, rate, nch);
    } /* bypass_dsp */
}

void
output_write_audio(gpointer ptr, gint length)
{
    OutputPlugin *op = get_current_output_plugin();

    /* Sanity check. */
    if (op == NULL)
        return;

    plugin_set_current((Plugin *)op);
    op->write_audio(ptr, length);
}

void
output_close_audio(void)
{
    OutputPlugin *op = get_current_output_plugin();

    freeSAD();

    AUDDBG("clearing RG settings\n");
    replay_gain_info.track_gain = 0.0;
    replay_gain_info.track_peak = 0.0;
    replay_gain_info.album_gain = 0.0;
    replay_gain_info.album_peak = 0.0;

#ifdef USE_SAMPLERATE
    src_flow_free();
#endif
    /* Do not close if there are still songs to play and the user has
     * not requested a stop.  --nenolod
     */
    Playlist *pl = playlist_get_active();
    if (ip_data.stop == FALSE &&
       (playlist_get_position_nolock(pl) < playlist_get_length(pl) - 1) &&
       !cfg.stopaftersong &&
       !(cfg.no_playlist_advance && !cfg.repeat)) {
            AUDDBG("leaving audio opened\n");
            return;
        }

    /* Sanity check. */
    if (op == NULL)
        return;

    plugin_set_current((Plugin *)op);
    AUDDBG("closing audio\n");
    op->close_audio();
    AUDDBG("done\n");

    /* Reset the op_state. */
    op_state.fmt = op_state.rate = op_state.nch = 0;
    equalizer_flow_free();
}

void
output_flush(gint time)
{
    OutputPlugin *op = get_current_output_plugin();

    if (op == NULL)
        return;

    plugin_set_current((Plugin *)op);
    op->flush(time);
}

void
output_pause(gshort paused)
{
    OutputPlugin *op = get_current_output_plugin();

    if (op == NULL)
        return;

    plugin_set_current((Plugin *)op);
    op->pause(paused);
}

gint
output_buffer_free(void)
{
    OutputPlugin *op = get_current_output_plugin();

    if (op == NULL)
        return 0;

    plugin_set_current((Plugin *)op);
    return op->buffer_free();
}

gint
output_buffer_playing(void)
{
    OutputPlugin *op = get_current_output_plugin();

    if (op == NULL)
        return 0;

    plugin_set_current((Plugin *)op);
    return op->buffer_playing();
}

/* called by input plugin when data is ready */
void
output_pass_audio(InputPlayback *playback,
              AFormat fmt,       /* output format        */
              gint nch,          /* channels             */
              gint length,       /* length of sample     */
              gpointer ptr,      /* data                 */
              int *going         /* 0 when time to stop  */
              )
{
    static Flow *visualization_flow = NULL;
    static Flow *postproc_flow = NULL;
    static Flow *legacy_flow = NULL;
    OutputPlugin *op = playback->output;
    gint writeoffs;
    void * orig_ptr = ptr;
    void * old_ptr, * new_ptr;

    if (visualization_flow == NULL)
    {
        visualization_flow = flow_new();
        flow_link_element(visualization_flow, vis_flow);
    }

    plugin_set_current((Plugin *)(playback->output));
    gint time = playback->output->written_time();

    old_ptr = ptr;
    flow_execute(visualization_flow, time, &ptr, length, fmt, decoder_srate, nch);
    if (ptr != old_ptr && old_ptr != orig_ptr) free (old_ptr);

    if (!bypass_dsp) {

        if(length <= 0 || sad_state_from_float == NULL || sad_state_to_float == NULL) return;

        if (legacy_flow == NULL)
        {
            legacy_flow = flow_new();
            flow_link_element(legacy_flow, effect_flow);
        }

        if (postproc_flow == NULL)
        {
            postproc_flow = flow_new();
#ifdef USE_SAMPLERATE
            flow_link_element(postproc_flow, src_flow);
#endif
            flow_link_element(postproc_flow, equalizer_flow);
            flow_link_element(postproc_flow, volumecontrol_flow);
        }

        if (fmt != FMT_FLOAT)
        {
            if (IS_S16_LE (fmt))
            {
                int samples = length >> 1;

                length = sizeof (float) * samples;
                new_ptr = malloc (length);
                s16_le_to_float (ptr, new_ptr, samples);
                if (ptr != orig_ptr) free (ptr);
                ptr = new_ptr;
            }
            else
            {
                int frames = length / nch / FMT_SIZEOF (fmt);

                length = sizeof (float) * nch * frames;
                new_ptr = malloc (length);
                SAD_dither_process_buffer (sad_state_to_float, ptr, new_ptr,
                 frames);
                if (ptr != orig_ptr) free (ptr);
                ptr = new_ptr;
            }
        }

        old_ptr = ptr;
        length = flow_execute (postproc_flow, time, & ptr, length,
         FMT_FLOAT, decoder_srate, nch);
        if (ptr != old_ptr && old_ptr != orig_ptr) free (old_ptr);

        if (op_state.fmt != FMT_FLOAT)
        {
            if (cfg.no_dithering && IS_S16_LE (op_state.fmt))
            {
                int samples = length / sizeof (float);

                length = samples << 1;
                new_ptr = malloc (length);
                float_to_s16_le (ptr, new_ptr, samples);
                if (ptr != orig_ptr) free (ptr);
                ptr = new_ptr;
            }
            else
            {
                int frames = length / nch / sizeof (float);

                length = FMT_SIZEOF (op_state.fmt) * nch * frames;
                new_ptr = malloc (length);
                SAD_dither_process_buffer (sad_state_from_float, ptr, new_ptr,
                 frames);
                if (ptr != orig_ptr) free (ptr);
                ptr = new_ptr;
            }
        }

        if (IS_S16_NE (op_state.fmt))
        {
            old_ptr = ptr;
            length = flow_execute(legacy_flow, time, &ptr, length, op_state.fmt, op_state.rate, op_state.nch);
            if (ptr != old_ptr && old_ptr != orig_ptr) free (old_ptr);
        } else {
            AUDDBG("legacy_flow can deal only with S16_NE streams\n"); /*FIXME*/
        }
    } /* !bypass_dsp */

    /**** write it out ****/

    writeoffs = 0;
    while (writeoffs < length)
    {
        int writable = length - writeoffs;

        if (writable > 2048)
            writable = 2048;

        if (writable == 0)
            return;

        while (op->buffer_free() < writable)   /* wait output buf */
        {
            GTimeVal pb_abs_time;

            g_get_current_time(&pb_abs_time);
            g_time_val_add(&pb_abs_time, 10000);

            if (going && !*going)              /* thread stopped? */
                return;                        /* so finish */

            if (ip_data.stop)                  /* has a stop been requested? */
                return;                        /* yes, so finish */

            /* else sleep for retry */
            g_mutex_lock(playback->pb_change_mutex);
            g_cond_timed_wait(playback->pb_change_cond, playback->pb_change_mutex, &pb_abs_time);
            g_mutex_unlock(playback->pb_change_mutex);
        }

        if (ip_data.stop)
            return;

        if (going && !*going)                  /* thread stopped? */
            return;                            /* so finish */

        /* do output */
	plugin_set_current((Plugin *)op);
        op->write_audio(((guint8 *) ptr) + writeoffs, writable);

        writeoffs += writable;
    }

    if (ptr != orig_ptr) free (ptr);
}

/* called by input plugin when RG info available --asphyx */
void
output_set_replaygain_info (InputPlayback *pb, ReplayGainInfo *rg_info)
{
    replay_gain_info = *rg_info;
    apply_replaygain_info(rg_info);
}

static void
apply_replaygain_info (ReplayGainInfo *rg_info)
{
    SAD_replaygain_mode mode;
    SAD_replaygain_info info;
    gboolean rg_enabled;
    gboolean album_mode;

    if(sad_state_from_float == NULL) {
        AUDDBG("SAD not initialized!\n");
        return;
    }

    rg_enabled = cfg.enable_replay_gain;
    album_mode = cfg.replay_gain_album;
    mode.clipping_prevention = cfg.enable_clipping_prevention;
    mode.hard_limit = FALSE;
    mode.adaptive_scaler = cfg.enable_adaptive_scaler;

    if(!rg_enabled) return;

    mode.mode = album_mode ? SAD_RG_ALBUM : SAD_RG_TRACK;
    mode.preamp = cfg.replay_gain_preamp;

    info.present = TRUE;
    info.track_gain = rg_info->track_gain;
    info.track_peak = rg_info->track_peak;
    info.album_gain = rg_info->album_gain;
    info.album_peak = rg_info->album_peak;

    AUDDBG("Applying Replay Gain settings:\n");
    AUDDBG("* mode:                %s\n",     mode.mode == SAD_RG_ALBUM ? "album" : "track");
    AUDDBG("* clipping prevention: %s\n",     mode.clipping_prevention ? "yes" : "no");
    AUDDBG("* adaptive scaler      %s\n",     mode.adaptive_scaler ? "yes" : "no");
    AUDDBG("* preamp:              %+f dB\n", mode.preamp);
    AUDDBG("Replay Gain info for current track:\n");
    AUDDBG("* track gain:          %+f dB\n", info.track_gain);
    AUDDBG("* track peak:          %f\n",     info.track_peak);
    AUDDBG("* album gain:          %+f dB\n", info.album_gain);
    AUDDBG("* album peak:          %f\n",     info.album_peak);

    SAD_dither_apply_replaygain(sad_state_from_float, &info, &mode);
}

void output_plugin_cleanup(void)
{
  OutputPlugin *op = get_current_output_plugin();
   op->init();
   output_close_audio();
   printf("output plugin cleanupn\n");
}
void output_plugin_reinit(void)
{

    printf("output plugin reinit \n");
}
