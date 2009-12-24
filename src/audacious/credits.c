/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2009  Audacious Team
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib/gi18n.h>
#include "credits.h"

static const gchar *audacious_brief =
    N_("<big><b>Audacious %s</b></big>\n"
       "An audio player for many platforms.\n"
       "Copyright (C) 2005-2009 Audacious Development Team");

static const gchar *credit_text[] = {
    N_("Core developers:"),
    "Christian Birchinger",
    "Michael Färber",
    "Matti Hämäläinen",
    "John Lindgren",
    "Cristi Măgherușan",
    "Tomasz Moń",
    "William Pitcock",
    "Jonathan Schleifer",
    "Ben Tucker",
    "Tony Vroon",
    "Yoshiki Yazawa",
    NULL,

    N_("Graphics:"),
    "George Averill",
    "Stephan Sokolow",
    NULL,

    N_("Default skin:"),
    "George Averill",
    "Michael Färber",
    "William Pitcock",
    NULL,

    N_("Plugin development:"),
    "Kiyoshi Aman",
    "Luca Barbato",
    "Daniel Barkalow",
    "Michael Färber",
    "Shay Green",
    "Matti Hämäläinen",
    "Sascha Hlusiak",
    "John Lindgren",
    "Giacomo Lozito",
    "Cristi Măgherușan",
    "Boris Mikhaylov",
    "Tomasz Moń",
    "Sebastian Pipping",
    "William Pitcock",
    "Derek Pomery",
    "Jonathan Schleifer",
    "Andrew O. Shadoura",
    "Tony Vroon",
    "Yoshiki Yazawa",
    NULL,

    N_("Patch authors:"),
    "Chris Arepantis",
    "Alexis Ballier",
    "Eric Barch",
    "Carlo Bramini",
    "Massimo Cavalleri",
    "Stefano D'Angelo",
    "Jean-Louis Dupond",
    "Laszlo Dvornik",
    "Ralf Ertzinger",
    "Mike Frysinger",
    "Mark Glines",
    "Hans de Goede",
    "Jussi Judin",
    "Teru KAMOGASHIRA",
    "Chris Kehler",
    "Michał Lipski",
    "Mark Loeser",
    "Alex Maclean",
    "Michael Hanselmann",
    "Joseph Jezak",
    "Henrik Johansson",
    "Rodrigo Martins de Matos Ventura",
    "Diego Pettenò",
    "Edward Sheldrake",
    "Kirill Shendrikowski",
    "Kazuki Shimura",
    "Valentine Sinitsyn",
    "Johan Tavelin",
    "Christoph J. Thompson",
    "Bret Towe",
    "John Wehle",
    "Tim Yamin",
    "Ivan N. Zlatev",
    NULL,

    N_("1.x developers:"),
    "George Averill",
    "Daniel Barkalow",
    "Christian Birchinger",
    "Daniel Bradshaw",
    "Adam Cecile",
    "Michael Färber",
    "Matti Hämäläinen",
    "Troels Bang Jensen",
    "Giacomo Lozito",
    "Cristi Măgherușan",
    "Tomasz Moń",
    "William Pitcock",
    "Derek Pomery",
    "Mohammed Sameer",
    "Jonathan Schleifer",
    "Ben Tucker",
    "Tony Vroon",
    "Yoshiki Yazawa",
    "Eugene Zagidullin",
    NULL,

    N_("BMP Developers:"),
    "Artem Baguinski",
    "Edward Brocklesby",
    "Chong Kai Xiong",
    "Milosz Derezynski",
    "David Lau",
    "Ole Andre Vadla Ravnaas",
    "Michiel Sikkes",
    "Andrei Badea",
    "Peter Behroozi",
    "Bernard Blackham",
    "Oliver Blin",
    "Tomas Bzatek",
    "Liviu Danicel",
    "Jon Dowland",
    "Artur Frysiak",
    "Sebastian Kapfer",
    "Lukas Koberstein",
    "Dan Korostelev",
    "Jolan Luff",
    "Michael Marineau",
    "Tim-Philipp Muller",
    "Julien Portalier",
    "Andrew Ruder",
    "Olivier Samyn",
    "Martijn Vernooij",
    NULL,

    NULL
};

static const gchar *translators_text[] = {
    N_("Basque:"),
    "Iñaki Larrañaga Murgoitio",
    NULL,
    N_("Brazilian Portuguese:"),
    "Fábio Antunes",
    "Philipi Pinto",
    NULL,
    N_("Breton:"),
    "Thierry Vignaud",
    NULL,
    N_("Bulgarian:"),
    "Andrew Ivanov",
    NULL,
    N_("Catalan:"),
    "Ernest Adrogué",
    NULL,
    N_("Croatian:"),
    "Marin Glibic",
    NULL,
    N_("Czech:"),
    "Petr Pisar",
    NULL,
    N_("Dutch:"),
    "Laurens Buhler",
    "Tony Vroon",
    NULL,
    N_("Estonian:"),
    "Ivar Smolin",
    NULL,
    N_("Finnish:"),
    "Pauli Virtanen",
    "Matti Hämäläinen",
    NULL,
    N_("French:"),
    "Adam Cecile",
    "Stanislas Zeller",
    "Stany Henry",
    NULL,
    N_("German:"),
    "Michael Färber",
    "Michael Hanselmann",
    "Matthias Debus",
    NULL,
    N_("Georgian:"),
    "George Machitidze",
    NULL,
    N_("Greek:"),
    "Kouzinopoulos Haris",
    "Stavros Giannouris",
    "Stathis Kamperis",
    NULL,
    N_("Hindi:"),
    "Dhananjaya Sharma",
    NULL,
    N_("Hungarian:"),
    "Laszlo Dvornik",
    NULL,
    N_("Italian:"),
    "Alessio D'Ascanio",
    "Diego Pettenò",
    NULL,
    N_("Japanese:"),
    "Dai",
    NULL,
    N_("Korean:"),
    "DongCheon Park",
    NULL,
    N_("Lithuanian:"),
    "Rimas Kudelis",
    NULL,
    N_("Macedonian:"),
    "Arangel Angov",
    NULL,
    N_("Polish:"),
    "Wojciech Myrda",
    NULL,
    N_("Romanian:"),
    "Daniel Patriche",
    "Cristi Măgherușan",
    NULL,
    N_("Russian:"),
    "Sergey V. Mironov",
    "Alexandr Orlov",
    NULL,
    N_("Serbian (Latin):"),
    "Strahinja Kustudić",
    NULL,
    N_("Serbian (Cyrillic):"),
    "Strahinja Kustudić",
    NULL,
    N_("Simplified Chinese:"),
    "Yang Zhang",
    NULL,
    N_("Slovak:"),
    "Andrej Herceg",
    NULL,
    N_("Spanish:"),
    "Gustavo D. Vranjes",
    NULL,
    N_("Swedish:"),
    "Martin Persenius",
    NULL,
    N_("Traditional Chinese:"),
    "Cheng-Wei Chien",
    "Sylecn Song",
    "Yang Zhang",
    NULL,
    N_("Turkish:"),
    "Murat Şenel",
    "Eren Turkay",
    NULL,
    N_("Ukrainian:"),
    "Mykola Lynnyk",
    NULL,
    N_("Welsh:"),
    "Edward Brocklesby",
    "William Pitcock",
    NULL,

    NULL
};

void
get_audacious_credits(const gchar ** brief, const gchar *** credits, const gchar ***translators)
{
    if (brief != NULL)
        *brief = audacious_brief;
    if (credits != NULL)
        *credits = credit_text;
    if (translators != NULL)
        *translators = translators_text;
}
