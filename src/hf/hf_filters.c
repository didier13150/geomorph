/* hf_filters.c - HF filters management
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "hf_filters.h"

#include <math.h>

void gauss_filter_init (filter_struct *filter, dist_matrix_struct *dist) {
	gint i, j;
	gfloat norm_value;
	gauss_opt *opt;
	opt = filter->opt;
//	We normalize the filter by subtracting a value so that the edges dots == 0
	norm_value = BELL(opt->base, opt->exp, filter->hf_size>>1, filter->hf_size*opt->scale);
// printf("NORM_VALUE: %f\n",norm_value);
	for (i=0; i<(filter->hf_size>>1); i++) {
		for (j=0; j<(filter->hf_size>>1); j++) {
			(*(filter->values+VECTORIZE(i,j,filter->hf_size>>1))) =
				MAX(0.0,BELL(opt->base, opt->exp,
					*(dist->distances+VECTORIZE(i,j,dist->hf_size>>1)),
					filter->hf_size*opt->scale) - norm_value);
		}
	}
}

void gauss_filter_reinit (filter_struct *filter, gint hf_size, dist_matrix_struct *dist) {
//	Reinit a gauss - type filter after a size change
//		or at the fist execution, for the predefined filters
	filter->hf_size = hf_size;
	filter->size = (hf_size>>1) * (hf_size>>1);
	if (filter->values)
		x_free(filter->values);
	filter->values = (gfloat*) x_malloc(sizeof(gfloat)*filter->size, "gfloat (filter->values - gauss reinit))");
	if (hf_size>dist->hf_size)
		dist_matrix_init(dist, hf_size);
	gauss_filter_init (filter, dist);
}

gauss_opt *gauss_opt_new(gfloat base, gfloat exp, gfloat scale) {
	gauss_opt *gopt;
	gopt = (gauss_opt*) x_malloc(sizeof(gauss_opt), "gauss_opt");
	gopt->base = base;
	gopt->exp = exp;
	gopt->scale = scale;
	return gopt;
}

filter_struct *wide_filter_new(gint hf_size, dist_matrix_struct *dist) {
	filter_struct *filter;
	filter = (filter_struct *) x_malloc(sizeof(filter_struct), "filter_struct (wide)");
	filter->values = NULL;
	filter->opt = gauss_opt_new(CONST_E, 5, 0.6);
	gauss_filter_reinit(filter, hf_size, dist);
	filter->hf = NULL;
	filter->hf_scaled = NULL;
	return filter;
}

filter_struct *medium_filter1_new(gint hf_size, dist_matrix_struct *dist) {
	filter_struct *filter;
	filter = (filter_struct *) x_malloc(sizeof(filter_struct), "filter_struct (medium1)");
	filter->values = NULL;
	filter->opt = gauss_opt_new(CONST_E, 3, 0.5);
	gauss_filter_reinit(filter, hf_size, dist);
	filter->hf = NULL;
	filter->hf_scaled = NULL;
	return filter;
}

filter_struct *medium_filter2_new(gint hf_size, dist_matrix_struct *dist) {
	filter_struct *filter;
	filter = (filter_struct *) x_malloc(sizeof(filter_struct), "filter_struct (medium2)");
	filter->values = NULL;
	filter->opt = gauss_opt_new(CONST_E, 2, 0.45);
	gauss_filter_reinit(filter, hf_size, dist);
	filter->hf = NULL;
	filter->hf_scaled = NULL;
	return filter;
}

filter_struct *sharp_filter_new(gint hf_size, dist_matrix_struct *dist) {
	filter_struct *filter;
	filter = (filter_struct *) x_malloc(sizeof(filter_struct), "filter_struct (sharp)");
	filter->values = NULL;
	filter->opt = gauss_opt_new(CONST_E, 1.3, 0.4);
	gauss_filter_reinit(filter, hf_size, dist);
	filter->hf = NULL;
	filter->hf_scaled = NULL;
	return filter;
}

filter_struct *image_filter_new() {
	filter_struct *filter;
	filter = (filter_struct *) x_malloc(sizeof(filter_struct), "filter_struct (image)");
	filter->size = 0;
	filter->hf_size = 0;
	filter->values = NULL;
	filter->opt = NULL;
	filter->hf = NULL;
	filter->hf_scaled = NULL;
	filter->if_scaled = FALSE;
	return filter;
}

filter_struct *gauss_filter_new(gint hf_size, dist_matrix_struct *dist, gauss_opt *opt) {
//	Doesn't calculate the filter, since this one is always recalculated
//	in the calling function
//	2003-01:  check... this program doesn't seem to be executed!
	filter_struct *filter;
	filter = (filter_struct *) x_malloc(sizeof(filter_struct), "filter_struct (gauss)");
	filter->hf_size = hf_size;
	filter->size = (hf_size>>1) * (hf_size>>1);
	filter->values = (gfloat*) x_malloc(sizeof(gfloat)*filter->size, "gfloat (filter->values - gauss)");
	filter->opt = opt;
	if (hf_size>dist->hf_size)
		dist_matrix_init(dist, hf_size);
	filter->hf = NULL;
	filter->hf_scaled = NULL;
	return filter;
}

void gauss_opt_free (gauss_opt *go) {
	if (go)
		x_free(go);
}

void filter_free(filter_struct *filter) {
	if (filter) {
		if (filter->values)
			x_free(filter->values);
		if (filter->opt)
			gauss_opt_free (filter->opt);
		x_free(filter);
	}
}

void image_filter_apply(hf_struct_type *hf, filter_struct *image_filter, 
		gboolean revert, gfloat level, gint merge_oper) {
//	Applying an image as a filter
//	1st check the HF size to merge against the main HF size
//		Upscale or downscale or do nothing
//	2nd apply the merge function
	gint dif;
	if (image_filter->hf->max_x != hf->max_x) {
		dif = log2i(hf->max_x) - log2i(image_filter->hf->max_x);
// printf("DIF: %d!\n",dif);
		if (image_filter->if_scaled)
			if (image_filter->hf_scaled)
				hf_free(image_filter->hf_scaled);
		image_filter->hf_scaled = hf_scale(image_filter->hf, dif);
		image_filter->if_scaled = TRUE;	
	}
	else {
		image_filter->hf_scaled = image_filter->hf;
		image_filter->if_scaled = FALSE;
	}

	hf_merge(image_filter->hf_scaled, hf,
		hf->max_x>>1,
		hf->max_y>>1,
		merge_oper,
		TRUE,
		level/100.0,
		TRUE);
}

void filter_apply(hf_struct_type *hf_str, filter_struct *filter, dist_matrix_struct *dist,
		gboolean revert, gfloat level, gint merge_oper) {
	//	"hf" is the hf data
	//	"dist" is a distance matrix, avoiding repeatedly calculating square roots
	//	"revert" - if TRUE, we apply 1-filter value
	//	"level" - from 0 to 100%
	//		we mix the original image with the filtered image in this ratio
	//		ex.. filter at 30% = 30% filtered values + 70% non filtered values
	//	The "dist" matrix could be null or too small for the current hf_size,
	//	so that we could have to expand it
	//	"filter" shouldn't be NULL but initialized by the calling app
	//	Here we don't know how to calculate the filter!
	//	The filter size should be reevaluated by the calling app, for the current HF_SIZE
	gint i,j, ratio, hfs, hf_size;
	hf_type value;
	hf_type *hf;
	double dvalue;
	if (!filter)
		return;
	hf = hf_str->hf_buf;
	hf_size = hf_str->max_x;
//	If the filter is an image, the algorithm is not the same - we use hf_merge
	if (!filter->values) {
		if (filter->hf)
			image_filter_apply(hf_str,filter, revert, level, merge_oper);
		return;
	}	
//	At any moment, hf_size for which the distance matrix is computed should be >= 
//		than the hf_size for which the current filter is computed, which should be >=
//		than the hf_size
	if (filter->hf_size < hf_size)
		gauss_filter_reinit (filter, hf_size, dist);

	//	1st we find the actual distance from HF center with the CENTER_XY macro
	//	... any distance for a HF smaller than the size with which we created
	//	the distances matrix can be found in the matrix
	//	2nd we find the filter value
	//	... here we must multiply indices for smaller HF, so that the bell shape
	//	is kept whatever the HF size is
	ratio = log2i(filter->hf_size / hf_size);
	hfs = hf_size<<ratio;
	for (i=0; i<hf_size; i++)
		for (j=0; j<hf_size; j++) {
			if (merge_oper == ADD) {
				if (revert)  { 
					dvalue = ((*(hf+VECTORIZE(i,j,hf_size))) 
						-0xEFFF *level/100.0);
					dvalue = dvalue +
					(level/100.0) * 0xEFFF * (1.0 - *(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1)));
				}
				else  {
					dvalue = ((*(hf+VECTORIZE(i,j,hf_size))) 
						-0xEFFF * level/100.0);
					dvalue = dvalue +
					(level/100.0) * 0xEFFF * *(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1));
				}
				value = (hf_type) MAX(0,MIN(dvalue, 0xFFFF));
			}
			else {	// Defaults to MULTIPLY
				if (revert) 
					value =
					(*(hf+VECTORIZE(i,j,hf_size)))
					 * (1-*(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1)));
				else
					value = 
					(*(hf+VECTORIZE(i,j,hf_size)))
					 * *(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1));
				if (level>98.0)
					*(hf+VECTORIZE(i,j,hf_size)) = value;
				else
					value = (level/100.0)*value +
						(1.0-level/100.0)* *(hf+VECTORIZE(i,j,hf_size));
			}
			*(hf+VECTORIZE(i,j,hf_size)) = value;

		}
}

void filter_apply_uns_char(unsigned char *hf8, filter_struct *filter, gint hf_size, dist_matrix_struct *dist, 
		gboolean revert, gfloat level, gint merge_oper) {
	//	Same as filter_apply, but for the 8 bits HF displaying buffer
	//	This allows apply the filter without modifying the real HF struct (hf_type *)
	gint i,j, ratio, hfs;
	unsigned char value;
	double dvalue;
	if (!filter)
		return;
	if (filter->hf_size < hf_size)
		gauss_filter_reinit (filter, hf_size, dist);
	ratio = log2i(filter->hf_size / hf_size);
	hfs = hf_size<<ratio;
// printf("****\nhf_size: %d;  filter->hf_size: %d;  hfs: %d;  Ratio: %d\n****\n",hf_size, filter->hf_size,hfs,ratio);  
	for (i=0; i<hf_size; i++)
		for (j=0; j<hf_size; j++) {
			if (merge_oper == ADD) {
				if (revert)  { 
					dvalue = ((*(hf8+VECTORIZE(i,j,hf_size))) 
						-0xEF *level/100);
					dvalue = dvalue +
					(level/100) * 0xEF * (1-*(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1)));
				}
				else  {
					dvalue = ((*(hf8+VECTORIZE(i,j,hf_size))) 
						-0xEF * level/100);
					dvalue = dvalue +
					(level/100) * 0xEF * *(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1));
				}
				value = (unsigned char) MAX(0,MIN(dvalue, 0xFF));
			}
			else {	// Defaults to MULTIPLY
				if (revert) 
					value =
					(*(hf8+VECTORIZE(i,j,hf_size)))
					 * (1-*(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1)));
				else
					value = 
					(*(hf8+VECTORIZE(i,j,hf_size)))
					 * *(filter->values+
					VECTORIZE(CENTER_XY(i<<ratio,hfs), 
					CENTER_XY(j<<ratio,hfs),filter->hf_size>>1));
				if (level>98)
					*(hf8+VECTORIZE(i,j,hf_size)) = value;
				else
					value = (level/100)*value +
						(1-level/100)* *(hf8+VECTORIZE(i,j,hf_size));
			}
			*(hf8+VECTORIZE(i,j,hf_size)) = value;

		}
}


