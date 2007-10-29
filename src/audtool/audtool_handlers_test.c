/*
 * Audtool2
 * Copyright (c) 2007 Audacious development team
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <mowgli.h>
#include <locale.h>
#include "libaudclient/audctrl.h"
#include "audtool.h"

void test_activate(gint argc, gchar **argv)
{
    audacious_remote_activate(dbus_proxy);
}

void test_enqueue_to_temp(gint argc, gchar **argv)
{
    gint playpos;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	playpos = atoi(argv[1]);

	if (playpos < 1 || playpos > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", playpos);
		return;
	}

    audacious_remote_get_playqueue_queue_position(dbus_proxy, playpos - 1); // xxx playpos - 1?
}

void test_toggle_aot(gint argc, gchar **argv)
{
    gboolean ontop;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <boolean>", argv[0]);
		return;
	}

	ontop = (gboolean)atoi(argv[1]);
    printf("ontop = %d\n", ontop);
    audacious_remote_toggle_aot(dbus_proxy, ontop);
}

void test_get_skin(gint argc, gchar **argv)
{
    gchar *skin = NULL;
    skin = audacious_remote_get_skin(dbus_proxy);
    printf("skin = %s\n", skin);
    g_free(skin);
}

void test_set_skin(gint argc, gchar **argv)
{
	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <skin>", argv[0]);
		return;
	}
    printf("argc = %d\n", argc);
    printf("skin = %p\n", argv[1]);


    if(!argv[1] || !strcmp(argv[1], ""))
       return;

    audacious_remote_set_skin(dbus_proxy, argv[1]);
}

void test_get_info(gint argc, gchar **argv)
{
    gint rate, freq, nch;

    audacious_remote_get_info(dbus_proxy, &rate, &freq, &nch);
    printf("rate = %d freq = %d nch = %d\n", rate, freq, nch);
}




#if 0
void playlist_reverse(gint argc, gchar **argv)
{
	audacious_remote_playlist_prev(dbus_proxy);
}

void playlist_advance(gint argc, gchar **argv)
{
	audacious_remote_playlist_next(dbus_proxy);
}

void playlist_add_url_string(gint argc, gchar **argv)
{
	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <url>", argv[0]);
		return;
	}

	audacious_remote_playlist_add_url_string(dbus_proxy, argv[1]);
}

void playlist_delete(gint argc, gchar **argv)
{
	gint playpos;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	playpos = atoi(argv[1]);

	if (playpos < 1 || playpos > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", playpos);
		return;
	}

	audacious_remote_playlist_delete(dbus_proxy, playpos - 1);
}

void playlist_length(gint argc, gchar **argv)
{
	gint i;

	i = audacious_remote_get_playlist_length(dbus_proxy);

	audtool_report("%d", i);
}

void playlist_song(gint argc, gchar **argv)
{
	gint playpos;
	gchar *song;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	playpos = atoi(argv[1]);

	if (playpos < 1 || playpos > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", playpos);
		return;
	}

	song = audacious_remote_get_playlist_title(dbus_proxy, playpos - 1);

	audtool_report("%s", song);
}


void playlist_song_length(gint argc, gchar **argv)
{
	gint playpos, frames, length;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	playpos = atoi(argv[1]);

	if (playpos < 1 || playpos > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", playpos);
		return;
	}

	frames = audacious_remote_get_playlist_time(dbus_proxy, playpos - 1);
	length = frames / 1000;

	audtool_report("%d:%.2d", length / 60, length % 60);
}

void playlist_song_length_seconds(gint argc, gchar **argv)
{
	gint playpos, frames, length;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	playpos = atoi(argv[1]);

	if (playpos < 1 || playpos > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", playpos);
		return;
	}

	frames = audacious_remote_get_playlist_time(dbus_proxy, playpos - 1);
	length = frames / 1000;

	audtool_report("%d", length);
}

void playlist_song_length_frames(gint argc, gchar **argv)
{
	gint playpos, frames;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	playpos = atoi(argv[1]);

	if (playpos < 1 || playpos > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", playpos);
		return;
	}

	frames = audacious_remote_get_playlist_time(dbus_proxy, playpos - 1);

	audtool_report("%d", frames);
}

void playlist_display(gint argc, gchar **argv)
{
	gint i, ii, frames, length, total;
	gchar *songname;
	gchar *fmt = NULL, *p;
	gint column;

	i = audacious_remote_get_playlist_length(dbus_proxy);

	audtool_report("%d track%s.", i, i != 1 ? "s" : "");

	total = 0;

	for (ii = 0; ii < i; ii++)
	{
		songname = audacious_remote_get_playlist_title(dbus_proxy, ii);
		frames = audacious_remote_get_playlist_time(dbus_proxy, ii);
		length = frames / 1000;
		total += length;

		/* adjust width for multi byte characters */
		column = 60;
		if(songname){
			p = songname;
			while(*p){
				gint stride;
				stride = g_utf8_next_char(p) - p;
				if(g_unichar_iswide(g_utf8_get_char(p))
#if ( (GLIB_MAJOR_VERSION == 2) && (GLIB_MINOR_VERSION >= 12) )
				   || g_unichar_iswide_cjk(g_utf8_get_char(p))
#endif
                                ){
					column += (stride - 2);
				}
				else {
					column += (stride - 1);
				}
				p = g_utf8_next_char(p);
			}

		}

		fmt = g_strdup_printf("%%4d | %%-%ds | %%d:%%.2d", column);
		audtool_report(fmt, ii + 1, songname, length / 60, length % 60);
		g_free(fmt);
	}

	audtool_report("Total length: %d:%.2d", total / 60, total % 60);
}

