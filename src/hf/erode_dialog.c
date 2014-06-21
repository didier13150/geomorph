/* erode_dialog.c:  dialogs for different kinds of erosion and cresting,
 * including craters.
 * Works with img_process_dialog...
 *
 * Copyright (C) 2003 Patrice St-Gelais
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

// get_current_dir_name() is only prototyped if _GNU_SOURCE is defined
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>
#include <unistd.h>

#include "img_process_dialog.h"
#include "hf_wrapper.h"
#include "erode_dialog.h"
#include "erode.h"
#include "draw_crater_dialog.h"
#include "dialog_utilities.h"

//	Icons for N/S/W/E direction
#include "../icons/north.xpm"
#include "../icons/south.xpm"
#include "../icons/east.xpm"
#include "../icons/west.xpm"

static gint frame_counter;

extern gchar *HF_OUTPUT_FOR_RENDERING, *OUTPUT_PREFIX, *TMP_DIR;

/************************* Constructors / destructors *************************/

whimsical_erosion_dialog_struct *whimsical_erosion_dialog_struct_new() {
	whimsical_erosion_dialog_struct *weds;
	weds = (whimsical_erosion_dialog_struct *) x_malloc(sizeof(whimsical_erosion_dialog_struct), "whimsical_erosion_dialog_struct");
	weds->dialog = NULL;
	weds->accept = NULL;
	weds->adj_radius = NULL;
	weds->adj_smooth_ribs = NULL;
	weds->merge = NULL;
	
	return weds;
}

void whimsical_erosion_dialog_struct_free(whimsical_erosion_dialog_struct *wesd) {
	if (!wesd)
		return;
	if (wesd->merge)
		x_free(wesd->merge);
	x_free(wesd);
}

rain_erosion_dialog_struct *rain_erosion_dialog_struct_new () {
	rain_erosion_dialog_struct *reds;
	reds = (rain_erosion_dialog_struct *) x_malloc(sizeof(rain_erosion_dialog_struct),"rain_erosion_dialog_struct");
	reds->dialog = NULL;
	reds->accept = NULL;
	reds->adj_drops = NULL;
	reds->adj_threshold = NULL;
	reds->adj_hardness = NULL;
	reds->adj_strength = NULL;
	reds->adj_interval = NULL;
	reds->refresh = NULL;
	reds->old_algorithm = NULL;
	
	return reds;
}

void rain_erosion_dialog_struct_free (rain_erosion_dialog_struct* reds) {
	if (reds)
		x_free(reds);
}

gravity_dialog_struct *gravity_dialog_struct_new () {
	gravity_dialog_struct * gds;
	gds = (gravity_dialog_struct *) x_malloc(sizeof(gravity_dialog_struct),"gravity_dialog_struct");
	gds->dialog = NULL;
	gds->accept = NULL;
	gds->adj_steps = NULL;
	gds->adj_threshold = NULL;
	gds->merge = NULL;
	
	return gds;
}

void gravity_dialog_struct_free (gravity_dialog_struct* gds) {
	if (gds)
		x_free(gds);
}

oriented_gravity_dialog_struct *oriented_gravity_dialog_struct_new () {
	oriented_gravity_dialog_struct * gds;
	gds = (oriented_gravity_dialog_struct *) x_malloc(sizeof(oriented_gravity_dialog_struct), "oriented_gravity_dialog_struct");
	gds->dialog = NULL;
	gds->accept = NULL;
	gds->adj_steps = NULL;
	gds->adj_threshold = NULL;
	gds->wdg_direction = NULL;
	gds->merge = NULL;
	
	return gds;
}

void oriented_gravity_dialog_struct_free (oriented_gravity_dialog_struct* gds) {
	if (gds)
		x_free(gds);
}

craters_erosion_dialog_struct*  craters_erosion_dialog_struct_new() {
	craters_erosion_dialog_struct *ceds;
	ceds = (craters_erosion_dialog_struct *) x_malloc( sizeof(craters_erosion_dialog_struct), "craters_erosion_dialog_struct");
	
	ceds->dialog = NULL;
	ceds->accept = NULL;
	ceds->adj_qty = NULL;
	ceds->adj_peak_threshold = NULL;
	ceds->adj_slope_threshold = NULL;
	return ceds;
}

void craters_erosion_dialog_struct_free (craters_erosion_dialog_struct *ceds) {
	if (!ceds)
		return;
	x_free(ceds);
}
/************************* ERODED RIBS (Whimsical erosion) *************************/

