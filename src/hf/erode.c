/* erode.c - erosion and cresting functions
 *
 * Copyright (C) 2001-2005 Patrice St-Gelais
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

#include <string.h>
#include "hf.h"
#include "hf_calc.h"
#include "erode.h"

#ifndef MAX_CACHE
#define MAX_CACHE 32768
#endif

static gdouble **cache;
static gboolean if_cache_init=FALSE;

rain_erosion_struct *rain_erosion_struct_new (gint drops, gint threshold, gint strength, gint hardness, gboolean progressive_refresh, gint interval, gboolean old_algorithm) {
	rain_erosion_struct *res;
	res = (rain_erosion_struct *) x_malloc(sizeof(rain_erosion_struct), "rain_erosion_struct");
	res->drops = drops;
	res->threshold = threshold;
	res->strength = MIN(100,MAX(strength,0));
	res->hardness = MIN(100,MAX(hardness,25));
	res->refresh = progressive_refresh;
	res->interval = interval;
	res->old_algorithm = old_algorithm;
	res->gener_frames = FALSE;
	
	return res;
}

void rain_erosion_struct_free (rain_erosion_struct *res) {
	if (res)
		x_free(res);
}

gravity_struct *gravity_struct_new (gint steps, gint slope_threshold) {
	gravity_struct *ges;
	ges = (gravity_struct *) x_malloc(sizeof(gravity_struct), "gravity_struct");
	ges->steps = steps;
	ges->threshold = slope_threshold;
	return ges;
}

void gravity_struct_free (gravity_struct *ges){
	if (ges)
		x_free(ges);
}

oriented_gravity_struct *oriented_gravity_struct_new (gint steps, gint threshold, gint direction) {
	oriented_gravity_struct *ogs;
	ogs = (oriented_gravity_struct *) x_malloc(sizeof(oriented_gravity_struct), "oriented_gravity_struct");
	ogs->steps = steps;
	ogs->threshold = threshold;
	ogs->direction = direction;
	return ogs;
}

void oriented_gravity_struct_free (oriented_gravity_struct *ogs) {
	if (ogs)
		x_free(ogs);
}

whimsical_erosion_struct *whimsical_erosion_struct_new (gint radius, gint smooth_ribs, gboolean auto_contrast) {
	whimsical_erosion_struct *wes;
	wes = (whimsical_erosion_struct *) x_malloc(sizeof(whimsical_erosion_struct), "whimsical_erosion_struct");
	wes->radius = radius;
	wes->smooth_ribs = smooth_ribs;
	wes->auto_contrast = auto_contrast;
	return wes;
}

void whimsical_erosion_struct_free (whimsical_erosion_struct *wes){
	if (wes)
		x_free(wes);
}

craters_erosion_struct *craters_erosion_struct_new (gint qty, gint display_interval, gint peak_threshold, gint slope_threshold) {
	craters_erosion_struct *ces;
	ces = (craters_erosion_struct *) x_malloc(sizeof(craters_erosion_struct), "craters_erosion_struct");
	ces->qty = qty;
	ces->display_interval = display_interval;
	ces->peak_threshold = peak_threshold;
	ces->slope_threshold = slope_threshold;
	ces->crater_str = crater_struct_new();
	return ces;
}

void craters_erosion_struct_free (craters_erosion_struct *ces) { 
	if (!ces)
		return;
	if (ces->crater_str)
		crater_struct_free(ces->crater_str);
	x_free(ces);
}

void hf_eroded_ribs (hf_type *hf_in, hf_type *hf_out, gint max) {
//	Build a "ribbed" (eroded) template by taking the minimum
//	absolute value of the difference between each pixel and its neighborus
//	Neighbours are numbered from 1 to 8, clockwise, 1 being (1,0)
//	The default for this version of the program is to wrap
	static gint z1, z2, z3, z4, z5, z6, z7, z8;
	hf_type value;
	static gint x,y, i;
	for (y=0; y<max; y++)
		for (x=0; x<max; x++) {
			i = VECTORIZE(x,y,max);
			value = *(hf_in + i);
			z1 =  *(hf_in+VECTORIZE(WRAP2(x+1,max),y,max))  ;
			z2 = *(hf_in+VECTORIZE(WRAP2(x+1,max),
				WRAP2(y+1,max),max)) ;
			z3 = *(hf_in+VECTORIZE(x,WRAP2(y+1,max),max));
			z4 = *(hf_in+VECTORIZE(WRAP2(x-1,max),WRAP2(y+1,max),max));
			z5 =  *(hf_in+VECTORIZE(WRAP2(x-1,max),y,max)) ;
			z6 = *(hf_in+VECTORIZE(WRAP2(x-1,max),
				WRAP2(y-1,max),max)) ;
			z7 = *(hf_in+VECTORIZE(x,WRAP2(y-1,max),max)) ;
			z8 = *(hf_in+VECTORIZE(WRAP2(x+1,max),
				WRAP2(y-1,max),max)) ;

//			Absolute minimum (negative of)
			z1 = ABS(z1-value);
			z2 = ABS(z2-value);
			z3 = ABS(z3-value);
			z4 = ABS(z4-value);
			z5 = ABS(z5-value);
			z6 = ABS(z6-value);
			z7 = ABS(z7-value);
			z8 = ABS(z8-value);

			*(hf_out + i)  = ~ (hf_type) MIN(z1,MIN(z2,MIN(z3,MIN(z4,MIN(z5,MIN(z6,MIN(z7,z8))))))) ;

// if (!(i%512)) {
// printf("*(hf_out+i): %d; z1: %d, z2: %d, z3: %d, z4: %d, z5: %d, z6: %d, z7: %d, z8: %d\n",*(hf_out+i), z1, z2, z3, z4, z5, z6, z7, z8);
// }
		}
}

void hf_oriented_relax (hf_struct_type *hf, gint steps, gint max_slope,  gint direction) {
	// Oriented gravity relaxation (for wind functions)
	
	gint v1, v2, i, slope, x, y, iprec, step;
	
	// Direction is one of NORTH (ymax to 0), SOUTH (0 to ymax), WEST (xmax to 0), EAST (0 to xmax)
	// Note: spreading the values with a 3x3 kernel gives no significant visible difference
	slope = iget_absolute_slope(max_slope,hf->max_x);
	for (step=0; step<steps; step++) {
		for (y=0; y<hf->max_y; y++) {
			for (x=0; x<hf->max_x; x++) {
				switch (direction) {
					case NORTH: // For N and S, swap x & y
						i = VECTORIZE(x,hf->max_x-y-1,hf->max_x);
						iprec = VECTORIZE (x, WRAP2(hf->max_x-y,hf->max_x), hf->max_x);
						break;
					case SOUTH:	
						i = VECTORIZE(x,y,hf->max_x);
						iprec = VECTORIZE ( x, WRAP2(y-1,hf->max_x), hf->max_x);
						break;
					case WEST:		
						i = VECTORIZE(hf->max_x-x-1,y,hf->max_x);
						iprec = VECTORIZE ( WRAP2(hf->max_x-x,hf->max_x), y, hf->max_x);
						break;
					default: // East		
						i = VECTORIZE(x,y,hf->max_x);
						iprec = VECTORIZE ( WRAP2(x-1,hf->max_x), y, hf->max_x);
				}
				
				v1 = *(hf->hf_buf+iprec);
				v2 =  ((gint) *(hf->hf_buf+i)) + slope;
				if ( v1 > v2 ) {

					*(hf->hf_buf+iprec) -= (v1-v2) /2;
					*(hf->hf_buf+i) += (v1-v2) / 2  ;
				 
				}
			}
		}
	}
}

gboolean relax_hex (hf_type *hf, gint max_x, gint max_y, gint slope_threshold, gint wrap) {

//	Gravity relaxation, on an hexagonally sampled buffer
//	Variation over cresting, more natural-like (I hope!)
//	Written 2005-02 for sand / wind processes
//	hf: buffer to transform
//	x,y: pixel to relax
//	max_x, max_y: size of the buffer (so it could work with non-square buffers!)
//	slope_threshold: maximum allowed slope, in absolute HF units (not degrees!)
//	max_steps: don't do more than this number of steps
//		The function can stop before if all slopes are lowver than max_slope
//	even, shift_right: hexagonal sampling parameters (see comments in other functions)
//	Returns: TRUE is relaxation done, FALSE if no relaxation to do
//
//	2006-11-08 Modified for randomizing the hexagonal approximation
//

	static gint v0[6] = {0,0,0,0,0,0};
	static gint x,y,v[6],z1, z2, z3, z4, z5, z6, i, ii, ninf, shift, sumdif, axis, even, xx, yy;
	long int value;
	gboolean test, found=FALSE;
	
	// The beginning is the same as find_all_neighbours
	// Then we take all the neighbour pixels inferior to the threshold
	// We average the difference between these pixels and the threshold
	// Then the current pixel is decreased and the "soil" is evenly
	// spread on the selected pixels

	even = TRUE;
	axis = H_AXIS;
	for (x=0; x<max_x; x++) {
		for (y=0; y<max_y; y++) {
			ninf = 0;
			sumdif = 0;
			// 2006-11-08: better results with even and axis fixed
			even = rand()%2;
			if (rand()%2)
				axis = V_AXIS;
			else
				axis = H_AXIS;
			
			memcpy(v,v0,6*sizeof(gint));
			value = *(hf+VECTORIZE(x,y,max_x)) - slope_threshold ;
	//		test = y & (gint) 1; // test = TRUE if value is odd
	if (axis==H_AXIS)
		test = y & (gint) 1; // test = TRUE if value is odd
	else
		test = x & (gint) 1; // test = TRUE if value is odd
		
			if ( (test && even) || ((!test) && (!even)) )  //  XOR
				shift = 1;
			else
				shift = 0;
				
			if (axis==H_AXIS) {
				xx = x+1;
				yy = y;
			}
			else {
				xx = x;
				yy = y+1;
			}
			if (wrap)
				ii=VECTORIZE(WRAP2(xx,max_x),WRAP2(yy,max_y), max_x);
			else
				ii=VECTORIZE(MIN(xx,max_x-1),MIN(yy,max_y-1), max_x);

			z1 =  *(hf+ii);
			if (z1<value) {
				if (wrap || ( (x+1)<max_x ) ) {
					v[ninf] = ii;
					ninf++;
					sumdif += value-z1;
				}
			}
						
			if (axis==H_AXIS) {
				xx = x+shift;
				yy = y+1;
			}
			else {
				xx = x+1;
				yy = y+shift;
			}
			
			if (wrap)
				ii=VECTORIZE(WRAP2(xx,max_x), WRAP2(yy,max_y),max_x) ;
			else
				ii=VECTORIZE(MIN(xx,max_x-1), MIN(yy,max_y-1),max_x) ;
				
			z2 = *(hf+ii);
			if (z2<value) {
				if (wrap || (((x+shift)<max_x) && ((y+1)<max_y)) ) {
					v[ninf] = ii;
					ninf++;
					sumdif += value-z2;
				}
			}
						
			if (axis==H_AXIS) {
				xx = x-1+shift;
				yy = y+1;
			}
			else {
				xx = x+1;
				yy = y-1+shift;
			}
			
			if (wrap)
				ii =VECTORIZE( WRAP2(xx,max_x), WRAP2(yy,max_y), max_x) ;
			else
				ii = VECTORIZE(MAX(0,xx),MIN(MAX(0,yy),max_y-1),max_x) ;	

			z3 = *(hf+ii) ;
			if (z3<value) {
				if (wrap || (((x+shift)>0) && ((y+1)<max_y)) ) {
					v[ninf] = ii;
					ninf++;
					sumdif += value-z3;
				}
			}
									
			if (axis==H_AXIS) {
				xx = x-1;
				yy = y;
			}
			else {
				xx = x;
				yy = y-1;
			}
			
			if (wrap)
				ii=VECTORIZE(WRAP2(xx,max_x), WRAP2(yy,max_x) , max_x)  ;
			else
				ii=VECTORIZE(MAX(xx,0), MAX(yy,0), max_x)  ;
	
			z4 = *(hf+ii);
			if (z4<value) {
				if (wrap || (x>0)) {
					v[ninf] = ii;
					ninf++;
					sumdif += value-z4;
				}
			}
						
									
			if (axis==H_AXIS) {
				xx = x-1+shift;
				yy = y-1;
			}
			else {
				xx = x-1;
				yy = y-1+shift;
			}
			if (wrap)
				ii=VECTORIZE(WRAP2(xx,max_x),
				 WRAP2(yy,max_x),max_x) ;
			else
				ii=VECTORIZE(MAX(xx,0), MAX(yy,0),max_x) ;
				
			z5 =  *(hf+ii) ;
			if (z5<value) {
				if (wrap || ((y>0) && ((x+shift)>0)) ) {
					v[ninf] = ii;
					ninf++;
					sumdif += value-z5;
				}
			}
					
			if (axis==H_AXIS) {
				xx = x+shift;
				yy = y-1;
			}
			else {
				xx = x-1;
				yy = y+shift;
			}
			if (wrap)
				ii=VECTORIZE(WRAP2(xx,max_x),WRAP2(yy,max_y),max_x) ;
			else
				ii=VECTORIZE(MIN(MAX(0,xx),max_x-1),MAX(yy,0),max_x) ;	

			z6 = *(hf+ii);
			if (z6<value) {
				if (wrap || ((y>0) && ((x+shift)<max_x)) ) { 
					v[ninf] = ii;
					ninf++;
					sumdif += value-z6;
				}
			}
		
			if (ninf)
				found = TRUE;
			else
				continue;	

// if (y==255)
// printf("(X,Y): (%d,%d); ninf: %d; sl_max: %d; value: %d; sumdif: %d; sd/ninf+1: %d; sd/ninf*ninf+1: %d\n",x,y,ninf,slope_threshold, value, sumdif, sumdif/(ninf+1), sumdif / (ninf*(ninf+1))  );			
			
			*(hf+VECTORIZE(x,y,max_x)) -= sumdif / (ninf+1);
			
			sumdif = sumdif / (ninf*(ninf+1));
			
			for (i=0; i<ninf; i++) {
				*(hf+v[i]) += sumdif;
			}

	
		} // Y loop
		
	} // X loop
	
	return found;
}

void hf_relax_hex (hf_struct_type *hf, gint max_steps, gint max_slope) {

//	Wrapper for relax_hex - gravity erosion
	
	gint steps=0, slope;
	
	slope = iget_absolute_slope(max_slope,hf->max_x);
	
	for (steps=0; steps<max_steps; steps++) {
//	printf("******************** STEP %d **********************\n",steps);
		if (!relax_hex (hf->hf_buf, hf->max_x, hf->max_y, slope, (hf->if_tiles==NULL) ? TRUE : *hf->if_tiles))
			break;
	}
	
//	printf("Steps: %d / %d\n",steps, max_steps);

}

gboolean find_all_neighbours_hex( gint x, gint y, gint *newx, gint *newy,
	gint max, hf_type *hf, gint threshold) {
	//	Find all the inferior neighbours of (x,y) in hf
	//	Randomly select one of the neighbours,
	//	given its height difference with the minimum is < to threshold
	//	Returns the result in newx and newy
	//	If (x,y) is a minimum, returns FALSE
	//	This version emulates an hexagonal symmetry (six neighbours)
	//	We need to know if even or odd rows have been shifted (even=TRUE)
	//	and if they have been shifted right or left
	//	Neighbours are counted clockwise from 1 to 6, 1 being (1,0)
	//	2, 3, 5 and 6 are being shifted on x, depending on "even"
	//
	//             5 | 6
	//           4 | 0 | 1
	//             3 | 2
	//
	// 2006.11.07: when axis is V_AXIS, assume that Y is shifted instead of X
	// Axis and even/ood shift are randomized, for some kind of antialiasing
	// This is a better replacement of the preceding simili-hexagonal sampling
	//
	static gint v[6],z1, z2, z3, z4, z5, z6, i, ii, nx, ny, ninf, ok, shift;
	long int value;
	long int lower= (long int) 0xFFFF;
	gint even, axis;
	gboolean test;
	ninf = 0;
	ok = 0;
	value = *(hf+VECTORIZE(x,y,max)) ;
	if (rand()%2)
		axis = V_AXIS;
	else
		axis = H_AXIS;
	even = rand()%2; // TRUE or FALSE
	if (axis==H_AXIS)
		test = y & (gint) 1; // test = TRUE if value is odd
	else
		test = x & (gint) 1; // test = TRUE if value is odd
	if ( (test && even) || ((!test) && (!even)) )  //  XOR
		shift = 1;
	else
		shift = 0;
	
	if (axis==H_AXIS)
		ii=VECTORIZE(WRAP2(x+1,max),y,max);
	else
		ii=VECTORIZE(x,WRAP2(y+1,max),max);
	z1 =  *(hf+ii);
	if (z1<value) {
		v[ninf] = ii;
		ninf++;
	}
	lower = z1;
	
	if (axis==H_AXIS) {
		nx = WRAP2(x+shift,max);
		ny = WRAP2(y+1,max);
	}
	else {
		nx = WRAP2(x+1,max);
		ny = WRAP2(y+shift,max);
	}
	ii=VECTORIZE(nx, ny,max) ;
	z2 = *(hf+ii);
	if (z2<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z2<z1)
		lower = z2;
	
	if (axis==H_AXIS) {	
		nx = WRAP2(x-1+shift,max);	
		ny = WRAP2(y+1,max) ;
	}
	else {
		nx = WRAP2(x+1,max);	
		ny = WRAP2(y-1+shift,max) ;
	}
	ii =VECTORIZE(nx,ny,max) ;
	z3 = *(hf+ii) ;
	if (z3<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z3<lower)
		lower = z3;
		
	
	if (axis==H_AXIS) {	
		nx = WRAP2(x-1,max);
		ii = VECTORIZE(nx,y,max)  ;
	}
	else {
		ny = WRAP2(y-1,max);
		ii = VECTORIZE(x,ny,max)  ;
	}
	z4 = *(hf+ii);
	if (z4<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z4<lower)
		lower = z4;
		
	if (axis==H_AXIS) {	
		nx = WRAP2(x-1+shift,max) ;	
		ny = WRAP2(y-1,max) ;
	}
	else {
		nx = WRAP2(x-1,max) ;	
		ny = WRAP2(y-1+shift,max) ;
	}
	ii =VECTORIZE(nx,ny,max) ;
	z5 =  *(hf+ii) ;
	if (z5<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z5<lower)
		lower = z5;
			
	if (axis==H_AXIS) {	
		nx = WRAP2(x+shift,max);
		ny = WRAP2(y-1,max);
	}
	else {	
		nx = WRAP2(x-1,max);
		ny = WRAP2(y+shift,max);
	}
	ii = VECTORIZE(nx, ny,max) ;
	z6 = *(hf+ii);
	if (z6<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z6<lower)
		lower = z6;

// printf("(X,Y): (%d,%d); ninf: %d; lower: %u; threshold: %d; value: %d; ",x,y,ninf, lower, threshold, value);
	if ( (lower+threshold) < value) {
		// Find all the pixels not farther away than "threshold" than the lower one
		for (i=0; i<ninf; i++) { 
			if ((*(hf+v[i])-threshold)<=lower) {
				v[ok] = v[i];
				ok ++;
			}
		}
// printf("ok: %d; ",ok);
		ii = v[rand()%ok];
// printf("ii: %d; ",ii);
		*newx = ii%max;
		*newy = (gint) (ii / max);
// printf("(NEWX,NEWY) = (%d,%d) = (%d,%d)\n", ii%max,(gint) ii/max,*newx,*newy);
		return TRUE;
	}
	else {
// printf("Returning FALSE\n");
		return FALSE;
	}
}

void gflow_add_part(hf_type remainder, hf_type from_value, gint tox, gint toy, hf_type *hf, gint max, gboolean wrap) {
	//	Flow some part of the H difference between (x,y) and (tox,toy)
	//	from (x,y) to (tox,toy)
	//	Float (double) version
//	Define a 3x3 matrix - proportion of the flowing chunk to displace
	static gdouble matrix[3][3]={{1.0,2.0,1.0},{2.0,4.0,2.0},{1.0,2.0,1.0}};
	static gdouble msum = 16.0;
//	static gdouble matrix[3][3]={0.0,1.0,0.0,1.0,2.0,1.0,0.0,1.0,0.0};
//	static gdouble msum = 6.0;
//	static gdouble matrix[3][3]={1.0,1.0,1.0,1.0,4.0,1.0,1.0,1.0,1.0};
//	static gdouble msum = 12.0;
	
	gint i, m, n, xi, yi, qtytot=0;
	gdouble qty, ratio;
	hf_type to_value;
	
	if (!remainder)
		return;
		
	ratio = remainder / msum;

//	if (x==64)
//		printf("FROM_VALUE: %d; TO_VALUE: %d; qtytot: %d\n",from_value, *(hf+VECTORIZE(tox,toy,max)),qtytot);
	for (m=0; m<3; m++)
		for (n=0; n<3; n++) {
			if (wrap)
				i = VECTORIZE(WRAP2(tox-1+n,max),WRAP2(toy-1+m,max),max);
			else {
				xi = tox-1+n;
				yi = toy-1+m;
				if ( (xi<0) || (yi<0) || (xi>=max) || (yi>=max) )
					continue;
				else
					i = VECTORIZE(xi,yi,max);
			}
			qty = ((gdouble) remainder) * matrix[m][n] * ratio;
			// Flow up to the point where the target value is equal to
			// the source value
			// Keep the remainder for the target center
			to_value = *(hf+i);
			if (from_value>to_value) {
				*(hf+i) = (hf_type) MIN(from_value,MAX(0,(gint) to_value + (gint) qty));
				qtytot -= (*(hf+i) - to_value);
			}
			else
				qtytot -= (gint) qty;
		}
	
	if (qtytot>0) {
//		printf("SOLDE QTYTOT (%d, %d): %d; NCHUNK: %d\n",x,y,qtytot, nchunk);
		*(hf+VECTORIZE(tox,toy,max)) -= qtytot;
	}

}

void flow(gint x, gint y, gint tox, gint toy, hf_type *hf, gint max, gint hardness, gint strength, gboolean wrap) {
	//	Flow some part of the H difference between (x,y) and (tox,toy)
	//	from (x,y) to (tox,toy)
//	Define a 3x3 matrix - proportion of the flowing chunk to displace
//	static gint matrix[3][3]={1,1,1,1,4,1,1,1,1};
//	static gint matrix[3][3]={0,1,0,1,2,1,0,1,0};
	static gint matrix[3][3]={{1,2,1},{1,4,1},{1,2,1}};
	gint i, m, n, qty, xi, yi, qtytot=0;
	hf_type nchunk, to_value, from_value, value;
	gdouble ratio;

	// nchunk == soil (delta) to move
	nchunk =  *(hf+VECTORIZE(x,y,max)) - *(hf+VECTORIZE(tox,toy,max));
//	nchunk = nchunk / 12;
	nchunk = ((hf_type) ((((gfloat) nchunk) * ((gfloat) strength)) / 100.0)) >> 4;
//	if (x==64)
//		printf("NCHUNK (%d,%d): %d\n",x,y,nchunk);
	ratio = ((gdouble) hardness) / 100.0;
	// Remove soil at (x,y)
	for (m=0; m<3; m++)
		for (n=0; n<3; n++) {
			if (wrap)
				i = VECTORIZE(WRAP2(x-1+n,max),WRAP2(y-1+m,max),max);
			else {
				xi = x-1+n;
				yi = y-1+m;
				if ( (xi<0) || (yi<0) || (xi>=max) || (yi>=max) )
					continue;
				else
					i = VECTORIZE(xi,yi,max);
			}
			qty = nchunk * matrix[m][n];
			qtytot += qty;
			*(hf+i) = (hf_type) MIN(0xFFFF,MAX(0,(gint) *(hf+i) - qty));
		}
	// Add soil at (tox,toy)
	from_value = *(hf+VECTORIZE(x,y,max));
//	if (x==64)
//		printf("FROM_VALUE: %d; TO_VALUE: %d; qtytot: %d\n",from_value, *(hf+VECTORIZE(tox,toy,max)),qtytot);
	for (m=0; m<3; m++)
		for (n=0; n<3; n++) {
			if (wrap)
				i = VECTORIZE(WRAP2(tox-1+n,max),WRAP2(toy-1+m,max),max);
			else {
				xi = tox-1+n;
				yi = toy-1+m;
				if ( (xi<0) || (yi<0) || (xi>=max) || (yi>=max) )
					continue;
				else
					i = VECTORIZE(xi,yi,max);
			}
			qty = nchunk * matrix[m][n];
			// Flow up to the point where the target value is equal to
			// the source value
			// Keep the remainder for the target center
			to_value = *(hf+i);
			if (from_value>to_value) {
				*(hf+i) = (hf_type) MIN(from_value,MAX(0,(gint) to_value + qty));
				qtytot -= (*(hf+i) - to_value);
			}
			else
				qtytot -= qty;
		}
	
	if (qtytot>0) {
//		printf("SOLDE QTYTOT (%d, %d): %d; NCHUNK: %d\n",x,y,qtytot, nchunk);
		value = (hf_type) (((gdouble) qtytot) * ratio);
		*(hf+VECTORIZE(tox,toy,max)) -= value;
		gflow_add_part((hf_type) qtytot - value, from_value, tox, toy, hf, max, wrap);
	}

}

void hf_rain_erosion_hex (hf_struct_type *hf, rain_erosion_struct *res, gpointer (*display_function) (gpointer), gpointer display_data) {
//	Simple iterative erosion
//	using hexagonal sampling for a more natural looking result
//	We generate random drops and follow their path
//	We don't flow a drop when the slope is more than 2 power threshold
//	Neighbours are numbered from 1 to 6, clockwise, 1 being (1,0)
//	Not extensively tested when the HF doesn't wrap
	
	static gint x,y, s, threshold, nextx, nexty, steps; // ii;
	gboolean tiling;
	
//	threshold is the future chunk... it must be at least 16, because the center
//	of the 3x3 averaging matrix == 4 and we divide the chunk by 16 before applying it
//	threshold = 15 + (gint) pow(2.0,(double) thresh);
	threshold = MAX(16,iget_absolute_slope(res->threshold,hf->max_x));
// printf("RAIN: Threshold (slope): %d = %d\n",slope_threshold,threshold);
//	In the 1st version, drops were given as an exponent
//	We could fallback to this in the future...
//	drops = (long int) pow(2.0,(gdouble) dr_log);
	if (hf->if_tiles)
		tiling = *hf->if_tiles;
	else
		tiling = TRUE;

	if (!hf->tmp_buf)
		hf_backup(hf);
// printf("RAIN EROSION, DROPS: %d; THRESHOLD: %d\n", drops, threshold);
	for (s=0; s<res->drops; s++) {
			
		x = rand()%hf->max_x;
		y = rand()%hf->max_y;
		// Make the (x,y) drop flow until it encounters a minimum
		steps = 0;
// printf("********** DROP: %u; (x,y): (%d,%d)\n",s,x,y);
		while (find_all_neighbours_hex(x,y,&nextx,&nexty,
				hf->max_x,hf->hf_buf,threshold))	{
			if (!tiling)
				if ( (x==0) || (y==0) ||
					(x==(hf->max_x-1)) ||
					(y==(hf->max_y-1)) )
						break;
			flow(x,y,nextx,nexty,hf->hf_buf,hf->max_x, res->hardness, res->strength, tiling);
			//ii = VECTORIZE(nextx,nexty,hf->max_x);
			y = nexty;
			x = nextx;
			steps++;
			if (steps>hf->max_x)
				break;
		}
		if (res->refresh && res->interval)
			if ((!(s%res->interval)) && display_function && display_data)
				(*display_function) (display_data);
	}

}

gboolean find_all_neighbours_hex_old( gint x, gint y, gint *newx, gint *newy,
	gint max, hf_type *hf, gint threshold, 
	gboolean even, gboolean shift_right) {
	//	Find all the inferior neighbours of (x,y) in hf
	//	Randomly select one of the neighbours,
	//	given its height difference with the minimum is < to threshold
	//	Returns the result in newx and newy
	//	If (x,y) is a minimum, returns FALSE
	//	This version works on an hexagonally samplec HF (six neighbours)
	//	We need to know if even or odd rows have been shifted (even=TRUE)
	//	and if they have been shifted right or left
	//	Neighbours are counted clockwise from 1 to 6, 1 being (1,0)
	//	2, 3, 5 and 6 are being shifted on x, depending on "even" and "shift_right"
	//
	//             5 | 6
	//           4 | 0 | 1
	//             3 | 2
	//
	static gint v[6],z1, z2, z3, z4, z5, z6, i, ii, nx, ny, ninf, ok, shift;
	long int value;
	long int lower= (long int) 0xFFFF;
	gboolean test;
	ninf = 0;
	ok = 0;
	value = *(hf+VECTORIZE(x,y,max)) ;
	
	test = y & (gint) 1; // test = TRUE if value is odd
	if ( (test && even) || ((!test) && (!even)) )  //  XOR
		shift = 1;
	else
		shift = 0;
	
	ii=VECTORIZE(WRAP2(x+1,max),y,max);
	z1 =  *(hf+ii);
	if (z1<value) {
		v[ninf] = ii;
		ninf++;
	}
	lower = z1;
	
	nx = WRAP2(x+shift,max);
	ny = WRAP2(y+1,max);
	ii=VECTORIZE(nx, ny,max) ;
	z2 = *(hf+ii);
	if (z2<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z2<z1)
		lower = z2;
		
	nx = WRAP2(x-1+shift,max);	
	ny = WRAP2(y+1,max) ;
	ii =VECTORIZE(nx,ny,max) ;
	z3 = *(hf+ii) ;
	if (z3<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z3<lower)
		lower = z3;
		
	nx = WRAP2(x-1,max);
	ii = VECTORIZE(nx,y,max)  ;
	z4 = *(hf+ii);
	if (z4<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z4<lower)
		lower = z4;
	
	nx = WRAP2(x-1+shift,max) ;	
	ny = WRAP2(y-1,max) ;
	ii =VECTORIZE(nx,ny,max) ;
	z5 =  *(hf+ii) ;
	if (z5<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z5<lower)
		lower = z5;
		
	nx = WRAP2(x+shift,max);
	ny = WRAP2(y-1,max);
	ii = VECTORIZE(nx, ny,max) ;
	z6 = *(hf+ii);
	if (z6<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z6<lower)
		lower = z6;

	if ( (lower+threshold) < value) {
		// Find all the pixels not farther away than "threshold" than the lower one
		for (i=0; i<ninf; i++) { 
			if ((*(hf+v[i])-threshold)<=lower) {
				v[ok] = v[i];
				ok ++;
			}
		}
		ii = v[rand()%ok];
		*newx = ii%max;
		*newy = (gint) (ii / max);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void flow_old(gint x, gint y, gint tox, gint toy, hf_type *hf, gint max, gboolean wrap) {
	//	Flow a quarter of the H difference between (x,y) and (tox,toy)
	//	from (x,y) to (tox,toy)
//	Define a 3x3 matrix - proportion of the flowing chunk to displace
	static gint matrix[3][3]={ {1,2,1}, {2,4,2}, {1,2,1} };
	gint i, m, n, qty, xi, yi;
	hf_type nchunk;

	nchunk = *(hf+VECTORIZE(x,y,max)) - *(hf+VECTORIZE(tox,toy,max));
	nchunk = nchunk >> 2;
//	nchunk = nchunk >> 4;
	// Remove soil at (x,y)
	for (m=0; m<3; m++)
		for (n=0; n<3; n++) {
			if (wrap)
				i = VECTORIZE(WRAP2(x-1+n,max),WRAP2(y-1+m,max),max);
			else {
				xi = x-1+n;
				yi = y-1+m;
				if ( (xi<0) || (yi<0) || (xi>=max) || (yi>=max) )
					continue;
				else
					i = VECTORIZE(xi,yi,max);
			}
			qty = nchunk * matrix[m][n];
			*(hf+i) = (hf_type) MIN(0xFFFF,MAX(0,(gint) *(hf+i) - qty));
		}
	// Add soil at (tox,toy)
	for (m=0; m<3; m++)
		for (n=0; n<3; n++) {
			if (wrap)
				i = VECTORIZE(WRAP2(tox-1+n,max),WRAP2(toy-1+m,max),max);
			else {
				xi = tox-1+n;
				yi = toy-1+m;
				if ( (xi<0) || (yi<0) || (xi>=max) || (yi>=max) )
					continue;
				else
					i = VECTORIZE(xi,yi,max);
			}
			qty = nchunk * matrix[m][n];
			*(hf+i) = (hf_type) MIN(0xFFFF,MAX(0,(gint) *(hf+i) + qty));
		}

}

void hf_rain_erosion_hex_old (hf_struct_type *hf, rain_erosion_struct *res, gpointer (*display_function) (gpointer), gpointer display_data) {
//	Simple iterative erosion
//	using hexagonal sampling for a more natural looking result
//	We generate random drops and follow their path
//	We don't flow a drop when the slope is more than 2 power thresh
//	Neighbours are numbered from 1 to 6, clockwise, 1 being (1,0)
//	Not extensively tested when the HF doesn't wrap
	
	static gint x,y, s, nextx, nexty; // threshold, ii;
	unsigned long int steps;
	hf_type *tmp;
	gboolean even=TRUE, tiling;
	
//	threshold is the future chunk... it must be at least 16, because the center
//	of the 3x3 averaging matrix == 4 and we divide the chunk by 16 before applying it
//	threshold = 15 + (gint) pow(2.0,(double) thresh);
	//threshold = MAX(16,iget_absolute_slope(res->threshold,hf->max_x));
// printf("RAIN: Threshold (slope): %d = %d\n",slope_threshold,threshold);
//	In the 1st version, drops were given as an exponent
//	We could fallback to this in the future...
//	drops = (long int) pow(2.0,(gdouble) dr_log);

	if (hf->if_tiles)
		tiling = *hf->if_tiles;
	else
		tiling = TRUE;

	if (!hf->tmp_buf)
		hf_backup(hf);
		
	tmp = hf->hf_buf;
	hf->hf_buf = hexagonal_row_sampling (hf->hf_buf, hf->max_x, tiling, even, TRUE); 
	x_free(tmp);

	for (s=0; s<res->drops; s++) {
		x = rand()%hf->max_x;
		y = rand()%hf->max_y;
		// Make the (x,y) drop flow until it encounters a minimum
		steps = 0;
		while (find_all_neighbours_hex_old(x,y,&nextx,&nexty,
				hf->max_x,hf->hf_buf,res->threshold, TRUE, TRUE))	{
			if (!tiling)
				if ( (x==0) || (y==0) ||
					(x==(hf->max_x-1)) ||
					(y==(hf->max_y-1)) )
						break;
			flow_old(x,y,nextx,nexty,hf->hf_buf,hf->max_x, tiling);
			//ii = VECTORIZE(nextx,nexty,hf->max_x);
			y = nexty;
			x = nextx;
			steps++;
			if (steps>hf->max_x)
				break;
		}
		if (res->refresh && res->interval)
			if ((!(s%res->interval)) && display_function && display_data)
				(*display_function) (display_data);
	}		
	// Restore the rectangular sampled hf
	tmp = hf->hf_buf;
	hf->hf_buf = hexagonal_row_sampling (hf->hf_buf, hf->max_x, tiling, even, FALSE); 
	x_free(tmp);
}

void hf_crests_erosion_hex (hf_struct_type *hf, gint steps, gint slope_threshold) {
//	Simple iterative erosion, giving crested mountains
//	Hexagonally sampled version (see hf_rain_erosion_hex) 2005-02
//	2006-02 Replaced by randomized axis / even / odd flow
//	We find the minimum neighbour
//	If its value is superior to the current pixel one,
//	we add half the difference to the current pixel
//	If the minimum neighbour is inferior, we subtract half the current pixel
//	This is like if half the difference was "flowing" from or to the current pixel
//	Neighbours are numbered from 1 to 6, clockwise, 1 being (1,0)
//	2004-03-11 Corrected to deal with wrapping

	static gint z1, z2, z3, z4, z5, z6, x, y, xx, yy;
	static gint  i, s, val, ii, minii, lower, threshold, shift, axis;
	hf_type value;
	gboolean test, even = FALSE, tiling;
	gfloat ratio; // Factor for dividing the weight of the minimum pixel, when it is on the diagonals
	
	// Slope_threshold is in degrees
	threshold = iget_absolute_slope(slope_threshold,hf->max_x);
// printf("Threshold (slope): %d = %d\n",slope_threshold,threshold);
	if (!hf->tmp_buf)
		hf_backup(hf);
	
	if (hf->if_tiles)
		tiling = *hf->if_tiles;
	else
		tiling = TRUE;

	test = y & (gint) 1; // test = TRUE if value is odd
	if ( (test && even) || ((!test) && (!even)) )  //  XOR
		shift = 1;
	else
		shift = 0;
	
	// We need a second temporary buffer for storing
	// the result of each step
	if (!hf->result_buf) {
		hf->result_buf = (hf_type *)x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (hf->result_buf in hf_crests_erosion_hex)");
	}
// printf("CRESTS EROSION, STEPS: %d; THRESHOLD: %d\n", steps, threshold);
	memcpy(hf->result_buf, hf->tmp_buf, hf->max_x*hf->max_y*sizeof(hf_type));
	for (s=0; s<steps; s++) {
	for (y=0; y<hf->max_y; y++) {
		for (x=0; x<hf->max_x; x++) {
			even = rand()%2;

			if (rand()%2)
				axis = V_AXIS;
			else
				axis = H_AXIS;
		
			i = VECTORIZE(x,y,hf->max_x);
			value = *(hf->hf_buf + i);
			if (axis==H_AXIS) {
				xx = x+1;
				yy = y;
			}
			else {
				xx = x;
				yy = y+1;
			}
			if (tiling)
				ii=VECTORIZE(WRAP2(xx,hf->max_x),WRAP2(yy,hf->max_y), hf->max_x);
			else
				ii=VECTORIZE(MIN(xx,hf->max_x-1),MIN(yy,hf->max_y-1), hf->max_x);
			z1 =  *(hf->hf_buf+ii);
			minii = ii;
			lower = z1;
			ratio = 1.0;
			
			if (axis==H_AXIS) {
				xx = x+shift;
				yy = y+1;
			}
			else {
				xx = x+1;
				yy = y+shift;
			}
			
			if (tiling)
				ii=VECTORIZE(WRAP2(xx,hf->max_x), WRAP2(yy,hf->max_y),hf->max_x) ;
			else
				ii=VECTORIZE(MIN(xx,hf->max_x-1), MIN(yy,hf->max_y-1),hf->max_x) ;
			z2 = *(hf->hf_buf+ii);
			if (z2<z1) {
				minii = ii;
				lower = z2;
				ratio = 1.155;
			}
			
			if (axis==H_AXIS) {
				xx = x-1+shift;
				yy = y+1;
			}
			else {
				xx = x+1;
				yy = y-1+shift;
			}
			
			if (tiling)
				ii =VECTORIZE( WRAP2(xx,hf->max_x), WRAP2(yy,hf->max_y),hf->max_x) ;
			else
				ii = VECTORIZE(MIN(MAX(xx,0),hf->max_x-1),MAX(0,yy),hf->max_x) ;
			z3 = *(hf->hf_buf+ii) ;
			if (z3<lower) {
				minii = ii;
				lower = z3;
				ratio = 1.155;
			}
						
			if (axis==H_AXIS) {
				xx = x-1;
				yy = y;
			}
			else {
				xx = x;
				yy = y-1;
			}
			
			if (tiling)
				ii=VECTORIZE(WRAP2(xx,hf->max_x), WRAP2(yy,hf->max_x) , hf->max_x)  ;
			else
				ii=VECTORIZE(MAX(xx,0), MAX(yy,0), hf->max_x)  ;
			z4 = *(hf->hf_buf+ii);
			if (z4<lower) {
				minii = ii;
				lower = z4;
				ratio = 1.0;
			}
									
			if (axis==H_AXIS) {
				xx = x-1+shift;
				yy = y-1;
			}
			else {
				xx = x-1;
				yy = y-1+shift;
			}
			if (tiling)
				ii=VECTORIZE(WRAP2(xx,hf->max_x),
				 WRAP2(yy,hf->max_x),hf->max_x) ;
			else
				ii=VECTORIZE(MAX(xx,0), MAX(yy,0) ,hf->max_x) ;
			z5 =  *(hf->hf_buf+ii) ;
			if (z5<lower) {
				minii = ii;
				lower = z5;
				ratio = 1.155;
			}
												
			if (axis==H_AXIS) {
				xx = x+shift;
				yy = y-1;
			}
			else {
				xx = x-1;
				yy = y+shift;
			}
			if (tiling)
				ii=VECTORIZE(WRAP2(xx,hf->max_x),WRAP2(yy,hf->max_y),hf->max_x) ;
			else
				ii=VECTORIZE(MIN(MAX(0,xx),hf->max_x-1),MAX(yy,0),hf->max_x) ;
			z6 = *(hf->hf_buf+ii);
			if (z6<lower) {
				minii = ii;
				lower = z6;
				ratio = 1.155;
			}
			
			val = ( ( ((gint) *(hf->hf_buf+minii)) - (gint) value ) / 2);
			if (threshold < ABS(val)) {
				*(hf->result_buf+i) = value + (gint) ( ((gfloat) val) / ratio);
//				*(hf->hf_buf+minii) -= (gint) ( ((gfloat) val) / ratio);
			}
			else
				*(hf->result_buf+i) = value;
		}
	}
	memcpy(hf->hf_buf, hf->result_buf, hf->max_x*hf->max_y*sizeof(hf_type));
	}
	
// printf("MAX: %d; MIN: %d; DELTA: %d\n",hf->max, hf->min, hf->max-hf->min);

}


gboolean dfind_all_neighbours( gint x, gint y, gint *newx, gint *newy,
	gint max, gdouble *hf, gdouble threshold) {
	//	Find all the inferior neighbours of (x,y) in hf
	//	Randomly select one of the neighbours,
	//	given its height difference with the minimum is < to threshold
	//	Returns the result in newx and newy
	//	If (x,y) is a minimum, returns FALSE
	static gint v[8], i, ii, nx, ny, ninf, ok;
	gdouble value,z1, z2, z3, z4, z5, z6, z7, z8;
	gdouble lower= (gdouble) MAX_HF_VALUE;
	ninf = 0;
	ok = 0;
	value = *(hf+VECTORIZE(x,y,max)) ;
	ii=VECTORIZE(WRAP2(x+1,max),y,max);
	z1 =  *(hf+ii);
	if (z1<value) {
		v[ninf] = ii;
		ninf++;
	}
	lower = z1;
	nx = WRAP2(x+1,max);
	ny = WRAP2(y+1,max);
	ii=VECTORIZE(nx, ny,max) ;
	z2 = *(hf+ii);
	if (z2<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z2<z1)
		lower = z2;
	ny=WRAP2(y+1,max) ;
	ii =VECTORIZE(x,ny,max) ;
	z3 = *(hf+ii) ;
	if (z3<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z3<lower)
		lower = z3;
	nx = WRAP2(x-1,max);
	ny = WRAP2(y+1,max);
	ii=VECTORIZE(nx,ny,max)  ;
	z4 = *(hf+ii);
	if (z4<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z4<lower)
		lower = z4;
	nx = WRAP2(x-1,max) ;
	ii =VECTORIZE(nx,y,max) ;
	z5 =  *(hf+ii) ;
	if (z5<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z5<lower)
		lower = z5;
	nx = WRAP2(x-1,max);
	ny = WRAP2(y-1,max);
	ii=VECTORIZE(nx, ny,max) ;
	z6 = *(hf+ii);
	if (z6<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z6<lower)
		lower = z6;
	ny = WRAP2(y-1,max) ;
	ii =VECTORIZE(x,ny,max) ;
	z7 = *(hf+ii);
	if (z7<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z7<lower)
		lower = z7;
	nx = WRAP2(x+1,max);
	ny = WRAP2(y-1,max) ;
	ii=VECTORIZE(nx,ny,max)  ;
	z8 = *(hf+ii);
	if (z8<value) {
		v[ninf] = ii;
		ninf++;
	}
	if (z8<lower)
		lower = z8;

// printf("(%d, %d) => LOWER: %5.2f; THRESHOLD: %5.2f; VALUE: %5.2f\n", x, y, lower, threshold, value);
	if ( (lower+threshold) < value) {
		// Find all the pixels not farther away than "threshold" than the lower one
		for (i=0; i<ninf; i++) { 
			if ((*(hf+v[i])-threshold)<=lower) {
				v[ok] = v[i];
				ok ++;
			}
		}
// printf("ok: %d; ",ok);
		ii = v[rand()%ok];
// printf("ii: %d; ",ii);
		*newx = ii%max;
		*newy = (gint) (ii / max);
// printf("(NEWX,NEWY) = (%d,%d) = (%d,%d)\n", ii%max,(gint) ii/max,*newx,*newy);
		return TRUE;
	}
	else {
// printf("Returning FALSE\n");
		return FALSE;
	}
}
void dadd_rain (gdouble *water, gint max_x, gint max_y, gdouble *v, gint vlength) {
	gint x,y;
	for (y=0; y<max_y; y++) {
		for (x=0; x<max_x; x++) {
			// Generate water  
			*(water+VECTORIZE(x,y,max_x)) += v[rand()%vlength];
		}
	}
}
/*
static gdouble *build_map (gint soil_qty, gint radius) {

	gdouble *map, sum, dval, ratio;
	gint i,j,map_size;

	map_size = (2*radius) + 1;
	map = (gdouble *) x_malloc(map_size*map_size*sizeof(gdouble), "gdouble (map in build_map in erode.c");
	sum = 0.0;
	if (!radius)
		sum = (gdouble) soil_qty;
	else {
		for (i=-radius; i<radius+1; i++) {
			for (j=-radius; j<radius+1; j++) {
				dval = BELLD(CONST_E,2.0,i,radius) * 
					BELLD(CONST_E,2.0,j,radius);
				sum += dval;
			}
		}
	}

	ratio = ((gdouble) soil_qty) / sum;
*/
/*
	printf("SUM: %6.3f; RADIUS: %5d; ",sum, radius);
	printf("SOIL_QTY: %5d ",soil_qty);
	printf("RATIO: %6.3f\n", ratio);
*/	
//	printf("MAP %d x %d:\n", (2*radius) + 1, (2*radius) + 1);
/*
	for (i=-radius; i<radius+1; i++) {
		for (j=-radius; j<radius+1; j++) {
			*(map+VECTORIZE(i+radius,j+radius,map_size)) = ratio * BELLD(CONST_E,2.0,i,radius) * 		BELLD(CONST_E,2.0,j,radius);
//			if (radius<4) printf("(%d, %d) == %5.2f;  ",i,j,*(map+VECTORIZE(i,j,map_size)) );
		}
//		if (radius<4) printf("\n");
	}
	return map;
}
*/

