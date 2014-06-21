/* swirl_dialog.c:  dialogs and callbacks for swirling
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include <gtk/gtk.h>
#include "../utils/menus_n_tools.h"
#include "globals.h"
#include "swirl.h"

GtkWidget *swirl_dialog_new(GtkWidget *window, GtkTooltips *tooltips, 
	swirl_struct *pen, gpointer data) {

	GtkWidget *frame, *vbox, *hbox;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);

	frame = frame_new("Continuous stroke",DEF_PAD);

	gtk_container_add(GTK_CONTAINER(frame),vbox);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);
	define_label_in_box("Not yet available",hbox,FALSE, FALSE,0);
/*
	define_label_in_box("Size",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->size, 3, 255, 2, 2, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->size);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);

	define_label_in_box("Angle",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->angle, 0, 360, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->angle);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, DEF_PAD*0.5);
	rotate_buttons_new(hbox, (gpointer) adj);
*/
	return frame;
}

