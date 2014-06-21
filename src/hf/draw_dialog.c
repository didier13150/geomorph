/* draw_dialog.c:  dialogs and callbacks for standard drawing (continuous stroke)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

typedef struct {
	void (*test_fn) (int a, int b);
} test_fn_struct;

#include "hf_wrapper.h"
#include "dialog_utilities.h"
#include "draw_dialog.h"
#include "draw_tools_dialog.h"
#include "../utils/menus_n_tools.h"

#include "../icons/sine_w.xpm"
// #include "../icons/sine_concave_w.xpm"
// #include "../icons/sine_conc_2_3_w.xpm"
#include "../icons/concave_w.xpm"
#include "../icons/triangle_w.xpm"
// #include "../icons/square_w.xpm"
#include "../icons/moyen10_2.xpm"

//	Private prototypes
static void sine_upd (GtkWidget *wdg, gpointer data) ;
static void concave_upd (GtkWidget *wdg, gpointer data) ;
static void triangle_upd (GtkWidget *wdg, gpointer data) ;
static void no_wave_shape_upd (GtkWidget *wdg, gpointer data);
// static void sine_concave_upd (GtkWidget *wdg, gpointer data) ;
// static void sine_concave_2_3_upd (GtkWidget *wdg, gpointer data) ;
// static void square_upd (GtkWidget *wdg, gpointer data) ;

//	NBTIPS = number of pen tips available

#define NBTIPS 4
command_item_struct tip_shapes[NBTIPS] = {
{ "Continuous stroke", "Gaussian", "Gaussian",
	0, (gchar **) moyen10_2_xpm, GDK_LEFT_PTR, no_wave_shape_upd,NULL, NULL,TRUE },
{ "Continuous stroke", "Sine", "Sine wave",
	0, (gchar **) sine_w_xpm, GDK_LEFT_PTR, sine_upd,NULL, NULL,FALSE },
{ "Continuous stroke", "Concave X*X", "Negative curvature wave X*X",
	0, (gchar **) concave_w_xpm, GDK_LEFT_PTR, concave_upd,NULL, NULL,FALSE },
// "Continuous stroke", "+Sine-Concave", "Growing wave = sine, decreasing = negative curvature X*X",
//	0, (gchar **) sine_concave_w_xpm, GDK_LEFT_PTR, sine_concave_upd,NULL, NULL,FALSE,
// "Continuous stroke", "+Sine 2/3, -Concave 1/3", "Growing wave = sine 2/3, decreasing = X*X 1/3",
//	0, (gchar **) sine_conc_2_3_w_xpm, GDK_LEFT_PTR, sine_concave_2_3_upd,NULL, NULL,FALSE,
{ "Continuous stroke", "Triangle", "Triangular wave",
	0, (gchar **) triangle_w_xpm, GDK_LEFT_PTR, triangle_upd,NULL, NULL,FALSE }
// "Continuous stroke", "Square", "Square wave",
// 	0, (gchar **) square_w_xpm, GDK_LEFT_PTR, square_upd,NULL, NULL,FALSE
};

static void compute_preview (pen_struct* pen,
	unsigned char *buf8, gint diameter, gint dummy) {

//	A function with a generic prototype for calculating the preview buffer
//	Normally "diameter" == width and "dummy" == height
//	Here the image is square, so width == height == diameter
//	When pen->map->square_symmetry is TRUE, the symmetry is radial,
//	only a quarter of the map is present in pen->map->data
//	Otherwise, the map is a complete grid of size diameter*diameter
	gint start, stop,x,y, sx, sy;
	gdouble value, ratio;
	static gdouble maxd = (gdouble) 0xFFFF;
	
	if (pen->spacing == SPACING_HIGH_QUALITY)
		ratio = maxd / LEVEL_HQ_SPACING;
	else
		if (pen->spacing == SPACING_LOW_QUALITY)
			ratio = maxd / LEVEL_LQ_SPACING ;
		else 
			if (pen->spacing == SPACING_VHIGH_QUALITY)
				ratio = maxd / LEVEL_VHQ_SPACING ;
			else	// SPACING_STANDARD_QUALITY
				ratio = maxd / LEVEL_SQ_SPACING;

	start =  (diameter/2) - pen->map->radius;
	stop = (diameter/2) + pen->map->radius -1;
	for (x=0; x<diameter; x++) {
		for (y=0; y<diameter; y++) {
			if ((x<start) || (x>=stop) || (y<start) || (y>=stop))
				*(buf8+VECTORIZE(x,y,diameter)) = 0;
			else {
				sx = x - start;
				sy = y - start;
				if (pen->map->square_symmetry) {
					if (sx>pen->map->radius)
						sx = 2*pen->map->radius - sx;
					if (sy>pen->map->radius)
						sy = 2*pen->map->radius - sy;
					value = ratio * 
						(gdouble) *(pen->map->data+VECTORIZE(sx,sy,pen->map->radius+1));
				}
				else
					value = ratio * 
						(gdouble) *(pen->map->data+VECTORIZE(sx,sy,2*pen->map->radius+1));
				*(buf8+VECTORIZE(x,y,diameter)) = ((hf_type) value)>>8;
			}
		}
	}
}

gboolean draw_line_n_refresh(gpointer hfw_ptr,
	gdouble from_x, gdouble from_y, gdouble *to_x, gdouble *to_y,
	gboolean draw_end) {

	// Draw a thick gaussian line and refresh the display

	gdouble tox,toy;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) hfw_ptr;

	tox = *to_x * pow(2.0,get_scale(hfw->sbs));
	toy = *to_y * pow(2.0,get_scale(hfw->sbs));

	if (!draw_continuous_line_by_dot (hfw->hf_struct,
		hfw->hf_options->draw->pen,
		from_x * pow(2.0,get_scale(hfw->sbs)),
		from_y * pow(2.0,get_scale(hfw->sbs)),
		&tox, &toy,
		draw_end,
		hfw->hf_options->gauss_list))

		return FALSE;

	*to_x = tox / pow(2.0,get_scale(hfw->sbs));
	*to_y = toy / pow(2.0,get_scale(hfw->sbs));

	draw_hf (hfw);
	return TRUE;
}

static void map_size_callb (GtkWidget *wdg, gpointer data) {
	// The actual pen size is 2 more than the size seen by the user
	// This lets room for edges with 0 value
	*((gint *) data) = 2 + (gint) GTK_ADJUSTMENT(wdg)->value;
}

static void update_map (GtkWidget *wdg, gpointer data) {
	pen_struct *pen;
	pen = (pen_struct *) data;
/*	if (pen->map) {
		// The map would be rebuilt just before drawing if its data is NULL
		// This avoids multiple allocations / deallocation when changing
		// map parameters with the scale widgets
		map_free(pen->map);
	}
*/
	// Immediate update is required for the preview area to be refreshed
	map_init( pen->map, pen->size, pen->level, pen->shape, pen->spacing) ;
