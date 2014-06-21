/* App.c - functions for processing the app. struct
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../icons/create.xpm"
#include "../icons/open.xpm"
#include "../icons/save.xpm"
#include "../icons/save_as.xpm"
#include "../icons/save_copy_as.xpm"
// #include "../icons/print.xpm"
#include "../icons/close.xpm"
#include "../icons/quit.xpm"
#include "../icons/undo.xpm"
#include "../icons/redo.xpm"
// #include "../icons/copy.xpm"
// #include "../icons/cut.xpm"
// #include "../icons/paste.xpm"
// #include "../icons/paste_special.xpm"
#include "../icons/stats.xpm"
#include "../icons/settings.xpm"
#include "../icons/help.xpm"
#include "../icons/about.xpm"

#include "app.h"
#include "doc.h"

gint ok_create_callb(GtkWidget *, gpointer);
gint cancel_create_callb(GtkWidget *,gpointer);

#define NBCOMMANDS 16
command_item_struct commands[NBCOMMANDS] = {
{ "File", "New",	"Create a new document",	'N', (gchar **) create_xpm, GDK_LEFT_PTR, create_callb, NULL, NULL,FALSE },
{ "File", "Open", "Open a file",'O', (gchar **) open_xpm, GDK_LEFT_PTR, open_callb,NULL, NULL,FALSE },
{ "File", "Save", "Save the current document", 'S', (gchar **) save_xpm, GDK_LEFT_PTR, save_callb,NULL, NULL,FALSE },
{ "File", "Save as...", "Save under a new name", 0, (gchar **) save_as_xpm, GDK_LEFT_PTR, save_as_callb,NULL, NULL, FALSE },
{ "File", "Copy as...", "Copy under a new name", 0, (gchar **) save_copy_as_xpm, GDK_LEFT_PTR, save_copy_as_callb,NULL, NULL, FALSE },
// "File", "Print", PRINT_TOOLTIP, 'P', (gchar **) print_xpm, GDK_LEFT_PTR, print_callb,NULL, NULL, FALSE },
{ "File", NULL, NULL, 0, NULL, 0, NULL,NULL, NULL, FALSE },	// Separator
{ "File", "Close", "Close the current document", 'W', (gchar **) close_xpm, GDK_LEFT_PTR, close_callb,NULL, NULL, FALSE },
{ "File", "Quit", 	"Quit application",	'Q', (gchar **) quit_xpm, GDK_LEFT_PTR, quit_callb,NULL, NULL, FALSE },
{ "Edit", 	  "Undo", "Undo",'Z', (gchar **) undo_xpm, GDK_LEFT_PTR, undo_callb,NULL, NULL, FALSE },
{ "Edit", 	  "Redo", "Redo", 'Y', (gchar **) redo_xpm, GDK_LEFT_PTR, redo_callb,NULL, NULL, FALSE },
// { "Edit", 	  NULL,	NULL, 0, NULL, 0, NULL,NULL, NULL, FALSE },	// Separator
// { "Edit", 	  "Copy", "Copy", 'C', (gchar **) copy_xpm, GDK_LEFT_PTR, copy_callb,NULL, NULL, FALSE },
// { "Edit", 	  "Cut", "Cut", 'X', (gchar **) cut_xpm, GDK_LEFT_PTR, cut_callb,NULL, NULL, FALSE },
// { "Edit", 	  "Paste", "Paste", 'V',(gchar **)  paste_xpm, GDK_LEFT_PTR, paste_callb,NULL, NULL, FALSE },
// { "Edit", 	  "Paste special", "Paste special (merge...)", 'T', (gchar **) paste_special_xpm, GDK_LEFT_PTR, paste_special_callb,NULL, NULL, FALSE },
{ "Tools",  NULL, NULL, 0, NULL, 0, NULL,NULL, NULL, FALSE },	// Separator
{ "Tools",  "Statistics", "Information and statistics about the current document", 0, (gchar **) stats_xpm, GDK_LEFT_PTR, stats_callb,NULL, NULL, FALSE },
{ "Tools",  NULL, NULL, 0, NULL, 0, NULL,	NULL, NULL, FALSE }, // Separator
{ "Tools",  "Options", "Options at the application launch", 0, (gchar **) settings_xpm, GDK_LEFT_PTR, options_callb,NULL, NULL, FALSE },
{ "?", 	  "Guide", "User guide", 'H', (gchar **) help_xpm, GDK_LEFT_PTR, help_callb, NULL, NULL, FALSE },
{ "?", 	  "About...", "Application and author", 0, (gchar **)  about_xpm, GDK_LEFT_PTR, about_callb, NULL, NULL, FALSE }
};


//	Global variables coming from the RC file or defined in globals.h
gint DEF_PAD,MAIN_BAR_X, MAIN_BAR_Y, MAX_HISTORY;
gint TOOLS_X, TOOLS_Y, CREATION_X, CREATION_Y, DISPLAY_DOC_OFFSET;
guint DEFAULT_SEED;
gchar *DEF_DIR=NULL, *DOC_READER, *DOC_DIR;
gboolean INTEGRATED_INTERFACE, MENU_IN_DOC_WINDOW, ICONS_IN_DOC_WINDOW;

void process_options(option_file_type *options) {
//	Process some generic defaults from the RC file
//	DEFAULT_PAD, MAXIMUM_HISTORY... are #defined in globals.h
//	DEF_PAD, MAX_HISTORY are declared "extern" in globals.h as well
	gchar *buf, *current_dir, *home;
	gboolean invalid;

	DEF_PAD = DEFAULT_PAD;
	if ( (buf = get_option(options,"interface","pad"))) {
		if (is_integer(buf)) {
			DEF_PAD = (gint) atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - interface:pad)");
		sprintf(buf,"%-4d", DEF_PAD);
		put_option(options,"interface","pad",buf);
	}
	
	if ((buf = get_option(options,"files","def_dir")) && strlen(buf))
		DEF_DIR = buf;
	else
		if (!DEF_DIR) {
		//	DEF_DIR is supposed to be initialized by main.c
		//	If not, it defaults to $HOME
			DEF_DIR = getenv("HOME");
			put_option(options,"files","def_dir",DEF_DIR);
		}
	// DEF_DIR should have a full path.
	// If not (like in Geomorph version prior to 0.50),
	// we try to build it
	if (DEF_DIR[0] != FILESEP) {
		// First try to test for a subdirectory
		// in the current one, then in the home directory
		current_dir = (gchar *) get_current_dir_name();
		// current_dir has no FILESEP at the end
		buf = (gchar *) x_malloc(strlen(current_dir)+2+strlen(DEF_DIR), "gchar (buf - DEF_DIR)");
		sprintf(buf,"%s%c%s",current_dir,FILESEP,DEF_DIR);
//		printf("CURRENT_DIR: %s; BUF: %s\n",current_dir,buf);
		if (directory_exists(buf))
			DEF_DIR = buf;
		else {
			// We try with the home directory
			x_free(buf);
			home = getenv("HOME");
			buf = (gchar *) x_malloc(strlen(home)+2+strlen(DEF_DIR), "gchar (buf - DEF_DIR)");
			sprintf(buf,"%s%c%s", home, FILESEP, DEF_DIR);
			if (directory_exists(buf))
				DEF_DIR = buf;
			else {
				DEF_DIR = home;
				x_free(buf);
			}
//			printf("DEF_DIR après BUF: %s\n",DEF_DIR);
		}
	}

	MAX_HISTORY = MAXIMUM_HISTORY;
	if ((buf = get_option(options,"application","max_history"))) {
		if (is_integer(buf)) {
			MAX_HISTORY = atoi(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - put_option)");
		sprintf(buf,"%-4d",MAX_HISTORY);
		put_option(options,"application","max_history",buf);
	}
	
	DEFAULT_SEED = rand();
	if ((buf = get_option(options,"application","default_seed"))) {
//	On some systems, is_integer does not recognizes Hex numbers (0x01b5...)
//		if (is_integer(buf))
			DEFAULT_SEED = (guint) strtoul(buf,NULL,0);
	}
	
	CREATION_X = CREATION_WINDOW_X;
	if ((buf = get_option(options,"interface","creation_window_x"))) {
		if (is_integer(buf))
			CREATION_X = atol(buf);
	}
	
	CREATION_Y = CREATION_WINDOW_Y;	
	if ((buf = get_option(options,"interface","creation_window_y"))) {
		if (is_integer(buf))
			CREATION_Y = atol(buf);
	}
	
	TOOLS_X = TOOLS_WINDOW_X;
	if ((buf = get_option(options,"interface","tools_window_x"))) {
		if (is_integer(buf))
			TOOLS_X = atol(buf);
	}
	
	TOOLS_Y = TOOLS_WINDOW_Y;
	if ((buf = get_option(options,"interface","tools_window_y"))) {
		if (is_integer(buf))
			TOOLS_Y = atol(buf);
	}
	
	MAIN_BAR_X = MAIN_MENU_BAR_X;
	if ((buf = get_option(options,"interface","main_bar_x"))) {
		if (is_integer(buf))
			MAIN_BAR_X = atol(buf);
	}
	
	MAIN_BAR_Y = MAIN_MENU_BAR_Y;
	if ((buf = get_option(options,"interface","main_bar_y"))) {
		if (is_integer(buf))
			MAIN_BAR_Y = atol(buf);
	}
	
	DISPLAY_DOC_OFFSET = DEFAULT_DOC_OFFSET;
	if ((buf = get_option(options,"interface","display_doc_offset"))) {
		if (is_integer(buf))
			DISPLAY_DOC_OFFSET = atol(buf);
	}
	
	if ((buf = get_option(options,"files","doc_dir")) && strlen(buf))
		DOC_DIR = buf;
	else {
		DOC_DIR = DOCUMENT_DIR;
		put_option(options,"files","doc_dir",DOC_DIR);
	}
		
	if ((buf = get_option(options,"files","doc_reader")) && strlen(buf))
		DOC_READER = buf;
	else {
		DOC_READER = DEFAULT_DOC_READER;
		put_option(options,"files","doc_reader",DOC_READER);
	}
	
}

void process_all_options(option_file_type *options) {
	if (options) {
		process_options(options);
		process_specific_options(options);  // Function given by thisappinit.h
	}
}

void check_integrated_interface (option_file_type *options) {

	gchar *buf;
	// The INTEGRATED option cannot be executed in process_options,
	// because it shouldn't be reinitialized when geomorphrc
	// is modified without restarting the application
	if ((buf = get_option(options,"interface","interface_style"))){
		if (!strcmp(buf,"Integrated"))
			INTEGRATED_INTERFACE = TRUE;
		else
			INTEGRATED_INTERFACE = FALSE;
	}
	else {
		INTEGRATED_INTERFACE = DEF_INTEGRATED_INTERFACE;
		if (INTEGRATED_INTERFACE)
			put_option (options, "interface","interface_style","Integrated");
		else
			put_option (options,"interface","interface_style","Gimp style");
	}
	
	// Menu mandatory with integrated interface
	if ((buf = get_option(options,"interface","menu_in_doc_window"))){
		if (!strcmp(buf,"TRUE"))
			MENU_IN_DOC_WINDOW = TRUE;
		else
			MENU_IN_DOC_WINDOW = INTEGRATED_INTERFACE;
	}
	else {
		MENU_IN_DOC_WINDOW = INTEGRATED_INTERFACE;
		if (MENU_IN_DOC_WINDOW)
			put_option (options,"interface","menu_in_doc_window","TRUE");
		else
			put_option (options,"interface","menu_in_doc_window","FALSE");
	}
	
	// When the ICONS in the main menu option is not specified, 
	// it defaults to TRUE for the integrated interface
	if ((buf = get_option(options,"interface","icons_in_doc_window"))){
		if (!strcmp(buf,"TRUE"))
			ICONS_IN_DOC_WINDOW = TRUE;
		else
			ICONS_IN_DOC_WINDOW = INTEGRATED_INTERFACE;
	}
	else {
		ICONS_IN_DOC_WINDOW = INTEGRATED_INTERFACE;
		if (ICONS_IN_DOC_WINDOW)
			put_option (options,"interface", "icons_in_doc_window", "TRUE");
		else
			put_option (options,"interface", "icons_in_doc_window", "FALSE");
	}
}


app_struct *app_new(	gchar *title, 
			option_file_type *current_opt, 
			option_file_type *allowed_opt,
			gchar *options_file) {
// Initializes application

	app_struct *app;
	app = (app_struct *) x_malloc(sizeof(app_struct), "app_struct");

	app->main_bar_pos_x = MAIN_BAR_X*gdk_screen_width()/100;
	app->main_bar_pos_y = MAIN_BAR_Y*gdk_screen_height()/100;

	app->default_dir = (gchar *) x_malloc(strlen(HF_DIR)+1, "gchar (HF_DIR)");
	strcpy(app->default_dir, HF_DIR);
	app->file_on_cmdline = NULL;
	app->title = title;
	app->docs = doc_swap_new();
	app->new_doc_count = 0;

	app->rc_options_file = options_file;
	app->allowed_options = allowed_opt;
	app->current_options = current_opt;
	app->options_dialog = NULL;
	app->stack = stack_struct_new (NULL);

//	printf("APP: %p;  APP->STACK: %p\n",app, app->stack);

	check_integrated_interface (allowed_opt);

        app->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_realize(GTK_WIDGET(app->window));

	gtk_widget_set_uposition(GTK_WIDGET(app->window),
		app->main_bar_pos_x, app->main_bar_pos_y);

	app->types = instantiate_types(app->window);

	if (!INTEGRATED_INTERFACE)
		gtk_window_set_resizable(GTK_WINDOW(app->window),FALSE);
	
       	gtk_signal_connect (GTK_OBJECT (app->window), "delete_event",
                        GTK_SIGNAL_FUNC(app_menu_delete),
                        (gpointer) app);
        gtk_window_set_title (GTK_WINDOW (app->window), app->title);
        gtk_container_border_width (GTK_CONTAINER(app->window),DEF_PAD);

	app->accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group (GTK_WINDOW(app->window), app->accel_group);

	app->tooltips = gtk_tooltips_new();

	if ((!INTEGRATED_INTERFACE) || (!MENU_IN_DOC_WINDOW))
		app->menu = menu_new(NBCOMMANDS,commands, app->accel_group, (gpointer) app);
	else
		app->menu = NULL;

	if ((!INTEGRATED_INTERFACE) || (!ICONS_IN_DOC_WINDOW))
		app->toolbar = standard_toolbar_new(NBCOMMANDS,
			commands,
			app->tooltips,
			app->window,
			(gpointer) app,
			GTK_ORIENTATION_HORIZONTAL,
			GTK_TOOLBAR_ICONS,
			FALSE);
	else
		app->toolbar = NULL;

	if (!INTEGRATED_INTERFACE) {
		creation_container_new(app->types);
		tools_container_new(app->types);
		// We need the accelerators for the tools window, 
		// but not for the creation window, which is modal
		if (GTK_IS_WINDOW(app->types->tools_container))
			gtk_window_add_accel_group (GTK_WINDOW(app->types->tools_container), app->accel_group);
	}
	return(app);
}

void app_show(app_struct *app) {

	GtkWidget *mainbox;

//	Eventually shows doc list
//	Eventually shows version list (undo / redo)
	
	if ((!INTEGRATED_INTERFACE) || ((!MENU_IN_DOC_WINDOW) && (!ICONS_IN_DOC_WINDOW))) {
		mainbox = gtk_vbox_new(FALSE,DEF_PAD);

		if (GTK_IS_WIDGET(app->menu->bar))
			gtk_box_pack_start(GTK_BOX(mainbox), app->menu->bar, TRUE, TRUE, 0);
		if (GTK_IS_WIDGET(app->toolbar))
			gtk_box_pack_start(GTK_BOX(mainbox), app->toolbar, TRUE, TRUE, 0);

		gtk_widget_show(mainbox);
	
	        gtk_container_add (GTK_CONTAINER (app->window), mainbox);
	
		gtk_widget_show (app->window);

	}
	if (INTEGRATED_INTERFACE) {
		create_callb(NULL,(gpointer) app);
	}

	return;
}

gint app_quit(GtkWidget* wdg, gpointer data) {
	GList *node;
	GtkWidget *window;
	gint answer, modcount, newmodcount;
	gboolean discard=TRUE;
	doc_wrapper *dw=NULL, *dwtmp;
	app_struct *app;
	app = (app_struct *) data;

	modcount = count_modified_documents (app->docs);

	// We try to avoid inconsistencies...
	if (app->docs->current_doc) {
		commit_or_reset (app->stack);
	}

	while (modcount) {
		if (modcount==1) {
			for (node = app->docs->doc_list; node; node = node->next) {
				dwtmp = (doc_wrapper *) node->data;
				if (dwtmp->if_modified) {
					dw = dwtmp;
					break;
				}
			}
			if (!dw) {
				my_msg("One document is marked as unsaved, but I'm not able to find it\nPlease save it with the menu bar", WARNING);
				return TRUE;
			}
			answer = doc_save_question(dw);
			if (answer==CANCEL_YESNO)
				return TRUE; // Quitting cancelled!
			else
				// We continue the process (quitting + freeing docs)
				break;
			}
		else {
			window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
			modal_dialog_with_titles_window_provided (window,
				save_all_dialog_new (window, app->docs), 
				_("Exiting Geomorph"), 
				(gpointer) gboolean_set_true, 
				"Discard all and quit", 
				(gpointer) gboolean_set_false, 
				"Return", &discard, GTK_WIN_POS_CENTER, TRUE);
			if (discard) {
				// If there are less unsaved documents than before,
				// the user is still saving them, so we re-display
				// the dialog.
				// Otherwise, we continue the quitting process
				newmodcount = count_modified_documents (app->docs);
				if (newmodcount==modcount)
					break; 
				else {
					modcount = newmodcount;
					continue; 
				}
			}
			else // !discard
				return TRUE; // Quitting cancelled!
		}
	}
	
	gtk_main_quit();

	// 2005-02: The "save" and the "destroy" parts are separated,
	// otherwise some events stay connected to freed structures
	for (node = app->docs->doc_list; node; node = node->next) {
		doc_wrapper_free((doc_wrapper *) node->data, FALSE);
	}
	
//	xalloc_print_all ();

	return TRUE;
}

gint app_menu_delete (GtkWidget *wdg, GdkEvent *event, gpointer data) {
	return app_quit (wdg, data);
}

void app_free(app_struct *app) {
	if (app->types)
 		types_wrapper_free(app->types);
//	At this point, documents (app->docs->doc_list) are supposed to be freed
	if (app->docs)
		doc_swap_free(app->docs);
	if (app)
		x_free(app);
}

/*************************** CALLBACKS FOR TOOLBAR AND MENUS *************************/

