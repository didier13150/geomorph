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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "hf.c"
#include "hf_calc.c"
#include "subdiv2.h"

subdiv2_opt *subdiv2_opt_new(gint *frq,gint nbrfrq) {
	// frq: relative frequency level (from 0 to 100%)
	// nbrfrq: length of the frq vector, usually 1 or 12
	// 1: all the frequency level are the same
	// We use the values as a cycle, thanks to the modulo op.,
	// so it's not important if the value domain of nbrfrq is not enforced
	gint i;
	subdiv2_opt *opt;
	opt = (subdiv2_opt *) x_malloc(sizeof(subdiv2_opt), "subdiv2_opt");
	opt->seed = DEFAULT_SEED;
	opt->roughness = 0;
	opt->distribution = 1.0;
	opt->top_random = TRUE;
	opt->top_value = 0xF000;
	for (i=0; i<12; i++)
		opt->frq_percent[i] = *(frq+i%nbrfrq);
	return opt;
}


void re_calc_2( hf_long val, hf_long *a, hf_long *b,
		hf_long *c, hf_long *d, int c_level) 
{
	static gfloat p1=0.2, p2 = 0.8;

	val = (hf_long) (p1 * (gfloat) val);
	*(a) = val + (hf_long) (p2 * (gfloat) *(a));
	*(b) = val + (hf_long) (p2 * (gfloat) *(b));
	*(c) = val + (hf_long) (p2 * (gfloat) *(c));
	*(d) = val + (hf_long) (p2 * (gfloat) *(d));
}

static long int calc_r(gint level, gfloat froughness, gfloat distribution) {
	static long int r;
	static gfloat f;
	static gfloat maxf = (gfloat) 0xFFFF;
	r = 1 + (long int) ((hf_type) rand());
	if (distribution > 1.0) {
		f = (gfloat) r / maxf;
		r = (long int) (powf(f,distribution) * maxf);
	}
// printf("LEVEL: %d;  R1: %d;  ",level,r);
	r = r - (0xFFFF>>(level+2));
// printf("R2: %d;  ",r);
 	r = (long int) (( (gfloat) r) * froughness);
// printf("R3: %d; level: %d; froughness: %f\n",r, level, froughness);
	return r;
}

void smooth_midpoints (hf_struct_type *hf, int x0, int y0, int x1, int y1, int c_level, hf_long *hfl) {
	int mid_x, mid_y, x_1, y_1;

	mid_x = WRAP((x0 + x1)>>1,hf->max_x); 
	mid_y = WRAP((y0 + y1)>>1,hf->max_x);

	x_1 = WRAP2(x0 - (mid_x - x0), hf->max_x);
	y_1 = WRAP2(mid_y - (y1 - y0),hf->max_y);

	x0 = WRAP(x0,hf->max_x);
	y0 = WRAP(y0,hf->max_y);
	x1 = WRAP(x1,hf->max_y);
	y1 = WRAP(y1,hf->max_y);

		re_calc_2(	*(hfl+VECTORIZE(mid_x,mid_y,hf->max_x))  ,
			hfl + VECTORIZE(x0,y0,hf->max_x),
			hfl + VECTORIZE(x0,y1,hf->max_x),
			hfl + VECTORIZE(x1,y0,hf->max_x),
			hfl + VECTORIZE(x1,y1,hf->max_x),
			c_level);
		re_calc_2(*(hfl+VECTORIZE(x0,mid_y,hf->max_x)) ,
			hfl +VECTORIZE(x0,y1,hf->max_x),
			hfl + VECTORIZE(mid_x,mid_y,hf->max_x),
			hfl + VECTORIZE(x0,y0,hf->max_x),
			hfl +VECTORIZE(x_1,mid_y,hf->max_x), c_level );

		re_calc_2(*(hfl+VECTORIZE(mid_x,y0,hf->max_x)) ,
			hfl +VECTORIZE(mid_x,y_1,hf->max_x),
			hfl + VECTORIZE(x0,y0,hf->max_x),
			hfl + VECTORIZE(x1,y0,hf->max_x),
			hfl +VECTORIZE(mid_x,mid_y,hf->max_x), c_level );
}

