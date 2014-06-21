/* waves.c - waves deformation processing
 *
 * Copyright (C) 2002 Patrice St-Gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "waves.h"
#include "img_process.h"
#include <math.h>

wave_struct *wave_new(gint shape, gint axis, gint order) {
//	Builds a linear structure with some defaults
	wave_struct *ws;
	ws = (wave_struct *) x_malloc(sizeof(wave_struct), "wave_struct");
	ws->order = order;
	ws->shape = shape;
	ws->axis = axis;
	ws->angle = 0;
	ws->period = 5;
	ws->amplitude = 0;
	ws->phase = 0;
	ws->randomness = 0;
	ws->seed = rand();
	return ws;
}

void wave_free(wave_struct *ws) {
	if (ws)
		x_free(ws);
}

gint compare_wave_struct (gpointer data1, gpointer data2) {
//	For use with g_list_sort
	wave_struct *n1, *n2;
	n1 = (wave_struct *) data1;
	n2 = (wave_struct *) data2;
	if (n1->order<n2->order)
		return -1;
	else
		if (n1->order>n2->order)
			return 1;
		else
			return 0;
}

void smooth_line(shape_type *line, gint length, gint radius) {
//	Simple line "block" smoothing
	gint i,j;
	shape_type *tmp;
	long int value;
	tmp = (shape_type *) x_malloc(sizeof(shape_type)*length, "shape_type (tmp in smooth_line)");
	for (i=0; i<length; i++) {
		value = 0;
		for (j=-radius; j<radius; j++) {
			value = value + *(line + WRAP(i + j, length));
		}
		*(tmp+i) = value / (1+(radius<<1));
	}
	memcpy(line, tmp, length*sizeof(shape_type));
	x_free(tmp);
}

shape_type *shape_type_new(gint shape, gint length) {
//	Pre-compiles waves shapes
//	For the wave tool, length is 4096 (max. width or height of a HF)
//	This is for one cycle (summit at 2047)
//	Can be used in other contexts (ex. pen tips in drawing)
//	Data is supposed to be 4 bytes unsigned integer, but ranges
//	from 0 to 0xFFFF (2 bytes)
//	The computing could be done in 2 ways:
//	1- MULTIPLICATIVE:  by multiplying the
//	HF value (2 bytes) with the wave value in a 4 bytes result,
//	then shifting the result right 16 bits to get a 2 bytes HF
//	2- ADDITIVE: subtract from the HF values half of the max wave value,
//	then add the wave value to the HF value

    gint i,end, cutoff;
    gdouble rad, ratio, maxd, dlast_value;
    shape_type *shape_data;
    static gdouble PIx2 = PI * 2.0;
    static gdouble PIx0_5 = PI * 0.5;
    shape_data = (shape_type *) x_malloc(length*sizeof(shape_type), "shape_type");
    maxd = (gdouble) (length - 1);

    switch (shape) {
	case SINE_WAVE:
		for (i=0; i<length; i++) {
			rad = PIx2 * ((gdouble) i) / maxd ;
			*(shape_data+i) = (shape_type) ((MAX_HF_VALUE>>1) * 
					(1.0 + sin(DWRAP(rad-PIx0_5,PIx2))));
		}
		break;

	case CONCAVE_WAVE:
	//	Simple quadratic function, with minima at 0 & length-1
		end = 1+ (length >> 1);
		for (i=0; i<end; i++) {
		//	Gives MAX_HF_VALUE at end-1
			*(shape_data+i) = (shape_type) ( (gdouble) MAX_HF_VALUE *
				( ((gdouble) (i+1)*(i+1)) / (gdouble) (end*end))) ;
		}
		for (i=end; i<length; i++)
			*(shape_data+i) = *(shape_data+length-1-i);
		break;

	case SINE_CONCAVE_WAVE:
	//	Sine when the wave increases, concave when it decreases
		cutoff = (gint) ((2300.0 / 4096.0) * (gdouble) length); // 1/2 ratio ->2049
		for (i=0; i<cutoff; i++) {
			rad = PI * ((gdouble) i) / maxd ;
			*(shape_data+i) = (shape_type) (MAX_HF_VALUE * sin(rad));
		}
		dlast_value =(gdouble) *(shape_data+i-1);
		ratio = dlast_value / (gdouble) MAX(1,((length-cutoff)*(length-cutoff)-1));
		for (i=cutoff; i<length; i++) {
		//	Gives MAX_HF_VALUE at length/2
			*(shape_data+i) = (shape_type) (ratio * (gdouble) MAX(0,((length-i)*(length-i)-1)));
		}
		break;

	case SINE_CONCAVE_2_3_WAVE:
	//	Sine when the wave increases, decreases on 1/6 of the length with sine,
	//	then drops with Pareto 1/x function
	//	2002.04.07:  not satisfied with 2/3 ratio, try 3/4 (cutoff = 3000 / MAX_HF_SIZE)
	//	2004.01.09:  drops with pow(x,3) - smoother.
		cutoff = (gint) ((3000.0 / (gdouble) MAX_HF_SIZE) * (gdouble) length); // 2/3 ratio mean 2731
		for (i=0; i<cutoff; i++) {
			rad = PI * ((gdouble) i) / maxd ;
			*(shape_data+i) = (shape_type) (MAX_HF_VALUE * sin(rad));
// printf("V @ %d: %5.2f\n",i, *(shape_data+i) );
		}
		dlast_value = (gdouble) *(shape_data+i-1);
		ratio = dlast_value / (gdouble) MAX(1,((length-cutoff)*(length-cutoff)*(length-cutoff)-1));

/*		// difd:  correction which zeroizes the function at length-1.
		difd = -dlast_value / pow(maxd-i,0.05);
		// ratio:  after applying difd, we need contiguous values for i=cutoff-1 and i=cutoff
		ratio =  dlast_value / (dlast_value + difd);
// printf("RATIO: %5.2f; POW(maxd-i,0.05): %5.2f\n", ratio,pow(maxd-i,0.05));
*/
		for (i=cutoff; i<length; i++) {
			*(shape_data+i) = (shape_type) (ratio * (gdouble) MAX(0,((length-i)*(length-i)*(length-i)-1)));
//			*(shape_data+i) = (shape_type) MAX(0,(ratio*(difd+(dlast_value / pow((i-cutoff+1),0.05)))));
// printf("V @ %d: %6.2f\n",i, *(shape_data+i) );
		}
		break;
	case TRIANGLE_WAVE:
		end = length >> 1;
		ratio = ((gdouble) MAX_HF_VALUE) / (gdouble) end;
		for (i=0; i<end; i++) {
			*(shape_data+i) = (shape_type) (i*ratio);
// printf("V @ %d: %5.2f\n",i, *(shape_data+i) );
		}
