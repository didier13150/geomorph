/* img_process.c - functions for filtering and transforming images
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

#include <string.h>
#include <math.h>
#include "img_process.h"
#include "hf.h"
#include "hf_calc.h"
#include "hf_filters.h"
#include "../fourier/fourier.h"

void hf_auto_contrast (hf_struct_type *hf) {
//	Stretch contrast so that the full value range is used (0 to 0xFFFF)
	hf_clamp (hf, 0, 0xFFFF);
}

void hf_brightness_contrast(hf_struct_type *hf, gint contrast_level, gint br_level,
		gboolean keep_luminosity, gboolean bound_overflow) {
//	
//	Increases / decreases the contrast & brightness of a square size X size 16 greyscale image
//	Levels are a % from -100 to 100
//	The brightness process is additive
//	At -100%, we subtract 0xFFFF>>1;  at 100%, we add 0xFFFF>>1.
//	The contrast is relative to the medium value between the min and max
//	The input is hf->tmp_buf (backup of the displayed buffer before getting into
//	the contrast dialog) and the output is the displayed buffer hf->hf_buf
//	Min and Max should be updated BEFORE, when entering into the dialog

//	The contrast process adjusts the delta between min and max value, so that
//	the new delta is 0 at -100% level and 2x at 100% level

	gfloat flevel;
	gint i, max = (gint) 0xFFFF, max2 , value, offset, delta_brightness ;
	max2 = 2 * max - 1;
	if (!hf->tmp_buf)
		hf_backup(hf);
	delta_brightness =  (gint) ( (0xFFFF>>1) * ((gfloat) br_level / 100));
//	flevel ranges from 0.0 to 2.0
	flevel =  ((gfloat) (100 + contrast_level)) / 100 ;
	if (keep_luminosity)
		offset = hf->avrg * (1.0-flevel);
	else
		offset = 0;
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		//	If contrast_level < 0, we apply brightness 2nd
		if (contrast_level < 0) {
			if (bound_overflow) {
			//	Contrast
				*(hf->hf_buf + i) = MAX(0,MIN(0xFFFF, offset 
					+ ((gfloat) *(hf->tmp_buf + i)) * flevel));
			//	Brightness
				if (br_level)
					*(hf->hf_buf + i) = 
						(hf_type) MIN(0xFFFF,MAX(0,
						(gint) *(hf->hf_buf + i) + delta_brightness));
			}
			else { // Reverted overflow
			// 	Contrast
				value = offset + ((gfloat) *(hf->tmp_buf+i)) * flevel;
			// 	Brightness
				if (br_level)
					value = value + delta_brightness;
				value = ABS(value);
				value = (value>=max)?(max2-value):value;
				*(hf->hf_buf+i) = (hf_type) value;
			}
		}
		else {
			if (bound_overflow) {
			//	Brightness
				*(hf->hf_buf + i) = 
					(hf_type) MIN(0xFFFF,MAX(0,
						(gint) *(hf->tmp_buf + i) + delta_brightness));
			//	Contrast
				if (contrast_level)
					*(hf->hf_buf + i) = MAX(0,MIN(0xFFFF, offset 
						+ ((gfloat) *(hf->hf_buf + i)) * flevel));
			}
			else { // Reverted overflow
			// 	Brightness
				value =  *(hf->tmp_buf+i) + delta_brightness;
			// 	Contrast
				if (contrast_level)
					value = offset + ((gfloat) value) * flevel;
				value = ABS(value);
				value = (value>=max)?(max2-value):value;
				*(hf->hf_buf+i) = (hf_type) value;
			}
		}
	}
}

void hf_remove_artifacts (hf_struct_type *hf, gint radius, dist_matrix_struct *dm) {
	// Removing artifacts in a quantized image (terrace)
	// Move a window of a given radius over the image
	// and fill it with the border color if all the border pixels are the same
	// In the simplest form, the window is square
	// The function "wraps"
	gint i, j, k, l;
	hf_type value;
	gboolean test;
	for (i=0; i<hf->max_x; i++) {
		for (j=0; j<hf->max_y; j++) {
			value = *(hf->hf_buf + i + j*hf->max_x);
			test = TRUE;
			for (k=-radius; k<radius+1; k++) {
				if (value != *(hf->hf_buf + WRAP2(i - radius,hf->max_x)
					 + WRAP2((j+k),hf->max_y) *hf->max_x)) {
					test = FALSE;
					break;
				}
				if (value != *(hf->hf_buf + WRAP2(i + radius,hf->max_x)
					 + WRAP2((j+k),hf->max_y) *hf->max_x)) {
					test = FALSE;
					break;
				}
			} // End for k (1st)
			if (!test)
				continue;
			for (k = -radius+1; k<radius; k++) {
				if (value != *(hf->hf_buf + WRAP2(i + k,hf->max_x)
					 + WRAP2((j-radius),hf->max_y)*hf->max_x)) {
					test = FALSE;
					break;
				}
				if (value != *(hf->hf_buf + WRAP2(i + k,hf->max_x)
					 + WRAP2((j+radius),hf->max_y)*hf->max_x)) {
					test = FALSE;
					break;
				}
			} // End for k (2nd)
 			if (!test)
				continue;
			// We fill up the window with "value"
			for (k=-radius+1; k<radius; k++)
				for (l=-radius+1; l<radius; l++)
					*(hf->hf_buf+VECTORIZE(
						WRAP2(i+l,hf->max_x),
						WRAP2(j+k,hf->max_y),
							hf->max_x) )= value;
					
		} // Enf for j
	} // End for i
}

void hf_terrace (hf_struct_type *hf, gint levels, gint seed, gint percent_random,
	gint smooth_radius, gboolean if_wrap, 
	dist_matrix_struct *dist_matrix,
	gint artifact_radius, gboolean apply_postprocess,
	gdouble **gauss_list) {

	gint i,*disturbance_vector, index, slice, index_offset;

	if (!levels)
		return;
	slice = (hf->max-hf->min) / levels;
	//	No process if slice is 0 - HF is uniform
	if (!slice)
		return;
	// Modified 2005-06-23: s_offset and r_offset are now %,not absolute values
	/*
	s_offset = (slice* source_offset) / max_f;	// source_offset and result_offset are % of slice
	r_offset = (slice* result_offset) / max_f ;
printf("slice: %d; s_offset: %f;  r_offset: %f\n",slice, s_offset, r_offset);
	*/
	if (!hf->tmp_buf)
		hf_backup(hf);
	// We need a second temporary buffer for storing
	// the result of the quantization process before the postprocessing
	if (!hf->result_buf) {
		hf->result_buf = (hf_type *)x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (result_buf in hf_terrace)");
	}
//	Variation implementing random disturbance of each level
//	Disturbance is bounded by 0 and slice_size
//	(NOTE: the terrace effect is calculating by "flooring" a value
//	to the nearest lower multiple of slice_size, so the total H
//	should never be more than 0xFFFF)
/*	disturbance_vector = (gint *) x_malloc(sizeof(gint)*(levels+1), "gint (disturbance_vector in hf_terrace)");
	srand(seed);
	*(disturbance_vector) = 0;	// We want the "floor" to stay black
	for (i=1; i<levels+1; i++) {
		*(disturbance_vector+i) = (percent_random*(rand()%slice)) / 100;
	}
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		index = *(hf->tmp_buf + i) / slice;
		*(hf->hf_buf + i) = ((gfloat) *(hf->tmp_buf +i)* (100-percent) / 100) +
			((gfloat) ( (*(disturbance_vector+index) +
			(slice * index) )* percent) / 100);
	}
*/
//	2nd variation
//	The disturbance vector is applied when quantizing, not after
	disturbance_vector = (gint *) x_malloc(sizeof(gint)*(levels+1), "gint (disturbance_vector in hf_terrace)");
	srand(seed);
	*(disturbance_vector) = hf->min;	// We want the "floor" to stay at minimum

	for (i=1; i<levels+1; i++) {
		*(disturbance_vector+i) = (percent_random*(rand()%slice)) / 100;
		// 	For the quantization algorithm to work, 
		// 	each slice displacement must be strictly < to slice_size (never =)
		*(disturbance_vector+i) = MIN(-1+((i+1)*slice), *(disturbance_vector+i));
		// 	Contrary to the simpler preceding algorithm,
		// 	the quantization requires cumulative values, so that we can
		// 	rapidly check upper and lower bounds
		*(disturbance_vector+i) = MIN(0xFFFF,hf->min + (i*slice)+*(disturbance_vector+i));
// printf("DIST-VECT[%d] = %d\n",i,*(disturbance_vector+i));
	}

//	When the minimum HF value is > slice size, we need to adjust the index
	index_offset = hf->min / slice;
//	If a smooth radius or an artifact removal radius is given, 
//	we must decompose the process in 2 passes:
   if ( apply_postprocess && (smooth_radius || artifact_radius) ) {
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		// 	First, we approximate quantization
		index = (*(hf->tmp_buf + i) / slice) - index_offset;
		// 	"index*slice_size" gives the 1st value to test
		// 	Since displacement is strictly less than the slice_size,
		// 	the looked for value can only be index or index-1
		if (*(disturbance_vector+index)>*(hf->tmp_buf+i))
			index = MAX(0,index-1);  // This MAX is not supposed to be required!!
		*(hf->hf_buf + i) = *(disturbance_vector+index);
	}
	// 	The 2 next processes operate on hf->hf_buf
	if (artifact_radius)
		hf_remove_artifacts(hf, artifact_radius, dist_matrix);
	if (smooth_radius) 
		hf_smooth(hf, smooth_radius, dist_matrix, if_wrap, gauss_list);
	// 	Swap result_buf and hf_buf, so that result_buf becomes the input
	// 	and hf_buf is prepared for a new output
	swap_buffers ((gpointer*) &hf->result_buf, (gpointer*) &hf->hf_buf);
  }	// 	end of "if (apply_postprocess)"
  else  {	// 	if ( ! apply_postprocess)

	for (i=0; i<hf->max_x*hf->max_y; i++) {
		index =  (*(hf->tmp_buf + i) / slice) - index_offset;
		if (*(disturbance_vector+index)>*(hf->tmp_buf+i))
			index = MAX(0,index-1);  
		*(hf->result_buf + i) = *(disturbance_vector+index);
	}	
  }	
  if (disturbance_vector)
	x_free(disturbance_vector);