gboolean swap_top_doc(GtkWidget *wdg, GdkEvent *event, gpointer data)
 {
//	Initializes the current document pointer when the user clicks on the window
	GList *node;
	app_struct *app;
	doc_wrapper *doc;
	app = (app_struct *) data;
	//  Do not execute the creation operation in another window!!
	if (app->docs->current_doc->creation_mode)
		return TRUE;
// printf("SWAP_TOP_DOC1:  %s\n", app->docs->current_doc->filename);
	// Find the document related to the window "wdg"
	node = g_list_first(app->docs->doc_list);
	while (node) {
		doc = (doc_wrapper *) node->data;
		if (wdg == doc->window)
			break;
		node = node->next;
	}
	if (!node)
		return TRUE; // not found, keep current value
	if (doc == app->docs->current_doc) {
// printf("SWAP THE SAME... %s\n",doc->filename);
		if (doc->type->display)
			(*doc->type->display) (doc->data);
		return TRUE; // no change
	}
	// We commit the last changes before focusing on the new document
	// (we suppose they were done on the last document displayed)
	commit_or_reset (app->stack);
	// The default is to place on top the tools window and the main window
	// We check the windows... under some circumstances (quitting...),
	// the gdk window of the current document is NULL
	if (app->window && app->window->window)
		gdk_window_raise(app->window->window);
	// This last test should not be necessary, this is not a document, but...
	if ((!INTEGRATED_INTERFACE) && app->types->tools_container && app->types->tools_container->window)
		gdk_window_raise(app->types->tools_container->window);
// printf("SWAP_TOP_DOC2:  %s\n", app->docs->current_doc->filename);
	doc_make_current(app->docs, doc);
	return TRUE; // stop the process
}

