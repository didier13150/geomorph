/* hf_calc.c - 	hf transformations utilities
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

#include <math.h>
#include "hf.h"
#include "hf_calc.h"
#include "hf_filters.h"
#include "fill.h"

void pow2_scale_grid (gpointer in, gpointer out, gint max_x, gint max_y, gint log_ratio, gint data_type_in, gint data_type_out) {

//	Scale input_grid in output_grid, given log_ratio
//	log_ratio = -1 : reduce by 50%
//	log_ratio = -2 : reduce by 75%
//	log_ratio = 1 : increase size by 200%
//	log_ratio = 2 : increase size by 400%...
//	output_grid should be allocated
//	max_x, max_y: size of the input grid
//	m_x, m_y: deduced size of the output grid
	gint i, j, indx, m_x, m_y, m_i;
	m_x = LEFT_SHIFT(max_x,log_ratio);
	m_y = LEFT_SHIFT(max_y,log_ratio);
// printf("MX: %d; MY: %d\n",m_x,m_y);
	for (i = 0; i < m_x; i++)
		for (j = 0; j < m_y; j++) {

		indx = RIGHT_SHIFT(i,log_ratio) +
				RIGHT_SHIFT(j,log_ratio)*max_x;
		m_i = j*m_y + i;

		switch (data_type_in) {
		case GDOUBLE_ID:

			switch (data_type_out) {
			case GDOUBLE_ID:
				*(((gdouble *) out) +m_i) = *(((gdouble *) in)+indx);
				break;
			case HF_TYPE_ID:
				*(((hf_type *) out) +m_i) = (hf_type) *(((gdouble *) in)+indx);
				break;
			case GINT_ID:
				*(((gint *) out) +m_i) = (gint) *(((gdouble *) in)+indx);
				break;
			default: // UNSIGNED_CHAR_ID:
				*(((unsigned char *) out) +m_i) = (unsigned char) *(((gdouble *) in)+indx);
			} // End switch (data_type_out)
			break;

		case HF_TYPE_ID:

			switch (data_type_out) {
			case GDOUBLE_ID:
				*(((gdouble *) out) +m_i) = (gdouble) *(((hf_type *) in)+indx);
				break;
			case HF_TYPE_ID:
				*(((hf_type *) out) +m_i) = *(((hf_type *) in)+indx);
				break;
			case GINT_ID:
				*(((gint *) out) +m_i) = (gint) *(((hf_type *) in)+indx);
				break;
			default: // UNSIGNED_CHAR_ID:
				*(((unsigned char *) out) +m_i) = *(((hf_type *) in)+indx) >> 8;

//			if ((i==127) && !(j%64))
//				printf("Value in: %d; Value out: %d\n",*(((hf_type *) in)+indx),*(((unsigned char *) out) +m_i));

			} // End switch (data_type_out)
			break;

		case GINT_ID:

			switch (data_type_out) {
			case GDOUBLE_ID:
				*(((gdouble *) out) +m_i) = (gdouble) *(((gint *) in)+indx);
				break;
			case HF_TYPE_ID:
				*(((hf_type *) out) +m_i) = (hf_type) *(((gint *) in)+indx);
				break;
			case GINT_ID:
				*(((gint *) out) +m_i) = *(((gint *) in)+indx);
				break;
			default: // UNSIGNED_CHAR_ID:
				*(((unsigned char *) out) +m_i) = (unsigned char) *(((gint *) in)+indx) >> 8;
			} // End switch (data_type_out)

			break;
		default: // UNSIGNED_CHAR_ID:

			switch (data_type_out) {
			case GDOUBLE_ID:
				*(((gdouble *) out) +m_i) = (gdouble) *(((unsigned char *) in)+indx);
				break;
			case HF_TYPE_ID:
				*(((hf_type *) out) +m_i) = (hf_type) *(((unsigned char *) in)+indx);
				break;
			case GINT_ID:
				*(((gint *) out) +m_i) = (gint) *(((unsigned char *) in)+indx);
				break;
			default: // UNSIGNED_CHAR_ID:
				*(((unsigned char *) out) +m_i) = *(((unsigned char *) in)+indx);
			} // End switch (data_type_out)

		}  // End switch (data_type_in)
		}

}

void pow2_scale_uchar_grid_with_mask (gpointer in, unsigned char *out, gint max_x, gint max_y, gint log_ratio, gint data_type_in, gdouble *mask, gpointer mask_value_ptr) {

//	Scale input_grid in output_grid, given log_ratio
//	Use a double mask, translated as a a RGB value, given mask_value_ptr
//	Range of the mask: cut to (0.0 - 1.0)
//	Input: any
//	Output: (pix8_rgb *) == 3 * (unsigned char *)
//	
//	log_ratio = -1 : reduce by 50%
//	log_ratio = -2 : reduce by 75%
//	log_ratio = 1 : increase size by 200%
//	log_ratio = 2 : increase size by 400%...
//	output_grid should be allocated
//	max_x, max_y: size of the input grid
//	m_x, m_y: deduced size of the output grid
	gint i, j, indx, m_x, m_y, m_i;
	gdouble mask_level;
	pix8_rgb *mask_value;
	unsigned char value;
	mask_value = (pix8_rgb *) mask_value_ptr;
	m_x = LEFT_SHIFT(max_x,log_ratio);
	m_y = LEFT_SHIFT(max_y,log_ratio);
// printf("MX: %d; MY: %d\n",m_x,m_y);
	for (i = 0; i < m_x; i++)
		for (j = 0; j < m_y; j++) {

			indx = RIGHT_SHIFT(i,log_ratio) +
				RIGHT_SHIFT(j,log_ratio)*max_x;
			m_i = j*m_y + i;

			switch (data_type_in) {
			case GDOUBLE_ID:
				value = (unsigned char) *(((gdouble *) in)+indx);
				break;
			case HF_TYPE_ID:
				value = *(((hf_type *) in)+indx) >> 8;
				break;
			case GINT_ID:
				value = *(((gint *) in)+indx);
				break;
			default: // UNSIGNED_CHAR_ID:
				value = *(((unsigned char *) in)+indx);
			}
			if (mask) {
				// When mask > 1.0, we revert the colour
				if (1.0 < *(mask+indx)) {
					mask_level = 1.0 - MIN(MAX(*(mask+indx)-1.0, 0.0),1.0);
				// Red
				*(out + m_i * 3) = (unsigned char) ((mask_level * (gdouble) value) + (1.0-mask_level) * (gdouble) ((~mask_value->r) & value));
				// Green
				*(out + 1 + m_i * 3) =  (unsigned char) ((mask_level * (gdouble) value) + (1.0-mask_level) * (gdouble) ((~mask_value->g) & value));
				// Blue
				*(out + 2 + m_i * 3) = (unsigned char) ((mask_level * (gdouble) value) + (1.0-mask_level) * (gdouble) ((~mask_value->b) & value));
				}
				else {
					mask_level = MIN(MAX(*(mask+indx),0.0),1.0);
				// Red
				*(out + m_i * 3) = (unsigned char) ((mask_level * (gdouble) value) + (1.0-mask_level) * (gdouble) (mask_value->r & value));
				// Green
				*(out + 1 + m_i * 3) =  (unsigned char) ((mask_level * (gdouble) value) + (1.0-mask_level) * (gdouble) (mask_value->g & value));
				// Blue
				*(out + 2 + m_i * 3) = (unsigned char) ((mask_level * (gdouble) value) + (1.0-mask_level) * (gdouble) (mask_value->b & value));
				}
			}
			else {
				*(out + m_i * 3) = mask_value->r & value;
				*(out + 1 + m_i * 3) = mask_value->g & value;
				*(out + 2 + m_i * 3) = mask_value->b & value;
			}
		}
}

hf_struct_type * hf_scale (hf_struct_type *hf, gint log_ratio) {
//	Scale a HF...
//	log_ratio = -1 : reduce by 50%
//	log_ratio = -2 : reduce by 75%
//	log_ratio = 1 : increase size by 200%
//	log_ratio = 2 : increase size by 400%...
	hf_struct_type *out;
	gint i, j, max_x, max_y;
	max_x = LEFT_SHIFT(hf->max_x,log_ratio);
	max_y = LEFT_SHIFT(hf->max_y,log_ratio);
	out = hf_new(max_x);
	for (i = 0; i < max_x; i++)
		for (j = 0; j < max_y; j++) {
			*(out->hf_buf + j*max_y + i) =
				*(hf->hf_buf + (RIGHT_SHIFT(i,log_ratio)) +
				(RIGHT_SHIFT(j,log_ratio))*hf->max_x) ;
			}
	return out;
}

void hf_merge_buf (hf_type *hf_pencil, gint pen_max_x, gint pen_max_y,
		hf_type *hf, gint max_x, gint max_y,
		int x, int y,
		gint pen_merge,
		gboolean pen_tiles,
		gfloat h_displacement,
		gboolean normalize) {

// Writes hf_pencil (or any other HF) in hf centered at (x,y)
// For instance, when drawing, (x,y) is the mouse position

// h_displacement is the relative level of the added HF, from 0.0 to 1.0
// normalize = TRUE applies to pen_merge = ADD
// ... reduce the added HFs by half, to avoid "burning"

	gint i,j, k, l, displ;	// displ = displacement (we center the HF on the mouse position)
	glong val;
	hf_type max_value = 1; // Should be > 0, even if HF is black
	if (pen_merge == MULTIPLY) {
		for (i=0; i<(max_x*max_y); i++)
			if (max_value<*(hf+i))
				max_value = *(hf+i);
	}
	displ = pen_max_x >> 1; // Pencil is square!
	x = x - displ;
	y = y - displ;
	for (i = 0; i < pen_max_x ; i++) {
		// Calculate X index in hf world
		k = x + i;
		if (!pen_tiles) {
			if (k>=max_x)
				break;
			else
				if (k<0)
					continue;
		}
		if (k < 0)
			k = max_x + k;
		else
			k = k%max_x;

		for (j = 0; j < pen_max_y; j++) {
			// Calculate Y index in hf world
			l = y + j;
			if (!pen_tiles) {
				if (l>=max_y)
					break;
				else
					if (l<0)
						continue;
			}
			if (l < 0)
				l = max_y + l;
			else
				l = l%max_y;
			if (pen_merge == SUBTRACT)
				val = (*(hf + k + l*max_x)) - h_displacement *
					(*(hf_pencil + i + j * pen_max_x));
			else 	if (pen_merge == MULTIPLY) {
					val = (glong) (((gfloat)*(hf + k + l*max_x)) *
						((gfloat) *(hf_pencil + i + j * pen_max_x)) / max_value );
					val = h_displacement * val + (1.0-h_displacement) * *(hf + k + l*max_x);
				}
				else	// pen_merge == ADD
					if (normalize)
					   val = ((1.0-h_displacement)* *(hf + k + l*max_x)) + h_displacement *
						(*(hf_pencil + i + j * pen_max_x));
					else
					   val = (*(hf + k + l*max_x)) + h_displacement *
						(*(hf_pencil + i + j * pen_max_x));
			*(hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),0xFFFF);
		}
	}
}


void hf_merge (hf_struct_type *hf_pencil, hf_struct_type *hf,
		int x, int y,
		gint pen_merge,
		gboolean pen_tiles,
		gfloat h_displacement,
		gboolean normalize) {

// See hf_merge_buf

hf_merge_buf (hf_pencil->hf_buf, hf_pencil->max_x, hf_pencil->max_y,
hf->hf_buf, hf->max_x, hf->max_y, x, y, 
pen_merge, pen_tiles, h_displacement, normalize);

}

void generalized_merge (gpointer map, gint map_data_type, gint size_x, gint size_y, 
	gpointer hf, gint hf_data_type, gint max_x, gint max_y, gint x, gint y,
	gint merge_type, gboolean wrap, gboolean square_symmetry) {

//	2004-01 Merge a map of size_x * size_y at center position (x,y) in hf
//	Used for drawing a continuous line and craters
//	[Eventually generalize to encompass hf_merge]
//	data_type: type of map, can be any of 
//	GINT_ID, HF_TYPE_ID, GDOUBLE_ID, UNSIGNED_CHAR_ID
//	merge_type can be any of : ADD, SUBTRACT, MULTIPLY, MULTIPLY2
//	square_symmetry is true only when the map has a square symmetry,
//	so that one quadrant is sufficient to deduce it
//	Implies size_x, size_y are odd, and generally that size_x == size_y
//	The size of a square map of radius R is (2*R+1) * (2*R+1)

	gint i,j,ix,iy, k, l, radius_x, radius_y,lx;
	glong val = 0;
	gdouble ratio, dval=0.0;
	gboolean if_val = TRUE;
	static gdouble dmax = (gdouble) MAX_HF_VALUE;

	radius_x = size_x >>1;
	radius_y = size_y >>1;

	x = x - radius_x;
	y = y - radius_y;

	if (square_symmetry)
		lx = radius_x + 1;
	else
		lx = size_x;

	for (i = 0; i < size_x ; i++) {
		// Calculate X index in hf world
		k = x + i;
		if (!wrap) {
			if (k>=max_x)
				break;
			else
				if (k<0)
					continue;
		}
		if (k < 0)
			k = max_x + k;
		else
			k = k%max_x;

		for (j = 0; j < size_y; j++) {
			// Calculate Y index in hf world
			l = y + j;
			if (!wrap) {
				if (l>=max_y)
					break;
				else
					if (l<0)
						continue;
			}
			if (l < 0)
				l = max_y + l;
			else
				l = l%max_y;

			ix = i;
			iy = j;
			if (square_symmetry) {
				if (i>radius_x)
					ix = 2*radius_x - i;
				if (j>radius_y)
					iy = 2*radius_y - j;
			}
// printf("(radius_y,radius_x): (%d,%d); (i,j): (%d,%d); (ix,iy): (%d,%d); lx: %d;\n",radius_x, radius_y,i,j, ix,iy, lx);
			switch (map_data_type) {
				case GINT_ID:
					val = (glong) *(((gint *) map) + ix + iy * lx);
					break;
				case HF_TYPE_ID:
					val = (glong) *(((hf_type *) map) + ix + iy * lx);
					break;
				case GDOUBLE_ID:
					dval = *(((gdouble *) map) + ix + iy * lx);
					if_val = FALSE;
					break;
				case UNSIGNED_CHAR_ID:
					val = (glong) *(((unsigned char *) map) + ix + iy * lx);
					break;
				default:
					printf(_("Unexpected option in %s; contact the programmer!"),"generalized_merge");
					printf("\n");
					return;
			}

			if (if_val && (hf_data_type==GDOUBLE_ID))
				dval = (gdouble) val;

			switch (merge_type) {
				case ADD:
				
				switch (hf_data_type) {
					case GINT_ID:
						val = *((gint *) hf + k + l*max_x) + val;
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = *((hf_type *) hf + k + l*max_x) + val;
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = *((gdouble *) hf + k + l*max_x) + dval;
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = *((unsigned char *) hf + k + l*max_x) + val;
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = *((hf_type *) hf + k + l*max_x) + val;
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				case SUBTRACT:
				switch (hf_data_type) {
					case GINT_ID:
						val = *((gint *) hf + k + l*max_x) - val;
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = *((hf_type *) hf + k + l*max_x) - val;
// printf("hf: %d\n", *((hf_type *) hf + k + l*max_x));
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = *((gdouble *) hf + k + l*max_x) - dval;
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = *((unsigned char *) hf + k + l*max_x) - val;
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = *((hf_type *) hf + k + l*max_x) - val;
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				case MULTIPLY:
				case MULTIPLY2:
					if (merge_type == MULTIPLY)
						ratio = ((gdouble) val) / dmax;
					else
						ratio = 1.0 + 0.5 * ((gdouble) val) / dmax;
				switch (hf_data_type) {
					case GINT_ID:
						val = (glong) (ratio * (gdouble) *((gint *) hf + k + l*max_x));
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val =(glong) (ratio * (gdouble) *((hf_type *) hf + k + l*max_x));
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						if (merge_type == MULTIPLY)
							ratio = dval / dmax;
						else
							ratio = 1.0 + 0.5 *  dval / dmax;
						dval = (ratio * *((gdouble *) hf + k + l*max_x));
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = (glong) (ratio * (gdouble) *((unsigned char *) hf + k + l*max_x));
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val =(glong) (ratio * (gdouble) *((hf_type *) hf + k + l*max_x));
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				case MAX_MERGE:
				switch (hf_data_type) {
					case GINT_ID:
						val = MAX(*((gint *) hf + k + l*max_x),val);
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = MAX(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = MAX(*((gdouble *) hf + k + l*max_x),dval);
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = MAX(*((unsigned char *) hf + k + l*max_x),val);
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = MAX(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				case MIN_MERGE:
				switch (hf_data_type) {
					case GINT_ID:
						val = MIN(*((gint *) hf + k + l*max_x),val);
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = MIN(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = MIN(*((gdouble *) hf + k + l*max_x),dval);
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = MIN(*((unsigned char *) hf + k + l*max_x),val);
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = MIN(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				default:
					printf(_("Unexpected option in %s; contact the programmer!"),"generalized_merge");
					printf("\n");
					return;
			}
		}
	}
}

void interpolated_merge (gpointer map, 
	gint map_data_type, gint size_x, gint size_y,
	gpointer hf, gint hf_data_type, gint max_x, gint max_y, 
	gdouble x, gdouble y,
	gint merge_type, gboolean wrap, gboolean square_symmetry) {

	// 2004-01 Merge similar to generalized merge
	// (x,y) are real coordinates in HF
	// We interpolate the pixels in the map to merge

	gint i,j, k, l, radius_x, radius_y,lx, return_gint = 0;
	unsigned char return_uns_char = 0;
	glong val = 0;
	gboolean if_val=TRUE;
	hf_type return_hf_type = 0;
	gdouble ratio, dx, dy, ix, iy, return_gdouble = 0.0, dval = 0.0;
	static gdouble dmax = (gdouble) MAX_HF_VALUE;

	radius_x = size_x >>1;
	radius_y = size_y >>1;

	dx = x - floor(x);
	dy = y - floor(y);
// printf("INTERPOLATED MERGE AT: (%5.2f, %5.2f); (DX,DY): (%5.2f,%5.2f)\n",x,y,dx,dy);
	x = floor(x) - (gdouble) radius_x ;
	y = floor(y) - (gdouble) radius_y;

	if (square_symmetry)
		lx = radius_x + 1;
	else
		lx = size_x;

	for (i = 0; i < (size_x+1) ; i++) {
		// Calculate X index in hf world
		k = i + (gint) x;
		if (!wrap) {
			if (k>=max_x)
				break;
			else
				if (k<0)
					continue;
		}
		if (k < 0)
			k = max_x + k;
		else
			k = k%max_x;

		for (j = 0; j < (size_y+1); j++) {
			// Calculate Y index in hf world
			l = j + (gint) y;
			if (!wrap) {
				if (l>=max_y)
					break;
				else
					if (l<0)
						continue;
			}
			if (l < 0)
				l = max_y + l;
			else
				l = l%max_y;

			ix = -dx + (gdouble) i;
			iy = -dy + (gdouble) j;
			if (square_symmetry) {
				if (ix>(gdouble) radius_x)
					ix = 2.0*(gdouble) radius_x - ix;
				if (iy>(gdouble) radius_y)
					iy = 2.0*(gdouble) radius_y - iy;
			}
// printf("(radius_y,radius_x): (%d,%d); (i,j): (%d,%d); (ix,iy): (%5.2f,%5.2f); lx: %d;\n",radius_x, radius_y,i,j, ix,iy, lx);
			switch (map_data_type) {
				case GINT_ID:
					interpolate (ix, iy, map, lx, lx, &return_gint,
						GINT_ID, OVERFLOW_ZERO);
					val = (glong) return_gint;
					break;
				case HF_TYPE_ID:
					interpolate (ix, iy, map, lx, lx, &return_hf_type,
						HF_TYPE_ID, OVERFLOW_ZERO);
					val = (glong) return_hf_type;
					break;
				case GDOUBLE_ID:
					interpolate (ix, iy, map, lx, lx, &return_gdouble,
						GDOUBLE_ID, OVERFLOW_ZERO);
//					val = (glong) return_gdouble;
					dval = return_gdouble;
					if_val = FALSE;
					break;
				case UNSIGNED_CHAR_ID:
					interpolate (ix, iy, map, lx, lx, &return_uns_char,
						UNSIGNED_CHAR_ID, OVERFLOW_ZERO);
					val = (glong) return_uns_char;
					break;
				default:
					printf(_("Unexpected option in %s; contact the programmer!"),"interpolated_merge");
					printf("\n");
					return;
			}

			if (if_val && (hf_data_type==GDOUBLE_ID))
				dval = (gdouble) val;

			switch (merge_type) {
				case ADD:				
				switch (hf_data_type) {
					case GINT_ID:
						val = *((gint *) hf + k + l*max_x) + val;
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = *((hf_type *) hf + k + l*max_x) + val;
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = *((gdouble *) hf + k + l*max_x) + dval;
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = *((unsigned char *) hf + k + l*max_x) + val;
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = *((hf_type *) hf + k + l*max_x) + val;
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				case SUBTRACT:
				switch (hf_data_type) {
					case GINT_ID:
						val = *((gint *) hf + k + l*max_x) - val;
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = *((hf_type *) hf + k + l*max_x) - val;
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = *((gdouble *) hf + k + l*max_x) - dval;
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = *((unsigned char *) hf + k + l*max_x) - val;
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = *((hf_type *) hf + k + l*max_x) - val;
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				case MULTIPLY:
				case MULTIPLY2:
					if (merge_type == MULTIPLY)
						ratio = ((gdouble) val) / dmax;
					else
						ratio = 1.0 + 0.5 * ((gdouble) val) / dmax;
				switch (hf_data_type) {
					case GINT_ID:
						val = (glong) (ratio * (gdouble) *((gint *) hf + k + l*max_x));
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val =(glong) (ratio * (gdouble) *((hf_type *) hf + k + l*max_x));
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						if (merge_type == MULTIPLY)
							ratio = dval / dmax;
						else
							ratio = 1.0 + 0.5 *  dval / dmax;
						dval = (ratio * *((gdouble *) hf + k + l*max_x));
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = (glong) (ratio * (gdouble) *((unsigned char *) hf + k + l*max_x));
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val =(glong) (ratio * (gdouble) *((hf_type *) hf + k + l*max_x));
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
				case MAX_MERGE:
				switch (hf_data_type) {
					case GINT_ID:
						val = MAX(*((gint *) hf + k + l*max_x),val);
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = MAX(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = MAX(*((gdouble *) hf + k + l*max_x),dval);
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = MAX(*((unsigned char *) hf + k + l*max_x),val);
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = MAX(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				case MIN_MERGE:
				switch (hf_data_type) {
					case GINT_ID:
						val = MIN(*((gint *) hf + k + l*max_x),val);
						*((gint *)hf + k + l*max_x) = (gint) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case HF_TYPE_ID:
						val = MIN(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					case GDOUBLE_ID:
						dval = MIN(*((gdouble *) hf + k + l*max_x),dval);
						*((gdouble *)hf + k + l*max_x) = (gdouble) MIN(MAX(dval,0.0),dmax);
						break;
					case UNSIGNED_CHAR_ID:
						val = MIN(*((unsigned char *) hf + k + l*max_x),val);
						*((unsigned char *)hf + k + l*max_x) = (unsigned char) MIN(MAX(val,0),MAX_HF_VALUE);
						break;
					default:
						val = MIN(*((hf_type *) hf + k + l*max_x),val);
						*((hf_type *)hf + k + l*max_x) = (hf_type) MIN(MAX(val,0),MAX_HF_VALUE);
				}
					break;
				default:
					printf(_("Unexpected option in %s; contact the programmer!"),"interpolated_merge");
					printf("\n");
					return;
			}
		}
	}
}

//	There are some explanations in hf.h!

void dist_matrix_init(dist_matrix_struct *dist, gint hf_size) {
//	Initializes / reinit the distances matrix with a new hf_size
	gint i, j;
	if (dist->hf_size == hf_size) // Nothing to do!
		return;
	dist->hf_size = hf_size;
	dist->size = (hf_size>>1) * (hf_size>>1);
	if (dist->distances)
		x_free(dist->distances);
	dist->distances = (gfloat *) x_malloc(sizeof(gfloat) * dist->size, "gfloat (dist->distances)");
	for (i=0; i<(hf_size>>1); i++) {
		for (j=0; j<(hf_size>>1); j++) {
			(*(dist->distances+VECTORIZE(i,j,hf_size>>1))) = (gfloat) sqrt(i*i + j*j);
// printf("I: %d;  J: %d;  VECTORIZE: %d;  DIST: %-10.2f\n",i, j, VECTORIZE(i,j,hf_size>>1), (gfloat) sqrt(i*i + j*j));
		}
	}
}

dist_matrix_struct *dist_matrix_new(gint hf_size) {

	dist_matrix_struct *dist;
	dist = (dist_matrix_struct *) x_malloc(sizeof(dist_matrix_struct), "dist_matrix_struct");
	dist->distances = NULL;
	dist->hf_size = 0;
	if (hf_size)
		dist_matrix_init(dist, hf_size);
	return dist;
}


void hf_translate(hf_type *source, hf_type *result, 
	gint max_x, gint max_y, gint dx, gint dy) {
	gint i,j;
	for (i=0; i<max_y; i++) {
		for (j=0; j<max_x; j++) {
			*(result + VECTORIZE(i,j,max_x)) = 
				*(source + VECTORIZE(WRAP(i+dy,max_y),
					WRAP2(j+dx,max_x), max_x));
		}
	}
}

void hf_slide(hf_struct_type *hf, gint slidev, gint slideh) {
//	Translate a HF vertically or horizontally from 0 to 100 %
//	Suitable for "wrappable" HF only...
	gint dx, dy;
	dx = (slideh * hf->max_x) / 100;
	dy = - (slidev * hf->max_y) / 100;
	if (!hf->tmp_buf)
		hf_backup(hf);
	hf_translate (hf->tmp_buf, hf->hf_buf, hf->max_x, hf->max_y, dx, dy);
}

void hf_revert (hf_struct_type *hf) {
	gint i;
	if (hf->tmp_buf) {
	//	We free the temp buffer because we don't need it and we want to avoid
	//	inconsistencies in other actions... "revert" is totally reversible
		x_free(hf->tmp_buf);
		hf->tmp_buf = NULL;
	}
	
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		*(hf->hf_buf + i) = ~*(hf->hf_buf +i);
	}
}

void hf_vertical_mirror(hf_struct_type *hf) {
	gint i,j;
	// We need a fresh temporary buffer here
	if (!hf->tmp_buf)
		hf_backup(hf);
	for (i=0; i<hf->max_y; i++)
		for (j=0; j<hf->max_x; j++) 
			*(hf->hf_buf+VECTORIZE(i,hf->max_x-j-1, hf->max_x)) =
				*(hf->tmp_buf+VECTORIZE(i,j, hf->max_x));
}

void hf_horizontal_mirror(hf_struct_type *hf) {
	gint i,j;
	// We need a fresh temporary buffer here
	if (!hf->tmp_buf)
		hf_backup(hf);
	for (i=0; i<hf->max_y; i++)
		for (j=0; j<hf->max_x; j++)
			*(hf->hf_buf+VECTORIZE(hf->max_y-i-1,j, hf->max_x)) =
				*(hf->tmp_buf+VECTORIZE(i,j, hf->max_x));
}

hf_type interpolate_get_xy (gdouble dbx, gdouble dby, hf_type *hf,
	gint x_size, gint y_size,
	gdouble (*get_x) (gdouble, gpointer), gdouble (*get_y) (gdouble, gpointer),
	gpointer arg_get_x, gpointer arg_get_y) {
	// See "interpolate"
	// "get_x" and "get_y" are functions for getting the coordinates of x and y,
	// when a special transformation is required (then dbx and dby are not used)
	// "get_x" and "get_y" could be NULL
	if (get_x)
		dbx = (gdouble) (*get_x) (dbx, arg_get_x);
	if (get_y)
		dby = (gdouble) (*get_y) (dby, arg_get_y);
	return interpolate2 (dbx,dby, hf, x_size, y_size);
}

hf_type interpolate2 (gdouble dbx, gdouble dby, hf_type *hf,
	gint x_size, gint y_size) {

	// Find an interpolated value for (dbx,dby) in *hf
	// Interpolation from distance
	// Used for rotation and similar functions
	// "hf" is not required to be square

	// 2004-01-02 Renamed interpolate2, kept for documentation
	// Use "interpolate" instead

	gdouble dx,dy, v1, v2, v3, v4,d1,d2,d3,d4, value;
	long int xa, ya, xb, yb;
	xa = (long int) floor(dbx);
	ya = (long int) floor(dby);
	xb = WRAP2(xa ,x_size);
	yb = WRAP2(ya ,y_size);
	dx = ABS(dbx - floor(dbx)); // dx,dy = float part, used for interpolation
	dy = ABS(dby - floor(dby));
// printf("(x,y): (%5.2f,%5.2f);  (dx,dy): (%f,%f); (xa,ya): (%d,%d); (xb,yb): (%d,%d)\n",dbx,dby,dx,dy,xa,ya,xb,yb);
//	Simple interpolation from distance
	d1 = MAX(0.0,1.0 - DIST2(0.0,0.0,dx,dy));
	d2 = MAX(0.0,1.0 - DIST2(0.0,1.0,dx,dy));
	d3 = MAX(0.0,1.0 - DIST2(1.0, 0.0, dx, dy));
	d4 = MAX(0.0,1.0- DIST2(1.0,1.0, dx,dy));
/*	Too simple bi-linear interpolation... produces some aliasing
	d1 = 1.0 - dx*dy;
	d2 = 1.0 - dx * (1.0-dy);
	d3 = 1.0 - (1.0-dx) * dy;
	d4 = 1.0 - (1.0-dx) * (1.0-dy);
*/
// printf("(d1,d2,d3,d4): (%f,%f,%f,%f)\n",d1,d2,d3,d4);
	v1 = d1 * (gdouble) *(hf+VECTORIZE(xb,yb,x_size)) ;
	v2 = d2 * (gdouble) *(hf+VECTORIZE(xb,WRAP(yb+1,y_size),x_size));
	v3 = d3 * (gdouble) *(hf+VECTORIZE(WRAP(xb+1,x_size),yb,x_size));
	v4 = d4 * (gdouble) *(hf+VECTORIZE(WRAP(xb+1,x_size),WRAP(yb+1,y_size),x_size));
	value =  ((v1+v2+v3+v4) / (d1+d2+d3+d4));
	return (hf_type) value;
}

