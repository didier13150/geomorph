/* img_process_dialog.c - filter and other transformations dialog class
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

#include "img_process_dialog.h"
#include "wind.h"
#include "wind_dialog.h"
#include "hf_wrapper.h"
#include "hf_creation_dialog.h" // For some HF generation dialogs
#include "hf_filters.h"
#include "subdiv2.h"
#include "time.h"
#include "craters.h"
#include "dialog_utilities.h"
#include "../fourier/fft.h"
#include "voronoi.h"
#include "voronoi_dialog.h"

#include "../icons/brightness_contrast.xpm"
#include "../icons/smooth.xpm"
#include "../icons/sharpen.xpm"
#include "../icons/threshold.xpm"
#include "../icons/revert.xpm"
#include "../icons/terraces.xpm"
#include "../icons/city.xpm"
#include "../icons/waves.xpm"
#include "../icons/rotate.xpm"
#include "../icons/slide.xpm"
#include "../icons/filters.xpm"
#include "../icons/noise.xpm"
#include "../icons/stretch_compress.xpm"
// #include "../icons/dilate.xpm"
// #include "../icons/erode.xpm"
#include "../icons/mirror_horizontal.xpm"
#include "../icons/mirror_vertical.xpm"
#include "../icons/merge.xpm"
#include "../icons/eroded_ribs.xpm"
#include "../icons/math_fn.xpm"
#include "../icons/crests.xpm"
#include "../icons/craters.xpm"
#include "../icons/rain.xpm"
#include "../icons/honeycomb.xpm"
#include "../icons/oriented_gravity.xpm"
#include "../icons/gravity.xpm"
#include "../icons/dunes.xpm"
#include "../icons/ripples.xpm"
#include "../icons/lift_edges.xpm"
#include "../icons/voronoi.xpm"

#define NBPROCESSES 31
command_item_struct img_processes[NBPROCESSES] = {
{ "Processes", "Brightness and contrast","Brightness and contrast" ,0, (gchar **) brightness_contrast_xpm, GDK_LEFT_PTR, brightness_contrast_callb,NULL, NULL,TRUE },
{ "Processes", "Threshold", "Threshold",0, (gchar **) threshold_xpm, GDK_LEFT_PTR, threshold_callb, NULL, NULL,TRUE },
{ "Processes", "Revert", "Revert",0, (gchar **) revert_xpm, GDK_LEFT_PTR, revert_callb,NULL, NULL,TRUE },
{ "Processes", "Mathematical transformations", "Mathematical transformations",0, (gchar **) math_fn_xpm, GDK_LEFT_PTR, math_fn_callb, NULL, NULL,TRUE },
{ "Processes", "City", "City",0, (gchar **) city_xpm, GDK_LEFT_PTR, city_callb,NULL, NULL,TRUE },
{ "Processes", "Giant Causeway", "Giant Causeway",0, (gchar **) honeycomb_xpm, GDK_LEFT_PTR, hexagon_callb,NULL, NULL,TRUE },
{ "Processes", "Terraces", "Terraces",0, (gchar **) terraces_xpm, GDK_LEFT_PTR, terrace_callb,NULL, NULL,TRUE },
{ "Processes", 	  NULL, NULL, 0, NULL, 0, NULL,NULL, NULL, FALSE },	// Separator
{ "Processes", "Smooth", "Smooth",0, (gchar **) smooth_xpm, GDK_LEFT_PTR, smooth_callb, NULL, NULL,TRUE },
{ "Processes", "Sharpen", "Sharpen (increase noise)",0, (gchar **) sharpen_xpm, GDK_LEFT_PTR, sharpen_callb, NULL, NULL,TRUE },
{ "Processes", "Translate", "Translate",0, (gchar **) slide_xpm, GDK_LEFT_PTR, slide_callb,NULL, NULL,TRUE },
{ "Processes", "Rotate", "Rotate",0, (gchar **) rotate_xpm, GDK_LEFT_PTR, rotate_callb,NULL, NULL,TRUE },
{ "Processes", "Stretch", "Stretch and compress",0, (gchar **) stretch_compress_xpm, GDK_LEFT_PTR, stretch_compress_callb,NULL, NULL,TRUE },
{ "Processes", "Horizontal mirror", "Horizontal mirror",0, (gchar **) mirror_horizontal_xpm, GDK_LEFT_PTR, mirror_horizontal_callb,NULL, NULL,TRUE },
{ "Processes", "Vertical mirror", "Vertical mirror",0, (gchar **) mirror_vertical_xpm, GDK_LEFT_PTR, mirror_vertical_callb,NULL, NULL,TRUE },
{ "Processes", 	  NULL, NULL, 0, NULL, 0, NULL,NULL, NULL, FALSE },	// Separator
{ "Processes", "Noise", "Add noise",0, (gchar **) noise_xpm, GDK_LEFT_PTR, noise_callb, NULL, NULL,TRUE },
{ "Processes", "Merge", "Merge",0, (gchar **) merge_xpm, GDK_LEFT_PTR, merge_callb, NULL, NULL,TRUE },
{ "Processes", "Shape filter", "Shape filter",0, (gchar **) filters_xpm, GDK_LEFT_PTR, filter_callb, NULL, NULL,TRUE },
{ "Processes", "Crests", "Crests",0, (gchar **) crests_xpm, GDK_LEFT_PTR, crests_callb,NULL, NULL,TRUE },
{ "Processes", "Rain erosion", "Rain erosion",0, (gchar **) rain_xpm, GDK_LEFT_PTR, rain_erosion_callb,NULL, NULL,TRUE },
{ "Processes", "Whimsical erosion", "Whimsical erosion",0, (gchar **) eroded_ribs_xpm, GDK_LEFT_PTR, whimsical_erosion_callb,NULL, NULL,TRUE },
{ "Processes", "Craters", "Craters",0, (gchar **) craters_xpm, GDK_LEFT_PTR, craters_erosion_callb,NULL, NULL,TRUE },
{ "Processes", 	  NULL, NULL, 0, NULL, 0, NULL,NULL, NULL, FALSE },	// Separator
{ "Processes", "Gravity erosion", "Gravity erosion",0, (gchar **) gravity_xpm, GDK_LEFT_PTR, gravity_callb,NULL, NULL,TRUE },
{ "Processes", "Oriented gravity erosion", "Oriented gravity erosion",0, (gchar **) oriented_gravity_xpm, GDK_LEFT_PTR, oriented_gravity_callb,NULL, NULL,TRUE },
{ "Processes", "Dunes", "Dunes",0, (gchar **) dunes_xpm, GDK_LEFT_PTR, dunes_callb,NULL, NULL,TRUE },
{ "Processes", "Ripples", "Ripples",0, (gchar **) ripples_xpm, GDK_LEFT_PTR, ripples_callb,NULL, NULL,TRUE },
{ "Processes", "Waves", "Waves",0, (gchar **) waves_xpm, GDK_LEFT_PTR, waves_callb,NULL, NULL,TRUE },
{ "Processes", "Edges", "Lift edges",0, (gchar **) lift_edges_xpm, GDK_LEFT_PTR, lift_edges_callb,NULL, NULL,TRUE },
{ "Processes", "Cracks", "Cracks network",0, (gchar **) voronoi_xpm, GDK_LEFT_PTR, voronoi_callb,NULL, NULL,TRUE }
};

//  Toolbar for terraces levels
//  Finally not used... keeping it for doc purpose....
/*
#define NBLEVELS 7
command_item_struct terrace_levels[NBLEVELS] = {
"Levels", "65536", "65536", 0, NULL, GDK_LEFT_PTR, terrace_upd,NULL, NULL,TRUE,
"Levels", "2", "2", 0, NULL, GDK_LEFT_PTR, terrace_upd,NULL, NULL,FALSE,
"Levels", "4", "4", 0, NULL, GDK_LEFT_PTR, terrace_upd,NULL, NULL,FALSE,
"Levels", "8", "8", 0, NULL, GDK_LEFT_PTR, terrace_upd,NULL, NULL,FALSE,
"Levels", "16", "16", 0, NULL, GDK_LEFT_PTR, terrace_upd,NULL, NULL,FALSE,
"Levels", "32", "32", 0, NULL, GDK_LEFT_PTR, terrace_upd,NULL, NULL,FALSE,
"Levels", "64", "64", 0, NULL, GDK_LEFT_PTR, terrace_upd,NULL, NULL,FALSE
};
*/

/************************* CLASS CONSTRUCTOR / DESTRUCTOR *************************/

