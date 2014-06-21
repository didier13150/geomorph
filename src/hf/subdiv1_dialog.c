/* subdiv1.c - subdivision algorithm #1:  cross / diamond
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

void roughness_scale(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	printf("ROUGHNESS: %d\n",(gint) GTK_ADJUSTMENT(wdg)->value);
	if (hfw->hf_options->subdiv1_options->roughness == 
		(gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	hfw->hf_options->subdiv1_options->roughness = 
		(gint) GTK_ADJUSTMENT(wdg)->value;
	gener_hf(hfw);
}

void topcenter_scale(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->subdiv1_options->top_value 
		== (hf_type) GTK_ADJUSTMENT(wdg)->value)
		return ;
	hfw->hf_options->subdiv1_options->top_value = (hf_type) GTK_ADJUSTMENT(wdg)->value ;
	gener_hf(hfw);
}

void top_random(GtkWidget *button, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->hf_options->subdiv1_options->top_random
		== (gboolean) GTK_TOGGLE_BUTTON (button)->active)
		return;
	hfw->hf_options->subdiv1_options->top_random = (gboolean) GTK_TOGGLE_BUTTON (button)->active;
	gener_hf(hfw);
}

gint change_subdiv1_seed(GtkWidget *entry, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	unsigned int s;
	// Avoid double display... the callback seems always to be called twice,
	// the first time with an empty entry, I don't know why
	// (maybe because "gtk_entry_set_text" blanks the field before writing in it??)
	if (! strlen( gtk_entry_get_text(GTK_ENTRY(entry))) )
		return FALSE;
	s = (unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry)));
//	printf("Change_subdiv1_seed: %d; lng: %d\n", s, strlen( gtk_entry_get_text(GTK_ENTRY(entry))));
	if (hfw->hf_options->subdiv1_options->seed == s)
		return FALSE; 
	hfw->hf_options->subdiv1_options->seed = s;
	gener_hf(hfw);
	return FALSE;
}

GtkWidget * subdiv1_dialog_new(gpointer hfo_ptr) {

	GtkObject *tmp,*topval;
	GtkWidget *wtmp, *button, *topvallbl, *hbox, *vbox, *scale;
	gpointer data;
	hf_options_struct *hfo = (hf_options_struct *) hfo_ptr;

	if (!hfo)
		return gtk_label_new(_("No subdivision 1 dialog"));

	data = hfo->hfw_ptr_ptr;

	//	Main dialog box
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

	//	Seed
	
	gtk_box_pack_start_defaults(GTK_BOX(vbox), 
		seed_dialog_new(data, hfo->subdiv1_options->seed, 
		change_subdiv1_seed));

	//	Roughness
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box ("Roughness", hbox, 0, 0, DEF_PAD);
	tmp = gtk_adjustment_new (0, -4, 4, 1, 1, 0.1);
	scale = define_scale_in_box (tmp, hbox, 0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (tmp), "value_changed",
		GTK_SIGNAL_FUNC(roughness_scale), data);
	gl_preview_optimize(scale, data);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	plus_minus_1_buttons (tmp, hbox);

	//	Flag:  TRUE if top center is random
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Random center", hbox, 0, 0, DEF_PAD);

	button = gtk_check_button_new ();
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(top_random), data);

//	topvallbl = define_label_in_box ("Center height", hbox, FALSE, FALSE, 0);
	topvallbl = define_label_in_box ("H", hbox, FALSE, FALSE, 0);

	//	Top center value

	topval = gtk_adjustment_new (60000, 0, 65535, 1, 1, 0.1);
	gtk_signal_connect (GTK_OBJECT (topval), "value_changed",
		GTK_SIGNAL_FUNC(topcenter_scale), data);
	wtmp = define_scale_in_box (topval, hbox, 0, DEF_PAD*0.5);

	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	//	We disable the scale when the top center is random

	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(unsensitive_if_true), topvallbl);

	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(unsensitive_if_true), wtmp);
		
	gl_preview_optimize(wtmp, data);
	
//	Defaults
	unsensitive_if_true(button, wtmp);
	unsensitive_if_true(button, topvallbl);
	
	return define_options_frame_with_hiding_arrows("Subdiv1", vbox, hfo->tools_window, TRUE);
}