//	write_png ("tmp_buf.png", 16, (unsigned char *) hf->tmp_buf, hf->max_x, hf->max_y);
//	write_png ("result_buf.png", 16, (unsigned char *) hf->result_buf, hf->max_x, hf->max_y);
//	write_png ("hf_buf.png", 16, (unsigned char *) hf->hf_buf, hf->max_x, hf->max_y);

}

void hf_threshold (hf_struct_type *hf, hf_type min, hf_type max, gint percent) {
	gint i;
	if (!hf->tmp_buf)
		hf_backup(hf);
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		*(hf->hf_buf + i) = (hf_type) ((long int) *(hf->tmp_buf +i)* (100-percent) / 100) +
			( ( (long int) ( MAX(min,MIN(max,*(hf->tmp_buf + i))) ) * percent) / 100);
	}
}

gdouble *create_gaussian_kernel (gint radius, gdouble base, gdouble exponent, gdouble relative_value_offset, gdouble scale, gboolean not_inverted) {
	// Create a gaussian kernel, suitable for hf_convolve
	// To get a "sharpen" or "noisify" kernel, use a negative
	// relative_value_offset and set not_inverted to FALSE
	gdouble *kernel, min, max, value_offset, distance, somme;
	gint x, y, size;
	size = 1 + 2*radius;
	if (scale<=0.0)
		scale=1.0;
	kernel = (gdouble *) x_malloc(sizeof(gdouble)*size*size, "gdouble (kernel in create_gaussian_kernel)");
	max = BELLD(base, exponent, (gdouble) 0.0, (gdouble) size*scale);
	min = BELLD(base, exponent, (gdouble) DIST2(0, 0, radius, radius), (gdouble) size*scale);
//	printf("Distance: %f\n",(gdouble) DIST2(0, 0, radius, radius));
	value_offset = relative_value_offset * (max-min);
//	printf("Radius: %d; Min: %f; Max: %f; Offset: %f\n",radius, min, max, value_offset);
	somme = 0.0;
	for (y=0; y<size; y++) {
		for (x=0; x<size; x++) {
			distance = DIST2(x,y,radius, radius);
			if (not_inverted)
				*(kernel + y*size + x) = value_offset + BELLD(base, exponent, distance, scale*size) - min;
			else
				*(kernel + y*size + x) = value_offset + max - BELLD(base, exponent, distance, scale*size);
//			printf(" %7.4f ",*(kernel + y*size + x) );
			somme += *(kernel + y*size + x);
//			if (x==(size-1))
//				printf("\n");
		}
	}
//	printf("Somme: %7.4f\n", somme);
	return kernel;
}

gdouble *create_hat_kernel (gint radius, gdouble relative_value_offset, gdouble scale, gboolean not_inverted) {
	gdouble *kernel, value_offset, somme, edges_value, sum_edges, center_value;
	gint x, y, size;
	size = 1 + 2*radius;
	if (scale<=0.0)
		scale=1.0;
	kernel = (gdouble *) x_malloc(sizeof(gdouble)*size*size, "gdouble (kernel in create_hat_kernel)");
	edges_value = 1.0;
	sum_edges = (4.0*(size-2.0) + 4.0) * edges_value;
	if (not_inverted)
		center_value = scale;
	else
		center_value = -(sum_edges*scale - 1.0) /(gdouble)((size-2) * (size-2));
	value_offset = relative_value_offset;
	printf("Radius: %d; Sum_edgest: %f; Center_value: %f; Offset: %f\n",radius, sum_edges, center_value, value_offset);
	somme = 0.0;
	for (y=0; y<size; y++) {
		for (x=0; x<size; x++) {
			if ((y==0) || (x==0) || (x==(size-1)) || (y==(size-1)))
				*(kernel + y*size + x) = edges_value;
			else
				*(kernel + y*size + x) = center_value;
			printf(" %7.4f ",*(kernel + y*size + x) );
//			printf("(%d,%d) = %7.4f ",x,y,*(kernel + y*size + x) );
			somme += *(kernel + y*size + x);
			if (x==(size-1))
				printf("\n");
		}
	}
	printf("Somme: %7.4f\n", somme);
	return kernel;
}

gdouble *create_pixel_kernel (gint radius, gdouble relative_value_offset, gdouble scale, gboolean not_inverted) {
	gdouble *kernel, value_offset, somme, edges_value, sum_edges, center_value;
	gint x, y, size;
	size = 1 + 2*radius;
	if (scale<=0.0)
		scale=1.0;
	kernel = (gdouble *) x_malloc(sizeof(gdouble)*size*size, "gdouble (kernel in create_pixel_kernel)");
	edges_value = scale * 1.0;
	sum_edges = ((gdouble) (size*size)-1.0) * edges_value;
	if (not_inverted)
		center_value = scale;
	else
		center_value = -sum_edges + 1.0;
	value_offset = relative_value_offset;
	printf("Radius: %d; Sum_edgest: %f; Center_value: %f; Offset: %f\n",radius, sum_edges, center_value, value_offset);
	somme = 0.0;
	for (y=0; y<size; y++) {
		for (x=0; x<size; x++) {
			if ((y==x) && (x==radius))
				*(kernel + y*size + x) = center_value;
			else
				*(kernel + y*size + x) = edges_value;
			printf(" %7.4f ",*(kernel + y*size + x) );
//			printf("(%d,%d) = %7.4f ",x,y,*(kernel + y*size + x) );
			somme += *(kernel + y*size + x);
			if (x==(size-1))
				printf("\n");
		}
	}
	printf("Somme: %7.4f\n", somme);
	return kernel;
}

gdouble *create_tent_kernel (gint radius, gdouble value_offset, gdouble scale, gboolean not_inverted) {
	gdouble *kernel, somme, dradius, di = 0.0;
	gint i, j, x, y, size;
	size = 1 + 2*radius;
	if (scale<=0.0)
		scale=1.0;
	kernel = (gdouble *) x_malloc(sizeof(gdouble)*size*size, "gdouble (kernel in create_tent_kernel)");
	printf("Radius: %d; Offset: %f\n",radius, value_offset);
	somme = 0.0;
	dradius = (gdouble) radius;
	for (i=0; i<radius; i++) {
		for (j=i; j<(size-i); j++) {
			di = (gdouble) i;
			if (not_inverted) {
				*(kernel + i*size + j) = scale * (di+value_offset);
				*(kernel + (size-i-1)*size + j) = scale * (di+value_offset);
			}
			else {
				*(kernel + i*size + j) = scale * (value_offset-di);
				*(kernel + (size-i-1)*size + j) = scale * (value_offset-di);
			}
		}
		for (j=(i+1); j<(size-i-1); j++) {
			if (not_inverted) {
				*(kernel + j*size + i) = scale * (di+value_offset);
				*(kernel + j*size + size - i - 1) = scale * (di+value_offset);
			}
			else {
				*(kernel + j*size + i) = scale * (value_offset-di);
				*(kernel + j*size + size - i - 1) = scale * (value_offset-di);
			}
		}
	}
	if (not_inverted)
		*(kernel + radius*size + radius) = scale * (dradius + value_offset);
	else
		*(kernel + radius*size + radius) = scale * (value_offset - dradius);
		
	for (y=0; y<size; y++) {
		for (x=0; x<size; x++) {
			printf(" %7.4f ",*(kernel + y*size + x) );
//			printf("(%d,%d) = %7.4f ",x,y,*(kernel + y*size + x) );
			somme += *(kernel + y*size + x);
			if (x==(size-1))
				printf("\n");
		}
	}
	printf("Somme: %7.4f\n", somme);
	return kernel;
}

void hf_convolve (hf_struct_type *hf, gint radius, gdouble *matrix, gboolean if_tiles) {
//	Apply a convolution filter, normalizing the sum of the weights to 1
//	No normalization takes place when the weights sum is 0 or 1
//	matrix size = (2*radius+1)*(2*radius+1)
	gint i,j,k,l, ii, x0,y0,width;
	gdouble value, weight=0.0, maxd = (gdouble) MAX_HF_VALUE;
	hf_type *hf_buf;
	gboolean if_weight = TRUE;
	if ( (!radius) || (!matrix))
		return;
	width = 2*radius + 1;
	hf_buf = (hf_type *) x_malloc(sizeof(hf_type)*hf->max_x*hf->max_y, "hf_type (hf_buf in hf_convolve)");
	for (i=0;i<width*width;i++) {
		weight += *(matrix+i);
	}
	if ((weight==0.0) || (weight==1.0))
		if_weight = FALSE;
// printf("W: %s... %f\n",if_weight?"TRUE":"FALSE",weight);
	for (i=0; i<hf->max_y; i++) {
		ii = i*hf->max_x;
		for (j=0; j<hf->max_x; j++) {
			value = 0.0;
			y0 = i - radius;
			x0 = j - radius;
		//	Add the values contained in the (2*radius)+1 *... square centered at current dot
		  	 for (k = 0; k < width; k++)
				for (l = 0; l < width; l++) {
					if ((*matrix+k*width+l)>0.0) {
						if (if_tiles)
			 			   value = value + *(matrix+k*width+l) * 
					 	     (gdouble) *(hf->hf_buf + 
						     WRAP2(y0+k,hf->max_y)*(hf->max_y) +
						     WRAP2(x0+l,hf->max_x)) ;
						else
			 			   value = value + *(matrix+k*width+l) *
					 	      (gdouble) *(hf->hf_buf  +
						     MAX(0,MIN(y0+k,hf->max_y-1))*(hf->max_y) +
						     MAX(0,MIN(x0+l,hf->max_x-1)) )  ;
					}
				}
			if (if_weight)
				value = value / weight;
			*(hf_buf + ii+j) = (hf_type) MIN(maxd,MAX(0.0,value)) ;
		}
	}
	memcpy(hf->hf_buf,hf_buf,sizeof(hf_type)*hf->max_x*hf->max_y);
	x_free(hf_buf);
}