void translate_real_forward_mapping (gpointer source_grid,
	gpointer output_grid,
	gint data_type,
	gint x_size, gint y_size,
	gint x,gint y,
	gdouble ox, gdouble oy) {

	// Translate the value source_grid(x,y) to output_grid(ox,oy),
	// given that (ox,oy) are real values, using forward mapping
	// Opposite of interpolating...
	// We "spread" the value over the 4 pixels surrounding the target (ox,oy),
	// using the distance between (ox,oy) and the integer coordinates
	// We assume that overflow / tiling control is not relevant

	gdouble dx, dy, d1,d2,d3,d4, tot, value;
	gint ix, iy,x1,y1, x2, y2, i, j1, j2, j3, j4;
	//gboolean t1=FALSE,t2=FALSE,t3=FALSE,t4=FALSE;

	ix = (gint) floor(ox);
	iy = (gint) floor(oy);
	x1 = x + ix;
	y1 = y + iy;
	if ((x1<-1) || (y1<-1) || (x1>x_size) || (y1>y_size)) // Target trivially out of boundaries
		return;
	x2 = x1+1;
	y2 = y1+1;
	dx = ox - floor(ox);
	dy = oy - floor(oy);

	d1 = MAX(0.0,1.0 - DIST2(0.0,0.0,dx,dy));
	d2 = MAX(0.0,1.0 - DIST2(0.0,1.0,dx,dy));
	d3 = MAX(0.0,1.0 - DIST2(1.0, 0.0, dx, dy));
	d4 = MAX(0.0,1.0- DIST2(1.0,1.0, dx,dy));
	tot = d1+d2+d3+d4;
// printf("D1: %5.2f; D2: %5.2f; D3: %5.2f; D4: %5.2f; tot: %5.2f; ",d1,d2,d3,d4, tot);
	i = VECTORIZE(x, y, x_size);
	j1= VECTORIZE(x1,y1, x_size); // (x1,y1)
	j2= j1 + x_size; 	// (x1,y2)
	j3 = j1 + 1; 	// (x2,y1)
	j4 = j2  + 1; 	// (x2,y2)

	switch (data_type) {
		case GINT_ID:
			value = (gdouble)  *( ((gint *) source_grid) + i);
			if ( (x1>=0) && (y1>=0) && (x1<x_size) && (y1<y_size))
				*( ((gint *) output_grid) + j1) += (gint) (d1 * value / tot);
			if ( (x1>=0) && (y2>=0) && (x1<x_size) && (y2<y_size))
				*( ((gint *) output_grid) + j2) += (gint) (d2 * value / tot);
			if ( (x2>=0) && (y1>=0) && (x2<x_size) && (y1<y_size))
				*( ((gint *) output_grid) + j3) += (gint) (d3 * value / tot);
			if ( (x2>=0) && (y2>=0) && (x2<x_size) && (y2<y_size))
				*( ((gint *) output_grid) + j4) += (gint) (d4 * value / tot);
			break;
		case HF_TYPE_ID:
			value = (gdouble) *( ((hf_type *) source_grid) + i);


// printf("VALUE (%d, %d): %5.2f; (ix,iy): (%d, %d); j1: %d; j2: %d; j3: %d; j4: %d\n",x,y,value, ix, iy, j1, j2, j3, j4);
			if ( (x1>=0) && (y1>=0) && (x1<x_size) && (y1<y_size)) {
				*( ((hf_type *) output_grid) + j1) += (hf_type) (d1 * value / tot);
				//t1 = TRUE;
			}
			if ( (x1>=0) && (y2>=0) && (x1<x_size) && (y2<y_size)) {
				*( ((hf_type *) output_grid) + j2) += (hf_type) (d2 * value / tot);
				//t2 = TRUE;
			}
			if ( (x2>=0) && (y1>=0) && (x2<x_size) && (y1<y_size)) {
				*( ((hf_type *) output_grid) + j3) += (hf_type) (d3 * value / tot);
				//t3 = TRUE;
			}
			if ( (x2>=0) && (y2>=0) && (x2<x_size) && (y2<y_size)) {
				*( ((hf_type *) output_grid) + j4) += (hf_type) (d4 * value / tot);
				//t4 = TRUE;
			}
//		if ( (x1<1) || (x1>=(x_size-1)) || (y1<1) || (y1>=(y_size-1)))
// printf ("VALUE (%d, %d): %5.2f; (x1,y1): (%d, %d); (x2,y2): (%d, %d); t1,t2,t3,t4: %d,%d,%d,%d;  j1: %d; j2: %d; j3: %d; j4: %d; maxi: %d\n",x,y,value,x1,y1,x2,y2,t1,t2,t3,t4,j1,j2,j3,j4,VECTORIZE(x_size-1,y_size-1,x_size));
			break;
		case GDOUBLE_ID:
			value = *( ((gdouble *) source_grid) + i);
			if ( (x1>=0) && (y1>=0) && (x1<x_size) && (y1<y_size))
				*( ((gdouble *) output_grid) + j1) += (gdouble) (d1 * value / tot);
			if ( (x1>=0) && (y2>=0) && (x1<x_size) && (y2<y_size))
				*( ((gdouble *) output_grid) + j2) += (gdouble) (d2 * value / tot);
			if ( (x2>=0) && (y1>=0) && (x2<x_size) && (y1<y_size))
				*( ((gdouble *) output_grid) + j3) += (gdouble) (d3 * value / tot);
			if ( (x2>=0) && (y2>=0) && (x2<x_size) && (y2<y_size))
				*( ((gdouble *) output_grid) + j4) += (gdouble) (d4 * value / tot);

			break;
		case UNSIGNED_CHAR_ID:
			value = *( ((unsigned char *) source_grid) + i);
			if ( (x1>=0) && (y1>=0) && (x1<x_size) && (y1<y_size))
				*( ((unsigned char *) output_grid) + j1) += (unsigned char) (d1 * value / tot);
			if ( (x1>=0) && (y2>=0) && (x1<x_size) && (y2<y_size))
				*( ((unsigned char *) output_grid) + j2) += (unsigned char) (d2 * value / tot);
			if ( (x2>=0) && (y1>=0) && (x2<x_size) && (y1<y_size))
				*( ((unsigned char *) output_grid) + j3) += (unsigned char) (d3 * value / tot);
			if ( (x2>=0) && (y2>=0) && (x2<x_size) && (y2<y_size))
				*( ((unsigned char *) output_grid) + j4) += (unsigned char) (d4 * value / tot);
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),
				"translate_real_forward_mapping");
			printf("\n");
	}
}