gint ok_create_callb(GtkWidget *wdg, gpointer data) {
//	Comitting the document creation
	app_struct *app;
	app = (app_struct*) data;
//	Add current doc to app->docs->doc_list
	app->docs->doc_list = g_list_append(app->docs->doc_list, app->docs->current_doc);
//	printf("OK_CREATE_CALLB: doc_list: %d;  doc_list->prev: %d;  doc_list->next: %d\n", app->docs->doc_list, app->docs->doc_list->prev, app->docs->doc_list->next);
//	Connect document window "focus_in_event" to current_doc/last_doc initialization
	gtk_signal_connect (GTK_OBJECT(app->docs->current_doc->window),
			"focus-in-event", GTK_SIGNAL_FUNC(swap_top_doc),
			(gpointer) app);
//	We probably need another signal, when the top most window is minimized,
//	for activating the window under it
//	gtk_signal_connect (GTK_OBJECT(app->docs->current_doc->window),
//			"expose-event", GTK_SIGNAL_FUNC(swap_top_doc),
//			(gpointer) app);

	// Attach the global menu accelerators
	gtk_window_add_accel_group (GTK_WINDOW(app->docs->current_doc->window), app->accel_group);

	app->new_doc_count++;
	app->docs->current_doc->creation_mode = FALSE;
	app->docs->current_doc->if_modified = TRUE;

//	We do some mandatory processes for committing the creation
	if (app->docs->current_doc->type->commit_creation) {
		(*app->docs->current_doc->type->commit_creation)
			((gpointer) app->docs->current_doc_data);
	}
//	Display the default tools dialog for the current document type
//	All commands should be connected to (gpointer) app->current_doc_data

/* NB: 	tools_dialog created in the file type choice callack (in thisappinit.c)
	or when opening a new file (open_callb here), with create_type_dialogs */
	if (INTEGRATED_INTERFACE) {
		gtk_widget_show(app->docs->current_doc->tools_container);
		if (GTK_IS_WIDGET(app->docs->current_doc->creation_container))
			// No creation container when opening a file
			gtk_widget_hide(app->docs->current_doc->creation_container);
	}
	else {
		if (app->docs->last_doc)  // last_doc NULL => creation of the 1st document
			if (app->docs->last_doc->type->tools_dialog ==
				app->docs->current_doc->type->tools_dialog)
		//	If type of new doc. = type of last one, same controls, do nothing!
				return;
		if (app->docs->last_doc)
			if (app->docs->last_doc->type->tools_dialog)  { 
				// Changing type - replace tools dialog
				gtk_widget_ref(GTK_WIDGET(app->docs->last_doc->type->tools_dialog));
				gtk_container_remove(GTK_CONTAINER(app->types->tools_container),
					app->docs->last_doc->type->tools_dialog);
		}
		gtk_widget_show(GTK_WIDGET(app->docs->current_doc->type->tools_dialog));
		if (!GTK_WIDGET(app->docs->current_doc->type->tools_dialog)->parent) {
			gtk_container_add(GTK_CONTAINER(app->types->tools_container),
				app->docs->current_doc->type->tools_dialog);
			gtk_object_sink(GTK_OBJECT(app->docs->current_doc->type->tools_dialog));
		}
		gtk_widget_show(app->types->tools_container);
	} // end else (==!INTEGRATED_INTERFACE)
	return FALSE;
}