void convolve (gpointer buf, gint max_x, gint max_y, gint data_type, gint radius, gdouble *matrix, gboolean if_tiles) {
//	Apply a convolution filter, normalizing the sum of the weights to 1
//	No normalization takes place when the weights sum is 0 or 1
//	matrix size = (2*radius+1)*(2*radius+1)
	gint i,j,k,l, ii, x0,y0,width;
	gdouble value, weight=0.0, maxd = (gdouble) MAX_HF_VALUE;
	gdouble *dbuf, *dbufin;
	gboolean if_weight = TRUE;
	if ( (!radius) || (!matrix))
		return;
	width = 2*radius + 1;
	dbufin = (gdouble *) x_malloc(sizeof(gdouble)*max_x*max_y, "gdouble (dbufin in convolve)");
	dbuf = (gdouble *) x_malloc(sizeof(gdouble)*max_x*max_y, "gdouble (dbuf in convolve)");
	switch (data_type) {
		case GINT_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(dbufin+i) = (gdouble) *(((gint*) buf)+i);
			}
			break;
		case HF_TYPE_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(dbufin+i) = (gdouble) *(((hf_type*) buf)+i);
			}
			break;
		case GDOUBLE_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(dbufin+i) = *(((gdouble*) buf)+i);
			}
			break;
		case UNSIGNED_CHAR_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(dbufin+i) = (gdouble) *(((unsigned char*) buf)+i);
			}
			break;
		default:
			printf("Unexpected data type in convolve\n");
			exit(0);
	}
	for (i=0;i<width*width;i++) {
		weight += *(matrix+i);
	}
	if ((weight==0.0) || (weight==1.0))
		if_weight = FALSE;
// printf("W: %s... %f\n",if_weight?"TRUE":"FALSE",weight);
	for (i=0; i<max_y; i++) {
		ii = i*max_x;
		for (j=0; j<max_x; j++) {
			value = 0.0;
			y0 = i - radius;
			x0 = j - radius;
		//	Add the values contained in the (2*radius)+1 *... square centered at current dot
		  	 for (k = 0; k < width; k++)
				for (l = 0; l < width; l++) {
					if ((*matrix+k*width+l)>0.0) {
						if (if_tiles)
			 			   value = value + *(matrix+k*width+l) * 
					 	     (gdouble) *(dbufin + 
						     WRAP2(y0+k,max_y)*max_y +
						     WRAP2(x0+l,max_x)) ;
						else
			 			   value = value + *(matrix+k*width+l) *
					 	      (gdouble) *(dbufin  +
						     MAX(0,MIN(y0+k,max_y-1))*max_y +
						     MAX(0,MIN(x0+l,max_x-1)) )  ;
					}
				}
			if (if_weight)
				value = value / weight;
			*(dbuf + ii+j) = MIN(maxd,MAX(0.0,value)) ;
		}
	}
	switch (data_type) {
		case GINT_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(((gint*) buf)+i) = (gint) *(dbuf+i);
			}
			break;
		case HF_TYPE_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(((hf_type*) buf)+i) = (hf_type) *(dbuf+i);
			}
			break;
		case GDOUBLE_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(((gdouble*) buf)+i) = *(dbuf+i);
			}
			break;
		case UNSIGNED_CHAR_ID:
			for (i=0; i<(max_x*max_y); i++) {
				*(((unsigned char*) buf)+i) = (unsigned char) *(dbuf+i);
			}
			break;
		default:
			printf("Unexpected data type in convolve\n");
			exit(0);
	}
	x_free(dbuf);
	x_free(dbufin);
}

void hf_convolveb (hf_struct_type *hf, gint radius, gdouble *matrix, gboolean if_tiles) {
//	"Boolean" convolution filter
//	matrix size = (2*radius+1)*(2*radius+1)
//	If the current pixel is different from any neighbour, put it black
	gint i,j,k,l, ii, x0,y0,width;
	gboolean test;
	gdouble weight=0.0;
	hf_type *hf_buf, value;
	//gboolean if_weight = TRUE;
	width = 2*radius + 1;
	hf_buf = (hf_type *) x_malloc(sizeof(hf_type)*hf->max_x*hf->max_y, "hf_type (hf_buf in hf_convolveb)");
	for (i=0;i<width*width;i++) {
		weight += *(matrix+i);
	}
	/*if ((weight==0.0) || (weight==1.0))
		if_weight = FALSE;*/
// printf("W: %s... %f\n",if_weight?"TRUE":"FALSE",weight);
	for (i=0; i<hf->max_y; i++) {
		ii = i*hf->max_x;
		for (j=0; j<hf->max_x; j++) {
			value = *(hf->hf_buf + ii+j) ;
			test = TRUE;
			y0 = i - radius;
			x0 = j - radius;
		//	Add the values contained in the (2*radius)+1 *... square centered at current dot
		  	 for (k = 0; k < width; k++)
				for (l = 0; l < width; l++) {
					if ((*matrix+k*width+l)>0.0) {
						if (if_tiles)
			 			   test = test && (value == 
					 	     *(hf->hf_buf + 
						     WRAP2(y0+k,hf->max_y)*(hf->max_y) +
						     WRAP2(x0+l,hf->max_x)) ) ;
						else
			 			   test = test && (value == 
					 	      *(hf->hf_buf  +
						     MAX(0,MIN(y0+k,hf->max_y-1))*(hf->max_y) +
						     MAX(0,MIN(x0+l,hf->max_x-1)) )  );
					}
				}
			if (!test)
				*(hf_buf + ii+j) = 0 ;
			else
				*(hf_buf + ii+j) = *(hf->hf_buf+ii+j) ;
		}
	}
	memcpy(hf->hf_buf,hf_buf,sizeof(hf_type)*hf->max_x*hf->max_y);
	x_free(hf_buf);
}

//	static gdouble matrix[] = {0.0625,0.125,0.0625,0.125,0.25,0.125,0.0625,0.125,0.0625}; // Blur
//	"Sharpen" / add noise;  without clamping and with sum = 0:  B&W edge detection
//	static gdouble matrix[] = {-0.5,-0.5,-0.5,-0.5,4.0,-0.5,-0.5,-0.5,-0.5}; 
//	static gdouble matrix[] = {1,1,1,1,1,1,1,1,1}; // For min/max (erode/dilate)
//	static gdouble matrix[] = {-1,-1,-1,-1,8.0,-1,-1,-1,-1}; 

void erode_or_dilate (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gdouble *matrix, gint radius, gint operation, gboolean wrap) {

//	Simple erosion with a modified min/max convolution filter
//	matrix size = (2*radius+1)*(2*radius+1)
	gint i,j,k,l, ii, x0,y0,width;
	gdouble value,last,current, weight;
	width = 2*radius + 1;
	for (i=0; i<max_y; i++) {
		ii = i*max_x;
		for (j=0; j<max_x; j++) {
			current = (gdouble) *(hf_in + ii+j) ;
			value = current;
			y0 = i - radius;
			x0 = j - radius;
		//	Process the values contained in the (2*radius)+1 *... square centered at current dot
			for (k = 0; k < width; k++)
				for (l = 0; l < width; l++) {
					weight = *(matrix+k*width+l);
					if (weight>0.0) 
								{
		//	The minimum is normalized with matrix weight (typically distance)
						if (wrap)
							last = 	(gdouble) *(hf_in + 
							WRAP2(y0+k,max_y)*(max_y) +
							WRAP2(x0+l,max_x) );
						else
							last = 	(gdouble) *(hf_in + 
							MAX(0,MIN(y0+k,max_y-1))*(max_y) +
							MAX(0,MIN(x0+l,max_x-1)) );
						if (operation==DILATE) {
							if (last>current)
							value = (1.0-weight)*current+weight*last;
						}
						else { // Erode
							if (last<current)
							value = (1.0-weight)*current+weight*last;
						}
					}
				}
			*(hf_out + ii+j) = (hf_type) value ;
		} // j (x) loop
	} // i (y) loop
}

void dilation (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gboolean wrap) {
	// Morphological dilation,
	// done with max instead of boolean OR, for coping with non-binary values
	static gdouble matrix[] = {0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0};
	erode_or_dilate (hf_in, hf_out, max_x, max_y, matrix, 1, DILATE, wrap);
}

void erosion (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gboolean wrap) {
	// Morphological erosion,
	// done with min instead of boolean AND, for coping with non-binary values
	static gdouble matrix[] = {0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0};
	erode_or_dilate (hf_in, hf_out, max_x, max_y, matrix, 1, ERODE, wrap);
}

