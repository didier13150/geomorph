/* fill_dialog.h - headers for fill bucket / pen
 *
 * Copyright (C) 2005 Patrice St-Gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _FILL_DIALOG
#define _FILL_DIALOG 1

#include <gtk/gtk.h>
#include "fill.h"

//	Prototypes

GtkWidget* fill_dialog_new (GtkWidget *window, GtkTooltips *tooltips, 
	gpointer fault_pen, gpointer data);

void display_fill_area (gpointer hf_wrapper);

void accept_fill_fn (gpointer hf_wrapper);
void reset_fill_fn (gpointer hf_wrapper);

gint update_fill_area (GtkWidget *wdg, gpointer hfw_ptr);

#endif // FAULT_DIALOG
