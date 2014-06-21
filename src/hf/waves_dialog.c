/* waves_dialog.c:  dialog for waves deformation filters
 *
 * Copyright (C) 2002 Patrice St-Gelais
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

#include "img_process_dialog.h"
#include "hf_wrapper.h"
#include "waves_dialog.h"
#include "dialog_utilities.h"
#include "../icons/sine_w.xpm"
#include "../icons/sine_concave_w.xpm"
#include "../icons/concave_w.xpm"
#include "../icons/horizontal.xpm"
#include "../icons/vertical.xpm"
#include "../icons/sine_conc_2_3_w.xpm"
#include "../icons/triangle_w.xpm"
#include "../icons/square_w.xpm"
// #include "../icons/circular_w.xpm"

//	Private prototypes
static void sine_upd (GtkWidget *wdg, gpointer data) ;
static void concave_upd (GtkWidget *wdg, gpointer data) ;
static void sine_concave_upd (GtkWidget *wdg, gpointer data) ;
static void sine_concave_2_3_upd (GtkWidget *wdg, gpointer data) ;
static void triangle_upd (GtkWidget *wdg, gpointer data) ;
static void square_upd (GtkWidget *wdg, gpointer data) ;

static void vertical_upd (GtkWidget *wdg, gpointer data) ;
static void horizontal_upd (GtkWidget *wdg, gpointer data) ;
// static void circular_upd (GtkWidget *wdg, gpointer data) ;

//	NBWAVES #defined in waves.h
command_item_struct w_shapes[NBWAVES] = {
{ "Waves", "Sine", "Sine wave",
	0, (gchar **) sine_w_xpm, GDK_LEFT_PTR, sine_upd,NULL, NULL,TRUE },
{ "Waves", "Concave X*X", "Negative curvature wave X*X",
	0, (gchar **) concave_w_xpm, GDK_LEFT_PTR, concave_upd,NULL, NULL,FALSE },
{ "Waves", "+Sine-Concave", "Growing wave = sine, decreasing = negative curvature X*X",
	0, (gchar **) sine_concave_w_xpm, GDK_LEFT_PTR, sine_concave_upd,NULL, NULL,FALSE },
{ "Waves", "+Sine 2/3, -Concave 1/3", "Growing wave = sine 2/3, decreasing = X*X 1/3",
	0, (gchar **) sine_conc_2_3_w_xpm, GDK_LEFT_PTR, sine_concave_2_3_upd,NULL, NULL,FALSE },
{ "Waves", "Triangle", "Triangular wave",
	0, (gchar **) triangle_w_xpm, GDK_LEFT_PTR, triangle_upd,NULL, NULL,FALSE },
{ "Waves", "Square", "Square wave",
	0, (gchar **) square_w_xpm, GDK_LEFT_PTR, square_upd,NULL, NULL,FALSE }
};

//	NBAXIS #defined in waves.h
command_item_struct w_axis[NBAXIS] = {
{ "Deformation axis", "Vertical", "Vertical",
	0, (gchar **) vertical_xpm, GDK_LEFT_PTR, vertical_upd,NULL, NULL,TRUE },
{ "Deformation axis", "Horizontal", "Horizontal",
	0, (gchar **) horizontal_xpm, GDK_LEFT_PTR, horizontal_upd,NULL, NULL,FALSE }
// { "Deformation axis", "Circular", "Circular",
//	0, (gchar **) circular_w_xpm, GDK_LEFT_PTR, circular_upd,NULL, NULL,FALSE }
};

void wave_calc(hf_wrapper_struct *hfw) {
//	Convenience function, recalculates and displays the HF
//	--> Eventually do a test to calculate only if the "direct update' checkbox is checked
	if (hfw->if_calculated)
		return;
	waves_apply(hfw->hf_struct,
		hfw->hf_options->img->wav_shapes,
		hfw->hf_options->img->nb_wav,
		hfw->hf_options->img->wav_data);
	begin_pending_record(hfw,"Waves",accept_fn,reset_fn);
	if (hfw->hf_options->img->wav_accept)
		gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->wav_accept),TRUE);
	hfw->if_calculated = TRUE;
	draw_hf(hfw);
}

void wave_upd(GtkWidget *button, gpointer data, gint wave_shape) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->shape) == wave_shape)
		return ;
//	Update the field
	ws->shape = wave_shape;
	hfw->if_calculated = FALSE;
	wave_calc(hfw);
}

static void sine_upd (GtkWidget *wdg, gpointer data) {
	wave_upd (wdg,data,SINE_WAVE);
}
static void concave_upd (GtkWidget *wdg, gpointer data) {
	wave_upd (wdg,data,CONCAVE_WAVE);
}
static void sine_concave_upd (GtkWidget *wdg, gpointer data) {
	wave_upd (wdg,data,SINE_CONCAVE_WAVE);
}
static void sine_concave_2_3_upd (GtkWidget *wdg, gpointer data) {
	wave_upd (wdg,data,SINE_CONCAVE_2_3_WAVE);
}
static void triangle_upd (GtkWidget *wdg, gpointer data) {
	wave_upd (wdg,data, TRIANGLE_WAVE);
}
static void square_upd (GtkWidget *wdg, gpointer data) {
	wave_upd (wdg,data, SQUARE_WAVE);
}

void axis_upd(GtkWidget *button, gpointer data, gint axis) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->axis) == axis)
		return ;
//	Update the field
	ws->axis = axis;
	hfw->if_calculated = FALSE;
//	Recalculate and display the HF
	wave_calc(hfw);
}

static void vertical_upd (GtkWidget *wdg, gpointer data) {
	axis_upd (wdg,data,VERTICAL_WAVE);
}
static void horizontal_upd (GtkWidget *wdg, gpointer data) {
	axis_upd (wdg,data, HORIZONTAL_WAVE);
}
/*
static void circular_upd (GtkWidget *wdg, gpointer data) {
	axis_upd (wdg,data, CIRCULAR_WAVE);
}
*/