img_dialog_struct *img_struct_new (GtkWidget *img_dialog, gpointer parent_data) {
	gint i;
	img_dialog_struct *img;
	img = (img_dialog_struct *) x_calloc(sizeof(img_dialog_struct),1, "img_dialog_struct");
	img->img_dialog = img_dialog;
	img->set_fn = NULL;
	img->accept_wdg = NULL;
	img->current_subdialog = NULL;
	
	img->br_contr_dialog = NULL;
	img->br_contr_accept = NULL;
	img->brightness_level =0;
	img->contrast_level = 0;
	img->adj_brightness = NULL;
	img->adj_contrast = NULL;
	img->brightness_bound_overflow = TRUE;
	img->overflow_radio_button = NULL;
	img->contrast_keep_luminosity_check_box = NULL;
	
	img->smooth_dialog = NULL;
	img->smooth_merge = NULL;
	img->adj_smooth_radius = NULL;
	img->adj_smooth_box = NULL;
	img->smooth_radius = 0;
	img->smooth_direct = FALSE;
	img->smooth_accept = NULL;
	img->smooth_wrap = TILING_AUTO;	
	
	img->sharpen_dialog = NULL;
	img->sharpen_merge = NULL;
	img->adj_sharpen_radius = NULL;
	img->adj_sharpen_level = NULL;
	img->sharpen_radius = 0;
	img->sharpen_level = 7.5;
	img->sharpen_accept = NULL;
	img->sharpen_wrap = TILING_AUTO;
	
	img->rotate_dialog = NULL;
	img->adj_rotate = NULL;
	img->auto_rotate = NULL;
	img->rotate_overflow_tiling = OVERFLOW_WRAP;
	img->rotate_overflow_notiling = OVERFLOW_REBOUND;
	img->rotate_accept = NULL;
	img->angle = 0;
	
	img->slide_dialog = NULL;
	img->adj_slidev = NULL;
	img->slidev = 0;
	img->adj_slideh = NULL;
	img->slideh = 0;
	img->slide_accept = NULL;
	
	img->terrace_dialog = NULL;
	img->terrace_levels = 0;
	img->terrace_seed = rand();
	img->terrace_percent_random = 50;
	img->adj_terrace_levels = NULL;	
	img->adj_terrace_percent_random = NULL;
	img->terrace_smooth_radius = 0;
	img->adj_terrace_smooth_radius = NULL;
	img->terrace_wrap = TILING_AUTO;
	img->adj_terrace_artifact_radius = NULL;
	img->terrace_artifact_radius = 0;
	img->terrace_apply_postprocess = FALSE;
	img->terrace_accept = NULL;
	img->terrace_merge = NULL;
	
	img->threshold_dialog = NULL;
	img->adj_threshold_min = NULL;
	img->threshold_min = 0;
	img->adj_threshold_max = NULL;
	img->threshold_max = 0xFFFF;
	img->adj_threshold_percent = NULL;
	img->threshold_percent = 100;
	img->threshold_accept = NULL;
	
	img->revert_dialog = NULL;
	
	img->city_dialog = NULL;
	img->adj_skyscraper_width = NULL;
	img->skyscraper_width = 0;
	img->adj_skyscraper_width_var = NULL;
	img->skyscraper_width_var = 0;
	img->adj_streets_width = NULL;
	img->streets_width = 0;
	img->cityscape_merge = NULL;
	img->cityscape_accept = NULL;
	
	for (i=0; i<NBWAVES; i++) 
		img->wav_shapes[i] = (shape_type *) NULL;
	img->wav_dialog = NULL;
	img->wav_notebook = NULL;	
	img->nb_wav = 0;
	img->wav_pages = NULL;
	img->wav_data = NULL;	
	img->wav_accept = NULL;
	img->wav_direct = TRUE;
	img->wav_to_calc = TRUE;
	
	img->fd_struct = NULL;
	img->filter_accept = NULL;
	
	img->noise_dialog = NULL;
	img->noise_accept = NULL;
	img->noise_level = 0;
	img->noise_merge = ADD;
	img->hf_noise = NULL;	// hf_wrapper_struct *
	img->adj_noise_level = NULL;
	img->noise_to_apply = NULL;
	
	img->stretch_dialog = NULL;
	img->stretch_accept = NULL;

	img->hexagon_dialog = NULL;
	img->hexagon_radius = 2;
	img->adj_hexagon_radius = NULL;
	img->hexagon_border = 0;
	img->adj_hexagon_border = NULL;
	img->hexagon_smooth_radius = 0;
	img->adj_hexagon_smooth_radius = NULL;
	img->hexagon_apply_postprocess = FALSE;
	img->hexagon_merge = NULL;
	img->hexagon_accept = NULL;
	
	img->mirror_vertical_dialog = NULL;
	img->mirror_horizontal_dialog = NULL;
	
	img->merge_dialog = NULL;
	img->merge = NULL;
	img->merge_accept = NULL;
	img->hf_to_merge = NULL;
	img->hf_to_merge_scaled = NULL;
	img->merge_scaled = FALSE;
	img->merge_listbox = NULL;
	img->merge_preview = NULL;
	img->merging_image_frame = NULL;
	
	img->math_fn_dialog = NULL;
	img->math_fn_accept = NULL;
	img->math_fn = POWER_OP;
	img->math_fn_param = 1.0;
	img->adj_math_fn = NULL;
		
	img->lift_edges_dialog = NULL;
	img->lift_edges_accept = NULL;
	img->lift_edges_radius = 20;
	img->adj_lift_edges_radius = NULL;
	img->lift_edges_level = 40;
	img->adj_lift_edges_level = NULL;
	img->lift_edges_level_dialog = NULL;	
	img->lift_edges_use_black_point = FALSE;
	img->lift_edges_wrap = TILING_AUTO;
	img->lift_edges_merge_done = FALSE;
	
	img->voronoi = voronoi_struct_new_with_defaults ();
	img->voronoi_dialog = voronoi_dialog_struct_new ();
	
	// Erosion
	
	img->rain_erosion = rain_erosion_struct_new (50000, 1, 50, 75, FALSE, 5000, FALSE);	
	img->rain_erosion_dialog = rain_erosion_dialog_struct_new ();
	
	img->gravity = gravity_struct_new (5, 60);
	img->gravity_dialog = gravity_dialog_struct_new ();
		
	img->oriented_gravity = oriented_gravity_struct_new (50, 30, EAST);
	img->oriented_gravity_dialog = oriented_gravity_dialog_struct_new ();	
		
	img->crests = gravity_struct_new (3, 30);
	img->crests_dialog = gravity_dialog_struct_new ();

	img->whimsical_erosion = whimsical_erosion_struct_new (6, 2, TRUE);
	img->whimsical_erosion_dialog = whimsical_erosion_dialog_struct_new ();
	
	img->craters_erosion = craters_erosion_struct_new (10, 10, 35, 0);
	img->craters_erosion_dialog = craters_erosion_dialog_struct_new ();
	
	img->ripples = wind_struct_new(50, 2, 0, 0.4, 4.0, FALSE, 2);
	img->ripples_dialog = wind_dialog_struct_new(RIPPLES, parent_data);
	
	img->dunes = wind_struct_new(50, 2, 30, 0.0, 5.0, TRUE, 10);
	img->dunes_dialog = wind_dialog_struct_new(DUNES, parent_data);
	
	return img;
}

void img_dialog_free(img_dialog_struct *img) {
	//	**** Don't forget to free the wav_pages & wav_data lists, plus
	//	the waves_shapes arrarys!
	gint i;
	GList *node;
	if (img) {
		for (i=0; i<NBWAVES; i++) 
			if (img->wav_shapes[i])
				x_free(img->wav_shapes[i]);
			if (img->wav_pages)
			//	This is a list of structs of widgets - no need to free more!
				g_list_free(img->wav_pages);
			if (img->wav_data) {
				for (node = img->wav_data; node; node=node->next) {
					wave_free((wave_struct *) node->data);
				}
				g_list_free(img->wav_data);
			}
		if (img->merge_listbox)
			img_listbox_free(img->merge_listbox);
		if (img->terrace_merge)
			x_free(img->terrace_merge);
		if (img->merge)
			x_free(img->merge);
		if (img->smooth_merge)
			x_free(img->smooth_merge);
		if (img->hexagon_merge)
			x_free(img->hexagon_merge);
		if (img->hf_to_merge_scaled && img->merge_scaled)
			hf_free(img->hf_to_merge_scaled);
		
		view_struct_free(img->merge_preview);
		
		voronoi_dialog_struct_free(img->voronoi_dialog);
		voronoi_struct_free(img->voronoi);
		craters_erosion_struct_free(img->craters_erosion);
		craters_erosion_dialog_struct_free(img->craters_erosion_dialog);
		gravity_struct_free(img->crests);
		gravity_dialog_struct_free(img->crests_dialog);
		rain_erosion_struct_free(img->rain_erosion);
		rain_erosion_dialog_struct_free(img->rain_erosion_dialog);
		gravity_struct_free(img->gravity);
		gravity_dialog_struct_free(img->gravity_dialog);
		oriented_gravity_struct_free(img->oriented_gravity);
		oriented_gravity_dialog_struct_free(img->oriented_gravity_dialog);
		whimsical_erosion_struct_free(img->whimsical_erosion);
		whimsical_erosion_dialog_struct_free(img->whimsical_erosion_dialog);
		
		wind_dialog_struct_free(img->ripples_dialog);
		wind_struct_free(img->ripples);
		wind_dialog_struct_free(img->dunes_dialog);
		wind_struct_free(img->dunes);
		
		x_free(img);
	}
}

GtkWidget * img_process_dialog_new(GtkWidget *window,
		GtkTooltips *tooltips,  gpointer callb_data) {
	// Initializes the dialog for mofying images
	// Two parts, from top to bottom:
	//	1. Toolbar of filters / actions
	//	2. Options for the current filter / action - given by the specific callback

	GtkWidget *dialog, *hbox;
	toolbar_struct *tb;

	dialog = gtk_vbox_new(FALSE, DEF_PAD);
	gtk_widget_show(dialog);
	hbox  = gtk_hbox_new(FALSE, DEF_PAD);
	gtk_widget_show(hbox);
//	lbl = gtk_label_new(_("Processes"));
//	gtk_widget_show(lbl);
//	gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, DEF_PAD);
	// toolbar_struct is a bit overkill for this purpose...
	tb = multi_toolbar_new(NBPROCESSES, img_processes,
		tooltips,
		window,
		callb_data,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),
		align_widget(tb->toolbarwdg,0.5,0.5), TRUE, TRUE, DEF_PAD);
	x_free(tb);

	gtk_box_pack_start(GTK_BOX(dialog), hbox, FALSE, FALSE, 0);
	return dialog;
}

/************************* GENERIC FUNCTIONS *************************/

void accept_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	accept_fn (hfw);
}

void reset_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	reset_fn (hfw);
}

GtkWidget *reset_accept_buttons_new (gpointer data, GtkWidget **accept_slot) {
	
	GtkWidget *button, *hbox;
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label (_("Reset"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) reset_callb, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Accept"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) accept_callb, data);
	gtk_widget_show(button);	
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	(*accept_slot) = button;
	gtk_widget_set_sensitive(button,FALSE);
		 
	return align_widget(hbox,0.5,0.5);
}

GtkWidget *apply_repeat_buttons_new (gpointer data,
	void (*apply_fn) (GtkWidget *, gpointer), 
	void (*repeat_fn) (GtkWidget *, gpointer) ) {
	
	GtkWidget *button, *hbox;
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label (_("Apply"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) apply_fn, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Repeat"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) repeat_fn, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	
	return align_widget(hbox,0.5,0.5);
}

static void merge_n_display (hf_wrapper_struct *hfw,
merge_struct *mrg) {
//	printf("Merge_n_display\n");
	
	gtk_widget_set_sensitive(GTK_WIDGET( hfw->hf_options->img->accept_wdg),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	unset_watch_cursor(hfw);
	set_merge_buffers (
		mrg,
		hfw->hf_struct->tmp_buf, 
		hfw->hf_struct->result_buf,
		hfw->hf_struct->hf_buf, 
		hfw->hf_struct->max_x, 
		hfw->hf_struct->max_y);
	simple_merge (mrg);
	draw_hf (hfw);
}

/************************* BRIGHTNESS - CONTRAST *************************/

void set_br_contr_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_brightness), 0);
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_contrast), 0);
}

void calc_br_contrast (hf_wrapper_struct *hfw) {
	if (!hfw) return;
	if (hfw->if_calculated)
		return;
	hf_brightness_contrast(hfw->hf_struct,
		hfw->hf_options->img->contrast_level,
		hfw->hf_options->img->brightness_level,
		gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(hfw->hf_options->
				img->contrast_keep_luminosity_check_box)),
		hfw->hf_options->img->brightness_bound_overflow );
	begin_pending_record(hfw,"Brightness and contrast",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->br_contr_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

void repeat_br_contr_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw) return;
	commit_pending_record(hfw);
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	calc_br_contrast(hfw);
}

void contrast_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw) return;
	if (hfw->hf_options->img->contrast_level == (gfloat) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->contrast_level = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_br_contrast(hfw);
}

void brightness_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw) return;
	if (hfw->hf_options->img->brightness_level == (gfloat) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->brightness_level = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_br_contrast(hfw);
}