void translate_forward_mapping (gpointer source_grid,
	gpointer output_grid,
	gint data_type,
	gint x_size, gint y_size,
	gint x,gint y,
	gint ix, gint iy) {

	// Translate the value source_grid(x,y) to output_grid(ix,iy),
	// given that (ix,iy) are integer values, using forward mapping
	// We assume that overflow / tiling control is not relevant
	// Typically used instead of translate_real_forward_mapping, in preview modes

	gint x1,y1, i, j1;

	x1 = x + ix;
	y1 = y + iy;
	if ((x1<0) || (y1<0) || (x1>=x_size) || (y1>=y_size)) // Target out of boundaries
		return;

	i = VECTORIZE(x, y, x_size);
	j1= VECTORIZE(x1,y1, x_size);

	switch (data_type) {
		case GINT_ID:
			*( ((gint *) output_grid) + j1) = *( ((gint *) source_grid) + i);
			break;
		case HF_TYPE_ID:
			*( ((hf_type *) output_grid) + j1) = *( ((hf_type *) source_grid) + i);
			break;
		case GDOUBLE_ID:
			*( ((gdouble *) output_grid) + j1) = *( ((gdouble *) source_grid) + i);
			break;
		case UNSIGNED_CHAR_ID:
			*( ((unsigned char *) output_grid) + j1) = *( ((unsigned char *) source_grid) + i);
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),
				"translate_forward_mapping");
			printf("\n");
	}
}
void interpolate (gdouble dbx, gdouble dby, gpointer grid,
	gint x_size, gint y_size, gpointer return_value_ptr, gint data_type,
	gint overflow) {

	// Find an interpolated value for (dbx,dby) in "grid"
	// Interpolation from distance
	// Used for rotation and similar functions
	// (dbx,dby):	real coordinates in "grid"
	// grid:		any table of type "data_type"; not required to be square
	// x_size * y_size:	size of the grid, in "data_type" units
	// return_value_ptr:	pointer in which the value is returned - allocated by the calling function
	// data_type:	any of GINT_ID, HF_TYPE_ID,
	//			GDOUBLE_ID, UNSIGNED_CHAR_ID (defined in hf.h)
	// overflow:		any of OVERFLOW_WRAP, ..._REBOUND, ..._ZERO, ..._IDLE

	gdouble dx,dy, v1=0.0, v2=0.0, v3=0.0, v4=0.0,d1,d2,d3,d4;
	gboolean tx1=TRUE,tx2=TRUE,ty1=TRUE,ty2=TRUE;
	glong xa, ya, i1, i2, i3, i4, x1, x2, y1, y2;

// printf("INTERPOLATING (%5.2f,%5.2f); ", dbx,dby);
	xa = (glong) floor(dbx);
	ya = (glong) floor(dby);
	dx = ABS(dbx - floor(dbx)); // dx,dy = float part, used for interpolation
	dy = ABS(dby - floor(dby));

//	Simple interpolation from distance
	d1 = MAX(0.0,1.0 - DIST2(0.0,0.0,dx,dy));
	d2 = MAX(0.0,1.0 - DIST2(0.0,1.0,dx,dy));
	d3 = MAX(0.0,1.0 - DIST2(1.0, 0.0, dx, dy));
	d4 = MAX(0.0,1.0- DIST2(1.0,1.0, dx,dy));

// printf("(d1,d2,d3,d4): (%f,%f,%f,%f)\n",d1,d2,d3,d4);
	switch (overflow) {
		case OVERFLOW_WRAP:
			x1 = WRAP2(xa,x_size);
			x2 = WRAP2(xa+1,x_size);
			y1 = WRAP2(ya,y_size);
			y2 = WRAP2(ya+1,y_size);
			break;
		case OVERFLOW_REBOUND:
			x1 = REBOUND(xa,x_size);
			x2 = REBOUND(xa+1,x_size);
			y1 = REBOUND(ya,y_size);
			y2 = REBOUND(ya+1,y_size);
			break;
		case OVERFLOW_IDLE:
			x1 = MAX(0,MIN(xa,x_size-1));
			x2 = MAX(0,MIN(xa+1,x_size-1));
			y1 = MAX(0,MIN(ya,y_size-1));
			y2 = MAX(0,MIN(ya+1,y_size-1));
			break;
		case OVERFLOW_ZERO:
			x1 = xa;
			x2 = xa+1;
			y1 = ya;
			y2 = ya+1;
			if ( (x1<0) || (x1>=x_size) ) {
				tx1 = FALSE;
				x1 =	MAX(0,MIN(x1,x_size-1));	// We initialize to a valid index, for avoiding errors
			}
			if ( (x2>=x_size) || (x2<0)  ) {
				tx2 = FALSE;
				x2 =	MAX(0,MIN(x2,x_size-1));
			}
			if ( (y1<0) || (y1>=y_size) ) {
				ty1 = FALSE;
				y1 =	MAX(0,MIN(y1,y_size-1));
			}
			if ( (y2>=y_size) || (y2<0)  ) {
				ty2 = FALSE;
				y2 =	MAX(0,MIN(y2,y_size-1));
			}
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),"interpolate");
			printf("\n");
			return;
	}

	i1 = VECTORIZE(x1,y1,x_size);
	i2 = VECTORIZE(x1,y2,x_size);
	i3 = VECTORIZE(x2,y1,x_size);
	i4 = VECTORIZE(x2,y2,x_size);

	switch (data_type) {
		case GINT_ID:
			if (tx1 && ty1)
				v1 = d1 * (gdouble) *( ((gint*) grid) + i1) ;
			if (tx1 && ty2)
				v2 = d2 * (gdouble) *( ((gint*) grid) + i2);
			if (tx2 && ty1)
				v3 = d3 * (gdouble) *( ((gint*) grid) + i3);
			if (tx2 && ty2)
				v4 = d4 * (gdouble) *( ((gint*) grid) + i4);
			*((gint *) return_value_ptr) = (gint) ((v1+v2+v3+v4) / (d1+d2+d3+d4)) ;
			break;
		case HF_TYPE_ID:
			if (tx1 && ty1)
				v1 = d1 * (gdouble) *( ((hf_type*) grid) + i1) ;
			if (tx1 && ty2)
				v2 = d2 * (gdouble) *( ((hf_type*) grid) + i2);
			if (tx2 && ty1)
				v3 = d3 * (gdouble) *( ((hf_type*) grid) + i3);
			if (tx2 && ty2)
				v4 = d4 * (gdouble) *( ((hf_type*) grid) + i4);
			*((hf_type *) return_value_ptr) = (hf_type) ((v1+v2+v3+v4) / (d1+d2+d3+d4)) ;
// printf("VALUE: %d; (x1,y1): (%d,%d);  (x2,y2): (%d,%d); d1: %5.2f; d2: %5.2f; d3: %5.2f; d4: %5.2f; v1: %5.2f; v2: %5.2f; v3: %5.2f; v4: %5.2f\n",* (hf_type*) return_value_ptr,x1,y1,x2,y2,d1,d2,d3,d4,v1,v2,v3,v4);
			break;
		case GDOUBLE_ID:
			if (tx1 && ty1)
				v1 = d1 * (gdouble) *( ((gdouble*) grid) + i1) ;
			if (tx1 && ty2)
				v2 = d2 * (gdouble) *( ((gdouble*) grid) + i2);
			if (tx2 && ty1)
				v3 = d3 * (gdouble) *( ((gdouble*) grid) + i3);
			if (tx2 && ty2)
				v4 = d4 * (gdouble) *( ((gdouble*) grid) + i4);
			*((gdouble *) return_value_ptr) = (gdouble) ((v1+v2+v3+v4) / (d1+d2+d3+d4)) ;
			break;
		case UNSIGNED_CHAR_ID:
			if (tx1 && ty1)
				v1 = d1 * (gdouble) *( ((unsigned char*) grid) + i1) ;
			if (tx1 && ty2)
				v2 = d2 * (gdouble) *( ((unsigned char*) grid) + i2);
			if (tx2 && ty1)
				v3 = d3 * (gdouble) *( ((unsigned char*) grid) + i3);
			if (tx2 && ty2)
				v4 = d4 * (gdouble) *( ((unsigned char*) grid) + i4);
			*((unsigned char *) return_value_ptr) = (unsigned char) ((v1+v2+v3+v4) / (d1+d2+d3+d4)) ;
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),"interpolate");
			printf("\n");
	}
}

