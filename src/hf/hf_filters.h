/* hf_filters.h - headers for managing filters
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

#ifndef _FILTERS
#define _FILTERS 1

#include "hf.h"
#include "hf_calc.h"

//	Filter merge operations

//	MULTIPLY: transform the multiplier as a ratio between 0.0 and 1.0
#ifndef MULTIPLY
	#define MULTIPLY 1
#endif

//	MULTIPLY2: transform the multiplier as a ratio between 1.0 and 2.0
#ifndef MULTIPLY2
	#define MULTIPLY2 2
#endif

#ifndef ADD
	#define ADD 3
#endif
#ifndef SUBTRACT
	#define SUBTRACT 4
#endif

//	Standard options for gaussian filter
typedef struct {
	gfloat base;	// From 1 to 20 (1st best guess!)
	gfloat exp;	// From 0 to 10
	gfloat scale;	// From 0 to 100 %
} gauss_opt;

typedef struct {
//	A filter struct
	gint hf_size;	// HF size;  allows fast check of the presence of a big enough filter struct
	gint size;	// Vector size = square of the quadrant size
			// Example:  256 for a 16 pixels wide square for a 32 pixels wide HF
	gfloat *values;	// == NULL when the filter is an image
			// These values are read using dist_matrix_struct->distances as indices
			// ranges:  from 0 to 1
	gauss_opt *opt;
//	2 next variables can be seen as img_opt * (may be embedded in the future?)
	hf_struct_type *hf;	// When the filter is an image;  otherwise == NULL;
	hf_struct_type *hf_scaled;	// Scaled version of the preceding structure
			// Could be a scaled copy of *this->hf->hf_buf
			// or == this->hf->hf_buf, when no scaling is necessary
			// Before re-scaling this->hf, we free this->hf_scaled
			// only when this->if_scaled is TRUE
			// this->hf_size is the size of this->scaled
	gboolean if_scaled;
} filter_struct;

filter_struct *wide_filter_new(gint hf_size, dist_matrix_struct *);
filter_struct *medium_filter1_new(gint hf_size, dist_matrix_struct *);
filter_struct *medium_filter2_new(gint hf_size, dist_matrix_struct *);
filter_struct *sharp_filter_new(gint hf_size, dist_matrix_struct *);
filter_struct *image_filter_new();
filter_struct *gauss_filter_new(gint hf_size, dist_matrix_struct *dist, gauss_opt *opt);

void gauss_filter_init(filter_struct *, dist_matrix_struct *);
void gauss_filter_reinit (filter_struct *filter, gint hf_size, dist_matrix_struct *dist);

gauss_opt *gauss_opt_new(gfloat base, gfloat exp, gfloat scale);

void filter_free(filter_struct *);
void gauss_opt_free (gauss_opt *);

void filter_apply(hf_struct_type *, filter_struct *, dist_matrix_struct *dist,
		gboolean revert, gfloat level, gint merge_oper);
void filter_apply_uns_char(unsigned char *, filter_struct *, gint hf_size,
		dist_matrix_struct *dist,
		gboolean revert, gfloat level, gint merge_oper);

#endif