void auto_br_contrast_callb(GtkWidget *wdg,gpointer data) {
//	Adjust the brightness and contrast so that the HF minimum becomes 0 and the max 0xFFFF
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw) return;
	hf_min_max(hfw->hf_struct);
	if (hfw->hf_struct->min == hfw->hf_struct->max)
		return;

	hf_auto_contrast(hfw->hf_struct);
	begin_pending_record(hfw,"Brightness and contrast",accept_fn, reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->br_contr_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

void set_overflow_bound (GtkWidget *wdg, gpointer data) {
	// Sets the overflow processing
	// "flag" is supposed to be TRUE for overflow = bound height, 
	// FALSE for overflow = revert slope.
	gboolean *flag;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	flag = (gboolean *) data;
	(*flag) = TRUE;
}

void set_overflow_revert (GtkWidget *wdg, gpointer data) {
	gboolean *flag;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	flag = (gboolean *) data;
	(*flag) = FALSE;
}

GtkWidget *br_contr_dialog_new(img_dialog_struct *img, gpointer data) {
	GtkWidget *vbox, *vbox2, *hbox, *frame, *frame2, *button, *scale;
	GtkObject *adj;
	GSList *group = NULL;

	frame = options_frame_new("Brightness and contrast");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

//	********* BRIGHTNESS

	frame2 = frame_new("Brightness",0.5*DEF_PAD);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0.5*DEF_PAD);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox2);

	gtk_container_add(GTK_CONTAINER(frame2), vbox2);

	adj = gtk_adjustment_new (0, -100, 100, 1, 1, 0.01);

    	scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_scale_set_digits (GTK_SCALE (scale), 0); 
	gtk_box_pack_start (GTK_BOX (vbox2), scale, TRUE, TRUE, DEF_PAD*0.5); 
	gtk_widget_show (scale);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (brightness_upd), data);
	img->adj_brightness = adj;
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox2), align_widget(hbox,0.5,0.5), FALSE, FALSE, 0);
	optimize_on_mouse_click (scale, data);
	percent_buttons_negative_new (hbox, (gpointer) adj);
	
//	********* CONTRAST

	frame2 = frame_new("Contrast",0.5*DEF_PAD);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0.5*DEF_PAD);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox2);

	gtk_container_add(GTK_CONTAINER(frame2), vbox2);

	adj = gtk_adjustment_new (0, -100, 100, 1, 1, 0.01);

    	scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_scale_set_digits (GTK_SCALE (scale), 0); 
	gtk_box_pack_start (GTK_BOX (vbox2), scale, TRUE, TRUE, DEF_PAD*0.5); 
	gtk_widget_show (scale);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (contrast_upd), data);
	img->adj_contrast = adj;
		
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox2), align_widget(hbox,0.5,0.5), FALSE, TRUE, 0);
	optimize_on_mouse_click (scale, data);	
	percent_buttons_negative_new (hbox, (gpointer) adj);

	// Contrast option:  if we try to keep luminosity when decreasing contrast
	// If not, black stays black

	img->contrast_keep_luminosity_check_box = 
		define_check_button_in_box ("Keep luminosity", vbox2, FALSE,FALSE,0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
		img->contrast_keep_luminosity_check_box ), TRUE);

//	******** Brightness and contrast option:  how we process overflow

	frame2 = frame_new("Overflow processing",0.5*DEF_PAD);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0.5*DEF_PAD);

	//	hbox for radio buttons
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_radio_button_in_box_with_data (hbox, &group, "Bound height",
		set_overflow_bound, &img->brightness_bound_overflow,
		img->brightness_bound_overflow) ;
	define_radio_button_in_box_with_data (hbox, &group, "Revert slope",
		set_overflow_revert, &img->brightness_bound_overflow,
		!img->brightness_bound_overflow) ;
	gtk_container_add(GTK_CONTAINER(frame2),hbox);

//	********* CONTROLS

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);

	button = gtk_button_new_with_label (_("Automatic"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) auto_br_contrast_callb, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Repeat"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) repeat_br_contr_callb, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5),
		 FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &img->br_contr_accept), FALSE, FALSE, DEF_PAD);

	return frame;	
}

void brightness_contrast_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
//	This dialog should be the default, so it is supposed to be defined with the parent dialog
//	if (!hfw->hf_options->img->br_contr_dialog) {
//		hfw->hf_options->img->br_contr_dialog = br_contr_dialog_new(data);
//		gtk_container_add(GTK_CONTAINER(hfw->tools_dialog),
//			hfw->hf_options->img->br_contr_dialog );
//	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->br_contr_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_br_contr_defaults;
// printf("ACCEPT_FN: %d\n",accept_fn);
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->br_contr_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* THRESHOLD *************************/

void set_threshold_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_threshold_min), 0);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_threshold_max), 0xFFFF);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_threshold_percent), 100);
}

void calc_threshold (hf_wrapper_struct *hfw) {
	if (hfw->if_calculated)
		return;
	hf_threshold(hfw->hf_struct, 
		hfw->hf_options->img->threshold_min,
		hfw->hf_options->img->threshold_max,
		hfw->hf_options->img->threshold_percent );
	begin_pending_record(hfw,"Threshold",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->threshold_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

void threshold_min_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Level to which we apply the threshold filter
	if (hfw->hf_options->img->threshold_min == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->threshold_min = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_threshold(hfw);
}

void threshold_max_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Level to which we apply the threshold filter
	if (hfw->hf_options->img->threshold_max == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->threshold_max = (gint) GTK_ADJUSTMENT(wdg)->value;	
	hfw->if_calculated = FALSE;
	calc_threshold(hfw);
}

void threshold_percent_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Level to which we apply the threshold filter
	if (hfw->hf_options->img->threshold_percent == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->threshold_percent = (gint) GTK_ADJUSTMENT(wdg)->value;	
	hfw->if_calculated = FALSE;
	calc_threshold(hfw);
}

GtkWidget *threshold_dialog_new(gpointer data) {
	GtkWidget *vbox, *frame, *table, *scale;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Threshold");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	table = gtk_table_new(3, 2, FALSE);
	gtk_widget_show(GTK_WIDGET(table));

	define_label_in_table("Minimum",table,0, 1, 0, 1, DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->threshold_min, 0, 0xFFFF, 10, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 0, 1, 0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (threshold_min_upd), data);
	hfw->hf_options->img->adj_threshold_min = adj;

	define_label_in_table("Maximum",table,0, 1, 1, 2, DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->threshold_max, 0, 0xFFFF, 10, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 1, 2, 0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (threshold_max_upd), data);
	hfw->hf_options->img->adj_threshold_max = adj;

	define_label_in_table("%",table,0, 1, 2, 3, DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->threshold_percent, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 2, 3, 0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (threshold_percent_upd), data);
	hfw->hf_options->img->adj_threshold_percent = adj;

  	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->threshold_accept), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void threshold_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
//		When we change options, we commit last changes by backing up
//		the current buffer into the temp buffer
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->threshold_dialog) {
		hfw->hf_options->img->threshold_dialog = threshold_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->threshold_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->threshold_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_threshold_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->threshold_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* SMOOTH *************************/

void set_smooth_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_smooth_radius), 0);
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->smooth_merge->adj_mix), 100);
	reset_merge_buffers (img->smooth_merge->content);
}

void calc_smooth (hf_wrapper_struct *hfw) {
	gint t1;
	gboolean wrap;
// printf("Calc_smooth\n");
	if (hfw->if_calculated)
		return;
	if (hfw->hf_struct->max_x>hfw->hf_options->dist_matrix->hf_size)
		dist_matrix_init(hfw->hf_options->dist_matrix, hfw->hf_struct->max_x);
	t1 = clock();
	set_watch_cursor(hfw);
	switch (hfw->hf_options->img->smooth_wrap) {
		case TILING_AUTO:
			wrap = *hfw->tiling_ptr;
			break;
		case TILING_YES:
			wrap = TRUE;
			break;
		case TILING_NO:
			wrap = FALSE;
	}

	hf_backup(hfw->hf_struct);

	hf_init_result_buf_from_hf (hfw->hf_struct);

	smooth_buf (hfw->hf_struct->result_buf, 
		hfw->hf_struct->max_x, 
		hfw->hf_struct->max_y,
		hfw->hf_options->img->smooth_radius,
		hfw->hf_options->dist_matrix,
		wrap,
		hfw->hf_options->gauss_list,
		HF_TYPE_ID);

// printf("TEMPS DE SMOOTH: %d\n",clock() - t1);
	begin_pending_record(hfw,"Smooth", accept_fn, reset_fn);
	merge_n_display (hfw, hfw->hf_options->img->smooth_merge->content);
}

void smooth_radius_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->smooth_radius == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->smooth_radius = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
}

void apply_smooth_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	calc_smooth(hfw);
}

void repeat_smooth_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->if_calculated = FALSE;
	calc_smooth(hfw);
}

GtkWidget *smooth_dialog_new(gpointer data) {
	GtkWidget *vbox, *hbox, *frame, *frame2;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	
	frame = options_frame_new("Smooth");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);

	define_label_in_box("Radius",hbox,FALSE, FALSE,DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->smooth_radius, 0, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (smooth_radius_upd), data);
	hfw->hf_options->img->adj_smooth_radius = adj;

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

/*
//	Sampling filter box, as a % of radius, for testing purposes...
//	Better at 150% with "sharp" filter
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);

	define_label_in_box("BOITE",hbox,TRUE, TRUE,DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->smooth_box, 0, 300, 1, 1, 0.01);
	define_scale_in_box(adj,hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (smooth_box_upd), data);
	hfw->hf_options->img->adj_smooth_box = adj;

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
*/
	hbox = tiling_control(&hfw->hf_options->img->smooth_wrap);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);

// 	Merge

	hfw->hf_options->img->smooth_merge = merge_dialog_struct_new( merge_struct_new(data,(gpointer) draw_hf_ptr));
	hfw->hf_options->img->smooth_merge->content->mix = 100;
	frame2 = merge_dialog_new(hfw->hf_options->img->smooth_merge,
		TRUE, hfw->hf_options->tools_window, NULL, NULL, NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0);
	gtk_widget_hide (GTK_WIDGET(hfw->hf_options->img->smooth_merge->main_box));
	
  	gtk_box_pack_start (GTK_BOX (vbox), apply_repeat_buttons_new (data, apply_smooth_callb, repeat_smooth_callb), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->smooth_accept), FALSE, FALSE, DEF_PAD);

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void smooth_callb(GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->smooth_dialog) {
		hfw->hf_options->img->smooth_dialog = smooth_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->smooth_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->smooth_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_smooth_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->smooth_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* SHARPEN (increase noise) *************************/

void set_sharpen_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_sharpen_radius), 0);
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->sharpen_merge->adj_mix), 100);
}

