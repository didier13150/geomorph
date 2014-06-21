/* surfadd.h - headers for surfaces addition algorithm
 * Copyright (C) 2002 Patrice St-Gelais
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _SURFADD
#define _SURFADD 1

#include "hf.h"
#include "hf_filters.h"

typedef struct {
//	Options for surface addition
	unsigned int seed;
	gint xy_random_range;
	gint h_random_range;
	gint slope;
	gint density;
	hf_struct_type *surf_to_add;
	filter_struct **current_filter;	// Inherited from hf_options->primit_surf
	gint *filter_merge;	// Merge mode, inherited from hf_options->primit_surf
	gboolean *revert_filter; 	// Inherited from hf_options->primit_surf
	gfloat *filter_level;	// Inherited from hf_options->primit_surf
	dist_matrix_struct *dist_matrix;
	gint frq_percent[12];	// % of max. height for each frequency, from 2 to 4096
} surfadd_opt;

surfadd_opt *surfadd_opt_new(void);
void surfadd(hf_struct_type *hf, surfadd_opt *opt);

#endif

