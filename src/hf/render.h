/* render.h - headers for render dialog class
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
#ifndef _RENDER
#define _RENDER 1

#include <gtk/gtk.h>
#include "hf.h"
#include "gl_preview.h"

typedef struct {
//	Mirror of data required to render a Povray HF
//	We use a ** when the pointer can be reallocated in the source structure
	gint povray_pid; // Last Povray launch
	gchar **dirname;
	gchar **filename;
	gboolean *if_creation; // Uncommitted changes
	gboolean *if_modified; // Uncommitted changes
	gl_preview_struct *gl_preview;
	hf_type *grid;
	gint max_x;
	gint max_y;
} render_struct;

render_struct *render_struct_new ();

void init_render_struct(render_struct *rs,
	gchar **dirname, 
	gchar **filename, 
	gboolean *if_creation, 
	gboolean *if_modified, 
	gl_preview_struct *glp, 
	hf_type *grid, 
	gint max_x, 
	gint max_y);

void set_render_buffer (render_struct *rs, hf_type *buffer, gint max_x, gint max_y);
void set_gl_render (render_struct *rs, gl_preview_struct *);

GtkWidget *create_render_bar(GtkWidget *window, GtkTooltips *tooltips, render_struct *rs);

// Private functions referenced in the table storing the command bar

void run_callb(GtkWidget *wdg, gpointer data);
void run_with_choice_callb(GtkWidget *wdg, gpointer data);
void stop_callb(GtkWidget *wdg, gpointer data);

#endif
