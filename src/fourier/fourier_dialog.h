/* fourier_dialog.h - Headers for the Geomorph Fourier Explorer dialog
 *
 * Copyright (C) 2005 Patrice St-Gelais
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
#ifndef FOURIER_DIALOG_H
#define FOURIER_DIALOG_H

#include <gtk/gtk.h>
#include "fourier.h"
#include "../hf/view_area.h"
#include "../utils/scale_buttons.h"
#include "../hf/gl_preview.h"
#include "../hf/render.h"

#ifndef FOURIER_VIEW_SIZE
	#define FOURIER_VIEW_SIZE 256
#endif

#ifndef SCALE_PAD
	#define SCALE_PAD 25
#endif

// Preview choice
#ifndef PREVIEW_RESULT
	#define PREVIEW_RESULT 0
#endif

#ifndef PREVIEW_TRANSFORM
	#define PREVIEW_TRANSFORM 1
#endif

typedef struct {
	GtkWidget *filter_box;
	GtkWidget *size_x_wdg;
	GtkObject *size_x_adj;
	GtkWidget *size_y_wdg;
	GtkObject *size_y_adj;
	GtkWidget *band_width_wdg;
	GtkObject *band_width_adj;
	GtkWidget *amplitude_wdg;
	GtkObject *amplitude_adj;
	GtkWidget *translate_x_wdg;
	GtkObject *translate_x_adj;
	GtkWidget *translate_y_wdg;
	GtkObject *translate_y_adj;
	GtkWidget *toolbar;
} fourier_filter_dialog_struct;

typedef struct {
	fourier_struct *fs;
	GtkWidget *notebook;
	guint real_tab; // Number of the tab containing the real display
	guint imaginary_tab;
	guint module_tab;
	view_struct *fmodule_view;
	GtkWidget *fmodule_scrwin;
	view_struct *freal_view;
	GtkWidget *freal_scrwin;
	view_struct *fimaginary_view;
	GtkWidget *fimaginary_scrwin;
	view_struct *finv_view;
	GtkWidget *window;
	gpointer input_buf;
	gpointer output_buf;
	gint max_x;
	gint max_y;
	gint buffer_type; // as defined in hf.h: GDOUBLE_ID, HF_TYPE_ID, GINT_ID, UNSIGNED_CHAR_ID
	scale_buttons_struct *sbs;
	fourier_filter_dialog_struct *ffds;
	// Flag indicating if the user is clicking on a scale
	// Repeated clicks trigger a delete event on the parent window,
	// which is an undesirable behaviour
	gboolean mouse_on_scale;
	// Function to execute when closing the Fourier dialog
	// and saving the data
	void (*post_processing) (gpointer data);
	gpointer post_processing_data;
	gl_preview_struct *gl_preview;
	gint preview_choice;
	render_struct *render_str;
} fourier_dialog_struct;

fourier_dialog_struct *fourier_dialog_new ();
void fourier_dialog_free (fourier_dialog_struct *fds);

// activate_fourier_dialog feeds the structure with the current height field data
// (or any other image data) and displays the dialog in a modal mode
void activate_fourier_dialog (fourier_dialog_struct *fds, gpointer input_buf, gpointer output_buf, gint max_x, gint max_y, gint data_type, gpointer post_process, gpointer post_process_data);

// unactivate_fourier_dialog transfers the result into the current HF if the
// user chose "OK" instead of "Cancel"
gint unactivate_fourier_dialog (GtkWidget *wdg, gpointer fds_ptr);

// build_fourier_widget builds the dialog embedded in the modal dialog
GtkWidget *build_fourier_widget (fourier_dialog_struct *);

#endif