void apply_whimsical (hf_wrapper_struct *hfw) {

	//gint t1;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
	//	We reapply the process on the original buffer (tmp_buf)
	//	But the process take hf_buf as the source	
	memcpy (hfw->hf_struct->hf_buf,hfw->hf_struct->tmp_buf,
		sizeof(hf_type)*hfw->hf_struct->max_x*hfw->hf_struct->max_y);
	//t1 = clock();
	// Computing MIN/MAX is required for applying the auto-contrast flag in "merge_erode"
	hf_min_max(hfw->hf_struct);
	if (hfw->hf_options->img->whimsical_erosion->radius)
		hf_smooth(hfw->hf_struct, hfw->hf_options->img->whimsical_erosion->radius, 
			hfw->hf_options->dist_matrix, *hfw->tiling_ptr,
			hfw->hf_options->gauss_list);
	if (!hfw->hf_struct->result_buf)
		hfw->hf_struct->result_buf = 
			(hf_type *) x_malloc(sizeof(hf_type) * hfw->hf_struct->max_x  * hfw->hf_struct->max_y, "hf_type (result_buf in apply_whimsical)");

	hf_eroded_ribs(hfw->hf_struct->hf_buf, hfw->hf_struct->result_buf, hfw->hf_struct->max_x);
	hf_clamp_buffer (hfw->hf_struct->result_buf, hfw->hf_struct->max_x * hfw->hf_struct->max_y, 0x3FFF,0x5FFF );
	if (hfw->hf_options->img->whimsical_erosion->smooth_ribs) {
	//	Some smoothing is needed to remove square artifacts (~radius = 2)
	//	Since hf_smooth operates on hf->hf_buf, and our current result is in hf->result_buf,
	//	we need to swap the buffers
		swap_buffers ((gpointer*) &hfw->hf_struct->result_buf, (gpointer*) &hfw->hf_struct->hf_buf);
		hf_smooth(hfw->hf_struct, hfw->hf_options->img->whimsical_erosion->smooth_ribs, 
			hfw->hf_options->dist_matrix, *hfw->tiling_ptr,
			hfw->hf_options->gauss_list);
		swap_buffers ((gpointer*) &hfw->hf_struct->result_buf, (gpointer*) &hfw->hf_struct->hf_buf);
	}

// printf("TEMPS D'ÉROSION: %d\n",clock() - t1);
}

static void merge_whimsical_postprocess (gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->whimsical_erosion->auto_contrast)
		hf_clamp_buffer (hfw->hf_struct->hf_buf, hfw->hf_struct->max_x *  hfw->hf_struct->max_y, hfw->hf_struct->min,hfw->hf_struct->max);
	draw_hf(hfw);
}

static void merge_whimsical (merge_struct *ms) {
	simple_merge(ms);
	merge_whimsical_postprocess (ms->data);
}

static void change_contrast_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw)
		merge_whimsical (hfw->hf_options->img->whimsical_erosion_dialog->merge->content);
}

void apply_whimsical_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->if_calculated)
		return;
	apply_whimsical(hfw);
	set_merge_buffers (	hfw->hf_options->img->whimsical_erosion_dialog->merge->content,
				hfw->hf_struct->tmp_buf, 
				hfw->hf_struct->result_buf,
				hfw->hf_struct->hf_buf, 
				hfw->hf_struct->max_x, 
				hfw->hf_struct->max_y);
	merge_whimsical (hfw->hf_options->img->whimsical_erosion_dialog->merge->content);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	begin_pending_record(hfw,"Whimsical erosion",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->whimsical_erosion_dialog->accept),TRUE);
}

void set_whimsical_defaults(img_dialog_struct *img) {
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->adj_erode_radius), 0);
}

GtkWidget *whimsical_erosion_dialog_new (gpointer data) {

	GtkWidget *vbox, *hbox, *frame, *frame2, *button;
	whimsical_erosion_struct *wes;
	whimsical_erosion_dialog_struct *weds;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	wes = hfw->hf_options->img->whimsical_erosion;
	weds = hfw->hf_options->img->whimsical_erosion_dialog;
	
	frame = options_frame_new("Whimsical erosion");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Radius", hbox, FALSE,TRUE, DEF_PAD);
 	weds->adj_radius = gtk_adjustment_new (
		wes->radius, 0.0, 100, 1.0, 1.0, 0.0);
	define_scale_in_box(weds->adj_radius,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (weds->adj_radius), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &wes->radius);
	gtk_signal_connect (GTK_OBJECT (weds->adj_radius), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Rib smoothing", hbox, FALSE,TRUE, DEF_PAD);
 	weds->adj_smooth_ribs = gtk_adjustment_new (wes->smooth_ribs, 0.0, 10, 1.0, 1.0, 0.0);
	define_scale_in_box(weds->adj_smooth_ribs,hbox,0,DEF_PAD);
	
	gtk_signal_connect (GTK_OBJECT (weds->adj_smooth_ribs), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &wes->smooth_ribs);
	gtk_signal_connect (GTK_OBJECT (weds->adj_smooth_ribs), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

//	Contrast control
	button = gtk_check_button_new_with_label (_("Automatic contrast"));
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(vbox), 
		align_widget(button,0.5,0.5), TRUE, TRUE, DEF_PAD*0.5);
	gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (button ), wes->auto_contrast);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
			GTK_SIGNAL_FUNC(toggle_check_button_callb),
			&(wes->auto_contrast ));
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
			GTK_SIGNAL_FUNC(change_contrast_callb), data);

	button = gtk_button_new_with_label (_("Apply"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) apply_whimsical_callb, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(vbox), align_widget(button,0.5,0.5), FALSE, FALSE, DEF_PAD);

// 	Merge

	weds->merge = merge_dialog_struct_new (merge_struct_new(data, (gpointer) merge_whimsical_postprocess));
	weds->merge->content->mix = 0;
	frame2 = merge_dialog_new(weds->merge,
		TRUE, hfw->hf_options->tools_window, NULL, NULL, NULL, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0);
