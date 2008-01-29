/* Scale & Dither library (libSAD)
 * High-precision bit depth converter with ReplayGain support
 *
 * (c)2007 by Eugene Zagidullin (e.asphyx@gmail.com)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "common.h"
#include "buffer.h"
#include "dither.h"

/* buffer ops: */
/********** 
 * 8-bit  *
 **********/

/* signed */
static sad_sint32 get_s8_i_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint8*)buf)[i*nch+ch];
}

static sad_sint32 get_s8_s_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint8**)buf)[ch][i];
}

static void put_s8_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_sint8*)buf)[i*nch+ch] = (sad_sint8)sample;
}

static void put_s8_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_sint8**)buf)[ch][i] = (sad_sint8)sample;
}

/* unsigned */
static sad_sint32 get_u8_i_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)(((sad_uint8*)buf)[i*nch+ch]) - 128;
}

static sad_sint32 get_u8_s_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)(((sad_uint8**)buf)[ch][i]) - 128;
}

static void put_u8_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint8*)buf)[i*nch+ch] = (sad_uint8)sample + 128;
}

static void put_u8_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint8**)buf)[ch][i] = (sad_uint8)sample + 128;
}

static SAD_buffer_ops buf_s8_i_ops = {
  &get_s8_i_sample,
  &put_s8_i_sample
};

static SAD_buffer_ops buf_s8_s_ops = {
  &get_s8_s_sample,
  &put_s8_s_sample
};

static SAD_buffer_ops buf_u8_i_ops = {
  &get_u8_i_sample,
  &put_u8_i_sample
};

static SAD_buffer_ops buf_u8_s_ops = {
  &get_u8_s_sample,
  &put_u8_s_sample
};

/**********
 * 16-bit *
 **********/

/* signed */
static sad_sint32 get_s16_i_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)(((sad_sint16*)buf)[i*nch+ch]);
}

static sad_sint32 get_s16_s_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)(((sad_sint16**)buf)[ch][i]);
}

static void put_s16_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
/*#ifdef DEEP_DEBUG
  printf("f: put_s16_i_sample\n");
#endif*/
  ((sad_sint16*)buf)[i*nch+ch] = (sad_sint16)sample;
}

static void put_s16_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
/*#ifdef DEEP_DEBUG
  printf("f: put_s16_s_sample\n");
#endif*/
  ((sad_sint16**)buf)[ch][i] = (sad_sint16)sample;
}

/* unsigned */
static sad_sint32 get_u16_i_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint32)(((sad_uint16*)buf)[i*nch+ch])) - 32768;
}

static sad_sint32 get_u16_s_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint32)(((sad_uint16**)buf)[ch][i])) - 32768;
}

static void put_u16_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint16*)buf)[i*nch+ch] = (sad_uint16)(sample + 32768);
}

static void put_u16_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint16**)buf)[ch][i] = (sad_uint16)(sample + 32768);
}

static SAD_buffer_ops buf_s16_i_ops = {
  &get_s16_i_sample,
  &put_s16_i_sample
};

static SAD_buffer_ops buf_s16_s_ops = {
  &get_s16_s_sample,
  &put_s16_s_sample
};

static SAD_buffer_ops buf_u16_i_ops = {
  &get_u16_i_sample,
  &put_u16_i_sample
};

static SAD_buffer_ops buf_u16_s_ops = {
  &get_u16_s_sample,
  &put_u16_s_sample
};

/**********
 * 24-bit *
 **********/

/*expand 24-bit signed value to 32-bit*/
#define EXPAND_24_TO_32(x) (((sad_sint32)(((x) & 0x00ffffff) << 8)) >> 8)

/* signed */
static sad_sint32 get_s24_i_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)EXPAND_24_TO_32(((sad_sint32*)buf)[i*nch+ch]);
}

static sad_sint32 get_s24_s_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)EXPAND_24_TO_32(((sad_sint32**)buf)[ch][i]);
}

static void put_s24_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_sint32*)buf)[i*nch+ch] = (sad_sint32)sample & 0x00ffffff;
}

static void put_s24_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_sint32**)buf)[ch][i] = (sad_sint32)sample & 0x00ffffff;
}

/* unsigned */
static sad_sint32 get_u24_i_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)EXPAND_24_TO_32(((sad_uint32*)buf)[i*nch+ch]) - 8388608;
}

static sad_sint32 get_u24_s_sample (void *buf, int nch, int ch, int i) {
  return (sad_sint32)EXPAND_24_TO_32(((sad_uint32**)buf)[ch][i]) - 8388608;
}

static void put_u24_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint32*)buf)[i*nch+ch] = ((sad_uint32)sample + 8388608) & 0x00ffffff;
}

static void put_u24_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint32**)buf)[ch][i] = ((sad_uint32)sample + 8388608) & 0x00ffffff;
}

