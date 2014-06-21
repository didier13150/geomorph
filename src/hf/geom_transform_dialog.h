/* geom_transform_dialog.h:  geometric transformations
 * Processes called in "img_process_dialog.c"
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "globals.h"

#ifndef _GEOM_TRANSFORM_DIALOG
#define _GEOM_TRANSFORM_DIALOG 1

GtkWidget *slide_dialog_new(gpointer data);
void slide_callb(GtkWidget *wdg, gpointer data);
GtkWidget *rotate_dialog_new(gpointer data);
void rotate_callb(GtkWidget *wdg, gpointer data) ;
GtkWidget *stretch_compress_dialog_new(gpointer data);
void stretch_compress_callb(GtkWidget *wdg, gpointer data);
GtkWidget *mirror_vertical_dialog_new(gpointer data) ;
void mirror_vertical_callb(GtkWidget *wdg, gpointer data) ;
GtkWidget *mirror_horizontal_dialog_new(gpointer data) ;
void mirror_horizontal_callb(GtkWidget *wdg, gpointer data) ;

#endif // _GEOM_TRANSFORM_DIALOG