//	gtk_widget_hide (GTK_WIDGET(hfw->hf_options->img->erode_merge->main_box));

	// Controls
		
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &weds->accept), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void whimsical_erosion_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	whimsical_erosion_dialog_struct *weds;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	
	weds = hfw->hf_options->img->whimsical_erosion_dialog;
	if (!weds->dialog) {
		weds->dialog = whimsical_erosion_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			weds->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = weds->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_whimsical_defaults;
	hfw->hf_options->img->accept_wdg = weds->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* RAIN EROSION *************************/

static void zero_percent (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), 0.0);
}

static void hundred_percent (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), (gfloat) MAX_DROPS);
}

static void plus_10_percent (GtkWidget *wdg, gpointer data) {
	long int value;
	value = (long int) GTK_ADJUSTMENT(data)->value;
	value = (value + (long int) (0.1* (gfloat) MAX_DROPS)) % (long int) MAX_DROPS;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
}

static void plus_25_percent (GtkWidget *wdg, gpointer data) {
	long int value;
	value = (long int) GTK_ADJUSTMENT(data)->value;
	value = (value + (long int) (0.25* (gfloat) MAX_DROPS)) % (long int) MAX_DROPS;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
}

static void erode_percent_buttons_new(GtkWidget *box, gpointer *adj) {
//	Returns 0, +10, +25 % buttons in the given hbox or vbox
//	The buttons activate an adjustment, which is passed to the callbacks functions
//	It "wraps" (90 + 25 = 15%)

	GtkWidget *button;
	button = gtk_button_new_with_label (" 0 ");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) zero_percent, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
	button = gtk_button_new_with_label ("+10%");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_10_percent, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
	button = gtk_button_new_with_label ("+25%");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_25_percent, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
	button = gtk_button_new_with_label ("100%");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) hundred_percent, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
}

void draw_n_write_hf (hf_wrapper_struct *hfw) {
	gchar *fname=NULL, *ext;
	static gchar frame_string[6];
	draw_hf(hfw);
	sprintf(frame_string,"%05d",frame_counter);
	fname = (gchar *) x_malloc(strlen(*hfw->filename)+7, "gchar (fname in draw_n_write_hf)");
	strcpy(fname,*hfw->filename);
	ext = strstr(fname,".png");
	if (!ext)
		ext = strstr(fname,".PNG");
	if (!ext)
		return;
	strcpy(ext, frame_string);
	strcpy(ext+strlen(frame_string),".png");
	printf("Fichier: %s\n",fname);
	hf_wrapper_save(hfw, fname);
	frame_counter++;
	if (fname)
		x_free(fname);
}

void apply_rain_erosion (hf_wrapper_struct *hfw) {
	//gint t1;
	gchar *dir;
	gpointer fn;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
//	t1 = clock();
	set_watch_cursor(hfw);
	// Frame counter is a global variable
	frame_counter = 0;
	dir = (gchar *) get_current_dir_name();
	chdir(TMP_DIR);
	if (hfw->hf_options->img->rain_erosion->gener_frames)
		fn = (gpointer) draw_n_write_hf;
	else
		fn = (gpointer) draw_hf;

	if (hfw->hf_options->img->rain_erosion->old_algorithm)
		hf_rain_erosion_hex_old(hfw->hf_struct, 
			hfw->hf_options->img->rain_erosion,
			fn, (gpointer) hfw) ;
	else
		hf_rain_erosion_hex(hfw->hf_struct, 
			hfw->hf_options->img->rain_erosion,
			fn, (gpointer) hfw) ;
/*	
	hf_water_erosion(hfw->hf_struct,
		hfw->hf_options->img->rain_erosion->drops / 1000,
		hfw->hf_options->img->rain_erosion->threshold) ;
*/	
// printf("TEMPS D'ÉROSION PAR LA PLUIE: %d\n",clock() - t1);
	begin_pending_record(hfw,"Water erosion",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->rain_erosion_dialog->accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	unset_watch_cursor(hfw);
	chdir(dir);
	x_free(dir);
}

void apply_erode_rain_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	We reapply the process on the source
//	(it works on hf-hf_buf, without copying tmp_buf to hf_buf);
	hf_restore(hfw->hf_struct);
//	printf("APPLY RAIN EROSION - HFW: %d - x%x; IF_CALCULATED: %d\n",hfw, hfw, hfw->if_calculated);
	if (hfw->if_calculated)
		return;
	apply_rain_erosion(hfw);
	draw_hf(hfw);
}

void set_erode_rain_defaults(img_dialog_struct *img) {
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->adj_erode_rain_drops), 0);
}

void repeat_erode_rain_callb (GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	apply_rain_erosion(hfw);
	draw_hf(hfw);
}