void cancel_create (app_struct *app) {
//  	Destroying the window is supposed to call doc_wrapper_free
//  	2005-02: Seg faults when closing with gtk_widget_destroy (GTK+??)
//	so we simply hides the window (a memory leak...)
//	gtk_widget_destroy(GTK_WIDGET(app->docs->current_doc->window));
	gtk_widget_hide (GTK_WIDGET(app->docs->current_doc->window));
	doc_wrapper_free(app->docs->current_doc, TRUE);
// printf("Current_doc destroyed in cancel_create_callb!\n");
	app->docs->current_doc = app->docs->last_doc;
	if (app->docs->current_doc)
		app->docs->current_doc_data = app->docs->current_doc->data;
	else
		app->docs->current_doc_data = NULL;
}

gint cancel_create_callb(GtkWidget *wdg, gpointer data) {
//	Destroy newly created document structure
//	Restore app->current_doc

	app_struct *app;
	app = (app_struct *)data;
	cancel_create (app);
	if (INTEGRATED_INTERFACE && (!count_documents(app->docs)))
		gtk_main_quit();
	return FALSE;
}

void create_callb(GtkWidget *wdg, gpointer data) {

//	Creates a new document with one of the allowed types for the current application
//	Dialog is modal and varies with application type

	GtkWidget *hbox, *button, *vbox, *app_toolbar=NULL, *app_menu=NULL;
	app_struct *app;
	app = (app_struct *)data;

	// First we commit the changes to the current document, if there are any

	commit_or_reset (app->stack);

	app->docs->last_doc = app->docs->current_doc;
//	Create a document window for the current doc
//	Fill it with the display / controls for the current document type
	app->docs->current_doc =
		doc_wrapper_new("*", app->types->default_type->def_dir, app->types->default_type);
	app->docs->current_doc->fname_tochoose = TRUE;

	if (INTEGRATED_INTERFACE) {

	// We pack the creation dialog with OK/CANCEL buttons here
		app->docs->current_doc->creation_container = creation_container_new(app->types);
		app->docs->current_doc->tools_container = tools_container_new(app->types);
		// Commit_data && current_doc_data must not
		// contain the last document pointer
		set_commit_data (app->stack, NULL);
		app->docs->current_doc_data = NULL;
		create_type_dialogs(app->types, 
			app->docs->current_doc->type, 
			(gpointer) &app->docs->current_doc_data,
			app->stack,
			app->docs,
			app->docs->current_doc);

		hbox = gtk_hbox_new(TRUE,0);
		gtk_widget_show(GTK_WIDGET(hbox));

		button = gtk_button_new_with_label(_("OK"));
		gtk_widget_show(GTK_WIDGET(button));
		gtk_container_set_border_width(GTK_CONTAINER(button),DEF_PAD);
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(ok_create_callb), (gpointer) app);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

		button = gtk_button_new_with_label(_("Cancel"));
		gtk_widget_show(GTK_WIDGET(button));
		gtk_container_set_border_width(GTK_CONTAINER(button),DEF_PAD);
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(cancel_create_callb), (gpointer) app);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);

		vbox = gtk_vbox_new(FALSE,0);
		gtk_widget_show(GTK_WIDGET(vbox));
		gtk_box_pack_start(GTK_BOX(vbox), app->docs->current_doc->creation_dialog, TRUE, TRUE, DEF_PAD);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD);
		gtk_container_add(GTK_CONTAINER(app->docs->current_doc->creation_container),vbox);

		// We don't forget the tools dialog, for later use
		gtk_container_add(GTK_CONTAINER(app->docs->current_doc->tools_container), app->docs->current_doc->tools_dialog);
		gtk_widget_hide(app->docs->current_doc->tools_container);
	}
	else { // Gimp style interface
		if (!app->types->type_choice_dialog)	{

		// Current document and interface callbacks 
		// are linked in the following process

	//	We create a modal window, containing a hbox which packs:
	//		1st widget from the top:  file type choice toolbar
	//		option widgets, in the middle
	//		OK / Cancel button, at the bottom
	//	The interface is created once, so we keep the widgets embedded in their structs

	//	Creates the type choice dialog if inexistent

			type_choice_dialog_new(app->types, (gpointer) app,
				(gpointer) &app->docs->current_doc_data,
				app->stack,
				app->docs);

			modal_dialog_showhide(app->types->creation_container,
				app->types->type_choice_dialog,
				_("New document"), 
				ok_create_callb, 
				cancel_create_callb,
				(gpointer) app, 0, TRUE);
		}

		else {	// Reactivates the modal dialog
	
			type_choice_dialog_show(app->types);
			gtk_widget_show(GTK_WIDGET(app->types->creation_container));
			// ... set_uposition deprecated in GTK2, but I still have to find an alternative...
			gtk_widget_set_uposition(GTK_WIDGET(app->types->creation_container),app->types->win_pos_x, app->types->win_pos_y);
			gtk_grab_add(GTK_WIDGET(app->types->creation_container));
		}
	} // Emd Gimp style interface
	if (MENU_IN_DOC_WINDOW)
		app_menu = (menu_new(NBCOMMANDS,commands, app->accel_group, (gpointer) app))->bar;
	if (ICONS_IN_DOC_WINDOW)
		app_toolbar = standard_toolbar_new(NBCOMMANDS,
			commands,
			app->tooltips,
			app->window,
			(gpointer) app,
			GTK_ORIENTATION_HORIZONTAL,
			GTK_TOOLBAR_ICONS,
			FALSE);

