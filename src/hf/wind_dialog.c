/* wind_dialog.c:  dialogs for wind erosion.
 * Works with img_process_dialog.
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
#include "img_process_dialog.h"
#include "hf_wrapper.h"
#include "wind_dialog.h"
#include "wind.h"
#include "dialog_utilities.h"

/************************* GENERIC CREATOR / DESTRUCTOR **********************/

wind_dialog_struct *wind_dialog_struct_new (gint process, gpointer parent_data) {

	wind_dialog_struct *wds;
	wds = (wind_dialog_struct *) x_malloc(sizeof(wind_dialog_struct), "wind_dialog_struct");
	
	wds->dialog = NULL;
	wds->accept = NULL;
	wds->adj_steps = NULL;
	wds->adj_wind_speed = NULL;
	wds->adj_slope_threshold = NULL;
	wds->adj_a = NULL;
	wds->adj_b = NULL;
	wds->process = process;
	wds->parent_data = parent_data;
	
	return wds;
}

void wind_dialog_struct_free (wind_dialog_struct *wds) {
	if (wds)
		x_free(wds);
}

/************************* SAND RIPPLES and DUNES *************************/

void apply_wind (wind_dialog_struct *wds) {
	//gint t1;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) wds->parent_data;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
	//t1 = clock();
	set_watch_cursor(hfw);
	switch (wds->process) {
		case RIPPLES:
			hf_ripples (hfw->hf_struct, hfw->hf_options->img->ripples, hfw->hf_options->dist_matrix, hfw->hf_options->gauss_list);
			begin_pending_record(hfw,"Ripples",accept_fn,reset_fn);
			gtk_widget_set_sensitive(GTK_WIDGET(wds->accept) ,TRUE);
			break;
		case DUNES:
		default:
			hf_dunes (hfw->hf_struct, hfw->hf_options->img->dunes, hfw->hf_options->dist_matrix, hfw->hf_options->gauss_list);
			begin_pending_record(hfw,"Dunes",accept_fn,reset_fn);
			gtk_widget_set_sensitive(GTK_WIDGET(wds->accept) ,TRUE);
	}

// printf("TEMPS D'�ROSION PAR LE VENT: %d\n",clock() - t1);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	unset_watch_cursor(hfw);
}

void apply_wind_callb (GtkWidget *wdg, gpointer data) {
	wind_dialog_struct *wds;
	hf_wrapper_struct *hfw;
	wds = (wind_dialog_struct *) data;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) wds->parent_data;
//	We reapply the process on the source
//	(it works on hf-hf_buf, without copying tmp_buf to hf_buf);
	hf_restore(hfw->hf_struct);
	apply_wind(wds);
	draw_hf(hfw);
}

void set_wind_ripples_defaults(img_dialog_struct *img) {
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->ripples_dialog->adj_steps), 0);
}

void set_wind_dunes_defaults(img_dialog_struct *img) {
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->ripples_dialog->adj_steps), 0);
}

void repeat_wind_callb (GtkWidget *wdg, gpointer data) {
	wind_dialog_struct *wds;	
	hf_wrapper_struct *hfw;
	wds = (wind_dialog_struct *) data;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) wds->parent_data;
	hf_backup(hfw->hf_struct);
	hfw->if_calculated = FALSE;
	apply_wind(wds);
	draw_hf(hfw);
}

