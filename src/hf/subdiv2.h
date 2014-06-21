/* subdiv1.h - headers for subdivision algorithms 1 & 2
 *
 * Copyright (C) 2001 Patrice St-Gelais
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

#ifndef _SUBDIV2
#define _SUBDIV2 1

#include "hf.h"

#ifndef calc_avrg
#define calc_avrg(a,b,c,d) (a+b+c+d)>>2
#endif

#define OP_HF_2 +
#define MAXLAG_2 99
#define MINLAG_2 -99

typedef struct {
//	Options for simple subdivision, square or triangle
	guint seed;
	gint roughness; 	// Variation of random displacement at each step
				//  From -5 (smooth result) to 5 (rough)
	gfloat distribution;	// exponent applied to the rand() result
				// 1:  bumps are equally spread over the surface
				// 2 and more:  high bumps scarcily spread, low bumps more common 
	gboolean top_random;	// TRUE if top value is random... unused, it doesn't work with subdiv2
	hf_type top_value;	// if top_random==FALSE;  from 0 to 0xFFFF... unused, see top_random
	gint frq_percent[12];	// % of max. height for each frequency, from 2 to 4096
} subdiv2_opt;

subdiv2_opt *subdiv2_opt_new(gint* frq_vector, gint nbrfrq);
void subdiv2(hf_struct_type *hf, subdiv2_opt *opt);

#endif