//	Empty window + document creation
// printf("app->docs->doc_list dans CREATE_CALLB: %d\n",app->docs->doc_list);
	doc_new(app->docs, app->types, NULL, app->new_doc_count, &app->stack->commit_data, app_menu, app_toolbar);

//	Execute some defaults, if any 
//	2008-01: Migrated to the type dialog level -doctype.c
//	if (app->docs->current_doc->type->defaults)
//		(*app->docs->current_doc->type->defaults)
//			((gpointer) &app->docs->current_doc_data);
}

void open_callb(GtkWidget *wdg, gpointer data) {
//	Opens an existing file, loading it in a new document
//	Dialog is modal and varies with application type
	app_struct *app;
	doc_wrapper *dw;
	gchar *path_n_file;
	GtkWidget *app_toolbar=NULL, *app_menu=NULL;
	app = (app_struct *)data;

//	If wdg is NULL, we are trying to open a file given on the command line
//	doc_read calls doc_prepare_to_load
//	doc_new (a few lines ahead) "fills up" the structure 
//	prepared by doc_prepare_to_load
	if (wdg) { // Interactive creation

		// In this case we commit the changes to the current 
		// document, if there are any

		commit_or_reset (app->stack);
		dw = doc_read(&path_n_file, app->types, app->docs, app->default_dir);
	}
	else	{ // File name given on the command line
		path_n_file = app->file_on_cmdline;
		dw = doc_prepare_to_load(path_n_file, app->types);
	}
	if (!dw)
		return;

	if (app->docs->current_doc)
		if (app->docs->current_doc->creation_mode && INTEGRATED_INTERFACE)
			cancel_create (app);

	app->docs->last_doc = app->docs->current_doc;
	app->docs->current_doc = dw;
	// current_doc_data may contain a pointer to another document
	app->docs->current_doc_data = NULL;
/************** CHECK: creating a creation dialog not required here **********/
	//	Create the dialogs needed for managing 
	//	this particular document type, if needed
	create_type_dialogs(app->types, 
		dw->type, 
		(gpointer) &app->docs->current_doc_data,
		app->stack,
		app->docs,
		dw);

//	Empty window + document creation
//	Load the data from path_n_file into doc_wrapper struct
	if (INTEGRATED_INTERFACE) {
		app->docs->current_doc->tools_container = tools_container_new(app->types);
		gtk_container_add(GTK_CONTAINER(app->docs->current_doc->tools_container), app->docs->current_doc->tools_dialog);
	}

	if (MENU_IN_DOC_WINDOW)
		app_menu = (menu_new(NBCOMMANDS,commands, app->accel_group, (gpointer) app))->bar;
	if (ICONS_IN_DOC_WINDOW)
		app_toolbar = standard_toolbar_new(NBCOMMANDS,
			commands,
			app->tooltips,
			app->window,
			(gpointer) app,
			GTK_ORIENTATION_HORIZONTAL,
			GTK_TOOLBAR_ICONS,
			FALSE);

	if (!doc_new(app->docs, app->types, path_n_file, app->new_doc_count, &app->stack->commit_data, app_menu, app_toolbar))
		cancel_create (app);
	else {
		ok_create_callb(NULL,data);
		app->default_dir = app->docs->current_doc->dir;
		if (!app->docs->current_doc->fname_tochoose)
			app->docs->current_doc->if_modified = FALSE;
//	If it's the first document, we have to re-position the window after building the dialogs
		place_document_window(app->docs, app->types, app->types->tools_container);
	}
}

