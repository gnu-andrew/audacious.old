/*
 * Copyright 2009 Paula Stanciu
 *
 * This file is part of Audacious.
 *
 * Audacious is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3 of the License.
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

#include <glib.h>
#include <libaudcore/tuple.h>
#include <libaudcore/vfs.h>
#include "util.h"
#include "tag_module.h"
#include "wma/module.h"
#include "id3/id3.h"
#include "ape/ape.h"
#include "aac/aac.h"

void init_tag_modules(void)
{
    mowgli_node_add((void *)&wma, mowgli_node_create(), &tag_modules);
    mowgli_node_add((void *)&id3, mowgli_node_create(), &tag_modules);
    mowgli_node_add((void *)&ape, mowgli_node_create(), &tag_modules);
/*
    mowgli_node_add((void *)&aac, mowgli_node_create(), &tag_modules);
*/
}

tag_module_t *find_tag_module(VFSFile * fd)
{
    mowgli_node_t *mod, *tmod;
    MOWGLI_LIST_FOREACH_SAFE(mod, tmod, tag_modules.head)
    {
        vfs_fseek(fd, 0, SEEK_SET);
        if (((tag_module_t *) (mod->data))->can_handle_file(fd))
            return (tag_module_t *) (mod->data);
    }

    AUDDBG("no module found\n");
    return NULL;
}
