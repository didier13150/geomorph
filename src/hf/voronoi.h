/**************************************************************************

    voronoi.h - Headers for building voronoi diagrams
    		(emulating cracks netwokrs and similar structures)

    Copyright (C) Patrice St-Gelais, January 2006
         patrstg@users.sourceforge.net
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#ifndef VORONOI_H
#define VORONOI_H

#include "hf.h"
#include "subdiv1.h"

#ifndef VORONOI_HEIGHT
#define VORONOI_HEIGHT 0x7FFF
#endif

// Sites distribution for Voronoi diagram

#ifndef UNIFORM
#define UNIFORM 0
#endif

#ifndef CENTERED
#define CENTERED 1
#endif

#ifndef REGULAR
#define REGULAR 2
#endif

// How to use the current HF

#ifndef USE_AS_NOISE
#define USE_AS_NOISE 0
#endif

#ifndef USE_AS_GUIDE
#define USE_AS_GUIDE 1
#endif

// Crack width type

#ifndef FIXED_WIDTH
#define FIXED_WIDTH 0
#endif

#ifndef FROM_DISTANCE
#define FROM_DISTANCE 1
#endif

// Scale (multiple or not)

#ifndef SCALE_1X
#define SCALE_1X 0
#endif

#ifndef SCALE_1X_2X
#define SCALE_1X_2X 1
#endif

#ifndef SCALE_1X_2X_4X
#define SCALE_1X_2X_4X 2
#endif

typedef struct {
	gint x;
	gint y;
} x_y;

typedef struct {
	gint x;
	gint y;
	gint prec;
	gint next;
	gdouble wx; // Cumulative weigth
	gdouble wy;
	gdouble weigth;
} x_y_l;

typedef struct {
	gdouble cell_size; // Average cell size, in % of hf, from 0,1 to 100,0 % - default 10%
	gint distribution_type; // UNIFORM, CENTERED, REGULAR - default UNIFORM
	gint random_variation; // For REGULAR placement of sites - relative, from 0 to 100%
	gint hf_use; // Use of current hf (USE_AS_NOISE or USE_AS_GUIDE)
	gboolean gener_noise; // Gener noise if HF is used as guide
	gint noise_level; // Noise relative strength (from 0 to 100% - default 50)
	gint crack_width_type; // FIXED_WIDTH or FROM_DISTANCE
	gint min_width; // From 0 to 10 (pixels for FIXED_WIDTH or relative for FROM_DISTANCE)
	gint max_width;
	gint edges_level; // Amplitude, from 0 to 100%
	gint seed;
	gint scale; // SCALE_1X, SCALE_1X_2X, SCALE_1X_2X_4X
	subdiv1_opt *noise_opt;
} voronoi_struct;


/************************************************************************************/

voronoi_struct *voronoi_struct_new (gdouble cell_size, gint distribution_type, gint random_variation, gint hf_use, gboolean gener_noise, gint noise_level, gint crack_width_type, gint min_width, gint max_width, gint edges_level);
voronoi_struct *voronoi_struct_new_with_defaults (); 
void voronoi_struct_free (voronoi_struct *vs);
	
void voronoi (hf_type *output, hf_type *output_dist, hf_type *noise, hf_type *guide, gint max_x, gint max_y, gdouble size, gint distribution, gint random_var, gint crack_width_type, gint min_width, gint max_width, gboolean gener_noise, gint noise_level, gboolean dist_required);

void voronoi_adjust_edges (hf_type *in, hf_type *out, hf_type *dist, gint edges_level, gint max_x, gint max_y);

void hf_voronoi (hf_struct_type *hf, voronoi_struct *vs);

x_y * create_site_list (gint max_x, gint max_y, gdouble size, gint distribution, gint random_var, gint *length);

#endif
