/* uniform.c - generates a white / grey / black uniform pixmap
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

void uniform_scale(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hf_type buf;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw)
		return;
	if ((*hfw->hf_options->uniform_value) == (hf_type) GTK_ADJUSTMENT(wdg)->value)
		return;
	buf = (hf_type) GTK_ADJUSTMENT(wdg)->value ;
	memcpy(hfw->hf_options->uniform_value, &buf, sizeof(hf_type)) ;
	hfw->hf_options->flag_uniform_done = FALSE;
	gener_hf(hfw);
}

GtkWidget *uniform_dialog_new(gpointer hfo_ptr, gint default_value) {
	GtkWidget *dialog, *wtmp, *vbox, *hbox;
	GtkObject *adj;
	hf_options_struct *hfo = (hf_options_struct *) hfo_ptr;

	if (!hfo)
		return gtk_label_new(_("No uniform dialog"));
	
	dialog = options_frame_new("Uniform");
	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(dialog),vbox);

	adj = gtk_adjustment_new (default_value, 0, MAX_HF_VALUE, 1, 1, 0.1);
    	wtmp = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_scale_set_digits (GTK_SCALE (wtmp), 0); 
	gtk_widget_show(GTK_WIDGET(wtmp));
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(uniform_scale), hfo->hfw_ptr_ptr);
	optimize_on_mouse_click (wtmp, hfo->hfw_ptr_ptr);
	gtk_box_pack_start(GTK_BOX(vbox), wtmp, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	uniform_percent_buttons_new(hbox, (gpointer) adj);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	return dialog;
}

