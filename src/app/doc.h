/* Doc.h - headers for managing classes of documents and individuals documents
 *	Allows generalized processing of multiple documents
 *	Shared between document.c, doctype.c
 *	Gives services to app.c
 *
 * Copyright (C) 2001, 2010 Patrice St-Gelais
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

#ifndef _DOC
#define _DOC 1

#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#include "../utils/menus_n_tools.h"
#include "globals.h"
#include "stack.h"

#ifndef DOC_WRAPPER_TYPE
#define DOC_WRAPPER_TYPE 1001
#endif

#ifndef TYPES_WRAPPER_TYPE
#define TYPES_WRAPPER_TYPE 1002
#endif

#ifndef DOC_TYPE_TYPE
#define DOC_TYPE_TYPE 1003
#endif

/*
Note 2008-01: Hiérarchie document type:

creation_container
	contient
	type_choice_dialog
		contient
		creation_dialog (pour différents types de documents)
*/

typedef struct {
	gint type_id;		// DOC_TYPE_TYPE
	gchar *type;		// Description
	gint nbextensions;	// Number of extensions (.png, .tif...)
	omenu_list_type *extensions;	// Extensions choices, for file seletion dialog
					// The first one is the default
	gchar *def_dir;		// Default directory for this file type (dir of the last saved file)
	GtkWidget *creation_dialog;  	// Widget(s) structure allowing entry 
				// of the current document parameters during creation
				// To be created if NULL
	gpointer widget_list;	// Optional widget list in creation_dialog
				// Allowing, for instance, setting of default values for each widget
	gpointer options_dialogs; // Particular alternate option for each subtypes
				// Could contain any struct we want the documents
				// to inherit from the the document type
				// Generated by options_dialog_new
	GtkWidget *tools_dialog;	// Default dialog for editing the current document type
// The next functions have to be initialized for the 
// particular document type we want to process
	GtkWidget * (*creation_dialog_new) 
			(GtkWidget *window, 
			GtkTooltips *tooltips, 
			gpointer callb_data,
			gpointer widget_list_ptr); // == &this->widget_list  
//	The next function typically generates an empty list of optional widgets
//	Could be NULL
//	Only the functions for the particular document type know how to handle the struct
	gpointer (*options_dialogs_new) (GtkWidget *window, 
			GtkWidget *creation_container,
			GtkTooltips *, gpointer widget_list, 
			gpointer callback_data,
			stack_struct *stack,
			gchar *doc_type,
			gpointer all_docs, // The doc_swap_struct 
			GList *(*get_doc_list_from_type) (gpointer, gchar *) );

	void (*options_dialogs_free) (gpointer options_dialogs);

	GtkWidget * (*tools_dialog_new) 
			(GtkWidget *window, 
			GtkTooltips *tooltips, 
			gpointer options_dialog,
			gpointer callb_data);

	gpointer (*new)  (
			gchar **dirname,
			gchar **filename,
			GtkWidget *doc_window, 
			GtkWidget *creation_container,
			gpointer options,
			gboolean *if_modified,
			gboolean *to_register_in_history,
			gboolean (*registering_fn) (gpointer),
			gpointer registering_data,
			gchar *path_n_file, 	// If given, we try to read the file
			gboolean *fname_tochoose,
			gint category);
					//  Returns a pointer to a doc. struct of the current type
					// Includes the display - created in window
					// If path_n_file is given, try to read the doc from it
					// If a conversion is required when reading,
					// set fname_tochoose to TRUE
	gpointer (*read) (gpointer);  // For future use (functionnality given by NEW)
	void (*free) (gpointer, gboolean free_interface); // Flag is FALSE when we clear the undo stack
	gpointer (*copy_struct) (gpointer from, gpointer to, gint operation);
		// Copy a document struct for undo / redo purposes
		// If "to" is NULL, we allocate and create a new struct
		// If not, we replace target data with source data
		// operation: REDO, UNDO or NONE
	void (*close) (void); // Callback for window destroy signal
	// Note:  close calls free and test for saving modified documents.
	void (*display) (gpointer);  	// Mandatory for generic undo / redo
	void (*save) (gpointer data, gchar *fname);	// Used for save, save_as, save_copy_as
	void (*print) (void);
	void (*undo) (gpointer);		// Non-mandatory
					// Try this function first, if not exists 
					// the default doc_undo apply
	void (*redo) (gpointer);		// See undo
	void (*cut) (void);
	void (*copy) (void);
	void (*paste) (void);
	void (*paste_special) (void);
	void (*run) (void);
	void (*defaults) (gpointer);
	void (*stats) (gpointer);
	void (*commit_creation) (gpointer);  // Any post-creation before-editing process
	gchar *(*get_last_action) (gpointer);	// For managing the undo/redo stack
						// not mandatory
	GtkWidget * (*get_icon) (gpointer, gint size_x, gint size_y); 
		// Get a small icon (typically a drawing area)
		// representing the document
	void (*set_creation_container) (gpointer structure, gpointer container);
	stack_struct *stack; // Global undo / redo stack structure, inherited from the app struct

} doc_type_struct;


//	Types wrapper:  
//	Structure which holds the list of allowed document types for the current
//	application, and the interface required to choose between types
typedef struct {
	gint type_id;		// TYPES_WRAPPER_TYPE
	gint nbtypes;
	GtkWidget *main_window; // Inherited from the calling application
	doc_type_struct *doc_type_list;
	doc_type_struct *default_type; 	// Main occurrence of doc_type_list (normally the first)
	command_item_struct *interface_items;
	GtkWidget *type_choice_dialog; 	// A toolbar or menu for choosing the file type
					// To be embedded in some container
					// If NULL, we generate it;  if not, we show it
	GtkWidget *creation_container;	//  Window (Gimp style interface) or frame (integrated interface) containing the creation dialog
	GtkTooltips *tooltips;	// Tooltips associated with the creation dialog
	gint win_pos_x;		// Creation window position
	gint win_pos_y;
	GtkWidget *tools_container; // Window (Gimp style interface) or frame (integrated interface) containing the edit tools widgets
	gint win_tools_pos_x;
	gint win_tools_pos_y;
	GtkTooltips *tools_tooltips; // Tooltips associated with the tools dialog
 } types_wrapper;

