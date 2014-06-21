/* hf.h - headers for height field class
 *
 * Copyright (C) 2001-2010 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _HF
#define _HF 1

#include "globals.h"
#include "camera.h"
#include <math.h>

typedef unsigned short int hf_type;
typedef guint hf_long; // We need a 4-byte integer, on any current architecture (i32, x64)

#ifndef MAX_HF_VALUE
#define MAX_HF_VALUE 0xFFFF
#endif

typedef struct {

//	Generic Height Field structure
	gint max_x;
	gint max_y;
	hf_type *hf_buf;		// Main HF buffer
	hf_type *tmp_buf;	// Temporary buffer
	hf_type *tmp2_buf;	// Temporary buffer #2
	hf_type *result_buf;	// Temporary result buffer, for some postprocessing
	hf_type *select_buf;
	hf_type *layer_buf;	// A layer, to add to the output buffer before display
				// Ex. a background canvas for drawing
	gboolean if_layer;	// Whether layer is active or not
	hf_type min;		// Mininum - maximum value in the buffer
	hf_type max;
	hf_type avrg;		// Average
	gboolean *if_tiles;	// Connected with a checkbox defined in the wrapper structure
	gboolean use_water;	// To be "transmitted" to the gl_preview struct
	gfloat water_level;	// To be "transmitted" to the gl_preview struct
} hf_struct_type;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} pix8_rgb;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} pix8_rgba;

typedef struct {
	unsigned short int r;
	unsigned short int g;
	unsigned short int b;
	unsigned short int a;
} pix16_rgba;

typedef struct {
	unsigned short int r;
	unsigned short int g;
	unsigned short int b;
} pix16_rgb;

//	Prototypes

gpointer hf_new(gint size);
void hf_backup(hf_struct_type *);
void hf_restore(hf_struct_type *);
void hf_free(hf_struct_type *);
hf_struct_type *hf_copy_new(hf_struct_type *from, gboolean flag_swap);
hf_struct_type *hf_read(gchar *path_n_file, gboolean *fname_tochoose, gchar **msg, gboolean *tiling_ptr);
void hf_write(hf_struct_type *hf, gchar *path_n_file, camera_struct *cameras[]);
void hf_set_tiling_ptr (hf_struct_type *hf, gboolean *tiling_ptr);

void hf_reset_buffer(hf_type *, gint maxx, gint maxy);
void hf_min_max(hf_struct_type *);

void hf_init_layer (hf_struct_type *);
void hf_cancel_layer (hf_struct_type *);
void hf_merge_layer (hf_struct_type *, gint merge_op);
void hf_init_result_buf_from_hf (hf_struct_type *);
void hf_init_tmp2_buf_from_hf (hf_struct_type *);
void hf_update_hf_with_layer (hf_struct_type *, gint merge_op);

void write_cameras_in_png (camera_struct *cameras[]);

#endif // _HF
