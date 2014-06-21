/* draw.h - headers for drawing smooth (gaussian) wide lines
 *
 * Copyright (C) 2003 Patrice St-Gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _DRAW
#define _DRAW 1

#include "hf.h"
#include "hf_filters.h"

#ifndef MAX_PEN_RADIUS
	#define MAX_PEN_RADIUS 128
#endif

#ifndef SMOOTHING_NONE
	#define SMOOTHING_NONE 0
#endif
#ifndef SMOOTHING_ALL
	#define SMOOTHING_ALL 1
#endif
#ifndef SMOOTHING_AUTO
	#define SMOOTHING_AUTO 2
#endif

// Radius under which we interpolate for removing aliasing (SMOOTHING_AUTO only)
#ifndef INTERPOLATE_THRESHOLD
	#define INTERPOLATE_THRESHOLD 40
#endif

#ifndef SPACING_LOW_QUALITY
	#define SPACING_LOW_QUALITY 0.4
#endif
#ifndef SPACING_STANDARD_QUALITY
	#define SPACING_STANDARD_QUALITY 0.1
#endif
#ifndef SPACING_HIGH_QUALITY
	#define SPACING_HIGH_QUALITY 0.05
#endif
#ifndef SPACING_VHIGH_QUALITY
	#define SPACING_VHIGH_QUALITY 0.025
#endif

// Level of the map to add for obtaining a maximum close to 0xFFFF with the current spacing
#ifndef LEVEL_VHQ_SPACING
	#define LEVEL_VHQ_SPACING 3907.0 // for line spacing = 0.025
#endif
#ifndef LEVEL_HQ_SPACING
	#define LEVEL_HQ_SPACING 7815.0 // for line spacing = 0.05
#endif
#ifndef LEVEL_SQ_SPACING
	#define LEVEL_SQ_SPACING 15625.0 // for line spacing = 0.1
#endif
#ifndef LEVEL_LQ_SPACING
//	#define LEVEL_LQ_SPACING 33250.0 // for line spacing = 0.2
	#define LEVEL_LQ_SPACING 53250.0 // for line spacing = 0.2
#endif

//	draw_buf stores the last pixels drawn, added up
//	It's a "moving window"
//	Required for avoiding overflow when a stroke crosses itself
typedef struct {
	gint x;
	gint y;
	gdouble scal_prod;
} tail_struct;

typedef struct {
	hf_type *data;
	hf_type *tmp;
	gint size;		// Usually the size of the pen tip
	gboolean active;
	gboolean delayed_dot; // Drawing the first dot may be postponed,
		// when we have to rotate it because the pen tip is
		// not symmetrical
	tail_struct *tail; // "Tail" used to control overlaping
	gint max_tail; // Max size of the tail (allocated)
	// max_tail == 1 / relative spacing
	gint current_tail; // Current tail pointer (modulo max_tail)
} draw_buf;

typedef struct {			// Structure containing the pixel map to draw
	hf_type *data;		// Contains one quadrant only when square_symmetry is TRUE
					// (ex. a 11x11 map for a 21x21 image)
	hf_type *tmp;		// Copy of data, required by some processes (ex. rotated)
	hf_type *map_to_use; // Copy of the ->tmp or ->data pointer
		// Must not be allocated per itself
	hf_type *units;	// Matrix of 1s (for counting purposes)
	gint radius;			// Width or length = (2*radius) + 1
	gboolean square_symmetry;	// Relates to the map structure (when TRUE, only one quadrant is used)
	draw_buf *dr_buf;	// Moving window containing the last pixels drawn
					// Size = width or diameter of the pen tip
} map_struct;


typedef struct {
	gint size;	// Diameter
	gint level;	// From 0 to 100 %, related to the grey level, from 0 to 0xFFFF
	gint merge;  	// ADD, SUBTRACT, SMOOTH_OP
	gint wrap;		// TILING_AUTO, TILING_YES, TILING_NO
	gint smoothing;	// SMOOTHING_NONE, SMOOTHING_ALL, SMOOTHING_AUTO
	gint shape;		// Same as wave shapes (see waves.h)
	gdouble spacing;	// Ratio from 0.0 to 1.0: SPACING_LOW_QUALITY (0.15),
					// SPACING_STANDARD_QUALITY (0.1), SPACING_HIGH_QUALITY (0.5)
	map_struct *map;		// The map which is actually the pen tip
	gboolean overlap; // TRUE: the stroke overlap when crossing itself
			// FALSE: it "intersects" with itself
} pen_struct;

/************************ Prototypes for public functions **********************/

gboolean draw_continuous_line_by_dot (hf_struct_type *hf, pen_struct *pen,
	gdouble begin_x, gdouble begin_y,
	gdouble *end_x_ptr, gdouble *end_y_ptr,
	gboolean draw_end, gdouble **gauss_list);

void draw_one_dot (hf_struct_type *hf, pen_struct *pen, gint x, gint y, gdouble **gauss_list) ;

map_struct *map_new ();
void map_init (map_struct *map, gint diameter, gint level, gint shape, gdouble spacing) ;
void map_free(map_struct *);

void draw_buf_init (map_struct *, gdouble spacing);
void translate_draw_buf (map_struct *, gint new_x, gint new_y, gint shape);

pen_struct *pen_new();
void pen_free(pen_struct *);

#endif // _DRAW
