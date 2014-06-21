/* fault_dialog.h - headers for fault pen
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

#ifndef _FAULT_DIALOG
#define _FAULT_DIALOG 1

#include <gtk/gtk.h>
#include "fault.h"
// #include "hf_wrapper.h"

//	Prototypes

GtkWidget* fault_dialog_new (GtkWidget *window, GtkTooltips *tooltips, 
	gpointer fault_pen, gpointer data);
	
// void accept_fault_fn (hf_wrapper_struct *hfw);

// Eventually remove the next function (required for tests only)
void display_fault_line (gpointer hf_wrapper, gint x0, gint y0, gint x1, gint y1);

#endif // FAULT_DIALOG