void order_upd (GtkWidget *wdg, gpointer data) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->order) == (gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	ws->order = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;	
	wave_calc(hfw);
}

void angle_upd (GtkWidget *wdg, gpointer data) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->angle) == (gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	ws->angle = (gint) GTK_ADJUSTMENT(wdg)->value;	
	hfw->if_calculated = FALSE;
	wave_calc(hfw);
}

void period_upd (GtkWidget *wdg, gpointer data) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->period) == (gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	ws->period = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	wave_calc(hfw);
}

void amplitude_upd (GtkWidget *wdg, gpointer data) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw->hf_options->img->wav_to_calc)
		return;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->amplitude) == (gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	ws->amplitude = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	wave_calc(hfw);
}

void randomness_upd (GtkWidget *wdg, gpointer data) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->randomness) == (gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	ws->randomness = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	wave_calc(hfw);
}

void phase_upd (GtkWidget *wdg, gpointer data) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);
//	No change...
	if ((ws->phase) == (gint) GTK_ADJUSTMENT(wdg)->value)
		return ;
	ws->phase = (gint) GTK_ADJUSTMENT(wdg)->value;
	hfw->if_calculated = FALSE;
	wave_calc(hfw);
}

void set_axis(GtkWidget *tb, gint axis) {
//	Sets the axis toolbar to "axis" value
//	Important:  order-dependent! 
//	Don't mess the toolbar structure without modifying this function!
	GtkToolbarChild *child;
	switch (axis) {
		case VERTICAL_WAVE:
			child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(tb)->children,0);
			break;
		case HORIZONTAL_WAVE:
			child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(tb)->children,1);
			break;
		case CIRCULAR_WAVE:
			child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(tb)->children,2);
			break;
		default:		// VERTICAL_WAVE
			child = (GtkToolbarChild *) g_list_nth_data(GTK_TOOLBAR(tb)->children,0);
	} 
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON( child->widget), TRUE);
}