GtkWidget *rain_erosion_dialog_new (gpointer data) {

	GtkWidget *vbox, *hbox, *frame, *hbox1, *hbox2;
	hf_wrapper_struct *hfw;
	rain_erosion_struct *res;
	rain_erosion_dialog_struct *reds;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	res = hfw->hf_options->img->rain_erosion;
	reds = hfw->hf_options->img->rain_erosion_dialog;
	
	frame = options_frame_new("Water erosion");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Drops", hbox, FALSE,TRUE, DEF_PAD);
 	reds->adj_drops =
		gtk_adjustment_new (res->drops, 0.0, MAX_DROPS, 100.0, 1000.0, 0.0);
	define_scale_in_box(reds->adj_drops,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (reds->adj_drops), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &res->drops);
	gtk_signal_connect (GTK_OBJECT (reds->adj_drops), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);
	
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	erode_percent_buttons_new (hbox, (gpointer) reds->adj_drops);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Threshold", hbox, FALSE,TRUE, DEF_PAD);
 	reds->adj_threshold = 
		gtk_adjustment_new (res->threshold, 1.0, 80.0, 1.0, 1.0, 0.0);
	define_scale_in_box(reds->adj_threshold,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (reds->adj_threshold), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &res->threshold);
	gtk_signal_connect (GTK_OBJECT (reds->adj_threshold), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
		
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Strength", hbox, FALSE,TRUE, DEF_PAD);
 	reds->adj_strength = 
		gtk_adjustment_new (res->strength, 1.0, 100.0, 1.0, 1.0, 0.0);
	define_scale_in_box(reds->adj_strength,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (reds->adj_strength), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &res->strength);
	gtk_signal_connect (GTK_OBJECT (reds->adj_strength), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	hbox1 = hbox;
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Hardness", hbox, FALSE,TRUE, DEF_PAD);
 	reds->adj_hardness = 
		gtk_adjustment_new (res->hardness, 1.0, 100.0, 1.0, 1.0, 0.0);
	define_scale_in_box(reds->adj_hardness,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (reds->adj_hardness), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &res->hardness);
	gtk_signal_connect (GTK_OBJECT (reds->adj_hardness), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	hbox2 = hbox;
			
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
	
	reds->refresh = define_check_button_in_box ("Progressive refresh?", hbox, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (reds->refresh), res->refresh);
	gtk_signal_connect (GTK_OBJECT(reds->refresh), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &res->refresh);
	
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
		
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
		
	define_label_in_box("Interval", hbox, FALSE,TRUE, DEF_PAD);
 	reds->adj_interval = 
		gtk_adjustment_new (res->interval, 100.0, 10000.0, 1.0, 1.0, 0.0);
	define_scale_in_box(reds->adj_interval,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (reds->adj_interval), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &res->interval);
	gtk_signal_connect (GTK_OBJECT (reds->adj_interval), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	gtk_signal_connect (GTK_OBJECT (reds->refresh), "toggled",
	       (GtkSignalFunc) show_if_true, (gpointer) hbox);
	       	
	if (res->refresh==FALSE)
		gtk_widget_hide(GTK_WIDGET(hbox));
			
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
	
	reds->gener_frames = define_check_button_in_box ("Generate frames?", hbox, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (reds->gener_frames), res->gener_frames);
	gtk_signal_connect (GTK_OBJECT(reds->gener_frames), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &res->gener_frames);
	
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	gtk_signal_connect (GTK_OBJECT (reds->refresh), "toggled",
	       (GtkSignalFunc) show_if_true, (gpointer) hbox);
	       	
	if (res->refresh==FALSE)
		gtk_widget_hide(GTK_WIDGET(hbox));

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
	
	reds->old_algorithm = define_check_button_in_box ("Use old algorithm (< v.0.4)", hbox, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (reds->old_algorithm), res->old_algorithm);
	gtk_signal_connect (GTK_OBJECT(reds->old_algorithm), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &res->old_algorithm);
	
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	gtk_signal_connect (GTK_OBJECT (reds->old_algorithm), "toggled",
	       (GtkSignalFunc) hide_if_true, (gpointer) hbox1);
	gtk_signal_connect (GTK_OBJECT (reds->old_algorithm), "toggled",
	       (GtkSignalFunc) hide_if_true, (gpointer) hbox2);
	       	      	
	if (res->old_algorithm) {
		gtk_widget_hide(GTK_WIDGET(hbox1));
		gtk_widget_hide(GTK_WIDGET(hbox2));
	}
	// Controls
	
  	gtk_box_pack_start (GTK_BOX (vbox), apply_repeat_buttons_new (data, apply_erode_rain_callb, repeat_erode_rain_callb), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &reds->accept), FALSE, FALSE, DEF_PAD);

	return frame;
}

void rain_erosion_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->rain_erosion_dialog->dialog) {
		hfw->hf_options->img->rain_erosion_dialog->dialog = rain_erosion_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->rain_erosion_dialog->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->rain_erosion_dialog->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_erode_rain_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->rain_erosion_dialog->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* CRATERS *************************/


void apply_craters (hf_wrapper_struct *hfw) {
	gint qty_drawn, data_size; // t1,  
	static gchar *buf=NULL;
	craters_erosion_struct *ces;
	ces = hfw->hf_options->img->craters_erosion;
	
	if (strcmp(hfw->last_action,_("Craters"))) {
		// It means we are beginning a new "craters transaction" for this HF
		// tmp_buf allows to rollback the whole transaction (with all repeats)
		// with the "reset" button
		hf_backup(hfw->hf_struct);
	}
	// "result_buf" is used as the most recent backend buffer
	// Allows to repeat the "crater rain"
	data_size = sizeof(hf_type)*hfw->hf_struct->max_x* hfw->hf_struct->max_y;
	if (!hfw->hf_struct->result_buf)
		hfw->hf_struct->result_buf = (hf_type *) x_malloc(data_size, "hf_type (result_buf in apply_craters)");
	memcpy(hfw->hf_struct->result_buf,hfw->hf_struct->hf_buf, data_size);
	//t1 = clock();
	qty_drawn = 
	        draw_many_craters (ces->crater_str,
		hfw->hf_struct,
		ces->qty,
		ces->peak_threshold,
		ces->slope_threshold,
		hfw->hf_options->gauss_list) ;
	if (ces->qty != qty_drawn) {
		if (!buf)
			buf = (gchar *) x_malloc(10+strlen(_("n%2d craters drawn over %2dnIncrease your slope threshold to draw more")), "const gchar (buf in apply_craters)");
		sprintf(buf,_("n%2d craters drawn over %2dnIncrease your slope threshold to draw more"),qty_drawn, ces->qty);
		my_msg(buf, WARNING);
	}

// printf("TEMPS DE GÉNÉRATION DES CRATÈRES: %d\n",clock() - t1);
	begin_pending_record(hfw,"Craters",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->craters_erosion_dialog->accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
}

void set_craters_defaults(img_dialog_struct *img) {
//	gtk_adjustment_set_value(
//			GTK_ADJUSTMENT(img->craters_qty, 10);
}

static gint change_craters_seed(GtkWidget *entry, gpointer data) {
	if (data)
		*((gint*) data) =
			(gint) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry)));
	return FALSE;
}

void apply_craters_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	We reapply the process on the source
//	(it works on hf->hf_buf, without copying tmp_buf to hf_buf);
// printf("HFW in apply_craters_callb: %d\n",hfw);
	hf_restore(hfw->hf_struct);
//	if (hfw->if_calculated)
//		return;
	apply_craters(hfw);
	draw_hf(hfw);
}

