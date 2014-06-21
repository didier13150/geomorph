/* hf_filters_dialog.c:  dialogs and callbacks for filters
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

#include "hf_filters_dialog.h"
#include "hf_wrapper.h"

// Toolbar icons for shape filters
#define NBSHAPEFILTERS 7
#include "../icons/nofilter.xpm"
#include "../icons/evase.xpm"
#include "../icons/moyen10_2.xpm"
#include "../icons/moyen20_1_8.xpm"
#include "../icons/pointu.xpm"
#include "../icons/filt_gauss.xpm"
#include "../icons/filt_hf.xpm"

command_item_struct shapefilters[NBSHAPEFILTERS] = {
{ "Shape filter", "No filter", "No filter",0, (gchar **) nofilter_xpm, GDK_LEFT_PTR, nofilter_callb, NULL, NULL,TRUE },
{ "Shape filter", "Wide", "Wide",0, (gchar **) evase_xpm, GDK_LEFT_PTR, widefilter_callb,NULL, NULL,FALSE },
{ "Shape filter", "Med 1", "Med 1",0, (gchar **) moyen10_2_xpm, GDK_LEFT_PTR, mediumfilter1_callb, NULL, NULL,FALSE },
{ "Shape filter", "Med 2", "Med 2",0, (gchar **) moyen20_1_8_xpm, GDK_LEFT_PTR, mediumfilter2_callb,NULL, NULL,FALSE },
{ "Shape filter", "Sharp", "Sharp",0, (gchar **) pointu_xpm, GDK_LEFT_PTR, sharpfilter_callb,NULL, NULL,FALSE },
{ "Shape filter", "Param", "Gaussian bell with parameters choice",0, (gchar **) filt_gauss_xpm, GDK_LEFT_PTR, gaussfilter_callb,NULL, NULL,FALSE },
{ "Shape filter", "Image", "Filter with an open document",0, (gchar **) filt_hf_xpm, GDK_LEFT_PTR, imagefilter_callb,NULL, NULL,FALSE }
};


void nofilter_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Be sure that the filter is calculated for a uniform HF (this flag allows to not recalculate
//	the filter when a uniform filter is used repeatedly - when drawing)
	hfw->hf_options->flag_uniform_done = FALSE;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;
	hfw->hf_options->fd_struct->current_filter = NULL;	
	hfw->if_calculated = FALSE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}
	
void widefilter_callb(GtkWidget *wdg, gpointer data) {

	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->flag_uniform_done = FALSE;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
			return;
		}
//	Create the filter struct on an as-needed basis:
	if (!hfw->hf_options->wide_filter)
		hfw->hf_options->wide_filter = wide_filter_new(
				hfw->hf_struct->max_x, hfw->hf_options->dist_matrix);
	hfw->hf_options->fd_struct->current_filter = hfw->hf_options->wide_filter;
	hfw->if_calculated = FALSE;
	hfw->apply_filter = TRUE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void mediumfilter1_callb(GtkWidget *wdg, gpointer data) {

	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->flag_uniform_done = FALSE;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
			return;
		}
//	Create the filter struct on an as-needed basis:
	if (!hfw->hf_options->medium_filter1)
		hfw->hf_options->medium_filter1 = medium_filter1_new(
				hfw->hf_struct->max_x, hfw->hf_options->dist_matrix);
	hfw->hf_options->fd_struct->current_filter = hfw->hf_options->medium_filter1;
	hfw->if_calculated = FALSE;
	hfw->apply_filter = TRUE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void mediumfilter2_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->flag_uniform_done = FALSE;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))){
			return;
		}
//	Create the filter struct on an as-needed basis:
	if (!hfw->hf_options->medium_filter2)
		hfw->hf_options->medium_filter2 = medium_filter2_new(
				hfw->hf_struct->max_x, hfw->hf_options->dist_matrix);
	hfw->hf_options->fd_struct->current_filter = hfw->hf_options->medium_filter2;
	hfw->if_calculated = FALSE;
	hfw->apply_filter = TRUE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void sharpfilter_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->flag_uniform_done = FALSE;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
			return;
		}
//	Create the filter struct on an as-needed basis:
	if (!hfw->hf_options->sharp_filter)
		hfw->hf_options->sharp_filter = sharp_filter_new(
				hfw->hf_struct->max_x, hfw->hf_options->dist_matrix);
	hfw->hf_options->fd_struct->current_filter = hfw->hf_options->sharp_filter;
	hfw->if_calculated = FALSE;
	hfw->apply_filter = TRUE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void gauss_filter_apply(hf_wrapper_struct *hfw) {

//	Create the filter struct on an as-needed basis:
	if (!hfw->hf_options->gauss_filter)
		hfw->hf_options->gauss_filter = gauss_filter_new(
				hfw->hf_struct->max_x,
				hfw->hf_options->dist_matrix,
				hfw->hf_options->fd_struct->gauss_options);
//	Since this function is executed after a scale variation, we should recalculate the filter
	gauss_filter_init(hfw->hf_options->gauss_filter,hfw->hf_options->dist_matrix);
	hfw->hf_options->fd_struct->current_filter = hfw->hf_options->gauss_filter;
	hfw->hf_options->flag_uniform_done = FALSE;
	hfw->if_calculated = FALSE;
	hfw->apply_filter = TRUE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void gauss_exp_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->fd_struct->gauss_options->exp = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	gauss_filter_apply(hfw);
}

void gauss_scale_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->fd_struct->gauss_options->scale = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	gauss_filter_apply(hfw);
}

GtkWidget *gauss_dialog_new (GtkWidget *window, gpointer data) {

	GtkWidget *table, *scale;
	GtkObject *adj;

	table = gtk_table_new (2, 3, FALSE);
	gtk_widget_show(table);

	gtk_container_set_border_width (GTK_CONTAINER (table), DEF_PAD);

	define_label_in_table("Slope",table,0,1,1,2,0);
	adj = gtk_adjustment_new (CONST_E, 0, 10, 0.01, 0.01, 0.001);
	scale = define_scale_in_table(adj,table,1,2,1,2,2,DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gauss_exp_upd), data);

	define_label_in_table("Width",table,0,1,2,3,0);
	adj = gtk_adjustment_new (0.5, 0, 1, 0.001, 0.001, 0.001);
	scale = define_scale_in_table(adj,table,1,2,2,3,3,DEF_PAD);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gauss_scale_upd), data);

//	Small arrows for hiding / showing the dialog...

	return define_frame_with_hiding_arrows("Parameters for gaussian filter",table, window, TRUE);
}

void gaussfilter_callb(GtkWidget *wdg, gpointer data) {
//	Displays controls for gaussian filter
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->flag_uniform_done = FALSE;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
			if (hfw->hf_options->fd_struct->gauss_dialog)
				gtk_widget_hide(GTK_WIDGET(hfw->hf_options->fd_struct->gauss_dialog));
			return;
		}
	if (!hfw->hf_options->fd_struct->gauss_dialog) {
	//	The struct which will receive the data is created in hf_options_dialog_new
	//	Dialog creation (with some defaults from gauss_opt_new())
		hfw->hf_options->fd_struct->gauss_dialog = 
			gauss_dialog_new(hfw->hf_options->tools_window, data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->fd_struct->filter_vbox),
			hfw->hf_options->fd_struct->gauss_dialog);
	}
	gtk_widget_show(GTK_WIDGET(hfw->hf_options->fd_struct->gauss_dialog));
//	Refresh with the current / default values
	gauss_filter_apply(hfw);
}

void img_filter_apply(hf_wrapper_struct *hfw) {

//	Create the filter struct on an as-needed basis:
	if (!hfw->hf_options->image_filter)
		hfw->hf_options->image_filter = image_filter_new();
	hfw->hf_options->fd_struct->current_filter = hfw->hf_options->image_filter;
	hfw->hf_options->flag_uniform_done = FALSE;
	hfw->if_calculated = FALSE;
	hfw->apply_filter = TRUE;;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);

}

void select_listbox_callb (GtkWidget *wdg, gpointer data) {
	gchar *lbl;
	GList *node;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	gtk_label_get(GTK_LABEL(GTK_BIN(wdg)->child),&lbl);
// printf("SELECT_LISTBOX_CALLB: %s\n", lbl);
//	Find the HF address from its name:
	hfw->hf_options->fd_struct->current_filter->hf = NULL;
	for (node = hfw->hf_options->doc_list ; node; node = node->next) {
		if (!strcmp(lbl,*((hf_wrapper_struct *)node->data)->filename)) {
			hfw->hf_options->fd_struct->current_filter->hf = 
				(hf_struct_type *) ((hf_wrapper_struct *) node->data)->hf_struct;
			break;
		}
	}
	img_filter_apply(hfw);
}


void refresh_img_listbox (img_listbox_struct *lbs, gpointer data, 
	void (*select_callb) (GtkWidget *, gpointer) ) {
//	HF documents listbox initialization
	GList *node, *old_list;
	GtkWidget *item;
	hf_wrapper_struct *hfw, *hfw_tmp;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
// printf("REFRESH_LISTBOX_CALLB\n");
	old_list = hfw->hf_options->doc_list;
	if (hfw->hf_options->get_doc_list_from_type) {
		hfw->hf_options->doc_list = hfw->hf_options->get_doc_list_from_type (hfw->hf_options->all_docs, hfw->hf_options->doc_type);
	}	
	if (old_list)
		g_list_free(old_list);
		
	if (hfw->hf_options->doc_list && g_list_length(hfw->hf_options->doc_list)) {
		if (!lbs->listbox) {
			lbs->listbox = gtk_list_new();
			gtk_widget_show(lbs->listbox);
			gtk_container_add (GTK_CONTAINER (lbs->parent), lbs->listbox);
		}
		else {
			gtk_list_clear_items (GTK_LIST(lbs->listbox), 0, -1);		
		}
		if (lbs->no_doc_label)
			gtk_widget_hide (lbs->no_doc_label);
		for (node = hfw->hf_options->doc_list ; node; node = node->next) {
			hfw_tmp = (hf_wrapper_struct *) node->data;
			item = gtk_list_item_new_with_label (*hfw_tmp->filename);
	// 		printf("HFW_TMP->FILENAME: %s\n",*hfw_tmp->filename);
			gtk_widget_show(item);
			gtk_container_add (GTK_CONTAINER (lbs->listbox), item);	
			gtk_signal_connect (GTK_OBJECT(item ), "select",  
				GTK_SIGNAL_FUNC(select_callb), data);
		}
	}
	else {
		if (!lbs->no_doc_label) {
			lbs->no_doc_label = gtk_label_new(_("There is no open document\nto filter with"));
			gtk_label_set_justify (GTK_LABEL(lbs->no_doc_label), GTK_JUSTIFY_CENTER);
			gtk_widget_show (lbs->no_doc_label);
			gtk_container_add (GTK_CONTAINER (lbs->parent), lbs->no_doc_label);
		}
		if (lbs->listbox) {
			gtk_widget_hide (lbs->listbox);
		}
	}
}

GtkWidget *image_filter_dialog_new (GtkWidget *window, gpointer data) {

	GtkWidget *vbox;	
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);
	
	hfw->hf_options->fd_struct->img_listbox = img_listbox_new (vbox);
	
	refresh_img_listbox(hfw->hf_options->fd_struct->img_listbox, data, select_listbox_callb);

	return define_frame_with_hiding_arrows("Document filter choice",vbox, window, TRUE);
}

void imagefilter_callb(GtkWidget *wdg, gpointer data) {
//	Displays controls for image filter
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->flag_uniform_done = FALSE;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
			if (hfw->hf_options->fd_struct->img_filter_dialog)
				gtk_widget_hide(GTK_WIDGET(hfw->hf_options->fd_struct->img_filter_dialog));
			return;
		}
	if (!hfw->hf_options->fd_struct->img_filter_dialog) {
	//	The struct which will receive the data is created in hf_options_dialog_new
	//	dialog creation (with some defaults from gauss_opt_new())
		hfw->hf_options->fd_struct->img_filter_dialog = 
			image_filter_dialog_new(hfw->hf_options->tools_window, data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->fd_struct->filter_vbox),
			hfw->hf_options->fd_struct->img_filter_dialog);
	}
	else {
		refresh_img_listbox(hfw->hf_options->fd_struct->img_listbox, data, select_listbox_callb);
	}
	gtk_widget_show(GTK_WIDGET(hfw->hf_options->fd_struct->img_filter_dialog));
//	Refresh with the current / default values
	img_filter_apply(hfw);
}

void revert_filter_callb (GtkWidget *checkbutton, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (GTK_TOGGLE_BUTTON(checkbutton)->active)
		hfw->hf_options->fd_struct->revert_filter = TRUE;
	else
		hfw->hf_options->fd_struct->revert_filter = FALSE;
	hfw->hf_options->flag_uniform_done = FALSE;
	hfw->if_calculated = FALSE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void level_scale_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->fd_struct->filter_level = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	hfw->hf_options->flag_uniform_done = FALSE;
	hfw->if_calculated = FALSE;	
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void multiply_callb (GtkWidget *button, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw)	return;
	hfw->hf_options->fd_struct->merge_oper = MULTIPLY;
	hfw->hf_options->flag_uniform_done = FALSE;
	hfw->if_calculated = FALSE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void add_callb (GtkWidget *button, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw)	return;
	hfw->hf_options->fd_struct->merge_oper = ADD;
	hfw->hf_options->flag_uniform_done = FALSE;
	hfw->if_calculated = FALSE;
	if (hfw->hf_options->fd_struct->apply_filter)
		(*hfw->hf_options->fd_struct->apply_filter) (hfw);
}

void set_filter_merge(filter_dialog_struct *fds, gint opr) {
	GtkToolbarChild *child;
	child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(fds->merge_toolbar)->children,opr);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( child->widget), TRUE);
	if (opr)
		fds->merge_oper = MULTIPLY;
	else
		fds->merge_oper = ADD;
}

void set_filter_defaults(filter_dialog_struct *fds) {
	GtkToolbarChild *child;
	fds->current_filter = NULL;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fds->revert_button),FALSE);
	fds->revert_filter = FALSE;
	gtk_adjustment_set_value( GTK_ADJUSTMENT(fds->adj_level), 100);
	fds->filter_level = 100;
	child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(fds->merge_toolbar)->children,0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( child->widget), TRUE);
	fds->merge_oper = ADD;
	child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(fds->filter_toolbar)->children,NO_FILTER);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( child->widget), TRUE);
}

void set_filter_type (filter_dialog_struct *fds, gint filter_index) {
	GtkToolbarChild *child;
	
	child = g_list_nth_data (GTK_TOOLBAR(fds->filter_toolbar)->children,filter_index); 
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( child->widget), TRUE);
}

filter_dialog_struct *filter_toolbar_new(GtkWidget *window, GtkTooltips *tooltips, gpointer data) {

	GtkWidget *hbox, *vbox, *wdg, *scale;
	filter_dialog_struct *fds;

	fds = (filter_dialog_struct *) x_malloc(sizeof(filter_dialog_struct), "filter_dialog_struct");

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	fds->filter_toolbar = standard_toolbar_new(NBSHAPEFILTERS, shapefilters,
		tooltips,
		window,
		data,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);

	gtk_box_pack_start(GTK_BOX(hbox), align_widget( fds->filter_toolbar, 0.5, 0.5), FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox),hbox, FALSE, FALSE, DEF_PAD*0.5);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

//	Filter merge mode

	define_label_in_box("Merge", hbox, FALSE, FALSE, DEF_PAD *0.5);
	fds->merge_toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(fds->merge_toolbar),GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(fds->merge_toolbar),GTK_TOOLBAR_TEXT);
	gtk_widget_show(GTK_WIDGET(fds->merge_toolbar));
	wdg = gtk_toolbar_append_element(GTK_TOOLBAR(fds->merge_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,"+",
		_("Add"),NULL,NULL,GTK_SIGNAL_FUNC(add_callb),data);
	gtk_toolbar_append_element(GTK_TOOLBAR(fds->merge_toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,"x",
		_("Multiply"),NULL,NULL,GTK_SIGNAL_FUNC(multiply_callb),data);

	gtk_box_pack_start(GTK_BOX(hbox),fds->merge_toolbar, FALSE, FALSE, DEF_PAD *0.5);

//	Filter revert toggle button

	fds->revert_button = define_check_button_in_box ("Revert", hbox, 0, 0, DEF_PAD*0.5);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fds->revert_button), FALSE);
	gtk_signal_connect (GTK_OBJECT(fds->revert_button), "toggled",
		GTK_SIGNAL_FUNC(revert_filter_callb), data);

	gtk_box_pack_start(GTK_BOX(vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, 0);

//	Filter level scale

	define_label_in_box("%",hbox,TRUE, TRUE,0);
	fds->adj_level = gtk_adjustment_new (100, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_box(fds->adj_level,hbox,0, 0);
	optimize_on_mouse_click (scale, data);
	gtk_signal_connect (GTK_OBJECT (fds->adj_level ), "value_changed",
		GTK_SIGNAL_FUNC (level_scale_upd), data);

//	Main HBOX

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

	fds->gauss_options = gauss_opt_new(CONST_E,3.0,0.5);
	fds->current_filter = NULL;
	fds->filter_dialog = hbox;
	fds->filter_vbox = NULL;
	fds->gauss_dialog = NULL;
	fds->img_filter_dialog = NULL;
	fds->img_listbox = NULL;
	set_filter_defaults(fds);
	return fds;
}

filter_dialog_struct *filter_dialog_new(GtkWidget *window, GtkTooltips *tooltips, gpointer data) {

	GtkWidget *vbox;
	filter_dialog_struct *fds;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	fds =	filter_toolbar_new(window, tooltips, data);
	
	//	Embed the function which applies the filter

	fds->apply_filter = (gpointer) gener_hf;
	
	//	Embed in a vbox;  this allows adding optional widgets afterwards

	gtk_box_pack_start(GTK_BOX(vbox), fds->filter_dialog, FALSE, FALSE, 0);
	
	fds->filter_dialog = define_frame_with_hiding_arrows("Shape filter",vbox, window, TRUE);
	
	fds->filter_vbox = vbox;
	
	return fds;
}

void filter_dialog_free (filter_dialog_struct *fds) {
	if (fds) {
		if (fds->img_listbox)
			x_free(fds->img_listbox);
		if (fds->gauss_options)
			gauss_opt_free(fds->gauss_options);
		x_free(fds);
	}	
}

img_listbox_struct *img_listbox_new (GtkWidget *parent) {
	img_listbox_struct *lbs;
	lbs = (img_listbox_struct *) x_malloc(sizeof(img_listbox_struct), "img_listbox_struct");
	lbs->listbox = NULL;
	lbs->no_doc_label = NULL;
	lbs->parent = parent;
	return lbs;
}

void img_listbox_free (img_listbox_struct *lbs) {
	if (lbs)
		x_free(lbs);
}