static gint change_wave_seed(GtkWidget *entry, gpointer data) {
	gint current_page;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
//	Find the current notebook page
	current_page = gtk_notebook_get_current_page 
			(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data,current_page);

// printf("ORDER: %d; SHAPE: %d; AXIS: %d; ANGLE: %d; PERIOD: %d; AMPLITUDE: %d; PHASE: %d; RANDOMNESS: %d; SEED: %d\n",
// ws->order, ws->shape, ws->axis, ws->angle, ws->period, ws->amplitude, ws->phase, ws->randomness, ws->seed);

	ws->seed = (unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry))); 

	hfw->if_calculated = FALSE;
	wave_calc(hfw);
	return FALSE;
}

void wave_page_new(	img_dialog_struct *img,
				gint axis,
				GtkWidget *tools_window,
				GtkTooltips *tooltips,
				gpointer data) {

	//	Creating a new wave page dialog in "img->wav_notebook"
	GtkWidget *vbox, *hbox, *table;
	wave_page_struct *page;
	wave_struct *ws;
	gchar *buf;
	toolbar_struct *tb;
	if (img->nb_wav==99) {
		my_msg(_("Maximum page reached (99)"),WARNING);
		return;
	}

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

	page = (wave_page_struct *) x_malloc(sizeof(wave_page_struct), "wave_page_struct");
	img->nb_wav++;
	img->wav_pages = g_list_append(img->wav_pages,(gpointer) page);
	ws = wave_new(SINE_WAVE, VERTICAL_WAVE, img->nb_wav);
	img->wav_data = g_list_append(img->wav_data,ws);
	page->notebook_page = vbox;

	buf = (gchar *) malloc(5);
	sprintf(buf,"%d",img->nb_wav);
	gtk_notebook_append_page (GTK_NOTEBOOK(img->wav_notebook), 
		vbox, gtk_label_new(buf));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(img->wav_notebook), 
		gtk_notebook_page_num(GTK_NOTEBOOK(img->wav_notebook),
			page->notebook_page));

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_label_in_box ("Order", hbox, FALSE, TRUE, DEF_PAD);
 	page->adj_order = gtk_adjustment_new (img->nb_wav, 0.0, 100.0, 1.0, 5.0, 0.0);
	gtk_signal_connect (GTK_OBJECT (page->adj_order), "value_changed",
		GTK_SIGNAL_FUNC (order_upd), data);
	page->order_spinner =   gtk_spin_button_new (GTK_ADJUSTMENT(page->adj_order), 0, 0);
	gtk_widget_show(page->order_spinner);
 	gtk_box_pack_start (GTK_BOX (hbox), page->order_spinner, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, DEF_PAD);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_label_in_box ("Shape", hbox, FALSE, TRUE, DEF_PAD);
	tb = toolbar_new(NBWAVES, w_shapes, 
		tooltips,
		tools_window,
		data,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), tb->toolbarwdg, FALSE, FALSE, 0);
	page->shape_toolbar = tb->toolbarwdg;
	x_free(tb);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, DEF_PAD);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_label_in_box ("Deformation axis", hbox, FALSE, TRUE, DEF_PAD);
	tb = toolbar_new(NBAXIS, w_axis, 
		tooltips,
		tools_window,
		data,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), tb->toolbarwdg, FALSE, FALSE, 0);
	page->axis_toolbar = tb->toolbarwdg;
	x_free(tb);

	set_axis(page->axis_toolbar, axis);

	gtk_box_pack_start_defaults (GTK_BOX (vbox), hbox);

	gtk_box_pack_start(GTK_BOX(vbox), 
		seed_dialog_new(data, ws->seed,
		change_wave_seed),FALSE,TRUE,DEF_PAD);

	table = gtk_table_new(2, 6, FALSE);
	gtk_widget_show(GTK_WIDGET(table));

	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, TRUE, DEF_PAD);

	define_label_in_table("Amplitude", table, 0,1, 0, 1, DEF_PAD);
 	page->adj_amplitude = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 1.0, 0.0);
	page->amplitude_slider = define_scale_in_table(page->adj_amplitude, table, 1,2,0,1, 0, DEF_PAD);
	optimize_on_mouse_click (page->amplitude_slider, data);
	gtk_signal_connect (GTK_OBJECT (page->adj_amplitude), "value_changed",
		GTK_SIGNAL_FUNC (amplitude_upd), data);

	define_label_in_table("Random var.", table, 0,1, 1, 2, DEF_PAD);
 	page->adj_randomness = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 1.0, 0.0);
	page->randomness_slider = define_scale_in_table(page->adj_randomness, table, 1,2,1,2, 0, DEF_PAD);
	optimize_on_mouse_click (page->randomness_slider, data);
	gtk_signal_connect (GTK_OBJECT (page->adj_randomness), "value_changed",
		GTK_SIGNAL_FUNC (randomness_upd), data);

	define_label_in_table("Period", table, 0, 1, 2, 3, DEF_PAD);
 	page->adj_period = gtk_adjustment_new (5, 1.0, 10.0, 1.0, 1.0, 0.0);
	page->period_slider = define_scale_in_table(page->adj_period, table, 1,2,2,3, 0, DEF_PAD);
	optimize_on_mouse_click (page->period_slider, data);
	gtk_signal_connect (GTK_OBJECT (page->adj_period), "value_changed",
		GTK_SIGNAL_FUNC (period_upd), data);

	define_label_in_table("Phase", table, 0,1,3,4,DEF_PAD);
 	page->adj_phase = gtk_adjustment_new (0, 0.0, 100.0, 1.0, 1.0, 0.0);
	page->phase_slider = define_scale_in_table(page->adj_phase, table, 1,2,3,4, 0, DEF_PAD);
	optimize_on_mouse_click (page->phase_slider, data);
	gtk_signal_connect (GTK_OBJECT (page->adj_phase), "value_changed",
		GTK_SIGNAL_FUNC (phase_upd), data);

	define_label_in_table("Angle", table, 0, 1, 4, 5, DEF_PAD);
 	page->adj_angle = gtk_adjustment_new (0, 0.0, 360.0, 1.0, 1.0, 0.0);
	page->angle_slider = define_scale_in_table(page->adj_angle, table, 1, 2, 4, 5, 0, DEF_PAD);
	optimize_on_mouse_click (page->angle_slider, data);
	gtk_signal_connect (GTK_OBJECT (page->adj_angle), "value_changed",
		GTK_SIGNAL_FUNC (angle_upd), data);
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	rotate_buttons_new(hbox, (gpointer) page->adj_angle);
	gtk_table_attach (GTK_TABLE (table), align_widget(hbox, 0.5, 0.5), 1, 2, 5, 6, 
		GTK_FILL, GTK_FILL, 0, 0);
}