void dflow_map(gint x, gint y, gint tox, gint toy, gdouble *hf, gdouble *water, gint max, gboolean wrap) {
	// Flow a quarter of the water (part considered as soil in suspension)
	// between (x,y) and (tox,toy) from (x,y) to (tox,toy)

	gint fromi, isoil; // map_size, toi, radius;
	
	gdouble soil;
	// gdouble *map;
		
	// 1. Check if required map is already cached
	// 2. Build it if not, add to the cache
	
	fromi = VECTORIZE(x,y,max);
	//toi = VECTORIZE(tox,toy,max);
	
	soil = *(water + fromi);
	
// printf("FROM (%d, %d) TO (%d, %d); SOIL: %5.2f; \n", x, y, tox, toy, soil);
	isoil = MIN(MAX_CACHE-1,(gint) soil);

	if (!isoil)
		return;
	
	//radius = (gint) log((gdouble) isoil);
	//map_size = 2*radius +1;
/*	
	map = *(cache+isoil);
	
	if (! map ) {
		map = build_map (isoil, radius);
		*(cache+isoil) = map;
	}
*/
//	if (map_size==1)
		*(water+VECTORIZE(tox,toy,max)) += soil;
//	else
//		generalized_merge ((gpointer) map, GDOUBLE_ID, map_size, map_size, (gpointer) water, GDOUBLE_ID, max, max, tox, toy, ADD, wrap, FALSE);
/*

	if (map_size==1) {
		*(hf+VECTORIZE(x,y,max)) -= soil;
		*(hf+VECTORIZE(tox,toy,max)) += soil;
	}	
	else {
		generalized_merge ((gpointer) map, GDOUBLE_ID, map_size, map_size, (gpointer) hf, GDOUBLE_ID, max, max, x, y, SUBTRACT, wrap, FALSE);
	
		generalized_merge ((gpointer) map, GDOUBLE_ID, map_size, map_size, (gpointer) hf, GDOUBLE_ID, max, max, tox, toy, ADD, wrap, FALSE);
	}


	// We flow all the water mass
	*(water+toi) += *(water+fromi);
	*(water+fromi) = 0.0;
*/
}