void save_callb(GtkWidget *wdg, gpointer data) {
//	Saves current document
	app_struct *app;
	app = (app_struct *)data;
	if (app && app->docs && app->docs->doc_list)
		if (g_list_length(app->docs->doc_list))
			if (app->docs->current_doc) {
				commit_or_reset (app->stack);
				doc_save(app->docs->current_doc);
			}
}

void save_as_callb(GtkWidget *wdg, gpointer data) {
//	Saves top document under a new name
//	Change file name in current doc of app
//	Then apply save_callb
	app_struct *app;
	app = (app_struct *)data;
	if (app && app->docs && app->docs->doc_list)
		if (g_list_length(app->docs->doc_list))
			if (app->docs->current_doc) {
				commit_or_reset (app->stack);
				app->docs->current_doc->fname_tochoose = TRUE;
				doc_save(app->docs->current_doc);
			}
}

void save_copy_as_callb(GtkWidget *wdg,gpointer data) {
//	Saves top document under a new name,
//	after copying it in a new window
//	Choose new file name
	//	Returns if cancelled
//	Create new doc_wrapper

//	Simple implementation (probably with overhead...):
//	Save under a new name, reopen the original document,
//	swap on the top the old renamed document

	app_struct *app;
	gchar *dname, *fname;
	doc_wrapper *doc;
	app = (app_struct *)data;
	if (!app->docs)
		return;
	if (!app->docs->current_doc)
		return;
	doc = app->docs->current_doc;

	if (doc->fname_tochoose) {
		my_msg(_("You'll be asked to save the document before cloning it."),INFO);
	}
	// Any modification should be saved, whatever happens
	commit_or_reset (app->stack);
	doc_save(app->docs->current_doc);
	
	if (app->file_on_cmdline)
		x_free(app->file_on_cmdline);
	app->file_on_cmdline = concat_dname_fname(doc->dir, doc->filename);

	app->docs->current_doc->fname_tochoose = TRUE;
	if (!doc_save(app->docs->current_doc)) {
		app->docs->current_doc->fname_tochoose = FALSE;
		return;
	}

	fname = doc->filename;	// new name
	dname = doc->dir;
	open_callb(NULL,data);  // Open the filename given by app->file_on_cmdline (old name)
//	Swap the file and dir names (some window focusing problems, otherwise)
	doc->filename = app->docs->current_doc->filename; // old name
	doc->dir = app->docs->current_doc->dir;
	app->docs->current_doc->filename = fname;
	app->docs->current_doc->dir = dname;
//	Old name for original doc
	gtk_window_set_title(GTK_WINDOW(doc->window), g_filename_to_utf8(app->file_on_cmdline,-1, NULL, NULL, NULL));
//	New name for newly opened doc
	gtk_window_set_title(GTK_WINDOW(app->docs->current_doc->window),
		 g_filename_to_utf8(concat_dname_fname(dname, fname), -1, NULL, NULL, NULL));
}