void calc_midx_midy_2(	hf_struct_type *hf, int x0, int y0, int x1, int y1, int c_level, subdiv2_opt *opt,
			gfloat froughness, hf_long *hfl) {

	int mid_x, mid_y;
	hf_long avrg;
	long int val;

	mid_x = WRAP((x0 + x1)>>1,hf->max_x); 
	mid_y = WRAP((y0 + y1)>>1,hf->max_x);
	x0 = WRAP(x0,hf->max_x);
	y0 = WRAP(y0,hf->max_y);
	x1 = WRAP(x1,hf->max_y);
	y1 = WRAP(y1,hf->max_y);

	avrg = calc_avrg (
		*(hfl + VECTORIZE(x0,y0,hf->max_x)),
		*(hfl +VECTORIZE(x0,y1,hf->max_x)),
		*(hfl +VECTORIZE(x1,y0,hf->max_x)),
		*(hfl +VECTORIZE(x1,y1,hf->max_x)) );

	// mid_x, mid_y
/*
//	Disconnected, it doesn't work as intended, probably because of the smoothing process
	if ((!opt->top_random) && (c_level == 0)) {
// printf("TOP VALUE: %d; FRQ_PERCENT: %d\n", opt->top_value, opt->frq_percent[log2i(hf->max_x)-1]);
		*(hfl+VECTORIZE(mid_x,mid_y,hf->max_x)) =
				(hf_long) ( ( (gfloat) opt->frq_percent[log2i(hf->max_x)-1]/50.0) 
				* (gfloat) opt->top_value) + (gfloat) ((~(hf_long) 0)>>25) ;
// printf("VALUE: %d\n",*(hfl+VECTORIZE(mid_x,mid_y,hf->max_x)) );
	  }
	else {
*/

	val = avrg OP_HF_2 calc_r (c_level, froughness, opt->distribution);
	*(hfl+VECTORIZE(mid_x,mid_y,hf->max_x))  = (hf_long) MAX(0,val);

// if (c_level<2)
// printf("(mid_x,mid_y) == (%d,%d): AVRG: %u, VAL: %d; \n",mid_x,mid_y,avrg, val);
//	}
}

void calc_x_y_2(	hf_struct_type *hf,
		int x0, int y0, int x1, int y1, 
		int c_level,
		subdiv2_opt *opt,
		gfloat froughness,
		hf_long *hfl)
{
	int mid_x, mid_y, x_1, y_1;
	hf_long avrg;
	long int val;

	mid_x = WRAP((x0 + x1)>>1,hf->max_x); 
	mid_y = WRAP((y0 + y1)>>1,hf->max_x);

	// Initialize height values - avrg taken from cross pos.
	x_1 = WRAP2(x0 - (mid_x - x0), hf->max_x);
	y_1 = WRAP2(mid_y - (y1 - y0),hf->max_y);

	x0 = WRAP(x0,hf->max_x);
	y0 = WRAP(y0,hf->max_y);
	x1 = WRAP(x1,hf->max_x);
	y1 = WRAP(y1,hf->max_y);

	// All points averaged from the mid points of the same level
	// All extremities are present, so we divide by 4 (>>2)

	// For x, mid_y
	avrg = calc_avrg (*(hfl +( y1 * hf->max_x) + x0),
		*(hfl +( mid_y * hf->max_x) + mid_x),
		*(hfl + (y0 * hf->max_x) + x0),
		*(hfl +( mid_y * hf->max_x) + x_1 ));
 
	// x0, mid_y

	val = avrg OP_HF_2 calc_r (c_level, froughness,opt->distribution);
// if (c_level<2)
// printf("(x0,mid_y) == (%d,%d): AVRG: %u, VAL: %d; \n",x0,mid_y,avrg, val);

	*(hfl+VECTORIZE(x0,mid_y,hf->max_x)) = (hf_long) MAX(val,0);

	// For mid_x, y
	avrg = calc_avrg
		(*(hfl +( y_1 * hf->max_x) + mid_x), 
		*(hfl +( y0 * hf->max_x) + x0),
		*(hfl + (y0 * hf->max_x) + x1),
		*(hfl +( mid_y * hf->max_x) + mid_x ));	

	// mid_x, y0

	val = avrg OP_HF_2 calc_r (c_level, froughness, opt->distribution);

// if (c_level<2)
// printf("(mid_x,y0) == (%d,%d): AVRG: %u, VAL: %d\n",mid_x,y0,avrg, val);

	*(hfl+VECTORIZE(mid_x,y0,hf->max_x)) = (hf_long) MAX(0,val);

}