void min_max_erode (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gint steps, gboolean wrap) {

//	Simple erosion with a modified min/max convolution filter
//	matrix size = (2*radius+1)*(2*radius+1)
//	static gdouble matrix[] = {0.7,1,0.7,1,1,1,0.7,1,0.7};
//	static gdouble matrix[] = {0,1,0,1,1,1,0,1,0};
//	static gdouble matrix[] = {1,1,1,1,1,1,1,1,1};
//	gint i,j,k,l, ii, s, x0,y0;
//	gdouble value,last,current, weight;
	gint s; // width, radius = 1
	//width = 2*radius + 1;
	hf_type *out; //*in,
	//in = hf_in;
	out = hf_out;
	for (s=1; s<steps; s++) {
  		printf("STEP %d\n",s);
		if (s>1) 
			swap_buffers ((gpointer*) &hf_in, (gpointer*) &hf_out);
	  	erosion (hf_in, hf_out, max_x, max_y, wrap);
	}
	if (out==hf_in)
		memcpy(out,hf_out,max_x*max_y*sizeof(hf_type));
}

void min_max_spread (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gint steps, gboolean wrap) {

//	Simple erosion (dark areas spread) derivated from min_max_erode
//	matrix size = 2*2 of 1.0
//	Use for expanding dark areas (lines) 1 pixel at a time, towards east and south
	gint i,j, ii, s, x0,y0;
	gdouble value,last,current;
	hf_type *out; // *in,
	//in = hf_in;
	out = hf_out;
	for (s=1; s<steps; s++) {
	   if (s>1)
		swap_buffers ((gpointer*) &hf_in, (gpointer*) &hf_out);
  	   for (i=0; i<max_y; i++) {
		ii = i*max_x;
		for (j=0; j<max_x; j++) {
			current = (gdouble) *(hf_in + ii+j) ;
			value = current;
			if (wrap) {
				y0 = WRAP2(i - 1,max_y);
				x0 = WRAP2(j - 1,max_y);
			}
			else {
				y0 = MAX(0,i - 1);
				x0 = MAX(0,j - 1);
			}
			last = 	(gdouble) *(hf_in + VECTORIZE(x0,y0,max_x) );
			if (last<current)
				value = last;
			y0 = i;
			last = 	(gdouble) *(hf_in + VECTORIZE(x0,y0,max_x) );
			if (last<current)
				value = last;
			if (wrap) {
				y0 = WRAP2(i - 1,max_y);
			}
			else {
				y0 = MAX(0,i - 1);
			}
			x0 = j;
			last = 	(gdouble) *(hf_in + VECTORIZE(x0,y0,max_x) );
			if (last<current)
				value = last;
			*(hf_out + ii+j) = (hf_type) value ;
		} // j (x) loop
	   } // i (y) loop
	} // s (steps) loop
	if (out==hf_in)
		memcpy(out,hf_out,max_x*max_y*sizeof(hf_type));
}

void hf_erode (hf_struct_type *hf, gint steps) {
	hf_type *hf_in;
	gint totsize;
	totsize = sizeof(hf_type) * hf->max_x *hf->max_y;
	hf_in = (hf_type *) x_malloc(totsize, "hf_type (hf_in in hf_erode)");
	memcpy (hf_in,hf->hf_buf, totsize);
	if (hf->if_tiles)
		min_max_erode (hf_in, hf->hf_buf, hf->max_x, hf->max_y, steps, *hf->if_tiles);
	else
		min_max_erode (hf_in, hf->hf_buf, hf->max_x, hf->max_y, steps, TRUE);
	x_free(hf_in);
}

gpointer init_blocks (gpointer buf, gint max_x, gint max_y, gint radius, gint nblocks,
		gfloat phase, gboolean if_tiles, gint data_type) {
//	Average the pixels of hf into a smaller nblocks * nblocks hf_buf
//	Kind of "undersampling";  basis for processes as "smooth" and "cityscape"
	gint i, j, k, l, x, y, size;
	gpointer blocks;
	gdouble value;
	switch (data_type) {
		case GINT_ID:
			size = sizeof(gint);
			break;
		case HF_TYPE_ID:
			size = sizeof(hf_type);
			break;
		case GDOUBLE_ID:
			size = sizeof(gdouble);
			break;
		case UNSIGNED_CHAR_ID:
			size = sizeof(unsigned char);
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),
			"init_blocks");
			printf("\n");
			return 0;
	}
	blocks = (gpointer) x_malloc(nblocks * nblocks * size, "gpointer (blocks in init_blocks)");
	for (i=0; i<nblocks; i++) {
  	  for (j=0; j<nblocks; j++) {
		value = 0;
		for (k=((i+phase)*radius); k<((i+1+phase)*radius); k++)
		for (l=((j+phase)*radius); l<((j+1+phase)*radius); l++) {
			if (if_tiles) {
				x = (max_x+l)%max_x;
				y = (max_y+k)%max_y;
			}
			else {
				x = MIN(l, max_x-1);
				y = MIN(k, max_y-1);
			}
			switch (data_type) {
				case GINT_ID:
					value = value + (gdouble) *( ((gint *) buf) + y*max_x + x);
					break;
				case HF_TYPE_ID:
					value = value + (gdouble) *( ((hf_type *) buf) + y*max_x + x);
					break;
				case GDOUBLE_ID:
					value = value + *( ((gdouble *) buf) + y*max_x + x);
					break;
				case UNSIGNED_CHAR_ID:
					value = value + (gdouble) *( ((unsigned char *) buf) + y*max_x + x);
					break;
				default:
					printf(_("Unexpected option in %s; contact the programmer!"),
					"init_blocks");
					printf("\n");
					return 0;
			}
		}	// End k and l loops
		value = value / (gdouble) (radius * radius);
		switch (data_type) {
			case GINT_ID:
				*( ((gint *) blocks) + (i*nblocks)+j) = (gint) value;
				break;
			case HF_TYPE_ID:
				*( ((hf_type *) blocks) + (i*nblocks)+j) = (hf_type) value;
				break;
			case GDOUBLE_ID:
				*( ((gdouble *) blocks) + (i*nblocks)+j) = value;
				break;
			case UNSIGNED_CHAR_ID:
				*( ((unsigned char *) blocks) + (i*nblocks)+j) = (unsigned char) value;
				break;
			default:
				printf(_("Unexpected option in %s; contact the programmer!"),
				"init_blocks");
				printf("\n");
				return 0;
		}
	  }	// End j loop
	}	// End i loop
	return blocks;
}

void smooth_buf (gpointer buf, gint max_x, gint max_y, gint radius, dist_matrix_struct *dm, gboolean wrap, gdouble **gauss_list, gint data_type) {
//	Smoothing process, based on a sampling, followed by a rebuild 
//	based on a normal (gaussian) 3d function (used as "shape filters" elsewhere in Geomorph)
//	For radius <= 36, we use a "separated" convolution in the spacial domain
//
//	dist_matrix = the usual precompiled distances matrix, for managing gaussian filters
//	wrap = TRUE =>  we reuse pixel 0 after pixel max_x or max_y

	gint i, j, k, l, nblocks, box, index, indexf, offset, half_radius;
	gpointer blocks;
	filter_struct *filter;
	gdouble *hf_weight,*hf_values, value;
	gdouble *v;

	if (!radius)	// We are not supposed to be here!
		return;

	if (radius<=36) {
	
		if (!gauss_list[radius])
			gauss_list[radius] = normalized_bell_new(radius);
		v = gauss_list[radius];
		
		convolve_normalized_vector (buf,
			buf, 
			max_x, 
			max_y, 
			wrap, 
			radius, 
			v,
			data_type);
		}
	else {

//	We add repeatedly a gaussian bell, at each sampled pixel
//	2003-02-14:  finally there is only 1 level left, since a separable convolution is used
//	Finally the best rebuilding box radius is equal to the specified radius
	box = radius;
	filter = sharp_filter_new(box<<1,dm);	// The size must be even
	half_radius = radius>>1;

//	1- Sample
	nblocks = (gint) ((max_x-1+half_radius) / half_radius);
// printf("NBLOCKS: %d\n",nblocks);
	blocks = init_blocks(buf, max_x, max_y, half_radius, nblocks, 0, wrap, data_type);

	hf_values = (gdouble *) x_calloc(max_x*max_y, sizeof(gdouble), "gdouble (hf_values in smooth_buf)");
	hf_weight = (gdouble *) x_calloc(max_x*max_y, sizeof(gdouble), "gdouble (hf_weight in smooth_buf)");

//	2- Rebuild
	// The sampling / rebuild process introduces a translation, which we must correct
	offset = half_radius/2;
	for (i=0; i<nblocks; i++) {	
  	  for (j=0; j<nblocks; j++) {	
		
		for (k=0; k<filter->hf_size; k++)
			for (l=0; l<filter->hf_size; l++) {

				if (wrap)
				   index = WRAP2((i*half_radius+k-box+offset),max_y)*max_x
						 + WRAP2(j*half_radius +l-box+offset, max_x);
				else
				   index = MAX(0,MIN(i*half_radius+k-box+offset,max_y-1))*max_x
						 + MAX(0,MIN(j*half_radius +l-box+offset, max_x-1)) ;
				indexf = VECTORIZE(CENTER_XY(k,filter->hf_size),
						CENTER_XY(l,filter->hf_size),filter->hf_size>>1) ;
// printf("INDEX dans filter->value: %d\n", indexf );
				switch (data_type) {
					case GINT_ID:
						value = (gdouble) *(((gint *)blocks) + i*nblocks+j);
						break;
					case HF_TYPE_ID:
						value = (gdouble) *(((hf_type *)blocks) + i*nblocks+j);
						break;
					case GDOUBLE_ID:
						value = *(((gdouble *)blocks) + i*nblocks+j);
						break;
					case UNSIGNED_CHAR_ID:
						value = (gdouble) *(((unsigned char *) blocks) + i*nblocks+j);
						break;
					default:
						printf(_("Unexpected option in %s; contact the programmer!"),
						"smooth_buf");
						printf("\n");
						return;
				}
				*(hf_values + index) = *(hf_values + index) + 		value * *(filter->values + indexf) ;
				*(hf_weight+index) = *(hf_weight+index) +  *(filter->values+ indexf);
// printf("(i,j,k,l): (%d,%d,%d,%d); INDEX: %d; values: %f; w: %f\n",i,j,k,l,index,*(hf_values + index) ,*(hf_weight+index) );
			}
	  }
	} 

	for (i=0; i<max_x*max_y; i++) {

		// Modified 2006-01-27 for different data types
		switch (data_type) {
			case GINT_ID:
				*( ((gint *) buf) + i) = (gint) ( (*(hf_values+i))/(*(hf_weight+i)) )  ;
				break;
			case HF_TYPE_ID:
				*( ((hf_type *) buf) + i) = (hf_type) ( (*(hf_values+i))/(*(hf_weight+i)) )  ;
				break;
			case GDOUBLE_ID:
				*( ((gdouble *) buf) + i) = (gdouble) ( (*(hf_values+i))/(*(hf_weight+i)) )  ;
				break;
			case UNSIGNED_CHAR_ID:
				*( ((unsigned char *) buf) + i) = (unsigned char) ( (*(hf_values+i))/(*(hf_weight+i)) )  ;
				break;
			default:
				printf(_("Unexpected option in %s; contact the programmer!"),
				"smooth_buf");
				printf("\n");
		}
	}
	x_free(blocks);
	x_free(hf_values);
	x_free(hf_weight);
	filter_free(filter);
//	We reapply the process for smoothing the artifacts of the rebuilt HF
	smooth_buf(buf, max_x, max_y, 36, dm, wrap, gauss_list, data_type);
	}	// End else radius <= 36
}