void vector_interpolate (gdouble dbx, gpointer vector, gint x_size,
	gpointer return_value_ptr, gint data_type, gint overflow) {
	// Same as "interpolate", for a vector instead of a grid
	glong xa, x1, x2;
	gdouble fx, d1, d2, v1=0.0, v2=0.0;
	gboolean tx1=TRUE, tx2=TRUE;

	fx = floor(dbx);
	xa = (glong) fx;
	d2 = ABS(dbx - fx);
	d1 = 1.0 - d2;

	switch (overflow) {
		case OVERFLOW_WRAP:
			x1 = WRAP2(xa,x_size);
			x2 = WRAP2(xa+1,x_size);
			break;
		case OVERFLOW_REBOUND:
			x1 = REBOUND(xa,x_size);
			x2 = REBOUND(xa+1,x_size);
			break;
		case OVERFLOW_IDLE:
			x1 = MAX(0,MIN(xa,x_size));
			x2 = MAX(0,MIN(xa+1,x_size));
			break;
		case OVERFLOW_ZERO:
			x1 = xa;
			x2 = xa+1;
			if ( (x1<0) || (x1>=x_size) ) {
				tx1 = FALSE;
				x1 =	MAX(0,MIN(x1,x_size));	// We initialize to a valid index, for avoiding errors
			}
			if ( (x2>=x_size) || (x2<0)  ) {
				tx2 = FALSE;
				x2 =	MAX(0,MIN(x2,x_size));
			}
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),"vector_interpolate");
			printf("\n");
			return;
	}
	switch (data_type) {
		case GINT_ID:
			if (tx1)
				v1 = d1 * (gdouble) *( ((gint*) vector) + x1) ;
			if (tx2)
				v2 = d2 * (gdouble) *( ((gint*) vector) + x2);
			*((gint *) return_value_ptr) = (gint) ((v1+v2) / (d1+d2)) ;
			break;
		case HF_TYPE_ID:
			if (tx1)
				v1 = d1 * (gdouble) *( ((hf_type*) vector) + x1) ;
			if (tx2)
				v2 = d2 * (gdouble) *( ((hf_type*) vector) + x2);
			*((hf_type *) return_value_ptr) = (hf_type) ((v1+v2) / (d1+d2)) ;

// printf("VALUE: %d; x1: %d;  x2: %d; d1: %5.2f; d2: %5.2f; v1: %5.2f; v2: %5.2f;\n",* (hf_type*) return_value_ptr,x1,x2,d1,d2,v1,v2);
			break;
		case GDOUBLE_ID:
			if (tx1)
				v1 = d1 * (gdouble) *( ((gdouble*) vector) + x1) ;
			if (tx2)
				v2 = d2 * (gdouble) *( ((gdouble*) vector) + x2);
			*((gdouble *) return_value_ptr) = (gdouble) ((v1+v2) / (d1+d2)) ;
// printf("VALUE: 5.2f; dbx: %5.2f; x1: %d;  x2: %d; d1: %5.2f; d2: %5.2f; v1: %5.2f; v2: %5.2f;\n",* (gdouble*) return_value_ptr,dbx,x1,x2,d1,d2,v1,v2);
			break;
		case UNSIGNED_CHAR_ID:
			if (tx1)
				v1 = d1 * (gdouble) *( ((unsigned char*) vector) + x1) ;
			if (tx2)
				v2 = d2 * (gdouble) *( ((unsigned char*) vector) + x2);
			*((unsigned char *) return_value_ptr) = (unsigned char) ((v1+v2) / (d1+d2)) ;
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),"vector_interpolate");
			printf("\n");
	}
}

