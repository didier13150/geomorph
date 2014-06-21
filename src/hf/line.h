/* line.h - headers for drawing polylines (fractal lines)
 *
 * Copyright (C) 2004 Patrice St-Gelais
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

#ifndef _LINE
#define _LINE 1

#include "hf.h"

//	Generalized polyline struct

typedef struct {
	gdouble x0;
	gdouble y0;
	gdouble x1;
	gdouble y1;
} segment_struct;

typedef struct {
	gint max_seg;
	gint cur_seg;
	gint alloc_seg;
	gint size; // tip size
	// The 1st segment should generally start at (0.0,0.0)
	// It facilitates rotations and translations
	gdouble transl_x;	// Offset for the first segment
	gdouble transl_y;
	// Scale is normally equal to the buf size we are drawing in
	gdouble scale_x;
	gdouble scale_y;
	// Indexes are normalized so they fit between 0.0 and 1.0
	segment_struct *segments;
} line_struct;

//	The fractal line struct used for drawing faults and linear cracks

typedef struct {
	line_struct *polyline;
	// Sudivision steps; number of segments = pow(2,steps)
	gint steps; 	// 0 to 10; default: 5
	// Random variation of subdivision points; x: axial; y: vertical
	gint seed;	// Seed for the random factors
	gint random_x;	// 0 to 100 %; default 0
	gint random_y;	// 0 to 100 %; default 50
	gdouble distribution; // 0.0 to 10.0
	gint width; // 0 to 50 pixels, translates to tip_size in *polyline
	gint random_w; // The width would vary from node to node (future enhancement!)
	gboolean if_cracks; 	// Some outgrowth... typically for faults
					// When TRUE, draw sparse spikes in the line,
					// looking like cracks. Default FALSE.
	gint cracks_width;	// Relative width, 1 to 4, default 2
	gint cracks_depth;	// Relative value, 0 to 100, default 50%
	gint cracks_branching_steps; // We want tree-like structures in the future...
} fractal_line_struct;

/************************ Prototypes for public functions **********************/

line_struct *line_new(gint max_seg, gint tip_size);
void add_segment (line_struct *line, gdouble x0, gdouble y0, gdouble x1, gdouble y1);
void extend_line (line_struct *l, gint new_max_seg);
void reset_line (line_struct *l, gint max_seg, gint tip_size);
void set_line_translation (line_struct *l, gint x, gint y, gint sizex, gint sizey);
void set_line_scale (line_struct *l, gdouble sizex, gdouble sizey);

void draw_thin_segment_in_buf (segment_struct *segment,
	gdouble translate_x, gdouble translate_y,
	gpointer buf, gint max_x, gint max_y, gpointer value_ptr,
	gint data_type);

void draw_n_transform_all_segments (line_struct *l, gdouble dx, gdouble dy,
	gdouble unscaled_transl_x, gdouble unscaled_transl_y,
	gpointer buf, gint max_x, gint max_y, gpointer value_ptr,
	gint data_type, gint overflow);

void divide_n_draw (fractal_line_struct *f, gint level, gdouble x0, gdouble y0,
	gdouble x1, gdouble y1, gdouble length, gdouble cos, gdouble sin);

void init_fractal_polyline (
	fractal_line_struct*polyline,
	gint sizex, gint sizey,
	gint x0, gint y0,
	gint x1, gint y1);

fractal_line_struct *fractal_polyline_new ( gint steps,
	gint random_x, gint random_y,
	gint width, gint random_w,
	gint seed, gdouble distribution,
	gboolean if_cracks, gint cracks_depth, gint cracks_width,
	gint cracks_branching_steps ) ;

void fractal_polyline_free (fractal_line_struct *f);

#endif // _LINE