void hf_smooth (hf_struct_type *hf, gint radius,
	dist_matrix_struct *dm, gboolean wrap,gdouble **gauss_list) {
	smooth_buf(hf->hf_buf, hf->max_x, hf->max_y, radius, dm, wrap, gauss_list, HF_TYPE_ID);
};
void hf_mosaic (hf_struct_type *hf, gint radius, gint nblocks, gfloat phase) {
	gint i,j;
	hf_type *blocks;
//	Build the mosaic
	blocks = (hf_type *) init_blocks(hf->tmp_buf, hf->max_x, hf->max_y, radius, nblocks, phase, FALSE, HF_TYPE_ID);
//	Transfer the mosaic into the hf buffer
	for (i=0; i<hf->max_y; i++)
		for (j=0; j<hf->max_x; j++)
			*(hf->hf_buf+WRAP(i+(gint) (radius*phase),hf->max_x)*hf->max_x+j+
				WRAP((gint) (radius*phase),hf->max_y)) = 
				*(blocks+((gint) (i/radius) )*nblocks + (gint) (j/radius));
}

void hf_honeycomb (hf_type *hf_in, hf_type *hf_out, gint max, gint radius, gint border) {
	// Quantize with an hexagonal structure
	// Distance between cells centers = 2*radius
	// "border" = with of the line separating each hexagon, in pixels
	// This transformation does not preserve tiling
	// Important:  radius must be even, otherwise we get black pixels (rounding problem)
	gint x,y,i,j, nbcellsh, nbcellsw, h, w, index, nrow, npix;
	gint baseh, basew, current_cell, block_h, xoffset, xstep;
	static gfloat tan60=1.73205, sin60=0.86602;
	unsigned long int sum; 
	hf_type *cell_values;
	// Hexagons are not symmetrical in a square world...
	// I chose to put the longer axis on the width
	// h = height of each cell
	// w = width
/*	 _    _ 
	/  \_/  \
	\_/  \_/
*/
	radius = 2* (gint) ceil((gdouble) radius/2.0);
	h = (gint) ((gfloat) radius * sin60);
	w = 3 * radius;
	nbcellsh = (gint) ceil ( ((gdouble) 2*radius + max) / (gdouble) h);
	nbcellsw = (gint) ceil (((gdouble) 2*radius + max) / (gdouble) w);
	cell_values = (hf_type *) x_calloc(1+nbcellsw*nbcellsh, sizeof(hf_type), "hf_type (cell_values in hf_honeycomb)");
	// cell_values contains the average value for the current cell
	// We use hf->hf_buf to store the index of the cell to which the related pixel belongs
	for (i=0; i<max*max; i++)
		*(hf_out+i) = 0;
	// Processing the neighbourhood of each cell...
	// We "center" the structure
	baseh = (max%h)/2;
//	basew = (max%radius)/2;
	basew = 0;
	current_cell = 1;
	// Cell 0 value is 0, so we can detect glitches...
	block_h = radius*sin60;
	nrow=0;
// printf("RADIUS: %d; CH: %d; CW: %d; WxH: %d; BASEH: %d; BASEW: %d; BLOCK_h: %d\n",radius, nbcellsh, nbcellsw, nbcellsh*nbcellsw, baseh, basew, block_h);
	for (y=baseh-h/2; y<max+h; y+=h) {
		xstep = 3*radius ;
		// The x offset is not the same depending if the row is even or odd
		if (nrow%2) {
			xoffset = basew-radius/2;  // This is why radius must be even
		}
		else {
			xoffset = basew+radius;
		}
// printf("ROW: %d; XOFFSET: %d; XSTEP: %d; CELL: %d\n",nrow,xoffset,xstep, current_cell);
		nrow++;
		for (x=xoffset; x<max+radius; x+=xstep) {
// printf("X: %d; ",x);
			sum = 0;
			npix = 0;
			for (i=-block_h+border; i<block_h-border; i++) {
				if ((y+i)<0)
					continue;
				if ((y+i)>=max)
					break;
				for (j=-radius+border; j<radius-border; j++) {
					index = x + j;
					if (index<0)
						continue;
					if (index>=max)
						break;
					index = VECTORIZE(index,y+i,max);
				//	Test: is it inside the hexagon?
				//	If abs(x) (x is j) <=cos60*radius (<= radius/2), it's certainly OK
				//	otherwise, we must check if abs(i) / (radius-abs(x)) < tan60
					if ( (ABS(j)<= (radius-border)/2) ||
						(( ((gfloat) ABS(i)) / (gfloat) (radius-border-ABS(j)) ) < tan60 ))
					{
						*(hf_out+index) = current_cell;
						npix++;
						sum = sum + *(hf_in+index);
					}
				}
			}
// printf("Current_CELL: %d; NPIX: %d; SUM: %d\n",current_cell,npix,sum);
			if (npix) // Some cells are outside the image, for some radius
				*(cell_values+current_cell) = sum / npix;
			current_cell++;	
		}
	}
	for (i=0; i<max*max; i++)
		if ((nbcellsh*nbcellsw)<*(hf_out+i)) // "Debugging" test - it's not supposed to happen...
			*(hf_out+i) = 0xFFFF;
		else
			*(hf_out+i) = *(cell_values+*(hf_out+i));
	x_free(cell_values);
}

void hf_hexagon (hf_struct_type *hf, gint radius, gint border,
	gint smooth_radius, 
	dist_matrix_struct *dist_matrix,
	gboolean apply_postprocess,
	gdouble **gauss_list) {

	if (radius<4)
		return;
	
	if (!hf->tmp_buf)
		hf_backup(hf);
	// We need a second temporary buffer for storing
	// the result of the quantization process before the postprocessing
	if (!hf->result_buf)
		hf->result_buf = (hf_type *) x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (result_buf in hf_hexagon)");

	hf_honeycomb (hf->tmp_buf, hf->result_buf, hf->max_x, radius, border);

   	if ( apply_postprocess && smooth_radius  ) {
		// Smooth works on hf_buf, so we must swap it with result_buf
		swap_buffers ((gpointer*) &hf->result_buf, (gpointer*) &hf->hf_buf);
		if (smooth_radius) 
			hf_smooth(hf, smooth_radius, dist_matrix, FALSE, gauss_list);
		swap_buffers ((gpointer*) &hf->result_buf, (gpointer*) &hf->hf_buf);
  	}	// 	end of "if (apply_postprocess)"

}


