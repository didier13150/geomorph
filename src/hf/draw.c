/* draw.c:  Drawing functions (continuous stroke) in a hf_struct
 *
 * Copyright (C) 2003-2004 Patrice St-Gelais
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

#include <string.h>
#include "draw.h"
#include "waves.h"
#define MULT_SIZE 2
#define DIV_SIZE 1

pen_struct *pen_new() {
	pen_struct *p;
	p = (pen_struct *) x_malloc(sizeof(pen_struct), "pen_struct");
	p->size = 83;
	p->level = 25; 	// 1% to 100%
	p->smoothing = SMOOTHING_AUTO;
	p->spacing = SPACING_HIGH_QUALITY;
	p->merge = ADD;
	p->wrap = TILING_AUTO;
	p->shape = NO_WAVE_SHAPE;
	p->map = map_new();
	p->overlap = FALSE;
	map_init(p->map, p->size, p->level, p->shape, p->spacing) ;
	return p;
}

void pen_free(pen_struct *ps) {
	if (!ps)
		return;
	if (ps->map)
		map_free(ps->map);
	x_free(ps);
}

draw_buf *draw_buf_new (gint size) {
	//	Buffer containing a "moving window" of the last pixels drawn
	//	Allows a proper merging of the pixels drawn with the original image.
	//	so that there are no holes nor superimposition when the angles
	//	of successive and connected segments are not the same
	draw_buf *db;
	db = (draw_buf *) x_malloc(sizeof(draw_buf), "draw_buf");
	db->size = size;		// Diameter
	db->data = (hf_type *) x_calloc(db->size * db->size,sizeof(hf_type), "hf_type (db->data in draw_buf_new)");
	db->tmp = (hf_type *) x_calloc(db->size * db->size,sizeof(hf_type), "hf_type (db->tmp in draw_buf_new)");
	db->active = FALSE;
	db->tail = NULL;
	db->max_tail = 0;
	db->delayed_dot = FALSE;
	return db;
}

void draw_buf_init (map_struct *map, gdouble spacing) {
	// Reinitialize the draw buffer to 0 - public method
	gint i;
	draw_buf *d=map->dr_buf;
	d->active = FALSE;
	for (i=0; i<(d->size*d->size); i++) {
		*(d->data+i) = (hf_type) 0;
	}
	d->max_tail = (gint)  (1.5+(1.0 / spacing));
//	printf("MAX TAIL: %d\n",d->max_tail);
	d->current_tail = d->max_tail-1; // before the 1st
	if (d->tail)
		x_free(d->tail);
	d->tail = (tail_struct*) x_calloc(d->max_tail,sizeof(tail_struct), "tail_struct (d->tail in draw_buf_init)");
	d->delayed_dot = FALSE;
}

void draw_buf_reinit (map_struct *map) {
	// Reinitialize the draw buffer to 0 - public method
	gint i, j, ii;
	draw_buf *d = map->dr_buf;
//	printf("RADIUS: %d; SIZE: %d\n",map->radius, d->size);
	for (i=0; i<d->size; i++)
		for (j=0; j<d->size; j++) {
			ii = i + j*d->size;
//			*(d->data+ii) = 0.7 * *(d->data+ii);
			*(d->data+ii) = 0;
		}
	if (d->tail)
		x_free(d->tail);
	d->tail = (tail_struct *) x_calloc(d->max_tail,sizeof(tail_struct), "tail_struct (d->tail in draw_buf_reinit)" );
	d->current_tail = d->max_tail-1; // before the 1st
}


void subtract_older_dot (map_struct *map) {
	draw_buf *d;
	gint older_x, older_y, current_x, current_y;
	gint map_size = 2*map->radius+1;
//	printf("Subtract older dot\n");
	d = map->dr_buf;
	older_x = (d->tail+(d->current_tail+1)%d->max_tail)->x;
	older_y = (d->tail+(d->current_tail+1)%d->max_tail)->y;
	if ((!older_x) && (!older_y))
		return;
	current_x = (d->tail+d->current_tail)->x;
	current_y = (d->tail+d->current_tail)->y;

	generalized_merge( (gpointer) map->data,
		HF_TYPE_ID,
		map_size, map_size, 
		d->data, HF_TYPE_ID, 
		d->size, d->size, 
		d->size/2 - current_x + older_x,
		d->size/2 - current_y + older_y,
		SUBTRACT,
		FALSE, // no wrap
		map->square_symmetry);
}

void translate_draw_buf (map_struct *map, gint new_x, gint new_y, gint shape) {
	gint dx, dy, x, y, s, sx, sy;
	gint previous_x, previous_y, previous_previous_x, previous_previous_y;
	gdouble scal_prod;
	hf_type *map_to_use;
	draw_buf *d = map->dr_buf;
	gint map_size = 2*map->radius+1;
	dx = (d->tail+d->current_tail)->x - new_x;
	dy = (d->tail+d->current_tail)->y - new_y;

	previous_x = (d->tail+d->current_tail)->x;
	previous_y = (d->tail+d->current_tail)->y;

	previous_previous_x = (d->tail+((d->current_tail+d->max_tail-1)%d->max_tail))->x;
	previous_previous_y = (d->tail+((d->current_tail+d->max_tail-1)%d->max_tail))->y;
	// The scalar product
	// is used as an approximation of the curvature
	// Higher curvature -> drawing is less additive

	scal_prod = NORM_SCALAR_PRODUCT(previous_previous_x, previous_previous_y, previous_x, previous_y, new_x, new_y);
	
//	printf("SCALAR_PRODUCT: (%d,%d), (%d,%d), (%d,%d), %5.2f, %5.2f; %5.2f;\n",previous_previous_x, previous_previous_y, previous_x, previous_y, new_x, new_y, scal_prod, acos(scal_prod)*180.0/M_PI, acos(scal_prod)*180.0/M_PI);

	// If the angle <= 90 degrees, we start a fresh buffer
	// with the previous dot (we start a new stroke),
	// to avoid "overflow" due to overlapping
	if ((previous_previous_x || previous_previous_y) && (scal_prod>=0.3)) {
		draw_buf_reinit(map);

//	printf("********* SCAL PROD *********: %5.2f = %5.2f˚\n",scal_prod,acos(scal_prod)*180.0/M_PI);

		// Non-symmetrical pen tips must be rotated
		
		if (shape != NO_WAVE_SHAPE) {
			rotate ( (gdouble) dx, (gdouble) dy, 
				map->data, map->tmp, 
				map_size, map_size, 
				HF_TYPE_ID, OVERFLOW_ZERO);
				map_to_use = map->tmp;
		}
			else
				map_to_use = map->data;

		generalized_merge( (gpointer) map_to_use,
			HF_TYPE_ID,
			map_size, map_size, 
			d->data, HF_TYPE_ID, 
			d->size, d->size, 
			d->size / 2, d->size / 2,
			ADD,
			FALSE, // no wrap
			map->square_symmetry);
		(d->tail+d->current_tail)->x = previous_x;
		(d->tail+d->current_tail)->y = previous_y;

	}
	else
	{
		subtract_older_dot (map);

	}
	d->current_tail = (d->current_tail+1)%d->max_tail;

	(d->tail+d->current_tail)->x = new_x;
	(d->tail+d->current_tail)->y = new_y;
	(d->tail+d->current_tail)->scal_prod = scal_prod;

	if ((new_x==0) && (new_y==0)) { // We begin a stroke
		return;
	}

	s = d->size;
	memcpy(d->tmp, d->data, s * s * sizeof(hf_type));
	for (x=0; x<s; x++)
		for (y=0; y<s; y++) {
			sx = x-dx;
			sy = y-dy;
			// == 0 if out of boundaries
			if ((sx<0) || (sy<0) || (sx>=s) || (sy>=s)) {
				*(d->data + x + s*y) =0;
			}
			else {
				*(d->data + x + s*y) = *(d->tmp + sx + s*sy);
			}
	}
}

map_struct *map_new () {
	//	A simple allocator
	map_struct *ms;
	ms = (map_struct *) x_malloc(sizeof(map_struct), "map_struct (ms in map_new in draw.c)");
	// We allocate the map to the max radius*2+1, to allow maps without square symmetry
	ms->data = NULL;
	ms->tmp = NULL;
	ms->map_to_use = NULL;
	ms->units = NULL;
	ms->radius = 0;
	ms->square_symmetry = TRUE;
	ms->dr_buf = NULL;
	return ms;
}

void draw_buf_free (draw_buf *dr_buf) {
	if (dr_buf->data)
		x_free(dr_buf->data);
	if (dr_buf->tmp)
		x_free(dr_buf->tmp);
	if (dr_buf->tail)
		x_free(dr_buf->tail);
	x_free(dr_buf);
}

void map_init (map_struct *map, gint size,
	gint level, gint shape, gdouble spacing) {

	gint x,y, radius, ss;
	gdouble dlevel, offset, ddist, p, maxd = (gdouble) MAX_HF_VALUE;
	shape_type *shape_data=NULL;
	radius = size >> 1;
	// The map size should always be odd
	size = 2*radius+1;
	ss = size * size;
	map->data = (hf_type *) x_realloc (map->data, sizeof(hf_type) * ss, "hf_type (map->data in draw.c)");
	map->tmp = (hf_type *) x_realloc (map->tmp, sizeof(hf_type) * ss, "hf_type (map->tmp in draw.c)");
	map->map_to_use = map->data;
	map->units = (hf_type *) x_realloc (map->units, sizeof(hf_type) * ss, "hf_type (map->units in draw.c)");

	FILL_MAP(map->units,ss,1);

	if (spacing == SPACING_HIGH_QUALITY) {
		dlevel = LEVEL_HQ_SPACING;
	}
	else {
		if (spacing == SPACING_STANDARD_QUALITY)
			dlevel = LEVEL_SQ_SPACING;
		else
			if (spacing == SPACING_VHIGH_QUALITY)
				dlevel = LEVEL_VHQ_SPACING;
			else
				dlevel = LEVEL_LQ_SPACING;
	}
	if (shape != NO_WAVE_SHAPE) {
		shape_data = shape_type_new(shape, size);
		map->square_symmetry = FALSE;
	}
	else
		map->square_symmetry = TRUE;

	// Position (0,0) in the map is the minimum value
	// We "clamp" all the edges to 0
	// The maximum edge value is at (0,radius) or (radius,0) or (2*radius, radius) or (radius, 2*radius)
	offset = BELLD(CONST_E,2.0,1.5*DIST2(0,0,0,radius),radius);
	if (shape_data) { // square_symmetry == FALSE
		dlevel = ((gdouble) level)*dlevel/(100.0*(gdouble) MAX_HF_VALUE);
		// We intersect the gaussian bell (values from 0.0 to 1.0) with the pen tip shape
		for (x=0; x<size; x++)
			for (y=0; y<size; y++) {
				// Technique #1: gaussian bell on x axis, shape on y axis
				// A basic technique, giving, with sharps tips, straight lines when the stroke curves
/*				*(map->data+VECTORIZE(x,y,size)) = (hf_type)
					(dlevel * *(shape_data+y) *
 					MAX(0.0, BELLD(e,2.0,1.5*ABS(radius-x),radius) - offset) );
*/
 				// Technique #2: same as technique #1, but the shape
				// is emphasized only around the center
				// Towards the edges, we average it with a gaussian bell
				// in proportion with the square of the distance
				// Increasing the base to 10.0 shortens the shape
				// so that even with sharp tips, straight lines are invisible in stroke curves
				p = MAX(0.0, BELLD(10.0,2.0,1.5*ABS(radius-x),radius) - offset);
				*(map->data+VECTORIZE(x,y,size)) = (hf_type) (dlevel *
					 (p * *(shape_data+y) + (1.0-p) * MAX(0.0, BELLD(CONST_E,2.0,1.5*ABS(radius-y),radius) - offset) * maxd ) *
 					MAX(0.0, BELLD(CONST_E,2.0,1.5*ABS(radius-x),radius) - offset) );

			}
	}
	else {
		// Level is relative (50 means 50%)
		dlevel = ((gdouble) level)*dlevel/100.0;
		for (x=0; x<(radius+1); x++)
			for (y=0; y<(radius+1); y++) {
				ddist = (gdouble) DIST2(0,0,x,y);
				if (ddist >= radius)
					*(map->data+VECTORIZE(radius-x,radius-y,radius+1)) = 0;
				else
					*(map->data+VECTORIZE(radius-x,radius-y,radius+1)) =
						(hf_type) (dlevel * MAX(0.0, BELLD(CONST_E,2.0,1.5*ddist,radius) - offset) );
//						 0x04FF; // TEST
			}
	}
	map->radius = radius;
	if (map->dr_buf)
		draw_buf_free(map->dr_buf);
	map->dr_buf = draw_buf_new (MULT_SIZE*size/DIV_SIZE);
	draw_buf_init (map, spacing);
	if (shape_data)
		free(shape_data);
}

