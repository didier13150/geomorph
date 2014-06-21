/* surfadd_dialog.c - dialog for surface addition
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

#include <string.h>
#include "hf_wrapper.h"
#include "dialog_utilities.h"
// #include "hf_filters_dialog.h"

static gint change_surfadd_seed(GtkWidget *entry, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if ( ! strlen( gtk_entry_get_text(GTK_ENTRY(entry))) )
		return FALSE;
	if (hfw->hf_options->surfadd_options->seed == 
		(unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry))))
		return FALSE;
	hfw->hf_options->surfadd_options->seed = 
		(unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry))); 
// printf("hfw->hf_options->surfadd_options->seed: %d\n",hfw->hf_options->surfadd_options->seed);
	gener_hf(hfw);
	return FALSE;
}

static void xy_scale_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->surfadd_options->xy_random_range = 
			(gint) GTK_ADJUSTMENT(wdg)->value;
}

static void h_scale_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->surfadd_options->h_random_range = 
			(gint) GTK_ADJUSTMENT(wdg)->value;
}

static void slope_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->surfadd_options->slope = 
			(gint) GTK_ADJUSTMENT(wdg)->value;
}

static void density_upd(GtkWidget *wdg,gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->hf_options->surfadd_options->density = 
			(gint) GTK_ADJUSTMENT(wdg)->value;
}

static gint compute_hf_callb(GtkWidget *wdg,GdkEventButton *event, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	gener_hf(hfw);
	return FALSE;
}

static void percent_upd(GtkWidget *wdg, gpointer data) {
	gint *percent;
	percent = (gint *) data;
	(*percent) = (gint) GTK_ADJUSTMENT(wdg)->value;
}

static void set_freq_global(GtkWidget *button, gpointer data) {
	gint i, value;
	gchar *txt;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	gtk_label_get(GTK_LABEL(GTK_BIN(button)->child), &txt);
	value = atoi(txt);
	for (i=0; i<12; i++) {
		hfw->hf_options->surfadd_options->frq_percent[i] = value;
		gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(hfw->hf_options->freq_control_adj[i]), value);
	}
	gener_hf(hfw);
}

static GtkWidget * surfadd_frq_dialog_new (gpointer hfo_ptr) {
//	Fine frequency control dialog
	GtkWidget *table, *vbox, *hbox, *button, *scale;
	GtkObject *adj;
	hf_options_struct *hf_options = (hf_options_struct *) hfo_ptr;
	if (!hfo_ptr)
		return gtk_label_new(_("No frequency control"));
	gpointer data = hf_options->hfw_ptr_ptr;

	table = gtk_table_new(6, 4, FALSE);
	gtk_widget_show(table);

	define_label_in_table("4096",table, 0, 1, 0, 1, 0);
 	define_label_in_table("2048",table, 0, 1, 1, 2, 0); 
	define_label_in_table("1024",table, 0, 1, 2, 3, 0);
 	define_label_in_table(" 512",table, 0, 1, 3, 4, 0); 
	define_label_in_table(" 256",table, 0, 1, 4, 5, 0);
 	define_label_in_table(" 128",table, 0, 1, 5, 6, 0);
 	define_label_in_table(" 64",table, 2, 3, 0, 1, 0);
 	define_label_in_table(" 32",table, 2, 3, 1, 2, 0); 
	define_label_in_table(" 16",table, 2, 3, 2, 3, 0);
 	define_label_in_table("  8",table, 2, 3, 3, 4, 0); 
	define_label_in_table("  4",table, 2, 3, 4, 5, 0);
 	define_label_in_table("  2",table, 2, 3, 5, 6, 0);
// 4096
	adj= gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[11], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,0,1,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[11]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[11] =adj;

// 2048
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[10], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,1,2,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[10]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[10]  = adj;
// 1024
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[9], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,2,3,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[9]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[9]  = adj;

// 512
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[8], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,3,4,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[8]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[8] =adj;
// 256
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[7], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,4,5,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[7]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[7] =adj;
// 128
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[6], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,5,6,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[6]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[6]  =adj;

// 64
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[5], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,0,1,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[5]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[5] =adj;
// 32
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[4], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,1,2,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[4]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[4] =adj;
// 16
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[3], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,2,3,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[3]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[3] =adj;

// 8
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[2], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,3,4,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[2]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[2] = adj;
// 4
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[1], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,4,5,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[1]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[1] =adj;
// 2
	adj = gtk_adjustment_new 
		(hf_options->surfadd_options->frq_percent[0], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,5,6,0,DEF_PAD*0);
	// gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->surfadd_options->frq_percent[0]);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);
	hf_options->freq_control_adj[0]  = adj;

//	Adding fast reset buttons
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	button = gtk_button_new_with_label ("0");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) set_freq_global, data);
	gtk_widget_show(button);
  	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("25");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) set_freq_global, data);
	gtk_widget_show(button);
  	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("50");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) set_freq_global, data);
	gtk_widget_show(button);
  	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("75");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) set_freq_global, data);
	gtk_widget_show(button);
  	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("100");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) set_freq_global, data);
	gtk_widget_show(button);
  	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,0);

//	The default is to hide this dialog at the beginning, because it takes room
//	gtk_widget_show(hbox);

	gtk_box_pack_start(GTK_BOX(hbox), align_widget(vbox,0.5,0.5), FALSE,FALSE, DEF_PAD*0.5);
	
	gtk_box_pack_start(GTK_BOX(hbox), table, FALSE,FALSE, 0);

	return define_frame_with_hiding_arrows("Wave length control", hbox, hf_options->tools_window, FALSE);
}

GtkWidget * surfadd_dialog_new(gpointer hfo_ptr) {

	GtkObject *adj;
	GtkWidget *table, *hbox, *vbox, *scale, *scrolled_window; // *button, 
	GtkWidget *frame;
	static gboolean dummy_flag;

	gpointer data;
	hf_options_struct *hfo = (hf_options_struct *) hfo_ptr;

	if (!hfo)
		return gtk_label_new(_("No surface addition dialog"));

	data = hfo->hfw_ptr_ptr;

//	Main dialog box
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

//	2008-02 Now with scrolled window
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (scrolled_window);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled_window), vbox);

	gtk_widget_set_size_request (scrolled_window, -1, gdk_screen_get_height(gdk_screen_get_default())/3);

//	Seed
	
	gtk_box_pack_start_defaults(GTK_BOX(vbox), 
		seed_dialog_new(data, hfo->surfadd_options->seed, 
		change_surfadd_seed));

	table = gtk_table_new(4, 2, FALSE);
	gtk_widget_show(table);

//	Slope / roughness (kind of)
//	From 0 to 100%
//	100: we keep the same H for each step, even if the surfaces to add are smaller
//	50:  we divide the H by 2 for each step (default), because  the surfaces are divided by two
//	100:  we divide the H by 3 for each step.

	define_label_in_table("Slope",table, 0, 1, 0, 1, 0); 

	adj = gtk_adjustment_new (hfo->surfadd_options->slope, 0, 100, 1, 1, 0.01);

	scale = define_scale_in_table(adj,table,1,2,0,1,0,DEF_PAD*0.5);
	g_signal_connect (G_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (slope_upd), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);

//	Random x-y displacement scale
//	From 0 to 100 % (100 % = twice the size of the current surface to add)

	define_label_in_table("XZ Displacement ",table, 0, 1, 1, 2, 0); 

	adj = gtk_adjustment_new 
		(hfo->surfadd_options->xy_random_range, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,1,2,0,DEF_PAD*0.5);
	g_signal_connect (G_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (xy_scale_upd), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);

//	Random height displacement scale
//	From 0 to 100 % (100 % = twice the height of the current surface to add)

	define_label_in_table("H Displacement ",table, 0, 1, 2, 3, 0); 
	adj = gtk_adjustment_new 
		(hfo->surfadd_options->h_random_range, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,2,3,0,DEF_PAD*0.5);
	g_signal_connect (G_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (h_scale_upd), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);

//	Lateral density variation
//	From 0 to 100 % (100 % = twice the default density)

	define_label_in_table("Density",table, 0, 1, 3, 4, 0); 
	adj = gtk_adjustment_new 
		(hfo->surfadd_options->density, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,3,4,0,DEF_PAD*0.5);
	g_signal_connect (G_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (density_upd), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (compute_hf_callb), data);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	gtk_box_pack_start_defaults(GTK_BOX(hbox), table);

	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

//	Filters for the primitive surface and preview
	frame = frame_new("Primitive surface", DEF_PAD*0.5);
	gtk_container_add(GTK_CONTAINER(vbox), frame);

//	HBOX will contain the shape filter dialog and the preview
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	
	gtk_container_add(GTK_CONTAINER(frame), hbox);

//	We create the primitive surface, and the preview area at the same time
	hfo->primit_surf = hf_wrapper_new(NULL,NULL,hbox, hbox, 
		hf_options_dialog_new(hfo->tools_window,
			hbox, 
			hfo->tooltips, 
			NULL, 
			&hfo->primit_surf,
			hfo->stack,
			hfo->doc_type,
			hfo->all_docs,
			hfo->get_doc_list_from_type),
		&dummy_flag, &dummy_flag, NULL, NULL, NULL, NULL, HFPRIMITIVE);
//	Shared data:
	((hf_wrapper_struct *)hfo->primit_surf)->hf_options->dist_matrix =
			hfo->dist_matrix;

	((hf_wrapper_struct *)hfo->primit_surf)->hf_options->current_calculation = (gpointer) uniform;
	((hf_wrapper_struct *)hfo->primit_surf)->hf_options->current_callb = NULL;
	((hf_wrapper_struct *)hfo->primit_surf)->hf_options->current_options = (gpointer) hfo->uniform_value;

	set_filter_type (((hf_wrapper_struct *) hfo->primit_surf)->hf_options->creation_fd_struct, MEDIUM_FILTER_1);
	
	set_dependent_process ((hf_wrapper_struct *) hfo->primit_surf, gener_hf_from_ptr_ptr, hfo->hfw_ptr_ptr);

//	Set the merge operation to MULTIPLY:

	set_filter_merge(((hf_wrapper_struct *) hfo->primit_surf)->hf_options->creation_fd_struct, 1);

//	Frequency dialog
	gtk_box_pack_start(GTK_BOX(vbox),
		surfadd_frq_dialog_new(hfo_ptr), FALSE, FALSE,0);

	return define_options_frame_with_hiding_arrows("sum of surfaces", scrolled_window, hfo->tools_window, TRUE);
}