void hf_city(hf_struct_type *hf, gint radius, gint skyscraper_var, gint streets_width) {
//	radius:  skyscraper width, in pixels
//	skyscaper_var:  upper limit of random width variation, in % of radius
//	streets_width:  width of black lines between blocks simulating streets, in % of radius
	gint i,j,k,l,nblocks,str_w, var;
	hf_type *blocks;
	gint *rand_y, *rand_x;
	
	if (!radius)
		return;
		
	if (!hf->tmp_buf)
		hf_backup(hf);
	if (!hf->result_buf)
		hf->result_buf = (hf_type *) x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (result_buf in hf_city)");
	
	nblocks = (gint) ((hf->max_x-1+radius) / radius);
	//	Sample the height of skyscapers
	blocks = (hf_type *) init_blocks(hf->tmp_buf, hf->max_x, hf->max_y, radius, nblocks, 0, FALSE, HF_TYPE_ID);
	//	Transfer the "mosaic" into the hf buffer
	//	Apply random width variation, bounded by skyscraper_var %, and draw streets
	str_w = (streets_width*radius)/200;	// Divide by 200:  a street extends one half on each block
	rand_x = (gint *) x_malloc((nblocks+2)*sizeof(gint), "gint (rand_x in hf_city)");	
	rand_y = (gint *) x_malloc((nblocks+2)*sizeof(gint), "gint (rand_y in hf_city)");
	var = (skyscraper_var*radius)/100;
	*(rand_x) = 0;
	*(rand_y) = 0;
	if (var) {
		for (i=1; i<(nblocks+1); i++) {
			*(rand_y+i) = ((i-1)*radius) + ((rand()%var) - (var/2));
			*(rand_x+i) = ((i-1)*radius) + ((rand()%var) - (var/2));
	// printf("RAND_X(%d): %d;  RAND_Y(%d): %d\n",i, *(rand_x+i),i,*(rand_y+i));
		}
	}
	else
		for (i=1; i<(nblocks+1); i++) {
			*(rand_y+i) = (i-1)*radius;
			*(rand_x+i) = (i-1)*radius;
		}
	*(rand_y+nblocks+1) = hf->max_y;
	*(rand_x+nblocks+1) = hf->max_x;
	for (i=0; i<(nblocks+1); i++) {
		for (j=0; j<(nblocks+1); j++) {
			for (k= *(rand_y+i); (k<*(rand_y+i+1)) ; k++)
				for (l= *(rand_x+j); (l<*(rand_x+j+1)) ; l++)
					if ((k<(*(rand_y+i) + str_w)) || (k>*(rand_y+i+1)-str_w)   || 
						(l<(*(rand_x+j) + str_w)) || (l>*(rand_x+j+1)-str_w) )
						*(hf->result_buf+WRAP(k,hf->max_y)*hf->max_x+
							WRAP(l,hf->max_x)) = 0;
					else
						*(hf->result_buf+WRAP(k,hf->max_y)*hf->max_x+
						WRAP(l,hf->max_x)) = 
						*(blocks+WRAP(i-1,nblocks)*nblocks + WRAP(j-1,nblocks)) ;
		}
	}
	if (rand_y)
		x_free(rand_y);
	if (rand_x)
		x_free(rand_x);
}

void hf_stretch_v (hf_struct_type *hf) {
//	Stretch vertically (compress horizontally)
//	We simply divide the scale by 2 horizontally
	gint x,y;
	if (!hf->tmp_buf)
		hf_backup(hf);
	for (y=0; y<hf->max_y; y++) {
		for (x=0; x<hf->max_x; x++) {
			*(hf->hf_buf + VECTORIZE(x,y,hf->max_x)) =
				*(hf->tmp_buf + VECTORIZE(WRAP(x<<1,hf->max_x),y,hf->max_x)); 
		}
	}
}
void hf_stretch_h (hf_struct_type *hf) {
//	Stretch horizontally (compress vertically)
//	We simply divide the scale by 2 vertically
	gint x,y;
	if (!hf->tmp_buf)
		hf_backup(hf);
	for (y=0; y<hf->max_y; y++) {
		for (x=0; x<hf->max_x; x++) {
			*(hf->hf_buf + VECTORIZE(x,y,hf->max_x)) =
				*(hf->tmp_buf + VECTORIZE(x,WRAP((y<<1),hf->max_y),hf->max_x)); 
		}
	}
}

void hf_math_fn (hf_struct_type *hf, gint math_op, gdouble parameter) {
//	Some mathematical pixel transformations
	gint i;
	static gdouble PI2 = 6.283185307, maxd = (gdouble) 0xFFFF;
	gdouble *doublebuf, ratiolog;
	doublebuf = xalloc_double_hf(hf->max_x, "gdouble (doublebuf in hf_math_fn)");
	ratiolog = (10.0-parameter) * log1p(maxd) / maxd;
// printf("MAXD:  %f; LOG(MAX): %f;  RATIOLOG: %f\n",maxd, log1p(maxd), ratiolog);
	if (!hf->tmp_buf)
		hf_backup(hf);
//	printf("MATH_OP: %d; PARAM: %f\n",math_op, parameter);

	// Patch 2010-11-21 to avoid black result
	if (parameter==1.0)
		parameter += 0.001;

	for (i=0; i<hf->max_x*hf->max_y; i++) {

		switch (math_op) {
		case POWER_OP:
			*(doublebuf +i) = pow( (gdouble) *(hf->tmp_buf+i), parameter);
			break;
		case BASE_OP:
			*(doublebuf +i) = pow( parameter, ((gdouble) *(hf->tmp_buf+i)) / maxd );
			break;
		case LOG_OP:
			*(doublebuf+i) = ratiolog * *(hf->tmp_buf+i) + parameter * log1p((gdouble)*(hf->tmp_buf+i));
			break;
		case SINE_OP:
			*(doublebuf+i) = sin( parameter * PI2 *((gdouble) *(hf->tmp_buf+i)) / maxd );
		default:
			*(doublebuf+i) = (gdouble) *(hf->tmp_buf+i);
		}
//		if (!((i+(hf->max_x>>1))%hf->max_x))
//			printf("@i= %d; tmp_buf: %d; doublebuf: %f; \n", i,*(hf->tmp_buf+i),*(doublebuf+i) );	
	}
	hf_min_max (hf);
	hf_double_clamp (hf, hf->min, hf->max, doublebuf);
	x_free(doublebuf);
}

void hf_black_n_white (hf_type *buf, gint length, hf_type pivot) {
	// Stretch the contrast of buf to 0x0000 and 0xFFFF,
	// using pivot as the value under which all is black
	// and over which all is white
	gint i;
	for (i=0; i<length; i++) {
		if (*(buf+i)<=pivot)
			*(buf+i) = 0;
		else
			*(buf+i) = 0xFFFF;
	}
}

void merge_lift_edges (hf_struct_type *hf, gint level) {
	gint i;
	hf_type *hf1,*hf2,*out;
	gdouble ratio, glevel;
	glong abs_level = ((LIFT_EDGES_MAX>>1) * level) / 100;
	glevel = ((gdouble) level) / 100.0;
	hf1 = hf->tmp_buf;
	hf2 = hf->result_buf;
	out = hf->hf_buf;
	if ((hf->min>abs_level) || ((MAX_HF_VALUE-hf->max)>abs_level)) {
		ratio = ((gdouble) (hf->max - hf->min)) / ((gdouble) (hf->max - hf->min + 2*abs_level));
		for (i=0; i<(hf->max_x*hf->max_y); i++)
			*(out+i) = (hf_type) (ratio * (gdouble) ((glong) *(hf1+i))  + (glong) (glevel * (gdouble) *(hf2+i)) );
	}
	else
		for (i=0; i<(hf->max_x*hf->max_y); i++)
			*(out+i) = (hf_type) ((glong) *(hf1+i)) + (glong) (glevel * (gdouble) *(hf2+i));
}

void lift_edges (hf_type *in, hf_type *out, gint max_x, gint max_y, gint radius,
	dist_matrix_struct *dm, gboolean wrap,gdouble **gauss_list, gboolean use_black_point) {
	// Writes in *out the differences to add to *in to lift the edges
	// of areas with black borders
	//
	// 1. Threshold the grid
	// (all pixels under the "black point" are black, others are white)
	// 2006-12-25: threshold only if use_black_point is TRUE
	// TRUE: better for surfaces with dark lines (e.g. hexagons)
	// FALSE: better for other quantized surfaces (e.g. terraces)
	// 2. Smooth
	// 3. Subtract the smooth result from the lower contrast source
	//

	hf_type *tmp, min, max;
	gint i;
	tmp = (hf_type *) x_malloc(sizeof(hf_type)*max_x*max_y, "hf_type (tmp in lift_edges)");
	if ((!radius) || (!in) || (!out))
		return;
	min = max = *in;
	for (i=1; i<(max_x*max_y); i++)
		if (min>*(in+i))
			min = *(in+i);
		else
			if (max<*(in+i))
				max = *(in+i);
	if (use_black_point)
		hf_black_n_white (in, max_x*max_y, (max-min)>>3);
	memcpy(tmp,in,sizeof(hf_type)*max_x*max_y);
	smooth_buf (in, max_x, max_y, radius, dm, wrap, gauss_list, HF_TYPE_ID);
	hf_subtract (tmp, in, out, max_x*max_y, OVERFLOW_ZERO);
	hf_clamp_buffer (out, max_x * max_y, 0, LIFT_EDGES_MAX);
	x_free(tmp);
}

hf_type *build_noise_map (gdouble wave_length_in_percent, gint max_x, gint max_y, dist_matrix_struct *dm, gdouble **gauss_list) {
	hf_type *output;
	gint nbpix, i;
	output = (hf_type *) x_calloc(sizeof(hf_type), max_x*max_y, "hf_type (output in build_noise_map)");
	nbpix = (gint) (100.0 / wave_length_in_percent);
	nbpix *= nbpix;
	nbpix *= 4;
	for (i=0; i<nbpix; i++) {
		*(output + (rand()%max_x) + (rand()%max_y)*max_x) = MAX_HF_VALUE;
	}
	smooth_buf (output, max_x, max_y, (gint) ((wave_length_in_percent / 100.0) * (gdouble) max_x), dm, TRUE, gauss_list, HF_TYPE_ID);
	hf_clamp_buffer(output, max_x*max_y, 0, MAX_HF_VALUE);
//	smooth_hf_buf (output, max_x, max_y, (gint) ((wave_length_in_percent / 100.0) * (gdouble) max_x), dm, TRUE, gauss_list);
	return output;
}