// printf("UPDATE_MAP %d with SIZE: %d; LEVEL: %d; RADIUS: %d\n",pen->map,pen->size, pen->level, pen->map->radius);
}

static void draw_dts_area_callb (GtkWidget *wdg, gpointer data) {
	draw_tools_struct *dts;
	dts = (draw_tools_struct *) data;
	draw_area(dts->preview);
}

void set_spacing_vlow (GtkWidget *wdg, gpointer data) {
	pen_struct *pen;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	pen = (pen_struct *) data;
	if (!pen)
		return;
	pen->spacing = SPACING_VHIGH_QUALITY;
	// The height depends on the spacing, so we must reinit the map
	map_init( pen->map, pen->size, pen->level, pen->shape, pen->spacing) ;
}

void set_spacing_low (GtkWidget *wdg, gpointer data) {
	pen_struct *pen;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	pen = (pen_struct *) data;
	if (!pen)
		return;
	pen->spacing = SPACING_HIGH_QUALITY;
	// The height depends on the spacing, so we must reinit the map
	map_init( pen->map, pen->size, pen->level, pen->shape, pen->spacing) ;
}
void set_spacing_standard (GtkWidget *wdg, gpointer data) {
	pen_struct *pen;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	pen = (pen_struct *) data;
	if (!pen)
		return;
	pen->spacing = SPACING_STANDARD_QUALITY;
	map_init( pen->map, pen->size, pen->level, pen->shape, pen->spacing) ;
}
void set_spacing_high (GtkWidget *wdg, gpointer data) {
	pen_struct *pen;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	pen = (pen_struct *) data;
	if (!pen)
		return;
	pen->spacing = SPACING_LOW_QUALITY;
	map_init( pen->map, pen->size, pen->level, pen->shape, pen->spacing) ;
}

