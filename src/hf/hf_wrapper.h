/* hf_wrapper.h - headers for the height field wrapper class
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
#ifndef _HF_WRAPPER
#define _HF_WRAPPER 1

#include <gtk/gtk.h>
#include "globals.h"
#include "hf.h"
#include "../fourier/fourier.h"
#include "gl_preview.h"
#include "dialog_utilities.h"
#include "hf_dialog_options.h"
#include "../utils/scale_buttons.h"

//	HF types:  pen (for painting) or main (background)
#ifndef HFPEN
	#define HFPEN 1
#endif
#ifndef HFMAIN
	#define HFMAIN 2
#endif
//	This one is for progressive surface addition
#ifndef HFPRIMITIVE
	#define HFPRIMITIVE 3
#endif

//	For adding noise by merging a simple subdivision with another HF
#ifndef HFNOISE
	#define HFNOISE 4
#endif

//	Scale buttons labels
#ifndef SCALE_12_5
	#define SCALE_12_5 "12,5"
#endif
#ifndef SCALE_25
	#define SCALE_25 "25"
#endif
#ifndef SCALE_50
	#define SCALE_50 "50"
#endif
#ifndef SCALE_100
	#define SCALE_100 "100"
#endif
#ifndef SCALE_200
	#define SCALE_200 "200"
#endif
#ifndef SCALE_400
	#define SCALE_400 "400"
#endif

#ifndef SCALE_PAD
#define SCALE_PAD 25
#endif

#ifndef realloc_buf8

#define realloc_buf8(hfw) hfw->hf_buf8=(unsigned char *)x_realloc(hfw->hf_buf8,RIGHT_SHIFT(hfw->hf_struct->max_x,hfw->display_scale)*RIGHT_SHIFT(hfw->hf_struct->max_y,hfw->display_scale)*(hfw->if_rgb?3:1),"unsigned char (realloc_buf8)")

#define alloc_buf8(hfw) hfw->hf_buf8=(unsigned char *)x_malloc(RIGHT_SHIFT(hfw->hf_struct->max_x,hfw->display_scale)*RIGHT_SHIFT(hfw->hf_struct->max_y,hfw->display_scale)*(hfw->if_rgb?3:1),"unsigned char (alloc_buf8)")

#endif

typedef struct {
	gchar **filename;			// Inherited from the embedding document structure
					// Needed for manipulating list of open HF
	gchar **dirname;			// Inherited from the embedding document structure
	GtkWidget *window;		// HF main window
	GtkWidget *area;
	GtkWidget *scrolled_window;	// Scrolled window for the image
	GtkWidget *draw_area_frame;	// Frame containing the HF pixmap
	GtkWidget *draw_area_viewport;	// Viewport in the scrolled window
	guint exp_handler;		// Connect handler to "area" expose event
	gint area_size;			// Size of area (supposed to be square, avoid calculation)
	scale_buttons_struct *sbs;
	gl_preview_struct *gl_preview;
	control_line_struct *control_line;
	GtkWidget *creation_container;	// Where we put the hf options during creation
					// Inherited from the doc_type_struct
	GtkWidget *tiling_wdg;		// A checkbox connected to *tiling_flag_ptr
	gboolean *tiling_ptr;
	gint type;			// HFPEN, HFMAIN, HFPRIMITIVE
	hf_struct_type *hf_struct;		// Contains the hf data
	hf_options_struct *hf_options;  // Redundancy of optional parts of the creation dialog
				// The original is in doc_type_struct as a gpointer
	gboolean mouse_dragging;	// Used for mouse tracking
	GtkWidget *mouse_position;	// A label containing (mousex,mousey)
	gboolean creation_mode;		// TRUE if creation not commited
	gboolean if_calculated;		// For managing recomputing from options
	gboolean apply_filter;		// TRUE:  we must apply the current filter
	gboolean *if_modified;		// "Bucket" inherited from the document wrapper
					// in the calling application structure.
					// Set to TRUE after any update
	gboolean (*registering_fn) (gpointer);
	gpointer registering_data;
	gboolean *to_register_in_history;	// Flag used by the calling app for managing
					// the undo-redo stack;
					// must be set to TRUE after any significant update
	gchar *last_action;		// Last modif, info for undo / redo stack
	unsigned char *hf_buf8;  // 8 bits copy, for displaying HF
				// Scaled depending on display_scale
	gboolean if_rgb;	// TRUE if hf_buf8 is a RGB buffer, for instance for
				// combining the selection and the output buffer
	gint display_scale;  	// 12,5%:  3; 25%: 2; 50%: 1; 100 %: 0; 200 %: -1; 400 %: -2
				// allows using the >> operator for scaling
//	GSList *scale_group;		// List of scale buttons (widgets)
					// for building menus, etc.   Inherited.
	gboolean size_change_request;	// For managing expose and configure events
					// after a size change request
	render_struct *render_str; // Povray launch management
	void (*dependent_process) (gpointer); // Process to execute after generating the height field
	gpointer dependent_process_data;
} hf_wrapper_struct;

gpointer hf_wrapper_new (
		gchar **dirname, gchar **filename, 
		GtkWidget *window, GtkWidget *creation_dialog,
		gpointer callback_data,
		gboolean *if_modified, gboolean *to_register_in_history,
		gboolean (*registering_fn) (gpointer),
		gpointer registering_data,
		gchar *path_n_file, 
		gboolean *fname_tochoose,
		gint hf_category); // HFMAIN, HFPEN, HFPRIMITIVE, HFNOISE
void hf_wrapper_free(gpointer, gboolean);
gpointer hf_wrapper_copy(gpointer hfw_from, gpointer hfw_to, gint operation);
void hf_wrapper_display(gpointer hfw_struct);
void draw_hf (hf_wrapper_struct *) ;
void draw_hf_ptr (gpointer hfw_ptr_ptr);
void draw_hf_partial (hf_wrapper_struct *hfw,
		gint from_x, gint to_x, gint from_y, gint to_y,
		gboolean update_gl_area) ;
void gener_hf (hf_wrapper_struct *hfw);
gchar* hf_get_last_action (gpointer);

gint hf_default_size();
void hf_wrapper_save (gpointer data, gchar *path_n_file);
void hf_stats (gpointer hf_wrapper);

GtkWidget * hf_get_icon (gpointer hfw, gint size_x, gint size_y);

void set_widget_display_scale (hf_wrapper_struct *hfw);

void set_watch_cursor (hf_wrapper_struct *hfw);
void unset_watch_cursor (hf_wrapper_struct *hfw);

// Open GL preview refresh management

gint gl_no_refresh_callb (GtkWidget *wdg, GdkEventButton *event, gpointer hfw_ptr_ptr);
gint gl_restore_refresh_callb (GtkWidget *wdg, GdkEventButton *event, gpointer hfw_ptr_ptr);
void 	gl_preview_optimize (GtkWidget *scale, gpointer data);

// Undo / redo stack management
// Should it be a distinct class in the future?

void record_action(hf_wrapper_struct *, gchar *action);
void begin_pending_record(hf_wrapper_struct *, 
	gchar *action,
	gpointer commit_fn,
	gpointer reset_fn);
void commit_pending_record(hf_wrapper_struct *);
void accept_fn (hf_wrapper_struct *hfw);
void reset_fn (hf_wrapper_struct *hfw);
void accept_draw_fn (hf_wrapper_struct *hfw);
void reset_draw_fn (hf_wrapper_struct *hfw);
void accept_fault_fn (hf_wrapper_struct *hfw);
void hf_wrapper_commit_or_reset (hf_wrapper_struct *hfw);


// Misc (eventually check consistency!)

void set_frq_vector(hf_wrapper_struct *hfw,gint *frq, gint nbrfrq);	// For setting frequencies in subdiv2

gint get_display_scale(gint hf_size);

void set_dependent_process (hf_wrapper_struct *hfw, void (*dependent_process) (gpointer), gpointer dependent_process_data);

void display_16_to_8(hf_wrapper_struct *);
void display_16_to_8_partial (hf_wrapper_struct *hf, gint from_x, gint to_x, gint from_y, gint to_y);

#endif
