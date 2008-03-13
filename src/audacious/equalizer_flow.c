/*  Audacious - Cross-platform multimedia player
 *  Copyright (C) 2005-2008  Audacious team
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

#include <glib.h>
#include <math.h>
#include "main.h"
#include "plugin.h"
#include "flow.h"

#include "af_compat.h"
#include "equalizer_flow.h"

int equalizer_open(af_instance_t* af); /* af_equalizer.c */

static af_instance_t *eq = NULL;
static gint eq_nch = 0;
static gint eq_rate = 0;
static gboolean bands_changed = FALSE;

static void
equalizer_flow_reinit(gint rate, gint nch)
{
    af_data_t data;

    AUDDBG("\n");
    if(eq == NULL) return;

    data.rate = rate;
    data.nch = nch;
    data.bps = 4;
    data.format = AF_FORMAT_FLOAT_NE;
    eq->control(eq, AF_CONTROL_REINIT, &data);
}

void
equalizer_flow(FlowContext *context)
{
    af_data_t data;

    if(!cfg.equalizer_active || eq == NULL) return;

    if(context->fmt != FMT_FLOAT) {
        context->error = TRUE;
        return;
    }

    if(eq_nch != context->channels ||
       eq_rate != context->srate ||
       bands_changed) {
        equalizer_flow_reinit(context->srate, context->channels);
        eq_nch = context->channels;
        eq_rate = context->srate;
        bands_changed = FALSE;
    }
    
    data.nch = context->channels;
    data.audio = context->data;
    data.len = context->len;
    eq->play(eq, &data);
}

void
equalizer_flow_set_bands(gfloat pre, gfloat *bands)
{
    int i;
    af_control_ext_t ctl;
    gfloat b[10];
    gfloat adj = 0.0;
    AUDDBG("\n");
    
    if(eq == NULL) {
        eq = g_malloc(sizeof(af_instance_t));
        equalizer_open(eq);
    }

    for(i = 0; i < 10; i++)
        b[i] = bands[i] + pre;

    for(i = 0; i < 10; i++)
        if(fabsf(b[i]) > fabsf(adj)) adj = b[i];

    if(fabsf(adj) > EQUALIZER_MAX_GAIN) {
        adj = adj > 0.0 ? EQUALIZER_MAX_GAIN - adj : -EQUALIZER_MAX_GAIN - adj;
        for(i = 0; i < 10; i++) b[i] += adj;
    }

    ctl.arg = b;
    for(i = 0; i < AF_NCH; i++) {
        ctl.ch = i;
        eq->control(eq, AF_CONTROL_EQUALIZER_GAIN | AF_CONTROL_SET, &ctl);
    }

    bands_changed = TRUE;
}

void
equalizer_flow_free()
{
    AUDDBG("\n");
    if(eq != NULL) {
        eq->uninit(eq);
        g_free(eq);
        eq = NULL;
        eq_nch = 0;
        eq_rate = 0;
        bands_changed = FALSE;
    }
}