/* waves.h:  data structures & prototypes for linear or circular waves deformation functions
 *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "globals.h"
#include "hf.h"

#ifndef _WAVES
#define _WAVES 1

// #define NBAXIS 3
#define NBAXIS 2
#define CIRCULAR_WAVE 0
#define VERTICAL_WAVE 1
#define HORIZONTAL_WAVE 2

#define NBWAVES 6
#define SINE_WAVE 0
#define CONCAVE_WAVE 1
#define SINE_CONCAVE_WAVE 2
#define SINE_CONCAVE_2_3_WAVE 3
#define TRIANGLE_WAVE 4
#define SQUARE_WAVE 5

#define NO_WAVE_SHAPE 99 // Used in drawing functions - no shape is intersected with the pen tip

typedef struct {
	gint order;	// Calculation order (Vertical / lateral deformations are not commutative)
	gint shape;	// SINE_WAVE, SQUARE_WAVE...
	gint axis; 	// CIRCULAR_WAVE, VERTICAL_WAVE, HORIZONTAL_WAVE
	gint angle;	// Rotation of the deformation around Y axis (hf  in XZ plane)
	gint period;	// from 0 to 10... actual period = hf_size / pow(2,period)
	gint amplitude;	// 0 to 100% of max. height (width for LATERAL_WAVE)
	gint phase;	// 0 to 100% of period
	gint randomness;	// 0 to 100 %, for amplitude only
	gint seed;
} wave_struct;

// DO NOT FORGET TO CHANGE THE TYPE ID WHEN CALLING INTERPOLATE FUNCTIONS
// IF ONE DECIDES TO CHANGE SHAPE_TYPE TO SOMETHING ELSE THAN gdouble OR double

typedef gdouble shape_type;

// typedef unsigned long int shape_type;

shape_type *shape_type_new(gint shape_id, gint length);

wave_struct * wave_new (gint shape, gint axis, gint order);

void waves_apply(hf_struct_type *hf,
		shape_type **wave_shapes,
		gint nb_wav,
		GList *wave_pages);

void wave_free(wave_struct *ws);

#endif // _WAVES
