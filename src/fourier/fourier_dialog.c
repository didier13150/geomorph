/* fourier_dialog.c - The Geomorph Fourier Explorer dialog
 *
 * Copyright (C) 2005, 2006, 2007 Patrice St-Gelais
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
#include "fourier_dialog.h"
#include "../hf/hf.h"
#include "../hf/hf_calc.h"
#include "../hf/dialog_utilities.h"
#include "../hf/render.h"
#include "../utils/scale_buttons.h"
#include "../utils/menus_n_tools.h"

#include "../icons/reuse_result.xpm"

#define NBFFILTERS 5
#include "../icons/blank.xpm"
#include "../icons/fft_block_filter1r.xpm"
#include "../icons/fft_block_filter2r.xpm"
#include "../icons/fft_block_filter3r.xpm"
#include "../icons/fft_block_filter4r.xpm"

static void no_filter_callb (GtkWidget *wdg, gpointer fds_ptr);
static void block_filter_callb (GtkWidget *wdg, gpointer fds_ptr);
static void frame_filter_callb (GtkWidget *wdg, gpointer fds_ptr);
static void empty_box_filter_callb (GtkWidget *wdg, gpointer fds_ptr);
static void ditch_filter_callb (GtkWidget *wdg, gpointer fds_ptr);

command_item_struct fourier_filters[NBFFILTERS] = {
{"Type", "No filter", "No filter",0, (gchar **) blank_xpm, GDK_LEFT_PTR, no_filter_callb, NULL, NULL,TRUE},
{"Type", "Block", "Block",0, (gchar **) fft_block_filter1r_xpm, GDK_LEFT_PTR, block_filter_callb,NULL, NULL,FALSE},
{"Type", "Frame", "Frame",0, (gchar **) fft_block_filter2r_xpm, GDK_LEFT_PTR, frame_filter_callb, NULL, NULL,FALSE},
{"Type", "Empty Box", "Empty Box",0, (gchar **) fft_block_filter3r_xpm, GDK_LEFT_PTR, empty_box_filter_callb, NULL, NULL,FALSE},
{"Type", "Ditch", "Ditch",0, (gchar **) fft_block_filter4r_xpm, GDK_LEFT_PTR, ditch_filter_callb, NULL, NULL,FALSE}
};

static void draw_frequencies (fourier_dialog_struct *fds) {
		switch (fds->fs->current_view) {
			case REAL:
				draw_area(fds->freal_view);
//				printf("Draw REAL\n");
				break;
			case IMAGINARY:
				draw_area(fds->fimaginary_view);
//				printf("Draw IMAGINARY\n");
				break;
			default: // MODULE	
				draw_area(fds->fmodule_view);
//				printf("Draw MODULE\n");
		}
}

static void draw_inverse (fourier_dialog_struct *fds) {
		fourier_apply_filter (fds->fs);
		fourier_compute_inverse (fds->fs);
		draw_area(fds->finv_view);
		gl_area_update (fds->gl_preview);
}

static void draw_fourier (fourier_dialog_struct *fds) {
	draw_frequencies (fds);
	draw_inverse (fds);
}

static gint draw_inverse_callb (GtkWidget *wdg, GdkEventButton *event, gpointer data) {
	draw_inverse ((fourier_dialog_struct *) data);
	return FALSE;
}

static void draw_inverse_callb2 (GtkWidget *wdg, gpointer data) {
	draw_inverse ((fourier_dialog_struct *) data);
}

static void no_filter_callb (GtkWidget *wdg, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return; // "Untoggling" the button
	fds->fs->ffs->filter_type = NO_FOURIER_FILTER;
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_x_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_y_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->band_width_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->amplitude_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_x_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_y_wdg),FALSE);
	instantiate_filter (fds->fs->ffs, fds->fs->max_x, fds->fs->max_y, fds->fs->mask);
	draw_fourier (fds);
}

static void block_filter_callb (GtkWidget *wdg, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return; // "Untoggling" the button
	fds->fs->ffs->filter_type = F_BOX;
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_y_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->band_width_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->amplitude_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_y_wdg),TRUE);
	instantiate_filter (fds->fs->ffs, fds->fs->max_x, fds->fs->max_y, fds->fs->mask);
	draw_fourier (fds);
}

static void frame_filter_callb (GtkWidget *wdg, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return; // "Untoggling" the button
	fds->fs->ffs->filter_type = F_BOX_INVERTED;
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_y_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->band_width_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->amplitude_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_y_wdg),TRUE);
	instantiate_filter (fds->fs->ffs, fds->fs->max_x, fds->fs->max_y, fds->fs->mask);
	draw_fourier (fds);
}

static void empty_box_filter_callb (GtkWidget *wdg, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return; // "Untoggling" the button
	fds->fs->ffs->filter_type = F_EMPTY_BOX;
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_y_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->band_width_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->amplitude_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_y_wdg),TRUE);
	instantiate_filter (fds->fs->ffs, fds->fs->max_x, fds->fs->max_y, fds->fs->mask);
	draw_fourier (fds);
}

static void ditch_filter_callb (GtkWidget *wdg, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return; // "Untoggling" the button
	fds->fs->ffs->filter_type = F_EMPTY_BOX_INVERTED;
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->size_y_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->band_width_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->amplitude_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_x_wdg),TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(fds->ffds->translate_y_wdg),TRUE);
	instantiate_filter (fds->fs->ffs, fds->fs->max_x, fds->fs->max_y, fds->fs->mask);
	draw_fourier (fds);
}

static void compute_fourier_grid (fourier_dialog_struct* fds,
	unsigned char *buf8, gint width, gint height, gint fourier_type) {

// Outputs one of the fourier grid in the buf8 buffer
// width x height = size of the buf8 buffer
// --> assumed to be square and a power of 2
// fourier_type: MODULE, REAL, IMAGINARY

	static pix8_rgb mask_value = {0xFF, 0x0F, 0x0F};
	//gint i;
	if (!fds->fs->tmp_buf)
		return;
	convert_complex (fds->fs, (gpointer) fds->fs->tmp_buf, HF_TYPE_ID, fourier_type, F_INPUT_BUFFER) ;
	pow2_scale_uchar_grid_with_mask (fds->fs->tmp_buf, buf8, fds->fs->max_x, fds->fs->max_y,   log2i(width) - log2i(fds->fs->max_x), HF_TYPE_ID, fds->fs->mask, (gpointer) &mask_value);

}

static void compute_fmodule (fourier_dialog_struct* fds,
	unsigned char *buf8, gint width, gint height) {
	compute_fourier_grid (fds, buf8, width, height, MODULE);
}

static void compute_fimaginary (fourier_dialog_struct* fds,
	unsigned char *buf8, gint width, gint height) {
	compute_fourier_grid (fds, buf8, width, height, IMAGINARY);
}

static void compute_freal (fourier_dialog_struct* fds,
	unsigned char *buf8, gint width, gint height) {
	compute_fourier_grid (fds, buf8, width, height, REAL);
}

static void compute_finv (fourier_struct* fs,
	unsigned char *buf8, gint width, gint height) {
// Outputs the fourier inverse transform in the buf8 buffer
// width x height = size of the buf8 buffer
// --> assumed to be square and a power of 2

//	gint i;
	if (!fs->tmp_buf_inv)
		return;
	convert_finv (fs, (gpointer) fs->tmp_buf_inv, HF_TYPE_ID) ;
//	for (i=0; i<fs->max_x*fs->max_y; i++)
//		*(buf8+i) = *(((unsigned char *)fs->tmp_buf_inv)+1+i*2);
	pow2_scale_grid (fs->tmp_buf_inv, buf8, fs->max_x, fs->max_y, log2i(width) - log2i(fs->max_x), HF_TYPE_ID, UNSIGNED_CHAR_ID);

}

static void change_scale_callb (GtkWidget *wdg, gpointer fds_ptr) {

	gint display_x, display_y;
	gdouble hpos, vpos, ratio_x, ratio_y;
	GtkAdjustment *vadj, *hadj;
	GtkRequisition rqv, rqh; //rq;
	fourier_dialog_struct *fds;
	fds = (fourier_dialog_struct *) fds_ptr;
	if (!fds)
		return;
	if (!fds->fs)
		return;
	if ((!fds->sbs) || (!fds->freal_view) || (!fds->fmodule_view) ||(!fds->fimaginary_view) )
		return;
	display_x = RIGHT_SHIFT(fds->max_x,fds->sbs->current);
	display_y = RIGHT_SHIFT(fds->max_y,fds->sbs->current);

	// We scale the actual position of the scrolled window
	// This calls synchronizing callbacks for other notebook tabs
	vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(fds->freal_scrwin));
	hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(fds->freal_scrwin));
	hpos = gtk_adjustment_get_value (GTK_ADJUSTMENT(hadj));
	vpos = gtk_adjustment_get_value (GTK_ADJUSTMENT(vadj));
	ratio_x = ((gdouble) display_x) / (gdouble) fds->freal_view->width;
	ratio_y = ((gdouble) display_y) /  (gdouble) fds->freal_view->height;

	resize_area (fds->freal_view, display_x, display_y);
	resize_area (fds->fimaginary_view, display_x, display_y);
	resize_area (fds->fmodule_view, display_x, display_y);

//	gtk_widget_size_request(GTK_WIDGET(fds->freal_scrwin),&rq);

	gtk_widget_size_request(GTK_WIDGET(gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(fds->freal_scrwin))),&rqh);
	gtk_widget_size_request(GTK_WIDGET(gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(fds->freal_scrwin))),&rqv);

//	gtk_adjustment_set_value (GTK_ADJUSTMENT(hadj), ratio_x * hpos + (((gdouble) (rq.width - rqv.width)) / 2.0) * (ratio_x - 1.0) );
//	gtk_adjustment_set_value (GTK_ADJUSTMENT(vadj), ratio_y * vpos + (((gdouble) (rq.height - rqh.height)) / 2.0) * (ratio_y - 1.0)  );
	gtk_adjustment_set_value (GTK_ADJUSTMENT(hadj), ratio_x * hpos + (((gdouble) (GTK_WIDGET(fds->freal_scrwin)->allocation.width - rqv.width)) / 2.0) * (ratio_x - 1.0) );
	gtk_adjustment_set_value (GTK_ADJUSTMENT(vadj), ratio_y * vpos + (((gdouble) (GTK_WIDGET(fds->freal_scrwin)->allocation.height - rqh.height)) / 2.0) * (ratio_y - 1.0)  );
	
}

fourier_dialog_struct *fourier_dialog_new ( ) {
	fourier_dialog_struct *fds;
	fds = (fourier_dialog_struct *) x_calloc(1,sizeof(fourier_dialog_struct), "fourier_dialog_struct");
	fds->fs = fourier_struct_new();
	fds->notebook = NULL;
	fds->window = NULL;
	fds->fmodule_view = NULL ;
	fds->freal_view = NULL ;
	fds->fimaginary_view = NULL ;
	fds->fmodule_scrwin = NULL ;
	fds->freal_scrwin = NULL ;
	fds->fimaginary_scrwin = NULL ;
	fds->finv_view = view_struct_new (FOURIER_VIEW_SIZE, FOURIER_VIEW_SIZE,
		(gpointer) compute_finv, (gpointer) fds->fs);
	fds->real_tab = 0;
	fds->imaginary_tab = 0;
	fds->module_tab = 0;
	fds->sbs = scale_buttons_new_with_callback (GTK_ORIENTATION_HORIZONTAL, SCALE_ID_50, SCALE_ID_800, (gpointer) change_scale_callb, (gpointer) fds);
	set_current_button (fds->sbs, SCALE_ID_200);
	fds->ffds = NULL;
	// mouse_on_scale controls if we forward the delete event
	// on the window, triggered when clicking in the scale
	fds->mouse_on_scale = FALSE;
	fds->post_processing = NULL;
	fds->post_processing_data = NULL;
	fds->preview_choice = PREVIEW_RESULT;
	fds->render_str = render_struct_new();

	return fds;	
}

void fourier_dialog_free (fourier_dialog_struct *fds) {

	if (!fds)
		return;

	if (fds->fmodule_view)
		view_struct_free(fds->fmodule_view);
	if (fds->finv_view)
		view_struct_free(fds->finv_view);
	if (fds->fimaginary_view)
		view_struct_free(fds->fimaginary_view);
	if (fds->freal_view)
		view_struct_free(fds->freal_view);
	if (fds->fs)
		fourier_struct_free(fds->fs);
	if (fds->sbs)
		scale_buttons_free(fds->sbs);
	if (fds->render_str)
		x_free(fds->render_str);
	x_free(fds);
}

static gint update_filter (GtkWidget *wdg, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
//	printf("UPDATE_FILTER\n");
	instantiate_filter (fds->fs->ffs, fds->fs->max_x, fds->fs->max_y, fds->fs->mask);
	draw_frequencies (fds);
	// mouse_on_scale controls if we forward the delete event
	// on the window, triggered when clicking in the scale
	fds->mouse_on_scale = FALSE;
	return FALSE;
}

gint synchro_depth_callb (GtkWidget *adj, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	if (fds->fs->ffs->xy_synchro)
		gtk_adjustment_set_value (GTK_ADJUSTMENT(fds->ffds->size_y_adj), gtk_adjustment_get_value (GTK_ADJUSTMENT(adj)));
	return FALSE;
}
 
gint synchro_width_callb (GtkWidget *adj, gpointer fds_ptr) {
	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	if (fds->fs->ffs->xy_synchro)
		gtk_adjustment_set_value (GTK_ADJUSTMENT(fds->ffds->size_x_adj), gtk_adjustment_get_value (GTK_ADJUSTMENT(adj)));
	return FALSE;
}

fourier_filter_dialog_struct *fourier_filter_dialog_new (fourier_dialog_struct *fds) {

	GtkWidget *main_box, *hbox, *tb, *scale, *table, *button;
	GtkObject *adj;
	fourier_filter_dialog_struct *ffds;
	fourier_struct *fs = fds->fs;

	ffds = (fourier_filter_dialog_struct *) x_malloc(sizeof(fourier_filter_dialog_struct), "fourier_filter_dialog_struct");

	main_box = gtk_vbox_new(FALSE,0);
	gtk_widget_show (main_box);
	ffds->filter_box = main_box;

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);
	gtk_box_pack_start( GTK_BOX(main_box),hbox, TRUE, TRUE, 0);

	define_label_in_box("Filter", hbox, FALSE, FALSE, DEF_PAD*0.5);

	tb = standard_toolbar_new(NBFFILTERS,
		fourier_filters,
		gtk_tooltips_new(),
		fds->window,
		(gpointer) fds,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);

	gtk_box_pack_start(GTK_BOX(hbox), tb, FALSE, FALSE, DEF_PAD*0.5);
	ffds->toolbar = tb;

	table = gtk_table_new(6, 4, FALSE);
	gtk_widget_show(GTK_WIDGET(table));

	gtk_box_pack_start( GTK_BOX(main_box),table, TRUE, TRUE, 0);
	
	//	Width	

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);

	define_label_in_table ("Width", table, 0, 1, 0, 1, DEF_PAD);
	adj = gtk_adjustment_new (fs->ffs->size_x, 0.0, 100.0, 0.1, 0.1, 0);
	scale = define_scale_in_box (adj, hbox, 1, DEF_PAD*0.5);

	g_signal_connect (G_OBJECT (scale), "button_press_event", 
		GTK_SIGNAL_FUNC(gboolean_set_true_event), (gpointer) &fds->mouse_on_scale);
	g_signal_connect (G_OBJECT (scale), "button_release_event", 
		GTK_SIGNAL_FUNC(draw_inverse_callb), (gpointer) fds);
	g_signal_connect (G_OBJECT (adj), "value_changed", 
		GTK_SIGNAL_FUNC(gfloat_adj_callb), (gpointer) &fs->ffs->size_x);
	g_signal_connect (G_OBJECT (adj), "value_changed", 
		GTK_SIGNAL_FUNC(synchro_depth_callb), (gpointer) fds);
	g_signal_connect (G_OBJECT (adj), "value_changed", 
		GTK_SIGNAL_FUNC(update_filter), (gpointer) fds);

	ffds->size_x_adj = adj;
	ffds->size_x_wdg = hbox;

	percent_buttons_new_with_callb (hbox, adj, draw_inverse_callb2, (gpointer) fds);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 0, 1, 0, 0, 0, 0);

	//	Depth

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);

	define_label_in_table ("Depth", table, 0, 1, 1, 2, DEF_PAD);
	adj = gtk_adjustment_new (fs->ffs->size_y, 0.0, 100.0, 0.1, 0.1, 0);
	scale = define_scale_in_box (adj, hbox, 1, DEF_PAD*0.5);
//	scale = define_scale_in_table (adj, table, 1, 2, 1, 2, 1, DEF_PAD*0.5);
	
	g_signal_connect (G_OBJECT (scale), "button_press_event", 
		GTK_SIGNAL_FUNC(gboolean_set_true_event), (gpointer) &fds->mouse_on_scale);
	g_signal_connect (G_OBJECT (scale), "button_release_event", 
		GTK_SIGNAL_FUNC(draw_inverse_callb), (gpointer) fds);
	g_signal_connect (G_OBJECT (adj), "value_changed", 
		GTK_SIGNAL_FUNC(gfloat_adj_callb), (gpointer) &fs->ffs->size_y);
	g_signal_connect (G_OBJECT (adj), "value_changed", 
		GTK_SIGNAL_FUNC(synchro_width_callb), (gpointer) fds);
	g_signal_connect (G_OBJECT (adj), "value_changed", 
		GTK_SIGNAL_FUNC(update_filter), (gpointer) fds);

	ffds->size_y_adj = adj;
	ffds->size_y_wdg = hbox;

	percent_buttons_new_with_callb (hbox, adj, draw_inverse_callb2, (gpointer) fds);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 1, 2, 0, 0, 0, 0);

	// Button for synchronozing width and depth
	button = gtk_toggle_button_new_with_label ("=");
	gtk_widget_show (GTK_WIDGET(button));
	gtk_table_attach (GTK_TABLE (table), button, 3, 4, 0, 2, 0, 0, GTK_FILL, GTK_EXPAND | GTK_FILL);
	gtk_signal_connect (	GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(toggle_check_button_callb),
				(gpointer) &fds->fs->ffs->xy_synchro);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(button), fds->fs->ffs->xy_synchro);

	//	Band width

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);

	define_label_in_table ("Band", table, 0, 1, 2, 3, DEF_PAD);
	adj = gtk_adjustment_new (fs->ffs->band_width, 0.0, 100.0, 0.1, 0.1, 0);
	scale = define_scale_in_box (adj, hbox, 1, DEF_PAD*0.5);
//	scale = define_scale_in_table (adj, table, 1, 2, 2, 3, 1, DEF_PAD*0.5);	
	g_signal_connect (G_OBJECT (scale), "button_press_event", 
		GTK_SIGNAL_FUNC(gboolean_set_true_event), (gpointer) &fds->mouse_on_scale);
	g_signal_connect (G_OBJECT (scale), "button_release_event", 
		GTK_SIGNAL_FUNC(draw_inverse_callb), (gpointer) fds);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(gfloat_adj_callb),&fs->ffs->band_width );
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(update_filter), fds);
	ffds->band_width_adj = adj;
	ffds->band_width_wdg = hbox;

	percent_buttons_new_with_callb (hbox, adj, draw_inverse_callb2, (gpointer) fds);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 2, 3, 0, 0, 0, 0);

	//	Amplitude

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);

	define_label_in_table ("Amplitude", table, 0, 1, 3, 4, DEF_PAD);
	adj = gtk_adjustment_new (fs->ffs->amplitude, -100.0, 100.0, 0.1, 0.1, 0);
	scale = define_scale_in_box (adj, hbox, 1, DEF_PAD*0.5);
//	scale = define_scale_in_table (adj, table, 1, 2, 3, 4, 1, DEF_PAD*0.5);	
	g_signal_connect (G_OBJECT (scale), "button_press_event", 
		GTK_SIGNAL_FUNC(gboolean_set_true_event), (gpointer) &fds->mouse_on_scale);
	g_signal_connect (G_OBJECT (scale), "button_release_event", 
		GTK_SIGNAL_FUNC(draw_inverse_callb), (gpointer) fds);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(gdouble_adj_callb),&fs->ffs->amplitude );
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(update_filter), fds);
	ffds->amplitude_adj = adj;
	ffds->amplitude_wdg = hbox;

	percent_buttons_new_with_callb (hbox, adj, draw_inverse_callb2, (gpointer) fds);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 3, 4, 0, 0, 0, 0);


	//	Translation X

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);

	define_label_in_table ("+X", table, 0, 1, 4, 5, DEF_PAD);
	adj = gtk_adjustment_new (fs->ffs->translate_x, -50.0, 50.0, 0.1, 0.1, 0);
	scale = define_scale_in_box (adj, hbox, 1, DEF_PAD*0.5);
//	scale = define_scale_in_table (adj, table, 1, 2, 4, 5, 1, DEF_PAD*0.5);
	g_signal_connect (G_OBJECT (scale), "button_press_event", 
		GTK_SIGNAL_FUNC(gboolean_set_true_event), (gpointer) &fds->mouse_on_scale);
	g_signal_connect (G_OBJECT (scale), "button_release_event", 
		GTK_SIGNAL_FUNC(draw_inverse_callb), (gpointer) fds);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(gfloat_adj_callb),&fs->ffs->translate_x );
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(update_filter), fds);
	ffds->translate_x_adj = adj;
	ffds->translate_x_wdg = hbox;

	percent_buttons_new_with_callb (hbox, adj, draw_inverse_callb2, (gpointer) fds);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 4, 5, 0, 0, 0, 0);

	//	Translation Y (Z...)

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);

	define_label_in_table ("+Y", table, 0, 1, 5, 6, DEF_PAD);
	adj = gtk_adjustment_new (fs->ffs->translate_y, -50.0, 50.0, 0.1, 0.1, 0);
	scale = define_scale_in_box (adj, hbox, 1, DEF_PAD*0.5);
//	scale = define_scale_in_table (adj, table, 1, 2, 5, 6, 1, DEF_PAD*0.5);	
	g_signal_connect (G_OBJECT (scale), "button_press_event", 
		GTK_SIGNAL_FUNC(gboolean_set_true_event), (gpointer) &fds->mouse_on_scale);
	g_signal_connect (G_OBJECT (scale), "button_release_event", 
		GTK_SIGNAL_FUNC(draw_inverse_callb), (gpointer) fds);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(gfloat_adj_callb),&fs->ffs->translate_y );
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(update_filter), fds);
	ffds->translate_y_adj = adj;
	ffds->translate_y_wdg = hbox;

	percent_buttons_new_with_callb (hbox, adj, draw_inverse_callb2, (gpointer) fds);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 5, 6, 0, 0, 0, 0);

	// The default is "no filter"

	gtk_widget_set_sensitive(GTK_WIDGET(ffds->size_x_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ffds->size_y_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ffds->band_width_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ffds->amplitude_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ffds->translate_x_wdg),FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(ffds->translate_y_wdg),FALSE);

	return ffds;
}

GtkWidget *build_scrolled_window (view_struct *vs, gint win_width, gint win_height) {

	GtkWidget *scrolled_window;
	GtkAdjustment *vadj,*hadj;

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(scrolled_window));
	hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(scrolled_window));

	// We process the viewport separately, instead of using
	// gtk_scrolled_window_add_with_viewport
	// to be able to rescale the area when its size changes
	vs->viewport = gtk_viewport_new (hadj, vadj);
	gtk_widget_show(vs->viewport);
	gtk_widget_set_size_request (vs->viewport, vs->width, vs->height);

	gtk_widget_set_size_request (scrolled_window, win_width+SCALE_PAD, win_height+SCALE_PAD);

	gtk_container_add (GTK_CONTAINER(vs->viewport), vs->area);
	gtk_container_add (GTK_CONTAINER(scrolled_window), vs->viewport);

	gtk_widget_show (scrolled_window);

	return scrolled_window;
};

static gint result_preview (GtkWidget *wdg, gpointer fds_ptr) {
	// Set the Fourier inverse transform as the image to preview
 	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;	
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return TRUE;
	fds->preview_choice = PREVIEW_RESULT;
	gl_set_input_grid (fds->gl_preview, fds->fs->tmp_buf_inv, fds->max_x, fds->max_y, HF_TYPE_ID);
	gl_area_update (fds->gl_preview);
	set_render_buffer (fds->render_str, fds->fs->tmp_buf_inv, fds->max_x, fds->max_y);
	return FALSE;
}

static gint transform_preview (GtkWidget *wdg, gpointer fds_ptr) {
	// Set the Fourier transform as the image to preview
 	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;	
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return TRUE;
	fds->preview_choice = PREVIEW_TRANSFORM;
	gl_set_input_grid (fds->gl_preview, fds->fs->tmp_buf, fds->max_x, fds->max_y, HF_TYPE_ID);
	gl_area_update (fds->gl_preview);
	set_render_buffer (fds->render_str, fds->fs->tmp_buf, fds->max_x, fds->max_y);
	return FALSE;
}

static gint switch_page_callback(GtkWidget *notebook, GtkWidget *page, gint page_num, gpointer fds_ptr)
{
 	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
//	printf("Switch page\n");
	if (page_num==fds->real_tab) {
		fds->fs->current_view = REAL;
	}
	else
		if (page_num==fds->imaginary_tab) {
			fds->fs->current_view = IMAGINARY;
		}
		else { // Module
			fds->fs->current_view = MODULE;
		}
	draw_fourier (fds);
    	return FALSE;
}

static gint reuse_result_callb (GtkWidget *button, gpointer fds_ptr) {
 	fourier_dialog_struct *fds = (fourier_dialog_struct *) fds_ptr;
	fourier_struct_init (fds->fs, fds->fs->tmp_buf_inv,fds->fs->max_x, fds->fs->max_y, HF_TYPE_ID) ;
	fourier_compute_inverse (fds->fs);
	set_toolbar_button_from_label (fds->ffds->toolbar, _("No filter"), TRUE);
	draw_fourier(fds);
	return TRUE;
}

GtkWidget *build_fourier_widget (fourier_dialog_struct *fds) {

	GtkWidget *main_box, *view_box, *fourier_parts_box, *notebook, *scr_win, *hbox, *vbox, *vbox2, *frame, *button; // *scale
	
	GSList *group=NULL;

	// main_box gives room to add future top or bottom widgets

	main_box = gtk_vbox_new(FALSE,0);
	gtk_widget_show (main_box);

	view_box = gtk_hbox_new(FALSE,0);
	gtk_widget_show (view_box);
	gtk_box_pack_start( GTK_BOX(main_box), view_box, TRUE, TRUE, 0);

	fourier_parts_box = gtk_vbox_new(FALSE,0);
	gtk_widget_show (fourier_parts_box);
	gtk_box_pack_start( GTK_BOX(view_box), fourier_parts_box, TRUE, TRUE, 0);

	// Adding scale % controls
	gtk_box_pack_start(GTK_BOX(fourier_parts_box),fds->sbs->box, TRUE, TRUE, DEF_PAD*0.5);

	/* REAL, MODULE and IMAGINARY parts appear on different tabs */
 	notebook = gtk_notebook_new ();
	fds->notebook = notebook;
    	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	frame = frame_new ("Frequency domain", DEF_PAD);
	gtk_container_add(GTK_CONTAINER(frame),notebook);
	gtk_box_pack_start(GTK_BOX(fourier_parts_box),frame, TRUE, TRUE, DEF_PAD*0.5);
    	gtk_widget_show(notebook);

	scr_win = build_scrolled_window(fds->freal_view, FOURIER_VIEW_SIZE, FOURIER_VIEW_SIZE );
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), scr_win, 
		gtk_label_new(_("Real")));
	fds->real_tab = 
		gtk_notebook_page_num(GTK_NOTEBOOK(notebook), scr_win);
	fds->freal_scrwin = scr_win;

	scr_win = build_scrolled_window(fds->fimaginary_view, FOURIER_VIEW_SIZE, FOURIER_VIEW_SIZE) ;
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), scr_win, 
		gtk_label_new(_("Imaginary")));
	fds->imaginary_tab = 
		gtk_notebook_page_num(GTK_NOTEBOOK(notebook), scr_win);
	fds->fimaginary_scrwin = scr_win;

	scr_win = build_scrolled_window(fds->fmodule_view, FOURIER_VIEW_SIZE, FOURIER_VIEW_SIZE);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), scr_win, 
		gtk_label_new(_("Magnitude")));
	fds->module_tab = 
		gtk_notebook_page_num(GTK_NOTEBOOK(notebook), scr_win);
	fds->fmodule_scrwin = scr_win;

	synchro_scr_win (fds->fmodule_scrwin, fds->fimaginary_scrwin);
	synchro_scr_win (fds->fmodule_scrwin, fds->freal_scrwin);
	synchro_scr_win (fds->fimaginary_scrwin, fds->freal_scrwin);