void hf_circular_projection (hf_type *hf_in, hf_type *hf_out, gint hf_size) {
	// Compresses the corners of the HF so that it fits into a circle
	// Completes the corner by wrapping the coordinates
	// The HF should be tileable for a seamless result
	// 2003-11-29
	// Intended uses: (1) curiosity; (2) minimize the "square effect" of some transformations
	// 2003-11-30 A bit disappointing - won't implement
	// We use inverse mapping
	gint x,y, half;
	gdouble dist, dbx, dby, in_x, in_y;
	half = hf_size / 2;  // We center the projection
	for (y=0; y<hf_size; y++) {
		dby = (gdouble) (y-half);
		for (x=0; x<hf_size; x++) {
			dbx = (gdouble) (x-half);
			if ((x==0) || (y==0)) // Nothing to do 
				*(hf_out+VECTORIZE(x,y,hf_size)) = *(hf_in+VECTORIZE(x,y,hf_size));
			else {
				dist = DIST2(0.0,0.0, dbx, dby); // Distance from center
				if (ABS(dbx)>ABS(dby)) {
					in_x = (dbx<0?-1:1)*dist;
					in_y = in_x*dby/dbx;
				}
				else {
					in_y = (dby<0?-1:1)*dist;
					in_x = in_y*dbx/dby;
				} // if (test abs) ... else

// printf("(x,y): (%d,%d); (dbx,dby): (%5.1f,%5.1f); dist: %7.3f; (in_x,in_y): (%d,%d)\n",x,y,dbx,dby,dist,half +(gint) in_x,half + (gint) in_y);
			*(hf_out+VECTORIZE(x,y,hf_size)) =
				*(hf_in+VECTORIZE(WRAP2(half + (gint) in_x,hf_size),
						WRAP2(half + (gint) in_y,hf_size),hf_size));
			} // if (test 0)...
		} // for y
	} // for x
}

