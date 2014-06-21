/* subdiv2.c - subdivision algorithm #2
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

static void refresh_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	hfw->if_calculated =  FALSE;
	if (hfw->hf_options->subdiv2_direct_upd)
		gener_hf(hfw);
}

static gint calculate_event_callb(GtkWidget *wdg,GdkEventButton *event, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw->hf_options->subdiv2_direct_upd)
		gener_hf(hfw);
	return FALSE;
}

/*static void calculate_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	gener_hf(hfw);
}*/

static void roughness_scale(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->subdiv2_options->roughness == 
		(gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	hfw->hf_options->subdiv2_options->roughness = 
		(gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated =  FALSE;
	gener_hf(hfw);
}

static void distribution_scale(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->subdiv2_options->distribution == 
		(gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	hfw->hf_options->subdiv2_options->distribution = 
		(gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated =  FALSE;
	gener_hf(hfw);
}

/*
static void topcenter_scale(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->subdiv2_options->top_value 
		== (hf_type) GTK_ADJUSTMENT(wdg)->value)
		return ;
	hfw->hf_options->subdiv2_options->top_value = (hf_type) GTK_ADJUSTMENT(wdg)->value ;
	hfw->if_calculated =  FALSE;
	gener_hf(hfw);
}

static void top_random(GtkWidget *button, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->subdiv2_options->top_random
		== (gboolean) GTK_TOGGLE_BUTTON (button)->active)
		return;
	hfw->hf_options->subdiv2_options->top_random = (gboolean) GTK_TOGGLE_BUTTON (button)->active;
	hfw->if_calculated =  FALSE;
	gener_hf(hfw);
}
*/

static gint change_subdiv2_seed(GtkWidget *entry, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if ( ! strlen( gtk_entry_get_text(GTK_ENTRY(entry))))
		return FALSE;
	if (hfw->hf_options->subdiv2_options->seed == 
		(unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry))))
		return FALSE; 
	hfw->hf_options->subdiv2_options->seed = 
		(unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry))); 
	hfw->if_calculated =  FALSE;
	gener_hf(hfw);
	return FALSE;
}

static void direct_upd_callb (GtkWidget *check_button, gpointer data) {
	hf_options_struct *hfo;
	hfo = (hf_options_struct *)data;
	if (GTK_TOGGLE_BUTTON(check_button)->active) {
		hfo->subdiv2_direct_upd = TRUE;
	}
	else {
		hfo->subdiv2_direct_upd = FALSE;
	}
}

/*****************************FREQUENCY CONTROL************************************/
//	Future enhancement:  check how to make a reusable class for this control
//	It is used in surfadd.c and for adding noise

static void percent_upd(GtkWidget *wdg, gpointer data) {
	gint *percent;
	percent = (gint *) data;
	(*percent) = (gint) GTK_ADJUSTMENT(wdg)->value;
}

void set_frq_vector(hf_wrapper_struct *hfw,gint *frq, gint nbrfrq) {
//	Sets the adjustment scales in frq_adj with the values in frq
//	There must be 12 adjustements - we don't test it
//	There could be any number of *frq, we simply reuse it from i=0
//	Begins with lower amplitudes, higher frequencies (wave length = 2)
//	Ends (i=11) with higher amplitudes, lower frequences (wave length = 4096)
	
	gint i;
	if (!nbrfrq)
		nbrfrq = 1;
	for (i=0; i<12; i++) {
		hfw->hf_options->subdiv2_options->frq_percent[i] = *(frq+i%nbrfrq);	
		gtk_adjustment_set_value( 
			GTK_ADJUSTMENT(hfw->hf_options->freq_subdiv2_adj[i]), *(frq+i%nbrfrq));
	}
}

static void set_freq_global(GtkWidget *button, gpointer data) {
	gint value;
	gboolean dir_upd;
	gchar *txt;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	dir_upd = hfw->hf_options->subdiv2_direct_upd;
	hfw->hf_options->subdiv2_direct_upd = FALSE;
	gtk_label_get(GTK_LABEL(GTK_BIN(button)->child), &txt);
	value = atoi(txt);
	set_frq_vector(hfw,&value,1);
	hfw->if_calculated =  FALSE;
	gener_hf(hfw);
	hfw->hf_options->subdiv2_direct_upd = dir_upd;
}

