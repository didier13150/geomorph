/* draw_tools_dialog.c:  dialogs and callbacks for pen choice dialog and some utilitaries
 *
 * Copyright (C) 2003-2005 Patrice St-Gelais
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "hf_calc.h"
#include "hf_wrapper.h"
#include "draw_hf.h"
#include "hf_tools_dialog.h"
#include "draw_tools_dialog.h"

#define NBDRAWTOOLS 5
#include "../icons/penhf2.xpm"
#include "../icons/faultpen.xpm"
#include "../icons/penstandard.xpm"
#include "../icons/pencrater.xpm"
#include "../icons/bucketfill.xpm"
// #include "../icons/crackpen.xpm"

command_item_struct drawtools[NBDRAWTOOLS] = {
{ "Edit", 	  "Draw HF", "Draw with a height field", 0, (gchar **) penhf2_xpm, GDK_PENCIL, draw_hf_callb,NULL, NULL,TRUE },
{ "Edit", 	  "Draw", "Draw with a continuous stroke", 0, (gchar **) penstandard_xpm, GDK_PENCIL, draw_callb,NULL, NULL,FALSE },
{ "Edit", 	  "Craters", "Draw craters one by one", 0, (gchar **) pencrater_xpm, GDK_PENCIL, draw_crater_callb,NULL, NULL,FALSE },
{ "Edit", 	  "Faults", "Draw faults one by one", 0, (gchar **) faultpen_xpm, GDK_PENCIL, fault_callb,NULL, NULL,FALSE },
{ "Edit", 	  "Fill", "Fill", 0, (gchar **) bucketfill_xpm, GDK_PENCIL, fill_callb,NULL, NULL,FALSE }
// "Edit", 	  "Cracks", "Draw cracks one by one", 0, (gchar **) crackpen_xpm, GDK_PENCIL, crack_callb,NULL, NULL,FALSE
};

draw_tools_struct *draw_tools_new (GtkWidget *draw_tools_dialog,
	dist_matrix_struct *dist_matrix, gdouble **gauss_list) {

	draw_tools_struct *dts;

	dts = (draw_tools_struct *) x_calloc(sizeof(draw_tools_struct), 1, "draw_tools_struct");
//	printf("DTS: %p\n",dts);
	dts->draw_tools_dialog = draw_tools_dialog;

	dts->dist_matrix = dist_matrix;

	dts->current_draw_tools = NULL;	// Current subdialog
	dts->context = 0;		// Current subdialog ID

	dts->draw_begin_x = 0.0;
	dts->draw_begin_y = 0.0;
	dts->line_drawing = FALSE;

	dts->draw_dialog = NULL;
	dts->pen = pen_new();
	dts->preview = NULL;

	dts->draw_hf_dialog = NULL;
	dts->hf_pen = NULL;

	dts->hf_pen = draw_hf_new(dist_matrix);
	set_gauss_list (dts->hf_pen, gauss_list);

	dts->hf_preview = NULL;

	dts->draw_crater_dialog = NULL;
	dts->crater_pen = crater_struct_new();
	dts->swirl_dialog = NULL;
	dts->swirl_pen = swirl_pen_new();

	dts->fault_dialog = NULL;
	dts->fault_pen = fault_pen_new();
	dts->fault_line_dialog = NULL;
	dts->fault_accept = NULL;
	dts->fault_auto_accept_flag = TRUE;
	
	dts->fill_dialog = NULL;
	dts->fill_pen = fill_pen_new(MAX_HF_VALUE>>2, SELECT_REPLACE, 50);
	dts->fill_accept = NULL;

//	dts->crack_dialog = NULL;
//	dts->crack_pen = crack_pen_new();

	return dts;
}

void draw_tools_free (draw_tools_struct *draw_tools) {
	if (draw_tools) {
		if (draw_tools->crater_pen)
			crater_struct_free(draw_tools->crater_pen);
		if (draw_tools->pen)
			pen_free(draw_tools->pen);
		if (draw_tools->preview)
			view_struct_free(draw_tools->preview);
		if (draw_tools->hf_pen)
			draw_hf_free(draw_tools->hf_pen);
		if (draw_tools->fault_pen)
			fault_pen_free(draw_tools->fault_pen);
		if (draw_tools->fill_pen)
			fill_pen_free(draw_tools->fill_pen);
		if (draw_tools->fault_line_dialog)
			line_dialog_free(draw_tools->fault_line_dialog);
		x_free(draw_tools);
	}
}

GtkWidget *draw_tools_dialog_new (GtkWidget *window,
		GtkTooltips *tooltips, draw_tools_struct *dts, gpointer hf_opt) {
	// Initializes the dialog for drawing in images
	// Two parts, from top to bottom:
	//	1. Toolbar of pen types
	//	2. Options for the current pen - given by the specific callback
	// data is a hfw_ptr_ptr... should be changed for a hf_options_ptr in the future

	GtkWidget *dialog, *hbox, *lbl;
	toolbar_struct *tb;
	gpointer data;
	if ((!hf_opt)) 
		return gtk_label_new (_("Not able to initialize draw tools"));
	data = ((hf_options_struct *) hf_opt)->hfw_ptr_ptr;

	dialog = gtk_vbox_new(FALSE, DEF_PAD);
	gtk_widget_show(dialog);
	hbox  = gtk_hbox_new(FALSE, DEF_PAD);
	gtk_widget_show(hbox);
	lbl = gtk_label_new(_("Pen type"));
	gtk_widget_show(lbl);
	gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, DEF_PAD);

	// toolbar_struct is a bit overkill for this purpose...
	tb = multi_toolbar_new(NBDRAWTOOLS, drawtools,
		tooltips,
		window,
		data,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);
	gtk_box_pack_start(GTK_BOX(hbox),
		tb->toolbarwdg, FALSE, FALSE, DEF_PAD);
	x_free(tb);

	gtk_box_pack_start(GTK_BOX(dialog), hbox, FALSE, FALSE, 0);

	//	Sub-dialogs creation
	//	Maybe try to create them "on-demand" when there would be more of them

	dts->draw_dialog = draw_dialog_new(window,tooltips,dts);
	dts->draw_crater_dialog = draw_crater_dialog_new(window,tooltips,dts->crater_pen,data);

	dts->draw_hf_dialog = draw_hf_dialog_new(window,tooltips,(gpointer) dts);

	dts->fault_dialog = fault_dialog_new(window,tooltips,dts,data);
	dts->fill_dialog = fill_dialog_new(window,tooltips,dts,data);
	dts->swirl_dialog = swirl_dialog_new(window,tooltips,dts->swirl_pen, data);

	gtk_container_add(GTK_CONTAINER(dialog), dts->draw_dialog);
	if (dts->draw_hf_dialog)
		gtk_container_add(GTK_CONTAINER(dialog), dts->draw_hf_dialog);
	gtk_container_add(GTK_CONTAINER(dialog), dts->draw_crater_dialog);
	gtk_container_add(GTK_CONTAINER(dialog), dts->swirl_dialog);
	gtk_container_add(GTK_CONTAINER(dialog), dts->fault_dialog);
	gtk_container_add(GTK_CONTAINER(dialog), dts->fill_dialog);

	gtk_widget_hide(dts->draw_crater_dialog);
	gtk_widget_hide(dts->draw_dialog);
	gtk_widget_hide(dts->fault_dialog);
	gtk_widget_hide(dts->swirl_dialog);
	gtk_widget_hide(dts->fill_dialog);

	if (dts->draw_hf_dialog) {
		gtk_widget_show(dts->draw_hf_dialog);		// The preliminary default
		dts->current_draw_tools = dts->draw_hf_dialog;
		dts->context = DRAW_HF_CONTEXT;
	}
	else {
		gtk_widget_show(dts->draw_dialog);
		dts->current_draw_tools = dts->draw_dialog;
		dts->context = DRAW_CONTEXT;
}
	return dialog;
}

void draw_callb(GtkWidget *wdg,gpointer data) {
//	Displays the widgets for painting in the current HF
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
//	The user has clicked another choice
			hf_commit_or_reset (hfw->hf_options);
			return;
		}
	if (!hfw) return;
	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->pencil_cursor;
	dts = hfw->hf_options->draw;
	//  Options dialog
	if (dts->draw_dialog) {
		gtk_widget_hide(dts->current_draw_tools);
		dts->current_draw_tools = dts->draw_dialog;
		gtk_widget_show(dts->current_draw_tools);
	}
	dts->context = DRAW_CONTEXT;

	begin_pending_record(hfw,"Stroke",accept_draw_fn,reset_draw_fn);

// Tests 2004-01-04

//		tox =400.0; toy = 300.0;
//		draw_line_n_refresh(hfw, 100.0, 300.0, &tox, &toy, FALSE, FALSE); // OK
}
void draw_crater_callb(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
//	The user has clicked another choice
			hf_commit_or_reset (hfw->hf_options);
			return;
		}
	if (!hfw) return;
	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->pencil_cursor;
	dts = hfw->hf_options->draw;
	//  Options dialog
	if (dts->draw_crater_dialog) {
		gtk_widget_hide(dts->current_draw_tools);
		dts->current_draw_tools = dts->draw_crater_dialog;
		gtk_widget_show(dts->current_draw_tools);
	}
	dts->context = DRAW_CRATER_CONTEXT;
}

void draw_hf_callb(GtkWidget *wdg,gpointer data) {
//	Displays the widgets for painting in the current HF
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
//	The user has clicked another choice
			hf_commit_or_reset (hfw->hf_options);
			return;
		}
	if (!hfw) return;
	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->pencil_cursor;
	dts = hfw->hf_options->draw;
	//  Options dialog
	if (dts->draw_hf_dialog) {
		gtk_widget_hide(dts->current_draw_tools);
		dts->current_draw_tools = dts->draw_hf_dialog;
		gtk_widget_show(dts->current_draw_tools);
	}
	dts->context = DRAW_HF_CONTEXT;
}

void swirl_callb(GtkWidget *wdg,gpointer data) {
//	Displays the widgets for swirling a circular zone in the current HF
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
//	The user has clicked another choice
			hf_commit_or_reset (hfw->hf_options);
			return;
		}
	if (!hfw) return;
	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->pencil_cursor;
	dts = hfw->hf_options->draw;
	//  Options dialog
	if (dts->swirl_dialog) {
		gtk_widget_hide(dts->current_draw_tools);
		dts->current_draw_tools = dts->swirl_dialog;
		gtk_widget_show(dts->current_draw_tools);
	}
	dts->context = SWIRL_CONTEXT;
}

void fault_callb(GtkWidget *wdg,gpointer data) {
//	Displays the widgets for drawing a fault in the current HF
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
			hf_commit_or_reset (hfw->hf_options);
//	The user has clicked another choice
			return;
		}
	if (!hfw) return;
	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->pencil_cursor;
	dts = hfw->hf_options->draw;
	//  Options dialog
	if (dts->fault_dialog) {
		gtk_widget_hide(dts->current_draw_tools);
		dts->current_draw_tools = dts->fault_dialog;
		gtk_widget_show(dts->current_draw_tools);
	}
	dts->context = FAULT_CONTEXT;
}

void fill_callb(GtkWidget *wdg,gpointer data) {
//	Displays the widgets for filling a zone in the current HF
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
			hf_commit_or_reset (hfw->hf_options);
//	The user has clicked another choice
			return;
		}
	if (!hfw) return;
	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->pencil_cursor;
	dts = hfw->hf_options->draw;
	//  Options dialog
	if (dts->fill_dialog) {
		gtk_widget_hide(dts->current_draw_tools);
		dts->current_draw_tools = dts->fill_dialog;
		gtk_widget_show(dts->current_draw_tools);
	}
	dts->context = FILL_CONTEXT;
	hfw->if_rgb = TRUE;
}

//	The next functions manage mouse movements tracking during drawing

void begin_stroke (draw_tools_struct *dts, gdouble x, gdouble y) {
//	Begin a multiple-line stroke
// printf("BEGIN STROKE AT (%5.0f, %5.0f)\n",x,y);
	dts->draw_begin_x = x;
	dts->draw_begin_y = y;
	dts->line_drawing = TRUE;		// TRUE until we release the button
}

void end_stroke (draw_tools_struct *dts) {
	dts->line_drawing = FALSE;
}


static void press_draw_hf_context (hf_wrapper_struct *hfw) {
	draw_tools_struct *dts = hfw->hf_options->draw;
	begin_pending_record(hfw,"Draw HF",accept_draw_fn,reset_draw_fn);
	stroke_dot (hfw->hf_struct, dts->hf_pen,
		LEFT_SHIFT((gint) dts->draw_begin_x, get_scale(hfw->sbs)),
		LEFT_SHIFT((gint) dts->draw_begin_y,get_scale(hfw->sbs)));
	(*hfw->if_modified) = TRUE;
	draw_hf(hfw);
}

static void press_draw_crater_context (hf_wrapper_struct *hfw) {

	draw_tools_struct *dts = hfw->hf_options->draw;
	begin_pending_record(hfw,"Draw a crater",accept_draw_fn,reset_draw_fn);
	draw_crater(dts->crater_pen,
		hfw->hf_struct,
		LEFT_SHIFT((gint) dts->draw_begin_x, get_scale(hfw->sbs)),
		LEFT_SHIFT((gint) dts->draw_begin_y,get_scale(hfw->sbs)),
		hfw->hf_options->gauss_list);
	(*hfw->if_modified) = TRUE;
	draw_hf(hfw);
}

static void press_fill_context (hf_wrapper_struct *hfw) {

	draw_tools_struct *dts = hfw->hf_options->draw;
	begin_pending_record(hfw,"Fill", accept_fill_fn, reset_fill_fn);
	
	select_for_fill (dts->fill_pen,
		hfw->hf_struct,
		LEFT_SHIFT((gint) dts->draw_begin_x, get_scale(hfw->sbs)),
		LEFT_SHIFT((gint) dts->draw_begin_y,get_scale(hfw->sbs)));

	add_seed (dts->fill_pen,
		LEFT_SHIFT((gint) dts->draw_begin_x, get_scale(hfw->sbs)),
		LEFT_SHIFT((gint) dts->draw_begin_y, get_scale(hfw->sbs)));
		
	update_fill_area (NULL, &hfw);
}

static void press_fault_context (hf_wrapper_struct *hfw, GdkEventButton *event) {

// printf("Press_fault_context\n");

draw_tools_struct *dts = hfw->hf_options->draw;
if (!hfw->control_line) {
	hfw->control_line =
		control_line_new (hfw->area,
			(gint) dts->draw_begin_x,
			(gint) dts->draw_begin_y,
			(gint) dts->draw_begin_x,
			(gint) dts->draw_begin_y);
	control_line_unactivate (hfw->control_line);
}
if (hfw->control_line->active) { // The user is stretching / compressing the control line
	if (test_control_line (hfw->control_line, event->x, event->y)) {
		if (hfw->control_line->mouse_on_0) {
			control_line_update0 (hfw->control_line, event->x, event->y);
			begin_stroke (dts, hfw->control_line->x1, hfw->control_line->y1);
			hfw->control_line->revert_ends = ~hfw->control_line->revert_ends ;
		}
		if (hfw->control_line->mouse_on_1) {
			control_line_update1 (hfw->control_line, event->x, event->y);
			begin_stroke (dts, hfw->control_line->x0, hfw->control_line->y0);
//							hfw->control_line->revert_ends = FALSE ;
		}
	} // test_control_line
	else
		if (dts->fault_auto_accept_flag) { // We begin a new control line
			accept_fault_fn(hfw);
			begin_pending_record(hfw,"Fault",accept_fault_fn, reset_fn);
			refresh_seed_from_seedbox (NULL, dts->fault_line_dialog->seed_dialog);
			begin_fault (dts->fault_pen, hfw->hf_struct);
			control_line_activate (hfw->control_line);
			gtk_widget_set_sensitive (GTK_WIDGET(dts->fault_accept), TRUE);
		}

} // control_line->active
else { // The user begins a new control line
	begin_pending_record(hfw,"Fault",accept_fault_fn, reset_fn);
	begin_fault (dts->fault_pen, hfw->hf_struct);
	control_line_activate (hfw->control_line);
	gtk_widget_set_sensitive (GTK_WIDGET(dts->fault_accept), TRUE);
}
}

static void press_draw_context (hf_wrapper_struct *hfw) {

	draw_tools_struct *dts = hfw->hf_options->draw;
	begin_pending_record(hfw,"Stroke",accept_draw_fn,reset_draw_fn);
	// We don't draw the 1st dot now because we need to know a
	// second point for rotating the pen tip, when it is not regular
	// We need a temporary buffer for avoiding
	// overflows when the stroke crosses itself
	draw_buf_init (dts->pen->map, dts->pen->spacing);
	if ((dts->pen->merge!=SMOOTH_OP) && (!dts->pen->overlap)) {
		hf_init_layer (hfw->hf_struct);
		hf_init_tmp2_buf_from_hf (hfw->hf_struct);
	}
}

gint button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) data;
	if (!hfw)
		return TRUE;
//	printf("BUTTON_PRESS (%d,%d)\n",(gint) event->x, (gint) event->y);
	if (hfw->hf_options->context == TRANSFORM_CONTEXT)
		return TRUE;
	dts = hfw->hf_options->draw;
	// 	We test the DRAW_CONTEXT for beginning the undo / redo transaction
	//	We draw lines on motion_notify event or on release_button
	//	We commit the undo / redo transaction on release_button
	if (event->button == 1 && hfw->area != NULL) {

		begin_stroke (dts, event->x, event->y);
		// Decrease the detail for speedying the drawing
		gl_details_min(hfw->gl_preview);
		switch (dts->context) {
			case DRAW_HF_CONTEXT:
				press_draw_hf_context (hfw);
				break;
			case DRAW_CRATER_CONTEXT:
				press_draw_crater_context (hfw);
				break;
			case FILL_CONTEXT:
				press_fill_context (hfw);
				break;
			case DRAW_CONTEXT:
				press_draw_context (hfw);
				break;
			case SWIRL_CONTEXT:
				break;
			case FAULT_CONTEXT:
				press_fault_context (hfw, event);
				break;
		}
	}
	return TRUE;
}

static void release_draw_context (hf_wrapper_struct *hfw, gdouble *gdx, gdouble *gdy) {

	draw_tools_struct *dts = hfw->hf_options->draw;

	if (!hfw->mouse_dragging)  { // Draw at least one dot
		draw_one_dot (hfw->hf_struct, hfw->hf_options->draw->pen,
			LEFT_SHIFT((gint) dts->draw_begin_x, get_scale(hfw->sbs)), 
			LEFT_SHIFT((gint) dts->draw_begin_y, get_scale(hfw->sbs)),
			hfw->hf_options->gauss_list);
		(*hfw->if_modified) = TRUE;
		draw_hf(hfw);
	}
	else {
		if (draw_line_n_refresh(hfw,
			dts->draw_begin_x,
			dts->draw_begin_y,
			gdx, gdy, FALSE) )
			(*hfw->if_modified) = TRUE;
	}
	(*hfw->to_register_in_history) = TRUE;
	if ((dts->pen->merge!=SMOOTH_OP) && (!dts->pen->overlap)) {
		hf_update_hf_with_layer (hfw->hf_struct, dts->pen->merge);
		hf_cancel_layer (hfw->hf_struct);
		draw_hf(hfw);
	}
	commit_pending_record(hfw);
}

static void release_fault_context (hf_wrapper_struct *hfw) {

	if (hfw->control_line->active) {

		hfw->hf_options->draw->fault_pen->mode = FAULT_FINAL;
		if (hfw->control_line->revert_ends) {
			compute_fault (hfw->hf_options->draw->fault_pen,
				hfw->hf_struct,
				LEFT_SHIFT((gint) hfw->control_line->x1, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->y1, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->x0, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->y0, get_scale(hfw->sbs)),
				hfw->hf_options->dist_matrix,
				hfw->hf_options->gauss_list);
		}
		else {
			compute_fault (hfw->hf_options->draw->fault_pen, 
				hfw->hf_struct,
				LEFT_SHIFT((gint) hfw->control_line->x0, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->y0, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->x1, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->y1, get_scale(hfw->sbs)),
				hfw->hf_options->dist_matrix,
				hfw->hf_options->gauss_list);
		}
	} // control_line->active
	control_line_undrag (hfw->control_line);
	draw_hf(hfw);
	(*hfw->if_modified) = TRUE;
}

gint button_release (GtkWidget *widget, GdkEventButton *event, gpointer data) {
	gint x,y;
	gdouble gdx,gdy;
	hf_wrapper_struct *hfw;
	draw_tools_struct *dts;
	hfw = (hf_wrapper_struct *) data;
	if (!hfw)
		return TRUE;

	if (hfw->hf_options->context == TRANSFORM_CONTEXT)
		return TRUE;

	dts = hfw->hf_options->draw;

	x = (gint) event->x;
	y = (gint) event->y;
	gdx = (gdouble) x;
	gdy = (gdouble) y;

	if (dts->line_drawing) {
	gl_details_restore (hfw->gl_preview);
	gl_area_update (hfw->gl_preview);

	switch (dts->context) {

		case DRAW_HF_CONTEXT	:
			if (draw_hf_line (hfw->hf_struct, dts->hf_pen, 
				(gint) dts->draw_begin_x,
				(gint) dts->draw_begin_y,
				&x,&y,
				get_scale(hfw->sbs))) {
				draw_hf (hfw);
				(*hfw->if_modified) = TRUE;
				(*hfw->to_register_in_history) = TRUE;
			}
			commit_pending_record(hfw);
			break;
		case DRAW_CONTEXT:
			release_draw_context (hfw, &gdx, &gdy);
			break;
		case FAULT_CONTEXT:
			release_fault_context (hfw);

	} // switch (dts->context)
	} // dts->line_drawing

	hfw->mouse_dragging = FALSE;
	end_stroke (dts);
	return TRUE;
}

static void notify_draw_context (hf_wrapper_struct *hfw, gdouble *gdx, gdouble *gdy) {

	draw_tools_struct *dts = hfw->hf_options->draw;

	if (!hfw->mouse_dragging) { // Draw the 1st dot
		draw_one_dot (hfw->hf_struct, 
			hfw->hf_options->draw->pen,
			LEFT_SHIFT((gint) 
				dts->draw_begin_x, 
				get_scale(hfw->sbs)), 
			LEFT_SHIFT((gint) 
				dts->draw_begin_y,
				get_scale(hfw->sbs)),
			hfw->hf_options->gauss_list);
		draw_hf(hfw);
		(*hfw->if_modified) = TRUE;
	}
	hfw->mouse_dragging = TRUE;
	if (!draw_line_n_refresh(hfw, 
		dts->draw_begin_x,
		dts->draw_begin_y,
		gdx,gdy, FALSE) ) {
		if ((*hfw->if_modified) &&
			(dts->pen->merge!=SMOOTH_OP) && 
			(!dts->pen->overlap))		
			hf_update_hf_with_layer (hfw->hf_struct, dts->pen->merge);
		return;
		}
	(*hfw->if_modified) = TRUE;
	dts->draw_begin_x = *gdx;
	dts->draw_begin_y = *gdy;
	if ((dts->pen->merge!=SMOOTH_OP) && (!dts->pen->overlap))
		hf_update_hf_with_layer 
			(hfw->hf_struct, 
			dts->pen->merge);
	return;
}

static void notify_fault_context (hf_wrapper_struct *hfw, gint x, gint y) {

	draw_tools_struct *dts = hfw->hf_options->draw;

	if (!hfw->control_line)
		return; // Not supposed to!

	if (hfw->control_line->active) {
		control_line_update (hfw->control_line,
			(gint) dts->draw_begin_x,
			(gint) dts->draw_begin_y, x,y);

		dts->fault_pen->mode = FAULT_PREVIEW;
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
				LEFT_SHIFT((gint) hfw->control_line->x0, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->y0, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->x1, get_scale(hfw->sbs)),
				LEFT_SHIFT((gint) hfw->control_line->y1, get_scale(hfw->sbs)),
				hfw->hf_options->dist_matrix,
				hfw->hf_options->gauss_list);
		}
		draw_hf(hfw);
	}
}

gint motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	//	Tracks the mouse
	//	This function is connected once for each open document
	//	So:  data is a hf_wrapper_struct * instead of a hf_wrapper_struct **
	gint x,y;
	gdouble gdx, gdy;
	static gchar buf[20];
	GdkModifierType state;
	draw_tools_struct *dts;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) data;
	if (!hfw)
		return TRUE;

	dts = hfw->hf_options->draw;
	if (event->is_hint)
		gdk_window_get_pointer(event->window, &x, &y, &state);
	else {
		x = (gint) event->x;
		y = (gint) event->y;
		state = event->state;
	}

	//	If we are dragging, x and y can be < 0, a behavior we don't want
	x = MIN(hfw->area_size-1,MAX(0,x));
	y = MIN(hfw->area_size-1,MAX(0,y));
	gdx = (gdouble) x;
	gdy = (gdouble) y;
// printf("MOTION_NOTIFY (%d,%d)\n",x,y);
	if (hfw->mouse_position) {
		snprintf(buf,20,"(%4d,%-4d) %-7d",
			LEFT_SHIFT(x, get_scale(hfw->sbs)),
			LEFT_SHIFT(y, get_scale(hfw->sbs)),
			*(hfw->hf_struct->hf_buf+VECTORIZE(LEFT_SHIFT(x, get_scale(hfw->sbs)),
			LEFT_SHIFT(y, get_scale(hfw->sbs)),
			hfw->hf_struct->max_x)));
		gtk_label_set_text(GTK_LABEL(hfw->mouse_position),buf);
	}

	if (hfw->hf_options->context == TRANSFORM_CONTEXT)
		return TRUE;
		
	if ((dts->context==FAULT_CONTEXT) && hfw->control_line) {
		if (hfw->control_line->active) {
			if (test_control_line(hfw->control_line, x, y)) {
				gdk_window_set_cursor (hfw->area->window,
					hfw->hf_options->fleur_cursor);
				hfw->hf_options->current_cursor = hfw->hf_options->fleur_cursor;
			}
			else {
				gdk_window_set_cursor (hfw->area->window,
					hfw->hf_options->default_cursor);
				hfw->hf_options->current_cursor = hfw->hf_options->default_cursor;
			}
			gdk_flush();
		}
	}
	// Mouse button down??
	// Only relevant to DRAW_HF_CONTEXT, DRAW_CONTEXT, FAULT_CONTEXT

	if ((dts->context==DRAW_HF_CONTEXT) ||
		(dts->context==FAULT_CONTEXT) ||
		(dts->context==DRAW_CONTEXT) ) {
		if (state & GDK_BUTTON1_MASK && (hfw->area!=NULL) && dts->line_drawing) {

// printf("GDK_BUTTON1_MASK... DRAGGING: %d\n",hfw->mouse_dragging);
			
			switch (dts->context) {

				case DRAW_HF_CONTEXT	:
					hfw->mouse_dragging = TRUE;
					if (!draw_hf_line (hfw->hf_struct,
						dts->hf_pen, 
						(gint) dts->draw_begin_x,
						(gint) dts->draw_begin_y,
						&x,&y,
						get_scale(hfw->sbs)))
						return TRUE;
					draw_hf (hfw);
					(*hfw->if_modified) = TRUE;
					dts->draw_begin_x = (gdouble) x;
					dts->draw_begin_y = (gdouble) y;
					break;

				case DRAW_CONTEXT:
					notify_draw_context (hfw, &gdx, &gdy);
					break;
				case SWIRL_CONTEXT:
					break;
				case FAULT_CONTEXT:
					notify_fault_context (hfw, x, y);
					break;
			} // Switch dts->context
		} // GDK_BUTTON1_MASK test

	} // DRAW_.._CONTEXT test
	return TRUE;
}
