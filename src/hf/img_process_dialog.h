/* img_process_dialog.h - headers for image dialog class - filters and other transformations
 *
 * Copyright (C) 2001 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         http://geomorph.sourceforge.net
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

#ifndef _IMGPROCESS_DIALOG
#define _IMGPROCESS_DIALOG 1

#include <gtk/gtk.h>
#include "../utils/menus_n_tools.h"
#include "globals.h"
#include "img_process.h"
#include "hf_filters_dialog.h"
#include "waves_dialog.h"
#include "waves.h"
#include "merge_dialog.h"
#include "erode_dialog.h"
#include "wind.h"
#include "wind_dialog.h"
#include "geom_transform_dialog.h"
#include "craters.h"
#include "erode.h"
#include "erode_dialog.h"
#include "voronoi.h"
#include "voronoi_dialog.h"

typedef struct {
	GtkWidget *img_dialog;		// Master dialog (a toolbar)
	GtkWidget *current_subdialog;	// Could be any of the next
	void (*set_fn) (gpointer);		// Sets default values for the current dialog
					// For use in accept / reset callbacks
	GtkWidget *accept_wdg;		// Current accept button, 
					// for use in accept / reset callbacks

// Brightness - contrast
	GtkWidget *br_contr_dialog;
	gint brightness_level;
	gint contrast_level;
	GtkObject *adj_brightness;
	GtkObject *adj_contrast;
	GtkWidget *overflow_radio_button;
	gboolean brightness_bound_overflow;
	GtkWidget *contrast_keep_luminosity_check_box;
	GtkWidget *br_contr_accept;

// Smooth
	GtkObject *adj_smooth_radius;
	GtkObject *adj_smooth_box;
	GtkWidget *smooth_dialog;
	gint smooth_radius;
	gboolean smooth_direct;
	// Merge struct. containing operation (+,*,.-...) and parameters
	merge_dialog_struct *smooth_merge;
	GtkWidget *smooth_accept;
	gint smooth_wrap;

// Sharpen (increase noise) 
	GtkObject *adj_sharpen_radius;
	GtkObject *adj_sharpen_level;
	GtkWidget *sharpen_dialog;
	gint sharpen_radius;
	gdouble sharpen_level;
	// Merge struct. containing operation (+,*,.-...) and parameters
	merge_dialog_struct *sharpen_merge;
	GtkWidget *sharpen_accept;
	gint sharpen_wrap;

// Slider
	GtkWidget *slide_dialog;
	GtkObject *adj_slidev;
	GtkObject *adj_slideh;
	GtkWidget *slide_accept;
	gint slidev;	// From 0 to 100 %
	gint slideh;

// Rotate
	GtkWidget *rotate_dialog;
	GtkObject *adj_rotate;
	gint angle;
	gint rotate_overflow_tiling ;
	gint rotate_overflow_notiling ;
	GtkWidget *rotate_accept;
	GtkWidget *auto_rotate;

// Terraces
	GtkWidget *terrace_dialog;
	gint terrace_levels;
	GtkObject *adj_terrace_levels;
	gint terrace_seed;
	gint terrace_percent_random;
	GtkObject *adj_terrace_percent_random;
	// Merge struct. containing operation (+,*,.-...) and parameters
	merge_dialog_struct *terrace_merge;
	// Smooth parameters, applied on the result
	gint terrace_smooth_radius;
	GtkObject *adj_terrace_smooth_radius;
	gint terrace_wrap;
	// Artifact removal
	gint terrace_artifact_radius;
	GtkObject *adj_terrace_artifact_radius;
	// Flag controlling if we apply smooth or artifact removal
	gboolean terrace_apply_postprocess;
	GtkWidget *terrace_accept;

// Threshold (cut off)
	GtkWidget *threshold_dialog;
	GtkObject *adj_threshold_min;
	hf_type threshold_min;
	GtkObject *adj_threshold_max;
	hf_type threshold_max;
	GtkObject *adj_threshold_percent;
	gint threshold_percent;
	GtkWidget *threshold_accept;

// Revert
	GtkWidget *revert_dialog;

// Cityscape
	GtkWidget *city_dialog;
	GtkObject *adj_skyscraper_width;
	gint skyscraper_width;
	GtkObject *adj_skyscraper_width_var;
	gint skyscraper_width_var;	// In %, upper boundary of random skyscraper width var.
	GtkObject *adj_streets_width;
	gint streets_width;		// In % of skyscrapers width, from 0 to 50
	GtkWidget *cityscape_accept;
	merge_dialog_struct *cityscape_merge;

// Precompiled waves
	shape_type* wav_shapes[NBWAVES];

// Linear & circular waves
	GtkWidget *wav_dialog;		// A frame containing a notebook with nb_lin_wav pages
					// plus control buttons (new / delete / reset...)
	GtkNotebook *wav_notebook;	// The child notebook
	gint nb_wav;
	GList *wav_pages;		// A list of wave_page_struct
	GList *wav_data;			// A list of wave_struct
	GtkWidget *wav_accept;
	gboolean wav_direct;
	gboolean wav_to_calc;	// FALSE if the amplitude widgets must be inactivated
						// Then we are committing the changes and
						// we don't want the callbacks to be reexecuted

// Shape filter
	filter_dialog_struct *fd_struct;
	GtkWidget *filter_accept;
// Noise
	GtkWidget *noise_dialog;
	GtkWidget *noise_accept;
	GtkObject *adj_noise_level;
	gint noise_level;
	gint noise_merge;
	gpointer *hf_noise;	// hf_wrapper_struct *
	hf_struct_type *noise_to_apply;

// Stretch-compress
	GtkWidget *stretch_dialog;
	GtkWidget *stretch_accept;
	
// Hexagonal quantization (Giant Causeway)
	GtkWidget *hexagon_dialog;
	gint hexagon_radius;
	GtkObject *adj_hexagon_radius;
	gint hexagon_border;
	GtkObject *adj_hexagon_border;
	// Smooth parameters, applied on the result
	gint hexagon_smooth_radius;
	GtkObject *adj_hexagon_smooth_radius;
	gboolean hexagon_apply_postprocess;
	merge_dialog_struct *hexagon_merge;
	GtkWidget *hexagon_accept;

// Mirrors
	GtkWidget *mirror_horizontal_dialog;
	GtkWidget *mirror_vertical_dialog;

// Generic merge
	// Merge struct. containing operation (+,*,.-...) and parameters
	GtkWidget *merge_dialog;
	merge_dialog_struct *merge;
	GtkWidget *merge_accept;
	hf_struct_type *hf_to_merge;
	gboolean merge_scaled;
	hf_struct_type *hf_to_merge_scaled;
	GtkWidget *merging_image_frame;
	img_listbox_struct *merge_listbox;
//	gpointer hfw_merge_display;
	view_struct *merge_preview;

// Mathematical transformations (power, log, etc.)
	GtkWidget *math_fn_dialog;
	GtkWidget *math_fn_accept;
	gint math_fn;
	gdouble math_fn_param;
	GtkObject *adj_math_fn;
	
// Lift edges (for instance, for emulating drying mud)

	GtkWidget *lift_edges_dialog;
	GtkWidget *lift_edges_accept;
	gint lift_edges_radius;
	GtkObject *adj_lift_edges_radius;
	gint lift_edges_level;
	GtkObject *adj_lift_edges_level;
	GtkWidget *lift_edges_level_dialog;
	gint lift_edges_wrap;
	gboolean lift_edges_merge_done;
	gboolean lift_edges_use_black_point;
	
// Crack networks

	voronoi_struct *voronoi;
	voronoi_dialog_struct *voronoi_dialog;
	
// Structures for erosion processes

	rain_erosion_struct *rain_erosion;
	rain_erosion_dialog_struct *rain_erosion_dialog;
	
	gravity_struct *gravity;
	gravity_dialog_struct *gravity_dialog;
	
	oriented_gravity_struct *oriented_gravity;
	oriented_gravity_dialog_struct *oriented_gravity_dialog;
	
	gravity_struct *crests;
	gravity_dialog_struct *crests_dialog;
	
	whimsical_erosion_struct *whimsical_erosion;
	whimsical_erosion_dialog_struct *whimsical_erosion_dialog;
	
	craters_erosion_struct *craters_erosion;
	craters_erosion_dialog_struct *craters_erosion_dialog;
	
// Wind (ripples, dunes)

	wind_dialog_struct *ripples_dialog;
	wind_struct *ripples;
	
	wind_dialog_struct *dunes_dialog;
	wind_struct *dunes;

} img_dialog_struct;

img_dialog_struct *img_struct_new(GtkWidget *img_dialog, gpointer parent_data);

void img_dialog_free(img_dialog_struct *img);

GtkWidget* img_process_dialog_new (GtkWidget *window, GtkTooltips *tooltips, gpointer);

GtkWidget *br_contr_dialog_new(img_dialog_struct *img, gpointer data) ;

void brightness_contrast_callb(GtkWidget *wdg, gpointer data);
// void contrast_callb(GtkWidget *wdg, gpointer data);
void smooth_callb(GtkWidget *wdg, gpointer data);
void sharpen_callb(GtkWidget *wdg, gpointer data);
void terrace_callb(GtkWidget *wdg, gpointer data);
void threshold_callb(GtkWidget *wdg, gpointer data);
void revert_callb(GtkWidget *wdg, gpointer data);
void city_callb(GtkWidget *wdg, gpointer data);
void filter_callb(GtkWidget *wdg, gpointer data);
void noise_callb(GtkWidget *wdg, gpointer data);
void dilate_callb(GtkWidget *wdg, gpointer data);
void mirror_vertical_callb(GtkWidget *wdg, gpointer data);
void mirror_horizontal_callb(GtkWidget *wdg, gpointer data);
void merge_callb(GtkWidget *wdg, gpointer data);
void math_fn_callb(GtkWidget *wdg, gpointer data);
void hexagon_callb(GtkWidget *wdg, gpointer data);
void lift_edges_callb(GtkWidget *wdg, gpointer data);

//	These prototypes should have been included in waves_dialog.h
//	but this gives circular definitions (since waves_dialog.h would require img_process_dialog.h)
//	Some design problem to think about in the future...
void wave_page_new (	img_dialog_struct *img, 
				gint axis,
				GtkWidget *tools_window,
				GtkTooltips *tooltips,
				gpointer data);
void set_wave_defaults( img_dialog_struct *img);

void accept_callb(GtkWidget *wdg, gpointer data);
void reset_callb(GtkWidget *wdg, gpointer data);
GtkWidget *reset_accept_buttons_new (gpointer data, GtkWidget **accept_slot);
GtkWidget *apply_repeat_buttons_new (gpointer data,
	void (*apply_fn) (GtkWidget *, gpointer), 
	void (*repeat_fn) (GtkWidget *, gpointer) );
void set_br_contr_defaults(img_dialog_struct *img) ;

#endif // _IMGPROCESS_DIALOG