void dfind_maximum (gdouble *in, hf_type *output, gint max_x, gint max_y, hf_type foreground_value, hf_type background_value) {
	// Find the maxima of the "in" map
	// Fill the maxima with a foreground value, 
	// fill the other pixels with a background value
	// in the "output" result
	// "output" is already allocated
	
	// If both foreground and background values are 0, we use 0 as the background
	// and the "in" value as the foreground
	
	gdouble current;
	gint x,y,xx,yy,xxx,yyy;
	gboolean test;
	
	test = (!foreground_value) && !background_value;
	
	for (y=0; y<max_y; y++)
		for (x=0; x<max_x; x++) {
			
			// We test the 4 axis to determine if the current pixel is
			// a local maximum
			
			current = *(in + x + y*max_x);
			
			if (test)
				foreground_value = (hf_type) current;
			
			xx = WRAP2(x-1,max_x);
			yy = WRAP2(y-1,max_y);
			xxx = WRAP2(x+1,max_x);
			yyy = WRAP2(y+1,max_y);
			
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
			
			xx = x;
			yy = WRAP2(y-1,max_y);
			xxx = x;
			yyy = WRAP2(y+1,max_y);
			
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
						
			xx = WRAP2(x+1,max_x);
			yy = WRAP2(y-1,max_y);
			xxx = WRAP2(x-1,max_x);
			yyy = WRAP2(y+1,max_y);
						
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
									
			xx = WRAP2(x+1,max_x);
			yy = y;
			xxx = WRAP2(x-1,max_x);
			yyy = y;
									
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
			
			*(output+x+y*max_x) = background_value;
	}
}

void find_maximum (hf_type *in, hf_type *output, gint max_x, gint max_y, hf_type foreground_value, hf_type background_value) {
	// Find the maxima of the "in" map
	// Fill the maxima with a foreground value, the other pixels a background value
	// in the "output" result
	// "output" is already allocated
		
	// If both foreground and background values are 0, we use 0 as the background
	// and the "in" value as the foreground
	
	hf_type current;
	gint x,y,xx,yy,xxx,yyy;
	gboolean test;
	
	test = (!foreground_value) && !background_value;
	
	for (y=0; y<max_y; y++)
		for (x=0; x<max_x; x++) {
			
			// We test the 4 axis to determine if the current pixel is
			// a local maximum
			
			current = *(in + x + y*max_x);
					
			if (test)
				foreground_value = (hf_type) current;
			
			xx = WRAP2(x-1,max_x);
			yy = WRAP2(y-1,max_y);
			xxx = WRAP2(x+1,max_x);
			yyy = WRAP2(y+1,max_y);
			
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
			
			xx = x;
			yy = WRAP2(y-1,max_y);
			xxx = x;
			yyy = WRAP2(y+1,max_y);
			
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
						
			xx = WRAP2(x+1,max_x);
			yy = WRAP2(y-1,max_y);
			xxx = WRAP2(x-1,max_x);
			yyy = WRAP2(y+1,max_y);
						
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
									
			xx = WRAP2(x+1,max_x);
			yy = y;
			xxx = WRAP2(x-1,max_x);
			yyy = y;
									
			if ((current>*(in+xxx+yyy*max_x)) &&
				(current>*(in+xx+yy*max_x))) {
				*(output+x+y*max_x) = foreground_value;
				continue;
			}
			
			*(output+x+y*max_x) = background_value;
	}
}

void spread_over_max (hf_type *input, hf_type *output, gint max_x, gint max_y, gboolean wrap, gint min_width, gint max_width, hf_type min_val, hf_type max_val, gint function) {
	// Widen the areas in input which are not null
	// "Spread" them depending on the pixel value
	// Typically: used for thickening lines with a variable width
	// We scale the values from 1 to max_width, depending on min_val and max_val
	// If both min_val and max_val are 0, or if max_val < min_val, 
	// we recalculate them from *input
	
	gint i, x, y, index_x, index_y, index_i;
	gdouble ratio;
	gint value;
	hf_type *tmp;

	if (((!max_val) && (!min_val)) || (max_val<min_val)) {
		// Note: min_val is the lowest value greater than 0
		min_val = MAX_HF_VALUE;
		max_val = 0;
		for (i=0; i<(max_y*max_x); i++) { 
			if (!*(input+i))
				continue;
			if (*(input+i)<min_val)
				min_val = *(input+i);
			if (*(input+i)>max_val)
				max_val = *(input+i);
		}
	}
	if (max_val==min_val) {
		printf("Nothing to spread!\n");
		return;
	}
	
	ratio = ((gdouble) max_width) / (gdouble) max_val ;
	
//	printf("MIN_VAL: %d; MAX_VAL: %d; MIN_WIDTH: %d; MAX_WIDTH: %d;  RATIO: %7.3f\n",min_val, max_val, min_width, max_width, ratio);
	
	tmp = (hf_type *) x_calloc(max_x*max_y,sizeof(hf_type), "hf_type (tmp in spread_over_max)");
	
//	write_png ("output0.png", 16, input, max_x, max_y);
	
//	write_png ("input.png", 16, input, max_x, max_y);	
	// This is a separable algorithm, with a half-kernel of 'radius' width
	// When wrap == FALSE, we simply skip the outbounded indices
	// 1st pass
	for (y=0; y<max_y; y++) {
		for (x=0; x<max_x; x++) {
			for (i=x; i<(x+max_width); i++) {
				if (!wrap) {
					if ((i<0) || (i>=max_x))
						continue;
				}
				index_x = VECTORIZE(x,y,max_x);
				index_i = VECTORIZE(WRAP2(i,max_x),y,max_x);
				// When ABS(i-x) <= normalize(*input+index_i), we output normalize(*input+index_i) in output+index_x, if it's >
				value = (hf_type) (0.5 + ratio * ((gdouble) *(input+index_i)));
				if (value)
					value = MAX(value,min_width);
				else
					continue;
				if (ABS(i-x)<=value) {
					value = value - ABS(i-x); // TENT_FUNC
					if (value>*(tmp+index_x))
						*(tmp+index_x) = value;
				}
			}
		}
	}
	
//	write_png ("output1.png", 16, tmp, max_x, max_y);	

	for (x=0; x<max_x; x++) {
		for (y=0; y<max_y; y++) {
			for (i=y; i<(y+max_width); i++) {
				if (!wrap) {
					if ((i<0) || (i>=max_y))
						continue;
				}
				index_y = VECTORIZE(x,y,max_x);
				index_i = VECTORIZE(x,WRAP2(i,max_y),max_x);
				value = *(tmp+index_i);
				if (!value)
					continue;
				if (ABS(i-y)<=value) {
					value = value - ABS(i-y); // TENT_FUNC
					if (value>*(output+index_y))
						*(output+index_y) = value;
				}
			}
		}
	}

//	write_png ("output2.png", 16, output, max_x, max_y);
//	memcpy(output,tmp,sizeof(hf_type)*max_x*max_y);
	x_free(tmp);
}

void improve_edges (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gboolean wrap, hf_type threshold) {

	// Improve edges (or lines) by smoothing against a threshold value
	// Works only with dark edges
	//
	// Pixel numbering:
	//           4 | 3 | 2
	//           5 | 0 | 1
	//           6 | 7 | 8
	//
	// Edges type == {DARK_EDGES, BRIGHT_EDGES}
	//
	gint i,j, ii, index; // endy, endx,startx, starty,  
	hf_type v0, v1, v2, v3, v4, v5, v6, v7, v8;

	/*if (wrap) {
		startx = starty = 0;
		endx = max_x;
		endy = max_y;
	}
	else {
		startx = starty = 1;
		endx = max_x - 1;
		endy = max_y - 1;
	}*/
	
	// We test each axis (N-S, E-W, diagonal) to replace v0 so that it is coherent with its neighbours
	
	// 2006-04-04 2-pass version - the first one works best when input==output
	
	// This pass fills "holes", for instance (B = background, F = foreground):
	// 	B F B
	//	F B B
	// becomes:
	// 	B F B
	//	F F B
	for (i=0; i<max_y; i++) {
		for (j=0; j<max_x; j++) {
			ii = i*max_x;
			index = ii + j;
			v0 = *(hf_in + index);
			v1 = *(hf_in + ii + WRAP2(j+1,max_x));
			v5 = *(hf_in + ii + WRAP2(j-1,max_x));
			ii = WRAP2(i-1,max_y)*max_x;
			v3 = *(hf_in + ii + j);
			v2 = *(hf_in + ii + WRAP2(j+1,max_x));
			v4 = *(hf_in + ii + WRAP2(j-1,max_x));
			ii = WRAP2(i+1,max_y)*max_x;
			v7 = *(hf_in + ii + j);
			v8 = *(hf_in + ii + WRAP2(j+1,max_x));
			v6 = *(hf_in + ii + WRAP2(j-1,max_x));
			if (v0>threshold) {
				if (((v3<=threshold) && (v1<=threshold) && (v2>threshold)) ) {
					*(hf_in+index) = hf_type_avrg(v1,v3);
					continue;
				}
				if ((v1<=threshold) && (v7<=threshold) && (v8>threshold)){
					*(hf_in+index) = hf_type_avrg(v7,v1);
					continue;
				}
				if ((v3<=threshold) && (v5<=threshold) && (v4>threshold)) {
					*(hf_in+index) = hf_type_avrg(v3,v5);
					continue;
				}
				if ((v5<=threshold) && (v7<=threshold) && (v6>threshold)){
					*(hf_in+index) = hf_type_avrg(v5,v7);
					continue;
				}

			}			
		} // j (x) loop
	} // i (y) loop
	
	// This pass tests that kind of patterns (h or v):
	//     2
	//   1 1 1
	// Here we remove "2" - being the background	
	
	for (i=0; i<max_y; i++) {
		for (j=0; j<max_x; j++) {
			ii = i*max_x;
			index = ii + j;
			v0 = *(hf_in + index);
			v1 = *(hf_in + ii + WRAP2(j+1,max_x));
			v5 = *(hf_in + ii + WRAP2(j-1,max_x));
			ii = WRAP2(i-1,max_y)*max_x;
			v3 = *(hf_in + ii + j);
			v2 = *(hf_in + ii + WRAP2(j+1,max_x));
			v4 = *(hf_in + ii + WRAP2(j-1,max_x));
			ii = WRAP2(i+1,max_y)*max_x;
			v7 = *(hf_in + ii + j);
			v8 = *(hf_in + ii + WRAP2(j+1,max_x));
			v6 = *(hf_in + ii + WRAP2(j-1,max_x));
			*(hf_out+index) = v0;
			if (v0<=threshold) {
				if ((v3>threshold) && (v7>threshold)) {
					if (((v2>threshold) && (v1>threshold) && (v8>threshold)) ||((v4>threshold) && (v5>threshold) && (v6>threshold)) ) {
						*(hf_out+index) = hf_type_avrg(v3,v7);
						continue;
					}
				}
				if ((v5>threshold) && (v1>threshold))  {
					if (((v2>threshold) && (v3>threshold) && (v4>threshold)) ||((v6>threshold) && (v7>threshold) && (v8>threshold)) ) {
						*(hf_out+index) = hf_type_avrg(v5,v1);
					}
					continue;
				}
			} // Threshold test
		} // j (x) loop
	} // i (y) loop
	
	// memcpy(hf_out,hf_in,max_x*max_y*sizeof(hf_type));
}

