/* fill_dialog.c:  dialogs and callbacks for filling a zone
 *
 * Copyright (C) 2005 Patrice St-Gelais
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

#include "fill.h"
#include "draw_tools_dialog.h"
#include "dialog_utilities.h"
#include <gtk/gtk.h>
// #include "../utils/menus_n_tools.h"
#include "globals.h"
// #include "view_area.h"
#include "hf_wrapper.h"

gint update_fill_area (GtkWidget *wdg, gpointer data) {
	draw_tools_struct *dts;
	hf_wrapper_struct *hfw;
	hf_struct_type *hf;
//	printf("UPDATE_FILL_AREA\n");
	if (!data)
		return TRUE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf = hfw->hf_struct;
	dts = hfw->hf_options->draw;
	if (!fill_from_selection (hf->tmp_buf, hf->select_buf, hf->hf_buf, hf->max_x, hf->max_y, dts->fill_pen))
		return TRUE;;
	(*hfw->if_modified) = TRUE;
	gtk_widget_set_sensitive (GTK_WIDGET(dts->fill_accept), TRUE);
	hfw->if_rgb = TRUE;
	draw_hf(hfw);
	return FALSE;
}

void accept_fill_fn (gpointer hfw_ptr) {
	hf_wrapper_struct *hfw = (hf_wrapper_struct *) hfw_ptr;
	if (!hfw)
		return;
//	printf("ACCEPT_FILL_FN\n");
	hf_backup(hfw->hf_struct);
	if (hfw->hf_struct->select_buf) {
		x_free(hfw->hf_struct->select_buf);
		hfw->hf_struct->select_buf = NULL;
	}
	reset_seeds (hfw->hf_options->draw->fill_pen);	
	commit_pending_record(hfw);
//	Here, reset the control widgets if needed
	hfw->if_calculated = FALSE;
	draw_hf(hfw);
}

void reset_fill_fn (gpointer hfw_ptr) {
	hf_wrapper_struct *hfw = (hf_wrapper_struct *) hfw_ptr;
//	printf("RESET_FILL_FN\n");
	hf_restore (hfw->hf_struct);
	reset_seeds (hfw->hf_options->draw->fill_pen);
	if (hfw->hf_struct->select_buf) {
		x_free(hfw->hf_struct->select_buf);
		hfw->hf_struct->select_buf = NULL;
	}
}

static gint accept_fill_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return TRUE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	accept_fill_fn (hfw);
	return FALSE;
}

static gint reset_fill_callb(GtkWidget *wdg, gpointer data) {
//	Cancels the current operation by copying the temp buffer (backuped)
// 	into the current (modified) one
	hf_wrapper_struct *hfw;
	if (!data)
		return FALSE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	reset_fill_fn (hfw);
	draw_hf(hfw);
	return FALSE;
}

static void set_replace (GtkWidget *wdg, gpointer data) {
	if (!data)
		return;
	*((gint *) data) = SELECT_REPLACE;
}

static void set_add (GtkWidget *wdg, gpointer data) {
	if (!data)
		return;
	*((gint *) data) = SELECT_ADD;
}

static void set_subtract (GtkWidget *wdg, gpointer data) {
	if (!data)
		return;
	*((gint *) data) = SELECT_SUBTRACT;
}

gint reset_height (GtkWidget *wdg, gpointer data) {
	// Reapply the filling process after a height change
	hf_wrapper_struct *hfw;
//	printf("RESET_HEIGHT\n");
	if (!data)
		return TRUE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	refresh_select_from_seeds (hfw->hf_options->draw->fill_pen, hfw->hf_struct);
	return FALSE;
}

GtkWidget *fill_dialog_new(GtkWidget *window, GtkTooltips *tooltips,
	gpointer dts_ptr, gpointer data) {

	//	Note: data is a hf_wrapper_struct **
	GtkWidget *frame, *vbox, *hbox, *button, *scale;
	GtkObject *adj;
	GSList *group = NULL;
	fill_struct *pen;
	draw_tools_struct *dts = (draw_tools_struct *) dts_ptr;

	pen = dts->fill_pen;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	
	// Reset / accept box
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	dts->fill_accept = hbox;
	
	frame = frame_new("Selection mode", DEF_PAD*0.5);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, DEF_PAD*0.5);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);

	define_radio_button_in_box_with_data (hbox, &group, "Replace",
		set_replace, &pen->select_mode, TRUE);
	define_radio_button_in_box_with_data (hbox, &group, "Add",
		set_add, &pen->select_mode, FALSE) ;
	define_radio_button_in_box_with_data (hbox, &group, "Subtract",
		set_subtract, &pen->select_mode, FALSE) ;
		
	gtk_container_add (GTK_CONTAINER(frame), hbox);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Filling height", hbox, FALSE,TRUE, DEF_PAD);
	// Normally height should be >= 1 and <= MAX_HF_VALUE-1, but we use 0 and MAX_HF_VALUE
	// as start and end points for being consistent with the percent buttons row
 	adj = gtk_adjustment_new (pen->height, 0, MAX_HF_VALUE, 1.0, 1.0, 0.0);
	scale = define_scale_in_box(adj,hbox,0,DEF_PAD);
//	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), &pen->height);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (reset_height), data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_fill_area), data);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
			
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	uniform_percent_buttons_new (hbox, adj);
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("%", hbox, FALSE,TRUE, DEF_PAD);
 	adj = gtk_adjustment_new (pen->percent, 0.0, 100, 1.0, 1.0, 0.0);
	scale = define_scale_in_box(adj,hbox,0,DEF_PAD);
//	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), &pen->percent);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_fill_area), data);

	percent_buttons_new (hbox, adj);
		
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, DEF_PAD*0.5);
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Noise exponent", hbox, FALSE,TRUE, DEF_PAD);
 	adj = gtk_adjustment_new (pen->noise_exponent, 1.0, 10.0, 0.1, 0.1, 0.01);
	scale = define_scale_in_box(adj,hbox,1,DEF_PAD);
//	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gdouble_adj_callb), &pen->noise_exponent);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_fill_area), data);
		
  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, DEF_PAD*0.5);
	
//	Accept / reset

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(dts->fill_accept,0.5,0.5), FALSE, FALSE, DEF_PAD*0.5);

	button = gtk_button_new_with_label (_("Reset"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) reset_fill_callb, data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(dts->fill_accept), button, FALSE, FALSE, DEF_PAD);
		
	// Unactivate the buttons after a reset or accepting the change
	// ("Apply_fill" reactivates the buttons)
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) unset_sensitive_callb, (gpointer) dts->fill_accept);
	       
	button = gtk_button_new_with_label (_("Accept"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) accept_fill_callb, data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(dts->fill_accept), button, FALSE, FALSE, DEF_PAD);
	
	// Unactivate the buttons after a reset or accepting the change
	// ("Apply_fill" reactivates the buttons)
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) unset_sensitive_callb, (gpointer) dts->fill_accept);

	gtk_widget_set_sensitive (GTK_WIDGET(dts->fill_accept), FALSE);
	
	frame = frame_new("Fill",0.5*DEF_PAD);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	
	return frame;
}