//	The switch page callback computes (initializes) the default filter
//	Setting the default page triggers the callback
   	g_signal_connect(G_OBJECT(notebook), "switch_page",
			 G_CALLBACK(switch_page_callback), (gpointer) fds);

	switch (fds->fs->current_view) {
		case REAL:
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), fds->real_tab);
			break;
		case IMAGINARY:
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), fds->imaginary_tab);
			break;
		default: // MODULE	
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), fds->module_tab);
	}

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show (vbox);

	gtk_box_pack_start( GTK_BOX(view_box), vbox, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(vbox),align_widget(create_render_bar (fds->window, NULL, fds->render_str) ,0.5,0.0), FALSE, FALSE, DEF_PAD *0.5);

	frame = frame_new ("Preview choice", DEF_PAD);
	gtk_box_pack_start( GTK_BOX(vbox), frame, FALSE, FALSE, DEF_PAD*0.5);

	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show (vbox2);
	gtk_container_add(GTK_CONTAINER(frame),vbox2);
	define_radio_button_in_box_with_data (vbox2, &group, "Spatial domain", (gpointer) result_preview, fds, fds->preview_choice==PREVIEW_RESULT);
	define_radio_button_in_box_with_data (vbox2, &group, "Frequency domain", (gpointer) transform_preview, fds, fds->preview_choice==PREVIEW_TRANSFORM);

	frame = frame_new ("Spatial domain", DEF_PAD);
	gtk_container_add(GTK_CONTAINER(frame), fds->finv_view->area);
	gtk_box_pack_start(GTK_BOX(vbox),align_widget(frame,0.5,0.2), TRUE, TRUE, DEF_PAD *0.5);

	// Reuse result button
	button = gtk_button_new ();

	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (reuse_result_callb), (gpointer) fds);

	hbox = xpm_label_box (fds->window, reuse_result_xpm, _("Reuse result"));

	gtk_widget_show (hbox);

    	gtk_container_add (GTK_CONTAINER (button), hbox);

    	gtk_widget_show (button);

	gtk_box_pack_start(GTK_BOX(vbox),align_widget(button,0.5,0.0), TRUE, TRUE, DEF_PAD *0.5);

	// Fourier parts controls

	fds->ffds = fourier_filter_dialog_new(fds);
	gtk_box_pack_start(GTK_BOX(fourier_parts_box),fds->ffds->filter_box, TRUE, TRUE, DEF_PAD*0.5);

	// Open GL preview
	fds->gl_preview = create_gl_preview_dialog (gl_defaults_init(), fds->window, fds->fs->tmp_buf_inv, fds->fs->max_x, fds->fs->max_y, HF_TYPE_ID, NULL, NULL);	

	gtk_box_pack_start(GTK_BOX(view_box), fds->gl_preview->gl_dialog,
		TRUE, TRUE, DEF_PAD *0.5);
	return main_box;
}

