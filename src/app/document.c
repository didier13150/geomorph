/* Document.c - generalized processing of multiple documents
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <string.h>
#include "globals.h"
#include "doc.h"
#include "../icons/attention.xpm"
#include "../icons/discard_red.xpm"
#include "../icons/save.xpm"

extern gint DISPLAY_DOC_OFFSET;
extern gboolean INTEGRATED_INTERFACE;

doc_swap_struct *doc_swap_new() {
	doc_swap_struct *dsw;
	dsw = (doc_swap_struct *) x_malloc(sizeof(doc_swap_struct), "doc_swap_struct");
	dsw->current_doc = NULL;
	dsw->current_doc_data = NULL;
	dsw->last_doc = NULL;
	dsw->doc_list = NULL;
	return dsw;
}

void doc_swap_free(doc_swap_struct *dsw) {
	if (dsw)
		x_free(dsw);
}

void doc_make_current(doc_swap_struct *dsw, doc_wrapper *new_current_doc) {
//	Makes new_current_doc the new current doc....
	if (dsw->current_doc == new_current_doc)
		return;
	// If current_doc does not exist, last_doc takes a NULL value
	// (for instance, after a destruction)
	if (!g_list_find(dsw->doc_list, dsw->current_doc) )
		dsw->last_doc = NULL;
	else
		dsw->last_doc = dsw->current_doc;
	dsw->current_doc = new_current_doc;
	dsw->current_doc_data = dsw->current_doc->data;
//	printf("MAKE CURRENT: %d == %x == %s\n", dsw->current_doc_data, dsw->current_doc_data, new_current_doc->filename);
	if (new_current_doc->type->display)
		(*new_current_doc->type->display) (new_current_doc->data);
}

gint doc_save (doc_wrapper *doc) {

//	Returns TRUE when the file is saved,
//		FALSE when it is not
	gchar *path_n_file, *new_path_n_file;
	gchar *default_dir=NULL;

	if (doc->dir)
		default_dir = doc->dir;

	path_n_file = (gchar *) x_malloc(strlen(default_dir)+strlen(doc->filename)+1, "gchar (path_n_file)");
	strcpy(path_n_file,default_dir);
	strcat(path_n_file, doc->filename);

	if (doc->fname_tochoose) {
		new_path_n_file =
			GetFilename(doc->type->extensions,
				doc->type->nbextensions,
				_("Save as..."),
				path_n_file,
				NEW_FILE,
				doc->type->extensions[0].lbl);
		x_free(path_n_file);
		if (!new_path_n_file)
			return FALSE;
		path_n_file = new_path_n_file;
	}
// printf("path_n_file dans DOC_SAVE: %s\n",path_n_file);
	(*doc->type->save) (doc->data, path_n_file);
	split_dir_file(path_n_file, &doc->dir, &doc->filename, FILESEP);
//	We reinitialize the default directory for the current type after a new file save
	if (doc->fname_tochoose) {
		doc->type->def_dir = (gchar *) x_realloc(doc->type->def_dir, strlen(doc->dir)+1, "gchar (doc->type->def_dir)");
		strcpy(doc->type->def_dir,doc->dir);
	}
	doc->fname_tochoose = FALSE;
	doc->if_modified = FALSE;
	gtk_window_set_title (GTK_WINDOW (doc->window), g_filename_to_utf8(path_n_file,-1,NULL,NULL,NULL));
	return TRUE;
}

void doc_stats (doc_wrapper *doc) {
	if (doc->type->stats)
		(*doc->type->stats) (doc->data);
}

void place_document_window(doc_swap_struct *dsw, types_wrapper *tw, GtkWidget *window) {

//	Guess best placement from tools & creation dialogs positions / size
//	"window" can be any window at the screen left
//	... tw->tools_window or tw->window (= create_window)
	GtkRequisition rq;
	static gint pad=0;
	gint x, y;
//	pad = g_list_length(dsw->doc_list)*DEF_PAD;
	if (window) {
		gtk_widget_size_request(GTK_WIDGET(window),&rq);
		x = DISPLAY_DOC_OFFSET*gdk_screen_width()/100
			 + pad + rq.width + tw->win_pos_x;
		y = pad + tw->win_tools_pos_y;
	}
	else { // Probably an integrated interface
		x = DISPLAY_DOC_OFFSET*gdk_screen_width()/100
			 + pad + tw->win_pos_x;
		y = pad + (tw->win_pos_y + tw->win_tools_pos_y)/2;

	}
	gtk_widget_set_uposition(GTK_WIDGET(dsw->current_doc->window), x, y);
	pad += DEF_PAD*2;
}

doc_wrapper *doc_wrapper_new(gchar *filename, gchar *dir, doc_type_struct *current_type) {
	doc_wrapper *dw;
	dw = (doc_wrapper *) x_malloc(sizeof(doc_wrapper), "doc_wrapper");
//	printf("DOC_WRAPPER_NEW: %d\n",dw);
	dw->type_id = DOC_WRAPPER_TYPE;
	dw->type = current_type;
	dw->data = NULL;
	dw->window = NULL;
	dw->filename = (gchar *) x_malloc(strlen(filename)+1, "gchar (dw->filename)");
	strcpy(dw->filename,filename);
	dw->dir = NULL;
	if (dir) {
		dw->dir = (gchar *) x_malloc(strlen(dir)+1, "gchar (dw->dir)");
		strcpy(dw->dir,dir);
	}
	dw->history = NULL;
	dw->max_history = MAX_HISTORY;
	dw->to_register_in_history = TRUE;
	dw->if_modified = FALSE;
	dw->fname_tochoose = FALSE;
	dw->creation_mode = TRUE; // Remember to put it FALSE when reading from a file!
//	Data struct for the current document type is initialized in doc_window_new
	dw->data = NULL;
	dw->data_to_show = NULL;
	dw->commit_data_adr = NULL;
	dw->creation_container = NULL;
	dw->tools_container = NULL;
	dw->creation_dialog = NULL;  
	dw->tools_dialog = NULL;
	dw->option_dialogs = NULL;
	return dw;

}

gboolean doc_new(doc_swap_struct *dsw, types_wrapper *tw, 
	gchar *path_n_file, gint sequence, gpointer commit_data_adr, 
	GtkWidget *menu, GtkWidget *toolbar) {
//	Creates a main display window for a new document
//	Creates the document
//	Initializes some variables

	gchar *buf;
	GtkWidget *vbox, *hbox, *window_box;
// printf("DOC_LIST dans DOC_NEW: %d\n",dsw->doc_list);
//	printf("CURRENT_DOC dans DOC_NEW: %d\n",dsw->current_doc);

	// Address of the data eventually used for committing the current transaction
	// Inherited from app
	// *commit_data_adr must be set to NULL when destroying the current document
	dsw->current_doc->commit_data_adr = commit_data_adr;
	
	dsw->current_doc->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(dsw->current_doc->window),TRUE);

	place_document_window(dsw, tw, tw->creation_container);

	gtk_container_border_width (GTK_CONTAINER(dsw->current_doc->window),DEF_PAD);
//	Default size:  25% x W/H
	gtk_window_set_default_size(GTK_WINDOW(dsw->current_doc->window),
		gdk_screen_width()/4,
		gdk_screen_height()/4);
//	Some processes need a realized window (because of XPMs icons...)
	gtk_widget_realize(GTK_WIDGET(dsw->current_doc->window));

	if (GTK_IS_WIDGET(menu) || GTK_IS_WIDGET(toolbar)) {
		window_box = gtk_vbox_new(FALSE,0);
		gtk_widget_show(GTK_WIDGET(window_box));
		gtk_container_add(GTK_CONTAINER(dsw->current_doc->window), window_box);
		if (GTK_IS_WIDGET(menu))
			gtk_box_pack_start(GTK_BOX(window_box), menu, FALSE, FALSE, 0);
		if (GTK_IS_WIDGET(toolbar))
			gtk_box_pack_start(GTK_BOX(window_box), toolbar, FALSE, FALSE, 0);
	}
	else
		window_box = dsw->current_doc->window;

//	Data and widget struct for current document are initialized and linked to ->window
	if (INTEGRATED_INTERFACE) {
		// The "window" passed to the type->new function
		// is the right part of the actual window, the left
		// part being reserved to the creation and tools dialogs
	
		hbox = gtk_hbox_new(FALSE,0);
		gtk_widget_show(GTK_WIDGET(hbox));
		vbox = gtk_vbox_new(FALSE,0);
		gtk_widget_show(GTK_WIDGET(vbox));
		gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, DEF_PAD);
		gtk_container_add(GTK_CONTAINER(window_box), hbox);
		if (GTK_IS_WIDGET(dsw->current_doc->creation_container))
			// There is no creation container, for instance,
			// when opening a file
			gtk_box_pack_start(GTK_BOX(vbox), dsw->current_doc->creation_container, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), dsw->current_doc->tools_container, FALSE, FALSE, 0);
		dsw->current_doc->data = (gpointer) (*dsw->current_doc->type->new) 
			(&dsw->current_doc->dir,
			&dsw->current_doc->filename,
			hbox,
			dsw->current_doc->creation_dialog,
			dsw->current_doc->option_dialogs,
			&dsw->current_doc->if_modified,
			&dsw->current_doc->to_register_in_history,
			(gpointer) doc_register_in_history,
			dsw->current_doc,
			path_n_file,
			&dsw->current_doc->fname_tochoose,
			0);
	}
	else {
		dsw->current_doc->data = (gpointer) (*dsw->current_doc->type->new) 
			(&dsw->current_doc->dir,
			&dsw->current_doc->filename,
			window_box,
			dsw->current_doc->type->creation_dialog,
			dsw->current_doc->type->options_dialogs,
			&dsw->current_doc->if_modified,
			&dsw->current_doc->to_register_in_history,
			(gpointer) doc_register_in_history,
			dsw->current_doc,
			path_n_file,
			&dsw->current_doc->fname_tochoose,
			0);
	}

	if (!dsw->current_doc->data)	//	Something had gone bad!
		return FALSE;

	dsw->current_doc_data = dsw->current_doc->data;

	if (dsw->current_doc->fname_tochoose) {
		// File name = "document" + new doc count
		// 2005-11-25: added default extension
		buf = (gchar *) x_malloc(strlen(_("document"))+8, "gchar (buf - documentN)");
		sprintf(buf,"%s%-i.%s",_("document"),sequence%1000,strlow(dsw->current_doc->type->extensions[0].lbl));
		dsw->current_doc->filename = buf;
	}

	gtk_window_set_title (GTK_WINDOW (dsw->current_doc->window),
		g_filename_to_utf8(concat_dname_fname(dsw->current_doc->dir,dsw->current_doc->filename), -1, NULL, NULL, NULL));
			
//	The standard close / destroy event
	gtk_signal_connect (GTK_OBJECT (dsw->current_doc->window), "delete_event",
                        GTK_SIGNAL_FUNC(doc_close), (gpointer) dsw);
// printf("dsw->current_doc->dir: %s; \n",dsw->current_doc->dir);

	gtk_widget_show(GTK_WIDGET(dsw->current_doc->window));
//	printf("File name: %s;  Ptr of newly created doc: %d\n", dsw->current_doc->filename, dsw->current_doc->data);
	return TRUE;
}

doc_wrapper *doc_prepare_to_load(gchar *path_n_file, types_wrapper *types) {
//	Prepares a document to be read from its extension
//	Creates the doc_wrapper struct
	gint doc_type_index;
	gchar *ext, *dname, *fname;
	doc_wrapper *dw;
	gchar *buf, *utf8;

	if (path_n_file)
		split_dir_file(path_n_file, &dname, &fname, FILESEP);
	else
		return NULL;
	ext  = (gchar *) strrchr(fname, '.');

//	printf("Dans DOC_LOAD:  PATH_N_FILE: %s;  DNAME: %s;  FNAME: %s; EXT: %s; ",path_n_file,dname, fname, ext, doc_type_index);
//	Document type deduced from extension
	if (ext)
		doc_type_index = find_doc_type_from_ext(1+ext, types);
	else {
		doc_type_index = -1;
	}
//	printf("IDX: %d\n", doc_type_index);
	if (doc_type_index > 0) {
		buf = (gchar *) x_malloc(strlen(fname)+strlen(_("File extension <<%s>> unknown or non-existent"))+1, "gchar (fname)");
		sprintf(buf,_("File extension <<%s>> unknown or non-existent"),fname);
		my_msg(buf,WARNING);
		x_free(buf);
		return NULL;
	}
	if (!filexists(path_n_file)) {
		utf8 = g_filename_to_utf8 (path_n_file, -1, NULL, NULL, NULL);
		buf = (gchar *) x_malloc(strlen(_("File %s not found or unreadable!"))+strlen(utf8)+1, "gchar (buf - File not found MSG)");
		sprintf(buf,_("File %s not found or unreadable!"),utf8);
		my_msg(buf,WARNING);
		x_free(buf);
		free(utf8);
		return NULL;
	    }
	if (dname) {
		(types->doc_type_list+doc_type_index)->def_dir = (gchar *)
			x_realloc((types->doc_type_list+doc_type_index)->def_dir,strlen(dname)+1, "gchar (def_dir)");
		strcpy((types->doc_type_list+doc_type_index)->def_dir,dname);
	}
	dw =  doc_wrapper_new(fname, dname, types->doc_type_list+doc_type_index);
	return dw;
}

doc_wrapper *doc_read(gchar **path_n_file_adr, types_wrapper *types,
	doc_swap_struct *dsw, gchar *app_default_dir) {
//	Returns NULL if the file cannot be read,
//	otherwise returns a new doc_wrapper pointer
	gchar *default_dir;
	gint nb_ext=0, n_ext, i, j;
	omenu_list_type *ext_menu=NULL;

//	Find all allowable extensions for the current application
//	Build an integrated option menu with these	extensions
//	... optimization:  calculate once for the session
	for (i=0; i<types->nbtypes; i++) {
		n_ext = nb_ext;
		nb_ext += types->doc_type_list[i].nbextensions;
		ext_menu = (omenu_list_type *) x_realloc(ext_menu,nb_ext*sizeof(omenu_list_type), "omenu_list_type");
		for (j=0; j<types->doc_type_list[i].nbextensions; j++) {
			memcpy(ext_menu+n_ext+j,
				types->doc_type_list[i].extensions+j,
				sizeof(omenu_list_type));
		}
	}
//	The best guess for the default dir comes from the type of the current document
	if (dsw && dsw->current_doc && dsw->current_doc->type->def_dir)
		default_dir = dsw->current_doc->type->def_dir;
	else
		if (types->default_type->def_dir)
			default_dir = types->default_type->def_dir;
		else
			default_dir = app_default_dir;
	(*path_n_file_adr) =
		GetFilename(	ext_menu,
				nb_ext,
				_("Open"),
				default_dir,
				EXISTING_FILE,
				ext_menu[0].lbl);
	x_free(ext_menu);

	return doc_prepare_to_load(*path_n_file_adr,types);
}

gint doc_save_question (doc_wrapper *dw) {

	gint lendir;
	gchar *buf, *buf2, *utf8;
	
	lendir =strlen(dw->dir);
	buf2 = (gchar *) x_malloc(lendir+strlen(dw->filename)+1, "gchar (buf2 - dw->filename)");
	strcpy(buf2,dw->dir);
//	buf2[lendir] = FILESEP; // Already included int dw->dir
	buf2[lendir] = '\0';
	strcat(buf2, dw->filename);
	utf8 = g_filename_to_utf8 (buf2, -1, NULL, NULL, NULL);
	buf = (gchar *) x_malloc(strlen(_("<%s> has been modified. Save?"))+strlen(utf8)+1, "gchar (buf - Document modified message)");
	sprintf(buf,_("<%s> has been modified. Save?"),utf8);
	x_free(buf2);
	free(utf8);
	switch (yes_no_cancel(buf,TRUE)) {
		case TRUE:
			doc_save(dw);
			x_free(buf);
			return TRUE;
		case CANCEL_YESNO:
			x_free(buf);
			return CANCEL_YESNO;  	// window / document not destroyed
		case FALSE:		// destroy window / document
			x_free(buf);
			return FALSE;
	} // end switch
	return TRUE;
}

gint doc_close (GtkWidget *wdg, GdkEvent *event, gpointer callb_data) {

	gint answer;
	GList *node = 0;
	doc_swap_struct *dsw;
	dsw = (doc_swap_struct *) callb_data;

	// We check if the window receiving the delete event is the window
	// of the current document
	// If not, we make the document related to the window the current one
	if (wdg)
		if (dsw->current_doc && (wdg!=dsw->current_doc->window)) {
			// Search the related document
			for (node = dsw->doc_list; node; node=node->next) {
				if (((doc_wrapper *) node->data)->window == wdg) {
					doc_make_current(dsw, (doc_wrapper *) node->data);
					break;
				}
			}
		}
	answer = FALSE; // Quit without saving when not modified
	if (!node) {
		my_msg("Not able to find the document to close!\nClick on a document window to activate the related document",WARNING);
		return TRUE;
	}
	if (dsw->current_doc->if_modified) {
		answer = doc_save_question(dsw->current_doc);
	} 
	
	if (answer==CANCEL_YESNO)
		return TRUE;
	
	doc_wrapper_destroy(dsw);

	if (INTEGRATED_INTERFACE && (!count_documents(callb_data)))
		gtk_main_quit();

	return FALSE;
}

void doc_wrapper_free(doc_wrapper* dw, gboolean free_interface) {
	GList *node;
	if (dw->type->free) {
		(*dw->type->free) (dw->data, free_interface);

		// We avoid committing the last transaction 
		// if it's about the document we are destroying		
		if (dw->data == *dw->commit_data_adr)
			*dw->commit_data_adr = NULL;
		
//	We free all the doc instances in the history
		for (node = dw->history; node; node=node->next) {
//			printf("DW_FREE- NODE->DATA: %d\n",node->data);
			(*dw->type->free) (node->data, FALSE);
		}
		g_list_free(dw->history);

	}
	x_free(dw->filename);
	x_free(dw->dir);
	if (dw->option_dialogs) // For the integrated interface
		if (dw->type->options_dialogs_free) {
			(*dw->type->options_dialogs_free) (dw->option_dialogs);
			dw->option_dialogs = NULL;
		}
	x_free(dw);
}

void doc_wrapper_destroy(doc_swap_struct *dsw) {
	gint i;
	GtkWidget *tools_window=NULL;
	i = g_list_length(dsw->doc_list);

	dsw->doc_list = g_list_remove(dsw->doc_list, (gpointer) dsw->current_doc);
	i = g_list_length(dsw->doc_list);
	if ((!INTEGRATED_INTERFACE) && GTK_IS_WIDGET(dsw->current_doc->type->tools_dialog))
		tools_window = gtk_widget_get_parent(dsw->current_doc->type->tools_dialog);

	doc_wrapper_free(dsw->current_doc, TRUE);

//	CURRENT_DOC should be the next on the list, except if list length is 0!
	dsw->current_doc = NULL;
	dsw->current_doc_data = NULL;
	if (i) {
		doc_make_current(dsw,(doc_wrapper *) dsw->doc_list->data);

//		gtk_widget_grab_focus(GTK_WIDGET(dsw->current_doc->window));

		if (dsw->current_doc)
			gdk_window_show(dsw->current_doc->window->window);
	}
	else
	// We hide the tools dialog if there is no document left!
		if ((!INTEGRATED_INTERFACE) && GTK_IS_WIDGET(tools_window))
			gtk_widget_hide (tools_window);
}

GList *get_doc_list_from_type (gpointer gdsw, gchar * type) {
//	Builds a list of dsw->(GList)->doc_data from a given type (HFPNG, POVINI...)
	doc_swap_struct *dsw;
	GList *node,*list = NULL;
	dsw = (doc_swap_struct *) gdsw;
	for (node = dsw->doc_list; node; node = node->next) {
		if ( !strcmp(((doc_wrapper*)(node->data))->type->type,type) ) {
			list = g_list_append(list,((doc_wrapper*)(node->data))->data);
		}
	}
	return list;
}

gint count_modified_documents (doc_swap_struct *dsw) {
	doc_wrapper *dw;
	GList *node;
	gint count=0;
	for (node = dsw->doc_list; node; node = node->next) {
		dw = (doc_wrapper *) node->data;
		if (dw->if_modified) {
			count++;
		}
	}
	return count;
}

gint count_documents (doc_swap_struct *dsw) {
	//doc_wrapper *dw;
	GList *node;
	gint count=0;
	for (node = dsw->doc_list; node; node = node->next) {
		//dw = (doc_wrapper *) node->data;
		count++;
	}
	return count;
}

void dummy (GtkWidget *wdg, gpointer data) {
}

void doc_save_callb (GtkWidget *wdg, gpointer data) {
	if (data) {
		doc_save ((doc_wrapper *) data);
	}
}

GtkWidget *save_all_dialog_new (GtkWidget *window_to_destroy, doc_swap_struct *dsw) {
	
	// Dialog for saving / discarding all the docs at once
	
	// "window" is the dialog window to destroy after saving a document
	// (then, we regenerate the dialog)
	
	GtkWidget *dialog, *vbox, *table, *label, *icon, *hbox, *hbox2, *scrolled_window, *window, *toolbar, *wdg;
	doc_wrapper *dw;
	gint i, modcount;
	GList *node;
		
	modcount = count_modified_documents (dsw);
	
	// Window reference for creating pixmaps from xpms
	window = ((doc_wrapper *) dsw->doc_list->data)->window;
	
	dialog = gtk_vbox_new (FALSE, DEF_PAD);
	gtk_widget_show (dialog);
	
	vbox = gtk_vbox_new (FALSE, DEF_PAD);
	gtk_widget_show (vbox);
	
	label = gtk_label_new (_("These documents haven't been saved"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX(vbox), align_widget (label, 0.5, 0.5) , FALSE, FALSE, DEF_PAD);
	
	hbox2 = gtk_hbox_new (FALSE, DEF_PAD);
	gtk_widget_show (hbox2);
	label = gtk_label_new (_("Click on "));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX(hbox2), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(hbox2), create_widget_from_xpm (window, save_xpm), FALSE, FALSE, 0);
	label = gtk_label_new (_(" for those you want to save") );
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX(hbox2), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), align_widget(hbox2,0.5,0.5), FALSE, FALSE, 0);
		
	hbox = gtk_hbox_new (FALSE, DEF_PAD);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX(hbox), create_widget_from_xpm (window, attention_xpm), FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX(hbox), vbox, FALSE, FALSE, DEF_PAD);
	
	gtk_box_pack_start (GTK_BOX(dialog), align_widget(hbox,0.4, 0.5), FALSE, FALSE, 0);
	
	vbox = gtk_vbox_new (FALSE, DEF_PAD);
	gtk_widget_show (vbox);
	
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (scrolled_window);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), vbox);
	gtk_widget_set_size_request (scrolled_window, 500, MIN(modcount,6)*80);
	
	table = gtk_table_new (modcount, 3, FALSE);
	gtk_widget_show (GTK_WIDGET(table));
	gtk_box_pack_start(GTK_BOX(vbox), align_widget(table,0.5,0.5), TRUE, TRUE, 0);

	i = 0;
	for (node = dsw->doc_list; node; node=node->next) {
		dw = (doc_wrapper*)(node->data);
		if (!dw->if_modified)
			continue;
		if (dw->type->get_icon) {
			icon = (*dw->type->get_icon) (dw->data, 64, 64);
			gtk_table_attach (GTK_TABLE(table), icon, 0, 1, i, i+1, 0, 0, DEF_PAD, DEF_PAD);
		}
		toolbar = gtk_toolbar_new();
		gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar),GTK_ORIENTATION_HORIZONTAL);
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
		gtk_widget_show(GTK_WIDGET(toolbar));
		wdg = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
			GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,_("Discard"),
			_("Discard"),NULL,
			create_widget_from_xpm(window,discard_red_xpm),
			GTK_SIGNAL_FUNC(dummy), (gpointer) NULL);
		wdg = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
			GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("Save"),
			_("Save"),NULL,
			create_widget_from_xpm(window,save_xpm),
			GTK_SIGNAL_FUNC(doc_save_callb), (gpointer) dw);
		if (window_to_destroy && GTK_IS_WIDGET(window_to_destroy))
			gtk_signal_connect_object(GTK_OBJECT(wdg), "clicked",
				GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) window_to_destroy);
		gtk_table_attach (GTK_TABLE(table), GTK_WIDGET(toolbar), 1, 2, i, i+1, 0, 0, DEF_PAD, DEF_PAD);
		label = gtk_label_new(g_filename_to_utf8(concat_dname_fname(dw->dir, dw->filename), -1, NULL, NULL, NULL));
		gtk_widget_show(label);
		gtk_table_attach (GTK_TABLE(table), GTK_WIDGET(label), 2, 3, i, i+1, 0, 0, DEF_PAD, DEF_PAD );
		i++;
	}
	
	gtk_box_pack_start(GTK_BOX(dialog), scrolled_window, FALSE, FALSE, DEF_PAD);
	
	return dialog;
}