void map_free (map_struct *map) {
	if (!map)
		return;
	if (map->data) {
		x_free(map->data);
		map->data = NULL;
	}
	if (map->tmp) {
		x_free(map->tmp);
		map->tmp = NULL;
	}
	if (map->units) {
		x_free(map->units);
		map->units = NULL;
	}
	map->radius = 0;
	if (map->dr_buf) {
		draw_buf_free(map->dr_buf);
		map->dr_buf = NULL;
	}
}

void draw_dot (hf_struct_type *hf, pen_struct *pen, gint x, gint y, gdouble **gauss_list) {
//	Drawing a unique dot, usually for drawing the first or the last dot of a stroke
//	The remainder of the stroke is drawn with draw_continuous_line_by_dot
	gboolean wrap;
	draw_buf *d;
	gint map_size; // i, j, ii;
	map_struct *map;
	map = pen->map;
	d = map->dr_buf;
	map_size = 2*map->radius+1;
	if (hf->if_tiles==NULL)
		wrap = (pen->wrap==TILING_YES) || (pen->wrap==TILING_AUTO);
	else
		wrap = (pen->wrap==TILING_YES) ||
			((pen->wrap==TILING_AUTO) && *hf->if_tiles);

	if (pen->merge == SMOOTH_OP)
		map_convolve (map->map_to_use, map_size, map_size,
			hf->hf_buf, hf->max_x, hf->max_y, x, y,
			wrap, 100, gauss_list, TRUE);
	else {
		if (pen->overlap) {
			generalized_merge( 
				(gpointer) map->map_to_use, HF_TYPE_ID,
				2*map->radius+1, 2*map->radius+1, 
				hf->hf_buf, HF_TYPE_ID, 
				hf->max_x, hf->max_y, x, y,
				pen->merge,
				wrap,
				map->square_symmetry);
		}
		else {
			// 1. Translate the drawing buffer "under" the
			//    current map (pen tip)
			// 2. Write in the drawing buffer
			// 3. Write the drawing buffer in the layer buffer

			translate_draw_buf (map, x, y, pen->shape);

			generalized_merge( (gpointer) map->map_to_use,
				HF_TYPE_ID,
				map_size, map_size, 
				d->data, HF_TYPE_ID, 
				d->size, d->size, 
				d->size/2, d->size/2,
	//			map->radius, map->radius,
				ADD,
				FALSE, // no wrap
				map->square_symmetry);

			generalized_merge( (gpointer) d->data,
				HF_TYPE_ID,
				d->size, d->size, 
				hf->layer_buf, HF_TYPE_ID, 
				hf->max_x, hf->max_y, 
				x, y,
				MAX_MERGE,
				wrap,
				FALSE ); // square_symmetry
		}
	}
}