void calc_sharpen (hf_wrapper_struct *hfw) {
	gint t1;
	gboolean wrap;
	gdouble *v;
	if (hfw->if_calculated)
		return;
	if (!hfw->hf_options->img->sharpen_radius)
		return;
	t1 = clock();
	set_watch_cursor(hfw);
	switch (hfw->hf_options->img->sharpen_wrap) {
		case TILING_AUTO:
			wrap = *hfw->tiling_ptr;
			break;
		case TILING_YES:
			wrap = TRUE;
			break;
		case TILING_NO:
			wrap = FALSE;
	}

	hf_backup(hfw->hf_struct);
		
	v = create_gaussian_kernel (hfw->hf_options->img->sharpen_radius, CONST_E, 10, ((hfw->hf_options->img->sharpen_level)/100.0) - 0.81, 0.55, FALSE);
	hf_convolve (hfw->hf_struct, 
		hfw->hf_options->img->sharpen_radius,
		v, wrap);
	x_free(v);

// printf("TEMPS DE SHARPEN: %d\n",clock() - t1);
	begin_pending_record(hfw,"Sharpen", accept_fn, reset_fn);

	merge_n_display (hfw, hfw->hf_options->img->sharpen_merge->content);
}

void sharpen_radius_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->sharpen_radius == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->sharpen_radius = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
}

void sharpen_level_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->sharpen_level == (gdouble) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->sharpen_level = (gdouble) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
}

void apply_sharpen_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	calc_sharpen(hfw);
}

void repeat_sharpen_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	calc_sharpen(hfw);
}

GtkWidget *sharpen_dialog_new(gpointer data) {
	GtkWidget *vbox, *hbox, *frame, *frame2;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	
	frame = options_frame_new("Sharpen (increase noise)");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);

	define_label_in_box("Radius",hbox,FALSE, FALSE,DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->sharpen_radius, 0, 20, 1, 1, 0.01);
	define_scale_in_box(adj,hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (sharpen_radius_upd), data);
	hfw->hf_options->img->adj_sharpen_radius = adj;

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	
	define_label_in_box("Level",hbox,FALSE, FALSE,DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->sharpen_level, 0.0, 10.0, 0.01, 0.01, 0.001);
	define_scale_in_box(adj,hbox, 1, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (sharpen_level_upd), data);
	hfw->hf_options->img->adj_sharpen_level = adj;

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox = tiling_control(&hfw->hf_options->img->sharpen_wrap);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);

// 	Merge

	hfw->hf_options->img->sharpen_merge = merge_dialog_struct_new ( merge_struct_new(data,(gpointer) draw_hf_ptr) );
	hfw->hf_options->img->sharpen_merge->content->mix = 100;
	frame2 = merge_dialog_new(hfw->hf_options->img->sharpen_merge,
		TRUE, hfw->hf_options->tools_window, NULL, NULL, NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0);
	gtk_widget_hide (GTK_WIDGET(hfw->hf_options->img->sharpen_merge->main_box));
	
  	gtk_box_pack_start (GTK_BOX (vbox), apply_repeat_buttons_new (data, apply_sharpen_callb, repeat_sharpen_callb), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->sharpen_accept), FALSE, FALSE, DEF_PAD);

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void sharpen_callb(GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->sharpen_dialog) {
		hfw->hf_options->img->sharpen_dialog = sharpen_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->sharpen_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->sharpen_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_sharpen_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->sharpen_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* CITYSCAPE *************************/

void set_cityscape_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_skyscraper_width), 0);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_skyscraper_width_var), 0);
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_streets_width), 0);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->cityscape_merge->adj_mix), 100);
}

void calc_cityscape (hf_wrapper_struct *hfw) {
	if (hfw->if_calculated)
		return;
	hf_city( hfw->hf_struct, 
		hfw->hf_options->img->skyscraper_width, 
		hfw->hf_options->img->skyscraper_width_var,
		hfw->hf_options->img->streets_width);
	begin_pending_record(hfw,"City",accept_fn, reset_fn);
	merge_n_display (hfw, hfw->hf_options->img->cityscape_merge->content);
}

void skyscraper_width_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->skyscraper_width == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->skyscraper_width = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_cityscape(hfw);
}

void skyscraper_width_var_upd (GtkWidget *wdg, gpointer data) {
//	% of skyscraper size variation
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->skyscraper_width_var == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->skyscraper_width_var = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_cityscape(hfw);
}

void streets_width_upd (GtkWidget *wdg, gpointer data) {
//	Streets width, as a % of skyscraper size (max. 50%, means 25% of each surrounding sk-scr)
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->streets_width == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->streets_width = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_cityscape(hfw);
}

void smooth_1_pixel_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	static gdouble blur_matrix[] = {0.0625,0.125,0.0625,0.125,0.25,0.125,0.0625,0.125,0.0625};
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	begin_pending_record(hfw,"Smooth 1 pixel",accept_fn, reset_fn);
	hf_convolve (hfw->hf_struct,1, blur_matrix, FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->cityscape_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	draw_hf(hfw);
}

GtkWidget *city_dialog_new(gpointer data) {
	GtkWidget *vbox, *frame, *frame2, *button, *table, *scale;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	frame = options_frame_new("City");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(vbox);

	table = gtk_table_new(2, 3, FALSE);
	gtk_widget_show(GTK_WIDGET(table));

	define_label_in_table("Skyscaper width", table, 0, 1, 0, 1, DEF_PAD);

	adj = gtk_adjustment_new (hfw->hf_options->img->skyscraper_width, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,0,1,0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (skyscraper_width_upd), data);
	hfw->hf_options->img->adj_skyscraper_width = adj;

	define_label_in_table("Streets width",table,0,1,1,2,DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->skyscraper_width, 0, 50, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,1,2,0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (streets_width_upd), data);
	hfw->hf_options->img->adj_streets_width = adj;

	define_label_in_table("Width variation (%)",table,0,1,2,3,DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->skyscraper_width_var, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,2,3,0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (skyscraper_width_var_upd), data);
	hfw->hf_options->img->adj_skyscraper_width_var = adj;

	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);

	button = gtk_button_new_with_label (_("Smooth 1 pixel"));
	gtk_widget_show(button);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) smooth_1_pixel_callb, data);

// 	Merge

	hfw->hf_options->img->cityscape_merge = merge_dialog_struct_new ( merge_struct_new(data, (gpointer) draw_hf_ptr));
	hfw->hf_options->img->cityscape_merge->content->mix = 100;
	frame2 = merge_dialog_new(hfw->hf_options->img->cityscape_merge,
		TRUE, hfw->hf_options->tools_window, NULL, NULL, NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0);
	gtk_widget_hide (GTK_WIDGET(hfw->hf_options->img->cityscape_merge->main_box));
	
  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(button,0.5,0.5), FALSE, FALSE, DEF_PAD);

	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->cityscape_accept), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void city_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->city_dialog) {
		hfw->hf_options->img->city_dialog = city_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->city_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->city_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_cityscape_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->cityscape_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* TERRACES *************************/

void set_terrace_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_terrace_levels), 0);
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->terrace_merge->adj_mix), 100);
}

void calc_terrace (hf_wrapper_struct *hfw) {
	gboolean wrap;
	if (hfw->if_calculated)
		return;
	if (hfw->hf_options->img->terrace_smooth_radius)
		if (hfw->hf_struct->max_x>hfw->hf_options->dist_matrix->hf_size)
			dist_matrix_init(hfw->hf_options->dist_matrix, hfw->hf_struct->max_x);
	switch (hfw->hf_options->img->terrace_wrap) {
		case TILING_AUTO:
			wrap = *hfw->tiling_ptr;
			break;
		case TILING_YES:
			wrap = TRUE;
			break;
		case TILING_NO:
			wrap = FALSE;
	}
	if (hfw->hf_options->img->terrace_apply_postprocess )
		set_watch_cursor(hfw);

	hf_terrace(hfw->hf_struct,
		hfw->hf_options->img->terrace_levels,
		hfw->hf_options->img->terrace_seed,
		hfw->hf_options->img->terrace_percent_random,
		hfw->hf_options->img->terrace_smooth_radius,
		wrap,
		hfw->hf_options->dist_matrix,
		hfw->hf_options->img->terrace_artifact_radius,
		hfw->hf_options->img->terrace_apply_postprocess,
		hfw->hf_options->gauss_list);

	if (hfw->hf_options->img->terrace_apply_postprocess )
		unset_watch_cursor(hfw);
	hfw->hf_options->img->terrace_apply_postprocess = FALSE;
	begin_pending_record(hfw,"Terraces", accept_fn, reset_fn);
	merge_n_display (hfw, hfw->hf_options->img->terrace_merge->content);
}

void terrace_levels_upd (GtkWidget *wdg, gpointer data) {
//	Updates the number of terrace levels and displays the HF
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->terrace_levels == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->terrace_levels = (gint) GTK_ADJUSTMENT(wdg)->value;

	if (!hfw->hf_options->img->terrace_levels) {
		reset_callb(wdg,data);
		return;
	}
	hfw->if_calculated = FALSE;
	calc_terrace(hfw);
}

void terrace_percent_random_upd (GtkWidget *wdg, gpointer data) {
	// Updates the level of randomness variation of each "step"
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->terrace_percent_random == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->terrace_percent_random = (gint) GTK_ADJUSTMENT(wdg)->value;		
	hfw->if_calculated = FALSE;
	calc_terrace(hfw);
}

void terrace_smooth_radius_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->terrace_smooth_radius == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->terrace_smooth_radius = (gint) GTK_ADJUSTMENT(wdg)->value;		
	hfw->if_calculated = FALSE;
//	calc_terrace(hfw);
}

void terrace_apply_postprocess_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->img->terrace_apply_postprocess = TRUE;
	hfw->if_calculated = FALSE;
	calc_terrace(hfw);
}

static gint change_terrace_seed(GtkWidget *entry, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->img->terrace_seed = 
		(unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry))); 
	hfw->if_calculated = FALSE;
	calc_terrace(hfw);
	return FALSE;
}

void terrace_artifact_radius_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->terrace_artifact_radius == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->terrace_artifact_radius = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
//	calc_terrace(hfw);
}

GtkWidget *terrace_dialog_new(gpointer data) {
	GtkWidget *vbox, *vbox2, *vbox3, *hbox, *frame, *frame2, *button;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Terraces");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);	// vbox for all options
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD); // vbox for quantization
	gtk_widget_show(vbox2);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("Levels", hbox,FALSE, FALSE, DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->terrace_levels, 0, 50, 1, 1, 0.01);
	define_scale_in_box(adj, hbox,  0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (terrace_levels_upd), data);
	hfw->hf_options->img->adj_terrace_levels = adj;

	gtk_box_pack_start(GTK_BOX(vbox2),hbox, TRUE, TRUE, DEF_PAD *0.5);

//	Random variation

	vbox3 = gtk_vbox_new(FALSE,DEF_PAD); 
	gtk_widget_show(vbox3);
	frame2 = frame_new("Random var.",DEF_PAD*0.5);
	gtk_container_add(GTK_CONTAINER(frame2),vbox3);
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("%", hbox,FALSE,FALSE, DEF_PAD);
 	adj = gtk_adjustment_new (hfw->hf_options->img->terrace_percent_random, 0.0, 100.0, 1.0, 1.0, 0.0);
	define_scale_in_box(adj, hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (terrace_percent_random_upd), data);
	hfw->hf_options->img->adj_terrace_percent_random = adj;
	gtk_box_pack_start(GTK_BOX(vbox3), hbox, TRUE, TRUE, DEF_PAD*0.5);
	gtk_box_pack_start(GTK_BOX(vbox3), 
		seed_dialog_new(data, hfw->hf_options->img->terrace_seed, change_terrace_seed), 
		TRUE, TRUE, DEF_PAD*0.5);

	gtk_box_pack_start(GTK_BOX(vbox2), frame2, TRUE, TRUE, DEF_PAD*0.5);