void repeat_craters_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
// printf("HFW in repeat_craters_callb: %d\n",hfw);
	hfw->if_calculated = FALSE;
	apply_craters(hfw);
	draw_hf(hfw);
}

GtkWidget *craters_erosion_dialog_new(gpointer data) {
	GtkWidget *hbox, *vbox, *vbox2, *vbox3, *frame, *frame2;
	hf_wrapper_struct *hfw;
	craters_erosion_struct *ces;
	craters_erosion_dialog_struct *ceds;
	
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	ces = hfw->hf_options->img->craters_erosion;
	ceds =  hfw->hf_options->img->craters_erosion_dialog;

	frame = options_frame_new("Craters");

	vbox = gtk_vbox_new(FALSE,DEF_PAD*0.5);
	gtk_widget_show(GTK_WIDGET(vbox));
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, DEF_PAD);

	define_label_in_box("Quantity", hbox, FALSE,TRUE, DEF_PAD);
 	ceds->adj_qty = gtk_adjustment_new (ces->qty, 1.0, 50, 1.0, 1.0, 0.01);
	define_scale_in_box(ceds->adj_qty,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (ceds->adj_qty), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &ces->qty);
	gtk_signal_connect (GTK_OBJECT (ceds->adj_qty), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);
	
	gtk_box_pack_start(GTK_BOX(vbox), 
		tiling_control((gpointer) &ces->crater_str->wrap), TRUE, TRUE, DEF_PAD);

//	Define a frame with small arrows for hiding / showing the dialog...

	frame2 = frame_new("Diameter (% of HF)",DEF_PAD);
	gtk_box_pack_start(GTK_BOX(vbox),frame2, TRUE, TRUE, DEF_PAD *0.5);

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox2));

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox3);
	gtk_container_add(GTK_CONTAINER(frame2), vbox3);

	gtk_box_pack_start(GTK_BOX(vbox3), 
		hideshow_dialog_new(hfw->hf_options->tools_window,
			GTK_ORIENTATION_HORIZONTAL, vbox2,NULL), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox3), vbox2, FALSE, FALSE, 0);
	gtk_widget_hide(vbox2);

	crater_boundaries_dialog(vbox2, ces->crater_str );
	gtk_box_pack_start(GTK_BOX(vbox2), 
		seed_dialog_new( &ces->crater_str->seed, ces->crater_str->seed, change_craters_seed), TRUE, TRUE, DEF_PAD*0.5);

	define_label_in_box ("Central peak from...", vbox2, FALSE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
  	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
	define_label_in_box("%", hbox, FALSE,TRUE, DEF_PAD);
 	ceds->adj_peak_threshold = 
		gtk_adjustment_new (ces->peak_threshold, 0.0, 100, 1.0, 1.0, 0.01);
	define_scale_in_box(ceds->adj_peak_threshold,hbox,0,DEF_PAD);
	
	gtk_signal_connect (GTK_OBJECT (ceds->adj_peak_threshold), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &ces->peak_threshold);
	gtk_signal_connect (GTK_OBJECT (ceds->adj_peak_threshold), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

//	Depth and noise
//	Another frame with small arrows for hiding / showing the dialog...

	frame2 = frame_new("Depth and noise",DEF_PAD);
	gtk_box_pack_start(GTK_BOX(vbox),frame2, TRUE, TRUE, DEF_PAD *0.5);

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox2));

	vbox3 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox3);
	gtk_container_add(GTK_CONTAINER(frame2), vbox3);

	gtk_box_pack_start(GTK_BOX(vbox3), 
		hideshow_dialog_new(hfw->hf_options->tools_window,
			GTK_ORIENTATION_HORIZONTAL, vbox2,NULL), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox3), vbox2, FALSE, FALSE, 0);
	gtk_widget_hide(vbox2);

	crater_depth_n_noise_dialog(vbox2, ces->crater_str );