void draw_one_dot (hf_struct_type *hf, pen_struct *pen, gint x, gint y, gdouble **gauss_list) {
//	Drawing a unique dot, usually for drawing the first or the last dot of a stroke
//	The remainder of the stroke is drawn with draw_continuous_line_by_dot

	draw_buf *d = pen->map->dr_buf;

	d->current_tail = (d->current_tail+1)%d->max_tail;

	(d->tail+d->current_tail)->x = x;
	(d->tail+d->current_tail)->y = y;

	pen->map->map_to_use = pen->map->data;

	// With a non-symmetrical pen tip, we postpone the first dot,
	// so that we can draw it with the right angle (given by 1st-2nd dots)
	if (pen->shape != NO_WAVE_SHAPE) {
		pen->map->dr_buf->delayed_dot = TRUE;
		return;
	}
	else
		draw_dot (hf, pen, x, y, gauss_list);
}

gboolean draw_continuous_line_by_dot (hf_struct_type *hf, pen_struct *pen,
	gdouble begin_x, gdouble begin_y,
	gdouble *end_x_ptr, gdouble *end_y_ptr, 
	gboolean draw_end, gdouble **gauss_list) {

	// Same concept as draw_hf_line in draw_hf.c
	// Draw in real coordinates (interpolated from (gdouble))
	// Instead of drawing a HF, we draw a gaussian map, whose size is more versatile
	// We smooth progressively what is drawn
	// Return TRUE if something has been drawn, FALSE otherwise

	gint h, steps, map_size; // i, j, ii;
	gdouble spacing, dist, dx, dy, end_x, end_y;
	gboolean wrap;
	map_struct *map;
	draw_buf *d;

	map = pen->map;
	d = map->dr_buf;

	// We force the map size to be odd
	map_size = 2*map->radius+1;

	end_x = *end_x_ptr;
	end_y = *end_y_ptr;

	if (hf->if_tiles==NULL)
		wrap = (pen->wrap==TILING_YES) || (pen->wrap==TILING_AUTO);
	else
		wrap = (pen->wrap==TILING_YES) ||
			((pen->wrap==TILING_AUTO) && *hf->if_tiles);

	// 	Radius is 0 for a size of 1, 1 for a size of 3... up to 127 for a size of 255
	//	Size should always be odd

	// Draw each dot from the last x,y
	// Won't draw the last (just before the mouse release)
	// spacing is the increment applied, in pixels, each time we draw a dot

	spacing = pen->spacing * (gdouble) map_size;

	dist = (gdouble) DIST2((gdouble) begin_x,
			(gdouble) begin_y,
			(gdouble) end_x, (gdouble) end_y);

	dx = spacing * ((gdouble) (end_x - begin_x)) / dist;
	dy = spacing * ((gdouble) (end_y - begin_y)) /dist ;

	steps = (gint) floor(dist/spacing);
	if (!steps) {
		// We return FALSE when there is nothing to draw,
		// so that the (begin_x, begin_y) coordinates are not changed
		// for the next line to draw
		return FALSE;
	}

	if (pen->shape != NO_WAVE_SHAPE) {
		rotate ( (gdouble) dx, (gdouble) dy, 
			map->data, map->tmp, 
			map_size, map_size, 
			HF_TYPE_ID, OVERFLOW_ZERO);
		map->map_to_use = map->tmp;
	}
	else
		map->map_to_use = map->data;

	if (d->delayed_dot) {
		draw_dot (hf, pen, (d->tail+d->current_tail)->x, (d->tail+d->current_tail)->y, gauss_list);
		d->delayed_dot = FALSE;
	}

// printf("DX: %5.2f; DY: %5.2f; shape: %d; symm: %d\n",dx,dy,pen->shape, pen->map->square_symmetry);
	for (h=1; h<=steps; h++) {
		end_x = begin_x + dx * (gdouble) h;
		end_y = begin_y + dy * (gdouble) h;
		if (pen->merge == SMOOTH_OP)
			map_convolve (map->map_to_use,
				map_size, map_size,
				hf->hf_buf, hf->max_x, hf->max_y, 
				(gint) end_x, (gint) end_y,
				wrap, 100, gauss_list, TRUE);
		else {

			if (pen->overlap) {
			
				generalized_merge( 
					(gpointer) map->map_to_use, HF_TYPE_ID,
					map_size, map_size, 
					hf->hf_buf, HF_TYPE_ID, 
					hf->max_x, hf->max_y, 
					(gint) end_x, (gint) end_y,
					pen->merge,
					wrap,
					map->square_symmetry);
			}
			else {
			// 1. Translate the drawing buffer "under" the
			//    current map (pen tip)
			// 2. Write in the drawing buffer
			// 3. Write the drawing buffer in the layer buffer

			translate_draw_buf (map, (gint) end_x, 
				(gint) end_y, pen->shape);

			generalized_merge( (gpointer) map->map_to_use,
				HF_TYPE_ID,
				map_size, map_size, 
				d->data, HF_TYPE_ID, 
				d->size, d->size, 
				d->size/2, d->size/2,
				ADD,
				FALSE, // no wrap
				pen->map->square_symmetry);

			generalized_merge( (gpointer) map->dr_buf->data,
				HF_TYPE_ID,
				d->size, d->size, 
				hf->layer_buf, HF_TYPE_ID, 
				hf->max_x, hf->max_y, 
				(gint) end_x, (gint) end_y,
				MAX_MERGE,
				wrap,
				FALSE ); // square_symmetry
			}
		}
	}

	// The last dot drawn doesn't fall on the coordinates where the motion motify event was emitted,
	// so we must adjust these coordinates
	(*end_x_ptr) = end_x;
	(*end_y_ptr) = end_y;

	return TRUE;
}
