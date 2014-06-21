/* img_process.h - headers for image processing filters
 *
 * Copyright (C) 2001 Patrice St-Gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _IMGPROCESS
#define _IMGPROCESS 1

#include "globals.h"
#include "hf.h"
#include "hf_calc.h"
#include "merge_dialog.h"

// "Damping function" for "lift edges" (for emulating surface shrinking)

#ifndef LINEAR
#define LINEAR 0
#endif

#ifndef QUADRATIC
#define QUADRATIC 1
#endif

#ifndef EXPONENTIAL
#define EXPONENTIAL 2
#endif

#ifndef LIFT_EDGES_MAX
#define LIFT_EDGES_MAX 0x4000
#endif

// Convolution function IDs

#ifndef BOX_FUNC
#define BOX_FUNC 0
#endif

#ifndef TENT_FUNC
#define TENT_FUNC 1
#endif

#ifndef POW2_FUNC
#define POW2_FUNC 2
#endif

#ifndef EXP_FUNC
#define EXP_FUNC 3
#endif

#ifndef DILATE
#define DILATE 10
#endif

#ifndef ERODE
#define ERODE 20
#endif

void hf_brightness_contrast(	hf_struct_type *hf, 
				gint contrast_level, 
				gint brightness_level,
				gboolean keep_luminosity,
				gboolean bound_overflow);
void hf_auto_contrast (hf_struct_type *hf);
void hf_terrace (hf_struct_type *hf, gint levels, gint seed, gint percent_random,
	gint smooth_radius, gboolean if_wrap, 
	dist_matrix_struct *dist_matrix,
	gint artifact_radius, gboolean apply_postprocess,
	gdouble **gauss_list);
void hf_threshold (hf_struct_type *hf, hf_type min, hf_type max, gint percent);
void hf_smooth (hf_struct_type *hf, gint radius, dist_matrix_struct *, gboolean wrap, gdouble **gauss_list);
void smooth_buf (gpointer buf, gint max_x, gint max_y, gint radius, dist_matrix_struct *dm, gboolean wrap, gdouble **gauss_list, gint data_type);
void hf_city(hf_struct_type *hf, gint skycraper_width, gint skyscraper_var, gint streets_width);
void hf_simple_blur(hf_struct_type *hf, gint radius);
void hf_stretch_v(hf_struct_type *hf);
void hf_stretch_h(hf_struct_type *hf);
void hf_convolve (hf_struct_type *hf, gint radius, gdouble *matrix, gboolean if_tiles);
void hf_erode (hf_struct_type *hf, gint steps);
void hf_remove_artifacts (hf_struct_type *hf, gint radius, dist_matrix_struct *dm);

void hf_hexagon (hf_struct_type *hf, gint radius, gint border,
	gint smooth_radius, 
	dist_matrix_struct *dist_matrix,
	gboolean apply_postprocess,
	gdouble **gauss_list);

void hf_math_fn (hf_struct_type *hf, gint math_op, gdouble parameter);

gdouble *create_gaussian_kernel (gint radius, gdouble base, gdouble exponent, gdouble value_offset, gdouble scale, gboolean not_inverted);
gdouble *create_hat_kernel (gint radius, gdouble relative_value_offset, gdouble scale, gboolean not_inverted);
gdouble *create_pixel_kernel (gint radius, gdouble relative_value_offset, gdouble scale, gboolean not_inverted);
gdouble *create_tent_kernel (gint radius, gdouble value_offset, gdouble scale, gboolean not_inverted);

void merge_lift_edges (hf_struct_type *hf, gint level);

void lift_edges (hf_type *in, hf_type *out, gint max_x, gint max_y, gint radius,
	dist_matrix_struct *dm, gboolean wrap,gdouble **gauss_list, gboolean use_black_point);

hf_type *build_noise_map (gdouble wave_length_in_percent, gint max_x, gint max_y, dist_matrix_struct *dm, gdouble **gauss_list);

void convolve (gpointer buf, gint max_x, gint max_y, gint data_type, gint radius, gdouble *matrix, gboolean if_tiles) ;

void dfind_maximum (gdouble *in, hf_type *output, gint max_x, gint max_y, hf_type foreground_value, hf_type background_value) ;
void find_maximum (hf_type *in, hf_type *output, gint max_x, gint max_y, hf_type foreground_value, hf_type background_value) ;

void spread_over_max (hf_type *input, hf_type *output, gint max_x, gint max_y, gboolean wrap, gint min_width, gint max_width, hf_type min_val, hf_type max_val, gint function);

void min_max_erode (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gint steps, gboolean wrap);
void min_max_spread (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gint steps, gboolean wrap);
void erosion (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gboolean wrap);
void dilation (hf_type *hf_in, hf_type *hf_out, gint max_x, gint max_y, gboolean wrap);

void improve_edges (hf_type *hf_in,hf_type *hf_out,gint max_x, gint max_y, gboolean wrap, hf_type threshold);

void hf_integrate (hf_struct_type *hf, gint angle);

void hf_cut_graph (hf_struct_type *hf, gint index, gint axis);

void hf_histogram (hf_struct_type *hf);

gboolean histogram (hf_struct_type *hf, guint width, guint height, guchar *output);

#endif // _IMGPROCESS

