/* Doctype.c - 	document type (class) processing, from which,
  * 		for instance, functions for processing documents are inherited
 *
 * Copyright (C) 2001 Patrice St-Gelais
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

#include <string.h>
#include "doc.h"

extern gint TOOLS_X, TOOLS_Y, CREATION_X, CREATION_Y, MAIN_BAR_X, MAIN_BAR_Y;
extern gboolean INTEGRATED_INTERFACE;

types_wrapper *types_wrapper_new(GtkWidget *main_window, 
		doc_type_struct *type_list,
		command_item_struct *interface_items, 
		gint nbitems) {
	// Creates the document types wrapper for the current application
	types_wrapper *tw;
	tw = (types_wrapper *) x_malloc(sizeof(types_wrapper), "types_wrapper");
// printf("TYPES_WRAPPER_NEW: %d\n", tw);

	tw->main_window = main_window;
	tw->nbtypes = nbitems;
	tw->doc_type_list = type_list;
	tw->default_type = type_list;
	tw->interface_items = interface_items;
	tw->type_choice_dialog = NULL;
	tw->creation_container = NULL;
	tw->tooltips = NULL;
	tw->tools_container = NULL;
	tw->tools_tooltips = NULL;
	if (INTEGRATED_INTERFACE) {
		tw->win_pos_x = MAIN_BAR_X*gdk_screen_width()/100;
		tw->win_pos_y = MAIN_BAR_Y*gdk_screen_height()/100;
	}
	else {
		tw->win_pos_x = CREATION_X*gdk_screen_width()/100;
		tw->win_pos_y = CREATION_Y*gdk_screen_height()/100;
	}
	tw->win_tools_pos_x = TOOLS_X*gdk_screen_width()/100;
	tw->win_tools_pos_y = TOOLS_Y*gdk_screen_height()/100;

	return tw;

}

void create_type_dialogs(types_wrapper *tw, 
		doc_type_struct *type, 
		gpointer callb_data, 
		stack_struct *stack, 
		gpointer doc_swap_struct, 
		doc_wrapper *dw) {
	gpointer p1=NULL, p2=NULL, p3=NULL;
//	Create the creation and type dialog for the current type, if needed,
//	without displaying them
//	Callb_data: pointer of document wrapper (pointer of pointer)
	type->stack = stack;
	if ((!type->creation_dialog) || INTEGRATED_INTERFACE ) {
		if (type->options_dialogs_new) { // Options for creation
			p1 =	(*type->options_dialogs_new)
					(tw->main_window, NULL,
					tw->tooltips, 
					NULL, // Widget list
					(gpointer) callb_data,
					stack,
					type->type,
					doc_swap_struct,
					get_doc_list_from_type);
			if (INTEGRATED_INTERFACE && dw) 
				dw->option_dialogs = p1;
			else
				type->options_dialogs = p1;
//			printf("1-TYPE->OPTIONS_DIALOGS: %d\n",type->options_dialogs);
			if (type->tools_dialog_new) // Widget: edition tools
				p2 = (*type->tools_dialog_new) 
					(tw->main_window, tw->tools_tooltips,
					(gpointer) p1, callb_data);
				if (INTEGRATED_INTERFACE) {
					if (dw)
						dw->tools_dialog = p2;
				}
				else
					type->tools_dialog = p2;
		}
		if (type->creation_dialog_new) { // Widget for creating a new document
			p3 = (*type->creation_dialog_new)
				(tw->main_window,
				tw->tooltips,
				(gpointer) p1,
				&type->widget_list);

//			printf("TYPE->WIDGET_LIST: %d\n",type->widget_list);
//			printf("2-TYPE->OPTIONS_DIALOGS: %d\n",type->options_dialogs);
		}
		else  {  // Default dialog if creation_dialog_new FN doesn't exist
			p3 = gtk_label_new(_("No options\nfor this document type"));
			gtk_widget_show(p3);
		}
					
		if (INTEGRATED_INTERFACE && dw)
			dw->creation_dialog = p3;
		else
			type->creation_dialog = p3;

		if (p1 && p3 && type->set_creation_container) {
			(*type->set_creation_container) (p1, p3);
		}
//		printf("3-TYPE->OPTIONS_DIALOGS: %d\n",type->options_dialogs);
		if (p1 && type->defaults)
			(*type->defaults) (p1);
	}
}

void types_wrapper_free(types_wrapper*tw) {
//	Freeing widgets not needed when they are destroyed after the
//	destruction of a parent window...
//	We don't free doc_type_list nor interface_items because they aren't "malloced"

	x_free(tw);
}

void initype(types_wrapper *tw,
		GtkWidget  *main_window, 
		doc_type_struct *type, 
		doc_wrapper ** current_doc_adr, 
		gpointer callb_data,
		stack_struct *stack,
		gpointer doc_swap_struct) {
//	Display a particular dialog (options) after choosing the document type
//	Check if the user is trying to replace a newly created doc / window
//	tw:	type wrapper for the current app
//		gives the current wrapping window and the current widget
//		in which we insert the dialog fot the choosen type
//	type:	new type to assign to the current document
//	current_doc_adr:	the address of the document we are creating
//	callb_data:  internal struct of the document 
//	(about which the application manager knows nothing)

	doc_wrapper *current_doc;
	current_doc = (doc_wrapper *) *current_doc_adr;
	tw->main_window = main_window;
	type->stack = stack;
	// printf("INITYPE;  types_wrapper: %d;  current_doc: %d;  adr: %d \n", tw, current_doc, current_doc_adr);
	// printf("INITYPE 2: CURRENT_DOC_DATA adr: %d; CURRENT_DOC_DATA: %d\n", callb_data, * (gpointer *) callb_data);
	if (current_doc->if_modified)
		if (!yes_no(_("Destroy the document you are creating?"), FALSE))
			return;
	// Remove current options widget from container (parent of wdg)
	gtk_widget_ref(GTK_WIDGET(current_doc->type->creation_dialog));
	gtk_container_remove(GTK_CONTAINER(tw->type_choice_dialog),
		GTK_WIDGET(current_doc->type->creation_dialog));

//	Re-initialize the document type (typically we are in creation mode 
//	and changing the kind of document we want to create...)
	current_doc->type = type;

	create_type_dialogs(tw, 
			type, 
			callb_data,
			stack,
			doc_swap_struct,
			NULL);

	gtk_container_add(GTK_CONTAINER(tw->type_choice_dialog), type->creation_dialog);
	gtk_object_sink(GTK_OBJECT(type->creation_dialog));
}

gint find_doc_type_from_ext(char *ext, types_wrapper *types) {

// Returns doc type struct index

	gint i, j;
	gchar *buf;
	gboolean found = FALSE;
	buf = strupp(ext);
	for (i=0; i<types->nbtypes; i++) {
		for (j=0; j<(types->doc_type_list+i)->nbextensions; j++) {
			if (!strcmp(buf,(types->doc_type_list+i)->extensions[j].lbl)) {
				found = TRUE;
				break;
			}
		}
		if (found)
			break;
	}
	free(buf);
	if (i>=types->nbtypes)
		return -1;
	else
		return i;
}

GtkWidget *creation_container_new(types_wrapper *types) {
//	Creation of the window which will contain a different
//	creation dialog structure for each file type
	GtkWidget *creation_container;
	if (INTEGRATED_INTERFACE) {
		types->tooltips = NULL;
		creation_container = frame_new ("New document",DEF_PAD);
		return creation_container;
	}
	types->tooltips = gtk_tooltips_new();
	creation_container = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize(GTK_WIDGET(creation_container));
	gtk_window_set_default_size(GTK_WINDOW(creation_container),150,150);
	gtk_widget_set_uposition(GTK_WIDGET(creation_container),
		types->win_pos_x, types->win_pos_y);
	gtk_window_set_resizable(GTK_WINDOW(creation_container),TRUE);
	types->creation_container = creation_container;
	return creation_container;
}

GtkWidget *tools_container_new(types_wrapper *types) {
//	Creation of the dialog (window) which will contain a different
//	tools dialog structure for each file type

	GtkWidget *tools_container;

	if (INTEGRATED_INTERFACE) {
		types->tooltips = NULL;
		tools_container = frame_new ("Tools",DEF_PAD);
		return tools_container;
	}

	types->tools_tooltips = gtk_tooltips_new();
	tools_container = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_widget_realize(GTK_WIDGET(tools_container));
//	It's a persistent window
  	gtk_signal_connect (GTK_OBJECT(tools_container), "delete-event",
                      (GtkSignalFunc) gtk_true, NULL);
	gtk_widget_set_uposition(GTK_WIDGET(tools_container),
		types->win_tools_pos_x, types->win_tools_pos_y);
	gtk_window_set_default_size(GTK_WINDOW(tools_container),150,150);
	gtk_window_set_resizable(GTK_WINDOW(tools_container),TRUE);
	gtk_window_set_title(GTK_WINDOW(tools_container), _("Tools"));
	types->tools_container = tools_container;
	return tools_container;
}

void type_choice_dialog_new(types_wrapper *types, 
		gpointer tb_callb_data, 
		gpointer dialog_callb_data,
		stack_struct *stack,
		gpointer doc_swap_struct) {
//	Creation of the dialog for choosing the file type of the document to create
//	types->type_choice_dialog is the main box (a vbox)
//	tb_callb_data should be the & of the pointer to the current application struct
//	dialog_callb_data should be the & of the pointer to the current document data
//	(not the wrapper, but the struct about which only the class (type) functions knows!)
//	We need to keep the dialog for packing other parts of the interface

	GtkWidget  *hbox, *lbl, *tbar;

	types->type_choice_dialog = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(types->type_choice_dialog);
	types->default_type->stack = stack;
// printf("Type Choice Dialog New avant création TOOLBAR\n");
	if (types->nbtypes>1) {
		tbar = 
		standard_toolbar_new (types->nbtypes, types->interface_items, 
			GTK_TOOLTIPS(types->tooltips),
			GTK_WIDGET(types->main_window),
			tb_callb_data,
			GTK_ORIENTATION_HORIZONTAL,
			GTK_TOOLBAR_BOTH,
			TRUE);

		// Pack dialogs in vbox
		hbox = gtk_hbox_new(FALSE, DEF_PAD);
		gtk_widget_show(hbox);
		lbl = gtk_label_new(_("Document type"));
		gtk_widget_show(lbl);
		gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, DEF_PAD);
		gtk_box_pack_start(GTK_BOX(hbox), tbar, FALSE, FALSE, DEF_PAD);
		gtk_box_pack_start(GTK_BOX(types->type_choice_dialog), hbox, FALSE, FALSE, 0);
	}

	// Types->current_type->creation_dialog varies with the kind of document
	// It must be removed from the container in a callback called by the
	// document types toolbar
	// *** maybe default_type->creation_dialog init should be put elsewhere?...
	// *** when we swap document type!!??

	create_type_dialogs(types, 
		types->default_type, 
		dialog_callb_data,
		stack,
		doc_swap_struct,
		NULL);

// printf("TYPE_CHOICE_DIALOG_NEW - type->default_type->creation_dialog: %d\n",types->default_type->creation_dialog);
	gtk_widget_show(types->default_type->creation_dialog);
	gtk_box_pack_start(GTK_BOX(types->type_choice_dialog), 
		types->default_type->creation_dialog, TRUE, TRUE, 0);
//	gtk_widget_show_all(GTK_WIDGET(types->window)); ... done in the calling FN
}

void type_choice_dialog_show(types_wrapper *type) {
	if (INTEGRATED_INTERFACE)
		return;
	gtk_widget_show(GTK_WIDGET(type->creation_container));
	// ... set_uposition deprecated in GTK2, but I still have to find an alternative...
	gtk_widget_set_uposition(GTK_WIDGET(type->creation_container),
		type->win_pos_x, type->win_pos_y);
	gtk_grab_add(GTK_WIDGET(type->creation_container));
} 