//	Slope threshold

	frame2 = frame_new("Draw if slope less than..." ,DEF_PAD);
	gtk_box_pack_start(GTK_BOX(vbox),frame2, TRUE, TRUE, DEF_PAD *0.5);

	vbox2 = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox2));
	gtk_container_add(GTK_CONTAINER(frame2), vbox2);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_hide(GTK_WIDGET(hbox));

  	gtk_box_pack_start (GTK_BOX (vbox2),
		hideshow_dialog_new(hfw->hf_options->tools_window,
			GTK_ORIENTATION_HORIZONTAL, hbox, NULL)	, FALSE, FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (vbox2),hbox, FALSE, FALSE, 0);
	define_label_in_box("%", hbox, FALSE,TRUE, DEF_PAD);
 	ceds->adj_slope_threshold = gtk_adjustment_new (ces->slope_threshold,
		 0.0, MAX_SLOPE_THRESHOLD, 1.0, 1.0, 0.01);
	define_scale_in_box(ceds->adj_slope_threshold,hbox,0,DEF_PAD);

	gtk_signal_connect (GTK_OBJECT (ceds->adj_slope_threshold), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &ces->slope_threshold);
	gtk_signal_connect (GTK_OBJECT (ceds->adj_slope_threshold), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

//	Controls

  	gtk_box_pack_start (GTK_BOX (vbox), apply_repeat_buttons_new (data, apply_craters_callb, repeat_craters_callb), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &ceds->accept), FALSE, FALSE, DEF_PAD);


	return frame;
}

void craters_erosion_callb(GtkWidget *wdg, gpointer data) {

	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->craters_erosion_dialog->dialog) {
		hfw->hf_options->img->craters_erosion_dialog->dialog = craters_erosion_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->craters_erosion_dialog->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->craters_erosion_dialog->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_craters_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->craters_erosion_dialog->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/********************* Common gravity functions *******************/

void set_merge_gravity_defaults (merge_dialog_struct *merge) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) merge->content->data;
	if (hfw->hf_struct)
		if (hfw->hf_struct->result_buf) {
			x_free(hfw->hf_struct->result_buf);
			hfw->hf_struct->result_buf = NULL;
		}
	merge_set_mix(merge, 100);
	merge_set_op(merge, MAX_MERGE);
}

/************************* CRESTS *************************/

GtkWidget *crests_gravity_dialog_new (gpointer data, gchar *title, 
	gravity_struct *cs, gravity_dialog_struct *cds, gboolean *if_calculated,
	GtkWidget *parent_window,
	void (*apply_fn) (GtkWidget *, gpointer), 
	void (*repeat_fn) (GtkWidget *, gpointer) ) {
	
	GtkWidget *vbox, *hbox, *frame;
	
	frame = options_frame_new(title);

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Steps", hbox, FALSE,TRUE, DEF_PAD);
 	cds->adj_steps = gtk_adjustment_new (cs->steps, 0.0, 100, 1.0, 1.0, 0.0);
	define_scale_in_box(cds->adj_steps,hbox,0,DEF_PAD);	
	gtk_signal_connect (GTK_OBJECT (cds->adj_steps), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &cs->steps);
	gtk_signal_connect (GTK_OBJECT (cds->adj_steps), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Threshold", hbox, FALSE,TRUE, DEF_PAD);
 	cds->adj_threshold = gtk_adjustment_new (cs->threshold,1.0, 80.0, 1.0, 1.0, 0.0);
	define_scale_in_box(cds->adj_threshold,hbox,0,DEF_PAD);	
	gtk_signal_connect (GTK_OBJECT (cds->adj_threshold), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &cs->threshold);
	gtk_signal_connect (GTK_OBJECT (cds->adj_threshold), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
// 	Merge
	if (!strcmp(title,"Gravity erosion")) {
		cds->merge = merge_dialog_struct_new (merge_struct_new(data, (gpointer) draw_hf_ptr));
		cds->merge->content->mix = 100;
		cds->merge->content->merge_op = MAX_MERGE;
		
		gtk_box_pack_start(GTK_BOX(vbox), merge_dialog_new(cds->merge, TRUE, parent_window, NULL, NULL, NULL, NULL), FALSE, FALSE, 0);
		gtk_widget_hide (GTK_WIDGET(cds->merge->main_box));
	}
	// Controls
	
  	gtk_box_pack_start (GTK_BOX (vbox), apply_repeat_buttons_new (data, apply_fn, repeat_fn), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &cds->accept), FALSE, FALSE, DEF_PAD);

	return frame;
}

void apply_crests (hf_wrapper_struct *hfw) {
	//gint t1;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
	//t1 = clock();
	set_watch_cursor(hfw);
	hf_crests_erosion_hex(hfw->hf_struct,
		hfw->hf_options->img->crests->steps,
		hfw->hf_options->img->crests->threshold) ;

//	hf_relax_hex (hfw->hf_struct, hfw->hf_options->img->crests_steps,  hfw->hf_options->img->crests_threshold ) ;
//	hf_directed_relax (hfw->hf_struct, hfw->hf_options->img->crests_steps,  hfw->hf_options->img->crests_threshold, EAST ) ;
	
// printf("TEMPS DE CONSTRUCTION DES CRËTES: %d\n",clock() - t1);
	begin_pending_record(hfw,"Crests",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->crests_dialog->accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	unset_watch_cursor(hfw);
}

void apply_crests_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->if_calculated)
		return;
	apply_crests(hfw);
	draw_hf(hfw);
}

