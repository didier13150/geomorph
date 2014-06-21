/* App.h - headers for the application structure
 * One application = a menu / tool bar + a list of documents
 *
 * Copyright (C) 2010 Patrice St-Gelais
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

#ifndef _APP
#define _APP 1

#include <gtk/gtk.h>
#include "thisappinit.h"
// #include "../utils/menus_n_tools.h"  // included in thisappinit.h
#include "doc.h"
#include "stack.h"
// #include "globals.h"  // included in thisappinit.h

typedef struct {
	gchar *title;
	gchar *default_dir;
	gchar *file_on_cmdline;
	types_wrapper *types;	// Allowable document types
	gint new_doc_count; 	// For coherent new (empty) document numbering
				// Does not decrease when a doc. is closed / destroyed!
	doc_swap_struct *docs;	// Includes a doc. list, ptr to current doc and to last doc.
	GtkWidget *window;
	gboolean ifmenu;
	gboolean iftoolbar;
	GtkWidget *toolbar;
	menu_struct *menu;
	GtkAccelGroup *accel_group;
	GtkTooltips *tooltips;
	gint main_bar_pos_x;
	gint main_bar_pos_y;
	gchar *rc_options_file;
	option_file_type *current_options;
	option_file_type *allowed_options;
	GtkWidget *options_dialog;
	stack_struct *stack;
} app_struct;


/****************	PROTOTYPES **************/

// Initializes the application, including "toolbook" and menus
app_struct *app_new(gchar *title, 
	option_file_type *current_opt, 
	option_file_type *allowed_opt,
	gchar *rc_options_file);

// Shows all widgets in the structure
void app_show(app_struct *);

// Obvious...
gint app_quit(GtkWidget *, gpointer);
// Same as app_quit, connected on the menu bar "delete_event"
gint app_menu_delete(GtkWidget *, GdkEvent *, gpointer);

void app_free(app_struct *);

// Callbacks for toolbar and menus
void create_callb(GtkWidget *, gpointer );
void open_callb(GtkWidget *, gpointer );
void save_callb(GtkWidget *, gpointer );
void save_as_callb(GtkWidget *, gpointer );
void save_copy_as_callb(GtkWidget *,gpointer );
void print_callb(GtkWidget *, gpointer );
void close_callb(GtkWidget *, gpointer );
void quit_callb(GtkWidget *, gpointer );

void undo_callb(GtkWidget *, gpointer );
void redo_callb(GtkWidget *, gpointer );

void copy_callb(GtkWidget *, gpointer );
void cut_callb(GtkWidget *, gpointer );
void paste_callb(GtkWidget *, gpointer );
void paste_special_callb(GtkWidget *, gpointer );

void stats_callb(GtkWidget *, gpointer );
void options_callb(GtkWidget *, gpointer );

void help_callb(GtkWidget *, gpointer );
void about_callb(GtkWidget *, gpointer );

void process_options(option_file_type *options);
void process_all_options(option_file_type *current_options);
#endif // _APP


