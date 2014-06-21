/* draw_hf.c:  callbacks for drawing with a hf in a hf
 *
 * Copyright (C) 2008 Patrice St-Gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "hf.h"
#include "hf_calc.h"
#include "draw_hf.h"
#include "subdiv2.h"
#include "hf_filters.h"
#include <time.h>

#ifndef calc_map_size
#define calc_map_size(size) ((gint)pow(2.0,1.0+(gdouble)log2i(size)))
#endif

draw_hf_struct *draw_hf_new (dist_matrix_struct *dist) {
	gint frq=50, i;
	draw_hf_struct *d;
	d = (draw_hf_struct *) x_malloc (sizeof(draw_hf_struct), "draw_hf_struct");
	for (i=0; i<DRAW_CACHE_LENGTH; i++) {
		d->hf[i] = NULL;
		d->scaled_hf[i] = NULL;
	}
	d->opt = subdiv2_opt_new (&frq,1);
	d->size = 83;
	d->map_size = calc_map_size(d->size);
	d->level = 35;
	d->roughness = 0;
	d->merge = ADD;
	d->wrap = TILING_AUTO;
	d->control_spacing = TRUE;
	d->spacing = 25;
	d->dist_matrix = dist;
	d->filter = NULL;
	d->gauss_list = NULL;
	update_hf_map (d);
	return d;
}

static void free_hf_cache (draw_hf_struct *pen) {
	gint i;
	for (i=0; i<DRAW_CACHE_LENGTH; i++) {
		if (pen->hf[i]) {
			hf_free(pen->hf[i]);
			pen->hf[i] = NULL;
		}
	}
}

static void free_scaled_cache (draw_hf_struct *pen) {
	gint i;
	for (i=0; i<DRAW_CACHE_LENGTH; i++) {
		if (pen->scaled_hf[i]) {
			x_free(pen->scaled_hf[i]);
			pen->scaled_hf[i] = NULL;
		}
	}

}

static void free_cache (draw_hf_struct *pen) {
	free_hf_cache (pen);
	free_scaled_cache (pen);
}

void draw_hf_free (draw_hf_struct *pen) {
	if (pen) {
		free_cache(pen);
		if (pen->opt) {
			x_free(pen->opt);
			pen->opt = NULL;
		}
		x_free(pen);
	}
}

void set_gauss_list (draw_hf_struct *d, gdouble **gauss_list) {
	if (d && gauss_list)
		d->gauss_list = gauss_list;
}

static void scale_pen (draw_hf_struct *pen, gint cache_index) {
	// We scale down the HF (power of 2 sizes)
	// to the size of the drawing map
	gdouble ratio;
	gint i,j;
	if (!pen->hf[cache_index])
		return;
	ratio = ((gdouble) pen->map_size) / (gdouble) pen->size;
	if (!pen->scaled_hf[cache_index])
		pen->scaled_hf[cache_index] = (hf_type *) x_malloc(pen->size * pen->size * sizeof(hf_type), "hf_type (scaled_hf in scale_pen)");
	for (i=0; i<pen->size; i++)
		for (j=0; j<pen->size; j++) {
			interpolate (
				ratio * (gdouble) i,
				ratio * (gdouble) j, 
				pen->hf[cache_index]->hf_buf,
				pen->map_size, pen->map_size,
				pen->scaled_hf[cache_index] + VECTORIZE(i,j,pen->size), 
				HF_TYPE_ID,
				OVERFLOW_ZERO);
	}
}

void update_hf_map (draw_hf_struct *pen) {
	// Update the values for computing the HFs used when drawing
	// after a size change or a roughness change
	// Create or update the first HF in the cache
	// (this one is displayed)
	gint new_map_size;
	new_map_size = calc_map_size(pen->size);
	if ((!pen->hf[0]) || (new_map_size != pen->map_size) || 
		(pen->opt->roughness != pen->roughness)) {
		pen->map_size = new_map_size;
		pen->opt->roughness = pen->roughness;
		// Create or update the filter
		if (!pen->filter)
			pen->filter = wide_filter_new (pen->map_size, pen->dist_matrix);
		else
			gauss_filter_reinit (pen->filter, pen->map_size, pen->dist_matrix);
		free_hf_cache (pen);
		// Calculate the first HF
		pen->opt->seed = DEFAULT_SEED;
		pen->hf[0] = hf_new(pen->map_size);
		subdiv2(pen->hf[0], pen->opt);	
		filter_apply(pen->hf[0],
			pen->filter,
			pen->dist_matrix,
			FALSE,
			100,
			ADD);
	}
	free_scaled_cache (pen);
	scale_pen (pen, 0);
}


void stroke_dot (hf_struct_type *hf, draw_hf_struct *pen, gint x, gint y) {
//	Draw one dot of a stroke
// printf("STROKE_DOT\n");
	gboolean wrap;
	guint cache_index, t;

	if (hf->if_tiles==NULL)
		wrap = (pen->wrap==TILING_YES) || (pen->wrap==TILING_AUTO);
	else
		wrap = (pen->wrap==TILING_YES) || ((pen->wrap==TILING_AUTO) && *hf->if_tiles);

	t = rand();
	cache_index = t%DRAW_CACHE_LENGTH;

	if (!pen->hf[cache_index]) {
		pen->opt->seed = rand();
		pen->hf[cache_index] = hf_new(pen->map_size);
		subdiv2(pen->hf[cache_index], pen->opt);
		
		filter_apply(pen->hf[cache_index],
			pen->filter,
			pen->dist_matrix,
			FALSE,
			100,
			ADD);
		scale_pen (pen, cache_index);
	}

	if (!pen->scaled_hf[cache_index])
		scale_pen (pen, cache_index);

	if (pen->merge==SMOOTH_OP) {
		map_convolve (pen->scaled_hf[cache_index], 
			pen->size, pen->size, 
			hf->hf_buf, hf->max_x, hf->max_y, 
			x, y, wrap, 
			pen->level, pen->gauss_list, FALSE);
	}

	else {

		hf_merge_buf(pen->scaled_hf[cache_index], pen->size, pen->size,
			hf->hf_buf, hf->max_x, hf->max_y, 
			x, y,
			pen->merge,
			wrap,
			(gfloat) pen->level * (gfloat) pen->level / 10000.0,
			FALSE);

		}
}

gboolean draw_hf_line (hf_struct_type *hf, draw_hf_struct *pen,
	gint begin_x, gint begin_y, 
	gint *end_x_ptr, gint *end_y_ptr, gint scale) {

	// Return TRUE if something has been drawn, FALSE otherwise
	gint i,end_x,end_y, steps;
	gdouble spacing, dist, dx, dy;

	end_x = *end_x_ptr;
	end_y = *end_y_ptr;
//	Draw a line built by repeating a HF
	if (pen->control_spacing) {
		// Draw each dot from the last x,y
		// Won't draw the last (just before the mouse release)
		spacing = ((gdouble) pen->spacing*(pen->size))/100.0;
		spacing = (spacing<1.0?1.0:spacing); // Should be more than one pixel
// printf("(last_x,last_y): (%d,%d); (x,y): (%d,%d)\n",begin_x,begin_y,end_x,end_y);
		dist = pow(2.0,scale) * (gdouble) DIST2((gdouble) begin_x,
				(gdouble) begin_y,
				(gdouble) end_x, (gdouble) end_y);

		steps = (gint) floor(dist/spacing);
		if (!steps)
			return FALSE;
// printf("SPACING: %5.2f; DIST: %5.2f; STEPS: %d; ddx: %5.2f; ddy: %5.2f;  \n",spacing, dist, steps, end_x - begin_x,end_y -begin_y);
		dx = spacing * ((gdouble) (end_x - begin_x)) / dist;
		dy = spacing * ((gdouble) (end_y - begin_y)) /dist ;
// printf("ddx: %5.2f; ddy: %5.2f;  \n",dx,dy);
		for (i=1; i<=steps; i++) {
			end_x =begin_x + (gint) (i*dx);
			end_y = begin_y + (gint) (i*dy);
			stroke_dot (hf, pen, 
				LEFT_SHIFT(end_x, scale),
				LEFT_SHIFT(end_y, scale));
		}
	} // control_spacing test
	else {		// Behavior of Geomorph 0.11 and 0.12
		stroke_dot (hf, pen,
			LEFT_SHIFT(end_x, scale),
			LEFT_SHIFT(end_y, scale));
	}
	// The last dot drawn doesn't fell on the coordinates where the motion motify event was emitted,
	// so we must adjust these coordinates
	(*end_x_ptr) = end_x;
	(*end_y_ptr) = end_y;
	return TRUE;
}