void check_water (gdouble *water, gint max) {

	gint i, count=0;
	gdouble sum=0.0;
	
	for (i=0; i<max*max; i++) {
		if (*(water+i)>0.0) {
			count ++;
			sum += *(water+i);
		}
	}
	printf("WATER PIX > 0: %d; SUM: %5.2f; AVRG: %5.2f\n",count, sum, sum / (gdouble) count);

}

void check_hf (gdouble *hf, gint max) {

	gint i, countneg=0, countoverflow=0;
	gdouble sumo=0.0, sumn=0.0;
	
	for (i=0; i<max*max; i++) {
		if (*(hf+i)<1.0) {
			countneg ++;
			sumn += *(hf+i);
		}
		else
		if (*(hf+i)>(gdouble)MAX_HF_VALUE) {
			countoverflow ++;
			sumo += *(hf+i) - (gdouble) MAX_HF_VALUE;
		}
		
		
	}
	printf("HF < 1: %d; AVRG: %7.2f HF > MAX: %d; AVRG: %7.2f\n",countneg, sumn / (gdouble) countneg, countoverflow, sumo / (gdouble) countoverflow);

}

void hf_water_erosion (hf_struct_type *hf, gint steps, gint slope_threshold) {
	// Water / fluvial erosion
	// Simple model without speed and direction matrix
	// Derivated from the rain erosion
	// Basically, we add drops in a water matrix
	// We never spread water explicitly
	// Howvever, we spread soil with a gaussian matrix which size depends
	// on the water weight
	// Buffers:
	//	input, output: height field converted to gdouble, 0 - 0xFFFF -> 0 - 1.0
	//	water = water mass;
	gdouble threshold;
	gdouble *water,*input;
	gint i,s,x,y, nextx, nexty;
	gboolean tiling;
	gdouble vpow2[15] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 4.0, 4.0 ,8.0};
	