static SAD_buffer_ops buf_s24_i_ops = {
  &get_s24_i_sample,
  &put_s24_i_sample
};

static SAD_buffer_ops buf_s24_s_ops = {
  &get_s24_s_sample,
  &put_s24_s_sample
};

static SAD_buffer_ops buf_u24_i_ops = {
  &get_u24_i_sample,
  &put_u24_i_sample
};

static SAD_buffer_ops buf_u24_s_ops = {
  &get_u24_s_sample,
  &put_u24_s_sample
};

/**********
 * 32-bit *
 **********/

/* signed */
static sad_sint32 get_s32_i_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint32*)buf)[i*nch+ch];
}

static sad_sint32 get_s32_s_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint32**)buf)[ch][i];
}

static void put_s32_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_sint32*)buf)[i*nch+ch] = (sad_sint32)sample;
}

static void put_s32_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_sint32**)buf)[ch][i] = (sad_sint32)sample;
}

/* unsigned */
static sad_sint32 get_u32_i_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint32*)buf)[i*nch+ch] - (sad_sint32)(1L<<31);
}

static sad_sint32 get_u32_s_sample (void *buf, int nch, int ch, int i) {
  return ((sad_sint32**)buf)[ch][i] - (sad_sint32)(1L<<31);
}

static void put_u32_i_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint32*)buf)[i*nch+ch] = (sad_uint32)(sample + (sad_sint32)(1L<<31));
}

static void put_u32_s_sample (void *buf, sad_sint32 sample, int nch, int ch, int i) {
  ((sad_uint32**)buf)[ch][i] = (sad_uint32)(sample + (sad_sint32)(1L<<31));
}

static SAD_buffer_ops buf_s32_i_ops = {
  &get_s32_i_sample,
  &put_s32_i_sample
};

static SAD_buffer_ops buf_s32_s_ops = {
  &get_s32_s_sample,
  &put_s32_s_sample
};

static SAD_buffer_ops buf_u32_i_ops = {
  &get_u32_i_sample,
  &put_u32_i_sample
};

static SAD_buffer_ops buf_u32_s_ops = {
  &get_u32_s_sample,
  &put_u32_s_sample
};

static SAD_buffer_ops *SAD_buffer_optable[SAD_SAMPLE_MAX][SAD_CHORDER_MAX] = {
  {&buf_s8_i_ops, &buf_s8_s_ops},	/* SAD_SAMPLE_S8     */
  {&buf_u8_i_ops, &buf_u8_s_ops},	/* SAD_SAMPLE_U8     */
  {&buf_s16_i_ops, &buf_s16_s_ops},	/* SAD_SAMPLE_S16    */
  {NULL,           NULL},		/* SAD_SAMPLE_S16_LE */
  {NULL,           NULL},		/* SAD_SAMPLE_S16_BE */
  {&buf_u16_i_ops, &buf_u16_s_ops},	/* SAD_SAMPLE_U16    */
  {NULL,           NULL}, 		/* SAD_SAMPLE_U16_LE */
  {NULL,           NULL}, 		/* SAD_SAMPLE_U16_BE */
  {&buf_s24_i_ops, &buf_s24_s_ops},	/* SAD_SAMPLE_S24    */
  {NULL,           NULL}, 		/* SAD_SAMPLE_S24_LE */
  {NULL,           NULL}, 		/* SAD_SAMPLE_S24_BE */
  {&buf_u24_i_ops, &buf_u24_s_ops},	/* SAD_SAMPLE_U24    */
  {NULL,           NULL}, 		/* SAD_SAMPLE_U24_LE */
  {NULL,           NULL}, 		/* SAD_SAMPLE_U24_BE */
  {&buf_s32_i_ops, &buf_s32_s_ops}, 	/* SAD_SAMPLE_S32    */
  {NULL,           NULL}, 		/* SAD_SAMPLE_S32_LE */
  {NULL,           NULL}, 		/* SAD_SAMPLE_S32_BE */
  {&buf_u32_i_ops, &buf_u32_s_ops}, 	/* SAD_SAMPLE_U32    */
  {NULL,           NULL}, 		/* SAD_SAMPLE_U32_LE */
  {NULL,           NULL}, 		/* SAD_SAMPLE_U32_BE */
  {&buf_s32_i_ops, &buf_s32_s_ops}, 	/* SAD_SAMPLE_FIXED32*/
  {NULL,           NULL}		/* SAD_SAMPLE_FLOAT  */
};

SAD_buffer_ops* SAD_assign_buf_ops (SAD_buffer_format *format) {
#ifdef DEBUG
  printf("f: SAD_assign_buf_ops\n");
#endif
  if (format->sample_format < SAD_SAMPLE_MAX && format->channels_order < SAD_CHORDER_MAX) {
    return SAD_buffer_optable[format->sample_format][format->channels_order];
  } else {
    return NULL;
  }
}