// 	Main "quantization" frame

	frame2 = define_frame_with_hiding_arrows ("Quantization", vbox2, hfw->hf_options->tools_window, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, TRUE, TRUE, 0); // the main box

/********************** Post-process subdialogs *************************/ 

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD); // vbox for subdialogs
	gtk_widget_show(vbox2);

//	Terraces' smooth

	vbox3 = gtk_vbox_new(FALSE,DEF_PAD); 
	gtk_widget_show(vbox3);
	frame2 = frame_new("Smooth",DEF_PAD*0.5);
	gtk_container_add(GTK_CONTAINER(frame2),vbox3);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("Radius", hbox,FALSE,FALSE, DEF_PAD);
 	adj = gtk_adjustment_new (hfw->hf_options->img->terrace_smooth_radius, 0.0, 20.0, 1.0, 1.0, 0.0);
	define_scale_in_box(adj, hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (terrace_smooth_radius_upd), data);
	hfw->hf_options->img->adj_terrace_smooth_radius = adj;
	gtk_box_pack_start(GTK_BOX(vbox3), hbox, TRUE, TRUE, DEF_PAD*0.5);

	// hbox for "wrap" buttons

	hbox = tiling_control(&hfw->hf_options->img->terrace_wrap);

  	gtk_box_pack_start (GTK_BOX (vbox3), align_widget(hbox,0.5,0.5), 
		FALSE, FALSE, DEF_PAD*0.5);

	gtk_box_pack_start(GTK_BOX(vbox2), frame2, TRUE, TRUE, 0);

/********************** Smooth subdialog end ****************************/

//	Artifact removal

	vbox3 = gtk_vbox_new(FALSE,DEF_PAD); 
	gtk_widget_show(vbox3);
	frame2 = frame_new("Artifact removal",DEF_PAD*0.5);
	gtk_container_add(GTK_CONTAINER(frame2),vbox3);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);

	define_label_in_box("Radius", hbox,FALSE,FALSE, DEF_PAD);
 	adj = gtk_adjustment_new (hfw->hf_options->img->terrace_artifact_radius, 0.0, 20.0, 1.0, 1.0, 0.0);
	define_scale_in_box(adj, hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (terrace_artifact_radius_upd), data);
	hfw->hf_options->img->adj_terrace_artifact_radius = adj;
	gtk_box_pack_start(GTK_BOX(vbox3), hbox, TRUE, TRUE, DEF_PAD*0.5);

	gtk_box_pack_start(GTK_BOX(vbox2), frame2, TRUE, TRUE, 0);

/********************** Artifact subdialog end ****************************/

	button = gtk_button_new_with_label (_("Apply"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) terrace_apply_postprocess_callb, data);
	gtk_widget_show(button);

  	gtk_box_pack_start (GTK_BOX (vbox2), align_widget(button,0.5,0.5), 
		FALSE, FALSE, DEF_PAD*0.5);

// 	Post-process frame

	frame2 = define_frame_with_hiding_arrows ("Result post-processing", vbox2, hfw->hf_options->tools_window, FALSE);
	
	gtk_box_pack_start(GTK_BOX(vbox), frame2, TRUE, TRUE, 0); // the main box

/********************** Post-process frame end ****************************/

// 	Merge

	hfw->hf_options->img->terrace_merge = merge_dialog_struct_new (merge_struct_new(data, (gpointer) draw_hf_ptr));
	hfw->hf_options->img->terrace_merge->content->mix = 100;
	frame2 = merge_dialog_new(hfw->hf_options->img->terrace_merge,
		TRUE, hfw->hf_options->tools_window, NULL, NULL, NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0);
	gtk_widget_hide (GTK_WIDGET(hfw->hf_options->img->terrace_merge->main_box));

// 	Control buttons
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->terrace_accept), FALSE, FALSE, DEF_PAD);

	return frame;	
}

void terrace_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->terrace_dialog) {
		hfw->hf_options->img->terrace_dialog = terrace_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->terrace_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->terrace_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_terrace_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->terrace_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* REVERT *************************/

GtkWidget *revert_dialog_new(gpointer data) {
	GtkWidget *lbl, *frame;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	frame = options_frame_new("Revert");
	lbl = gtk_label_new(_("No parameters\nfor this operation"));
	gtk_widget_show(lbl);
	gtk_container_add(GTK_CONTAINER(frame), lbl);
	return frame;
}

// gdouble matrix[9] = {1.0, 1.0, 1.0, 1.0, -8.0, 1.0, 1.0, 1.0, 1.0};
// gdouble matrix[9] = {0.5, 0.5, 0.5, 0.5, -4.0, 0.5, 0.5, 0.5, 0.5};
// gdouble matrix[9] = {0.1, 0.1, 0.1, 0.1, -0.8, 0.1, 0.1, 0.1, 0.1};
// gdouble matrix[9] = {-0.1, -0.1, -0.1, -0.1, 1.8, -0.1, -0.1, -0.1, -0.1};

// gdouble matrix[9] = {0.2, 0.2, 0.2, 0.2, -0.6, 0.2, 0.2, 0.2, 0.2};

/*
gdouble matrix[25] = {
-0.1, -0.1, -0.1, -0.1, -0.1,
-0.1, -0.1, -0.1, -0.1, -0.1,
-0.1, -0.1, 5.0, -0.1, -0.1,
-0.1, -0.1, -0.1, -0.1, -0.1,
-0.1, -0.1, -0.1, -0.1, -0.1};

// Kernel for "noising" (sharpening)
gdouble matrix[25] = {
0.1, 0.1, 0.1, 0.1, 0.1,
0.1, 0.2, 0.2, 0.2, 0.1,
0.1, 0.2, -2.2, 0.2, 0.1,
0.1, 0.2, 0.2, 0.2, 0.1,
0.1, 0.1, 0.1, 0.1, 0.1};
*/

/*
// Kernels for "noising" (sharpening)
gdouble matrix3[25] = {
0.1, 0.1, 0.1, 0.1, 0.1,
0.1, -0.2, -0.2, -0.2, 0.1,
0.1, -0.2, -2.2, -0.2, 0.1,
0.1, -0.2, -0.2, -0.2, 0.1,
0.1, 0.1, 0.1, 0.1, 0.1};

gdouble matrix1[49] = {
0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.1,
0.1, 0.2, 0.4, 0.4, 0.4, 0.2, 0.1,
0.1, 0.2, 0.4, -8.6, 0.4, 0.2, 0.1,
0.1, 0.2, 0.4, 0.4, 0.4, 0.2, 0.1,
0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.1,
0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
gdouble matrix2[49] = {
0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.1,
0.1, 0.2, -0.2, -0.2, -0.2, 0.2, 0.1,
0.1, 0.2, -0.2, -2.4, -0.2, 0.2, 0.1,
0.1, 0.2, -0.2, -0.2, -0.2, 0.2, 0.1,
0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.1,
0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
gdouble matrix[49] = {
0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2,
0.2, -0.1, -0.1, -0.1, -0.1, -0.1, 0.2,
0.2, -0.1, -0.2, -0.2, -0.2, -0.1, 0.2,
0.2, -0.1, -0.2, -2.4, -0.2, -0.1, 0.2,
0.2, -0.1, -0.2, -0.2, -0.2, -0.1, 0.2,
0.2, -0.1, -0.1, -0.1, -0.1, -0.1, 0.2,
0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
*/

void revert_callb(GtkWidget *wdg, gpointer data) {

	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

//	gint i, min, max;
//	gdouble ratio;
//	static gboolean flag=TRUE;
//	static double complex *dc=NULL, *dci=NULL;
//	static gint *gint_buf=NULL;

//	printf("Revert_callb\n");
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		commit_pending_record(hfw);
//		hf_backup(hfw->hf_struct);	No need the commit the change here, no tmp_buf!
		return;
	}
	if (!hfw->hf_options->img->revert_dialog) {
		hfw->hf_options->img->revert_dialog = revert_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->revert_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->revert_dialog;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
//	hf_backup(hfw->hf_struct);	// Remove for hf_revert	

	hf_revert(hfw->hf_struct);
	
//	hf_erode (hfw->hf_struct, 1, matrix);
/*
	if (!gint_buf)
		gint_buf = (gint *) calloc(sizeof(gint), hfw->hf_struct->max_x * hfw->hf_struct->max_y);
	if (flag) {
		fourier_struct_init (hfw->hf_options->fourier->fs, (gpointer) hfw->hf_struct->tmp_buf, hfw->hf_struct->max_x, hfw->hf_struct->max_y, HF_TYPE_ID);
		fourier_filter_init (hfw->hf_options->fourier->fs->ffs, F_BOX, 5.0, 5.0, 0.0, 0.0, 30.0, 100.0);
		fourier_apply_filter (hfw->hf_options->fourier->fs);
		convert2module(hfw->hf_options->fourier->fs, (gpointer) hfw->hf_struct->hf_buf, HF_TYPE_ID);
		flag=FALSE;
	}
	else {
		fourier_compute_inverse (hfw->hf_options->fourier->fs);
		convert_finv (hfw->hf_options->fourier->fs, (gpointer) hfw->hf_struct->hf_buf, HF_TYPE_ID);
		flag=TRUE;
	}
*/

//	for (i=0; i<49; i++) {
//		*(matrix+i) = *(matrix1+i)*0.1;
//		printf("Mat[%d]: %f\n",i,*(matrix+i));
//	}

//	hf_convolve(hfw->hf_struct, 3, 	matrix, FALSE);
//	write_pixels(hfw->hf_struct,create_site_list (hfw->hf_struct->max_x, hfw->hf_struct->max_y, 20, UNIFORM), 0xFFFF);
//	write_pixels(hfw->hf_struct, create_site_list (hfw->hf_struct->max_x, hfw->hf_struct->max_y, 10, CENTERED), 0);
//	hf_erode(hfw->hf_struct, 5);

//	find_maximum (hfw->hf_struct->tmp_buf, hfw->hf_struct->hf_buf, hfw->hf_struct->max_x, hfw->hf_struct->max_y, 0, 0);
	
	begin_pending_record(hfw,"Revert", accept_fn, reset_fn);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = FALSE;
	draw_hf(hfw);
}

/************************************* SHAPE FILTERS *********************************/

void accept_filter_fn (hf_wrapper_struct *hfw) {
	accept_fn(hfw);
	hfw->apply_filter = FALSE;
}

void set_f_defaults(img_dialog_struct *img) {
	set_filter_defaults(img->fd_struct);
}

void calc_shape_filter (hf_wrapper_struct *hfw) {
//	"filter_apply" does not copy tmp_buf to hf_buf, because it was developped for
//	the creation process, so we have to refresh hf_buf with tmp_buf before applying it

	if (hfw->if_calculated)
		return;
	if (hfw->hf_struct->tmp_buf)
		memcpy(hfw->hf_struct->hf_buf, hfw->hf_struct->tmp_buf,
			hfw->hf_struct->max_x*hfw->hf_struct->max_y*sizeof(hf_type));
	else
		hf_backup(hfw->hf_struct);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->filter_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
//	The filter is applied in draw_hf, on the main buffer
	draw_hf(hfw);
	begin_pending_record(hfw,"Shape filter", accept_filter_fn, reset_fn);
}

GtkWidget *img_filter_dialog_new(gpointer data) {
	GtkWidget *frame, *vbox, *vbox2;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Shape filter");

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);
		
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	hfw->hf_options->img->fd_struct = filter_toolbar_new (hfw->hf_options->tools_window, hfw->hf_options->tooltips, data);

//	Embed the function which applies the filter
	hfw->hf_options->img->fd_struct->apply_filter = (gpointer) calc_shape_filter;
	
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox2);	
	gtk_box_pack_start(GTK_BOX(vbox), vbox2, FALSE, FALSE, 0);
	
	//	Embed in a vbox;  this allows adding optional widgets afterwards

	gtk_box_pack_start(GTK_BOX(vbox2), hfw->hf_options->img->fd_struct->filter_dialog, FALSE, FALSE, 0);
		
	hfw->hf_options->img->fd_struct->filter_vbox = vbox2;
		
	// For this one, we must redefine "filter_dialog", which is the dialog
	// we show / hide when selecting a new tool in the toolbaox
	// (not very elegant... should be improved in the future!)
	
	hfw->hf_options->img->fd_struct->filter_dialog = frame;
	
	// Controls in the main vbox
		
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->filter_accept), FALSE, FALSE, DEF_PAD);

	return frame;
	
}

