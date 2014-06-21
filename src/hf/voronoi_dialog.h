/* voronoi_dialog.h:  dialogs for building voronoi diagrams (cracks network)
 *
 * Copyright (C) 2006 Patrice St-Gelais
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

#ifndef _VORONOI_DIALOG
#define _VORONOI_DIALOG 1

#include <gtk/gtk.h>
#include "view_area.h"
#include "hf.h"

#ifndef NOISE_PREVIEW_SIZE
#define NOISE_PREVIEW_SIZE 128
#endif

typedef struct {
	GtkWidget *dialog;
	GtkWidget *accept;
	GtkObject *adj_cell_size;
	GtkObject *adj_random_variation;
	GtkWidget *gener_noise;
	GtkObject *adj_noise_level;
	GtkWidget *crack_width_type;
	GtkObject *adj_min_width;
	GtkObject *adj_max_width;
	GtkWidget *wdg_edges_level;
	GtkObject *adj_edges_level;
	view_struct *noise_preview;
	hf_type *preview_buf;
} voronoi_dialog_struct;

voronoi_dialog_struct *voronoi_dialog_struct_new ();
void voronoi_dialog_struct_free(voronoi_dialog_struct *);
GtkWidget *voronoi_dialog_new(gpointer data);
void voronoi_callb(GtkWidget *wdg, gpointer data) ;

#endif // _VORONOI_DIALOG
