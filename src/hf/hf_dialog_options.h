/* hf_dialog_options.h - headers for the "melting pot" structure
 * 		controlling the HF interface relationships
 *		Needs some rework in the future for clarity sake
 *		Separated from hf_wrapper.c/.h on 2006-01-02
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

//	This file includes the prototypes for the dialog creation functions
//	for each HF subtypes
//	These functions are defined in other files than hf_dialog.c

#ifndef _HF_DIALOG_OPTIONS
#define _HF_DIALOG_OPTIONS 1

#include <gtk/gtk.h>
#include "globals.h"
#include "hf.h"
#include "hf_creation_dialog.h"
#include "hf_filters.h"
#include "hf_filters_dialog.h"
#include "subdiv1.h"
#include "subdiv2.h"
#include "uniform.h"
#include "surfadd.h"
#include "img_process_dialog.h"
#include "draw_tools_dialog.h"
#include "gl_preview.h"
#include "../fourier/fourier.h"
#include "../fourier/fourier_dialog.h"
#include "../app/stack.h"

typedef struct {

//	HF creation and HF tools options

//	A few options inherited from the calling application
//	for building new widgets, for instance toolbars

//	HF CREATION OPTIONS

	hf_creation_wdg_struct *creation_widgets;
	GtkWidget *creation_container;
	GtkTooltips *tooltips;
	gpointer hfw_ptr_ptr;
	GtkWidget *current_dialog;	// Current creation subdialog
	gchar *last_creation_action;	// String describing the current creation dialog
						// It gives the last action performed on a new document,
						// recorded in the undo / redo stack
	gint hf_size;	// Result of the size widget, in the creation dialog
	
//	Some generation algorithm - related options
	GtkWidget *uniform_dialog;
	hf_type *uniform_value;
	gboolean flag_uniform_done;  // True if the HF has been committed with the filter
				// Avoid recomputing the HF when drawing (faster!)
	GtkWidget *subdiv1_dialog;
	subdiv1_opt *subdiv1_options;
	GtkWidget *subdiv2_dialog;
	GtkObject *freq_subdiv2_adj[12];	// Adjustments for frequency control, from [0]=2 to [11]=4096
	gboolean subdiv2_direct_upd;
	subdiv2_opt *subdiv2_options;
//	gpointer subdiv2_options;
	GtkWidget *surfadd_dialog;
	surfadd_opt *surfadd_options;
	gpointer primit_surf;	// HF wrapper for primitive surface (SURFADD)
	GtkWidget *waves_dialog;
//	waves_opt *waves_options;
	gpointer waves_options;
	void (*current_callb) (GtkWidget *wdg, gpointer data);  // For executing default options
	gpointer current_options;	// Can be *uniform_value, subdiv1-2_options, sufadd_options....
	GtkObject *freq_control_adj[12];	// Adjustments for frequency control, from [0]=2 to [11]=4096
	
	// (Re)calculates HF during the creation process
	// Varies with kind of HF algorithm
	gpointer (*current_calculation) (gpointer hf, gpointer options); 
	
//	Options shared by the whole HF dialog subsystem

//	Shape filters defined once in the application
	dist_matrix_struct *dist_matrix;
	filter_struct *wide_filter;
	filter_struct *medium_filter1;
	filter_struct *medium_filter2;
	filter_struct *sharp_filter;
	filter_struct *gauss_filter;
	filter_struct *image_filter;
//	Filter dialog struct for creation (there is another one in ->img)
	filter_dialog_struct *creation_fd_struct;
//	Current filter struct we are processing (= ->creation_fd_struct or ->img->filter_dialog)
	filter_dialog_struct *fd_struct;

//	List of open HF (required by the filter dialog, and maybe others in the future)
	GList *doc_list;
	render_struct *render_str; // Povray launch management
	
	gdouble *gauss_list[GAUSS_LIST_LENGTH];	// Cache of normalized gaussian vectors, for drawing
	gl_defaults_struct *gl_defaults;	// For OpenGL preview display
	
	stack_struct *stack;	// A pointer to an undo-redo structure initialized at the global application level
	gboolean need_tmp_buf;		// TRUE if we need to initialize hf_struct->tmp_buf after an undo

// 	HF TOOLS OPTIONS
	
	GtkWidget *tools_window;
	GtkWidget *tools_dialog;		// Where we put the tools dialog
					// Inherited from the doc_type struct
					// A vbox to which we can add / subtract widgets
	GtkWidget *current_tools;		// Current tools dialog, to show / hide
	gint context;			// Current context (dialog ID) - most important for drawing
	
	draw_tools_struct *draw;

	GdkCursor *current_cursor; // Current cursor for the document window (pen / arrow)
	GdkCursor *default_cursor;
	GdkCursor *pencil_cursor;
	GdkCursor *watch_cursor;
	GdkCursor *fleur_cursor;
	img_dialog_struct *img;		// Dialogs and structures for image transformations
	fourier_dialog_struct *fourier;

	gpointer all_docs;		// Structure inherited from the calling application,
					// containing all the open documents
	gchar *doc_type;		// Name of the current document type,
					// inherited from the calling application
	GList *(*get_doc_list_from_type) (gpointer all_docs, gchar* doc_type);
					// Extracts all the documents of the current type,
} hf_options_struct;


/*****************************************************************************/

gpointer hf_options_dialog_new (GtkWidget*, 
	GtkWidget *, 
	GtkTooltips*,
	gpointer widget_list, 
	gpointer callback_data,
	stack_struct *stack,
	gchar *doc_type,
	gpointer all_docs,
	GList *(*get_doc_list_from_type) (gpointer all_docs, gchar *doc_type));

void hf_options_dialog_free(gpointer);

//	Specific processing after committing or resetting a change
//	To be fed to the global stack_struct
void hf_specific_commit_or_reset (stack_struct *);

// Transitional function - 2010.12.05 - to be replaced by a "stack struct" function
void hf_commit_or_reset (hf_options_struct *hfo);

void set_creation_container (hf_options_struct *hfo, GtkWidget *creation_container);
void hf_dialog_defaults (hf_options_struct *hfo);

void gener_hf_from_ptr_ptr (gpointer hfw_ptr_ptr) ;

#endif // _HF_DIALOG_OPTIONS
