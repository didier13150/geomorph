/* draw_crater_dialog.h - headers for crater drawing with a pen
 *	Some functions are also used for automatic crater generation
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

#ifndef _DRAW_CRATER_DIALOG
#define _DRAW_CRATER_DIALOG 1

#include "craters.h"

//	Prototypes

GtkWidget* draw_crater_dialog_new (GtkWidget *window, GtkTooltips *tooltips, 
	draw_crater_struct *crater, gpointer data);
void crater_boundaries_dialog (GtkWidget *box, draw_crater_struct* );
void crater_depth_n_noise_dialog (GtkWidget *dialog, draw_crater_struct *crater) ;

#endif // DRAW_CRATER_DIALOG



