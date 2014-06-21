/* filesel.c - File selection dialog
 *
 * Copyright (C) 2000 Patrice St-Gelais
 * patrstg@users.sourceforge.net
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

/* 
 * File selection dialog, adapted from Eric Harlow's book
 * Allows choosing a NEW_FILE (saving) or an EXISTING_FILE (reading)
 * File extension is mandatory for NEW_FILE
 * My first GTK coding, needs a revision !! :-(
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include "filesel.h"
#include "x_alloc.h"

static	 GtkWidget 	*filew;
static   char         	*selected_file_name = NULL, *Default_Dir_File=NULL;
static	 int		Write_or_Read;

//	Global variable containing the current file types option menu
omenu_struct_type *file_format;

char *add_extension(char *file_name, char *ext) {
/*	Add lowercase extension if inexistent
	IMPORTANT: give ext without a beginning dot */

	char *extension, *buf, *dot;

/*	1. Extract the string after the last dot
	2. If string is empty, add the extension
	3. If not, check the string: if it <> ext, give a warning
	-> the extension typed prevails over the menu choice */

	if (!file_name) return NULL;
	if (strlen(file_name)==0) return NULL;
	if (file_name[strlen(file_name)-1] == FILESEP) return NULL;
	if (!strcmp(ext,_("<no extension>"))) return file_name;

	extension = strlow(ext);
	dot = rindex(file_name,'.');
	if (!dot) {
		buf=(char *) x_malloc(strlen(file_name)+5, "char (buf in add_extension in filesel.c)");
		strcpy(buf,file_name);
		strcpy(buf+strlen(file_name),".");
		strcpy(buf+1+strlen(file_name),extension);
		return(buf);
	}
	else
		if (strcmp(strlow(dot+1),extension))
			my_msg(_("Typed extension <> chosen format"),WARNING);
		return(file_name);
}

void filetype_sel_callback_fn (gpointer data, GtkWidget *emitting_widget) {
//	This function is called by each menu item of the extensions option menu,
//	if file_format->omenu_struct[].callback_fn is initialized with its pointer
//	It selects the displayed file list from the current extension
//	(connected with "gtk_signal_connect_object, this is why the args are reverted)
	char *tmplbl, *label;
	label = file_format->current_lbl;
	if (Write_or_Read==NEW_FILE)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(filew), Default_Dir_File);
	else {
		if (!strcmp(_("<no extension>"), label)) {
			gtk_file_selection_complete(GTK_FILE_SELECTION(filew),"*");
			return;
		}
		tmplbl = (char *) x_malloc(strlen(label)+3, "char (tmplbl in filetype_sel_callback_fn)");
		sprintf(tmplbl,"*.%s",strlow(label));
		gtk_file_selection_complete(GTK_FILE_SELECTION(filew),tmplbl);
		x_free(tmplbl);
	}
	
}

gint file_ok_sel (GtkWidget *w, GtkFileSelection *fs)
{
    char *buf=NULL;

    const char *sTempFile = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));

    /* --- Allocate space and save it. --- */
    selected_file_name = (char *) x_malloc (sizeof (char) * (strlen (sTempFile) + 5), "char (selected_file_name in file_ok_sel)");
	// +5:  allows space for dot and extension

    strcpy (selected_file_name, sTempFile);

    switch (Write_or_Read) {
	
	case NEW_FILE:  // We want a new file - typically for writing
// printf("NEW_FILE\n");
            selected_file_name = add_extension(selected_file_name,file_format->current_lbl);
    	    if (filexists(selected_file_name)) {
		buf = (char *) x_malloc(strlen(_("File %s already exists. OVERWRITE ?"))+strlen(selected_file_name)+1, "const char (message - buf in file_ok_sel)");
		sprintf(buf,_("File %s already exists. OVERWRITE ?"),selected_file_name);
		if (!yes_no(buf,FALSE))  {  // do we cancel or overwrite ?
			// Cancel!
			x_free(selected_file_name);
			selected_file_name = NULL;
			// x_free(buf);
			return TRUE;
		}
		// Overwrite -> simply continue -> destroy the dialog
		if (buf)
			x_free(buf);
	    }
     	    gtk_widget_destroy(filew);
	    break;
     	case EXISTING_FILE:  // We want an existing file - typically for reading
// printf("EXISTING_FILE\n");
	    if (!filexists(selected_file_name)) {
		buf = (char *) x_malloc(strlen(_("File %s not found or unreadable!"))+strlen(selected_file_name)+1,"const char (message - buf in file_ok_sel)" );
		sprintf(buf,_("File %s not found or unreadable!"),selected_file_name);
		my_msg(buf,WARNING);
		// x_free(buf);
		break;
	    }
	// else -> no break =  we destroy the file selection
     	case NO_CHECK:
// printf("NO_CHECK\n");
     		/* --- Destroy the file selection --- */
     		gtk_widget_destroy(filew);
     } // switch
     return FALSE;
}

