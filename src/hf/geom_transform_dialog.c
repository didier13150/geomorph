/* geom_transform_dialog.h:  geometric transformations
 * Processes called in "img_process_dialog.c"
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "img_process_dialog.h"
#include "hf_wrapper.h"
#include "geom_transform_dialog.h"

//	Local prototype
static void apply_rotate (hf_wrapper_struct *hfw);

/************************* HORIZONTAL MIRROR *************************/

GtkWidget *mirror_horizontal_dialog_new(gpointer data) {
	GtkWidget *lbl, *frame;
	//hf_wrapper_struct *hfw;
	//hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	frame = options_frame_new("Horizontal mirror");
	lbl = gtk_label_new(_("No parameters\nfor this operation"));
	gtk_widget_show(lbl);
	gtk_container_add(GTK_CONTAINER(frame), lbl);
	return frame;
}

void mirror_horizontal_callb(GtkWidget *wdg, gpointer data) {

	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->mirror_horizontal_dialog) {
		hfw->hf_options->img->mirror_horizontal_dialog = mirror_horizontal_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->mirror_horizontal_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->mirror_horizontal_dialog;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
	hf_backup(hfw->hf_struct);	// We need to copy the result in the tmp_buf if we want to allow
					// the user to revert the operation by clicking repeatedly on the mirror button
	hf_horizontal_mirror(hfw->hf_struct);
	begin_pending_record(hfw,"Horizontal mirror",accept_fn,reset_fn);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

/************************* VERTICAL MIRROR *************************/

GtkWidget *mirror_vertical_dialog_new(gpointer data) {
	GtkWidget *lbl, *frame;
	//hf_wrapper_struct *hfw;
	//hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	frame = options_frame_new("Vertical mirror");
	lbl = gtk_label_new(_("No parameters\nfor this operation"));
	gtk_widget_show(lbl);
	gtk_container_add(GTK_CONTAINER(frame), lbl);
	return frame;
}

void mirror_vertical_callb(GtkWidget *wdg, gpointer data) {

	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->mirror_vertical_dialog) {
		hfw->hf_options->img->mirror_vertical_dialog = mirror_vertical_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->mirror_vertical_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->mirror_vertical_dialog;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
	hf_backup(hfw->hf_struct);	// We need to copy the result in the tmp_buf if we want to allow
					// the user to revert the operation by clicking repeatedly on the mirror button
	hf_vertical_mirror(hfw->hf_struct);
	begin_pending_record(hfw,"Vertical mirror",accept_fn,reset_fn);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

/************************* ROTATE *************************/

void rotate_remove_tiling (hf_wrapper_struct *hfw) {
	// Rotation generally does not preserve the tiling capability, except for 0, 90, 180, 270, 360 angles
	if (hfw->tiling_ptr && (hfw->hf_options->img->angle%90)) {
		*hfw->tiling_ptr = FALSE;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hfw->tiling_wdg), *hfw->tiling_ptr);
	}
}

void accept_rotate_fn (hf_wrapper_struct *hfw) {
//	Special accept callback, could be used for committing the rotation
//	with a high quality anti-aliased but slow rotate function,
//	when the preview is done with a fast rotate function
//	See the difference between hf_fast_rotate and hf_rotate in hf_c
//	2002-06-16:  finally we do the preview with the normal rotate function,
//	this is not so bad with a 512x512 HF at 1 ghz speed!
//	Execute the anti-aliased rotation if not yet calculated
	if (!hfw)
		return;
	if (!hfw->if_calculated) {
		apply_rotate(hfw);
	}
	commit_pending_record(hfw);
	hf_backup(hfw->hf_struct);
	if (hfw->hf_options->img->set_fn)
		(*hfw->hf_options->img->set_fn) ((gpointer)hfw->hf_options->img);
	rotate_remove_tiling (hfw);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->accept_wdg),FALSE);
}

void apply_rotate (hf_wrapper_struct *hfw) {
//	Apply anti-aliased rotation, without redrawing the HF
	//gint t1;
	gboolean tiling;
	if (hfw->tiling_ptr) {
		tiling = *hfw->tiling_ptr;
//		printf("HFW->tiling_ptr in apply_rotate: %p->%p\n",hfw,hfw->tiling_ptr);
	}
	else
		tiling = TRUE;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
	if (hfw->if_calculated)
		return;
	//t1 = clock();
	hf_rotate(hfw->hf_struct->tmp_buf, hfw->hf_struct->hf_buf,
		hfw->hf_struct->max_x,
		hfw->hf_options->img->angle, 
		tiling,
		tiling ?
		hfw->hf_options->img->rotate_overflow_tiling : 
		hfw->hf_options->img->rotate_overflow_notiling);
//	printf("TEMPS DE ROTATION: %d; angle: %d\n",clock() - t1, hfw->hf_options->img->angle);
	begin_pending_record(hfw,"Rotate",accept_rotate_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->rotate_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
}

void apply_rotate_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->if_calculated)
		return;
	apply_rotate(hfw);
//	printf("Apply_rotate in apply_rotate_callb\n");
	draw_hf(hfw);
}

void rotate_remove_tiling_callb (GtkWidget *wdg, gpointer data) {
	// Rotation generally does not preserve the tiling capability, except for 0, 90, 180, 270, 360 angles
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	rotate_remove_tiling (hfw);
}

void accept_rotate_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	accept_rotate_fn (hfw);
}