void filter_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}

	if (!hfw->hf_options->img->fd_struct) {
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			img_filter_dialog_new(data));
	}
//	Let this dialog become the current filter (for callbacks in hf_filters.c)
	hfw->hf_options->fd_struct = hfw->hf_options->img->fd_struct ;
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->fd_struct->filter_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_f_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->filter_accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* NOISE *************************/

void set_noise_defaults(img_dialog_struct *img) {

	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_noise_level), 0);
}

void calc_noise (hf_wrapper_struct *hfw) {
//	See comments in calc_shape_filter
	if (hfw->if_calculated)
		return;

	if (hfw->hf_struct->tmp_buf)
		memcpy(hfw->hf_struct->hf_buf, hfw->hf_struct->tmp_buf, 
			hfw->hf_struct->max_x*hfw->hf_struct->max_y*sizeof(hf_type));
	else
		hf_backup(hfw->hf_struct);

	if (!hfw->hf_options->img->noise_to_apply) 
		hfw->hf_options->img->noise_to_apply = hf_new(hfw->hf_struct->max_x);
	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->if_calculated = FALSE;

	subdiv2(hfw->hf_options->img->noise_to_apply,
			((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->subdiv2_options);
	hf_merge(hfw->hf_options->img->noise_to_apply, hfw->hf_struct,
		hfw->hf_struct->max_x>>1,
		hfw->hf_struct->max_y>>1,
		hfw->hf_options->img->noise_merge,
		TRUE,
		((gfloat) hfw->hf_options->img->noise_level) / 100.0,
		FALSE);
	begin_pending_record(hfw,"Noise", accept_fn, reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->noise_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

void noise_level_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->noise_level == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->noise_level = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
//	calc_noise(hfw);
}

static void add_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->noise_merge == ADD)
		return;
	hfw->hf_options->img->noise_merge = ADD;
	hfw->if_calculated = FALSE;
	calc_noise(hfw);
}

static void subtract_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->noise_merge == SUBTRACT)
		return;
	hfw->hf_options->img->noise_merge = SUBTRACT;
	hfw->if_calculated = FALSE;
	calc_noise(hfw);
}
static void multiply_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->noise_merge == MULTIPLY)
		return;
	hfw->hf_options->img->noise_merge = MULTIPLY;
	hfw->if_calculated = FALSE;
	calc_noise(hfw);
}

static void apply_noise (gpointer data) {
	hf_wrapper_struct *hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->if_calculated = FALSE;
	calc_noise(hfw);
}

static void apply_noise_callb (GtkWidget *wdg, gpointer data) {
	apply_noise (data);
}
static gint apply_noise_event (GtkWidget *wdg,GdkEventButton *event, gpointer data) {
	apply_noise (data);
	return FALSE;
}

GtkWidget *noise_dialog_new (gpointer data) {

	GtkWidget *vbox, *hbox, *frame, *button, *vbox2, *merge_toolbar, *wdg, *scale;
	static gboolean dummy_flag;
	hf_options_struct *hfo;
	gint frq[12] = {50,50,50,0,0,0,0,0,0,0,0,0};
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new_pad("Noise",0);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("%", hbox, FALSE,TRUE, DEF_PAD);
 	hfw->hf_options->img->adj_noise_level = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 1.0, 0.0);
	scale = define_scale_in_box(hfw->hf_options->img->adj_noise_level,hbox,0,DEF_PAD);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (apply_noise_event), data);

//	Merge mode

	define_label_in_box("Merge", hbox, FALSE, FALSE, DEF_PAD *0.5);
	merge_toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(merge_toolbar),GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(merge_toolbar),GTK_TOOLBAR_TEXT);
	gtk_widget_show(GTK_WIDGET(merge_toolbar));
	wdg = gtk_toolbar_append_element(GTK_TOOLBAR(merge_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,"+",
		_("Add"),NULL,NULL,GTK_SIGNAL_FUNC(add_callb),data);
	wdg = gtk_toolbar_append_element(GTK_TOOLBAR(merge_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,"-",
		_("Subtract"),NULL,NULL,GTK_SIGNAL_FUNC(subtract_callb),data);
	gtk_toolbar_append_element(GTK_TOOLBAR(merge_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,"x",
		_("Multiply"),NULL,NULL,GTK_SIGNAL_FUNC(multiply_callb),data);

	gtk_box_pack_start(GTK_BOX(hbox),merge_toolbar, TRUE, TRUE, DEF_PAD *0.5);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, DEF_PAD);

	gtk_signal_connect (GTK_OBJECT (hfw->hf_options->img->adj_noise_level), "value_changed",
		GTK_SIGNAL_FUNC (noise_level_upd), data);

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox2));

	gtk_container_add(GTK_CONTAINER(vbox), vbox2);

//	We create the HF containing the noise, and the preview area
	hfo = hf_options_dialog_new(hfw->hf_options->tools_window,
			vbox, 
			hfw->hf_options->tooltips, 
			NULL,
			&hfw->hf_options->img->hf_noise,
			hfw->hf_options->stack,
			hfw->hf_options->doc_type, 
			hfw->hf_options->all_docs, 
			hfw->hf_options->get_doc_list_from_type);
	hfo->hf_size = MAX_NOISE_PREVIEW;

	hfw->hf_options->img->hf_noise = hf_wrapper_new(NULL,NULL,vbox2, vbox, hfo,
		&dummy_flag, &dummy_flag, NULL, NULL, NULL, NULL, HFNOISE);

//	Shared data:
	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->dist_matrix = hfw->hf_options->dist_matrix;