// unactivate_fourier_dialog is activated by the "keep and quit" button (OK)
// The event hides the modal window and releases the grab on it 
// (built-in in utils.c:modal_dialog_showhide)
// unactivate_fourier_dialog transfers the result into the current HF if the user chose "OK"
gboolean unactivate_fourier_dialog (GtkWidget *wdg, gpointer fds_ptr)
{
	fourier_dialog_struct *fds;
	fds = (fourier_dialog_struct *) fds_ptr;
	// We output the current display buffer
//	printf("output_buf in unactivate_fourier: %x -> %x\n",fds_ptr, fds->output_buf);
//	convert_finv (fds->fs, fds->output_buf, fds->buffer_type);
	if (fds->preview_choice==PREVIEW_RESULT)
		memcpy(fds->output_buf,fds->fs->tmp_buf_inv,fds->max_x*fds->max_y*sizeof(hf_type));
	else
		memcpy(fds->output_buf,fds->fs->tmp_buf,fds->max_x*fds->max_y*sizeof(hf_type));
	if (fds->post_processing) {
		(*fds->post_processing) (fds->post_processing_data);
	}
	return FALSE;
}

static void center_scrwin (GtkWidget *scrolled_window, gint width, gint height) {
	// Center the main widget contained in the scrolled window
	// Width X height = size of the contained widget
	GtkAdjustment *vadj,*hadj;
	GtkWidget *vsb,*hsb;
	if (!scrolled_window)
	 	return;
	vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(scrolled_window));
	hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(scrolled_window));
	hsb = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(scrolled_window));
	vsb = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(scrolled_window));
	gtk_adjustment_set_value (vadj, (gdouble) ((height>>1) + 
	(GTK_WIDGET(hsb)->allocation.height>>1) - 
	(GTK_WIDGET(scrolled_window)->allocation.height>>1)));
	gtk_adjustment_set_value (hadj, (gdouble) ((width>>1) +
	(GTK_WIDGET(vsb)->allocation.width>>1) -
	(GTK_WIDGET(scrolled_window)->allocation.width>>1)));
}

