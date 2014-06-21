/* wind_dialog.h:  wind erosion dialogs
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _WIND_DIALOG
#define _WIND_DIALOG 1

#include "globals.h"
#include <gtk/gtk.h>

typedef struct {
	GtkWidget *dialog;	
	GtkWidget *accept;
	GtkObject *adj_steps;
	GtkObject *adj_wind_speed;
	GtkObject *adj_slope_threshold;
	GtkObject *adj_a;
	GtkObject *adj_b;
	gpointer parent_data;
	gint process; // RIPPLES or DUNES
} wind_dialog_struct;

wind_dialog_struct *wind_dialog_struct_new(gint process, gpointer parent_data);

void wind_dialog_struct_free(wind_dialog_struct *);

GtkWidget *wind_ripples_dialog_new(gpointer hfw_ptr);
void ripples_callb(GtkWidget *wdg, gpointer data) ;

GtkWidget *wind_dunes_dialog_new(gpointer hfw_ptr);
void dunes_callb(GtkWidget *wdg, gpointer data) ;


/*
GtkWidget *dunes_dialog_new(gpointer data);
void dunes_callb(GtkWidget *wdg, gpointer data) ;
*/

#endif // _WIND_DIALOG
