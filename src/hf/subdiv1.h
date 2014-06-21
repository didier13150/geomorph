/* subdiv1.h - headers for subdivision algorithms 1 & 2
 *
 * Copyright (C) 2001 Patrice St-Gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _SUBDIV1
#define _SUBDIV1 1

#include "hf.h"

#define calc_avrg(a,b,c,d) (a+b+c+d)>>2

#define OP_HF ^
#define IF_RECALC TRUE
#define IF_RECALC2 TRUE
#define MAXLAG 99
#define MINLAG -99

typedef struct {
//	Options for simple subdivision, square or triangle
	guint seed;
	gint roughness; 	// Variation of random displacement at each step
				//  From -5 (smooth result) to 5 (rough)
	gboolean top_random;	// TRUE if top value is random
	hf_type top_value;	// if top_random==FALSE;  from 0 to 0xFFFF
	unsigned short int frq_control_mask;  	// Fine frequency control
						// 1 bit for each of a max. of 12 subdivisions
} subdiv1_opt;

subdiv1_opt *subdiv1_opt_new(void);
void subdiv1(hf_struct_type *hf, subdiv1_opt *opt);
void calc_subdiv1 (hf_type *hf_buf, int max_x, int max_y, subdiv1_opt *opt);

#endif