//	Initialize the current HF compute function:
	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->subdiv2_options =
		subdiv2_opt_new(frq, 12);
	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->subdiv2_options->distribution = 10;
	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->current_options =
		((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->subdiv2_options;
	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->subdiv2_dialog =
		subdiv2_dialog_new(((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options);
	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->current_dialog = 
		((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->subdiv2_dialog;

	((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->current_calculation = (gpointer) subdiv2;


	set_dependent_process (((hf_wrapper_struct *)hfw->hf_options->img->hf_noise), apply_noise, data);

//	Hide the filter subdialog (not relevant here):
	gtk_widget_hide( ((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->creation_fd_struct->filter_dialog);

	gtk_container_add(GTK_CONTAINER(vbox),
		((hf_wrapper_struct *)hfw->hf_options->img->hf_noise)->hf_options->subdiv2_dialog);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->noise_accept), FALSE, FALSE, DEF_PAD);

	return frame;
}

void noise_callb(GtkWidget *wdg, gpointer data) {
//	Adding noise ==  merging a subdivision HF with the current HF
//	The subdivision HF contains only higher frequencies (lower heights)
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->noise_dialog) {
		hfw->hf_options->img->noise_dialog = noise_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->noise_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->noise_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_noise_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->noise_accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}


/************************* MERGE *************************/

static void compute_merge_preview (hf_struct_type **hf_ptr, unsigned char *buf8, gint width, gint height) {
	//	A function with a generic prototype for calculating the preview buffer
	//	Almost the same as hf_16_to_8
	//	width == height, and width must be a power of 2	
	//	buf8 is already allocated
	gint i, j, display_scale;
	
	hf_struct_type *hf = (hf_struct_type *) *hf_ptr;

	display_scale = log2i(hf->max_x) - log2i(width) ;

//	printf("W -> MAX: %d x %d; display_scale = %d\n",width, hf->max_x, display_scale);
	if (display_scale)
		for (i = 0; i < width; i++)
			for (j = 0; j < height; j++) {
				*(buf8 + j*width + i) =
					*(hf->hf_buf + (LEFT_SHIFT(i,display_scale)) +
					(LEFT_SHIFT(j,display_scale))*hf->max_x) >> 8;
			}
	else
		for (i = 0; i<hf->max_x*hf->max_y; i++)
			*(buf8 + i) = *(hf->hf_buf + i) >> 8;
}

void set_merge_defaults(img_dialog_struct *img) {
//	This function is typically executed when clicking the "accept" button,
//	so we must adjust the "mix" scale so that the last result is kept
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->merge->adj_mix), -100);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->merge->adj_source_offset), 0);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->merge->adj_result_offset), 0);
}

void calc_merge (merge_struct *merge) {
	gint dif;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) merge->data;
	if (!hfw->hf_struct->tmp_buf) {
		hf_backup(hfw->hf_struct);
	}
	if (hfw->if_calculated)
		return;
		
// printf("*************** MERGE of %d (->%d) ***************\n",hfw->hf_struct, hfw->hf_struct->hf_buf);
	
//	We scale up or down the hf to merge with the current one
//	We cannot put this code in the select document to merge callback,
//	because we have to redo it when the current document is swapped
//	hf_to_merge_scaled has been initialized to its default value in select_merge_callb
	if (hfw->hf_options->img->hf_to_merge->max_x != hfw->hf_struct->max_x) {
		dif = log2i(hfw->hf_struct->max_x) - log2i(hfw->hf_options->img->hf_to_merge->max_x);
		if (hfw->hf_options->img->hf_to_merge_scaled)
			hf_free( hfw->hf_options->img->hf_to_merge_scaled);
		hfw->hf_options->img->hf_to_merge_scaled = 
			hf_scale(hfw->hf_options->img->hf_to_merge, dif);
		hfw->hf_options->img->merge_scaled = TRUE;
	}
	set_merge_buffers(merge, 
			hfw->hf_struct->tmp_buf, 
			hfw->hf_options->img->hf_to_merge_scaled->hf_buf, 
			hfw->hf_struct->hf_buf,
			hfw->hf_struct->max_x, 
			hfw->hf_struct->max_y) ;
	simple_merge (merge);
	begin_pending_record(hfw,"Merge",accept_fn, reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->merge_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

void select_merge_callb (GtkWidget *wdg, gpointer data) {
	gchar *lbl;
	GList *node;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	gtk_label_get(GTK_LABEL(GTK_BIN(wdg)->child),&lbl);
//	Find the HF address from its name:
	hfw->hf_options->img->hf_to_merge = NULL;
	for (node = hfw->hf_options->doc_list ; node; node = node->next) {
		if (!strcmp(lbl,*((hf_wrapper_struct *)node->data)->filename)) {
			hfw->hf_options->img->hf_to_merge = 
				(hf_struct_type *) ((hf_wrapper_struct *) node->data)->hf_struct;
			break;
		}
	}
	
	//	 We initialize the hf scaled, as a default (it would be scaled in calc_merge, if required)
	if (hfw->hf_options->img->hf_to_merge_scaled)
		hf_free(hfw->hf_options->img->hf_to_merge_scaled);
	hfw->hf_options->img->hf_to_merge_scaled = hf_copy_new(hfw->hf_options->img->hf_to_merge,FALSE);
	hfw->hf_options->img->merge_scaled = FALSE;

	hfw->if_calculated = FALSE;
	calc_merge(hfw->hf_options->img->merge->content);
	draw_area(hfw->hf_options->img->merge_preview);
}

GtkWidget *simple_merge_dialog_new(gpointer data) {
	GtkWidget *vbox, *vbox2, *frame, *listbox;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Merge");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	hfw->hf_options->img->merging_image_frame = frame_new ("Merging image selection", DEF_PAD*0.5);
	gtk_box_pack_start(GTK_BOX(vbox), hfw->hf_options->img->merging_image_frame, FALSE,FALSE, DEF_PAD*0.5);

	listbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(listbox));
	gtk_box_pack_start(GTK_BOX(vbox), listbox, FALSE,FALSE, DEF_PAD*0.5);
	
	hfw->hf_options->img->merge_listbox = img_listbox_new(listbox);
	refresh_img_listbox(hfw->hf_options->img->merge_listbox, data, select_merge_callb);

// VBOX2 will contain the preview image
	vbox2 = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox2));
	
	gtk_container_add(GTK_CONTAINER(vbox), vbox2);

	hfw->hf_options->img->merge = merge_dialog_struct_new (merge_struct_new(data, (gpointer) draw_hf_ptr));
	gtk_box_pack_start(GTK_BOX(vbox),
		merge_dialog_new (hfw->hf_options->img->merge, FALSE,
			hfw->hf_options->tools_window,
			_("Parameters"), _("Current"), _("Selected"), NULL),
		FALSE, FALSE, DEF_PAD * 0.5);

	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->merge_accept), FALSE, FALSE, DEF_PAD);

	// We initialize the merging HF with the current HF for avoiding seg faults
	// when the user executes the merge without choosing a HF
	hfw->hf_options->img->hf_to_merge = hfw->hf_struct;
	hfw->hf_options->img->hf_to_merge_scaled = hf_copy_new(hfw->hf_struct, FALSE);
	hfw->hf_options->img->merge_scaled = FALSE;

	hfw->hf_options->img->merge_preview =
		view_struct_new(MAX_NOISE_PREVIEW,MAX_NOISE_PREVIEW,(gpointer) compute_merge_preview,(gpointer) &hfw->hf_options->img->hf_to_merge);
	
	gtk_box_pack_start(GTK_BOX(vbox2),
		align_widget(hfw->hf_options->img->merge_preview->area,0.5,0.5),
		TRUE, TRUE, DEF_PAD *0.5);
		
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	draw_area (hfw->hf_options->img->merge_preview);
	return frame;
}

void merge_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
//		When we change options, we commit last changes by backing up
//		the current buffer into the temp buffer
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->merge_dialog) {
		hfw->hf_options->img->merge_dialog = simple_merge_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->merge_dialog );
	}
	else
	//	refresh_img_listbox from hf_filters_dialog.c
		refresh_img_listbox(hfw->hf_options->img->merge_listbox, data, select_merge_callb);
	
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->merge_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_merge_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->merge_accept ;
	set_merge_defaults(hfw->hf_options->img);
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}


/************************* MATHEMATICAL FUNCTIONS *************************/

void apply_math_fn (hf_wrapper_struct *hfw) {
//	gint t1;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
//	t1 = clock();
	hf_math_fn (hfw->hf_struct, 
		hfw->hf_options->img->math_fn,
		hfw->hf_options->img->math_fn_param );
// printf("TEMPS D'APPLICATION DE LA FONCTION: %d\n",clock() - t1);
	begin_pending_record(hfw,"Mathematical transformations", accept_fn, reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->math_fn_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
}

static gint apply_math_fn_callb (GtkWidget *wdg, GdkEventButton *event, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	if (hfw->if_calculated)
//		return;
	apply_math_fn(hfw);
	draw_hf(hfw);
	return FALSE;
}

static gint apply_math_fn_callb2 (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
//	if (hfw->if_calculated)
//		return;
	apply_math_fn(hfw);
	draw_hf(hfw);
	return FALSE;
}


void set_math_fn_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_math_fn), 1.0);
}

void repeat_math_fn_callb (GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	apply_math_fn(hfw);
	draw_hf(hfw);
}

void math_fn_param_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->math_fn_param == (gdouble) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->math_fn_param = (gdouble) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
}

void set_power_fn (GtkWidget *wdg, gpointer data) {
	gint *fn;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	fn = (gint *) data;
	(*fn) = POWER_OP;
}
void set_base_fn (GtkWidget *wdg, gpointer data) {
	gint *fn;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	fn = (gint *) data;
	(*fn) = BASE_OP;
}
void set_log_fn (GtkWidget *wdg, gpointer data) {
	gint *fn;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	fn = (gint *) data;
	(*fn) = LOG_OP;
}
void set_sine_fn (GtkWidget *wdg, gpointer data) {
	gint *fn;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	fn = (gint *) data;
	(*fn) = SINE_OP;
}
GtkWidget *math_fn_dialog_new (gpointer data) {

	GtkWidget *vbox, *hbox, *frame, *scale, *button;
	GSList *group = NULL;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Mathematical transformations");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	//	Radio buttons presenting the functions choice

	button = define_radio_button_in_box_with_data (vbox, &group, "Power (height, parameter)",
		set_power_fn, &hfw->hf_options->img->math_fn,
		(hfw->hf_options->img->math_fn==POWER_OP)) ;
	g_signal_connect (G_OBJECT (button), "toggled", 
		GTK_SIGNAL_FUNC(apply_math_fn_callb2), (gpointer) data);

	button = define_radio_button_in_box_with_data (vbox, &group, "Power (parameter, height / max)",
		set_base_fn, &hfw->hf_options->img->math_fn,
		(hfw->hf_options->img->math_fn==BASE_OP)) ;
	g_signal_connect (G_OBJECT (button), "toggled", 
		GTK_SIGNAL_FUNC(apply_math_fn_callb2), (gpointer) data);

	button = define_radio_button_in_box_with_data (vbox, &group, "Log (height)",
		set_log_fn, &hfw->hf_options->img->math_fn,
		(hfw->hf_options->img->math_fn==LOG_OP)) ;
	g_signal_connect (G_OBJECT (button), "toggled", 
		GTK_SIGNAL_FUNC(apply_math_fn_callb2), (gpointer) data);

	button = define_radio_button_in_box_with_data (vbox, &group, "Sine",
		set_sine_fn, &hfw->hf_options->img->math_fn,
		(hfw->hf_options->img->math_fn==SINE_OP)) ;
	g_signal_connect (G_OBJECT (button), "toggled", 
		GTK_SIGNAL_FUNC(apply_math_fn_callb2), (gpointer) data);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Parameter", hbox, FALSE,TRUE, DEF_PAD);
 	hfw->hf_options->img->adj_math_fn =
		gtk_adjustment_new (hfw->hf_options->img->math_fn_param, 0.0, 10.0, 0.01, 0.01, 0.001);
	scale = define_scale_in_box(hfw->hf_options->img->adj_math_fn,hbox,2,DEF_PAD);

	gtk_signal_connect (GTK_OBJECT (hfw->hf_options->img->adj_math_fn),  "value_changed", GTK_SIGNAL_FUNC (math_fn_param_upd), data);

	g_signal_connect (G_OBJECT (scale), "button_release_event", 
		GTK_SIGNAL_FUNC(apply_math_fn_callb), (gpointer) data);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	button = gtk_button_new_with_label (_("Repeat"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) repeat_math_fn_callb, data);
	gtk_widget_show(button);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(button, 0.5, 0.5), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->math_fn_accept), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void math_fn_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->math_fn_dialog) {
		hfw->hf_options->img->math_fn_dialog = math_fn_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->math_fn_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->math_fn_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_math_fn_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->math_fn_accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* HEXAGONS (GIANT CAUSEWAY) *************************/

void hexagon_remove_tiling (hf_wrapper_struct *hfw) {
	if (hfw->tiling_ptr && (hfw->hf_options->img->hexagon_radius>3)) // Do nothing if nothing has been calculated 
	{
		*hfw->tiling_ptr = FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hfw->tiling_wdg), *hfw->tiling_ptr);
	}
}

void accept_hexagon_fn (hf_wrapper_struct *hfw) {
	hexagon_remove_tiling (hfw);
	accept_fn(hfw);
}

void set_hexagon_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_hexagon_radius), 2);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_hexagon_border), 0);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->hexagon_merge->adj_mix), 100);
}

void calc_hexagon (hf_wrapper_struct *hfw) {
	if (hfw->if_calculated)
		return;
	if (hfw->hf_options->img->hexagon_smooth_radius)
		if (hfw->hf_struct->max_x>hfw->hf_options->dist_matrix->hf_size)
			dist_matrix_init(hfw->hf_options->dist_matrix, hfw->hf_struct->max_x);
	hf_hexagon(hfw->hf_struct, 
		hfw->hf_options->img->hexagon_radius,
		hfw->hf_options->img->hexagon_border,
		hfw->hf_options->img->hexagon_smooth_radius,
		hfw->hf_options->dist_matrix,
		hfw->hf_options->img->hexagon_apply_postprocess,
		hfw->hf_options->gauss_list);
	hfw->hf_options->img->hexagon_apply_postprocess = FALSE;
	begin_pending_record(hfw,"Giant Causeway", accept_hexagon_fn, reset_fn);
	merge_n_display (hfw, hfw->hf_options->img->hexagon_merge->content);
}

