/* wind.c - Wind erosion functions
 *
 * Copyright (C) 2005 Patrice St-Gelais
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
#include "wind.h"
#include "erode.h"
#include "img_process.h"

#define WIND_RANGE 2048

wind_struct refine_crests = {5, 2, 0, 0.4, 8.0, TRUE, FALSE, 0};

wind_struct *wind_struct_new (gint steps, gint wind_speed, gint sl_thr, gdouble a, gdouble b, gboolean smooth_before, gint radius) {
	
	wind_struct *ws;
	ws = (wind_struct *) x_malloc(sizeof(wind_struct), "wind_struct");
	ws->steps = steps;
	ws->wind_speed = wind_speed; // Relative value
	ws->slope_threshold = sl_thr; // Degrees
	ws->a = a; // Relative value, from 0.0 to 10.0 (slope derivative factor)
	ws->b = b; // Relative value from 0.0 to 10.0 (slope factor)
	ws->refine_crests = TRUE; // For dunes only: apply a slight amount of "ripples"
	ws->smooth_before =  smooth_before;
	ws->radius = radius; // Apply only when smooth_before is TRUE
	
	return ws;
}

void wind_struct_free (wind_struct *ws) {
	if (ws)
		x_free(ws);
}

void hf_ripples1 (hf_struct_type *hf, wind_struct *ws) {

//	Wind erosion - ripples
	
	static gint i,x,y, speed, slope,
		salt_threshold, salt_length, itarget; // slope_threshold;
	
	static hf_type chunk, removal, target;
	
	chunk = 64;
	
	if (!hf->tmp_buf)
		hf_backup(hf);

	// Initialize relative parameters in HF world
	
	speed = (ws->wind_speed*(gint) WIND_RANGE) / 10;
	//slope_threshold =  (gint) (sin(PI * ((gdouble) ws->slope_threshold) / 180.0) * ((gdouble) MAX_HF_VALUE) / (gdouble) hf->max_x);
	salt_threshold = (ws->b*(gint) WIND_RANGE) / 10;
	salt_length = 1+(((ws->a)*hf->max_x)/100);
	
	for (i=0; i<ws->steps; i++) {
		for (y=0; y<hf->max_y; y++) {
			for (x=0; x<hf->max_x; x++) {
				slope = *(hf->hf_buf+VECTORIZE(WRAP2(x+1,hf->max_x),y,hf->max_x)) - *(hf->hf_buf+VECTORIZE(x,y,hf->max_x));
				if ( (speed-slope) > salt_threshold) {
					removal = MIN(chunk,*(hf->hf_buf+VECTORIZE(x,y,hf->max_x)));
					*(hf->hf_buf+VECTORIZE(x,y,hf->max_x))-=removal;
					itarget = VECTORIZE(WRAP2(x+salt_length,hf->max_x),y,hf->max_x);
					target = *(hf->hf_buf+itarget);
					*(hf->hf_buf+itarget) = target + MIN(removal,MAX_HF_VALUE-target);
//				printf("SLOPE: %d; REMOVAL: %d; TARGET: %d\n",slope, removal, target, *(hf->hf_buf+itarget));
				}	
			}
		}
	}
}

void hf_ripples3 (hf_struct_type *hf, wind_struct *ws) {

//	Wind erosion - ripples
//	From Andreotti - Claudin - Douady 2D model
//	sand_flux = current sand flux
//	sat_sand_flux = saturated sand flux (depending on the slope and on
//	the variation of the slope)
//	Initial sand flux is equal to the wind speed on a flat ground
//	Saturated sand flux is equal to: 1 - A*h'' + B*h'
//	h being the height, h' the slope, h'' the variation of the slope
//	If sand flux is superior than saturated sand flux, there is a deposit
//	If not, there is erosion
//	slope_threshold = relaxation due to gravity
	
	static gint i,x,y, sat_sand_flux, slope1, slope2, slope_derivative, diff, WIND; // slope_threshold, 
	
	static hf_type value;
	
	static gdouble A,B;
	
	gint *vsand_flux;

	if (!hf->tmp_buf)
		hf_backup(hf);

	// Initialize relative parameters in HF world
	
	A = ws->a / 10.0;
	B = ws->b / 10.0;
	
	WIND = (gint) pow(2.0,(gdouble) ws->wind_speed);
//	printf("WIND: %d\n",WIND);

	vsand_flux = (gint *) x_malloc(sizeof(gint)*hf->max_y, "gint (vsand_flux in hf_ripples3)");
	
	for (i=0; i<hf->max_y; i++)
		*(vsand_flux+i) = WIND;
		
	//slope_threshold = iget_absolute_slope (ws->slope_threshold, hf->max_x);
	
	for (i=0; i<ws->steps; i++) {
		for (y=0; y<hf->max_y; y++) {	
		
			slope1 = *(hf->hf_buf+VECTORIZE(0,y,hf->max_x)) - *(hf->hf_buf+VECTORIZE(WRAP2(-1,hf->max_x),y,hf->max_x));
			for (x=0; x<hf->max_x; x++) {
				value = *(hf->hf_buf+VECTORIZE(x,y,hf->max_x));
				slope2 = *(hf->hf_buf+VECTORIZE(WRAP2(x+1,hf->max_x),y,hf->max_x)) - value;
				slope_derivative = slope2 - slope1;
				
//				sat_sand_flux = (gint) ( (WIND_RANGE * 0.5 *(1.0 - (((gdouble) value)/(gdouble) MAX_HF_VALUE))) + ( WIND_RANGE * 0.5 * MAX(0, 1.0 - (A* (gdouble) slope_derivative + B* (gdouble) slope2  )/WIND_RANGE)));

				sat_sand_flux = (gint) ( WIND * MAX(0, 1.0 - (A* (gdouble) slope_derivative - B* (gdouble) slope2  )/WIND));
				
				diff = sat_sand_flux - vsand_flux[y];
				
				// If diff>0, remove sand, add it up to sand_flux
				// Otherwise, deposit sand
				// In both cases, sand_flux takes the value of sat_sand_flux,
				// except where there is not enough sand to remove
// if (((i==0) || (i==5)) && (y==256))
//	printf("STEP: %d (%d,%d) = %d; SAT_FLUX = %d; SAND_FLUX = %d; diff = %d; SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; \n",i,x,y,value,sat_sand_flux, vsand_flux[y], diff, slope1, slope2, slope_derivative);
				if (!diff) {
					slope1 = slope2;
					continue;
				}
				if (diff>0)
					diff = MIN(diff,value);
				else
					diff = -MIN(ABS(MAX_HF_VALUE-value),ABS(diff));
				
//				add_spread_3x3 (hf->hf_buf, hf->max_x, x, y, -diff, TRUE);
				
				*(hf->hf_buf+VECTORIZE(x,y,hf->max_x)) = value - diff;
				
// printf("(%d, %d) => SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; VALUE: %d; DIFF: %d; sat_sand_flux: %d; sand_flux: %d; new sat_sf: %d\n",x,y, slope1, slope2, slope_derivative, value, diff, sat_sand_flux, sand_flux, sand_flux+diff);

				vsand_flux[y] += diff;
				slope1 = slope2;
			}
		}
	}
	x_free (vsand_flux);
}

void hf_ripples2 (hf_struct_type *hf, wind_struct *ws) {

//	Wind erosion - ripples
//	From Andreotti - Claudin - Douady 2D model
//	sand_flux = current sand flux
//	sat_sand_flux = saturated sand flux (depending on the slope and on
//	the variation of the slope)
//	Initial sand flux is equal to the wind speed on a flat ground
//	Saturated sand flux is equal to: 1 - A*h'' + B*h'
//	h being the height, h' the slope, h'' the variation of the slope
//	If sand flux is superior than saturated sand flux, there is a deposit
//	If not, there is erosion
//	slope_threshold = relaxation due to gravity
	
	static gint i,x,y, sand_flux, sat_sand_flux, slope1, slope2, slope_derivative, diff; // slope_threshold, 
	
	static hf_type value;
	
	static gdouble A,B;

	if (!hf->tmp_buf)
		hf_backup(hf);

	// Initialize relative parameters in HF world
	
	A = ws->a;
	B = ws->b;

	//slope_threshold = iget_absolute_slope (ws->slope_threshold, hf->max_x);
	
	sand_flux = (ws->wind_speed*(gint) WIND_RANGE) / 10;

//	printf("Initial sand flux: %d; SLOPE_THRESHOLD: %d;\n",sand_flux, slope_threshold);
	
	for (i=0; i<ws->steps; i++) {
		for (y=0; y<hf->max_y; y++) {	
			slope1 = *(hf->hf_buf+VECTORIZE(0,y,hf->max_x)) - *(hf->hf_buf+VECTORIZE(WRAP2(-1,hf->max_x),y,hf->max_x));
			for (x=0; x<hf->max_x; x++) {
				value = *(hf->hf_buf+VECTORIZE(x,y,hf->max_x));
				slope2 = *(hf->hf_buf+VECTORIZE(WRAP2(x+1,hf->max_x),y,hf->max_x)) - value;
				slope_derivative = (slope2 - slope1) / 2;
				sat_sand_flux = MAX(0,WIND_RANGE - A*slope_derivative + B*slope2);				
//				sat_sand_flux = (gint) ( (WIND_RANGE * 0.5 *(1.0 - (((gdouble) value)/(gdouble) MAX_HF_VALUE))) + ( WIND_RANGE * 0.5 * MAX(0, 1.0 - (A* (gdouble) slope_derivative + B* (gdouble) slope2  )/WIND_RANGE)));

				diff = sat_sand_flux - sand_flux;
				// If diff>0, remove sand, add it up to sand_flux
				// Otherwise, deposit sand
				// In both cases, sand_flux takes the value of sat_sand_flux,
				// except where there is not enough sand to remove
				if (!diff)
					continue;
				if (diff>0)
					diff = MIN(diff,value);
				else
					diff = -MIN(MAX_HF_VALUE-value,ABS(diff));
				*(hf->hf_buf+VECTORIZE(x,y,hf->max_x)) = value - diff;
				
// printf("(%d, %d) => SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; VALUE: %d; DIFF: %d\n",x,y, slope1, slope2, slope_derivative, value, diff);
				slope1 = slope2;
			}
		}
	}
}

gdouble convol_slope_derivative (hf_type *hf, gint x, gint y, gint max, gint window) {
//	Slope derivative 1/x convolution on the x axis
//	We add "window" points backward starting from (x,y)
	gdouble sum;
	gint i;
	gdouble value0, value1, value2;
	sum = 0.0;
	value0 = (gdouble) *(hf+VECTORIZE(x,y,max));
	value1 =  (gdouble) *(hf+VECTORIZE(WRAP2(x-1,max),y,max));
	for (i=1; i<window; i++) {
		value2 =  (gdouble) *(hf+VECTORIZE(WRAP2(x-i-1,max),y,max));
		sum += (value0 - 2*value1 + value2) / ((gdouble) (i+1));
//	if (y==255) printf("SL_DERIV: (%5.2f, %5.2f, %5.2f) = %d\n",value0, value1, value2, (gint) (value0 - 2*value1 + value2));
		value0 = value1;
		value1 = value2;
	}
//	if (y==255) printf("CONVOL SLOPE (%d,%d): %d\n",x,y,(gint) sum);
	return sum;
}

gdouble convol_slope (hf_type *hf, gint x, gint y, gint max, gint window) {
//	Slope 1/x convolution on the x axis
//	We add "window" points backward starting from (x,y)
	gdouble sum;
	gint i;
	gdouble value0, value1;
	sum = 0.0;
	value0 = (gdouble) *(hf+VECTORIZE(x,y,max));
	for (i=1; i<window; i++) {
		value1 =  (gdouble) *(hf+VECTORIZE(WRAP2(x-i,max),y,max));
		sum += (value0 - value1) / ((gdouble) (i+1));
//	if (y==255) printf("SL_CONVOL: (%5.2f, %5.2f) = %d\n", value0, value1, (gint) (value0 - value1));
//		value0 = value1;
	}
//	if (y==255) printf("CONVOL SLOPE (%d,%d): %d\n",x,y,(gint) sum);
	return sum;
}

gdouble avrg_slope (hf_type *hf, gint x, gint y, gint max, gint window) {
//	Slope average on the x axis
//	We add "window" points backward starting from (x,y)
	gdouble sum;
	gint i;
	gdouble value0, value1;
	sum = 0.0;
	value0 = (gdouble) *(hf+VECTORIZE(x,y,max));
	for (i=1; i<window; i++) {
		value1 =  (gdouble) *(hf+VECTORIZE(WRAP2(x-i,max),y,max));
		sum += value0 - value1;
//	if (y==255) printf("SL_CONVOL: (%5.2f, %5.2f) = %d\n", value0, value1, (gint) (value0 - value1));
		value0 = value1;
	}
//	if (y==255) printf("AVRG SLOPE (%d,%d): %d\n",x,y,(gint) (sum/i));
	return sum/i;
}

void hf_ripples4 (hf_struct_type *hf, wind_struct *ws) {

//	Wind erosion - ripples
//	From Andreotti - Claudin - Douady 2D model
//	sand_flux = current sand flux
//	sat_sand_flux = saturated sand flux (depending on the slope and on
//	the variation of the slope)
//	Initial sand flux is equal to the wind speed on a flat ground
//	Saturated sand flux is equal to: 1 - A*h'' + B*h'
//	h being the height, h' the slope, h'' the variation of the slope
//	If sand flux is superior than saturated sand flux, there is a deposit
//	If not, there is erosion
//	slope_threshold = relaxation due to gravity
	
	static gint i,x,y, sat_sand_flux, diff, wind, vsize; // slope_threshold, 
	
	static gdouble slope_derivative;
	
	static hf_type value, *vbuf;
	
	static gdouble A,B, WIND, slope;
	
	gint *vsand_flux;

	if (!hf->tmp_buf)
		hf_backup(hf);

	// Initialize relative parameters in HF world
	
	A = ws->a / 10.0;
	B = ws->b / 10.0;
	
	wind = 1 + ws->wind_speed*50;
	WIND = (gdouble) wind;
	
//	printf("WIND: %d\n",wind);

	vsand_flux = (gint *) x_malloc(sizeof(gint) * hf->max_y, "gint (vsand_flux in hf_ripples4)");
	vsize = sizeof(hf_type) * hf->max_x;
	vbuf = (hf_type *) x_malloc(vsize, "hf_type (vbuf in hf_ripples4)");
	
	for (i=0; i<hf->max_y; i++)
		*(vsand_flux+i) = wind;
		
	//slope_threshold = iget_absolute_slope (ws->slope_threshold, hf->max_x);
	
	for (i=0; i<ws->steps; i++) {
		for (y=0; y<hf->max_y; y++) {	
			vbuf = memcpy(vbuf,hf->hf_buf+y*hf->max_x,vsize);
			for (x=0; x<hf->max_x; x++) {
				
				value = *(hf->hf_buf+VECTORIZE(x,y,hf->max_x));
				// slope = (gdouble) (value - *(hf->hf_buf+VECTORIZE(WRAP2(x-1,hf->max_x),y,hf->max_x)));
				slope = (gdouble) (value - *(vbuf+VECTORIZE(WRAP2(x-1,hf->max_x),0,hf->max_x)));
								
				// slope_derivative = convol_slope (hf->hf_buf, x, y, hf->max_x, 8);
				slope_derivative = convol_slope (vbuf, x, 0, hf->max_x, 8);
								
				sat_sand_flux = (gint) MAX(0, WIND + A* slope_derivative + B * slope ) ;
				
				diff = sat_sand_flux - vsand_flux[y];
				
				// If diff>0, remove sand, add it up to sand_flux
				// Otherwise, deposit sand
				// In both cases, sand_flux takes the value of sat_sand_flux,
				// except where there is not enough sand to remove
// if (((i==0) || (i==5)) && (y==256))
//	printf("STEP: %d (%d,%d) = %d; SAT_FLUX = %d; SAND_FLUX = %d; diff = %d; SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; \n",i,x,y,value,sat_sand_flux, vsand_flux[y], diff, slope1, slope2, slope_derivative);
				if (!diff) {
					continue;
				}
				if (diff>0)
					diff = MIN(diff,value);
				else
					diff = -MIN(ABS(MAX_HF_VALUE-value),ABS(diff));
				
//				add_spread_3x3 (hf->hf_buf, hf->max_x, x, y, -diff, TRUE);
				
				*(hf->hf_buf+VECTORIZE(x,y,hf->max_x)) = value - diff;
				
// printf("(%d, %d) => SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; VALUE: %d; DIFF: %d; sat_sand_flux: %d; sand_flux: %d; new sat_sf: %d\n",x,y, slope1, slope2, slope_derivative, value, diff, sat_sand_flux, sand_flux, sand_flux+diff);

				vsand_flux[y] += diff;
			}
		}
	}
	x_free (vsand_flux);
	x_free (vbuf);
}

void hf_ripples5 (hf_struct_type *hf, wind_struct *ws) {

//	Wind erosion - ripples
//	From Andreotti - Claudin - Douady 2D model
//	sand_flux = current sand flux
//	sat_sand_flux = saturated sand flux (depending on the slope and on
//	the variation of the slope)
//	Initial sand flux is equal to the wind speed on a flat ground
//	Saturated sand flux is equal to: 1 - A*h'' + B*h'
//	h being the height, h' the slope, h'' the variation of the slope
//	If sand flux is superior than saturated sand flux, there is a deposit
//	If not, there is erosion
//	slope_threshold = relaxation due to gravity

//	Version with no sat_sand_flux
//	The core formula gives a ratio which is removed from H
	
	static gint i,x,y, diff, wind, vsize; // slope_threshold, 
	
	static gdouble slope_derivative; // prec_slope,
	
	static hf_type value, *vbuf;
	
	static gdouble A,B, WIND, slope;
	
	gint *vsand_flux;

	if (!hf->tmp_buf)
		hf_backup(hf);

	// Initialize relative parameters in HF world
	
	A = ws->a / 100.0;
	B = ws->b / 100.0;
	
	wind = 1 + ws->wind_speed*50;
	WIND = (gdouble) wind;
	
//	printf("WIND: %d\n",wind);

	vsand_flux = (gint *) x_malloc(sizeof(gint) * hf->max_y, "hf_type (vbuf in hf_ripples5)");
	vsize = sizeof(hf_type) * hf->max_x;
	vbuf = (hf_type *) x_malloc(vsize, "hf_type (vbuf in hf_ripples5)");
	
	for (i=0; i<hf->max_y; i++)
		*(vsand_flux+i) = wind;
		
	//slope_threshold = iget_absolute_slope (ws->slope_threshold, hf->max_x);
	
	for (i=0; i<ws->steps; i++) {
		for (y=0; y<hf->max_y; y++) {	
			
			vbuf = memcpy(vbuf,hf->hf_buf+y*hf->max_x,vsize);		
			//prec_slope = avrg_slope (vbuf, -1, 0, hf->max_x, 4);
			
			for (x=0; x<hf->max_x; x++) {
				
				value = *(hf->hf_buf+VECTORIZE(x,y,hf->max_x));
				
				// slope = (gdouble) (value - *(hf->hf_buf+VECTORIZE(WRAP2(x-1,hf->max_x),y,hf->max_x)));
				// slope = (gdouble) (value - *(vbuf+VECTORIZE(WRAP2(x-1,hf->max_x),0,hf->max_x)));
				
				slope = avrg_slope (vbuf, x, 0, hf->max_x, 8);
				
//				slope_derivative = slope - prec_slope;
						
				slope_derivative = convol_slope (vbuf, x, 0, hf->max_x, 8);
						
				// slope_derivative = convol_slope (hf->hf_buf, x, y, hf->max_x, 8);
				// slope_derivative = convol_slope (vbuf, x, 0, hf->max_x, 8);
								
				// sat_sand_flux = (gint) MAX(0, WIND + A* slope_derivative + B * slope ) ;
				
				// diff = sat_sand_flux - vsand_flux[y];
				
				diff = (gint) (WIND * (B*slope + A*slope_derivative));
				
				// If diff>0, remove sand, add it up to sand_flux
				// Otherwise, deposit sand
				// In both cases, sand_flux takes the value of sat_sand_flux,
				// except where there is not enough sand to remove
				
				//prec_slope = slope;
				
				if (!diff) {
					continue;
				}
				if (diff>0)
					diff = MIN(diff,value);
				else {
					diff = -MIN(ABS(MAX_HF_VALUE-value),ABS(diff));
					diff = -MIN(ABS(vsand_flux[y]), ABS(diff));
				}
				
// if (y==256)
//	printf("STEP: %d (%d,%d) = %d; SAND_FLUX = %d; diff = %d; SLOPE: %d; SLOPE_DERIV: %d; \n",i,x,y,value, vsand_flux[y], diff, (gint) slope, (gint) slope_derivative);
//				add_spread_3x3 (hf->hf_buf, hf->max_x, x, y, -diff, TRUE);
				
				*(hf->hf_buf+VECTORIZE(x,y,hf->max_x)) = value - diff;
				
// printf("(%d, %d) => SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; VALUE: %d; DIFF: %d; sat_sand_flux: %d; sand_flux: %d; new sat_sf: %d\n",x,y, slope1, slope2, slope_derivative, value, diff, sat_sand_flux, sand_flux, sand_flux+diff);

				vsand_flux[y] += diff;
			}
		}
	}
	x_free (vsand_flux);
	x_free (vbuf);
}

void hf_ripples (hf_struct_type *hf, wind_struct *ws, dist_matrix_struct *dm, gdouble **gauss_list) {

//	Wind erosion - ripples
//	From Andreotti - Claudin - Douady 2D model
//	sand_flux = current sand flux
//	sat_sand_flux = saturated sand flux (depending on the slope and on
//	the variation of the slope)
//	Initial sand flux is equal to the wind speed on a flat ground
//	Saturated sand flux is equal to: 1 - A*h'' + B*h'
//	h being the height, h' the slope, h'' the variation of the slope
//	If sand flux is superior than saturated sand flux, there is a deposit
//	If not, there is erosion
//	slope_threshold = relaxation due to gravity
	
	static gint i,x,y, sat_sand_flux, diff, wind; 
	
	static hf_type value;
	
	static gdouble A,B, WIND, slope;
	
	gint *vsand_flux;

//	printf("HF_RIPPLES; ws->steps: %d; ws->wind_speed: %d; ws->sl_thr: %d; ws->a: %5.2f; ws->b: %5.2f\n", ws->steps, ws->wind_speed, ws->slope_threshold, ws->a, ws->b);
	if (!hf->tmp_buf)
		hf_backup(hf);

	if (ws->smooth_before) {
		hf_smooth(hf, ws->radius, dm,  (hf->if_tiles==NULL) ? TRUE : *hf->if_tiles, gauss_list);
	}
	
	// Initialize relative parameters in HF world
	A = ws->a * 10.0;
	B = ws->b / 10.0;
	
	wind = 1 + ws->wind_speed*50;
	WIND = (gdouble) wind;
	
// printf("WIND: %d\n",wind);

	vsand_flux = (gint *) x_malloc(sizeof(gint)*hf->max_y, "hf_type (vbuf in hf_ripples)");
	
	for (i=0; i<hf->max_y; i++)
		*(vsand_flux+i) = wind;
	
	for (i=0; i<ws->steps; i++) {
		for (y=0; y<hf->max_y; y++) {	
		
			for (x=0; x<hf->max_x; x++) {
				
				value = *(hf->hf_buf+VECTORIZE(x,y,hf->max_x));
				
				slope = avrg_slope (hf->hf_buf, x, y, hf->max_x, (gint) A);
//				slope = (gdouble) ( value - *(hf->hf_buf+VECTORIZE(WRAP2(x-1,hf->max_x),y,hf->max_x)));
		
				sat_sand_flux = (gint) MAX(0, WIND + B * slope ) ;
				
				diff = sat_sand_flux - vsand_flux[y];
				
				// If diff>0, remove sand, add it up to sand_flux
				// Otherwise, deposit sand
				// In both cases, sand_flux takes the value of sat_sand_flux,
				// except where there is not enough sand to remove
// if (((i==0) || (i==5)) && (y==256))
//	printf("STEP: %d (%d,%d) = %d; SAT_FLUX = %d; SAND_FLUX = %d; diff = %d; SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; \n",i,x,y,value,sat_sand_flux, vsand_flux[y], diff, slope1, slope2, slope_derivative);
				if (!diff) {
					continue;
				}
				if (diff>0)
					diff = MIN(diff,value);
				else
					diff = -MIN(ABS(MAX_HF_VALUE-value),ABS(diff));
				
//				add_spread_3x3 (hf->hf_buf, hf->max_x, x, y, -diff, TRUE);
				
				*(hf->hf_buf+VECTORIZE(x,y,hf->max_x)) = value - diff;
				
// printf("(%d, %d) => SLOPE1: %d; SLOPE2: %d; SLOPE_DERIV: %d; VALUE: %d; DIFF: %d; sat_sand_flux: %d; sand_flux: %d; new sat_sf: %d\n",x,y, slope1, slope2, slope_derivative, value, diff, sat_sand_flux, sand_flux, sand_flux+diff);

				vsand_flux[y] += diff;
			}
		}
		if (ws->slope_threshold)
			hf_oriented_relax (hf, 1, ws->slope_threshold, EAST);
	}
	x_free (vsand_flux);
}

void hf_dunes (hf_struct_type *hf, wind_struct *ws, dist_matrix_struct *dm, gdouble **gauss_list) {

//	Wind erosion - ripples
//	Adapted from Andreotti - Claudin - Douady 2D model
//	sand_flux = current sand flux
//	sat_sand_flux = saturated sand flux 
//	This adaptation depends only on the slope. 
//	The original model also depends on the second derivative
//	Initial sand flux is equal to the wind speed on a flat ground
//	Saturated sand flux is equal to: 1 + B*h'
//	h being the height, h' the slope
//	If sand flux is superior than saturated sand flux, there is a deposit
//	If not, there is erosion
//	slope_threshold = relaxation due to gravity
	
	static gint i, x, y, diff; 
	
	static hf_type value;
	
	static glong sat_sand_flux,wind, slope, B;

	if (!hf->tmp_buf)
		hf_backup(hf);

	if (ws->smooth_before) {
		hf_smooth(hf, ws->radius, dm, (hf->if_tiles==NULL) ? TRUE : *hf->if_tiles, gauss_list);
	}
	
	// Initialize relative parameters in HF world

	B = ws->b;
	
	wind = 1.0 + ((gdouble) ws->wind_speed) *50.0;
	
// printf("WIND: %d\n",WIND);

	for (i=0; i<ws->steps; i++) {
		for (y=0; y<hf->max_y; y++) {	

			for (x=0; x<hf->max_x; x++) {
				value = *(hf->hf_buf+VECTORIZE(x,y,hf->max_x));
				
				slope = (glong) (value - *(hf->hf_buf+VECTORIZE(WRAP2(x-1,hf->max_x),y,hf->max_x)));
				
				sat_sand_flux = MAX(0.0, wind +  B * slope / 10);
				
				// If diff>0, remove sand
				// Otherwise, deposit sand
				// wind == sand_flux
								
				diff = (gint) ( sat_sand_flux - wind);

				if (!diff) {
					continue;
				}
				if (diff>0)
					diff = MIN(diff,value);
				else
					diff = -MIN(ABS(MAX_HF_VALUE-value),ABS(diff));
				
				*(hf->hf_buf+VECTORIZE(x,y,hf->max_x)) = value - diff;
				
			}
		}
		// Relax once each step
		if (ws->slope_threshold)
			hf_oriented_relax (hf, 1, ws->slope_threshold, EAST);
	}
	if (ws->refine_crests) {
		hf_ripples (hf, &refine_crests, dm, gauss_list);
	}
}