void rotate_sin_cos (gdouble sin, gdouble cos, gpointer map_in, gpointer map_out, gint hsize, gint vsize, gint data_type, gint overflow) {

//	Rotate map_in in map_out
//	Both arrays are of type data_type (GINT_ID, HF_TYPE_ID,
//	GDOUBLE_ID, UNSIGNED_CHAR_ID)
//	Both of size hsize * vsize
//	Sine and cosine are given by dx, dy (no explicit computation of angle)
//	map_out is supposed to be allocated to the right size
//	Reverse mapping process

	gdouble x_rot, y_rot, xx, yy, dradius_y, dradius_x;
	gint x, y;
	hf_type return_hf_type;
	unsigned char return_uns_char;
	gdouble return_gdouble;
	gint return_gint;
	sin = -sin ; // Reverse mapping
	dradius_y = 0.5 * ((gdouble) vsize - 1.0);
	dradius_x = 0.5 * ((gdouble) hsize - 1.0);

	for (y=0; y<vsize; y++) {
	    for (x=0; x<hsize; x++) {
		xx = ((gdouble) x) - dradius_x; // Rotate around the center, not (0,0)
		yy = ((gdouble) y) - dradius_y;
		x_rot = dradius_x + xx*cos - yy*sin;
		y_rot = dradius_y + yy*cos + xx*sin;

		switch (data_type) {
			case GINT_ID:
				interpolate (x_rot, y_rot, map_in, hsize, vsize, &return_gint,
					GINT_ID, overflow);
				*(((gint *) map_out)+VECTORIZE(x,y,hsize)) = return_gint;
				break;
			case HF_TYPE_ID:
				// The next lines help to test the effect of "interpolate"
//				return_hf_type = *(((hf_type *) map_in)+VECTORIZE(WRAP2((gint) x_rot,hsize), WRAP2((gint) y_rot, vsize), hsize));
//				return_hf_type = *(((hf_type *) map_in)+
//					VECTORIZE(REBOUND((gint) x_rot,hsize), REBOUND((gint) y_rot, vsize), hsize));
				interpolate (x_rot, y_rot, map_in, hsize, vsize, &return_hf_type,
					HF_TYPE_ID, overflow);
				*(((hf_type *) map_out)+VECTORIZE(x,y,hsize)) = return_hf_type;
				break;
			case GDOUBLE_ID:
				interpolate (x_rot, y_rot, map_in, hsize, vsize, &return_gdouble,
					GDOUBLE_ID, overflow);
				*(((gdouble *) map_out)+VECTORIZE(x,y,hsize)) = return_gdouble;
				break;
			case UNSIGNED_CHAR_ID:
				interpolate (x_rot, y_rot, map_in, hsize, vsize, &return_uns_char,
					UNSIGNED_CHAR_ID, overflow);
				*(((unsigned char *) map_out)+VECTORIZE(x,y,hsize)) = return_uns_char;
				break;
			default:
				printf(_("Unexpected option in %s; contact the programmer!"),"rotate_sin_cos");
				printf("\n");
				return;
		} // switch
	    } // for (x)
	 } // for (y)
}

void rotate (gdouble dx, gdouble dy, gpointer map_in, gpointer map_out,
	gint hsize, gint vsize, gint data_type, gint overflow) {

//	Rotate map_in in map_out
//	Both arrays are of type data_type (GINT_ID, HF_TYPE_ID,
//	GDOUBLE_ID, UNSIGNED_CHAR_ID)
//	Both of size hsize * vsize
//	Sine and cosine are given by dx, dy (no explicit computation of angle)
//	map_out is supposed to be allocated to the right size
//	Reverse mapping process

	gdouble ddist, cos, sin;
	ddist =  sqrt( pow( dx ,2.0) + pow(dy, 2.0) ) ;
	cos = dx / ddist;
	sin = dy / ddist;
	rotate_sin_cos (sin, cos, map_in, map_out, hsize, vsize, data_type, overflow);
}

void hf_rotate (hf_type *hf_in, hf_type *hf_out, gint hf_size,
		gint angle, gboolean wrap, gint overflow) {
//	Rotation of a square HF
//	Angle is in degrees
	gdouble a;
	a = PI * ((gdouble) angle) / 180.0;
	rotate_sin_cos (sin(a), cos(a), (gpointer) hf_in, (gpointer) hf_out, hf_size, hf_size, HF_TYPE_ID,
		overflow);
}

void hf_fast_rotate (hf_type *hf_in, hf_type *hf_out, gint hf_size, gint angle) {
//	Rotation of a square HF, preview style, with aliasing
//	"Wraps"
//	Angle is in degrees
	gdouble a, s, c, x_rot, y_rot;
	long int xa, ya, x, y;
	a = PI * ((gdouble) angle) / 180;
//	Reverse mapping
	s = sin(-a);
	c = cos(-a);
	for (y=0; y<hf_size; y++)
	    for (x=0; x<hf_size; x++) {
//	Nearest neighbor interpolation
		x_rot = x*c - y*s ;
		y_rot = y*c + x*s ;
		xa = WRAP2((long int) (x_rot+0.5),hf_size);
		ya = WRAP2((long int) (y_rot+0.5),hf_size);
		*(hf_out+VECTORIZE(x,y,hf_size)) = *(hf_in+VECTORIZE(xa,ya,hf_size)) ;
	    }
}

void norm(vector *v) {
	// Normalizes a vector, so that it's length is 1
	v->l = sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
	v->x = v->x  / v->l;
	v->y = v->y / v->l;
	v->z = v->z / v->l;
}

gdouble* normalized_bell_new(gint radius) {
	//	Allocates and initializes half a gaussian bell (plus the 0 value)
	//	It's normalized, so that the sum of the whole bell is 1.0
	gdouble *result, sum=0.0, value;
	gint i;
	result = (gdouble *) x_malloc(sizeof(gdouble)* (radius+1), "gdouble (result in normalized_bell_new)");
	if (!radius) {
		*result = 1.0;
		return result;
	}
	*result = BELLD(CONST_E,2.0,1,radius);
	sum = *result;
	for (i=1; i<=radius; i++) {
		value = BELLD(CONST_E,2.0,i+1,radius);
		*(result+i) = value;
		sum+=2*value; // once for the positive part, once for the negative
	}
	// Normalize
	value = 1.0 / sum;
	for (i=0; i<=radius; i++) {
		*(result+i) = *(result+i) * value;
	}
	return result;
}

void convolve_normalized_vector (gpointer in,gpointer out, gint max_x, gint max_y, gboolean wrap, 
		gint radius, gdouble *vector, gint data_type) {
	//	Convolve with a normalized "half-vector" on x and y
	//	(the vector is symmetric around 0 and the sum of the whole is 1)
	//	"Separated" algorithm
	//	Actually it's the same as convolving with a matrix
	//	equal to the cross product of the vector
	//	This function can take a rectangular image
	//	Accepts 4 data types:
	//		data_type = GINT_ID for gint / int
	//		data_type = HF_TYPE_ID for hf_type (unsigned short int)
	//		data_type = GDOUBLE_ID for gdouble / double
	//		data_type = UNSIGNED_CHAR_ID for unsigned char
	gdouble *buf, value, *d_in, *d_out;
	gint x,y,i, idx, idx_in,max2x, max2y, *i_in, *i_out;
	hf_type *hf_in, *hf_out;
	unsigned char *us_in, *us_out;
	i_in = in; i_out = out;
	d_in = in; d_out = out;
	hf_in = in; hf_out = out;
	us_in = in; us_out = out;
	max2x = 2*max_x-1;
	max2y = 2*max_y-1;
	buf = (gdouble *) x_malloc(sizeof(gdouble)*max_x*max_y, "gdouble (buf in convolve_normalized_vector)");
	// Summarize on the x axis
	for (y=0; y<max_y; y++) {
		for (x=0; x<max_x; x++) {
			idx = VECTORIZE(x,y,max_x);
			*(buf+idx) = 0.0; 
			for (i=-radius; i<=radius; i++) {
				if (wrap)
					idx_in = VECTORIZE(WRAP2(x+i,max_x),y, max_x);
				else { // "rebounds"
					idx_in = ABS(x+i);
					idx_in = (idx_in>=max_x)?(max2x-idx_in):idx_in;
					idx_in = VECTORIZE( idx_in, y, max_x);
				}
				switch (data_type) {
					case GINT_ID:
						*(buf+idx) += *(vector+ABS(i)) * (gdouble) *(i_in+idx_in);
						break;
					case HF_TYPE_ID:
						*(buf+idx) += *(vector+ABS(i)) * (gdouble) *(hf_in+idx_in);
						break;
					case GDOUBLE_ID:
						*(buf+idx) += *(vector+ABS(i)) * (gdouble) *(d_in+idx_in);
						break;
					case UNSIGNED_CHAR_ID:
						*(buf+idx) += *(vector+ABS(i)) * (gdouble) *(us_in+idx_in);
						break;
					default:
						*(buf+idx) += *(vector+ABS(i)) * (gdouble) *(d_in+idx_in);
				}
			}
		}
	}
	// Summarize on the y axis
	for (y=0; y<max_y; y++) {
		for (x=0; x<max_x; x++) {
			idx = VECTORIZE(x,y,max_x);
			value = 0.0 ;

			for (i=-radius; i<=radius; i++) {
				if (wrap)
					idx_in = VECTORIZE(x,WRAP2(y+i,max_y), max_y);
				else { // "rebounds"
					idx_in = ABS(y+i);
					idx_in = (idx_in>=max_y)?(max2y-idx_in):idx_in;
					idx_in = VECTORIZE( x, idx_in, max_y);
				}
				value += *(vector+ABS(i)) * *(buf+idx_in);
			}
			switch (data_type) {
				case GINT_ID:
					*(i_out+idx) = (gint) value;
					break;
				case HF_TYPE_ID:
					*(hf_out+idx) = (hf_type) value;
					break;
				case GDOUBLE_ID:
					*(d_out+idx) = (gdouble) value;
					break;
				case UNSIGNED_CHAR_ID:
					*(us_out+idx) = (unsigned char) value;
					break;
				default:
					*(d_out+idx) = value;
			}
		}
	}

	x_free(buf);
}