void hexagon_radius_upd (GtkWidget *wdg, gpointer data) {
//	Updates the hexagons radius and displays the HF
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->hexagon_radius == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->hexagon_radius = (gint) GTK_ADJUSTMENT(wdg)->value;

	if (hfw->hf_options->img->hexagon_radius<3) {
		reset_callb(wdg,data);
		return;
	}
	hfw->if_calculated = FALSE;
	calc_hexagon (hfw);
}

void hexagon_border_upd (GtkWidget *wdg, gpointer data) {
//	Updates the hexagons border width and displays the HF
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->hexagon_border == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->hexagon_border = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_hexagon (hfw);
}

void hexagon_smooth_radius_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->hexagon_smooth_radius == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->hexagon_smooth_radius = (gint) GTK_ADJUSTMENT(wdg)->value;		
	hfw->if_calculated = FALSE;
//	calc_hexagon (hfw);
}

void hexagon_apply_postprocess_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->img->hexagon_apply_postprocess = TRUE;
	hfw->if_calculated = FALSE;
	calc_hexagon (hfw);
}

void hexagon_remove_tiling_callb (GtkWidget *wdg, gpointer data) {
	// The hexagon transform does not preserve the tiling capability
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hexagon_remove_tiling (hfw);
}

GtkWidget *hexagon_dialog_new(gpointer data) {
	GtkWidget *vbox, *vbox2, *vbox3, *hbox, *frame, *frame2, *button, *scale;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Giant Causeway");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);	// vbox for all options
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD); // vbox for quantization
	gtk_widget_show(vbox2);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("Radius", hbox,FALSE, FALSE, DEF_PAD);
//	Important:  radius must be even, otherwise we get black pixels (rounding problem)
	adj = gtk_adjustment_new (hfw->hf_options->img->hexagon_radius, 2, 100, 2, 2, 0.01);
	scale = define_scale_in_box(adj, hbox,  0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (hexagon_radius_upd), data);
	hfw->hf_options->img->adj_hexagon_radius = adj;

	gtk_box_pack_start(GTK_BOX(vbox2),hbox, TRUE, TRUE, DEF_PAD *0.5);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("Border", hbox,FALSE, FALSE, DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->hexagon_border, 0, 10, 1, 1, 0.01);
	scale = define_scale_in_box(adj, hbox,  0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (hexagon_border_upd), data);
	hfw->hf_options->img->adj_hexagon_border = adj;

	gtk_box_pack_start(GTK_BOX(vbox2),hbox, TRUE, TRUE, DEF_PAD *0.5);

// 	Main "quantization" frame

	frame2 = frame_new("Quantization",DEF_PAD*0.5);
	gtk_container_add(GTK_CONTAINER(frame2), vbox2);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, TRUE, TRUE, 0); // the main box

/********************** Post-process subdialogs *************************/ 

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD); // vbox for subdialogs
	gtk_widget_show(vbox2);

//	Hexagons' smooth

	vbox3 = gtk_vbox_new(FALSE,DEF_PAD); // vbox for subdialogs
	gtk_widget_show(vbox3);	
	
	frame2 = define_frame_with_hiding_arrows ("Smooth", vbox3,  hfw->hf_options->tools_window, FALSE);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("Radius", hbox,FALSE,FALSE, DEF_PAD);
 	adj = gtk_adjustment_new (hfw->hf_options->img->hexagon_smooth_radius, 0.0, 20.0, 1.0, 1.0, 0.0);
	define_scale_in_box(adj, hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (hexagon_smooth_radius_upd), data);
	hfw->hf_options->img->adj_hexagon_smooth_radius = adj;
	
	gtk_box_pack_start(GTK_BOX(vbox3), hbox, TRUE, TRUE, DEF_PAD*0.5);

/********************** Smooth subdialog end ****************************/

	button = gtk_button_new_with_label (_("Apply"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) hexagon_apply_postprocess_callb, data);
	gtk_widget_show(button);

  	gtk_box_pack_start (GTK_BOX (vbox3), align_widget(button,0.5,0.5), FALSE, FALSE, DEF_PAD*0.5);

	gtk_box_pack_start(GTK_BOX(vbox2), frame2, TRUE, TRUE, 0);
	
// 	Post-process frame

	frame2 = define_frame_with_hiding_arrows ("Result post-processing", vbox2,  hfw->hf_options->tools_window, TRUE);

	gtk_box_pack_start(GTK_BOX(vbox), frame2, TRUE, TRUE, 0); // the main box

/********************** Post-process frame end ****************************/

// 	Merge

	hfw->hf_options->img->hexagon_merge = merge_dialog_struct_new (merge_struct_new(data, (gpointer) draw_hf_ptr));
	hfw->hf_options->img->hexagon_merge->content->mix = 100;
	frame2 = merge_dialog_new(hfw->hf_options->img->hexagon_merge,
		TRUE, hfw->hf_options->tools_window, NULL, NULL, NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), frame2, FALSE, FALSE, 0);
	gtk_widget_hide (GTK_WIDGET(hfw->hf_options->img->hexagon_merge->main_box));

// 	Control buttons

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label(_("Reset"));
	gtk_widget_show(GTK_WIDGET(button));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) reset_callb, data);
	gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Accept"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) hexagon_remove_tiling_callb, data);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) accept_callb, data);
	gtk_widget_show(button);	
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	hfw->hf_options->img->hexagon_accept = button;

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);

	return frame;	
}


void hexagon_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->hexagon_dialog) {
		hfw->hf_options->img->hexagon_dialog = hexagon_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->hexagon_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->hexagon_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_hexagon_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->hexagon_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* LIFT_EDGES *************************/

void set_lift_edges_defaults(img_dialog_struct *img) {
//	printf("SET_LIFT_EDGES_DEFAULTS\n");
	img->lift_edges_merge_done = TRUE;
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_lift_edges_radius), 20);
//	gtk_adjustment_set_value(
//			GTK_ADJUSTMENT(img->adj_lift_edges_level), 50);
	gtk_widget_hide(GTK_WIDGET(img->lift_edges_level_dialog));
}

void calc_lift_edges (hf_wrapper_struct *hfw) {
	gint t1;
	gboolean wrap;
//	printf("CALC_LIFT_EDGES\n");
	if (hfw->if_calculated)
		return;
	if (hfw->hf_struct->max_x>hfw->hf_options->dist_matrix->hf_size)
		dist_matrix_init(hfw->hf_options->dist_matrix, hfw->hf_struct->max_x);
	t1 = clock();
	set_watch_cursor(hfw);
	switch (hfw->hf_options->img->smooth_wrap) {
		case TILING_AUTO:
			wrap = *hfw->tiling_ptr;
			break;
		case TILING_YES:
			wrap = TRUE;
			break;
		case TILING_NO:
			wrap = FALSE;
	}

	// Steps:
	// 1. Decrease the contrast (keeping the "black point")
	// 2. Smooth
	// 3. Subtract the smooth result from the lower contrast source
	// These 3 steps are done in lift_edges
	// 4. Add the subtraction result to the original buffer (before lowering contrast)
	hf_min_max(hfw->hf_struct);
	hf_backup(hfw->hf_struct);
	// We need a second temporary buffer for storing the result
	// of the smoothing process before the postprocessing (== merge source with result)
	if (!hfw->hf_struct->result_buf) {
		hfw->hf_struct->result_buf =
			(hf_type *)x_malloc(hfw->hf_struct->max_x*hfw->hf_struct->max_y*sizeof(hf_type), "hf_type (result_buf in calc_lift_edges)");
	}
	lift_edges (hfw->hf_struct->hf_buf, 
		hfw->hf_struct->result_buf, 
		hfw->hf_struct->max_x, 
		hfw->hf_struct->max_y, 
		hfw->hf_options->img->lift_edges_radius,
		hfw->hf_options->dist_matrix,
		wrap,
		hfw->hf_options->gauss_list,
		hfw->hf_options->img->lift_edges_use_black_point);

	// At this point,
	// 	tmp_buf == the original buffer
	//	result_buf == the buffer with the lifted edges
	//	hf_buf == the output buffer, to display
//	swap_buffers(&hfw->hf_struct->hf_buf, 
//		&hfw->hf_struct->result_buf);

	hfw->hf_options->img->lift_edges_merge_done = FALSE;
	
	merge_lift_edges (hfw->hf_struct, hfw->hf_options->img->lift_edges_level); 

	gtk_widget_show(GTK_WIDGET(hfw->hf_options->img->lift_edges_level_dialog));
	
// printf("TEMPS DE RELÈVEMENT DES BORDS: %d\n",clock() - t1);
	begin_pending_record(hfw,"Lift edges", accept_fn, reset_fn);

	unset_watch_cursor(hfw);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	gtk_widget_set_sensitive(GTK_WIDGET( hfw->hf_options->img->accept_wdg),TRUE);
	draw_hf (hfw);
}

void lift_edges_level_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	printf("LIFT_EDGES_LEVEL_UPD!\n");
	if (hfw->hf_options->img->lift_edges_level == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->lift_edges_level = (gint) GTK_ADJUSTMENT(wdg)->value;
	
	if (hfw->hf_options->img->lift_edges_merge_done)
		return;
		
	merge_lift_edges (hfw->hf_struct, hfw->hf_options->img->lift_edges_level);

	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf (hfw);
}

void apply_lift_edges_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	calc_lift_edges(hfw);
}

void repeat_lift_edges_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	calc_lift_edges(hfw);
}

GtkWidget *lift_edges_dialog_new(gpointer data) {
	GtkWidget *vbox, *hbox, *frame, *wdg;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	img_dialog_struct *img;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	img = hfw->hf_options->img;
	
	frame = options_frame_new("Lift edges");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);

	define_label_in_box("Radius",hbox,FALSE, FALSE,DEF_PAD);
	adj = gtk_adjustment_new (img->lift_edges_radius, 0, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &img->lift_edges_radius);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);
	img->adj_lift_edges_radius = adj;
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	define_label_in_box("Level",hbox,FALSE, FALSE,DEF_PAD);
	adj = gtk_adjustment_new (img->lift_edges_level, 0, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox, 0, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed", GTK_SIGNAL_FUNC (lift_edges_level_upd), data);
	img->adj_lift_edges_level = adj;
	img->lift_edges_level_dialog = hbox;
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
		
	wdg = define_check_button_in_box ("Use a threshold for black", hbox, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wdg), img->lift_edges_use_black_point);
	gtk_signal_connect (GTK_OBJECT(wdg), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &img->lift_edges_use_black_point);
	
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	hbox = tiling_control(&img->lift_edges_wrap);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);
	
  	gtk_box_pack_start (GTK_BOX (vbox), apply_repeat_buttons_new (data, apply_lift_edges_callb, repeat_lift_edges_callb), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &img->lift_edges_accept), FALSE, FALSE, DEF_PAD);

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void lift_edges_callb(GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->lift_edges_dialog) {
		hfw->hf_options->img->lift_edges_dialog = lift_edges_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->lift_edges_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->lift_edges_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_lift_edges_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->lift_edges_accept ;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}