static GtkWidget * frq_dialog_new (gpointer hfo_ptr) {
//	Fine frequency control dialog
	GtkWidget *dialog, *table, *vbox, *hbox, *button, *scale; // *button2, 
	GtkObject *adj;
	hf_options_struct *hf_options = (hf_options_struct *) hfo_ptr;
	if (!hfo_ptr)
		return gtk_label_new(_("No frequency control"));
	gpointer data = hf_options->hfw_ptr_ptr;

//	dialog = frame_new("Wave length control",DEF_PAD*0.5);
	
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
		(hf_options->subdiv2_options->frq_percent[11], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,0,1,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[11]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[11] =adj;

// 2048
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[10], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,1,2,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[10]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[10]  = adj;
// 1024
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[9], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,2,3,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[9]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[9]  = adj;

// 512
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[8], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,3,4,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[8]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[8] =adj;
// 256
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[7], 0, 100, 1, 1, 0.01);
	define_scale_in_table(adj,table,1,2,4,5,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[7]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[7] =adj;
// 128
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[6], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,1,2,5,6,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[6]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[6]  =adj;

// 64
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[5], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,0,1,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[5]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[5] =adj;
// 32
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[4], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,1,2,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[4]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[4] =adj;
// 16
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[3], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,2,3,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[3]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[3] =adj;

// 8
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[2], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,3,4,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[2]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[2] = adj;
// 4
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[1], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,4,5,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[1]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[1] =adj;
// 2
	adj = gtk_adjustment_new 
		(hf_options->subdiv2_options->frq_percent[0], 0, 100, 1, 1, 0.01);
	scale = define_scale_in_table(adj,table,3,4,5,6,0,DEF_PAD*0);
	gl_preview_optimize (scale, data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (percent_upd), 
			&hf_options->subdiv2_options->frq_percent[0]);
	gtk_signal_connect (GTK_OBJECT (adj ), "value_changed",GTK_SIGNAL_FUNC (refresh_callb), data);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), data);
	hf_options->freq_subdiv2_adj[0]  = adj;

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
	gtk_widget_show(hbox);

	gtk_box_pack_start(GTK_BOX(hbox), align_widget(vbox,0.5,0.5), FALSE,FALSE, DEF_PAD*0.5);
	
	gtk_box_pack_start(GTK_BOX(hbox), table, FALSE,FALSE, 0);

	vbox = gtk_vbox_new(FALSE,0);
//	The default is to hide this dialog at the beginning, because it takes room
	gtk_widget_show(vbox);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE,FALSE, 0);

//	Frequency control

	button = define_check_button_in_box ("Auto update",vbox, FALSE,FALSE,0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	hf_options->subdiv2_direct_upd = TRUE;

	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(direct_upd_callb), (gpointer) hf_options);

	//	Small arrows for hiding / showing the dialog...
	
	dialog = define_frame_with_hiding_arrows ("Wave length control", vbox, hf_options->tools_window, FALSE);
	
	return dialog;
}

GtkWidget * subdiv2_dialog_new(gpointer hfo_ptr) {

	GtkObject *adj;
	GtkWidget *hbox, *vbox, *scale;
	hf_options_struct *hfo = (hf_options_struct *) hfo_ptr;

	if (!hfo)
		return gtk_label_new(_("No subdivision 2 dialog"));

	//	Main dialog box
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

	//	Seed
	
	gtk_box_pack_start_defaults(GTK_BOX(vbox), 
		seed_dialog_new(hfo->hfw_ptr_ptr, hfo->subdiv2_options->seed, 
		change_subdiv2_seed));

	//	Roughness
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box ("Roughness", hbox, 0, 0, DEF_PAD);
	adj = gtk_adjustment_new (0, -5, 5, 1, 1, 0.1);
	scale = define_scale_in_box (adj, hbox, 0, DEF_PAD*0.5);
	gl_preview_optimize(scale, hfo->hfw_ptr_ptr);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(roughness_scale), hfo->hfw_ptr_ptr);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), hfo->hfw_ptr_ptr);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	plus_minus_1_buttons (adj, hbox);
	
	//	Distribution
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box ("Distribution", hbox, 0, 0, DEF_PAD);
	adj = gtk_adjustment_new (hfo->subdiv2_options->distribution,
			 1, 20, 1, 1, 0.1);
	scale = define_scale_in_box (adj, hbox, 0, DEF_PAD*0.5);
	gl_preview_optimize(scale, hfo->hfw_ptr_ptr);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(distribution_scale), hfo->hfw_ptr_ptr);
	g_signal_connect (G_OBJECT (scale), "button_release_event", GTK_SIGNAL_FUNC (calculate_event_callb), hfo->hfw_ptr_ptr);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	plus_minus_1_buttons (adj, hbox);

//	Frequency dialog
	gtk_box_pack_start(GTK_BOX(vbox),
		frq_dialog_new(hfo_ptr), FALSE, FALSE,0);

	return define_options_frame_with_hiding_arrows("Subdiv2", vbox, hfo->tools_window, TRUE);
	
}