void set_crests_defaults(img_dialog_struct *img) {
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->crests_dialog->adj_steps), 0);
}

void repeat_crests_callb (GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	apply_crests(hfw);
	draw_hf(hfw);
}

GtkWidget *crests_dialog_new (gpointer data) {

	hf_wrapper_struct *hfw;
	gravity_struct *cs;
	gravity_dialog_struct *cds;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	cs = hfw->hf_options->img->crests;
	cds = hfw->hf_options->img->crests_dialog;
	return crests_gravity_dialog_new (data, "Crests",  cs, cds, &hfw->if_calculated, hfw->hf_options->tools_window, apply_crests_callb, repeat_crests_callb);
}

void crests_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->crests_dialog->dialog) {
		hfw->hf_options->img->crests_dialog->dialog = crests_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->crests_dialog->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->crests_dialog->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_crests_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->crests_dialog->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* GRAVITY *************************/

void apply_gravity (hf_wrapper_struct *hfw) {
	//gint t1;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
	//t1 = clock();
	set_watch_cursor(hfw);
	hf_relax_hex(hfw->hf_struct,
		hfw->hf_options->img->gravity->steps,
		hfw->hf_options->img->gravity->threshold) ;
		
	if (!hfw->hf_struct->result_buf)
		hfw->hf_struct->result_buf = (hf_type *) x_malloc(sizeof(hf_type) * hfw->hf_struct->max_x  * hfw->hf_struct->max_y, "hf_type (result_buf in apply_gravity)");
	memcpy (hfw->hf_struct->result_buf,hfw->hf_struct->hf_buf,
		sizeof(hf_type)*hfw->hf_struct->max_x*hfw->hf_struct->max_y);
		
// printf("TEMPS D'ÉROSION PAR GRAVITÉ: %d\n",clock() - t1);
printf("HFW: %p\n",hfw);
	set_merge_buffers (
		hfw->hf_options->img->gravity_dialog->merge->content,
		hfw->hf_struct->tmp_buf, 
		hfw->hf_struct->result_buf,
		hfw->hf_struct->hf_buf, 
		hfw->hf_struct->max_x, 
		hfw->hf_struct->max_y);
	begin_pending_record(hfw,"Gravity erosion",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->gravity_dialog->accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	unset_watch_cursor(hfw);
	simple_merge (hfw->hf_options->img->gravity_dialog->merge->content);
	draw_hf(hfw);
}

void apply_gravity_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->if_calculated)
		return;
	apply_gravity(hfw);
}

void set_gravity_defaults(img_dialog_struct *img) {
	set_merge_gravity_defaults (img->gravity_dialog->merge);
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->gravity_dialog->adj_steps), 0);
}

void repeat_gravity_callb (GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	apply_gravity(hfw);
}

GtkWidget *gravity_dialog_new (gpointer data) {

	hf_wrapper_struct *hfw;
	gravity_struct *cs;
	gravity_dialog_struct *cds;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	cs = hfw->hf_options->img->gravity;
	cds = hfw->hf_options->img->gravity_dialog;
	return crests_gravity_dialog_new (data,"Gravity erosion",  cs, cds, &hfw->if_calculated, hfw->hf_options->tools_window, apply_gravity_callb, repeat_gravity_callb);
}

void gravity_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->gravity_dialog->dialog) {
		hfw->hf_options->img->gravity_dialog->dialog = gravity_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->gravity_dialog->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->gravity_dialog->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_gravity_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->gravity_dialog->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* ORIENTED GRAVITY *************************/

void apply_oriented_gravity (hf_wrapper_struct *hfw) {
	//gint t1;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
	//t1 = clock();
	set_watch_cursor(hfw);
	hf_oriented_relax(hfw->hf_struct,
		hfw->hf_options->img->oriented_gravity->steps,
		hfw->hf_options->img->oriented_gravity->threshold,
		hfw->hf_options->img->oriented_gravity->direction) ;
	
	if (!hfw->hf_struct->result_buf)
		hfw->hf_struct->result_buf = (hf_type *) x_malloc(sizeof(hf_type) * hfw->hf_struct->max_x  * hfw->hf_struct->max_y, "hf_type (result_buf in apply_oriented_gravity)");
	memcpy (hfw->hf_struct->result_buf,hfw->hf_struct->hf_buf,
		sizeof(hf_type)*hfw->hf_struct->max_x*hfw->hf_struct->max_y);

// printf("TEMPS D'ÉROSION PAR GRAVITÉ: %d\n",clock() - t1);
	begin_pending_record(hfw,"Oriented gravity",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->oriented_gravity_dialog->accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	unset_watch_cursor(hfw);
	set_merge_buffers (
		hfw->hf_options->img->oriented_gravity_dialog->merge->content,
		hfw->hf_struct->tmp_buf, 
		hfw->hf_struct->result_buf,
		hfw->hf_struct->hf_buf, 
		hfw->hf_struct->max_x, 
		hfw->hf_struct->max_y);
	simple_merge (hfw->hf_options->img->oriented_gravity_dialog->merge->content);
	draw_hf (hfw);
}

void apply_oriented_gravity_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->if_calculated)
		return;
	apply_oriented_gravity(hfw);
}

