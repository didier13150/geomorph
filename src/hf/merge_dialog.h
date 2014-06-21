 /* merge_dialog.h - Merge utility headers (merging source - result)
 *
 * Copyright (C) 2002, 2010 Patrice St-Gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _MERGE_DIALOG
#define _MERGE_DIALOG 1
#include "globals.h"
#include "merge.h"

typedef struct {
	GtkWidget *merge_dialog;	// The dialog displayed in the tools window (the frame)
	GtkWidget *main_box;	// The box inside the frame (allowing show / hide)
	GtkWidget *op_toolbar;
	GtkObject *adj_mix;		// % of each image mixed 
					// (-100 = 100% source, 100 = 100 % result)
	GtkObject *adj_source_offset;	// Luminosity offset, from -100 to 100
	GtkObject *adj_result_offset;
	GtkObject *adj_x_translate;	// "result" geometric translation, from -100 to 100
	GtkObject *adj_y_translate;
	GtkObject *adj_hardness;	// For altitude merge only (added 2010-11)
	merge_struct *content;
} merge_dialog_struct;

merge_dialog_struct * merge_dialog_struct_new (merge_struct *mrg);

GtkWidget *merge_dialog_new(merge_dialog_struct *mrg_str, gboolean hide_control,
	GtkWidget *parent_window, gchar *frame_title, 
	gchar *source_title, gchar *result_title, gchar *mix_title);
	
void merge_set_op (merge_dialog_struct *mrg_str, gint merge_op);
void merge_set_mix (merge_dialog_struct *mrg_str, gint mix);

#endif // _MERGE_DIALOG
