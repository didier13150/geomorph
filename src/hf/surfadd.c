/* surfadd.c - Progressive surfaces addition
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "uniform.h"
#include "surfadd.h"
#include "uniform.h"

surfadd_opt *surfadd_opt_new() {
	gint i;
	surfadd_opt *opt;
	opt = (surfadd_opt *) x_malloc(sizeof(surfadd_opt), "surfadd_opt");
	opt->seed = rand();
	opt->xy_random_range = 50; // % of random displacement allowed; 100% = surface width
	opt->h_random_range = 50; // % of random displacement allowed; 50% = surface width
	opt->density = 50;	// 0 = 50% of normal density;  100 = 200% of normal density
	opt->slope = 50;
	opt->current_filter = NULL;
	opt->filter_merge = NULL;
	opt->filter_level = NULL;
	opt->revert_filter = NULL;
	opt->surf_to_add = NULL;
	opt->dist_matrix = NULL;
	for (i=0; i<12; i++)
		opt->frq_percent[i] = 50;
	return opt;
}

void subfadd_opt_free( surfadd_opt *opt) {
	if (opt->surf_to_add)
		hf_free(opt->surf_to_add);
	x_free(opt);
}

void calc_surf_level(hf_struct_type *hf, int level, surfadd_opt *opt) {
	gint chunk, i, j, surf_width, density_chunk, sign1,sign2;
//	displacement1 :  along X axis
//	displacement2 :  along Y axis
//	displacement3 :  height / along Z axis
	gint d12, d3, displacement1, displacement2;
	gfloat displacement3, base;
	hf_type value;

	chunk = hf->max_x >> level;	// Works only for squares!
//	surf_width = (gint) (1.5 * (gfloat) chunk); // ... replace 1.5 with a (future) parameter...
	surf_width = chunk;
//	We shrink a bit the density range, for avoiding lenghty and useless
//	computations when the value is near 100
	density_chunk = (gint) (chunk * (1.0 + 0.85*(50-opt->density) / 50.0));
// printf("DENSITY: %d;  CHUNK: %d; NEWCHUNK: %d\n",opt->density,chunk, density_chunk);

//	We create a primitive surface, a quarter of the max heigth
//	Sums of repeatidly halved values converge to the original value... 0.5 + 0.25 + ... = 1;
//	We start with a quarter for allowing random height displacement from 0 to 25%

	if (density_chunk <= 1)	// Useless and expensive
		return;
	if (!opt->frq_percent[log2i(hf->max_x)-level-1])	// Skip if the current frq is at 0%
		return;
	if (opt->surf_to_add)
		hf_free(opt->surf_to_add);

	opt->surf_to_add = hf_new(surf_width);

// printf("DIVISEUR DE VALUE: %f\n",MAX(1,pow(1+((gfloat) 100-opt->slope)/50, level+2)));

// printf("FRQ: %d\n",opt->frq_percent[log2i(hf->max_x)-level-1]);
	value = 0xFFFF>>2;
//	value = (0xFFFF>>2) * (gint) (opt->frq_percent[log2i(hf->max_x)-level-1]/50);
//	if (!value)
//		return;
	uniform(opt->surf_to_add, &value);

	filter_apply (	opt->surf_to_add, 
			*(opt->current_filter),
			opt->dist_matrix,
			*(opt->revert_filter),
			*(opt->filter_level),
			*(opt->filter_merge));

//	Random displacements
	// 1 and 2 are additive displacements (xy <= 100 % of the current height, h <= 200%)
	// 3 is multiplicative (from 0 % to 100% of the current level)
	// 2005.03.14 More naturally centered at 50% when we divide by 100
	d12 = hf->max_x * opt->xy_random_range / 200;

	base = ((gfloat) (0xFFFF / MAX(1,pow(1+((gfloat) 100-opt->slope)/50, level+2) ) ) ) / (gfloat) value;
	base = base * ((gfloat) opt->frq_percent[log2i(hf->max_x)-level-1]/50);
	d3 =  (0xFFFF >> (level+2)) * opt->h_random_range / 50;

// printf("Value: %d; base: %f; d12: %d; d3: %d\n",value, base, d12,d3);
	if (!level) {	// First step, in the center! (more "natural")
		displacement1 = d12 ? (rand() % d12) : 0;
		displacement2 = d12 ? (rand() % d12) : 0;
		sign1 = ((rand() %2)<<1)-1;
		sign2 = ((rand() %2)<<1)-1;
		displacement3 = d3 ? (base + (gfloat) (rand() % d3) / value) : base;
/*		displacement3 = (gfloat) (rand() % d3) ;
		displacement3 = displacement3*displacement3 / d3;
		displacement3 = d3 ? (base + displacement3 / value) : base;
*/		hf_merge( opt->surf_to_add, hf, 
//				hf->max_x>>1, hf->max_y>>1,
				(hf->max_x>>1)+sign1*displacement1,
				(hf->max_y>>1)+sign2*displacement2,
				ADD, 
				(hf->if_tiles==NULL) ? TRUE : *hf->if_tiles, 
//				1);
				displacement3,
				FALSE);
	}
	else

	     for (i = density_chunk>>1; i < hf->max_x; i= i+density_chunk)
		for (j = density_chunk>>1; j < hf->max_x; j = j + density_chunk ) {
			displacement1 = d12 ? (rand() % d12) : 0;
			displacement2 = d12 ? (rand() % d12) : 0;
			sign1 = ((rand() %2)<<1)-1;
			sign2 = ((rand() %2)<<1)-1;
		//	Height displacements are always positive,
		//	because the floor is 25% of the total H and displacements <= 25%,
		//	so that each step does not add a surface higher than 50%
		//	of the max H of the preceding step (0.5 + 0.25 + 0.125 + .... = 1)
			displacement3 = d3 ? (base + (gfloat) (rand() % d3) / value ) : base;
//	Just trying to vary the noise function - too uniform - alas it doesn't work as intended?!
/*		displacement3 = (gfloat) (rand() % d3) ;
		displacement3 = displacement3*displacement3*displacement3 / (gfloat) (d3*d3);
		displacement3 = d3 ? (base + displacement3 / value) : base;
*/
// if (level<4) printf("Displacement1: %d;  Displacement2: %d;  Displacement3: %f\n",displacement1, displacement2, displacement3);
			hf_merge( opt->surf_to_add, hf, 
//				i,j,
				i+sign1*displacement1,
				j+sign2*displacement2,
				ADD,  
				(hf->if_tiles==NULL) ? TRUE : *hf->if_tiles, 
//				1);
				displacement3,
				FALSE);
		}
}

void surfadd(hf_struct_type *hf, surfadd_opt *opt) {
	// Building a HF with progressive additions of similar decreasing surfaces
	// For the given level:
	// 1. Calculate values for all mid points
	// 2. Draw / merge a uniform & filtered sub-HF on each point

	gint i, levels;
	hf_type value=0;

	srand(opt->seed);
//	There could have been a size change, so we reallocate the memory
	hf->hf_buf = (hf_type *) x_realloc(hf->hf_buf, hf->max_x * hf->max_y * sizeof(hf_type), "hf_type (hf_buf in surfadd)");

//	We initialise the buffer with black
	uniform(hf, &value);

	levels = log2i(hf->max_x);

	for (i=0; i< levels; i++) {
		calc_surf_level(hf, i, opt);
	}
}