gint dir_ok_sel (GtkWidget *w, GtkFileSelection *fs)
{
    // 2007-03 Patched version, for directories
    
    char *buf=NULL, *f_utf8=NULL;
    // f_utf8 is returned by g_filename_to_utf8 and is statically allocated
    // (no need to free it)

    const char *sTempFile = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));

    /* --- Allocate space and save it. --- */
    selected_file_name = (char *) x_malloc (sizeof (char) * (strlen (sTempFile) + 5), "char (selected_file_name in dir_ok_sel)");
	// +5:  allows space for dot and extension

    strcpy (selected_file_name, sTempFile);

    switch (Write_or_Read) {
	case NEW_FILE:  // We want a new file - typically for writing
    	    if (directory_exists(selected_file_name)) {
	    	f_utf8 = g_filename_to_utf8(selected_file_name,-1,NULL,NULL,NULL);
		buf = (char *) x_malloc(strlen(_("Directory %s already exists."))+strlen(f_utf8)+1, "const char (message - buf in dir_ok_sel)");
		sprintf(buf,_("Directory %s already exists."),f_utf8);
		// Existing directory, we cancel the operation
		x_free(selected_file_name);
		selected_file_name = NULL;
		// x_free(buf);
		buf=NULL;
		return TRUE;
	    }
     	    gtk_widget_destroy(filew);
	    break;
     	case EXISTING_FILE:  // We want an existing directory
	    if (!directory_exists(selected_file_name)) {
	    	f_utf8 = g_filename_to_utf8(selected_file_name,-1,NULL,NULL,NULL);
		buf = (char *) x_malloc(strlen(_("File %s not found or unreadable!"))+strlen(f_utf8)+1, "const char (message - buf in dir_ok_sel)");
		sprintf(buf,_("File %s not found or unreadable!"),f_utf8);
		my_msg(buf,WARNING);
		// x_free(buf);
		buf=NULL;
		break;
	    }
	// else -> no break =  we destroy the file selection
     	case NO_CHECK:
     		/* --- Destroy the file selection --- */
     		gtk_widget_destroy(filew);
     } // switch
     
     return FALSE;
}

void file_cancel_sel (GtkWidget *w, GtkFileSelection *fs)
{
    /* --- Destroy the file selection --- */
    if (selected_file_name)
	x_free(selected_file_name);
    selected_file_name = NULL;
    gtk_widget_destroy (filew);
}

/*
 * DestroyDialog
 *
 * Destroy the dialog, remove the grab and close the modal. 
 */
int DestroyDialog (GtkWidget *widget, gpointer *data)
{
    gtk_grab_remove (widget);
    gtk_main_quit ();
    return (FALSE);
}

void init_file_format_menu(omenu_list_type *ff_list,
	int max_index, char *extension) {

 	GtkWidget *box;
	int i;

	for (i=0; i<max_index; i++) {
// printf("ff_list[%d].lbl = %s\n",i,ff_list[i].lbl);
		if (!strcmp(extension, ff_list[i].lbl))
			break;
	}
	if (i == max_index) i=0;

	if (!create_omenu( &file_format, ff_list, max_index, i))
		my_msg(_("Not able to allocate memory for file formats option menu ?!"),ABORT);

	box = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_container_set_border_width (GTK_CONTAINER (box), DEF_PAD);

	define_label_in_box(_("File format"), box, FALSE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(box),file_format->omenu_ptr, TRUE, TRUE, 0);

	gtk_widget_show(box);

	gtk_container_add(GTK_CONTAINER(GTK_FILE_SELECTION(filew)->action_area),box);
}

/*
 * GetFilename 
 * See prototype description in filesel.h
 */