void add_new_wave(GtkWidget *button, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	wave_page_new(	hfw->hf_options->img, 
				VERTICAL_WAVE, 
				hfw->hf_options->tools_window, 
				hfw->hf_options->tooltips,
				data);
}

void wave_page_free (wave_page_struct *wps) {
	if (!wps)
		return;
	x_free(wps);
}

/* Remove a page from the notebook */
//... inspired from the Gtk notebook example
void delete_current_wave( GtkButton   *button, gpointer data)
{
    	gint page;
	wave_page_struct *wp;
	wave_struct *ws;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	
    	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook));
    	gtk_notebook_remove_page (hfw->hf_options->img->wav_notebook, page);

	wp = (wave_page_struct *) g_list_nth_data(hfw->hf_options->img->wav_pages,page);
	hfw->hf_options->img->wav_pages = 
		g_list_remove(hfw->hf_options->img->wav_pages, (gpointer) wp);
	hfw->hf_options->img->nb_wav--;
	wave_page_free(wp);

	ws = (wave_struct *) g_list_nth_data(hfw->hf_options->img->wav_data, page);
	hfw->hf_options->img->wav_data =
		g_list_remove(hfw->hf_options->img->wav_data, (gpointer) ws);
	wave_free(ws);

    	/* Need to refresh the widget --
     	This forces the widget to redraw itself. */
    	gtk_widget_draw(GTK_WIDGET(hfw->hf_options->img->wav_notebook), NULL);
	wave_calc(hfw);
}