GtkWidget *spacing_control (gpointer data) {
	// Creates a row of radio buttons
	// for controlling the spacing (low spacing = high quality, smoother stroke)
	// "data" is a pen struct
	// SPACING_LOW_QUALITY (0.15), SPACING_STANDARD_QUALITY (0.1),
	// SPACING_HIGH_QUALITY (0.05), SPACING_VHIGH_QUALITY (0.05)

	GtkWidget *hbox;
	GSList *group = NULL;
	pen_struct *pen;
	pen = (pen_struct *) data;

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box ("Spacing",hbox,FALSE,FALSE,DEF_PAD*0.5);
	define_radio_button_in_box_with_data (hbox, &group, "++",
		set_spacing_high, data, (pen->spacing==SPACING_LOW_QUALITY)) ;
	define_radio_button_in_box_with_data (hbox, &group, "+",
		set_spacing_standard, data,  (pen->spacing==SPACING_STANDARD_QUALITY)) ;
	define_radio_button_in_box_with_data (hbox, &group, "-",
		set_spacing_low, data, (pen->spacing==SPACING_HIGH_QUALITY)) ;
	define_radio_button_in_box_with_data (hbox, &group, "--",
		set_spacing_vlow, data, (pen->spacing==SPACING_VHIGH_QUALITY)) ;
	return hbox;
}

void set_smoothing_none (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint *) data) = SMOOTHING_NONE;
}
void set_smoothing_auto (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint *) data) = SMOOTHING_AUTO;
}
void set_smoothing_all (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint *) data) = SMOOTHING_ALL;
}

GtkWidget *smoothing_control (gpointer data) {
	// Creates a row of radio buttons
	// for controlling the spacing (low spacing = high quality, smoother stroke)
	// "data" is a gint pointer, whose values can be:
	// SMOOTHING_NONE, SMOOTHING_ALL, SMOOTHING_AUTO
	GtkWidget *hbox;
	GSList *group = NULL;
	gint smoothing;
	smoothing = (gint) * (gint *) data;

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box ("Smoothing",hbox,FALSE,FALSE,DEF_PAD*0.5);
	define_radio_button_in_box_with_data (hbox, &group, "Never",
		set_smoothing_none, data, (smoothing==SMOOTHING_NONE)) ;
	define_radio_button_in_box_with_data (hbox, &group, "Auto",
		set_smoothing_auto, data,  (smoothing==SMOOTHING_AUTO)) ;
	define_radio_button_in_box_with_data (hbox, &group, "Always",
		set_smoothing_all, data, (smoothing==SMOOTHING_ALL)) ;
	return hbox;
}

static void update_tip (draw_tools_struct *dts) {
	if (!dts)
		return;
	map_init( dts->pen->map, dts->pen->size, dts->pen->level, dts->pen->shape, dts->pen->spacing) ;
	draw_area(dts->preview);
 }

static void no_wave_shape_upd (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	if (data)
		((draw_tools_struct*)data)->pen->shape = NO_WAVE_SHAPE;
	update_tip( (draw_tools_struct*)data);
}

static void sine_upd (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	if (data)
		((draw_tools_struct*)data)->pen->shape = SINE_WAVE;
	update_tip( (draw_tools_struct*)data);
}

static void concave_upd (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	if (data)
		((draw_tools_struct*)data)->pen->shape = CONCAVE_WAVE;
	update_tip( (draw_tools_struct*)data);
}
/*
static void sine_concave_upd (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	if (data)
		((draw_tools_struct*)data)->pen->shape = SINE_CONCAVE_WAVE;
	update_tip( (draw_tools_struct*)data);
}

static void sine_concave_2_3_upd (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	if (data)
		((draw_tools_struct*)data)->pen->shape = SINE_CONCAVE_2_3_WAVE;
	update_tip( (draw_tools_struct*)data);
}

static void square_upd (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	if (data)
		((draw_tools_struct*)data)->pen->shape = SQUARE_WAVE;
	update_tip( (draw_tools_struct*)data);
}

*/
static void triangle_upd (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	if (data)
		((draw_tools_struct*)data)->pen->shape  = TRIANGLE_WAVE;
	update_tip( (draw_tools_struct*)data);
}

GtkWidget *draw_dialog_new(GtkWidget *window, GtkTooltips *tooltips, 
	gpointer dts_ptr) {

	GtkWidget *frame, *vbox, *vbox2, *vbox3, *hbox, *hbox2, *frame2, *label, *tb;
	GtkObject *adj;
	GSList *group = NULL;
	pen_struct *pen;
	draw_tools_struct *dts = (draw_tools_struct *) dts_ptr;

	pen = dts->pen;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);

	frame = frame_new("Continuous stroke",DEF_PAD);

	gtk_container_add(GTK_CONTAINER(frame),vbox);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);
	