char *GetFilename (omenu_list_type *ff_list, int max_index, char *sTitle, 
		char *default_dir_file, int write_or_read, char *ext)	
{
     char *return_string;
     Write_or_Read = write_or_read;

// printf("default_dir_file: %s; ext: %s\n",default_dir_file, ext);
     // Ugly patch 2005-11 - to initialize correctly the preselected filename
     Default_Dir_File = default_dir_file;
     
    /* --- If the value is saved from last time we used the dialog,
     * --- free the memory
     */
    if (selected_file_name) {
        x_free (selected_file_name);
        selected_file_name = NULL;
    }

    /* --- Create a new file selection widget --- */
    filew = gtk_file_selection_new (_(sTitle));
    
    // It's generally a modal dialog, so we hide the buttons (no way to create a directory!)
    gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION(filew));
 
    /* --- Lets set the filename, as if this were a save 
     *     dialog, and we are giving a default filename 
     */

if (default_dir_file)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), default_dir_file);
    
    /* --- If it's destroyed --- */
    gtk_signal_connect (GTK_OBJECT (filew), "destroy",
            (GtkSignalFunc) DestroyDialog, &filew);

    /* --- Connect the ok_button to file_ok_sel function  --- */
    gtk_signal_connect (
            GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
            "clicked", (GtkSignalFunc) file_ok_sel, filew );
    
    /* --- Connect the cancel_button to destroy the widget  --- */
    gtk_signal_connect (
             GTK_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
             "clicked", (GtkSignalFunc) file_cancel_sel, filew);

    gtk_widget_show (filew);
    // File format (PNG, PBM...)    
    init_file_format_menu(ff_list, max_index, ext);
   
//    signaldir = gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION (filew)->dir_list),
//		"expose_event", (GtkSignalFunc) filetype_sel_callback_fn, NULL);

    // Some connections required to update the file list
    // with file pattern from current extension: *** DOESN'T WORK AS INTENDED!
/*
    gtk_signal_connect_object(
	GTK_OBJECT(gtk_option_menu_get_menu
	((GtkOptionMenu *) ((GtkFileSelection *)filew)->history_pulldown)),
		"selection-done", (GtkSignalFunc) test_fn,
		(GtkObject *) file_format->current_lbl);

    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION (filew)->dir_list),
		"", (GtkSignalFunc) filetype_sel_callback_fn,
		(GtkObject *) file_format->current_lbl);
*/

    /* --- Make sure we keep the focus --- */
    gtk_grab_add (filew);

    gtk_main ();
	
    x_free(file_format);
    if (!selected_file_name)
	return NULL;
    return_string = (char *) x_malloc(strlen(selected_file_name)+1, "char (return_string in GetFileName)");
    strcpy(return_string,selected_file_name);
    return return_string;
}


/*
 * GetFilename - version for directories
 * See prototype description in filesel.h
 */
char *GetDirname (omenu_list_type *ff_list, int max_index, char *sTitle, 
		char *default_dir_file, int write_or_read, char *ext)	
{
     char *return_string;
     Write_or_Read = write_or_read;

     // Ugly patch 2005-11 - to initialize correctly the preselected filename
 //    Default_Dir_File = default_dir_file;
     
    /* --- If the value is saved from last time we used the dialog,
     * --- free the memory
     */
    if (selected_file_name) {
        x_free (selected_file_name);
        selected_file_name = NULL;
    }

    /* --- Create a new file selection widget --- */
    filew = gtk_file_selection_new (_(sTitle));
    
    // It's generally a modal dialog, so we hide the buttons (no way to create a directory!)
    gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION(filew));

    /* --- If it's destroyed --- */
    gtk_signal_connect (GTK_OBJECT (filew), "destroy",
            (GtkSignalFunc) DestroyDialog, &filew);

    /* --- Connect the ok_button to file_ok_sel function  --- */
    gtk_signal_connect (
            GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
            "clicked", (GtkSignalFunc) dir_ok_sel, filew );
    
    /* --- Connect the cancel_button to destroy the widget  --- */
    gtk_signal_connect (
             GTK_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),
             "clicked", (GtkSignalFunc) file_cancel_sel, filew);
 
    /* --- Lets set the filename, as if this were a save 
     *     dialog, and we are giving a default filename 
     */

if (default_dir_file)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), default_dir_file);

    gtk_widget_show (filew);
    // File format (PNG, PBM...)    
 //   init_file_format_menu(ff_list, max_index, ext);

    /* --- Make sure we keep the focus --- */
    gtk_grab_add (filew);

    gtk_main ();

    if (!selected_file_name)
	return NULL;
    return_string = (char *) x_malloc(strlen(selected_file_name)+1, "char (return_string in GetDirName)");
    strcpy(return_string,selected_file_name);
    return return_string;
}