void set_wave_defaults(img_dialog_struct *img) {
	//	Sets all the amplitudes to 0%	
	GList *node, *node_data;
	wave_struct *ws;
	node_data = img->wav_data;
	img->wav_to_calc = FALSE;
	for (node = img->wav_pages; node; node=node->next) {
	//	Got some trouble with signals ordering here, 
	//	must bring the amplitude to 0 before the adjustment scale, 
	//	otherwise the amplitude for the 1st page is not modified
		ws = (wave_struct *) node_data->data;
		ws->amplitude=0;
		gtk_adjustment_set_value( 
		   GTK_ADJUSTMENT(((wave_page_struct *) (node->data))->adj_amplitude),0);
		node_data = node_data->next;
	}
	gtk_notebook_set_page(GTK_NOTEBOOK(img->wav_notebook),0);
	img->wav_to_calc = TRUE;
}

GtkWidget * wave_dialog_new(gpointer data) {

	GtkWidget *notebook,*button, *hbox, *vbox;
	GtkWidget *dialog;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;

	dialog = options_frame_new("Waves");
   
	//	Main dialog box
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));
	gtk_container_add( GTK_CONTAINER(dialog), vbox);

	/* Create a new notebook, place the position of the tabs */
 	notebook = gtk_notebook_new ();
    	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_box_pack_start_defaults(GTK_BOX(vbox),notebook);
    	gtk_widget_show(notebook);
 
	hfw->hf_options->img->wav_notebook = GTK_NOTEBOOK(notebook);

	//	Add the 2 default pages, 1 for vertical wave, other for lateral
	wave_page_new( hfw->hf_options->img, VERTICAL_WAVE,
			hfw->hf_options->tools_window, hfw->hf_options->tooltips, data);

	wave_page_new( hfw->hf_options->img, HORIZONTAL_WAVE,
			hfw->hf_options->tools_window, hfw->hf_options->tooltips, data);

	gtk_notebook_set_page(GTK_NOTEBOOK(hfw->hf_options->img->wav_notebook),0);

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	button = gtk_button_new_with_label(_("New"));
	gtk_widget_show(GTK_WIDGET(button));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) add_new_wave, data);
	gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label(_("Destroy"));
	gtk_widget_show(GTK_WIDGET(button));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) delete_current_wave, data);
	gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(hbox,0.5,0.5), FALSE, FALSE, DEF_PAD);

	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &hfw->hf_options->img->wav_accept), FALSE, FALSE, DEF_PAD);

	return dialog;
}

void waves_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	if (!hfw->hf_options->img->wav_dialog) {
		hfw->hf_options->img->wav_dialog = wave_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			hfw->hf_options->img->wav_dialog );
	//	This callback makes sure that the transformations are not reapplied
	//	when we come back in this dialog (I don't know why "set_defaults..." alone
	//	doesn't do the job - some notebook pages are still active - 2002.04.17)
	//	Probably some strange things are happening with the signals order.
		gtk_signal_connect(GTK_OBJECT (hfw->hf_options->img->wav_dialog ), "show",
			GTK_SIGNAL_FUNC (reset_callb), data);
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = hfw->hf_options->img->wav_dialog;
//	set_wave_defaults(hfw->hf_options->img);
	hfw->hf_options->img->set_fn = (gpointer) set_wave_defaults;
	hfw->hf_options->img->accept_wdg = hfw->hf_options->img->wav_accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}
