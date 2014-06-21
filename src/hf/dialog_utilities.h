/* dialog_utilities.h - headers for dialog utilities
 *
 * Copyright (C) 2003-2004 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         http://geomorph.sourceforge.net
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

#ifndef _DIALOG_UTILITIES
#define _DIALOG_UTILITIES 1

#include <gtk/gtk.h>

#ifndef MOUSE_TOLERANCE
	#define MOUSE_TOLERANCE 8
#endif

#ifndef VERTICAL_BOX
	#define VERTICAL_BOX 10
#endif

#ifndef HORIZONTAL_BOX
	#define HORIZONTAL_BOX 20
#endif

typedef struct {
//	Struct for a line with big dots at each end,
//	typically used as a control line over the display buffer
//	Each end is a handler which can be moved with the mouse
	gint x0;
	gint y0;
	gint x1;
	gint y1;
	gboolean active; // The line is displayed only when active==TRUE
	gboolean mouse_on_0; // TRUE when dragging the line from (x0,y0)
	gboolean mouse_on_1; // TRUE when dragging the line from (x1,y1)
	gboolean revert_ends; // Sometimes we need to change the direction of drawing
				// when the line controls a non-symmetric polyline
	GdkGC *gc;
	gint tolerance; // In pixels
	// Tolerance is the radius around (x0,y0) and (x1,y1) inside which
	// the mouse can drag the dot
} control_line_struct;

GtkWidget *seed_dialog_new (gpointer callb_data, gint seed, 
	gint (*change_seed) (GtkWidget *, gpointer));

// "box" == VERTICAL_BOX or HORIZONTAL_BOX
GtkWidget *seed_dialog_new_lbl(gpointer data, gint first_seed,
	gint (*change_seed) (GtkWidget *, gpointer), gboolean if_lbl, gint box );
	
gint refresh_seed(GtkWidget *button, gpointer entry) ;
gint refresh_seed_from_seedbox (GtkWidget *wdg, gpointer hbox) ;

void rotate_buttons_new(GtkWidget *box, gpointer adj) ;
void percent_buttons_new(GtkWidget *box, gpointer adj) ;
void percent_buttons_negative_new(GtkWidget *box, gpointer adj) ;

void percent_buttons_new_with_callb(GtkWidget *box, gpointer adj, void (*callb_fn) (GtkWidget *, gpointer), gpointer data) ;

GtkWidget *tiling_control (gpointer data);

GtkWidget *effect_toolbar (GtkWidget *window, gpointer data);

control_line_struct *control_line_new (GtkWidget *area,
	gint x0, gint y0, gint x1, gint y1);

void control_line_update (control_line_struct *cl,
	gint x0, gint y0, gint x1, gint y1);
void control_line_update0 (control_line_struct *cl, gint x0, gint y0);
void control_line_update1 (control_line_struct *cl, gint x1, gint y1);

gboolean test_control_line (control_line_struct *cl, gint x, gint y);
void control_line_undrag (control_line_struct *cl);

void control_line_activate (control_line_struct *cl);
void control_line_unactivate (control_line_struct *cl);

void control_line_draw (control_line_struct *cl,
	GdkDrawable *dw, gint xmax, gint ymax);
	
gint optimize_on_mouse_click (GtkWidget *scale, gpointer data);
GtkWidget *define_frame_with_hiding_arrows (gchar *label, GtkWidget *embedded_widget, GtkWidget * parent_window, gboolean display_on_start) ;

//	Creates an empty frame with text preceded by "Options for", 
//	ready for adding options dialog

GtkWidget *options_frame_new(gchar *label);
GtkWidget *options_frame_new_pad(gchar *label, gint pad);

GtkWidget *define_options_frame_with_hiding_arrows (gchar *label, GtkWidget *embedded_widget, GtkWidget * parent_window, gboolean display_on_start) ;

void plus_minus_1_buttons (GtkObject *adj, GtkWidget *box);

void uniform_percent_buttons_new (GtkWidget *box, gpointer adj);

void optimize_on_mouse_release (GtkWidget *scale, gboolean *refresh_on_release);

#endif // _DIALOG_UTILITIES