void calc_level_2(hf_struct_type *hf, int level, subdiv2_opt *opt, hf_long *hfl) {
	int chunk, i, j;
	gfloat froughness;
	chunk = hf->max_x >> level; // Works only for squares

	froughness = ((gfloat) opt->frq_percent[log2i(hf->max_x)-level-1]/50.0) /
			powf(2.5,(gfloat) MAX(0.0,((gfloat) level) - 2.5 - ((gfloat) opt->roughness)/2.5)) ;

// printf("LEVEL: %d; FROUGHNESS: %f\n",level,froughness);
	for (i = 0; i < hf->max_x; i= i+chunk) 			// diamond
		for (j = 0; j < hf->max_x; j = j + chunk)
			calc_midx_midy_2(hf, i, j, i+chunk, j+chunk,
				level, opt, froughness, hfl);
	for (i = 0 ; i < hf->max_x; i= i+chunk) 			// cross
		for (j = 0; j < hf->max_x; j = j + chunk)
			calc_x_y_2(hf, i, j, i+chunk, j+chunk, 
				level, opt, froughness, hfl);
	for (i = 0; i <= hf->max_x; i= i+chunk) 			// smooth
		for (j = 0; j <= hf->max_x; j = j + chunk)
			smooth_midpoints(hf, i, j, i+chunk, j+chunk, 
				level, hfl);		
}

void subdiv2(hf_struct_type *hf, subdiv2_opt *opt) {
	// Plasma - style subdivision - TEST
	// Non recursive algorithm
	// For the given level:
	// 1. Calculate values for all mid points
	//	Average: square
	// 2. Calculate values for all mid_x, y0
	// 3. Calculate values for x0, mid_y

	int i, levels;
	hf_long *hfl;
	hf_long min,max;
	gfloat ratio;
	srand(opt->seed);

//	There could have been a size change, so we reallocate the memory
	hf->hf_buf = (hf_type *) x_realloc(hf->hf_buf, hf->max_x * hf->max_y * sizeof(hf_type), "hf_type (hf_buf in subdiv2)");

//	long int buffer
	hfl = (hf_long *) x_malloc(hf->max_x * hf->max_y * sizeof(hf_long), "hf_long (hfl in subdiv2)");

//	*(hf->hf_buf) = (hf_type) rand() ;

//	Initialize the first pixel
	*(hfl) = (hf_long) ( ( (gfloat) opt->frq_percent[log2i(hf->max_x)-1]/50.0) *(hf_type) rand()) +
			 (gfloat) ((~(hf_long) 0)>>25) ;

	levels = log2i(hf->max_x);
// printf("First pixel: %u; levels: %d\n",*(hfl), levels );
	for (i=0; i< levels; i++) {
		calc_level_2(hf, i, opt, hfl);
	}
//	Find max and min
	min = *(hfl);
	max = min;
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		if (min>*(hfl+i) )
			min = *(hfl+i);
		else
			if (max<*(hfl+i))
				max = *(hfl+i);
	}
	if ((max-min)<=0xFFFF)
		ratio = 1.0;
	else
		ratio = ((gfloat) (max-min)) / (gfloat) 0xFFFF;
// printf("MIN: %u; MAX: %u; DIFF: %u;  RATIO: %f\n",min,max,max-min,ratio);
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		*(hf->hf_buf+i) = (hf_type) (( (gfloat) (*(hfl+i)-min))/ratio);
	}
	x_free(hfl);
}