//	Preview - reordered 2007-03-12 - for a possible initialization problem with x64 systems
//	Function casting modified from (gpointer) to (void *) for x64 systems
	dts->preview =
		view_struct_new(MAX_PEN_PREVIEW, MAX_PEN_PREVIEW, (gpointer) compute_preview,(gpointer) pen);
	define_label_in_box("Size",hbox,FALSE, FALSE,0);
	adj = gtk_adjustment_new (pen->size-2, 2, 2*MAX_PEN_RADIUS-1, 2, 2, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (map_size_callb), (gpointer) &pen->size);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_map), (gpointer) pen);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (draw_dts_area_callb), (gpointer) dts);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);

	define_label_in_box("Level",hbox,FALSE, FALSE,0);
	define_label_in_box("%",hbox,FALSE, FALSE,DEF_PAD*0.5);
	adj = gtk_adjustment_new (pen->level, 1, 100, 1, 1, 0.01);
	define_scale_in_box(adj,hbox,0, DEF_PAD*0.5);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &pen->level);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (update_map), (gpointer) pen);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC (draw_dts_area_callb), (gpointer) dts);

//	Adding the effect control (RAISE / DIG / SMOOTH)

	gtk_box_pack_start(GTK_BOX(vbox),
		effect_toolbar(window, (gpointer) &pen->merge), FALSE, FALSE, DEF_PAD);

//	Pen tip
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_label_in_box ("TIP", hbox, FALSE, TRUE, DEF_PAD);
	tb = standard_toolbar_new(NBTIPS, tip_shapes,
		tooltips,
		window,
		(gpointer) dts,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), tb, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);

	gtk_box_pack_start(GTK_BOX(vbox),
		align_widget(dts->preview->area,0.5,0.5),
		FALSE, FALSE, DEF_PAD *0.5);

//	Tiling control
	gtk_box_pack_start(GTK_BOX(vbox),tiling_control((gpointer) &pen->wrap),
			 FALSE, FALSE, DEF_PAD);

//	Stroke crossing control

	hbox = gtk_hbox_new (FALSE,0);
	gtk_widget_show (GTK_WIDGET(hbox));
	define_label_in_box ("Stroke overlapping", hbox, FALSE, TRUE, DEF_PAD);
	define_radio_button_in_box_with_data (hbox, &group,"Adds up", (gpointer) gboolean_set_true, &dts->pen->overlap, dts->pen->overlap);
	define_radio_button_in_box_with_data (hbox, &group, "Intersects", (gpointer) gboolean_set_false, &dts->pen->overlap, !dts->pen->overlap);
	
	gtk_box_pack_start(GTK_BOX(vbox),hbox, FALSE, FALSE, DEF_PAD);
	
//	Speed vs Quality
	frame2 =  frame_new("Speed vs Quality",DEF_PAD);
	gtk_container_add(GTK_CONTAINER(vbox),frame2);

	vbox2 = gtk_vbox_new(FALSE, DEF_PAD*0.5);
	gtk_widget_show(vbox2);
	gtk_container_add(GTK_CONTAINER(frame2),vbox2);

	vbox3 = gtk_vbox_new(FALSE, 0);
//	gtk_widget_show(vbox3);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox3),hbox,FALSE,FALSE,0);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox2);

	label = gtk_label_new("<--  ");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox2), label, FALSE,FALSE,0);
	label = gtk_label_new(_("Speed"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox2), label, FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(hbox), align_widget(hbox2,0.0,0.5), TRUE, FALSE,0);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox2);

	label = gtk_label_new(_("Quality"));
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox2), label, FALSE,FALSE,0);
	label = gtk_label_new("  -->");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox2), label, FALSE, FALSE,0);

	gtk_box_pack_start(GTK_BOX(hbox), align_widget(hbox2,1.0,0.5), TRUE, FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox3),spacing_control((gpointer) pen),
		FALSE,FALSE,DEF_PAD*0.5);
//	gtk_box_pack_start(GTK_BOX(vbox3),smoothing_control((gpointer) &pen->smoothing), FALSE,FALSE,DEF_PAD*0.5);

	gtk_box_pack_start(GTK_BOX(vbox2),
		hideshow_dialog_new(window,GTK_ORIENTATION_HORIZONTAL,
		vbox3 , NULL),
	 	FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox2),vbox3,FALSE,FALSE,0);

	return frame;
}


