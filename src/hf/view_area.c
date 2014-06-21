/* view_area.c - functions for managing standard view area
 *
 * Copyright (C) 2003, 2007 Patrice St-Gelais
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

 //	Modified 2005-01-09 for removing double buffering (now auto with GTK2) PSTG
 
#include "view_area.h"
#include <stdlib.h>
#include <stdio.h>

void resize_area (view_struct *vs, gint new_width, gint new_height) {
	if (vs->if_rgb)
		vs->buf8 = (unsigned char *) x_realloc(vs->buf8, 3*new_width * new_height * sizeof(unsigned char), "unsigned char (vs->buf8 in view_area.c - realloc)");
	else
		vs->buf8 = (unsigned char *) x_realloc(vs->buf8, new_width * new_height * sizeof(unsigned char), "unsigned char (vs->buf8 in view_area.c - realloc)");
	gtk_widget_set_size_request (vs->area, new_width, new_height);
	if (vs->viewport) {
		gtk_widget_set_size_request (vs->viewport, new_width, new_height);
	}
	vs->width = new_width;
	vs->height = new_height;
}

void draw_area (view_struct *vs) {
	if (vs->calc_buf)
		(*vs->calc_buf) (vs->external_data, vs->buf8, vs->width, vs->height);

	// This calls an expose_event for vs->area:
	gtk_widget_queue_draw_area ( vs->area, 0, 0, vs->width, vs->height);
}

void draw_area_callb (GtkWidget *wdg, gpointer view_struct_ptr) {
	// Convenience function
	if (view_struct_ptr)
		draw_area((view_struct *) view_struct_ptr);
}

static gint area_configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	view_struct *vs;
	vs = (view_struct *) data;
//	We initialize the drawable:
//	printf("Area_configure_event\n");
	gdk_draw_rectangle(vs->area->window,
		vs->area->style->black_gc,TRUE, 0, 0,
		vs->area->allocation.width,
		vs->area->allocation.height);
	draw_area (vs);

	return TRUE;
}

gint area_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	view_struct *vs;
	vs = (view_struct *) data;
  	if (event->count > 0) {
    		return(TRUE);
  	}
	if (GTK_IS_DRAWING_AREA(vs->area)) {
		if (vs->if_rgb) {
			gdk_draw_rgb_image(vs->area->window,
				vs->area->style->fg_gc[GTK_WIDGET_STATE (vs->area)],
				0,0,vs->width, 
				vs->height,
				GDK_RGB_DITHER_NONE,
				vs->buf8,
				3*vs->width);
		}
		else {
			gdk_draw_gray_image(vs->area->window, 
				vs->area->style->black_gc,
				0, 0, vs->width, 
				vs->height, 
				GDK_RGB_DITHER_NONE, 
				vs->buf8,
				vs->width);
		}
	}
	return TRUE;
}

view_struct *view_struct_new_with_rgb (gint width, gint height, void (*calc_buf) (gpointer, unsigned char*, gint, gint), gpointer external_data, gboolean if_rgb)  {
//	Builds a preview structure, for a 8 bits image
	view_struct *vs;
	vs = (view_struct *) x_malloc(sizeof(view_struct), "view_struct");
//	printf("View_struct_new: %d\n",vs);
	vs->width = width;
	vs->height = height;
	vs->calc_buf = calc_buf;
	vs->external_data = external_data;
	vs->if_rgb = if_rgb;

	if (if_rgb)
		vs->buf8 = (unsigned char *) x_calloc(3*width*height,sizeof(unsigned char), "unsigned char (vs->buf8 in view_area.c)");
	else
		vs->buf8 = (unsigned char *) x_calloc(width*height,sizeof(unsigned char), "unsigned char (vs->buf8 in view_area.c)");

	vs->area = gtk_drawing_area_new();
	gtk_widget_set_size_request( GTK_WIDGET(vs->area), vs->width, vs->height);
	gtk_signal_connect(GTK_OBJECT(vs->area), "configure_event",
		(GtkSignalFunc) area_configure_event, (gpointer) vs);
	gtk_signal_connect(GTK_OBJECT(vs->area),"expose_event",
		(GtkSignalFunc) area_expose_event, (gpointer) vs);
	gtk_widget_show(vs->area);

	vs->viewport = NULL;
	return vs;
}

view_struct *view_struct_new (gint width, gint height, void (*calc_buf) (gpointer, unsigned char*, gint, gint), gpointer external_data)  {

	return view_struct_new_with_rgb (width, height, calc_buf, external_data, FALSE);
}

void view_struct_free (view_struct *vs) {
	if (!vs)
		return;
	if (vs->buf8)
		x_free(vs->buf8);
	x_free(vs);
}

