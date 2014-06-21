/* crater.h - headers for processing craters
 *
 * Copyright (C) 2003 Patrice St-Gelais
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

#ifndef _CRATERS
#define _CRATERS 1

#include <gtk/gtk.h>
#include "hf.h"
#include "view_area.h"

// Craters types
#ifndef IRREGULAR_CRATER
	#define IRREGULAR_CRATER 1
#endif
#ifndef PEAK_CRATER
	#define PEAK_CRATER 2
#endif
#ifndef STANDARD_CRATER
	#define STANDARD_CRATER 3
#endif

//	Upper boundary of the slope threshold scale
#ifndef MAX_SLOPE_THRESHOLD
	#define MAX_SLOPE_THRESHOLD 50
#endif

//	Maximum number of attempts to draw one crater,
//	when there is a "slope threshold"
#define MAX_ATTEMPTS 10

typedef struct {
	gint type;	// IRREGULAR_CRATER, PEAK_..., STANDARD_...
	gint diameter;	// In % of current HF size
	gboolean random_diameter;	// TRUE if diameter is to be randomized
					// between diam_bound1 and diam_bound2
	gint diam_bound1;
	gint diam_bound2;
	gint distribution;	// From 1 to 10, exponent of D in K / pow(D,distribution)
			// D = diameter, K = number of craters generated
			// Applies when we generate craters randomly
			// between diam_bound1 and diam_bound2
	gboolean default_depth;  // If TRUE, we use:
		// 7% for IRREGULAR (allegedly very big ~ 75 km), 
		// 10% for PEAK (supposed to be big ~ 30 km),
		// 20% for STANDARD (supposed to be small ~ 7 km)
	gint depth;	// in % of diameter (2*radius), if default_depth not used
	gboolean if_smooth; // Do we smooth the zone before applying the crater?
		// (craters are supposed to replace existing objects - other craters)
	gint radial_noise;  // Random variation of the radius, % from 0 to 10%
	gint surface_noise;  // Noise kept from the original surface, it smoothed
		// A radius, from 0 to 10 pixels, 0 meaning we don't keep noise at all
	gint wrap;	// TILING_AUTO, TILING_YES, TILING_NO
	view_struct *preview;
	gint seed;	// If we want to keep the seed between each use or rand()...
} draw_crater_struct;

//	The maps
extern gfloat irregular_crater[], peak_crater[], standard_crater[];
#ifndef MAP_LENGTH
	#define MAP_LENGTH 256
#endif

//	Prototypes

draw_crater_struct *crater_struct_new();
void crater_struct_free();

void draw_crater (draw_crater_struct *dcs, hf_struct_type *hf, 
	gint x, gint y, gdouble** gauss_list);
gint draw_many_craters (draw_crater_struct *dcs, 
			hf_struct_type *hf,
			gint qty, 
			gint peak_threshold,
			gint slope_threshold,
			gdouble **gauss_list);
gint *get_sqrmap (draw_crater_struct *dcs, gint diam, gint depth);
void init_radial_noise();

#endif // _CRATERS









