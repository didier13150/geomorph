/* erode_dialog.h:  varieties of erosion and cresting dialogs
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

#ifndef _ERODE_DIALOG
#define _ERODE_DIALOG 1

#include "globals.h"
#include "craters.h"
#include "merge_dialog.h"

typedef struct {
	GtkWidget *dialog;
	GtkWidget *accept;
	GtkObject *adj_radius;
	GtkObject *adj_smooth_ribs;
	merge_dialog_struct *merge;
} whimsical_erosion_dialog_struct;

typedef struct {
	GtkWidget *dialog;
	GtkWidget *accept;
	GtkObject *adj_drops;
	GtkObject *adj_threshold;
	GtkObject *adj_strength;
	GtkObject *adj_hardness;
	GtkWidget *refresh; // Checkbox
	GtkWidget *gener_frames; // Checkbox
	GtkObject *adj_interval;
	GtkWidget *old_algorithm; // Checkbox
} rain_erosion_dialog_struct;

typedef struct {
	GtkWidget *dialog;
	GtkWidget *accept;
	GtkObject *adj_qty;
	GtkObject *adj_peak_threshold;
	GtkObject *adj_slope_threshold;
} craters_erosion_dialog_struct;

typedef struct { // Holds for gravity and crests
	GtkWidget *dialog;
	GtkWidget *accept;
	GtkObject *adj_steps;
	GtkObject *adj_threshold;
	merge_dialog_struct *merge;
} gravity_dialog_struct;

typedef struct {
	GtkWidget *dialog;
	GtkWidget *accept;
	GtkObject *adj_steps;
	GtkObject *adj_threshold;
	GtkWidget *wdg_direction;
	merge_dialog_struct *merge;
} oriented_gravity_dialog_struct;

whimsical_erosion_dialog_struct *whimsical_erosion_dialog_struct_new ();
void whimsical_erosion_dialog_struct_free(whimsical_erosion_dialog_struct *);
GtkWidget *whimsical_erosion_dialog_new(gpointer data);
void whimsical_erosion_callb(GtkWidget *wdg, gpointer data) ;

rain_erosion_dialog_struct *rain_erosion_dialog_struct_new ();
void rain_erosion_dialog_struct_free( rain_erosion_dialog_struct *);
GtkWidget *rain_erosion_dialog_new(gpointer data);
void rain_erosion_callb(GtkWidget *wdg, gpointer data) ;

GtkWidget *crests_dialog_new(gpointer data);
void crests_callb(GtkWidget *wdg, gpointer data) ;

craters_erosion_dialog_struct* craters_erosion_dialog_struct_new();
void craters_erosion_dialog_struct_free (craters_erosion_dialog_struct *);
GtkWidget *craters_erosion_dialog_new(gpointer data);
void craters_erosion_callb(GtkWidget *wdg, gpointer data) ;

gravity_dialog_struct *gravity_dialog_struct_new();
void gravity_dialog_struct_free (gravity_dialog_struct *);
GtkWidget *gravity_dialog_new(gpointer data);
void gravity_callb(GtkWidget *wdg, gpointer data) ;

oriented_gravity_dialog_struct *oriented_gravity_dialog_struct_new();
void oriented_gravity_dialog_struct_free (oriented_gravity_dialog_struct *);
GtkWidget *oriented_gravity_dialog_new(gpointer data);
void oriented_gravity_callb(GtkWidget *wdg, gpointer data) ;

#endif // _ERODE_DIALOG
