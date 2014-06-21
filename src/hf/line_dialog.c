/* line_dialog.c:  dialogs and callbacks for generating a polyline
 *
 * Copyright (C) 2004 Patrice St-Gelais
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

#include "line_dialog.h"
#include "draw_tools_dialog.h"
#include "dialog_utilities.h"
#include <gtk/gtk.h>
#include "../utils/menus_n_tools.h"
#include "globals.h"
#include "view_area.h"
#include "hf_wrapper.h"

#include "../icons/vertical.xpm"
#include "../icons/horizontal.xpm"

gint update_line (GtkWidget *wdg, gpointer data) {
	line_dialog_struct *l;
	l = (line_dialog_struct *) data;
	if (!l)
		return FALSE;
	// Draw the line in the middle of the preview buffer
	init_fractal_polyline (l->f, l->line_preview->width, l->line_preview->height,
		0,  l->line_preview->height>>1,
		 l->line_preview->width-1,  l->line_preview->height>>1);
	draw_area (l->line_preview);
	if (l->update_external_data && l->external_data)
		(*l->update_external_data) (NULL, l->external_data);
	return FALSE;
}

static gint change_line_seed(GtkWidget *entry, gpointer data) {
	line_dialog_struct *l;
	l = (line_dialog_struct *) data;
	if (!l)
		return FALSE;
	l->f->seed =(gint) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry)));
//	printf("SEED: %d\n",l->f->seed);
	// 0 seed not allowed (workaround for an annoying double update,
	// one with 0 seed, the second with the right seed - 2005.03.14)
	if (l->f->seed)
		update_line (NULL,data);
	return FALSE;
}

static void compute_preview (fractal_line_struct *f,
	unsigned char *buf8, gint width, gint height) {

//	A function with a generic prototype for calculating the preview buffer

	unsigned char value = 0;
	gint i;

	for (i=0; i<(width*height); i++) {
		*(buf8+i) = 0xFF;
	}

	set_line_scale (f->polyline, (gdouble) width-1, (gdouble) height-1);

	draw_n_transform_all_segments (f->polyline,
		0, 0,
		0, height>>1,
		(gpointer) buf8,
		width, height,
		(gpointer) &value,
		UNSIGNED_CHAR_ID,
		OVERFLOW_ZERO);
}

void line_dialog_free (line_dialog_struct *lds) {
	if (lds) {
		if (lds->line_preview)
			view_struct_free (lds->line_preview);
		// We don't free lds->f because this is a pointer copy, freed elsewhere
		x_free(lds);
	}
}

line_dialog_struct *line_dialog_new (GtkWidget *window, GtkTooltips *tooltips,
	gchar *title, fractal_line_struct *fl,
	gint preview_width, gint preview_height,
	gpointer external_data,
	gint (*update_external_data) (GtkWidget *, gpointer),
	gint (*on_press) (GtkWidget *widget, GdkEventButton *event, gpointer data),
	gint (*on_release) (GtkWidget *widget, GdkEventButton *event, gpointer data)) {

	GtkWidget *hbox, *vbox, *wdg, *button, *scale; // *label, 
	GtkObject *adj;
	line_dialog_struct *lds;

	lds = (line_dialog_struct*) x_malloc(sizeof(line_dialog_struct), "line_dialog_struct");

	lds->f = fl;

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);
	
	lds->dialog = define_frame_with_hiding_arrows(title, vbox, window, TRUE);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);

//	Sudvidision steps

	define_label_in_box("Subdivision steps",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (fl->steps, 0, 10, 1, 1, 0.01);
	scale = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	if (on_press)
		gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) on_press, external_data);
	if (on_release)
		gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) on_release, external_data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &fl->steps);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_line), (gpointer) lds);

//	Random x-y variations

	define_label_in_box("Random variation of subdivision points",vbox,FALSE, FALSE,0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	wdg = create_widget_from_xpm(window,horizontal_xpm);
	gtk_box_pack_start(GTK_BOX(hbox), wdg, FALSE, FALSE, DEF_PAD*0.5);
	adj = gtk_adjustment_new (fl->random_x, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	if (on_press)
		gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) on_press, external_data);
	if (on_release)
		gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) on_release, external_data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &fl->random_x);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_line), (gpointer) lds);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);

	wdg = create_widget_from_xpm(window,vertical_xpm);
	gtk_box_pack_start(GTK_BOX(hbox), wdg, FALSE, FALSE, DEF_PAD*0.5);
	adj = gtk_adjustment_new (fl->random_y, 0, 100, 1, 1, 0.01);
	scale = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	if (on_press)
		gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) on_press, external_data);
	if (on_release)
		gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) on_release, external_data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &fl->random_y);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_line), (gpointer) lds);

//	Distribution

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, DEF_PAD*0.5);

	define_label_in_box("Distribution",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (fl->distribution,0.0, 5.0, 0.01, 0.01, 0.01);
	scale = define_scale_in_box(adj,hbox,2, DEF_PAD*0.5);
	if (on_press)
		gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) on_press, external_data);
	if (on_release)
		gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) on_release, external_data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gdouble_adj_callb), (gpointer) &fl->distribution);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_line), (gpointer) lds);

//	Cracks

	button = define_check_button_in_box ("Cracks", vbox, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), fl->if_cracks);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &fl->if_cracks);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(update_line), (gpointer) lds);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	//label = 
	define_label_in_box("Length", hbox, FALSE, FALSE, 0);
	adj = gtk_adjustment_new (fl->cracks_depth, 0, 100, 1, 1, 0.01);
	wdg = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	if (on_press)
		gtk_signal_connect (GTK_OBJECT (wdg), "button_press_event",
			(GtkSignalFunc) on_press, external_data);
	if (on_release)
		gtk_signal_connect (GTK_OBJECT (wdg), "button_release_event",
			(GtkSignalFunc) on_release, external_data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &fl->cracks_depth);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_line), (gpointer) lds);

	//label = 
	define_label_in_box("Width", hbox, FALSE, FALSE, 0);
	adj = gtk_adjustment_new (fl->cracks_width, 1, 4, 1, 1, 0.01);
	wdg = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	if (on_press)
		gtk_signal_connect (GTK_OBJECT (wdg), "button_press_event",
			(GtkSignalFunc) on_press, external_data);
	if (on_release)
		gtk_signal_connect (GTK_OBJECT (wdg), "button_release_event",
			(GtkSignalFunc) on_release, external_data);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &fl->cracks_width);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_line), (gpointer) lds);

	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(show_if_true), hbox);
	if (!fl->if_cracks) {
		gtk_widget_hide(GTK_WIDGET(hbox));
	}

//	Seed

	lds->seed_dialog = seed_dialog_new( (gpointer) lds, fl->seed, change_line_seed);
	gtk_box_pack_start(GTK_BOX(vbox), lds->seed_dialog,
		TRUE, TRUE, DEF_PAD);

//	Preview

	lds->line_preview =
		view_struct_new(preview_width,preview_height,(gpointer) compute_preview,(gpointer) fl);
	gtk_box_pack_start(GTK_BOX(vbox),
		align_widget(lds->line_preview->area,0.5,0.5),
		TRUE, TRUE, DEF_PAD *0.5);

	// Compute the line at least once, without executing the external callback
	// (the data structures could be not ready for that)

	lds->external_data = NULL;
	lds->update_external_data = NULL;
	update_line (NULL, (gpointer) lds);

	lds->external_data = external_data;
	lds->update_external_data = update_external_data;

	return lds;
}