void playlist_position(gint argc, gchar **argv)
{
	gint i;

	i = audacious_remote_get_playlist_pos(dbus_proxy);

	audtool_report("%d", i + 1);
}

void playlist_song_filename(gint argc, gchar **argv)
{
	gint i;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	i = atoi(argv[1]);

	if (i < 1 || i > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", i);
		return;
	}

	audtool_report("%s", audacious_remote_get_playlist_file(dbus_proxy, i - 1));
}

void playlist_jump(gint argc, gchar **argv)
{
	gint i;

	if (argc < 2)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <position>", argv[0]);
		return;
	}

	i = atoi(argv[1]);

	if (i < 1 || i > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", i);
		return;
	}

	audacious_remote_set_playlist_pos(dbus_proxy, i - 1);
}

void playlist_clear(gint argc, gchar **argv)
{
	audacious_remote_playlist_clear(dbus_proxy);
}

void playlist_repeat_status(gint argc, gchar **argv)
{
	if (audacious_remote_is_repeat(dbus_proxy))
	{
		audtool_report("on");
		return;
	}
	else
	{
		audtool_report("off");
		return;
	}
}

void playlist_repeat_toggle(gint argc, gchar **argv)
{
	audacious_remote_toggle_repeat(dbus_proxy);
}

void playlist_shuffle_status(gint argc, gchar **argv)
{
	if (audacious_remote_is_shuffle(dbus_proxy))
	{
		audtool_report("on");
		return;
	}
	else
	{
		audtool_report("off");
		return;
	}
}

void playlist_shuffle_toggle(gint argc, gchar **argv)
{
	audacious_remote_toggle_shuffle(dbus_proxy);
}

void playlist_tuple_field_data(gint argc, gchar **argv)
{
	gint i;
	gpointer data;

	if (argc < 3)
	{
		audtool_whine("invalid parameters for %s.", argv[0]);
		audtool_whine("syntax: %s <fieldname> <position>", argv[0]);
		audtool_whine("  - fieldname example choices: performer, album_name,");
		audtool_whine("      track_name, track_number, year, date, genre, comment,");
		audtool_whine("      file_name, file_ext, file_path, length, formatter,");
		audtool_whine("      custom, mtime");
		return;
	}

	i = atoi(argv[2]);

	if (i < 1 || i > audacious_remote_get_playlist_length(dbus_proxy))
	{
		audtool_whine("invalid playlist position %d", i);
		return;
	}

	if (!(data = audacious_get_tuple_field_data(dbus_proxy, argv[1], i - 1)))
	{
		return;
	}
	
	if (!g_ascii_strcasecmp(argv[1], "track_number") || !g_ascii_strcasecmp(argv[1], "year") || !g_ascii_strcasecmp(argv[1], "length") || !g_ascii_strcasecmp(argv[1], "mtime"))
	{
		if (*(gint *)data > 0)
		{
			audtool_report("%d", *(gint *)data);
		}
		return;
	}

	audtool_report("%s", (gchar *)data);
}
#endif