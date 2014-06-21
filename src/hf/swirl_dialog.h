/* swirl_dialog.h - headers for swirl pen
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _SWIRL_DIALOG
#define _SWIRL_DIALOG 1

#include <gtk/gtk.h>
#include "hf.h"
#include "view_area.h"
#include "swirl.h"

//	Prototypes

GtkWidget* swirl_dialog_new (GtkWidget *window, GtkTooltips *tooltips, 
	swirl_struct *pen, gpointer data);

#endif // SWIRL_DIALOG