static gint center_callb (GtkWidget *wdg, gpointer fds_ptr) {
	fourier_dialog_struct *fds;
	fds = (fourier_dialog_struct *) fds_ptr;
	center_scrwin (fds->freal_scrwin, fds->freal_view->width, fds->freal_view->height);
	center_scrwin (fds->fimaginary_scrwin, fds->fimaginary_view->width, fds->fimaginary_view->height);
	if (fds->fmodule_scrwin)
		center_scrwin (fds->fmodule_scrwin, fds->fmodule_view->width, fds->fmodule_view->height);
	return FALSE;
}

static gint cancel_callb (GtkWidget *wdg, gpointer fds_ptr){
	fourier_dialog_struct *fds;
	fds = (fourier_dialog_struct *) fds_ptr;
//	printf("CANCEL_CALLB: %d\n",fds->mouse_on_scale);
	if (fds->mouse_on_scale)
		return TRUE;
	else
		return FALSE;
}

// activate_fourier_dialog feeds the structure with the current height field data
// (or any other image data) and displays the dialog in a modal mode
void activate_fourier_dialog (fourier_dialog_struct *fds, gpointer input_buf, gpointer output_buf, gint max_x, gint max_y, gint data_type, gpointer post_process, gpointer post_process_data)
{
	GtkWidget *wdg;
	gint display_x, display_y;
	fds->max_x = max_x;
	fds->max_y = max_y;
	fds->input_buf = input_buf;
	fds->output_buf = output_buf;
	fds->buffer_type = data_type;
//	printf("output_buf in activate_fourier: %x -> %x\n",fds, fds->output_buf);
	fourier_struct_init (fds->fs, input_buf, max_x, max_y, data_type) ;
	fourier_compute_inverse (fds->fs);
	fds->post_processing = post_process;
	fds->post_processing_data = post_process_data;
	display_x = RIGHT_SHIFT(max_x,fds->sbs->current);
	display_y = RIGHT_SHIFT(max_y,fds->sbs->current);

	if (fds->fmodule_view) {
		if ((display_x!=fds->fmodule_view->width) ||
		(display_y!=fds->fmodule_view->height)) {
			resize_area (fds->fmodule_view, display_x, display_y);
		}
	}
	else
		fds->fmodule_view = view_struct_new_with_rgb (display_x, display_y,
			(gpointer) compute_fmodule, (gpointer) fds, TRUE) ;

	if (fds->freal_view) {
		if ((display_x!=fds->freal_view->width) ||
		(display_y!=fds->freal_view->height))
			resize_area (fds->freal_view, display_x, display_y);
	}
	else
		fds->freal_view = view_struct_new_with_rgb (display_x, display_y,
			(gpointer) compute_freal, (gpointer) fds, TRUE);

	if (fds->fimaginary_view) {
		if ((display_x!=fds->fimaginary_view->width) ||
		(display_y!=fds->fimaginary_view->height))
			resize_area (fds->fimaginary_view, display_x, display_y);
	}
	else
		fds->fimaginary_view = view_struct_new_with_rgb (display_x, display_y,
			(gpointer) compute_fimaginary, (gpointer) fds, TRUE);

//	printf("ACTIVATE_FOURIER_DIALOG; display_x/y: (%d,%d); fds->fmodule_view_x/y: (%d,%d)\n", display_x, display_y, fds->fmodule_view->width, fds->fmodule_view->height);

	if (!fds->window) {
		fds->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_signal_connect (GTK_OBJECT (fds->window), "show",
		(GtkSignalFunc) center_callb, (gpointer) fds);
		gtk_widget_realize (fds->window);
		wdg = build_fourier_widget (fds);
		if (fds->preview_choice==PREVIEW_RESULT) {
			set_render_buffer (fds->render_str, fds->fs->tmp_buf_inv, fds->fs->max_x, fds->fs->max_y);
		}
		else {
			set_render_buffer (fds->render_str, fds->fs->tmp_buf, fds->fs->max_x, fds->fs->max_y);
		}
		set_gl_render (fds->render_str, fds->gl_preview);
		gtk_signal_connect_after (GTK_OBJECT (fds->window), "show", (GtkSignalFunc) refresh_gl_callb, (gpointer) fds->gl_preview);	

		modal_dialog_showhide_with_titles (
			fds->window,
			wdg, 
			_("The Geomorph's Fourier Explorer"),
			unactivate_fourier_dialog, _("Keep and quit"),
			cancel_callb, _("Discard and quit"),
			(gpointer) fds,
			GTK_WIN_POS_CENTER,
			FALSE);
	}
	else {
		// The toolbar filter is set to the "no filter" default
		set_toolbar_button_from_order (fds->ffds->toolbar, 0, TRUE);
		gtk_widget_show(GTK_WIDGET(fds->window));
		gtk_grab_add(GTK_WIDGET(fds->window));
		if (fds->preview_choice==PREVIEW_RESULT) {
			set_render_buffer (fds->render_str, fds->fs->tmp_buf_inv, fds->fs->max_x, fds->fs->max_y);
			gl_set_input_grid (fds->gl_preview, fds->fs->tmp_buf_inv, max_x, max_y, HF_TYPE_ID);
		}
		else {
			set_render_buffer (fds->render_str, fds->fs->tmp_buf, fds->fs->max_x, fds->fs->max_y);
			gl_set_input_grid (fds->gl_preview, fds->fs->tmp_buf, max_x, max_y, HF_TYPE_ID);
		}
		draw_fourier (fds);
	}
}