//	for (i=0; i<15; i++)
//		vpow2[i] /= (gdouble) MAX_HF_VALUE;
	
	if (hf->if_tiles)
		tiling = *hf->if_tiles;
	else
		tiling = TRUE;
	
	if (!if_cache_init) {
//		printf("@@@@@@@@@@@@@@@@ Initializing CACHE @@@@@@@@@@@@@@@@\n");
		cache = (gdouble **) x_malloc(MAX_CACHE*sizeof(gdouble *), "gdouble * (cache in hf_water_erosion)");
		for (i=0; i<MAX_CACHE; i++) {
			*(cache+i) = NULL;
		}
		if_cache_init = TRUE;
	}
	
	threshold = dget_absolute_slope(slope_threshold,hf->max_x);
	
	input = xalloc_double_hf(hf->max_x, "gdouble (input in hf_water_erosion");
	water = xcalloc_double_hf(hf->max_x, "gdouble (water in hf_water_erosion");
	
	hf_type_to_double (hf->hf_buf, hf->max_x, hf->max_y, input);
	
	// For each step, for each pixel:
	//	1. Add rain drops to the current water mass
	//		Choice at random
	//		water mass = power of 2, from 0 to 3), small chunks more frequent
	//	2. Flow the water mass with a simple rule, like we do with rain
	//		Modify the water matrix
	//		Add / subtract the soil from input to output
	
	for (s=0; s<steps; s++) {
		printf("STEP : %d / %d\n",s, steps);
		dadd_rain (water, hf->max_x, hf->max_y, vpow2, 15);
		
		for (y=0; y<hf->max_y; y++) {
			for (x=0; x<hf->max_x; x++) {
				i = VECTORIZE(x,y,hf->max_x);

				if (dfind_all_neighbours(x,y,&nextx,&nexty,
						hf->max_x,input,threshold))	{
					if (!tiling)
						if ( (x==0) || (y==0) ||
							(x==(hf->max_x-1)) ||
							(y==(hf->max_y-1)) )
								break;
//					printf("Flowing from (%d, %d) to (%d, %d)\n",x,y,nextx,nexty);
					dflow_map(x,y,nextx,nexty,input,water,hf->max_x, tiling);
				}
			}
		}
		for (i=0; i<hf->max_x*hf->max_y; i++)
			*(input+i) = *(water+i) + *(input+i);
//		check_water (water, hf->max_x);
//		check_hf (input, hf->max_x);
	}
	
	double_clamp (hf->hf_buf, hf->max_x, hf->max_y, 0, MAX_HF_VALUE, water);
//	double_clamp (hf->hf_buf, hf->max_x, hf->max_y, 0, MAX_HF_VALUE, input);

	x_free(input);
	x_free(water);
}
