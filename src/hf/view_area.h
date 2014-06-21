/* view_area.h - headers for standard view area
 *
 * Copyright (C) 2003, 2007 Patrice St-Gelais
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

#ifndef _VIEW_AREA
#define _VIEW_AREA 1

#include <gtk/gtk.h>

typedef struct {
//	Simple struct for a view / preview area
	GtkWidget *area;
	GtkWidget *viewport; // used only if the area is scrollable, for rescaling
	gint width;
	gint height;
	unsigned char *buf8;
//	The function used in the area callbacks, for recalculating the image
//	"buf8" is the output, it is normally equal to this->buf8
//	"width" and "height" are this->width and this->height
	void (*calc_buf) (gpointer external_data, unsigned char *buf8, gint width, gint height);
//	Embedded data, to apply on calc_buf
	gpointer external_data;
	gboolean if_rgb; // TRUE if buf8 is a RGB buffer
} view_struct;

view_struct *view_struct_new (
	gint width, 
	gint height, 
	void (*calc_buf) (gpointer, unsigned char *, gint, gint), 
	gpointer external_data);
view_struct *view_struct_new_with_rgb (
	gint width, 
	gint height, 
	void (*calc_buf) (gpointer, unsigned char *, gint, gint), 
	gpointer external_data,
	gboolean if_rgb);
void view_struct_free (view_struct * vs);
void draw_area (view_struct *vs);
void draw_area_callb (GtkWidget *wdg, gpointer view_struct_ptr);
void resize_area (view_struct *vs, gint new_width, gint new_height);

#endif // VIEW_AREA