void set_oriented_gravity_defaults(img_dialog_struct *img) {
	set_merge_gravity_defaults (img->oriented_gravity_dialog->merge);
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->oriented_gravity_dialog->adj_steps), 0);
}

void repeat_oriented_gravity_callb (GtkWidget *wdg, gpointer data) {	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	apply_oriented_gravity (hfw);
}

void north_callb (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint*) data) = NORTH;
}
void south_callb (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint*) data) = SOUTH;
}
void east_callb (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint*) data) = EAST;
}
void west_callb (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint*) data) = WEST;
}

GtkWidget *direction_toolbar_new (GtkWidget *window, gpointer data) {
	// Creates a toolbar for indicating N/S/W/E direction

	GtkWidget *hbox, *toolbar, *wdg;

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));

//	Effect - merge mode (mountains / valleys / smoothing)

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar),GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
	gtk_widget_show(GTK_WIDGET(toolbar));
	wdg = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,_("East"),
		_("East"),NULL,
		create_widget_from_xpm(window,east_xpm),
		GTK_SIGNAL_FUNC(east_callb), data);
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("West"),
		_("West"),NULL,
		create_widget_from_xpm(window,west_xpm),
		GTK_SIGNAL_FUNC(west_callb), data);
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("North"),
		_("North"),NULL,
		create_widget_from_xpm(window,north_xpm),
		GTK_SIGNAL_FUNC(north_callb), data);
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("South"),
		_("South"),NULL,
		create_widget_from_xpm(window,south_xpm),
		GTK_SIGNAL_FUNC(south_callb), data);

	define_label_in_box("Direction",hbox,FALSE,FALSE,DEF_PAD);
	gtk_box_pack_start(GTK_BOX(hbox),toolbar, TRUE, TRUE, DEF_PAD);

	return hbox;
}

GtkWidget *oriented_gravity_dialog_new (gpointer data) {

	GtkWidget *vbox, *hbox, *frame, *vbox2;
	hf_wrapper_struct *hfw;
	oriented_gravity_struct *cs;
	oriented_gravity_dialog_struct *cds;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	cs = hfw->hf_options->img->oriented_gravity;
	cds = hfw->hf_options->img->oriented_gravity_dialog;
	
	frame = options_frame_new("Oriented gravity erosion");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Steps", hbox, FALSE,TRUE, DEF_PAD);
 	cds->adj_steps = gtk_adjustment_new (cs->steps, 0.0, 100, 1.0, 1.0, 0.0);
	define_scale_in_box(cds->adj_steps,hbox,0,DEF_PAD);	
	gtk_signal_connect (GTK_OBJECT (cds->adj_steps), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &cs->steps);
	gtk_signal_connect (GTK_OBJECT (cds->adj_steps), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Threshold", hbox, FALSE,TRUE, DEF_PAD);
 	cds->adj_threshold = gtk_adjustment_new (cs->threshold,1.0, 80.0, 1.0, 1.0, 0.0);
	define_scale_in_box(cds->adj_threshold,hbox,0,DEF_PAD);	
	gtk_signal_connect (GTK_OBJECT (cds->adj_threshold), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &cs->threshold);
	gtk_signal_connect (GTK_OBJECT (cds->adj_threshold), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	cds->wdg_direction = direction_toolbar_new (hfw->window, &cs->direction);
	gtk_box_pack_start (GTK_BOX (vbox), cds->wdg_direction , FALSE, FALSE, DEF_PAD);
	
	vbox2 = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox2));
		
  	gtk_box_pack_start (GTK_BOX (vbox2), apply_repeat_buttons_new (data, apply_oriented_gravity_callb, repeat_oriented_gravity_callb), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX (vbox2), reset_accept_buttons_new (data, &cds->accept), FALSE, FALSE, DEF_PAD);
	
// 	Merge

	cds->merge = merge_dialog_struct_new (merge_struct_new(data, (gpointer) draw_hf_ptr));
	cds->merge->content->mix = 100;
	cds->merge->content->merge_op = MAX_MERGE;
	
	gtk_box_pack_start(GTK_BOX(vbox), merge_dialog_new(cds->merge, TRUE, hfw->hf_options->tools_window, NULL, NULL, NULL, NULL), FALSE, FALSE, 0);
	gtk_widget_hide (GTK_WIDGET(cds->merge->main_box));
	
	// Controls

	gtk_box_pack_start(GTK_BOX(vbox), vbox2, FALSE, FALSE, 0); 

	return frame;
}

void oriented_gravity_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->oriented_gravity_dialog->dialog) {
		hfw->hf_options->img->oriented_gravity_dialog->dialog = oriented_gravity_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->oriented_gravity_dialog->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->oriented_gravity_dialog->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_oriented_gravity_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->oriented_gravity_dialog->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}
