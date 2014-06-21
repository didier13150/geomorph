/* fault_dialog.c:  dialogs and callbacks for drawing a fault
 *
 * Copyright (C) 2003 Patrice St-Gelais
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

#include "fault.h"
#include "draw_tools_dialog.h"
#include "dialog_utilities.h"
#include "line_dialog.h"
#include <gtk/gtk.h>
// #include "../utils/menus_n_tools.h"
#include "globals.h"
// #include "view_area.h"
#include "hf_wrapper.h"

void display_fault_line (gpointer hf_wrapper, gint x0, gint y0, gint x1, gint y1) {
	// Display the fault line only, without computing the altitude difference,
	// typically when the mouse moves
	// 2006-01: not used anymore - seems to be kept for testing purposes only
	// (e.g. displaying the polyline in the HF)
	
	gdouble ddist, startx=0.0, starty=0.0, endx=0.0, endy=0.0;
	hf_type value = 0;
	fault_struct *f;
	hf_wrapper_struct *hfw = (hf_wrapper_struct *) hf_wrapper;

	// We need TMP_BUF for buffering the original HF, when displaying the fault line
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);

	hf_restore(hfw->hf_struct);

	f = hfw->hf_options->draw->fault_pen;

	// First we compute the axis on which to draw the line
	// We extend the line marked by the mouse from edge to edge

	if (! intersect_rect (0,0,hfw->hf_struct->max_x-1, hfw->hf_struct->max_y-1,
		x0, y0, x1, y1, &startx, &starty, &endx, &endy) )
			return;
// printf("********* Master line: (%d,%d) - (%d,%d); extended line: (%d, %d) - (%d, %d)\n",x0,y0,x1,y1,startx, starty, endx, endy);
//	set_line_translation (f->line, startx, starty,
//		hfw->hf_struct->max_x, hfw->hf_struct->max_y );

	ddist = DIST2(startx, starty, endx, endy);
	set_line_scale (f->fractal_line->polyline, ddist, ddist);

	draw_n_transform_all_segments (f->fractal_line->polyline,
		endx-startx, endy-starty,
		startx, starty,
		(gpointer) hfw->hf_struct->hf_buf,
		hfw->hf_struct->max_x,
		hfw->hf_struct->max_y,
		(gpointer) &value,
		HF_TYPE_ID,
		OVERFLOW_ZERO);

//	hf_simple_merge (hfw->hf_struct->tmp_buf, f->buffer,hfw->hf_struct->hf_buf , 100,
//		0, 0, f->buf_size, f->buf_size, MULTIPLY);

	draw_hf(hfw);
}

static gint update_fault_callb (GtkWidget *wdg, gpointer data) {
	draw_tools_struct *dts;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

// printf("UPDATE_FAULT_CALLB\n");
	// Compute only if a control line is active
	if (!hfw->control_line)
		return TRUE;
	if (!hfw->control_line->active)
		return TRUE;

	dts = hfw->hf_options->draw;

	if (hfw->control_line->revert_ends) {
		compute_fault (dts->fault_pen, hfw->hf_struct,
			LEFT_SHIFT((gint) hfw->control_line->x1, get_scale(hfw->sbs)),
			LEFT_SHIFT((gint) hfw->control_line->y1, get_scale(hfw->sbs)),
			LEFT_SHIFT((gint) hfw->control_line->x0, get_scale(hfw->sbs)),
			LEFT_SHIFT((gint) hfw->control_line->y0, get_scale(hfw->sbs)),
			hfw->hf_options->dist_matrix,
			hfw->hf_options->gauss_list);
	}
	else {
		compute_fault (dts->fault_pen, hfw->hf_struct,
			LEFT_SHIFT((gint) hfw->control_line->x1, get_scale(hfw->sbs)),
			LEFT_SHIFT((gint) hfw->control_line->y1, get_scale(hfw->sbs)),
			LEFT_SHIFT((gint) hfw->control_line->x0, get_scale(hfw->sbs)),
			LEFT_SHIFT((gint) hfw->control_line->y0, get_scale(hfw->sbs)),
			hfw->hf_options->dist_matrix,
			hfw->hf_options->gauss_list);
	}
	gtk_widget_set_sensitive (GTK_WIDGET(dts->fault_accept), TRUE);
	draw_hf(hfw);
	return FALSE;
}

void accept_fault_fn (hf_wrapper_struct *hfw) {
// printf("ACCEPT_FAULT_FN\n");
	if (!hfw)
		return;
	hf_backup(hfw->hf_struct);
	commit_pending_record(hfw);
//	Here, reset the control widgets if needed
	hfw->if_calculated = FALSE;
	control_line_unactivate (hfw->control_line);
	//	Faults remove tiling most of the time
	if (hfw->tiling_ptr) {
		*hfw->tiling_ptr = FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hfw->tiling_wdg),*hfw->tiling_ptr);
	}
	draw_hf(hfw);
}

static gint accept_fault_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return FALSE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	accept_fault_fn (hfw);
	return FALSE;
}

static gint reset_fault_callb(GtkWidget *wdg, gpointer data) {
//	Cancels the current operation by copying the temp buffer (backuped)
// 	into the current (modified) one
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	
	hf_restore (hfw->hf_struct);
//	Here, reset the control widgets if needed
	control_line_unactivate (hfw->control_line);
	draw_hf(hfw);
	return FALSE;
}

static gint unset_preview_mode (GtkWidget *widget, GdkEventButton *event, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw)
		return FALSE;
	if (!hfw->hf_options->draw)
		return FALSE;
	hfw->hf_options->draw->fault_pen->mode = FAULT_FINAL;
//	printf("UNSET_PREVIEW_MODE\n");
	hfw->gl_preview->refresh_on_mouse_down = TRUE;
	// We need to recompute the fault once after releasing the mouse button
	update_fault_callb (NULL,data);
	return FALSE;
}

static gint set_preview_mode (GtkWidget *widget, GdkEventButton *event, gpointer data) {
	gint *mode;
	mode = (gint *) data;
	if (!mode)
		return FALSE;
	*mode = FAULT_PREVIEW;
	return FALSE;
// printf("SET_PREVIEW_MODE\n");
}

static gint set_preview_mode_on_hfw (GtkWidget *widget, GdkEventButton *event, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw)
		return FALSE;
	if (!hfw->hf_options->draw)
		return FALSE;
	hfw->hf_options->draw->fault_pen->mode = FAULT_PREVIEW;
	hfw->gl_preview->refresh_on_mouse_down = FALSE;
// printf("SET_PREVIEW_MODE\n");
	return FALSE;
}

GtkWidget *fault_dialog_new(GtkWidget *window, GtkTooltips *tooltips,
	gpointer dts_ptr, gpointer data) {

	//	Note: data is a hf_wrapper_struct **
	GtkWidget *frame, *vbox, *hbox, *hbox2, *wdg, *button, *scale;
	GtkObject *adj;
	fault_struct *pen;
	draw_tools_struct *dts = (draw_tools_struct *) dts_ptr;

	pen = dts->fault_pen;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);

	frame = frame_new("Pen - fault",0.5*DEF_PAD);
	gtk_container_add(GTK_CONTAINER(frame),vbox);

	dts->fault_line_dialog =
		line_dialog_new (window, tooltips, "Fracture line", pen->fractal_line,
			FAULT_PREVIEW_SIZE, FAULT_PREVIEW_SIZE,
			data, update_fault_callb, set_preview_mode_on_hfw, unset_preview_mode);

	gtk_box_pack_start(GTK_BOX(vbox), dts->fault_line_dialog->dialog,
			FALSE, FALSE, DEF_PAD*0.5);

//	Altitude difference

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, DEF_PAD);

	define_label_in_box("Difference in altitude",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->altitude_difference, -50, 50, 1, 1, 0.01);
	scale = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) set_preview_mode, (gpointer) &pen->mode);

	gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) gl_no_refresh_callb, data);

	gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) unset_preview_mode, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->altitude_difference);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gboolean_set_true), (gpointer) &pen->buffer_reset_required);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_fault_callb), data);

	button = gtk_button_new_with_label ("0");
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) zero_callb, adj);

// 	Smoothing

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, DEF_PAD*0.5);

	define_label_in_box("Smoothing",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->smoothing,0, 20, 1, 1, 0.01);
	scale = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) set_preview_mode, (gpointer) &pen->mode);

	gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) gl_no_refresh_callb, data);

	gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) unset_preview_mode, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->smoothing);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_fault_callb), data);// 	Smoothing

//	Separation

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, DEF_PAD);

	define_label_in_box("Separation",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->separation, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) set_preview_mode, (gpointer) &pen->mode);
	gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) unset_preview_mode, data);
	gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) gl_no_refresh_callb, (gpointer) data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->separation);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gboolean_set_true), (gpointer) &pen->buffer_reset_required);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_fault_callb), data);

	percent_buttons_new (hbox, (gpointer *) adj);

// Check box for showing / hiding the accept buttons
	button = define_check_button_in_box ("Auto Accept & Refresh", vbox, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),dts->fault_auto_accept_flag);
	g_signal_connect (G_OBJECT (button), "toggled", GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &dts->fault_auto_accept_flag);
//	gtk_signal_connect (GTK_OBJECT(button), "toggled",
//		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &dts->fault_auto_accept_flag);
// printf("Toggle pointer (dts->fault_auto_accept_flag): %p\n",&dts->fault_auto_accept_flag);
//
//	Accept / reset
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);

	wdg = gtk_button_new_with_label (_("Reset"));
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
	       (GtkSignalFunc) reset_fault_callb, data);
	gtk_widget_show(wdg);

	gtk_box_pack_start(GTK_BOX(hbox), wdg, FALSE, FALSE, DEF_PAD);

	// 2nd hbox for combining the Accept and Accept / Refresh button
	hbox2 = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox2);

	// Control check box, for showing / hiding the accept buttons
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(hide_if_true), hbox2);
	if (dts->fault_auto_accept_flag)
		gtk_widget_hide(hbox2);

	button = gtk_button_new_with_label (_("Accept"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) accept_fault_callb, data);

	// Unactivate "Accept" button after a reset or accepting the change
	// ("Apply_fault" reactivates the accept button)
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
	       (GtkSignalFunc) unset_sensitive_callb, (gpointer) hbox2);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) unset_sensitive_callb, (gpointer) hbox2);

	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox2), button, FALSE, FALSE, DEF_PAD);

	//	Accept & Refresh button

	button = gtk_button_new_with_label (_("Accept & Refresh"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) accept_fault_callb, data);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) refresh_seed_from_seedbox, (gpointer) dts->fault_line_dialog->seed_dialog);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) update_line, (gpointer) dts->fault_line_dialog);

	// Unactivate "Accept" button after a reset or accepting the change

	// ("Apply_fault" reactivates the accept button)
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
	       (GtkSignalFunc) unset_sensitive_callb, (gpointer) hbox2);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) unset_sensitive_callb, (gpointer) hbox2);

	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox2), button, FALSE, FALSE, DEF_PAD);
	gtk_widget_set_sensitive(GTK_WIDGET(hbox2),FALSE);

	gtk_box_pack_start(GTK_BOX(vbox), align_widget(hbox2,0.5,0.5), FALSE, FALSE, 0);

	dts->fault_accept = hbox2;

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5),
		 FALSE, FALSE, DEF_PAD);

	return frame;
}