void set_rotate_defaults(img_dialog_struct *img) {
//	printf("Set rotate defaults\n");
	gtk_adjustment_set_value(
			GTK_ADJUSTMENT(img->adj_rotate), 0);
}

void rotate_angle_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->angle == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->angle = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
//	Replace with hf_fast_rotate, if too slow
//	NB:  hf_fast_rotate creates inconsistencies in the commitment sequence
//	... sometimes part of the process is not applied when the change is committed ??
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hfw->hf_options->img->auto_rotate))) {
		apply_rotate(hfw);
		draw_hf(hfw);
	}
}

GtkWidget *rotate_dialog_new (gpointer data) {

	GtkWidget *vbox, *hbox, *frame, *button, *scale;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Rotate");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Angle", hbox, FALSE,TRUE, DEF_PAD);
 	hfw->hf_options->img->adj_rotate = gtk_adjustment_new (0, 0.0, 360.0, 1.0, 1.0, 0.0);
	scale = define_scale_in_box(hfw->hf_options->img->adj_rotate,hbox,0,DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (hfw->hf_options->img->adj_rotate), "value_changed",
		GTK_SIGNAL_FUNC (rotate_angle_upd), data);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	rotate_buttons_new(hbox, (gpointer) hfw->hf_options->img->adj_rotate);

	hfw->hf_options->img->auto_rotate =
		define_check_button_in_box ("Auto update",hbox, FALSE,FALSE,0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (hfw->hf_options->img->auto_rotate),
		TRUE);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label (_("Apply"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) apply_rotate_callb, data);
	gtk_widget_set_sensitive(GTK_WIDGET(button),
		!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hfw->hf_options->img->auto_rotate)));
	gtk_widget_show(button);

//	The "auto-rotate" button unactivates the "apply" button
	gtk_signal_connect (GTK_OBJECT(hfw->hf_options->img->auto_rotate ), "toggled",  
		GTK_SIGNAL_FUNC(unsensitive_if_true), button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Reset"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) reset_callb, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Accept"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) rotate_remove_tiling_callb, data);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) accept_rotate_callb, data);
	gtk_widget_show(button);	
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	hfw->hf_options->img->rotate_accept = button;

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void rotate_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->rotate_dialog) {
		hfw->hf_options->img->rotate_dialog = rotate_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->rotate_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->rotate_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_rotate_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->rotate_accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* SLIDER (TRANSLATION) *************************/

void set_slide_defaults(img_dialog_struct *img) {
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_slidev), 0);
	gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(img->adj_slideh), 0);
}

void calc_slider (hf_wrapper_struct *hfw) {
	if (hfw->if_calculated)
		return;
	hf_slide(hfw->hf_struct,
		hfw->hf_options->img->slideh,
		hfw->hf_options->img->slidev);
	begin_pending_record(hfw,"Translate",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->slide_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

void slidev_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->slidev == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->slidev = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_slider(hfw);
}

void slideh_upd (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->img->slideh == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	hfw->hf_options->img->slideh = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	calc_slider(hfw);
}

GtkWidget *slide_dialog_new(gpointer data) {
	GtkWidget *vbox, *hbox, *frame, *table, *scale;
	GtkObject *adj;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Translate");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	table = gtk_table_new(4, 2, FALSE);
	gtk_widget_show(GTK_WIDGET(table));

	define_label_in_table("Horizontal",table,0, 1, 0, 1, DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->slideh, -100, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 0, 1, 0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	hfw->hf_options->img->adj_slideh = adj;
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (slideh_upd), data);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
	percent_buttons_negative_new (hbox, (gpointer) adj);	
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 1, 2, 0, 0, 0, 0);
	
	define_label_in_table("Vertical",table,0, 1, 2, 3, DEF_PAD);
	adj = gtk_adjustment_new (hfw->hf_options->img->slidev, -100, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 2, 3, 0, DEF_PAD);
	optimize_on_mouse_click (scale, data);
	hfw->hf_options->img->adj_slidev = adj;
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (slidev_upd), data);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
	percent_buttons_negative_new (hbox, (gpointer) adj);	
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 3, 4, 0, 0, 0, 0);
	
  	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->slide_accept), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void slide_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->slide_dialog) {
		hfw->hf_options->img->slide_dialog = slide_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->slide_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->slide_dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_slide_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->slide_accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}

/************************* STRETCH / COMPRESS *************************/

void calc_stretch(hf_wrapper_struct *hfw) {
	begin_pending_record(hfw,"Stretch",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->stretch_accept),TRUE);
	(*hfw->if_modified) = TRUE;
	draw_hf(hfw);
}

void stretch_v_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_stretch_v(hfw->hf_struct);
	calc_stretch(hfw);
}

void stretch_h_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hf_stretch_h(hfw->hf_struct);
	calc_stretch(hfw);
}

GtkWidget *stretch_dialog_new(gpointer data) {
	GtkWidget *vbox, *hbox, *frame, *button;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	frame = options_frame_new("Stretch and compress");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label (_("Stretch V"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) stretch_v_callb, data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Stretch H"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) stretch_h_callb, data);
	gtk_widget_show(button);	
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);

	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->stretch_accept), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void stretch_compress_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		hf_commit_or_reset (hfw->hf_options);
		return;
	}
	if (!hfw->hf_options->img->stretch_dialog) {
		hfw->hf_options->img->stretch_dialog = stretch_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->stretch_dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->stretch_dialog;
	hfw->hf_options->img->set_fn = NULL;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->stretch_accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}