// printf("END: %d; LENGTH: %d\n",end, length);
		for (i=end; i<length; i++) {
			*(shape_data+i) = (shape_type) ((length-i-1)*ratio);
// printf("V @ %d: %5.2f\n",i, *(shape_data+i) );
		}
		break;
	case SQUARE_WAVE:
		for (i=0; i<(length>>2); i++) {
			*(shape_data+i) = (shape_type) 0.0;
		}
		end = (length>>2) + (length >>1);
		for (i=(length>>2); i<end; i++) {
			*(shape_data+i) = (shape_type) MAX_HF_VALUE;
		}
		for (i=end; i<length; i++) {
			*(shape_data+i) = (shape_type) 0.0;
		}
	}
	return shape_data;
}

void vertical_wave(hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, wave_struct *ws, shape_type *sh) {
	gint x,y,i, step_exp, step, nb_wav_val, period, maxperiod;
	gdouble d_indx, value, sine,cosine, phase, *random_displacement, min;
	shape_type ampl, diff, *tmp_sh;
	static gdouble maxd = (gdouble) MAX_HF_VALUE;
	//	Frequency:  HF size / pow(2, ws->period) = size >> ws->period
	//	ADDITIVE merge
	if (!ws->amplitude)
		return;
	ampl = (ws->period*ws->amplitude) / 1000.0;
	diff = ( (shape_type) (0xFFFF>>1) ) * ampl;
//	diff=0.0;
	step_exp = MAX(0,MIN(11,(10-ws->period+11-log2i(max_x)) ));
	nb_wav_val = 4096>>step_exp;
// printf("AMPL: %5.2f, DIFF: %5.2f; NB_WAV_VAL: %d; step_exp: %d; randomness: %d\n",(gfloat) ampl, (gfloat) diff, nb_wav_val, step_exp, ws->randomness);
	sine = sin(PI * ((gdouble) -ws->angle) / 180.0) ;
	cosine = cos(PI * ((gdouble) -ws->angle) / 180.0) ;
	tmp_sh = (shape_type *) x_malloc(nb_wav_val*sizeof(shape_type), "shape_type (tmp_sh in vertical_wave)");
	step = pow(2,step_exp);
	min = HUGE_VAL;
	for (i=0; i<nb_wav_val; i++) {
		*(tmp_sh+i) = *(sh+(i*step)) * ampl;
		if (*(tmp_sh+i)<min)
			min = *(tmp_sh+i);
	}
	// We normalize the vector to 0.0 to avoid non continuous waves,
	// when the random parameter is used
	if (min!=0.0)
		for (i=0; i<nb_wav_val; i++) {
			*(tmp_sh+i)-=min;
		}
	phase = ((gdouble) ws->phase) * ((gdouble) nb_wav_val) / 100.0;

//	Random displacement: we don't need more periods than 
//	~ sqrt(2)*(max_x/nb_wav_val) (diagonal of a 1x1 square)

	maxperiod = MAX(1,(gint) (1.5 * ((gdouble) max_x) / (gdouble) nb_wav_val));
//	printf("MAXPERIOD: %d\n",maxperiod);
	if (ws->randomness)
		srand(ws->seed);
	random_displacement = (gdouble *) x_malloc(sizeof(gdouble)*maxperiod, "gdouble (random_displacement in vertical_wave)");
	for (i=0; i<maxperiod; i++) {
		if (ws->randomness) {
			*(random_displacement+i) = 1.0 +
				((gdouble) ((rand()%ws->randomness) - (ws->randomness>>1))) / 50.0;
		}
		else
			*(random_displacement+i) = 1.0;
//		printf("Random displacement[%d]: %5.2f\n",i,*(random_displacement+i));
	}
	for (y=0; y<max_y; y++) {
		for (x=0; x<max_x; x++) {
			d_indx = phase + (((gdouble)x)*sine+((gdouble)y)*cosine);
			period = WRAP2((gint) d_indx, max_x) / nb_wav_val;
			d_indx = DWRAP(d_indx,nb_wav_val);
			i = (y*max_x)+x;

			vector_interpolate (d_indx,
				tmp_sh, nb_wav_val, &value, GDOUBLE_ID, OVERFLOW_WRAP);
// if ((x==64)&&(*(random_displacement+2)!=1.0))
// printf("VALUE (%d,%d): %5.2f; displ: %5.2f; diff: %5.2f ",x,y,value,*(random_displacement + period), (gfloat) diff );
			value = (*(random_displacement + period) * value) + (gdouble) *(hf_in+i);
// if ((x==64)&&(*(random_displacement+2)!=1.0))
// printf("VALUE2: %5.2f\n",value);

			*(hf_out+i) = (hf_type) MAX(0.0,MIN(value - (gdouble) diff,maxd));
		}
	}
	free(tmp_sh);
}