void print_callb(GtkWidget *wdg, gpointer app) {
	my_msg("PRINT_CALLB",WARNING);
}

void close_callb(GtkWidget *wdg, gpointer callb_data) {
//	Closes current document (which should be the top one!)
	GtkWidget *wintemp;
	app_struct *app;
	app = (app_struct *) callb_data;
	if (app && app->docs && app->docs->doc_list)
		if (g_list_length(app->docs->doc_list))
			if (app->docs->current_doc) {
				commit_or_reset (app->stack);
				wintemp = app->docs->current_doc->window;
				if (doc_close(wintemp, NULL, (gpointer) app->docs))
					return;
				gtk_widget_hide(wintemp);
	//	Some problems here with destroy - "expose event" left on ghost data!!
	//			gtk_widget_destroy(app->docs->current_doc->window);
			}
}

void quit_callb(GtkWidget *wdg, gpointer callb_data) {
//	Quit the application, after closing all opened documents
//	Scan documents wrappers

	app_struct *app;
	app = (app_struct *) callb_data;
	if (app)
		commit_or_reset (app->stack);

	//	Check for document needing saving
	//	If so:  ask to save (modal), then save with the save function for this type
	//	If not:  destroy document window
	app_quit(wdg, callb_data);
}

void undo_callb(GtkWidget *wdg, gpointer callb_data) {
	app_struct *app;
	app = (app_struct *) callb_data;
	if (app && app->docs && app->docs->current_doc && app->docs->current_doc->history)
		if (g_list_length(app->docs->current_doc->history)) {
			doc_undo(app->docs);
		}
}

