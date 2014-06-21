/* merge_dialog.c - Merge utility functions (merging source - result)
 *
 * Copyright (C) 2002 Patrice St-Gelais
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
#include "merge_dialog.h"
#include "hf_wrapper.h"
#include "../utils/menus_n_tools.h"

static generic_callb (gpointer data, gint merge_op) {
	merge_struct *mrg = (merge_struct *) data;
	mrg->merge_op = merge_op;
	merge_calc (mrg);
}

static void merge_add_callb (GtkWidget *wdg, gpointer data) {
	generic_callb (data, ADD);
}

static void merge_subtract_callb (GtkWidget *wdg, gpointer data) {
	generic_callb (data, SUBTRACT);
}

static void merge_multiply_callb (GtkWidget *wdg, gpointer data) {
	generic_callb (data, MULTIPLY);
}

static void merge_min_callb (GtkWidget *wdg, gpointer data) {
	generic_callb (data, MIN_MERGE);
}

static void merge_max_callb (GtkWidget *wdg, gpointer data) {
	generic_callb (data, MAX_MERGE);
}

static void merge_altitude_callb (GtkWidget *wdg, gpointer data) {
	generic_callb (data, ALT_MERGE);
}

static void mix_upd (GtkWidget *wdg, gpointer data) {
	merge_struct *mrg = (merge_struct *) data;
	if (mrg->mix == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	mrg->mix = (gint) GTK_ADJUSTMENT(wdg)->value;
	merge_calc (mrg);
}

static void hardness_upd (GtkWidget *wdg, gpointer data) {
	merge_struct *mrg = (merge_struct *) data;
	if (mrg->mix == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	mrg->hardness = (gdouble) GTK_ADJUSTMENT(wdg)->value;
	merge_calc (mrg);
}

static void source_offset_upd (GtkWidget *wdg, gpointer data) {
	merge_struct *mrg = (merge_struct *) data;
	if (mrg->source_offset == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	mrg->source_offset = (gint) GTK_ADJUSTMENT(wdg)->value;
	merge_calc (mrg);
}

static void result_offset_upd (GtkWidget *wdg, gpointer data) {
	merge_struct *mrg = (merge_struct *) data;
	if (mrg->result_offset == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	mrg->result_offset = (gint) GTK_ADJUSTMENT(wdg)->value;
	merge_calc (mrg);
}

static void merge_x_translate_upd (GtkWidget *wdg, gpointer data) {
	merge_struct *mrg = (merge_struct *) data;
	if (mrg->x_translate == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	mrg->x_translate = (gint) GTK_ADJUSTMENT(wdg)->value;
	merge_calc (mrg);
}

static void merge_y_translate_upd (GtkWidget *wdg, gpointer data) {
	merge_struct *mrg = (merge_struct *) data;
	if (mrg->y_translate == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	mrg->y_translate = (gint) GTK_ADJUSTMENT(wdg)->value;
	merge_calc (mrg);
}

merge_dialog_struct *merge_dialog_struct_new (merge_struct* data) {
	merge_dialog_struct *mrg_dialog;
	mrg_dialog = (merge_dialog_struct *) x_malloc(sizeof(merge_dialog_struct), "merge_dialog_struct");
	mrg_dialog->merge_dialog = NULL;
	mrg_dialog->adj_mix = NULL;
	mrg_dialog->adj_source_offset = NULL;
	mrg_dialog->adj_result_offset = NULL;
	mrg_dialog->main_box = NULL;
	mrg_dialog->adj_hardness = NULL;
	mrg_dialog->content = data;
	return mrg_dialog;
}

void merge_set_mix (merge_dialog_struct *merge, gint new_mix) {
	if ( (new_mix>-101) && (new_mix<101) )
		gtk_adjustment_set_value(GTK_ADJUSTMENT(merge->adj_mix), new_mix);
}

void merge_set_op (merge_dialog_struct *mrg, gint merge_op) {
	mrg->content->merge_op = merge_op;
	switch (merge_op) {
		case SUBTRACT:
			set_toolbar_button_from_order(mrg->op_toolbar,1,TRUE);
			break;
		case MULTIPLY:
			set_toolbar_button_from_order(mrg->op_toolbar,2,TRUE);
			break;
		case MIN_MERGE:
			set_toolbar_button_from_order(mrg->op_toolbar,3,TRUE);
			break;
		case MAX_MERGE:
			set_toolbar_button_from_order(mrg->op_toolbar,4,TRUE);
			break;
		case ALT_MERGE:
			set_toolbar_button_from_order(mrg->op_toolbar,5,TRUE);
			break;
		case ADD:
		default:
			set_toolbar_button_from_order(mrg->op_toolbar,0,TRUE);
	}
}

GtkWidget *merge_dialog_new (merge_dialog_struct *mrg,
	gboolean hide_control,
	GtkWidget *parent_window,
	gchar *frame_title,
	gchar *source_title,
	gchar *result_title,
	gchar *mix_title) {
//	Standard controls for merging the source HF with the result of a transformation

	GtkWidget *frame, *frame2, *vbox,  *table, *hbox, *button, *scale, *but1, *but2, *but3, *but4, *but5, *but6;
	GtkObject *adj;

//	Titles default to predefined values when NULL
	if (!source_title)
		source_title = "Source";
	if (!result_title)
		result_title = "Result";
	if (!frame_title)
		frame_title = "Source & result merge";
	if (!mix_title)
		mix_title = "Mix";

	frame = frame_new (frame_title, DEF_PAD*0.5);

	vbox = gtk_vbox_new(FALSE,DEF_PAD*0.5);
	mrg->main_box = vbox;
	gtk_widget_show(vbox);
	if (hide_control) {
		frame = define_frame_with_hiding_arrows (frame_title, vbox, parent_window, TRUE);
	}
	else {
		frame = frame_new (frame_title, DEF_PAD*0.5);
		gtk_container_add(GTK_CONTAINER(frame),vbox);
	}
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("Mode", hbox, FALSE, FALSE, DEF_PAD *0.5);

	mrg->op_toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(mrg->op_toolbar),GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(mrg->op_toolbar),GTK_TOOLBAR_TEXT);
	gtk_widget_show(GTK_WIDGET(mrg->op_toolbar));
	but1 = gtk_toolbar_append_element(GTK_TOOLBAR(mrg->op_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,"+",
		_("Add"),NULL,NULL,GTK_SIGNAL_FUNC(merge_add_callb),(gpointer) mrg->content);
	but2 = gtk_toolbar_append_element(GTK_TOOLBAR(mrg->op_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON, but1,"-",
		_("Subtract"),NULL,NULL,GTK_SIGNAL_FUNC(merge_subtract_callb),(gpointer) mrg->content);
	but3 = gtk_toolbar_append_element(GTK_TOOLBAR(mrg->op_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,but1,"x",
		_("Multiply"),NULL,NULL,GTK_SIGNAL_FUNC(merge_multiply_callb),(gpointer) mrg->content);
	but4 = gtk_toolbar_append_element(GTK_TOOLBAR(mrg->op_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,but1,_("min"),
		_("Minimum"),NULL,NULL,GTK_SIGNAL_FUNC(merge_min_callb),(gpointer) mrg->content);
	but5 = gtk_toolbar_append_element(GTK_TOOLBAR(mrg->op_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,but1,_("max"),
		_("Maximum"),NULL,NULL,GTK_SIGNAL_FUNC(merge_max_callb),(gpointer) mrg->content);
	but6 = gtk_toolbar_append_element(GTK_TOOLBAR(mrg->op_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,but1,_("alt"),
		_("Altitude"),NULL,NULL,GTK_SIGNAL_FUNC(merge_altitude_callb),(gpointer) mrg->content);
	
	merge_set_op (mrg, mrg->content->merge_op);
//	XOR doesn't seem interesting enough...
//	gtk_toolbar_append_element(GTK_TOOLBAR(mrg->op_toolbar),
//		GTK_TOOLBAR_CHILD_RADIOBUTTON,but1,"^",
//		_("Exclusive OR (XOR)"),NULL,NULL,GTK_SIGNAL_FUNC(merge_xor_callb),(gpointer) mrg);

	gtk_box_pack_start(GTK_BOX(hbox),mrg->op_toolbar, TRUE, TRUE, DEF_PAD *0.5);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	define_label_in_box(mix_title,hbox,FALSE,FALSE,DEF_PAD);
	define_label_in_box(source_title, hbox, FALSE, FALSE, 0);
	adj = gtk_adjustment_new (mrg->content->mix, -100, 100, 1, 1, 0.01);
	scale = define_scale_in_box(adj,hbox,0, 0);
	optimize_on_mouse_click (scale, mrg->content->data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (mix_upd), (gpointer) mrg->content);
	mrg->adj_mix = adj;
	define_label_in_box(result_title, hbox,FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("0");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) zero_callb, adj);
	gtk_widget_show(button);	
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	define_label_in_box("Hardness",hbox, FALSE, FALSE, DEF_PAD);
	adj = gtk_adjustment_new (mrg->content->hardness, 0.0, 1.0, 0.01, 0.01, 0.01);
	scale = define_scale_in_box(adj,hbox,2,DEF_PAD);
	optimize_on_mouse_click (scale, mrg->content->data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (hardness_upd), (gpointer) mrg->content);
	mrg->adj_hardness = adj;
	// Display the hardness scale only for altitude merge:
	gtk_widget_hide (GTK_WIDGET(scale));
	gtk_signal_connect (GTK_OBJECT (but1), "clicked", (GtkSignalFunc) hide_callb, scale);
	gtk_signal_connect (GTK_OBJECT (but2), "clicked", (GtkSignalFunc) hide_callb, scale);
	gtk_signal_connect (GTK_OBJECT (but3), "clicked", (GtkSignalFunc) hide_callb, scale);
	gtk_signal_connect (GTK_OBJECT (but4), "clicked", (GtkSignalFunc) hide_callb, scale);
	gtk_signal_connect (GTK_OBJECT (but5), "clicked", (GtkSignalFunc) hide_callb, scale);
	gtk_signal_connect (GTK_OBJECT (but6), "clicked", (GtkSignalFunc) show_callb, scale);

	table = gtk_table_new(2, 3, FALSE);
	gtk_widget_show(GTK_WIDGET(table));
	frame2 = define_frame_with_hiding_arrows ("Brightness",table, parent_window, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, TRUE, TRUE, 0);

	define_label_in_table(source_title,table,0, 1, 0, 1, DEF_PAD*0.5);
	adj = gtk_adjustment_new (mrg->content->source_offset, -100, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 0, 1, 0, DEF_PAD*0.5);
	optimize_on_mouse_click (scale, mrg->content->data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (source_offset_upd), (gpointer) mrg->content);
	mrg->adj_source_offset = adj;

	button = define_button_in_table ("0", table, 2, 3, 0, 1, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) zero_callb, adj);

	define_label_in_table(result_title,table,0, 1, 1, 2, DEF_PAD*0.5);
	adj = gtk_adjustment_new (mrg->content->result_offset, -100, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 1, 2, 0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (result_offset_upd), (gpointer) mrg->content);
	optimize_on_mouse_click (scale, mrg->content->data);
	mrg->adj_result_offset = adj;

	button = define_button_in_table ("0", table, 2, 3, 1, 2, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) zero_callb, adj);

	table = gtk_table_new(2, 3, FALSE);
	gtk_widget_show(GTK_WIDGET(table));
	frame2 = define_frame_with_hiding_arrows ("Translate",table, parent_window, FALSE);

	define_label_in_table("Horizontal",table,0, 1, 0, 1, DEF_PAD);
	adj = gtk_adjustment_new (mrg->content->x_translate, -100, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 0, 1, 0, DEF_PAD);
	optimize_on_mouse_click (scale, mrg->content->data);
	mrg->adj_x_translate = adj;
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (merge_x_translate_upd), (gpointer) mrg->content);
	button = define_button_in_table ("0", table, 2, 3, 0, 1, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) zero_callb, adj);

	define_label_in_table("Vertical",table,0, 1, 1, 2, DEF_PAD);
	adj = gtk_adjustment_new (mrg->content->y_translate, -100, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1, 2, 1, 2, 0, DEF_PAD);
	optimize_on_mouse_click (scale, mrg->content->data);
	mrg->adj_y_translate = adj;
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (merge_y_translate_upd), (gpointer) mrg->content);
	button = define_button_in_table ("0", table, 2, 3, 1, 2, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) zero_callb, adj);

	gtk_box_pack_start(GTK_BOX(vbox), frame2, TRUE, TRUE, 0);

	return frame ;
}

