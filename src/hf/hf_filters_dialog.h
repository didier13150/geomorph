/* hf_filters_dialog.h - headers for filters dialog
 *
 * Copyright (C) 2001 Patrice St-Gelais
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

#ifndef _FILTERS_DIALOG
#define _FILTERS_DIALOG 1

#include "globals.h"
#include "hf_filters.h"
#include "../utils/menus_n_tools.h"

// Filter indexes (to use with set_filter_type)
#ifndef NO_FILTER
	#define NO_FILTER 0
#endif
#ifndef WIDE_FILTER
	#define WIDE_FILTER 1
#endif
#ifndef MEDIUM_FILTER_1
	#define MEDIUM_FILTER_1 2
#endif
#ifndef MEDIUM_FILTER_2
	#define MEDIUM_FILTER_2 3
#endif
#ifndef SHARP_FILTER
	#define SHARP_FILTER 4
#endif
#ifndef GAUSS_FILTER
	#define GAUSS_FILTER 5
#endif
#ifndef IMAGE_FILTER
	#define IMAGE_FILTER 6
#endif

typedef struct {
	GtkWidget *listbox; // List of currently open documents
	GtkWidget *no_doc_label; // Label telling that no document is open
	GtkWidget *parent; // Box containing _listbox and _no_doc
} img_listbox_struct;

typedef struct {
	GtkWidget *filter_dialog;	// The main filter dialog
	GtkWidget *filter_vbox;		// A container to add new widgets
	filter_struct *current_filter;
//	We keep the filter choice toolbar for future use (dynamically adding image filters!)
	GtkWidget *filter_toolbar;
	GtkWidget *revert_button;
	gboolean revert_filter;  // If true, we apply 1-filter_value instead of filter_value
	GtkObject *adj_level;
	gfloat filter_level; // From 0% to 100 % (0 to 1)
			// Images with and without filter are averaged in this ratio
			// 0:  no filter;  1: filter fully applied
	GtkWidget *merge_toolbar;
	gint merge_oper;	// Values MULTIPLY, ADD, SUBTRACT #defined in hf_filters.h
	GtkWidget *gauss_dialog;
	gauss_opt *gauss_options;
	GtkWidget *img_filter_dialog; // The whole image filter dialog (a frame)
	img_listbox_struct *img_listbox;
	gpointer (*apply_filter) (gpointer);
	
} filter_dialog_struct;

void nofilter_callb(GtkWidget *wdg, gpointer data) ;
void widefilter_callb(GtkWidget *wdg, gpointer data);
void mediumfilter1_callb(GtkWidget *wdg, gpointer data) ;
void mediumfilter2_callb(GtkWidget *wdg, gpointer data);
void sharpfilter_callb(GtkWidget *wdg, gpointer data) ;
void gaussfilter_callb(GtkWidget *wdg, gpointer data) ;
void imagefilter_callb(GtkWidget *wdg, gpointer data) ;
GtkWidget *gauss_dialog_new(GtkWidget *reference_window, gpointer hf_wrapper_adr);
filter_dialog_struct *filter_dialog_new(GtkWidget *, GtkTooltips*, 
	gpointer hf_wrapper_adr);
void filter_dialog_free (filter_dialog_struct *fds);
filter_dialog_struct *filter_toolbar_new(GtkWidget *, GtkTooltips*, 
	gpointer hf_wrapper_adr);
void set_filter_defaults(filter_dialog_struct *fds);
void set_filter_merge(filter_dialog_struct *fds, gint opr);

img_listbox_struct* img_listbox_new (GtkWidget *parent_box);
void img_listbox_free(img_listbox_struct*);
void refresh_img_listbox(img_listbox_struct *fds, gpointer data, 
	void (*select_callb) (GtkWidget *, gpointer));
	
void set_filter_type (filter_dialog_struct *fds, gint filter_index);

#endif