void map_convolve (hf_type *map, gint map_max_x, gint map_max_y,
	hf_type *background, gint max_x, gint max_y, gint cx, gint cy,
	gboolean wrap, gint level, gdouble **gauss_list, gboolean square_symmetry) {

	// Smoothing (normalized convolution) of a square subset of the image "background"
	// The map is typically a stroke dot from the pen
	// whose values are used as relative smoothing radii
	// The map is centered at (cx,cy) in the background
	// The map and the background don't have to be square
	// Level is % of maximum radius
	// The result is "local smoothing"
	// The gaussian vectors generated are cached in gauss_list, for performance reasons

	gint x,y,i, idxbuf, idxmap, idxin, idxout, max2x, max2y, 
		sx,sy, radius, radius_x, radius_y, mx, my, lx, ix, iy;
	// guint buf_HEAP_index=0xFFFF, background_HEAP_index=0xFFFF;
	gdouble *buf,value,*vector;
	gfloat ratio; // Ratio used to transform map values to radii
	ratio = (gfloat) ( ((gfloat) level) * ((gfloat) GAUSS_LIST_LENGTH) / ((gfloat) MAX_HF_VALUE) / 100.0);
//	printf("MAP_CONVOLVE_RATIO: %8.6f\n",ratio);
	// We prepare a buffer of the region to smooth
	// The buffer must range from (x or y)-(map_size>>1) to (x or y)+(map_size>>1)
	buf = (gdouble *) x_malloc(sizeof(gdouble)*map_max_x*map_max_y, "gdouble (buf in map_convolve)");
	max2x = 2*map_max_x-1;
	max2y = 2*map_max_y-1;
	// Control of the square symmetry (when the map contains only one quadrant)
	// lx: actual length of one line of the map
	radius_x = (map_max_x>>1);
	radius_y = (map_max_y>>1);
	if (square_symmetry)
		lx = radius_x + 1;
	else
		lx = map_max_x;
	// Start point in the background
	sy = cy - (map_max_y>>1);
	sx = cx - (map_max_x>>1);
	// Summarize along the x axis
	// (x,y) are indexes in the map
	// From the map standpoint, the convolution matrix always "rebounds" on the boundaries
	// (the input - index is "idxin")
	// From the background standpoint, the map wraps over the boundaries
	// when "wrap" is TRUE, or the map overflow is lost in a blackhole if "wrap" is FALSE
	// (the output - index is "idxout")

	for (y=0; y<map_max_y; y++) {
		for (x=0; x<map_max_x; x++) {
			idxbuf = VECTORIZE(x,y,map_max_x);
			*(buf+idxbuf) = 0.0;
			if (square_symmetry) { // Only one quadrant is defined in the map
				if (x>radius_x)
					ix = 2* radius_x - x;
				else
					ix = x;
				if (y>radius_y)
					iy = 2* radius_y - y;
				else
					iy = y;
				idxmap = VECTORIZE(ix,iy,lx);
			}
			else
				idxmap = idxbuf;
			// Find the current radius
			radius = (gint) (ratio * *(map+idxmap));
//			if (!radius)
//				continue;
// printf("RADIUS for (%d,%d,%d): %d\n",x,y,*(map+idxmap),radius);
			if (!radius) {
				*(buf+idxbuf) = (gdouble)*(background+VECTORIZE(WRAP2(sx+ABS(x),max_x),
					WRAP2(sy+y,max_y),max_x));
				continue;
			}
			if (!gauss_list[radius])
				gauss_list[radius] = normalized_bell_new(radius);
			vector = gauss_list[radius];
			for (i=-radius; i<=radius; i++) {
				// Find the index of the value to add at this step
				// The index is always in the "window" delimited by the map
				// Overflows rebound
				// (mx,y) is the relative pixel to use in the map "window"
				// (sx+mx, sy+y) would be the absolute address of the pixel, in the background
				// In this set of loops, we run along the X axis, so Y won't overflow here
				mx = ABS(x+i);
				mx = (mx>=map_max_x)?(max2x-mx):mx;
				idxin = VECTORIZE(WRAP2(sx+mx,max_x),WRAP2(sy+y,max_y),max_x);
				*(buf+idxbuf) += *(vector+ABS(i)) * (gdouble) *(background+idxin);
			}
		}
	}

	// Summarize along the y axis
	for (y=0; y<map_max_y; y++) {
		for (x=0; x<map_max_x; x++) {
			if (square_symmetry) { // Only one quadrant is defined in the map
				if (x>radius_x)
					ix = 2* radius_x - x;
				else
					ix = x;
				if (y>radius_y)
					iy = 2* radius_y - y;
				else
					iy = y;
				idxmap = VECTORIZE(ix,iy,lx);
			}
			else
				idxmap = VECTORIZE(x,y,map_max_x);
			value = 0.0 ;
			// Find the current radius
			radius = (gint) (ratio * *(map+idxmap));
			if (!radius) {
				value = *(buf+VECTORIZE(x,y,map_max_y));
			}
			else {
			// Obviously we don't need to check the existence of the vector for this radius!
				vector = gauss_list[radius];
				for (i=-radius; i<=radius; i++) {
					// (x,my) is the relative pixel to use in the map "window"
					// (sx+x, sy+my) would be the absolute address of the pixel, in the background
					my = ABS(y+i);
					my = (my>=map_max_y)?(max2y-my):my;
					// We take the buffer as input now
					idxin = VECTORIZE(x,my, map_max_y);
					value += *(vector+ABS(i)) * *(buf+idxin);
				}
			}
			// Here we drop outbound values in the nothingness if wrap is FALSE
			if (wrap)
				idxout = VECTORIZE(WRAP2(sx+x,max_x),WRAP2(sy+y, max_y), max_y);
			else {
				if ( ((sx+x)<0) || ((sy+y)<0) || ((sx+x)>=max_x) || ((sy+y)>=max_y) )
					continue;
				else
					idxout = VECTORIZE(sx+x,sy+y,max_y);
			}
			*(background+idxout) = (hf_type) value;
		} // end for(x...)
	} // end for(y...)
	x_free(buf);
}

gboolean intersect_rect (gint xmin, gint ymin, gint xmax, gint ymax,
	gint x0, gint y0, gint x1, gint y1,
	gdouble *startx, gdouble *starty, gdouble *endx, gdouble *endy) {

	// Extend the line (x0,y0) -> (x1,y1) so that it intersects
	// the rectangle (xmin,ymin) -> (xmax,ymax)
	// Put the result in (startx, starty) and (endx, endy)
	// Adapted from Liang-Barsky clipping line algorithm (Foley - van Dam)

	// IMPORTANT: xmax and ymax are not sizes, but actual indexes in base 0
	gdouble t[4], tn, tp, dx, dy;
	gint i=3;
	static gdouble epsilon=0.1,MAXPOS=1000000.0, MAXNEG=-1000000.0 ;

	tn = MAXNEG;
	tp = MAXPOS;

	// P0 and P1 must be in the rectangle
	if ( (x0<0) || (x0>xmax) || (y0<0) || (y0>ymax) ||
		(x1<0) || (x1>xmax) || (y1<0) || (y1>ymax) )
		return FALSE;
//	printf ("RECT: (%d,%d) - (%d,%d); LINE: (%d,%d) - (%d,%d); \n",
//	xmin,ymin,xmax,ymax, x0, y0, x1, y1);

	dx = (gdouble) (x1-x0);
	dy = (gdouble) (y1-y0);

	if (ABS(dx)<epsilon) {
		t[0] = MAXNEG;
		t[1] = MAXPOS;
	}
	else {
		t[0] = - ((gdouble) (x0 - xmin)) / dx; // Left
		t[1] = ((gdouble) (x0 - xmax)) / -dx; // Right
	}
	if (ABS(dy)<epsilon) {
		t[2] = MAXNEG;
		t[3] = MAXPOS;
	}
	else {
		t[2] = - ((gdouble) (y0-ymin)) / dy; // Bottom
		t[3] = ((gdouble) (y0-ymax)) / -dy; // Top
	}

// 	printf("T[0]: %5.2f; T[1]: %5.2f; T[2]: %5.2f; T[3]: %5.2f; \n",t[0],t[1],t[2],t[3]);
	// We retain the smallest negative T and the smallest positive (nearest from P0)
	// then we compute the points from the parametric formula of P0...P1
	// (since P0 and P1 are always inside the rectangle)

	for (i=0; i<4; i++) {
		if (t[i]>0) {
			if (tp>t[i])
				tp = t[i];
		}
		else {
			if (tn<t[i])
				tn = t[i];
		}
	}
// 	printf("TN: %5.2f; TP: %5.2f\n",tn,tp);

	if ((tn==MAXNEG) || (tp==MAXPOS))
		return FALSE; // It shoudn't happen!

	*startx = ((gdouble) x0) + dx *tn;
	*starty = ((gdouble) y0) + dy * tn;
	*endx =  ((gdouble) x0) + dx * tp;
	*endy = ((gdouble) y0) + dy * tp;

	return TRUE;
}

