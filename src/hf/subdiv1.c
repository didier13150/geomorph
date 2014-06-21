/* subdiv1.c - subdivision algorithm #1:  cross / diamond
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "subdiv1.h"

subdiv1_opt *subdiv1_opt_new() {
	subdiv1_opt *opt;
// printf("SUBDIV1_OPT_NEW\n");
	opt = (subdiv1_opt *) x_malloc(sizeof(subdiv1_opt), "subdiv2_opt");
	opt->seed = DEFAULT_SEED;
	opt->roughness = 0;
	opt->top_random = TRUE;
	opt->top_value = 0xF000;
	opt->frq_control_mask = 0;
	return opt;
}

void re_calc( hf_type val, hf_type *a, hf_type *b,
		hf_type *c, hf_type *d, int c_level) 
{
	int lag;
//	static float p1 = 0.9, p2 = 0.1;
	if (!IF_RECALC) return;

	// Rough at small scales with lag constant at 3
//	lag = MIN(1,c_level-8); // rough at high scales, high frq (low scale) quite smooth
				// must activate re_calc for all points,
				// otherwise the "calculation flow" is visible
//	lag = MIN(0,4-c_level); // high frq smooth, low frq rough
	lag = MAX(1,MIN(7,8-c_level)); // Best, better with level_factor = 1
//	lag = MAX(1,(8-c_level) >> 1); // Not bad with level_factor = 1
//... last one gives visible crests when painting
	val = val >> lag; 
	*(a) = val + ((*a)-((*a)>>lag));
	*(b) = val + ((*b)-((*b)>>lag));
	*(c) = val + ((*c)-((*c)>>lag));
	*(d) = val + ((*d)-((*d)>>lag));
}

void calc_midx_midy(hf_type *hf_buf, int max_x, int max_y, int x0, int y0, int x1, int y1, int c_level, subdiv1_opt *opt) {

	int mid_x, mid_y, r;
	hf_type avrg, val;
	mid_x = (x0 + x1)>>1; 
	mid_y = (y0 + y1)>>1;

	avrg = calc_avrg (
		*(hf_buf + (y0*max_x) + x0),
		*(hf_buf +( (y1%max_y) * max_x) +x0),
		*(hf_buf + (y0*max_x) + (x1%max_x) ),
		*(hf_buf +( (y1%max_y) * max_x) + (x1%max_x) ));

	// mid_x, mid_y

	if ((!opt->top_random) && (c_level == 0)) {
		*(hf_buf+(mid_y*max_x)+mid_x) = opt->top_value;
	  }
	else {

//		r = (hf_type) ( ((hf_type) rand()) /
//			 pow(2.282,(double) c_level - opt->roughness - 1 ) ) ;

		r = (hf_type) ( ((hf_type) rand()) >>
		 MAX(MINLAG,(MIN(MAXLAG,c_level - opt->roughness - 1))) );

		val = avrg OP_HF r;
		*(hf_buf+(mid_y*max_x)+mid_x) = val;

		// Some smoothing

		   re_calc(val,
			hf_buf + (y0*max_x) + x0,
			hf_buf +( (y1%max_y) * max_x) +x0,
			hf_buf + (y0*max_x) + (x1%max_x) ,
			hf_buf +( (y1%max_y) * max_x) + (x1%max_x),c_level);
	}
}

void calc_x_y(	hf_type *hf_buf, int max_x, int max_y,
		int x0, int y0, int x1, int y1, 
		int c_level,
		subdiv1_opt *opt)
{
	int mid_x, mid_y, x_1, y_1;
	hf_type avrg, r, val;

	mid_x = (x0 + x1)>>1; 
	mid_y = (y0 + y1)>>1;

	// Initialize height values - avrg taken from cross pos.
	x_1 = x0 - (mid_x - x0);
	if (x_1 < 0) 
		x_1 = max_x + x_1;
	y_1 = mid_y - (y1 - y0);
	if (y_1 < 0) 
		y_1 = max_y + y_1;
	x1 = x1%max_x;
	y1 = y1%max_y;
	// All points averaged from the mid points of the same level
	// All extremities are present, so we divide by 4 (>>2)

	// For x, mid_y
	avrg = calc_avrg (*(hf_buf +( y1 * max_x) + x0),
		*(hf_buf +( mid_y * max_x) + mid_x),
		*(hf_buf + (y0 * max_x) + x0),
		*(hf_buf +( mid_y * max_x) + x_1 ));

	r = (hf_type) ( ((hf_type) rand()) >> 
		MAX(MINLAG,(MIN(MAXLAG,c_level - opt->roughness - 1))) );
	// x0, mid_y

	val =	avrg OP_HF r; 

	*(hf_buf+(mid_y*max_x) + x0) = val;

	// Some smoothing
	if (IF_RECALC2)
		re_calc(val,hf_buf +( y1 * max_x) + x0,
			hf_buf +( mid_y * max_x) + mid_x,
			hf_buf + (y0 * max_x) + x0,
			hf_buf +( mid_y * max_x) + x_1, c_level );


	// For mid_x, y
	avrg = calc_avrg
		(*(hf_buf +( y_1 * max_x) + mid_x), 
		*(hf_buf +( y0 * max_x) + x0),
		*(hf_buf + (y0 * max_x) + x1),
		*(hf_buf +( mid_y * max_x) + mid_x ));	

	// mid_x, y0
	r = (hf_type) ( ((hf_type) rand()) >> 
		MAX(MINLAG,(MIN(MAXLAG,c_level - opt->roughness - 1))) );

	val = avrg OP_HF r;

	*(hf_buf+(y0*max_x) + mid_x) = val;

	// Some moothing
	if (IF_RECALC2)
		re_calc(val,
			hf_buf +( y_1 * max_x) + mid_x, 
			hf_buf +( y0 * max_x) + x0,
			hf_buf + (y0 * max_x) + x1,
			hf_buf + ( mid_y * max_x) + mid_x, c_level );

}

void calc_level(hf_type *hf_buf, int max_x, int max_y, int level, subdiv1_opt *opt) {
	int chunk, i, j;
	chunk = max_x >> level; // Works only for squares
	
	for (i = 0; i < max_x; i= i+chunk) // diamond
		for (j = 0; j < max_x; j = j + chunk)
			calc_midx_midy(hf_buf, max_x, max_y, 0 + i, 0 + j, i+chunk, j+chunk, level, opt);
	for (i = 0; i < max_x; i= i+chunk) // cross
		for (j = 0; j < max_x; j = j + chunk)
			calc_x_y(hf_buf, max_x, max_y, 0+i, 0+j, i+chunk, j+chunk, 
				level, opt);	
}
	
void calc_subdiv1 (hf_type *hf_buf, int max_x, int max_y, subdiv1_opt *opt) {

	int i, levels;
	srand(opt->seed);
	
	*(hf_buf) = (hf_type) rand() ;

	levels = log2i(max_x);
	for (i=0; i< levels; i++) {
		calc_level(hf_buf, max_x, max_y, i, opt);
	}
	
}

void subdiv1(hf_struct_type *hf, subdiv1_opt *opt) {
	// Plasma - style subdivision
	// Non recursive algorithm
	// For the given level:
	// 1. Calculate values for all mid points
	//	Average: square
	// 2. Calculate values for all mid_x, y0
	// 3. Calculate values for x0, mid_y_
// printf("SUBDIV1\n");
//	There could have been a size change, so we reallocate the memory

	hf->hf_buf = (hf_type *) x_realloc(hf->hf_buf, hf->max_x * hf->max_y * sizeof(hf_type), "hf_type (hf_buf in subdiv1)");

	calc_subdiv1 (hf->hf_buf, hf->max_x, hf->max_y, opt);
}


