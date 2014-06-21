/* line_dialog.h - headers for managing polyline (fractal line) controls
 *
 * Copyright (C) 2004 Patrice St-Gelais
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _LINE_DIALOG
#define _LINE_DIALOG 1

#include <gtk/gtk.h>
#include "line.h"
#include "view_area.h"

typedef struct {
	fractal_line_struct *f;
	GtkWidget *dialog;
	view_struct *line_preview;
	GtkWidget *seed_dialog;
	gpointer external_data;
	gint (*update_external_data) (GtkWidget*, gpointer);
} line_dialog_struct;

//	Prototypes

line_dialog_struct *line_dialog_new (GtkWidget *window, GtkTooltips *tooltips,
	gchar *title, fractal_line_struct *fl,
	gint preview_width, gint preview_height,
	gpointer external_data,
	gint (*update_external_data) (GtkWidget *, gpointer),
	gint (*on_press) (GtkWidget *widget, GdkEventButton *event, gpointer data),
	gint (*on_release) (GtkWidget *widget, GdkEventButton *event, gpointer data) )	;

void line_dialog_free (line_dialog_struct *lds);

gint update_line (GtkWidget *wdg, gpointer data); // data is a line_dialog_struct *

#endif // LINE_DIALOG
