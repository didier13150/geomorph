/* draw_tools_dialog.h - headers for pen choice dialog and some utilitaries
 *
 * Copyright (C) 2003-2005 Patrice St-Gelais
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

#ifndef _DRAW_TOOLS_DIALOG
#define _DRAW_TOOLS_DIALOG 1

#include <gtk/gtk.h>
// #include "hf.h"
#include "draw_hf.h"
#include "draw_hf_dialog.h"
#include "draw_dialog.h"
#include "draw_crater_dialog.h"
#include "fault_dialog.h"
#include "fill_dialog.h"
#include "swirl_dialog.h"
#include "line_dialog.h"
// #include "crack_dialog.h"
#include "view_area.h"

//	Context (dialog ID)
#ifndef DRAW_CONTEXT
	#define DRAW_CONTEXT 1
#endif
#ifndef DRAW_HF_CONTEXT
	#define DRAW_HF_CONTEXT 2
#endif
#ifndef DRAW_CRATER_CONTEXT
	#define DRAW_CRATER_CONTEXT 3
#endif
#ifndef SWIRL_CONTEXT
	#define SWIRL_CONTEXT 4
#endif
#ifndef FAULT_CONTEXT
	#define FAULT_CONTEXT 5
#endif
#ifndef FILL_CONTEXT
	#define FILL_CONTEXT 6
#endif

typedef struct {
//	Main dialog (container of all subdialogs)
	GtkWidget *draw_tools_dialog;

//	Current subdialog
	GtkWidget *current_draw_tools;
	
//	Context (kind of dialog ID)
	gint context;

//	Common options
	gdouble draw_begin_x;	// We need real coordinates for smooth lines
	gdouble draw_begin_y;
	gboolean line_drawing;	//	TRUE when button was pressed in the HF,
						// 	even when the mouse is not dragging yet
	
	dist_matrix_struct *dist_matrix; // Copy from hf_options, for convenience

//	DRAW OPTIONS (standard pen)
	GtkWidget *draw_dialog;	// Subdialog
	pen_struct *pen;
	view_struct *preview;

//	DRAW OPTIONS (hf pen -> fractal pen)
	GtkWidget *draw_hf_dialog;	// Subdialog
	draw_hf_struct *hf_pen;
	view_struct *hf_preview;

//	DRAW OPTIONS (craters)
	GtkWidget *draw_crater_dialog;	// Subdialog
	draw_crater_struct *crater_pen;
	
//	DRAW OPTIONS (swirl)
	GtkWidget *swirl_dialog;
	swirl_struct *swirl_pen;
	
//	DRAW OPTIONS (fill)
	GtkWidget *fill_dialog;
	GtkWidget *fill_accept;
	fill_struct *fill_pen;
	
//	DRAW OPTIONS (fault)
	GtkWidget *fault_dialog;
	fault_struct *fault_pen;
	line_dialog_struct *fault_line_dialog;
	gboolean fault_auto_accept_flag;
	GtkWidget *fault_accept;

//	DRAW OPTIONS (cracks)
//	GtkWidget *crack_dialog;
//	crack_struct *crack_pen;

} draw_tools_struct;

void draw_callb(GtkWidget *,gpointer );
void draw_hf_callb(GtkWidget *,gpointer );
void draw_crater_callb(GtkWidget *,gpointer );
void swirl_callb(GtkWidget *, gpointer);
void fault_callb(GtkWidget *, gpointer);
void fill_callb(GtkWidget *, gpointer);
// void crack_callb(GtkWidget *, gpointer);

draw_tools_struct *draw_tools_new (GtkWidget *draw_tools_dialog, 
	dist_matrix_struct *dist_matrix, gdouble** gauss_list);
void draw_tools_free (draw_tools_struct *);
void begin_stroke (draw_tools_struct *dts, gdouble x, gdouble y);
void end_stroke (draw_tools_struct *dts);

gint button_press (GtkWidget *widget, GdkEventButton *event, gpointer data);
gint button_release (GtkWidget *widget, GdkEventButton *event, gpointer data);
gint motion_notify (GtkWidget *widget, GdkEventMotion *event, gpointer data);

GtkWidget *draw_tools_dialog_new (GtkWidget *window,
		GtkTooltips *tooltips, draw_tools_struct *dts, gpointer callb_data);

#endif // DRAW_TOOLS_DIALOG