void hf_integrate (hf_struct_type *hf, gint angle) {
	// Integrates an image from an arbitrary angle, in degrees
	// Used basically for "simple view modelling"
	// - Building a HF from a terrain picture with lateral lighting
	// 1. Recalculate min, max, average
	// 2. Oversize the HF (for rotating it without loosing the edges)
	// 3. Convert the HF to floating point
	// 4. Rotate the HF
	// 5. Integrates
	// 6. Rotate back the HF
	// 7. Crop back the HF
	// 8. Convert back the HF to hf_type
	
	gdouble *buffer_in, *buffer_out, a, dmin, dmax, ratio; // avrg, 
	gint i, j, max, offset, index;
	
	if (!hf->tmp_buf)
		hf_backup(hf);
	hf_min_max (hf);
	//avrg = (gdouble) hf->avrg;
	
	max = (((gint) (1.7*hf->max_x))>>1)<<1; // Must be even
	offset = max>>2;
	buffer_in = xcalloc_double_hf(max, "gdouble (buffer_in in hf_integrate)");
	buffer_out = xcalloc_double_hf(max, "gdouble (buffer_out in hf_integrate)");
	
	for (i=0; i<hf->max_x; i++)
		for (j=0; j<hf->max_y; j++)
			// The content is centered
			// The average is adjusted to 0
			*(buffer_in+VECTORIZE(i+offset,j+offset,max)) = (gdouble) *(hf->tmp_buf+VECTORIZE(i,j,hf->max_x));
	
	a = PI * ((gdouble) angle) / 180.0;
	rotate (cos(a), sin(a), (gpointer) buffer_in, (gpointer) buffer_out, max, max, GDOUBLE_ID, OVERFLOW_ZERO);
	
	/*****************************************/
	// Integration
	// We adjust the resulting values to 0-MAX_HF_VALUE
	
	dmin = 0.0;
	dmax = 0.0;
	
	for (i=0; i<max; i++) {
		if (*(buffer_out+i)<dmin)
			dmin = *(buffer_out+i);
		if (*(buffer_out+i)>dmax)
			dmax = *(buffer_out+i);
		for (j=1; j<max; j++) {
			index = VECTORIZE(i,j,max);
			*(buffer_out + index) += *(buffer_out+VECTORIZE(i,j-1,max)) / 1.05;
			
			if (*(buffer_out+index)<dmin)
				dmin = *(buffer_out+index);
			if (*(buffer_out+index)>dmax)
				dmax = *(buffer_out+index);
		}
	}
				
	
	/*****************************************/
	
	a = -PI * ((gdouble) angle) / 180.0;
	rotate (cos(a), sin(a), (gpointer) buffer_out, (gpointer) buffer_in, max, max, GDOUBLE_ID, OVERFLOW_ZERO);
	
	ratio = ((gdouble) MAX_HF_VALUE ) / (dmax - dmin);
	
	for (i=0; i<hf->max_x; i++)
		for (j=0; j<hf->max_y; j++)
			// The content is translated back to (0,0) (cropped)
			// and scaled back to 0-MAX_HF_VALUE
			*(hf->hf_buf+VECTORIZE(i,j,hf->max_x)) = (hf_type)  (ratio * ( *(buffer_in+VECTORIZE(i+offset,j+offset,max)) - dmin ));
	
	x_free(buffer_in);
	x_free(buffer_out);
}

void hf_cut_graph (hf_struct_type *hf, gint index, gint axis) {

	// Draw a graph of line / column "index" on axis "axis"
	
	guint i,j, shift;
	hf_type value;
	
	if (!hf->tmp_buf)
		hf_backup(hf);
		
	index = MAX(0,MIN(hf->max_x,index));
	
	shift = 1 + log2i(MAX_HF_VALUE) - log2i(hf->max_x);
	
	for (i=0; i<hf->max_x; i++) {
		if (axis==V_AXIS) // index is an X value
			value = *(hf->tmp_buf+VECTORIZE(index,i,hf->max_x));
		else // index is a Y value
			value = *(hf->tmp_buf+VECTORIZE(i,index,hf->max_x));
		if (shift)
			value = value>>shift;
		for (j=0; j<hf->max_x; j++) // Y output
			if ((hf->max_y-value)>j)
				*(hf->hf_buf+VECTORIZE(i,j,hf->max_x)) = 0;
			else
				*(hf->hf_buf+VECTORIZE(i,j,hf->max_x)) = MAX_HF_VALUE>>1;
	}

}

void hf_histogram (hf_struct_type *hf) {

	// Draw a values histogram in hf->hf_buf
	
	guint i,j, shift;
	
	hf_type value;
	gulong *vector, max;
	
	vector = (gulong *) x_calloc(sizeof(gulong),hf->max_x, "gulong (vector in hf_histogram)");
	
	if (!hf->tmp_buf)
		hf_backup(hf);
	
	shift = 1 + log2i(MAX_HF_VALUE) - log2i(hf->max_x);
	
	for (i=0; i<(hf->max_x*hf->max_y); i++)
		*(vector+((*(hf->tmp_buf+i))>>shift)) += 1;
	
	max = 0;
	
	for (i=0; i<hf->max_x; i++)
		if (*(vector+i)>max)
			max = *(vector+i);
	
	if (max>hf->max_y)
		shift = 1 + log2i(max) - log2i(hf->max_x);
	
//	printf("MAX: %d; SHIFT: %d\n",max, shift);
	
	for (i=0; i<hf->max_x; i++) {
		value = *(vector+i);
		if (shift)
			value = value>>shift;
		for (j=0; j<hf->max_y; j++) // Y output
			if ((hf->max_y-value)>j)
				*(hf->hf_buf+VECTORIZE(i,j,hf->max_x)) = 0;
			else
				*(hf->hf_buf+VECTORIZE(i,j,hf->max_x)) = MAX_HF_VALUE>>1;
	}
	x_free(vector);
}

gboolean histogram (hf_struct_type *hf, guint width, guint height, guchar *output) {
	
	// Builds a histogram in output
	
	gint i,j, k, value;
	gdouble ratio;	
	gulong *vector, max;
	
	if (!output)
		return FALSE;
		
	vector = (gulong *) x_calloc(sizeof(gulong),width, "gulong (vector in histogram)");
	
	ratio = ((gdouble) MAX_HF_VALUE) / (gdouble) width;
	
	for (i=0; i<(hf->max_x*hf->max_y); i++) {
		k = (gint) (((gdouble)*(hf->hf_buf+i))/ratio);
		if (k>=width) {
			printf("WARNING! Vector index in histogram out of boundary: %d >= %d\n",k, width);
			continue;
		}
		*(vector+k) += 1;
	}
	
	// Smooth the vector and calculate the max number of pixels
/*	
	last = *vector;
	for (i=0; i<width; i++) {
		llast = *(vector+i);
		*(vector+i) += last;
		last = llast;
	}
*/
	max = 0;
	
	for (i=0; i<width; i++)
		if (*(vector+i)>max)
			max = *(vector+i);
	
	ratio = ((gdouble) max) / (gdouble) height;
	
	for (i=0; i<width; i++) {
		value = (guint) (((gdouble) *(vector+i)) / ratio);
		value = height - value;
		for (j=(height-1); j>value; j--) { // Y output
			k = VECTORIZE(i,j,width);
			if ((k<0) || (k>=(width*height))) {
				printf("WARNING! Output index in histogram out of boundary: %i >= %d\n",k, width*height); 
				continue;
			}
			*(output+k) = 127;
		}
	}
	
	x_free(vector);
	return TRUE;
}
