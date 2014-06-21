/* hf_creation_dialog.h - headers for height field creation dialog class
 *
 * Copyright (C) 2001, 2006 Patrice St-Gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

//	This file includes the prototypes for the dialog creation functions
//	for each HF subtypes
//	These functions are defined in other files than hf_dialog.c

#ifndef _HF_CREATION_DIALOG
#define _HF_CREATION_DIALOG 1

#include <gtk/gtk.h>
#include "globals.h"

typedef struct {
	GtkWidget *hf_types_toolbar;
	omenu_struct_type *size_menu;
} hf_creation_wdg_struct;

GtkWidget* hf_creation_dialog_new (GtkWidget *window, GtkTooltips *tooltips,
			gpointer hf_options_ptr, gpointer widget_list_ptr);

GtkWidget *hf_types_toolbar_new (GtkWidget *window, GtkTooltips *tooltips,
		GtkWidget **tb_widget, gpointer hf_options_ptr);

void hfuniform_callb(GtkWidget *wdg, gpointer data) ;
void hfsubdiv1_callb(GtkWidget *wdg, gpointer data);
void hfsubdiv2_callb(GtkWidget *wdg, gpointer data);
void hfsurfadd_callb(GtkWidget *wdg, gpointer data);

void hf_commit_creation(gpointer);
void set_size_menu_from_size (gpointer hfo_ptr, gint size);

/***********************************************************************************/

//	Prototypes for HF subtypes creation dialog functions
GtkWidget * uniform_dialog_new(gpointer data, gint default_value);

GtkWidget *subdiv1_dialog_new(gpointer data);
GtkWidget *subdiv2_dialog_new(gpointer data);
GtkWidget *surfadd_dialog_new(gpointer data);
// GtkWidget *waves_dialog_new(gpointer data);

#endif // _HF_CREATION_DIALOG
