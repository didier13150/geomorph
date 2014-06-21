/* draw_hf_dialog.c:  dialogs and callbacks for drawing with a hf in a hf
 *
 * Copyright (C) 2001, 2008 Patrice St-Gelais
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

#include "hf_wrapper.h"
#include "hf_calc.h"
#include "draw_hf_dialog.h"
#include "draw_tools_dialog.h"
#include "dialog_utilities.h"


static void compute_preview (draw_hf_struct* pen,
	unsigned char *buf8, gint diameter, gint dummy) {

//	A function with a generic prototype for calculating the preview buffer
//	Normally "diameter" == width and "dummy" == height
//	Here the image is square, so width == height == diameter

	gint start, stop,x,y, sx, sy, radius;
	gdouble value, ratio;
	static gdouble maxd = (gdouble) 0xFFFF;
	
//	Displayed pen tip density set at an arbitrary "believable" level
	ratio = (((gdouble) (pen->level * (125-pen->spacing))) / 10000.0) + 0.5 * log(((gdouble) (pen->level * (125-pen->spacing))))/log(10000.0);
	radius = pen->size>>1;
	start =  (diameter/2) - radius;
	stop = (diameter/2) + radius - 1;
	for (x=0; x<diameter; x++) {
		for (y=0; y<diameter; y++) {
			if ((x<start) || (x>=stop) || (y<start) || (y>=stop))
				*(buf8+VECTORIZE(x,y,diameter)) = 0;
			else {
				sx = x - start;
				sy = y - start;
				value = MIN(maxd,ratio * (gdouble) *(pen->scaled_hf[0] + VECTORIZE(sx,sy,pen->size)));
				*(buf8+VECTORIZE(x,y,diameter)) = ((hf_type) value)>>8;
			}
		}
	}
}

static void map_size_callb (GtkWidget *wdg, gpointer data) {
	// The actual pen size is 2 more than the size seen by the user
	// This lets room for edges with 0 value
	*((gint *) data) = 2 + (gint) GTK_ADJUSTMENT(wdg)->value;
}

static void draw_dts_area_callb (GtkWidget *wdg, gpointer data) {
	draw_tools_struct *dts;
	dts = (draw_tools_struct *) data;
	draw_area(dts->hf_preview);
}

void update_map_callb (GtkWidget *wdg, gpointer data) {
	update_hf_map( (draw_hf_struct *) data) ;
}

GtkWidget *draw_hf_dialog_new(GtkWidget *window, GtkTooltips *tooltips, 
	gpointer dts_ptr) {

	GtkWidget *frame, *hbox, *hbox2, *vbox, *button, *wdg;
	GtkObject *adj;
	draw_tools_struct *dts = (draw_tools_struct *) dts_ptr;
	draw_hf_struct *pen = dts->hf_pen;
	// Modified 2008-09: standardised on Subdiv2

// printf("DRAW_HF_DIALOG_NEW\n");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);

	frame = frame_new("Fractal Pen",DEF_PAD);

	gtk_container_add(GTK_CONTAINER(frame),vbox);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(GTK_WIDGET(hbox));
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);


	define_label_in_box("Size",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->size-2, 8, 2*MAX_PEN_RADIUS-1, 2, 2, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (map_size_callb), (gpointer) &pen->size);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_map_callb), (gpointer) pen);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (draw_dts_area_callb), (gpointer) dts);

//	Pen level

//	level is a number from 0 to 100
//	it is supposed to translate in a ratio from 0 to 1 with this rule:
//	ratio = (level*level)/10 000 (quadratic variation),
//	so that a 50 level means a 25% ratio.

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	define_label_in_box("Level",hbox,FALSE, FALSE,0);
	define_label_in_box("%",hbox,FALSE, FALSE,DEF_PAD*0.5);
	adj = gtk_adjustment_new (pen->level, 0, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->level);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_map_callb), (gpointer) pen);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (draw_dts_area_callb), (gpointer) dts);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD*0.5);


//	Pen roughness (relief)

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	define_label_in_box("Roughness",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->roughness, -5, 5, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->roughness);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_map_callb), (gpointer) pen);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (draw_dts_area_callb), (gpointer) dts);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD*0.5);

//	Adding the effect control (RAISE / DIG / SMOOTH)

	gtk_box_pack_start(GTK_BOX(vbox),
		effect_toolbar(window, (gpointer) &pen->merge), FALSE, FALSE, DEF_PAD);

//	Spacing control

	hbox2 = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox2));

	gtk_box_pack_start(GTK_BOX(vbox),align_widget(hbox2,0.5,0.5), FALSE, FALSE, 0);

	define_label_in_box("Spacing", hbox2, FALSE, TRUE, 0);
	define_label_in_box(" ", hbox2, FALSE, TRUE, 0);
	button = define_check_button_in_box (" ", hbox2, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
		pen->control_spacing);

	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &pen->control_spacing);

	define_label_in_box("%", hbox2, FALSE, TRUE, 0);
	adj = gtk_adjustment_new (pen->spacing, 5, 100, 1, 1, 0.01);
	wdg = define_scale_in_box(adj,hbox2,0, 0);

	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->spacing);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(sensitive_if_true), wdg);

	if (!pen->control_spacing)
		gtk_widget_set_sensitive(GTK_WIDGET(wdg),FALSE);

//	The preview pixmap...
	dts->hf_preview =
		view_struct_new(MAX_PEN_PREVIEW, MAX_PEN_PREVIEW, (gpointer) compute_preview, (gpointer) pen);

	gtk_box_pack_start(GTK_BOX(vbox),
		align_widget(dts->hf_preview->area,0.5,0.5),
		FALSE, FALSE, DEF_PAD *0.5);

//	Tiling - wrapping control

	gtk_box_pack_start(GTK_BOX(vbox),tiling_control((gpointer) &pen->wrap),
			 TRUE, TRUE, DEF_PAD);

	return frame;
}