GtkWidget *wind_dialog_new (wind_struct *ws, wind_dialog_struct *wds, hf_wrapper_struct *hfw, gchar *title, gboolean if_a, gboolean if_b) {

	GtkWidget *vbox, *hbox, *frame, *button;
	GtkObject *adj;
		
	frame = options_frame_new(title);

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	// Steps
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Steps", hbox, FALSE,TRUE, DEF_PAD);
 	wds->adj_steps = gtk_adjustment_new (ws->steps, 0.0, 100.0, 1.0, 1.0, 0.0);
	define_scale_in_box(wds->adj_steps,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (wds->adj_steps),
		 "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &ws->steps);
	gtk_signal_connect (GTK_OBJECT (wds->adj_steps),"value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) hfw->if_modified);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	// Wind speed
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Wind speed", hbox, FALSE,TRUE, DEF_PAD);
 	wds->adj_wind_speed = gtk_adjustment_new (ws->wind_speed, 0.0, 10.0, 1.0, 1.0, 0.0);
	define_scale_in_box(wds->adj_wind_speed,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (wds->adj_wind_speed), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &ws->wind_speed);
	gtk_signal_connect (GTK_OBJECT (wds->adj_wind_speed), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) hfw->if_modified);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);


	// Amplitude of the slope derivative factor
	if (if_a) {
		hbox = gtk_hbox_new(FALSE,DEF_PAD);
		gtk_widget_show(GTK_WIDGET(hbox));
	
		define_label_in_box("A", hbox, FALSE,TRUE, DEF_PAD);
		wds->adj_a = gtk_adjustment_new (ws->a, 0.0, 10.0, 0.1, 0.1, 0.0);
		define_scale_in_box(wds->adj_a,hbox,1,DEF_PAD);
		gtk_signal_connect (GTK_OBJECT (wds->adj_a), "value_changed", GTK_SIGNAL_FUNC (gdouble_adj_callb), (gpointer) &ws->a);
		gtk_signal_connect (GTK_OBJECT (wds->adj_a), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) hfw->if_modified);
	
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	}
	
	// Amplitude of the slope factor
	if (if_b) {	
		hbox = gtk_hbox_new(FALSE,DEF_PAD);
		gtk_widget_show(GTK_WIDGET(hbox));
	
		define_label_in_box("Amplitude", hbox, FALSE,TRUE, DEF_PAD);
		wds->adj_b = gtk_adjustment_new (ws->b, 0.0, 15.0, 0.1, 0.1, 0.0);
		define_scale_in_box(wds->adj_b,hbox,1,DEF_PAD);
		gtk_signal_connect (GTK_OBJECT (wds->adj_b), "value_changed", GTK_SIGNAL_FUNC (gdouble_adj_callb), (gpointer) &ws->b);
		gtk_signal_connect (GTK_OBJECT (wds->adj_b), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) hfw->if_modified);
	
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	}
		
	// Slope threshold
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Slope threshold", hbox, FALSE,TRUE, DEF_PAD);
 	wds->adj_slope_threshold = gtk_adjustment_new (ws->slope_threshold, 0.0, 80.0, 1.0, 1.0, 0.0);
	define_scale_in_box(wds->adj_slope_threshold,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (wds->adj_slope_threshold), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &ws->slope_threshold);
	gtk_signal_connect (GTK_OBJECT (wds->adj_slope_threshold), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) hfw->if_modified);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
	// Refine crests: only for dunes
	
	if (!strcmp(title,"Dunes")) {
		button = define_check_button_in_box ("Refine crests", vbox, TRUE, TRUE, DEF_PAD*0.5);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ws->refine_crests);
		gtk_signal_connect (GTK_OBJECT (button), "toggled", GTK_SIGNAL_FUNC (toggle_check_button_callb), (gpointer) &ws->refine_crests);
	}
	
	// Smooth before
	
	button = define_check_button_in_box ("Smooth before", vbox, TRUE, TRUE, 0.5 * DEF_PAD);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), ws->smooth_before);
	gtk_signal_connect (GTK_OBJECT (button), "toggled", GTK_SIGNAL_FUNC (toggle_check_button_callb), (gpointer) &ws->smooth_before);
	
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	define_label_in_box ("Radius", hbox, FALSE, FALSE, 0);
	adj = gtk_adjustment_new (ws->radius, 0, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &ws->radius);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(show_if_true), hbox);
	if (!ws->smooth_before) {
		gtk_widget_hide(GTK_WIDGET(hbox));
	}
	
	// Control buttons
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label (_("Apply"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) apply_wind_callb, (gpointer) wds);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Repeat"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) repeat_wind_callb,(gpointer) wds);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5),
		 FALSE, FALSE, DEF_PAD);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label (_("Reset"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) reset_callb, wds->parent_data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Accept"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) accept_callb, wds->parent_data);
	gtk_widget_show(button);	
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	wds->accept = button;

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5),
		 FALSE, FALSE, DEF_PAD);

	return frame;
}

GtkWidget *wind_ripples_dialog_new (gpointer data) {

	wind_struct *ws;
	wind_dialog_struct *wds;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	ws = hfw->hf_options->img->ripples;
	wds = hfw->hf_options->img->ripples_dialog;

	return wind_dialog_new (ws, wds, hfw, "Ripples", FALSE, TRUE);
}

GtkWidget *wind_dunes_dialog_new (gpointer data) {

	wind_struct *ws;
	wind_dialog_struct *wds;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	ws = hfw->hf_options->img->dunes;
	wds = hfw->hf_options->img->dunes_dialog;

	return wind_dialog_new (ws, wds, hfw, "Dunes", FALSE, TRUE);
}

void ripples_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->ripples_dialog->dialog) {
		hfw->hf_options->img->ripples_dialog->dialog = wind_ripples_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->ripples_dialog->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->ripples_dialog->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_wind_ripples_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->ripples_dialog->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

void dunes_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->dunes_dialog->dialog) {
		hfw->hf_options->img->dunes_dialog->dialog = wind_dunes_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->dunes_dialog->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->dunes_dialog->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_wind_dunes_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->dunes_dialog->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

