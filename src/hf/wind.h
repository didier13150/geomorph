/* wind.h - headers for wind erosion functions
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

#ifndef _WIND
#define _WIND 1

#include "hf.h"
#include "hf_calc.h"

#ifndef RIPPLES
#define RIPPLES 0
#endif

#ifndef DUNES
#define DUNES 1
#endif

typedef struct {	
	gint steps;
	gint wind_speed;
	gint slope_threshold;
	gdouble a;	// Slope derivative factor
	gdouble b;	// Slope factor (amplitude, kind of)
	gboolean refine_crests; // For dunes only, when TRUE, apply a slight amount of "ripples"
	gboolean smooth_before;
	gint radius;	// Smoothing radius
} wind_struct;

wind_struct *wind_struct_new(gint steps, gint wind_speed, gint slope_threshold,
	gdouble a, gdouble b, gboolean smooth_before, gint radius);
void wind_struct_free(wind_struct *);

void hf_ripples (hf_struct_type *hf, wind_struct *ws, dist_matrix_struct *dm, gdouble **gauss_list);
void hf_dunes (hf_struct_type *hf, wind_struct *ws, dist_matrix_struct *dm, gdouble **gauss_list);

// void hf_dunes (hf_struct_type *hf, wind_struct *ws);

#endif // _WIND
