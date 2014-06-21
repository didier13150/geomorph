/* draw_hf_dialog.c:  dialogs and callbacks for drawing a crater in a hf
 *
 * Copyright (C) 2003 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         www.oricom.ca/patrice.st-gelais
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
#include "dialog_utilities.h"

#include "../icons/irregular_crater.xpm"
#include "../icons/peak_crater.xpm"
#include "../icons/standard_crater.xpm"

static void compute_preview (draw_crater_struct *dcs, 
	unsigned char *buf8, gint diameter, gint dummy) {
//	A function with a generic prototype for calculating a display buffer
//	Normally "diameter" == width and "dummy" == height
//	Here the image is square, the symmetry is radial, so width == height == diameter
	gint *sqrmap, i;
	sqrmap = get_sqrmap (dcs, diameter, 0xFF>>2);
	for (i=0; i<(diameter*diameter); i++)
		*(buf8+i) = (0xFF>>1) + *(sqrmap+i) ;
	x_free(sqrmap);
}

void standard_crater_callb (GtkWidget *button, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		return;
	((draw_crater_struct *) data)->type = STANDARD_CRATER;
	draw_area(((draw_crater_struct *) data)->preview);
}

void peak_crater_callb (GtkWidget *button, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		return;
	((draw_crater_struct *) data)->type = PEAK_CRATER;
	draw_area(((draw_crater_struct *) data)->preview);
}

void irregular_crater_callb (GtkWidget *button, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		return;
	((draw_crater_struct *) data)->type = IRREGULAR_CRATER;
	draw_area(((draw_crater_struct *) data)->preview);
}

void crater_boundaries_dialog (GtkWidget *vbox2, draw_crater_struct *crater) {

	//	Define the boundaries dialog in "vbox2"
	//	Used for defining the draw crater dialog
	//	and the crater generation dialog
	GtkWidget *hbox;
	GtkObject *adj;
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(vbox2),hbox);
	define_label_in_box("Boundary 1", hbox, FALSE, FALSE, DEF_PAD);
	adj = gtk_adjustment_new (crater->diam_bound1, 2, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &crater->diam_bound1);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(vbox2),hbox);
	define_label_in_box("Boundary 2", hbox, FALSE, FALSE, DEF_PAD);
	adj = gtk_adjustment_new (crater->diam_bound2, 2, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &crater->diam_bound2);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(vbox2),hbox);
	define_label_in_box("Distribution", hbox, FALSE, FALSE, DEF_PAD);
	adj = gtk_adjustment_new (crater->distribution, 1, 10, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &crater->distribution);
}

void crater_depth_n_noise_dialog (GtkWidget *dialog, draw_crater_struct *crater) {
	// Define in the "dialog" box the widgets for controlling the crater depth and noise
	// Common to the draw crater dialog and the crater generation dialog
	GtkWidget *frame, *hbox, *vbox, *label, *button, *wdg;
	GtkObject *adj;

//	Depth
	frame = frame_new("Depth (% of diameter)",DEF_PAD);
	gtk_box_pack_start(GTK_BOX(dialog),frame, TRUE, TRUE, DEF_PAD *0.5);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame),vbox);

	button = define_check_button_in_box ("Use default (1. 20%; 2. 12%; 3. 7%)", vbox, 0, 0, DEF_PAD*0.5);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
		crater->default_depth);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &crater->default_depth);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(vbox),hbox);

	adj = gtk_adjustment_new (crater->depth, 1, 50, 1, 1, 0.01);
	wdg = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &crater->depth);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(hide_if_true), wdg);
	if (crater->default_depth)
		gtk_widget_hide(GTK_WIDGET(wdg));

//	Noise

	frame = frame_new("Noise control",DEF_PAD);
	gtk_box_pack_start(GTK_BOX(dialog),frame, TRUE, TRUE, DEF_PAD *0.5);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame),vbox);

	label = define_label_in_box("Radial noise (%)", vbox, FALSE, FALSE, 0);

	// hbox for getting an horizonal scale... (because of the default behavior of define_scale_in_box)
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	adj = gtk_adjustment_new (crater->radial_noise, 0, 10, 1, 1, 0.01);
	wdg = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &crater->radial_noise);

	button = define_check_button_in_box ("Smooth terrain under crater", vbox, 0, 0, DEF_PAD);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), crater->if_smooth);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &crater->if_smooth);

	label = define_label_in_box("Surface noise to keep (pixels)", vbox, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	adj = gtk_adjustment_new (crater->surface_noise, 0, 20, 1, 1, 0.01);
	wdg = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &crater->surface_noise);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(show_if_true), label);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(show_if_true), wdg);
	if (!crater->if_smooth) {
		gtk_widget_hide(GTK_WIDGET(label));
		gtk_widget_hide(GTK_WIDGET(wdg));
	}
}

GtkWidget *draw_crater_dialog_new(GtkWidget *window, GtkTooltips *tooltips,
	draw_crater_struct *crater, gpointer data) {

	GtkWidget *dialog, *frame, *frame2, *toolbar, *hbox,
		*vbox, *vbox2, *wdg, *button;
	GtkObject *adj;

	init_radial_noise();

	dialog = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(dialog);

	frame = frame_new("Pen - crater",DEF_PAD);

	gtk_container_add(GTK_CONTAINER(frame),dialog);

//	Box containing the crater toolbar and a preview area
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	gtk_box_pack_start(GTK_BOX(dialog),hbox, TRUE, TRUE, 0);

//	Box containing the toolbar and its label
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	gtk_box_pack_start(GTK_BOX(hbox),vbox, TRUE, TRUE, 0);

	define_label_in_box("Type",vbox,FALSE,FALSE,DEF_PAD);

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar),GTK_ORIENTATION_VERTICAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
	gtk_widget_show(GTK_WIDGET(toolbar));
	wdg = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,_("Standard"),
		_("1. Standard crater (small ~ 7 km or -, depth 20%)"),NULL,
		create_widget_from_xpm(window,standard_crater_xpm),
		GTK_SIGNAL_FUNC(standard_crater_callb), (gpointer) crater);
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("With peak"),
		_("2. Crater with central peak (big ~ 15 km, depth 12%)"),NULL,
		create_widget_from_xpm(window,peak_crater_xpm),
		GTK_SIGNAL_FUNC(peak_crater_callb), (gpointer) crater);
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("Irregular"),
		_("3. Irregular crater (huge ~ 30 km or +, depth 7%)"),NULL,
		create_widget_from_xpm(window,irregular_crater_xpm),
		GTK_SIGNAL_FUNC(irregular_crater_callb), (gpointer) crater);

	gtk_box_pack_start(GTK_BOX(vbox),toolbar, TRUE, TRUE, DEF_PAD *0.5);

	crater->preview =
		view_struct_new(64,64, (gpointer) compute_preview,(gpointer) crater);

	gtk_box_pack_start(GTK_BOX(hbox),
		align_widget(crater->preview->area,0.5,0.5),
		TRUE, TRUE, DEF_PAD *0.5);

//	Diameter

	gtk_box_pack_start(GTK_BOX(dialog),tiling_control((gpointer) &crater->wrap),
			 TRUE, TRUE, DEF_PAD);

	frame2 = frame_new("Diameter (% of HF)",DEF_PAD);
	gtk_box_pack_start(GTK_BOX(dialog),frame2, TRUE, TRUE, DEF_PAD *0.5);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame2),vbox);

	button = define_check_button_in_box ("Random", vbox, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
		crater->random_diameter);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &crater->random_diameter);

	// A hbox passed to define_scale_in_box warrants an horizontal adjustment,
	// otherwise it would be vertical
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, DEF_PAD*0.5);

	adj = gtk_adjustment_new (crater->diameter, 2, 100, 1, 1, 0.01);
	wdg = define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &crater->diameter);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox2);
	gtk_box_pack_start(GTK_BOX(vbox),vbox2, TRUE, TRUE, DEF_PAD);

	define_label_in_box("Variation between boundaries 1 & 2", vbox2, FALSE, FALSE, 0);

	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(show_if_true), vbox2);
	gtk_signal_connect (GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(hide_if_true), wdg);
	if (crater->random_diameter)
		gtk_widget_hide(wdg);
	else
		gtk_widget_hide(vbox2);

	crater_boundaries_dialog(vbox2,crater);

	crater_depth_n_noise_dialog (dialog, crater);

	return frame;
}