void horizontal_wave(hf_type *hf_in, hf_type *hf_outx, gint max_x, gint max_y, wave_struct *ws, shape_type *sh) {
	// Interpolated version
	gint x,y,i, step_exp, step, nb_wav_val, phase, div, ampl;
	gint maxperiod, period;
	gdouble *random_displacement,*delta_x=NULL, *delta_y=NULL, min_x, min_y;
	gdouble sine, cosine, cos2, sin2, d_indx, valx=0.0, valy=0.0;
	shape_type diff;
	hf_type *hf_out, value;
	//	Frequency:  HF size / pow(2, ws->period) = size >> ws->period
	//	HF_IN and HF_OUT must be different
	if (!ws->amplitude)
		return;
	if (hf_in == hf_outx)
		hf_out = (hf_type *) x_malloc(max_x*max_y*sizeof(hf_type), "hf_type (hf_out in horizontal_wave)");
	else
		hf_out = hf_outx;
//	DIFF:  the sine / cosine vectors are in absolute values from 0 to 0xFFFF
//	We transform the values to + / - displacements by subtracting half the max
	diff = (shape_type) (0xFFFF>>1);
	step_exp = MAX(0,MIN(11,(10-ws->period+11-log2i(max_x)) ));
	nb_wav_val = 4096>>step_exp;
	delta_x = (gdouble *) x_malloc(nb_wav_val*sizeof(gdouble), "gdouble (delta_x in horizontal_wave)");
	delta_y = (gdouble *) x_malloc(nb_wav_val*sizeof(gdouble), "gdouble (delta_y in horizontal_wave)");
	step = pow(2,step_exp);
	phase = (gint) ((gfloat) ws->phase * 4096.0 / 100.0);
// printf("STEP_EXP: %d;  STEP: %d; DIFF: %d; NB_WAV_VAL: %d\n",step_exp, step, diff, nb_wav_val);
	//	Rotate "in" (angle) = Rotate "out" (-angle)
	sine = sin(PI * ((gdouble) -ws->angle) / 180.0) ;
	cosine = cos(PI * ((gdouble) -ws->angle) / 180.0) ;
	cos2 = cos(PI * ((gdouble) ws->angle) / 180.0) ;
	sin2 = sin(PI * ((gdouble) ws->angle) / 180.0) ;
	div = pow(2,16-(gint) log2i(max_x));
	ampl =  ws->amplitude*ws->period*ws->period/200.0;
// printf("AMPL: %d\n",ampl);
	min_x = min_y = HUGE_VAL;
	for (i=0; i<nb_wav_val; i++) {
		*(delta_x+i) = (cosine * ((ampl * ( *(sh+(i*step))-diff))/100.0) / div);
		*(delta_y+i) = (sine * (gdouble) ((ampl * ( *(sh+(i*step))-diff))/100.0) / div);
		if (min_x>*(delta_x+i))
			min_x = *(delta_x+i);
		if (min_y>*(delta_y+i))
			min_y = *(delta_y+i);
	}
	// Adjust to 0.0 to avoid non continuous waves
	if (min_x!=0.0)
		for (i=0; i<nb_wav_val; i++) {
			*(delta_x+i)-=min_x;
		}
	if (min_y!=0.0)
		for (i=0; i<nb_wav_val; i++) {
			*(delta_y+i)-=min_y;
		}
//	We don't need more periods than ~ sqrt(2)*(max_x/nb_wav_val) (diagonal of a 1x1 square)
	maxperiod = (gint) (1.5 * ((gdouble) max_x) / (gdouble) nb_wav_val);
	random_displacement = (gdouble *) x_malloc(sizeof(gdouble)*maxperiod, "gdouble (random_displacement in horizontal_wave)");
	if (ws->randomness)
		srand(ws->seed);
	for (i=0; i<maxperiod; i++)
		if (ws->randomness) {
			*(random_displacement+i) = 1.0 +
				((gdouble) ((rand()%ws->randomness) - (ws->randomness>>1))) / 50.0;
		}
		else
			*(random_displacement+i) = 1.0;
	for (y=0; y<max_y; y++) {
		for (x=0; x<max_x; x++) {

			d_indx =  ((gdouble) (phase+ max_x)) + x*sin2 + y*cos2 ;
			if (ws->randomness) {
				period = WRAP((gint) d_indx,max_x) / nb_wav_val ;
				d_indx = DWRAP(d_indx,nb_wav_val);
				vector_interpolate (d_indx, delta_x, nb_wav_val, &valx, GDOUBLE_ID, OVERFLOW_WRAP);
				vector_interpolate (d_indx, delta_y, nb_wav_val, &valy, GDOUBLE_ID, OVERFLOW_WRAP);

				interpolate ( ((gdouble) x) +(*(random_displacement+ period) * valx),
						((gdouble) y) + (*(random_displacement+ period) * valy) ,
						hf_in, max_x, max_y, &value, HF_TYPE_ID, OVERFLOW_WRAP);
			}
			else {
				d_indx = DWRAP(d_indx,nb_wav_val);
				vector_interpolate (d_indx, delta_x, nb_wav_val, &valx, GDOUBLE_ID, OVERFLOW_WRAP);
				vector_interpolate (d_indx, delta_y, nb_wav_val, &valy, GDOUBLE_ID, OVERFLOW_WRAP);

				interpolate ( ((gdouble) x) + valx,
						((gdouble) y) + valy,
						hf_in, max_x, max_y, &value, HF_TYPE_ID, OVERFLOW_WRAP);
			}
			*(hf_out+y*max_x +x) = value;
		}
	}
	if (delta_x)
		x_free(delta_x);
	if (delta_y)
		x_free(delta_y);
	if (hf_in == hf_outx) {
		memcpy(hf_outx, hf_out, sizeof(hf_type)*max_x*max_y);
		x_free(hf_out);
	}
}
void waves_apply(hf_struct_type *hf,
		shape_type **wave_shapes,
		gint nb_wav,
		GList *wav_data) {
	gint i;
	hf_type *in;
	GList *ordered_list = NULL,*node = NULL;
	wave_struct *ws;
	shape_type *sh;
	if (!hf->tmp_buf)
		hf_backup(hf);
//	Reorder the list on ws->order
	for (node = wav_data; node; node = node->next)
		ordered_list = g_list_append(ordered_list,node->data);
	ordered_list = g_list_sort(ordered_list, (GCompareFunc) compare_wave_struct);
	i = 0;
	for (node = ordered_list; node; node=node->next) {
		ws = (wave_struct *) node->data;
// printf("Ordre: %d; axe: %d; amplitude: %d; période: %d\n",ws->order, ws->axis, ws->amplitude,ws->period); 

		sh = *(wave_shapes+ws->shape);
		if (!i)	// 1st step, we start with the backuped buffer
			in = hf->tmp_buf;
		else	// We apply the next deformations on the output buffer
			in = hf->hf_buf;
		if (ws->amplitude) 
		// Added 2006-01 - deals with the case when the 1st tab does nothing
			i++;
		if (!sh) {
		// 	"on demand" initialization
			sh = shape_type_new(ws->shape, MAX_HF_SIZE);
			*(wave_shapes+ws->shape) = sh;
		}
		switch (ws->axis) {
			case VERTICAL_WAVE:
				vertical_wave(in, hf->hf_buf, hf->max_x, hf->max_y,ws, sh);
				break;
			case HORIZONTAL_WAVE:
				horizontal_wave(in, hf->hf_buf, hf->max_x, hf->max_y,ws, sh);
				break;
			case CIRCULAR_WAVE:
				break;
		}
	}
	g_list_free(ordered_list);
}

