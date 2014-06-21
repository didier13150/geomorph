/* draw_hf.h - headers for HF pen (dotted stroke)
 *
 * Copyright (C) 2003, 2008 Patrice St-Gelais
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

#ifndef _DRAW_HF
#define _DRAW_HF 1

#include <gtk/gtk.h>
#include "hf.h"
#include "hf_calc.h"
#include "subdiv2.h"
#include "hf_filters.h"

#ifndef DRAW_CACHE_LENGTH
#define DRAW_CACHE_LENGTH 8
#endif

// Modified 2008-09 to standardize on Subdiv2

typedef struct {
	hf_struct_type* hf[DRAW_CACHE_LENGTH];
	hf_type *scaled_hf[DRAW_CACHE_LENGTH];
	subdiv2_opt * opt;
	gint size;
	gint map_size; // Deduced from size, nearest power of 2 (ceiling)
	gint level;
	gint roughness; // from -5 to +5 ("transmitted" to opt*)
	gint merge;  	// ADD, SUBTRACT, SMOOTH_OP
	gint wrap;	// TILING_AUTO, TILING_YES, TILING_NO
	gboolean control_spacing; 	// Initializes a check button,
					// controlling the activation of the spacing scale
	// TRUE: the pen flow is space-controlled
	// FALSE: the pen flow is time-controlled
	gint spacing; 			// A % of pen size, from 0 to 100
	dist_matrix_struct *dist_matrix; // Inherited (global)
	filter_struct *filter;
	gdouble **gauss_list; // Inherited (global)
} draw_hf_struct;

//	Prototypes

draw_hf_struct *draw_hf_new(dist_matrix_struct *);
void draw_hf_free(draw_hf_struct *);

void stroke_dot (hf_struct_type *hf, draw_hf_struct *pen, gint x, gint y);
gboolean draw_hf_line (hf_struct_type *hf, draw_hf_struct *pen,
	gint begin_x, gint begin_y, 
	gint *end_x_ptr, gint *end_y_ptr,
	gint scale);

void set_gauss_list (draw_hf_struct *d, gdouble **gauss_list);
void update_hf_map (draw_hf_struct *);

#endif // DRAW_HF


