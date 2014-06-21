/* fill.h - headers for fill.c - filling pen / bucket
 *
 * Copyright (C) 2005 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         www.oricom.ca/patrice.st-gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include "hf.h"

#ifndef _FILL
#define _FILL 1

 #ifndef FILL_PREVIEW_SIZE
 #define FILL_PREVIEW_SIZE 64
 #endif

// Direction of filling

#ifndef FILL_UP
#define FILL_UP 0
#endif

#ifndef FILL_DOWN
#define FILL_DOWN 1
#endif

#ifndef FILL_BOTH
#define FILL_BOTH 2
#endif

// Area selection mode

#ifndef SELECT_REPLACE
#define SELECT_REPLACE 0
#endif

#ifndef SELECT_ADD
#define SELECT_ADD 1
#endif

#ifndef SELECT_SUBTRACT
#define SELECT_SUBTRACT 2
#endif

// Fill mode
// FILL_RANGE is the original behaviour (ex. for faults)
// Here we fill around the actual value of (x,y), in a ± range
// FILL_MAX is the mode required for the "paint bucket"
// Here we fill up to a max value

#ifndef FILL_RANGE
#define FILL_RANGE 0
#endif

#ifndef FILL_MAX
#define FILL_MAX 1
#endif

#ifndef MIN_FILLING_VALUE
#define MIN_FILLING_VALUE 1
#endif

typedef struct {
	gint x;
	gint y;
	gint select_mode; // Only relevant for SELECT_ADD and SELECT_SUBTRACT
} seed_coord;

typedef struct {
	gint select_mode;
	hf_type height; // Fill up to this height; also used as a selection criterion
			// (we select all the contiguous pixels less than this value)
	gint percent;	// We multiply the soil we are supposed to add by this value
			// For instance: original H = 1000; desired H = 20000; 
			// percent = 25; => fill up to 1000 + (19000*25%).
	GList *seeds_list;  // List of seed_coord
	gdouble noise_exponent;
} fill_struct;

fill_struct *fill_pen_new(hf_type default_height, gint default_select_mode, gint default_percent);

void fill_pen_free (fill_struct *f);

void fill_area (hf_type *buffer_in, hf_type *buffer_out, gint xmax, gint ymax,
	hf_type filling_value, hf_type range, gint x, gint y);
void fill_area_with_mode (hf_type *buffer_in, hf_type *buffer_out, gint xmax, gint ymax,
	hf_type filling_value, hf_type range, gint x, gint y, gint fill_mode, gint select_mode);

void select_for_fill (fill_struct *pen, hf_struct_type *hf, gint x, gint y);
void refresh_select_from_seeds (fill_struct *f, hf_struct_type *hf);

void add_seed (fill_struct *f, gint x, gint y);
void reset_seeds (fill_struct *f);

gboolean fill_from_selection (hf_type *input, hf_type *selection, hf_type *output, gint maxx, gint maxy, fill_struct *pen);

#endif // _FILL