/**********************************************************************/

typedef struct {
	gint type_id;		// DOC_WRAPPER_TYPE
	doc_type_struct *type;	// From types_wrapper->doc_type_list
				// Returned by the interface callback
	gpointer *data;		// Embedded document
	GtkWidget *window;	// Edit / view window for the current doc
				// A callback connected to this window 
				// initializes the "current document" index
				// in the parent structure
				// For integrated interface: it's the main window

	// 2008-01-20: containers and pointers 
	// initialized only for the integrated interface

	// Containers: same as those in types_wrapper
	GtkWidget *creation_container;
	GtkWidget *tools_container;
	// Widgets / structure: same as those in doc_type_struct
	GtkWidget *creation_dialog;  
	GtkWidget *tools_dialog;
	gpointer option_dialogs;

	GtkWidget *container;	// Container for current document window widgets
				// Initialized with a method from doc_type_struct
				// Removed when we change doc. type during doc. creation
	gchar *filename;  // File name without path
	gchar *dir;	// Directory (path)
	GList *history; 	// Undo / redo stack
	gpointer data_to_show;	// Pointer to the last document shown after an undo / redo
	gpointer *commit_data_adr; // Address of the pointer to the data to commit
				// Inherited by the calling application
				// Pointer must be set to NULL if equal to
				// ((doc_wrapper *)this)->data
				// and (doc_wrapper *)this is being destroyed
				
	// TRUE if we are creating this document (so that we can enforce that another
	// document is not "swapped" on top during the creation)
	gboolean creation_mode;
	
	// TRUE if document has been modified and needs to be saved
	gboolean if_modified;
	
	// TRUE if the file name must be chosen by the user
	gboolean fname_tochoose;
	
	// TRUE if the current data must be registered in the history stack (undo/redo)
	gboolean to_register_in_history;
	
	// How many document instances we keep in the undo / redo history
	// Globally inherited, but can be set for each document (depending on it's size!)
	gint max_history;
} doc_wrapper;

/**********************************************************************/

typedef struct {	// Structure for managing multiple documents
	GList *doc_list;  // "doc_wrapper" list
	doc_wrapper* current_doc; 	// Current document
				// Initialized by the document window "focus-in-event"
				// or when creating a new doc
	gpointer *current_doc_data; 	// current_doc->data
			// "bucket" for embedded document callbacks
			// Redundancy ensuring embed. doc. class independency
	doc_wrapper* last_doc;	// Needed for managing cancelling of creation?
				// (remove if not!)
} doc_swap_struct;

/**********************************************************************/

// Prototypes for document type

gint find_doc_type_from_ext(char *ext, types_wrapper *); // Returns doc_type_index
types_wrapper *types_wrapper_new(GtkWidget *main_window, doc_type_struct *, command_item_struct *, gint);
void types_wrapper_free(types_wrapper*);
GtkWidget *tools_container_new(types_wrapper *);
GtkWidget *creation_container_new(types_wrapper *);
void type_choice_dialog_new(types_wrapper *, 
	gpointer toobar_callb_data, 
	gpointer dialog_callb_data,
	stack_struct *stack,
	gpointer doc_swap_struct);
void type_choice_dialog_show(types_wrapper *type);
void initype(types_wrapper *tw,
	GtkWidget *main_window,  
	doc_type_struct *type, 
	doc_wrapper **, 
	gpointer,
	stack_struct *stack,
	gpointer doc_swap_struct);
void create_type_dialogs(types_wrapper *types, 
	doc_type_struct *current_type, 
	gpointer callb_data,
	stack_struct *stack,
	gpointer doc_swap_struct,
	doc_wrapper *dw);
GList *get_doc_list_from_type (gpointer doc_swap_struct, gchar * type);

// Prototypes for document

doc_wrapper *doc_wrapper_new(gchar *filename, gchar *dir, doc_type_struct *);
doc_wrapper *doc_prepare_to_load(gchar *path_n_file, types_wrapper *types);
gboolean doc_new(doc_swap_struct *,
	types_wrapper *,
	gchar *path_n_file, 
	gint sequence, 
	gpointer commit_data_adr,
	GtkWidget *menu,
	GtkWidget *toolbar);
doc_wrapper *doc_read(gchar **path_n_file, types_wrapper *types, 
	doc_swap_struct * dsw, gchar *default_dir);
gint doc_save (doc_wrapper *doc);
gint doc_close (GtkWidget *wdg, GdkEvent *event, gpointer data) ;
void doc_wrapper_free(doc_wrapper* dw, gboolean free_interface);
void doc_wrapper_destroy(doc_swap_struct*);
void doc_stats (doc_wrapper *doc) ;

void place_document_window(doc_swap_struct *dsw, 
	types_wrapper *tw, 
	GtkWidget *window);

doc_swap_struct *doc_swap_new(void);
void doc_swap_free(doc_swap_struct *);

void doc_make_current(doc_swap_struct *dsw, doc_wrapper *new_current_doc) ;

GtkWidget *save_all_dialog_new (GtkWidget *window, doc_swap_struct *dsw);

gint doc_save_question (doc_wrapper *dw);

gint count_modified_documents (doc_swap_struct *dsw);
gint count_documents (doc_swap_struct *dsw);

#endif // _DOC



