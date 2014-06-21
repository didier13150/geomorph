/* draw_hf_dialog.h - headers for HF pen (dotted stroke)
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _DRAW_HF_DIALOG
#define _DRAW_HF_DIALOG 1

#include <gtk/gtk.h>
//	Prototypes

GtkWidget* draw_hf_dialog_new (GtkWidget *window, GtkTooltips *tooltips, 
	gpointer dts_ptr);

#endif // DRAW_HF_DIALOG