void redo_callb(GtkWidget *wdg, gpointer callb_data) {
	app_struct *app;
	app = (app_struct *) callb_data;
	if (app && app->docs && app->docs->current_doc && app->docs->current_doc->history)
		if (g_list_length(app->docs->current_doc->history)) {
			doc_redo(app->docs);
		}
}

void copy_callb(GtkWidget *wdg, gpointer app) {
	my_msg("COPY_CALLB",WARNING);
}
void cut_callb(GtkWidget *wdg, gpointer app) {
	my_msg("CUT_CALLB",WARNING);
}
void paste_callb(GtkWidget *wdg, gpointer app) {
	my_msg("PASTE_CALLB",WARNING);
}
void paste_special_callb(GtkWidget *wdg, gpointer app) {
	my_msg("PASTE_SPECIAL_CALLB",WARNING);
}
void stats_callb(GtkWidget *wdg, gpointer data) {
	app_struct *app;
	app = (app_struct *) data;
	if (app && app->docs && app->docs->doc_list)
		if (g_list_length(app->docs->doc_list))
			if (app->docs->current_doc) {
				doc_stats(app->docs->current_doc);
			}
}
void options_callb(GtkWidget *wdg, gpointer data) {
	app_struct *app;
	app = (app_struct *) data;
	if (!app->options_dialog) {
		app->options_dialog = options_dialog_new( 
			&app->rc_options_file,
			&app->current_options,
			app->allowed_options,
			process_all_options);
	}
	else {
		gtk_widget_show(GTK_WIDGET(app->options_dialog));
		gtk_grab_add(GTK_WIDGET(app->options_dialog));
	}
}

void help_callb(GtkWidget *wdg, gpointer app) {
	static char *buf[3] = {NULL, NULL, NULL};
	buf[0] = DOC_READER;
	buf[1] = DOC_DIR;
	execution(buf[0],buf);
}
void about_callb(GtkWidget *wdg, gpointer app) {
	GtkWidget *content=NULL, *lbl;
	gchar *fname = NULL,*utf8_msg=NULL;
	gsize br,bw;
	GError *ger;
	static gchar *f1=NULL;
	static gchar *f2 = "splash.jpg";
	static gchar *msg = "\nGEOMORPH 0.6\nhttp://geomorph.sourceforge.net\n\n(c) Patrice St-Gelais 2003-2009 (GPL)\npatrstg@users.sourceforge.net\n\nGerman translation by\nSimon Donike (2005-2008) and Tim Schuermann (2004)";
	if (!f1) {
		f1 = x_malloc(2+strlen("/usr/local/share/geomorph/")+strlen(VERSION)+strlen(f2), "Splash image");
		strcpy(f1,"/usr/local/share/geomorph/");
		strcat(f1,VERSION);
		strcat(f1,"/");
		strcat(f1,f2);
	}
	if (!utf8_msg) {
		utf8_msg = g_locale_to_utf8(msg,-1,&br,&bw,&ger);
	}
	if (!utf8_msg)
		utf8_msg = msg;
	if (filexists(f1))
		fname = f1;
	else
		if (filexists(f2))
			fname = f2;
	if (fname) {
		content = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(content),gtk_image_new_from_file(fname));
		lbl = gtk_label_new(utf8_msg);
		gtk_label_set_justify(GTK_LABEL(lbl),GTK_JUSTIFY_CENTER);
		gtk_container_add(GTK_CONTAINER(content),lbl);
		gtk_widget_show_all(content);
	}
	if (content)
		modal_dialog (content, " ", NULL, NULL, NULL, GTK_WIN_POS_CENTER, TRUE);
	else
		my_msg(utf8_msg,INFO);
}