gpointer hexagonal_row_sampling_with_type (gpointer hf, gint hf_size, 
	gboolean wrap, gboolean even, gboolean shift_right, gint data_type, gint axis) {
	// Creates a new hf map, translating each odd or even row by 0.5 pixel
	// This gives a rough approximation of a hexagonal tiling
	// This could improve some transformations, like the erosion
	// We use a very rough approximation, for now (average)
	// If (even), we modify the even rows, otherwise we modify the odd rows
	// If (shift_right), the row is translated to the right
	// (we put in the current pixel half of its value plus half
	// of the value of the preceding pixel)
	// If (!shift_right), the row is translated to the left
	hf_type *hf_out;
	gint x, y, i, j, shift;
	gboolean test;
	if (shift_right)
		shift = -1;
	else
		shift = 1;
	switch (data_type) {
		case GINT_ID:
			hf_out = (gpointer) x_malloc(sizeof(gint)*hf_size*hf_size, "gpointer (hf_out in hexagonal_row_sampling_with_type)");
			break;
		case HF_TYPE_ID:
			hf_out = (gpointer) x_malloc(sizeof(hf_type)*hf_size*hf_size, "gpointer (hf_out in hexagonal_row_sampling_with_type)");
			break;
		case GDOUBLE_ID:
			hf_out = (gpointer) x_malloc(sizeof(gdouble)*hf_size*hf_size, "gpointer (hf_out in hexagonal_row_sampling_with_type)");
			break;
		case UNSIGNED_CHAR_ID:
			hf_out = (gpointer) x_malloc(sizeof(unsigned char)*hf_size*hf_size, "gpointer (hf_out in hexagonal_row_sampling_with_type)");
			break;
		default:
			printf("Unexpected data type in hexagonal_row_sampling_with_type\n");
			exit(0);
	}
	for (y=0; y<hf_size; y++) {
		test = !(y & (gint) 1); // test = TRUE if value is odd
		for (x=0; x<hf_size; x++) {
			if (axis==H_AXIS)
//				i = VECTORIZE(x+shift, y, hf_size);
				i = VECTORIZE( WRAP2( x + shift, hf_size), y, hf_size);
			else
//				i = VECTORIZE(y, x+shift, hf_size);
				i = VECTORIZE( y, (WRAP2( x + shift, hf_size)), hf_size);
			if ( (test && even) || ((!test) && (!even)) ) { //  XOR
				switch (data_type) {
					case GINT_ID:
						*(((gint *) hf_out)+i) = *(((gint *) hf)+i);
						break;
					case HF_TYPE_ID:
						*(((hf_type *) hf_out)+i) = *(((hf_type *) hf)+i);
						break;
					case GDOUBLE_ID:
						*(((gdouble *) hf_out)+i) = *(((gdouble *) hf)+i);
						break;
					case UNSIGNED_CHAR_ID:
						*(((unsigned char *) hf_out)+i) = *(((unsigned char *) hf)+i);
						break;
					default:
						printf("Unexpected data type in hexagonal_row_sampling_with_type\n");
						exit(0);
				}
				continue;
			}
			if (wrap) {
				if (axis==H_AXIS)
					j = VECTORIZE( WRAP2( x + shift, hf_size), y, hf_size);
				else
					j = VECTORIZE( y, (WRAP2( x + shift, hf_size)), hf_size);
			}
			else { // Repeat the pixel
				if (axis==H_AXIS)
					j =  VECTORIZE(MAX( MIN(hf_size, x + shift),0), y, hf_size);
				else
					j =  VECTORIZE(y, MAX( MIN(hf_size, x + shift),0), hf_size);
			}
			
			switch (data_type) {
				case GINT_ID:
					*(((gint *) hf_out)+i) = (gint) ((((glong) *(((gint *) hf)+i) ) + (glong) *(((gint *) hf)+j) )>>1 );
					break;
				case HF_TYPE_ID:
					*(((hf_type *) hf_out) + i) = (hf_type) ((((glong) *(((hf_type *) hf)+i) ) + (glong) *(((hf_type *) hf)+j) )>>1 );
					break;
				case GDOUBLE_ID:
					*(((gdouble *) hf_out)+i) = ( *(((gdouble *) hf)+i)  + *(((gdouble *) hf)+j) ) / 2.0 ;
					break;
				case UNSIGNED_CHAR_ID:
					*(((unsigned char *) hf_out)+i) = (unsigned char) ((((glong) *(((unsigned char *) hf)+i) ) + (glong) *(((unsigned char *) hf)+j) )>>1 );
					break;
				default:
					printf("Unexpected data type in hexagonal_row_sampling_with_type\n");
					exit(0);
			}
//			printf("SHIFTING (%d, %d) = i: %d; j: %d\n",x,y,i,j);
		}
	}
	return hf_out;
}

hf_type *hexagonal_row_sampling (hf_type *hf, gint hf_size, 
	gboolean wrap, gboolean even, gboolean shift_right) {

	return (hf_type *) hexagonal_row_sampling_with_type (hf, hf_size, wrap, even, shift_right, HF_TYPE_ID, V_AXIS);	
}

void add_spread_3x3 (hf_type *hf, gint max, gint x, gint y, gint value, gboolean wrap) {
	// Add value in hf at point(x,y), spreading the value given "matrix"
	// (3x3 convolution)
	static gdouble matrix[3][3]={ {0.5, 1.0, 0.5}, {1.0, 8.0, 1.0}, {0.5, 1.0, 0.5}};
//	static gdouble matrix[3][3]={{1.0, 2.0, 1.0}, {2.0, 4.0, 2.0}, {1.0, 2.0, 1.0}};
	static gdouble sum = 14.0; // Sum of the matrix
	gint i, m, n, xi, yi;
	gdouble qty;
	
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
			qty = (((gdouble)value) * matrix[m][n]) / sum;
			*(hf+i) = (hf_type) MIN(MAX_HF_VALUE,MAX(0,((gint) *(hf+i)) + (gint) qty));
		}
}

void hf_clamp_buffer (hf_type *buffer, gint length, hf_type newmin, hf_type newmax) {
//	"Clamp" values between newmin and newmax
//	buffer is considered as a 1D array
	gfloat ratio;
	gint i;
	hf_type vmin, vmax;
	vmin = *buffer;
	vmax = vmin;
//	We first need to know the max and min value of the HF
	for (i=0; i<length; i++) {
		if (vmin>*(buffer+i) )
			vmin = *(buffer+i);
		else
			if (vmax<*(buffer+i))
				vmax = *(buffer+i);
	}
	if (vmin == vmax)
		return;
	ratio = ((gfloat) ABS(newmax-newmin)) / (gfloat) (vmax - vmin);
	for (i=0; i<length; i++) {
		*(buffer+i) = newmin + (hf_type) (ratio * (gfloat) (*(buffer+i) - vmin) ) ;
	}
}

void hf_clamp (hf_struct_type *hf, hf_type newmin, hf_type newmax) {
//	"Clamp" values between newmin and newmax
//	Version using the 16 bits display buffer
	hf_clamp_buffer(hf->hf_buf, hf->max_x*hf->max_y, newmin, newmax);
}

void double_clamp (hf_type *hf_buf, gint max_x, gint max_y, hf_type newmin, hf_type newmax, gdouble *dbuf) {
	gdouble ratio, min, max=0.0, newmind;
	static gdouble maxd = (gdouble) MAX_HF_VALUE;
	gint i;
	newmind = (gdouble) newmin;
	min = maxd;
	for (i=0; i<max_x*max_y; i++) {
		if (*(dbuf+i)<min)
			min = *(dbuf+i);
		else
			if (*(dbuf+i)>max)
				max = *(dbuf+i);
//		if (!((i+(max_x>>1))%max_x))
//			printf("%p; @i= %d :: min=%f;max=%f\n",dbuf,i,min,max);
	}
	if (min == max)
		ratio = 0.0;
	else
		ratio = ((gdouble) ABS(newmax-newmin)) / (max - min);
//	printf("NEWMIND: %f; MIN: %f; MAX: %f; RATIO: %f\n",newmind, min,max,ratio);
	for (i=0; i< max_x*max_y; i++) {
		*(hf_buf+i) = (hf_type) ( (newmind + ratio * ( ( (gdouble) *(dbuf+i)) - min)));
	}
}

void hf_double_clamp (hf_struct_type *hf, hf_type newmin, hf_type newmax, gdouble *dbuf) {
//	Same as "hf clamp", but with a double float buffer as input
	double_clamp (hf->hf_buf, hf->max_x, hf->max_y, newmin, newmax, dbuf);
}

void hf_type_to_double_scaled (hf_type *hf_buf, gint max_x, gint max_y, gdouble *doublebuf) {
	//	Convert hf_buf to double precision data
	//	Values are scaled from 0 - MAX_HF_VALUE to 0.0 - 1.0
	//	doublebuf MUST be allocated
	static gdouble maxd = (gdouble) MAX_HF_VALUE;
	gint i;
	for (i=0; i<max_x*max_y; i++) {
		*(doublebuf+i) = ((gdouble) *(hf_buf+i)) / maxd;
	}
}
void hf_type_to_double (hf_type *hf_buf, gint max_x, gint max_y, gdouble *doublebuf) {
	//	Convert hf_buf to double precision data
	//	doublebuf MUST be allocated
	gint i;
	for (i=0; i<max_x*max_y; i++) {
		*(doublebuf+i) = (gdouble) *(hf_buf+i);
	}
}

void hf_subtract(hf_type *hf1, hf_type *hf2, hf_type *result, gint length, gint behaviour) {
	// Simple subtraction of hf2 from hf1, in result
	// Already allocated buffers
	// behaviour is OVERFLOW_ZERO, OVERFLOW_REBOUND (absolute value) or
	// OVERFLOW_OFFSET (set the negative minimum value to 0)
	// OVERFLOW_SCALE (set the negative minimum value to 0, and scale down
	// if required)
	gint i;
	glong min, max, *array = NULL;
	gdouble ratio;
	if ((!hf1) || (!hf2) || (!result))
		return;
	if ( (behaviour==OVERFLOW_OFFSET) || (behaviour==OVERFLOW_SCALE) ) {
		// Calculate the minimum and maximum
		array = (glong*) x_malloc(sizeof(glong)*length, "glong (array in hf_subtract)");
		min = max = (*hf1) - *hf2;
		for (i=1; i<length; i++) {
			*(array+i) = ((glong) *(hf1+i)) - (glong) *(hf2+i);
			if (*(array+i)<min)
				min = *(array+i);
			else
				if (*(array+i)>max)
					max = *(array+i);
		}
		if ((max-min) < (glong) MAX_HF_VALUE)
			ratio = 1.0;
		else
			ratio = ((gdouble) MAX_HF_VALUE) / (gdouble) (max-min); 
//		printf("MIN: %ld; MAX: %ld; Ratio: %7.4f\n",min,max,ratio);
	}
	else {
		return;
	}
	for (i=0; i<length; i++) {
		switch (behaviour) {
			case OVERFLOW_ZERO:
				*(result+i) = (hf_type) MAX(0 , ((glong) *(hf1+i)) - (glong) *(hf2+i));
				break;
			case OVERFLOW_REBOUND:
				*(result+i) = (hf_type) ABS(((glong) *(hf1+i)) - (glong) *(hf2+i));
				break;
			case OVERFLOW_OFFSET:
				*(result+i) = (hf_type) (*(array+i) - min);
				break;
			case OVERFLOW_SCALE:
				*(result+i) = (hf_type) (((gdouble) (*(array+i) - min)) * ratio);
		}
	}
	if (array)
		x_free (array);
}

