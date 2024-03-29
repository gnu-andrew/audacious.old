/*
 * main.h
 * Copyright 2011 John Lindgren
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

#ifndef _AUDACIOUS_MAIN_H
#define _AUDACIOUS_MAIN_H

#include <glib.h>

/* adder.c */
void adder_init (void);
void adder_cleanup (void);

/* chardet.c */
void chardet_init (void);

/* config.c */
void config_load (void);
void config_save (void);
void config_cleanup (void);

/* main.c */
extern gboolean headless;
gboolean do_autosave (void);

/* mpris-signals.c */
void mpris_signals_init (void);
void mpris_signals_cleanup (void);

/* signals.c */
void signals_init (void);

/* smclient.c */
void smclient_init (void);

#endif
