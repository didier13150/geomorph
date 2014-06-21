/* utils.c - Personal utilities for Gtk+ development
 *
 * Copyright (C) 2000 - 2010 Patrice St-Gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "utils.h"
#include "x_alloc.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include "../icons/attention.xpm"
#include "../icons/bombe.xpm"
#include "../icons/fleur.xpm"
#include "../icons/peur.xpm"
#include "../icons/info.xpm"
#include "../icons/downarrow.xpm"
#include "../icons/uparrow.xpm"


void swap_buffers (gpointer *a, gpointer *b) {
	gpointer c;
	c = *a;
	*a = *b;
	*b = c;
}
void order_int (gint *i1, gint *i2) {
//	Order 2 integers so that i2 > i1
	gint i;
	if ((*i2)>=(*i1))
		return;
	i = *i2;
	*i2 = *i1;
	*i1 = i;
	return;
}

gint gint_adj_callb (GtkWidget *wdg, gpointer data) {
	*((gint *) data) = (gint) GTK_ADJUSTMENT(wdg)->value;
	return FALSE;
}

gint gdouble_adj_callb (GtkWidget *wdg, gpointer data) {
	*((gdouble *) data) = (gdouble) GTK_ADJUSTMENT(wdg)->value;
	return FALSE;
}

gint gfloat_adj_callb (GtkWidget *wdg, gpointer data) {
	*((gfloat *) data) = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	return FALSE;
}

gint gboolean_set_true (GtkWidget *wdg, gpointer data) {
	*((gboolean *) data) = TRUE;
	return FALSE;
}

gint gboolean_set_false (GtkWidget *wdg, gpointer data) {
	*((gboolean *) data) = FALSE;
	return FALSE;
}

gint gboolean_set_false_event (GtkWidget *wdg, GdkEventButton *event, gpointer data) {
	*((gboolean *) data) = FALSE;
	return FALSE;
}

gint gboolean_set_true_event (GtkWidget *wdg, GdkEventButton *event, gpointer data) {
	*((gboolean *) data) = TRUE;
	return FALSE;
}


void zero_callb (GtkWidget *wdg, gpointer adj) {
	// Resets any scale adjustment to 0
	if (!GTK_IS_ADJUSTMENT(adj))
		return;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), 0.0);
}

gint toggle_check_button_callb (GtkWidget *checkbutton, gpointer data) {
//	printf("DATA in toggle_check_button_callb: %p\n",data);
	if (GTK_TOGGLE_BUTTON(checkbutton)->active) {
		*((gboolean *) data) = TRUE;
	}
	else
		*((gboolean *) data) = FALSE;
	return FALSE;
}

void hide_callb(GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;
	gtk_widget_hide(GTK_WIDGET(data));
}
void show_callb(GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;
	gtk_widget_show(GTK_WIDGET(data));
}
void set_flag_hide(GtkWidget *wdg, gpointer show_flag) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;
	*((gboolean *) show_flag) = FALSE;
}
void set_flag_show(GtkWidget *wdg, gpointer show_flag) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;
	*((gboolean *) show_flag) = TRUE;
}

GtkWidget *hideshow_dialog_new(GtkWidget *window, 
				GtkOrientation orient, 
				GtkWidget *wdg,
				gboolean *flag) {
//	Generates a toolbar allowing to hide / show a widget
//	"wdg" is the widget to show/hide
	GtkWidget *toolbar, *button1, *button2;
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar),orient);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
	gtk_widget_show(GTK_WIDGET(toolbar));
	
	button1 = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,_("Show"),
		_("Show the dialog"), NULL,
		create_widget_from_xpm(window,downarrow_xpm),
		GTK_SIGNAL_FUNC(show_callb),wdg);

	button2 = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,button1,_("Hide"),
		_("Hide the dialog"), NULL,
		create_widget_from_xpm(window,uparrow_xpm),
		GTK_SIGNAL_FUNC(hide_callb),wdg);
	if (flag)	{
		gtk_signal_connect (	GTK_OBJECT (button1), 
					"toggled", 
					GTK_SIGNAL_FUNC(set_flag_show),
					(gpointer) flag);
		gtk_signal_connect (	GTK_OBJECT (button2), 
					"toggled", 
					GTK_SIGNAL_FUNC(set_flag_hide),
					(gpointer) flag);
		if (*flag) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button1),TRUE);
			show_callb(button1, (gpointer) wdg);
		}
		else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button2),TRUE);
			hide_callb(button2, (gpointer) wdg);
		}
	}
	return toolbar;
}

void print_list (GList *list) {
	GList *node;
	gint i = 0;
	for (node = g_list_first(list); node; node = node->next)
		printf("(%d)  prev: %p;  CURRENT: %p;  next: %p\n",
			++i, (node->prev)?node->prev->data:NULL, 
			node->data, (node->next)?node->next->data:NULL);
}

void emit_expose_event (GtkWidget *window) {
//	Emit an expose event on a widget (typically a window)
//	Kind of dummy event, allowing testing of document updates
	static GdkEventExpose event;
	static gint w, h;
	if ( ! window)
		return;
	if ( ! GDK_IS_WINDOW(window->window) )
		return;
	gdk_window_get_geometry (window->window, NULL, NULL, &w, &h, NULL);
	event.type = GDK_EXPOSE;
	event.window = window->window;
	event.send_event = FALSE;
	event.area.x = 0;
	event.area.y = 0;
	event.area.width = w;
	event.area.height = h;
	event.count = 0;	
	// Change for GTK2
//	gtk_widget_event(window, (GdkEvent *) &event);		
	gdk_window_invalidate_rect (event.window, &event.area , TRUE);
}

int directory_exists (char *directory_name) {

//	Check if directory_name exists using the stat command

	struct stat *statres;
	statres = (struct stat *) x_calloc(sizeof(struct stat),1, "struct stat (statres in directory_exists in utils.c)");
	stat(directory_name, statres);
	if (statres && S_ISDIR(statres->st_mode)) {
		x_free(statres);
		return TRUE;
	}
	else {
		x_free(statres);
		return FALSE;
	}
}

int filexists(char *file_name) {
	FILE *test;
	if (!file_name) return FALSE;
	test = fopen(file_name,"r");
	if (test) {
		fclose(test);
		return TRUE;
	}
	else
		return FALSE;
}

gchar *add_filesep (gchar *dname) {
//	Add a separator at the end of a dir name, if it's absent
//	Returns a new pointer if a separator is added, otherwise returns the same pointer
//	The calling function must free the old pointer, if required

	gint i;
	gchar *buf;
	i = strlen(dname)-1;
	while ((dname[i]==' ') && (i>0)) i--; 	// Remove spaces
	if (!dname)
		return dname;
	if (dname[i] == FILESEP)
		return dname;
	buf = (gchar *) x_malloc(strlen(dname)+2, "gchar (buf in add_filesep in utils.c)");
	strcpy(buf,dname);
	buf[i+1]= FILESEP ;
	buf[i+2]='\0';
	return buf;
}

gchar *concat_dname_fname (gchar *dname, gchar *fname) {
//	Written for catenating a dir name and a file name, but can be used in other contexts
//	Add a separator at the end of dname if it doesn't exist
	gchar *buf;
	gint sep=0,i;
	i = strlen(dname)-1;
	while ((dname[i]==' ') && (i>0)) i--;	// Remove spaces
	if (dname) {
		if (dname[i]!= FILESEP) sep=1;
		buf = (gchar *) x_malloc(strlen(dname)+strlen(fname)+1+sep, "gchar (buf in concat_dname_fname)");
		strcpy(buf,dname);
		if (sep) {
			buf[i+1]= FILESEP ;
			buf[i+2]='\0';
		}
		strcat(buf, fname);
	}
	else	{
		buf = (gchar *) x_malloc(strlen(fname)+1, "gchar (buf in concat_dname_fname)");
		strcpy(buf, fname);
	}
	return buf;
}

void split_dir_file (gchar *path_n_file, gchar **dname, gchar **fname, gchar sep) {
//	Splits a path into a file name and a directory name, given the separator
//	Creates new strings if a path is found,
//	otherwise returns the original string in the file address
	gchar *separator;
	separator = (gchar *) strrchr(path_n_file, sep);
	if (separator) {
		(*fname) = (gchar *) x_malloc(strlen(separator)+1, "gchar (fname in split_dir_file)");
		strcpy((*fname), separator+1);
		(*dname) = (gchar *) x_malloc(separator+2-path_n_file, "gchar (dname in split_dir_file)");
		strncpy((*dname), path_n_file, separator+1-path_n_file);
		(*dname)[separator+1-path_n_file] = '\0';
	}
	else {
		(*fname) = path_n_file;
		(*dname) = NULL;
	}
}

gchar *get_dir_name(gchar *path_n_file, gchar sep) {
//	Copy the dir name from path_n_file
//	If no separator found, returns NULL
	gchar *separator, *dname;
	separator = (gchar *) strrchr(path_n_file, sep);
	if (separator) {
		dname = (gchar *) x_malloc(separator+2-path_n_file, "gchar (dname in get_dir_name)");
		strncpy(dname, path_n_file, separator+1-path_n_file);
		dname[separator+1-path_n_file] = '\0';
	}
	else
		dname = NULL;
	return dname;
}

gchar *get_file_name(gchar *path_n_file, gchar sep) {
//	Copy the file name from path_n_file
//	If no separator found, returns a new string with the path_n_file content
	gchar *separator, *fname;
	gint i;
	separator = (gchar *) strrchr(path_n_file, sep);
	if (separator) {
		fname = (gchar *) x_malloc(strlen(separator)+1, "gchar (fname in get_file_name)");
		strcpy(fname, separator+1);
	}
	else
	{
		fname = (gchar *) x_malloc(strlen(path_n_file)+1,"gchar (fname in get_file_name)" );
		memcpy(fname, path_n_file, strlen(path_n_file)+1);
	}
	// Remove spaces at the end
	i = strlen(fname)-1;
	while ((fname[i]==' ') && (i>0)) i--;
	fname[i+1]='\0';
	fname = (gchar *) x_realloc(fname,1+strlen(fname), "gchar (fname - realloc in get_file_name)");
	return fname;
}

char* init_default_dir() {
//	Default dir is $HOME or "" otherwise
//	This function can be used to initialize the global variable
//	"default_dir" declared in utils.h, or to initialize a local variable
//	with the returned string
	char *buf;
	if (default_dir)
		return default_dir;
	buf = getenv("HOME");
	if (!buf) {
		default_dir = (char *) x_malloc(1, "gchar (default_dir in init_default_dir)");
		strcpy(default_dir,"");
	}
	else {
		default_dir = (char *) x_malloc(strlen(buf)+2, "gchar (default_dir in init_default_dir)");
		strcpy(default_dir,buf);
		strcat(default_dir,"/");
	}
	return default_dir;
}

GtkWidget *create_widget_from_xpm (GtkWidget *reference_window, gchar **data) {
//	Less or more from Harlow's example
	GdkBitmap *mask;
	GdkPixmap *pm_data;
	GtkWidget *pm_widget;
	if (!data)
		return (NULL);
	if (GTK_IS_WIDGET(reference_window))
		pm_data = gdk_pixmap_create_from_xpm_d (reference_window->window, &mask, NULL, (gchar **) data);
	else
		pm_data = gdk_pixmap_create_from_xpm_d (NULL, &mask, NULL, (gchar **) data);
	pm_widget = gtk_pixmap_new(pm_data, mask);
	gtk_widget_show(pm_widget);
	return(pm_widget);
}

void msg_callb1(GtkWidget *widget,int flag_exit) {
	if (flag_exit == ABORT) {
		exit(0);
	}
}


void my_msg_subcall(gchar * message, int flag_exit, GtkWidget *dialog, GtkWidget *label1, GtkWidget *pixmap_widget, int if_vertical) {

	GtkWidget *label2, *ok, *hbox, *vbox;
	GMainLoop *loop;
	loop = g_main_new(FALSE);
//	To be sure the user sees the dialog, we display it in the center...
	gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER);
	// The message should be translated before calling my_msg
	// Here we don't use gettext
	// It could be a composed message created with sprintf, in that
	// case it won't translate correctly here
	label2 = gtk_label_new(message);
	gtk_misc_set_padding(GTK_MISC(label1), 10, 10);
	gtk_misc_set_padding(GTK_MISC(label2), 10, 10);
	vbox = gtk_vbox_new(FALSE,10);
	hbox = gtk_hbox_new(FALSE,10);
	gtk_box_pack_start(GTK_BOX(vbox),label1, FALSE, FALSE, 0);
	if (if_vertical)
		gtk_box_pack_start(GTK_BOX(vbox), pixmap_widget, FALSE, FALSE, 0);
	else
		gtk_box_pack_start(GTK_BOX(hbox), pixmap_widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),label2, FALSE, FALSE, 0); 
	gtk_box_pack_start(GTK_BOX(hbox),vbox, FALSE, FALSE, 0);
	ok = gtk_button_new_with_label(_("OK"));
	gtk_signal_connect_object(GTK_OBJECT(ok), "clicked",
		GTK_SIGNAL_FUNC(gtk_grab_remove), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(ok), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));
	gtk_signal_connect(GTK_OBJECT(dialog), "destroy",
		GTK_SIGNAL_FUNC(msg_callb1), (void *)(intptr_t)flag_exit);
	gtk_signal_connect_object(GTK_OBJECT(dialog), "destroy",
		GTK_SIGNAL_FUNC(g_main_loop_quit), (void *) loop);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),ok);
	gtk_button_box_set_layout (GTK_BUTTON_BOX(GTK_DIALOG(dialog)->action_area),  GTK_BUTTONBOX_SPREAD);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
	gtk_widget_show_all(dialog);
	gtk_grab_add(dialog);
	g_main_run (loop);
	g_main_destroy(loop);
}

void my_msg(gchar * message, gint flag_exit) {
	GtkWidget *pixmap_widget, *label, *dialog;
	dialog = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialog));
	switch (flag_exit) {
		case WARNING: 
			label = gtk_label_new(_("WARNING!"));
			pixmap_widget = create_widget_from_xpm(GTK_WIDGET(dialog),attention_xpm);
			break;
		case FAIL:
			label = gtk_label_new(_("ERROR!"));
			pixmap_widget = create_widget_from_xpm(GTK_WIDGET(dialog),peur_xpm);
			break;
		case ABORT:
			label = gtk_label_new(_("FATAL ERROR! The program will abort"));
			pixmap_widget = create_widget_from_xpm(GTK_WIDGET(dialog),bombe_xpm);
			break;
		case SORRY:
			label = gtk_label_new(_("SORRY!"));
			pixmap_widget = create_widget_from_xpm(GTK_WIDGET(dialog),fleur_xpm);
			break;
		case INFO:
			label = gtk_label_new(_("INFORMATION"));
			pixmap_widget = create_widget_from_xpm(GTK_WIDGET(dialog),info_xpm);
		default:
			label = gtk_label_new(_("UNKNOWN"));
			pixmap_widget = create_widget_from_xpm(GTK_WIDGET(dialog),info_xpm);
	}
	my_msg_subcall (message, flag_exit, dialog, label, pixmap_widget, FALSE);
}

void my_msg_with_image(gchar *title, gchar * message, GtkWidget *pixmap_widget) {

	GtkWidget *label, *dialog;
	dialog = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialog));
	label = gtk_label_new(_(title));
	my_msg_subcall (message, INFO, dialog, label, pixmap_widget, TRUE);
}

gint modal_delete_callb (GtkWidget *dummy, GdkEvent *dummy_event, gpointer *args) {
//	Kind of "wrapper" for callback OK/CANCEL after a delete window event
	cb_args_struct *cb_args;
	cb_args = (cb_args_struct *) args;
	if (cb_args->callb_fn) {
		(*cb_args->callb_fn) (dummy, cb_args->callb_data);
	}
	return TRUE;
}


void modal_dialog_with_titles_window_provided (GtkWidget *window,
			GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gchar *ok_title,
			gint (*cancel_callb) (GtkWidget *, gpointer),
			gchar *cancel_title,
			gpointer callb_data,
			GtkWindowPosition pos,
			gboolean if_ok_default) {

	// See modal_dialog_with_titles for explanation
	// Providing the window allows to connect to it events from the outer context.
	// For instance, a click in a widget contained in "wdg" could destroy the window
	GtkWidget *button, *okbox, *vbox;
	GMainLoop *loop;
	cb_args_struct *args;
	args = (cb_args_struct *) x_malloc(sizeof(cb_args_struct), "cbs_args_struct (args in modal_dialog_with_titles_windows_provided)");
	loop = g_main_new(FALSE);
	// "title" is not translated here, see the explanations in "my_msg"
	gtk_window_set_title (GTK_WINDOW (window), title);
	if (pos)
		gtk_window_set_position(GTK_WINDOW(window), pos);
	gtk_widget_realize(GTK_WIDGET(window));

//	Defines a vbox for containing from top to bottom: 1. "wdg"; 2. OK - Cancel	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

//	OK / cancel buttons appear in a hbox at the bottom
	okbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(okbox));
	button = gtk_button_new_with_label(_(ok_title));
	gtk_widget_show(GTK_WIDGET(button));
	gtk_container_set_border_width(GTK_CONTAINER(button),DEF_PAD);
	if (ok_callb)
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(ok_callb), (gpointer) callb_data);

	gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) window);

	gtk_box_pack_start(GTK_BOX(okbox), button, FALSE, FALSE, DEF_PAD);

	if (cancel_callb) {
		button = gtk_button_new_with_label(_(cancel_title));

		gtk_widget_show(GTK_WIDGET(button));

		gtk_container_set_border_width(GTK_CONTAINER(button),DEF_PAD);
			
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(cancel_callb), (gpointer) callb_data);
			
		gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer) window);
	
		gtk_box_pack_start(GTK_BOX(okbox), button, FALSE, FALSE, DEF_PAD);
	}

	if (if_ok_default)
	//	Closing the window means "OK";  
		args->callb_fn = ok_callb;
	else
	//	Closing the window means "Cancel";
		args->callb_fn = cancel_callb;

	args->callb_data = callb_data;	
		
	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		GTK_SIGNAL_FUNC(modal_delete_callb), (gpointer) args);	
		
	gtk_signal_connect_object(GTK_OBJECT(window), "destroy",
		GTK_SIGNAL_FUNC(gtk_grab_remove), (gpointer) window);

	gtk_signal_connect_object(GTK_OBJECT(window), "destroy",
		GTK_SIGNAL_FUNC(g_main_loop_quit), (gpointer) loop);

	gtk_box_pack_start(GTK_BOX(vbox), wdg, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), align_widget(okbox,0.5,0.5), FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	gtk_widget_show(GTK_WIDGET(window));
	gtk_grab_add(window);

	g_main_run (loop);

	g_main_destroy(loop);
}

void modal_dialog_with_titles (GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gchar *ok_title,
			gint (*cancel_callb) (GtkWidget *, gpointer),
			gchar *cancel_title,
			gpointer callb_data,
			GtkWindowPosition pos,
			gboolean if_ok_default) {

//	Packs "wdg" in a modal window, with an OK and an optional cancel button
//	Returns TRUE if OK, FALSE if CANCEL
//	If cancel_callb is NULL, omit the cancel button
//	If ok_callb is NULL, display an OK button with basic exit behavior
//	"wdg" is usually a container packing peculiar controls / widgets
//	"wdg" is destroyed with the window, so maybe the calling app should nullify it
//	Note:  closing the window means "OK" (calls the ok callback)
//	All terminating events stop the main loop
	GtkWidget *window;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	modal_dialog_with_titles_window_provided (window, wdg, title, ok_callb,
		ok_title, cancel_callb, cancel_title, callb_data, pos, if_ok_default);
}
			
void modal_dialog(GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gint (*cancel_callb) (GtkWidget *, gpointer),
			gpointer callb_data,
			GtkWindowPosition pos,
			gboolean if_ok_default) {
	modal_dialog_with_titles (wdg, title, ok_callb, "OK", cancel_callb, "Cancel", callb_data, pos, if_ok_default);
}

gint modal_delete_showhide (GtkWidget *window, GdkEvent *dummy_event, gpointer args) {
//	Kind of "wrapper" for callback OK/CANCEL after a delete window event
	cb_args_struct *cb_args;
	cb_args = (cb_args_struct *) args;
	if (cb_args->callb_fn) {
//		printf("Executing grabbed window callback function\n");
		if ((*cb_args->callb_fn) (window, cb_args->callb_data))
			return TRUE;
	}
	gtk_grab_remove(window);
	gtk_widget_hide(window);
//	printf("Delete event on %d\n", window);
	return TRUE;
}


void modal_dialog_showhide_with_titles (GtkWidget *window, 
			GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gchar *ok_title,
			gint (*cancel_callb) (GtkWidget *, gpointer),
			gchar *cancel_title,
			gpointer callb_data,
			GtkWindowPosition pos,
			gboolean if_ok_default) {

//	Same as modal_dialog, without a new loop
//	The dialog is intended to be permanent
//	Unrealized parent window must be given by the caller
//	"delete_event" hides the window and grab_removes it
//	The parent app can reactivate the dialog this way:
//		gtk_widget_show(GTK_WIDGET(given_window));
//		gtk_grab_add(GTK_WIDGET(given_window));

	GtkWidget *button, *okbox, *vbox;
	cb_args_struct *args;
	args = (cb_args_struct *) x_malloc(sizeof(cb_args_struct), "cbs_args_struct (args in modal_dialog_showhide_with_titles_windows)");
	// "title" is not translated here, see the explanations in "my_msg"
	gtk_window_set_title (GTK_WINDOW (window), title);
	if (pos)
		gtk_window_set_position(GTK_WINDOW(window), pos);
	gtk_widget_realize(GTK_WIDGET(window));

//	Defines a vbox for containing from top to bottom: 1. "wdg"; 2. OK - Cancel	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));
	
//	OK / cancel buttons appear in a hbox at the bottom
	okbox = gtk_hbox_new(TRUE,0);
	gtk_widget_show(GTK_WIDGET(okbox));
	button = gtk_button_new_with_label(ok_title);
	gtk_widget_show(GTK_WIDGET(button));
	gtk_container_set_border_width(GTK_CONTAINER(button),DEF_PAD);
	if (ok_callb)
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(ok_callb), (gpointer) callb_data);
	
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(gtk_grab_remove), (gpointer) window);
	gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_hide), (gpointer) window);

	gtk_box_pack_start(GTK_BOX(okbox), button, FALSE, FALSE, DEF_PAD);

	if (cancel_callb) {
		button = gtk_button_new_with_label(cancel_title);
		gtk_widget_show(GTK_WIDGET(button));
		gtk_container_set_border_width(GTK_CONTAINER(button),DEF_PAD);
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(cancel_callb), (gpointer) callb_data);
		gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(gtk_grab_remove), (gpointer) window);
		gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			GTK_SIGNAL_FUNC(gtk_widget_hide), (gpointer) window);
		gtk_box_pack_start(GTK_BOX(okbox), button, FALSE, FALSE, DEF_PAD);
	}

	if (if_ok_default)
	//	Closing the window means "OK";  
		args->callb_fn = ok_callb;
	else
	//	Closing the window means "Cancel";
		args->callb_fn = cancel_callb;

	args->callb_data = callb_data;

	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		GTK_SIGNAL_FUNC(modal_delete_showhide), (gpointer) args);

	gtk_box_pack_start(GTK_BOX(vbox), wdg, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), align_widget(okbox,0.5,0.5), FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	gtk_widget_show(window);
	gtk_grab_add(window);
}



void modal_dialog_showhide(GtkWidget *window, GtkWidget *wdg, gchar *title,
			gint (*ok_callb) (GtkWidget*, gpointer), 
			gint (*cancel_callb) (GtkWidget *, gpointer),
			gpointer callb_data,
			GtkWindowPosition pos,
			gboolean if_ok_default) {
	modal_dialog_showhide_with_titles (window, wdg, title, ok_callb, _("OK"), cancel_callb, _("Cancel"), callb_data, pos, if_ok_default);
}

void process_yes (GtkWidget *widget, int *yes_no) {
	*yes_no = TRUE;
}
void process_no (GtkWidget *widget, int *yes_no) {
	*yes_no = FALSE;
}
void process_cancel (GtkWidget *widget, int *yes_no) {
	*yes_no = CANCEL_YESNO;
}

gboolean delete_yesno (GtkWidget *widget, GdkEvent *event, gpointer data) {
	// Do nothing when the users tries to close the window
	return TRUE;
}

int yes_no(gchar * message, int default_choice) {

	GtkWidget *dialog, *label, *yes, *no;
	int yes_no;
	GMainLoop *loop;
	loop = g_main_new(FALSE);
	dialog = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialog));
//	We want to be sure the user sees the dialog...
	gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER);
	// "message" is not translated here, see the explanations in "my_msg"
	label = gtk_label_new(message);
	gtk_misc_set_padding(GTK_MISC(label), 10, 10);
	yes = gtk_button_new_with_label(_("Yes"));
	no = gtk_button_new_with_label(_("No"));
	gtk_signal_connect(GTK_OBJECT(yes), "clicked",
		GTK_SIGNAL_FUNC(process_yes),&yes_no);
	gtk_signal_connect(GTK_OBJECT(no), "clicked",
		GTK_SIGNAL_FUNC(process_no), &yes_no); 
	gtk_signal_connect(GTK_OBJECT(dialog), "delete_event", GTK_SIGNAL_FUNC(delete_yesno), NULL); 
	gtk_signal_connect_object(GTK_OBJECT(yes), "clicked",
		GTK_SIGNAL_FUNC(gtk_grab_remove), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(no), "clicked",
		GTK_SIGNAL_FUNC(gtk_grab_remove), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(yes), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(no), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(dialog), "destroy",
		GTK_SIGNAL_FUNC(g_main_loop_quit), (void *) loop);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), yes, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), no, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	GTK_WIDGET_SET_FLAGS(yes, GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(no, GTK_CAN_DEFAULT);
	if (default_choice)
		gtk_widget_grab_default(yes);
	else
		gtk_widget_grab_default(no);
	gtk_widget_show_all(dialog);
	gtk_grab_add(dialog);
	g_main_run (loop);
	g_main_destroy(loop);
	return yes_no;
}

int yes_no_cancel(gchar * message, int default_choice) {

	GtkWidget *dialog, *label, *yes, *no, *cancel;
	int yes_no;
	GMainLoop *loop;
	loop = g_main_new(FALSE);
	dialog = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialog));
//	We want to be sure the user sees the dialog...
	gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER);
//	The message should be already translated, see the explanations in my_msg
	label = gtk_label_new(message);
	gtk_misc_set_padding(GTK_MISC(label), 10, 10);
	yes = gtk_button_new_with_label(_("Yes"));
	no = gtk_button_new_with_label(_("No"));
	cancel = gtk_button_new_with_label(_("Cancel"));
	gtk_signal_connect(GTK_OBJECT(yes), "clicked",
		GTK_SIGNAL_FUNC(process_yes),&yes_no);
	gtk_signal_connect(GTK_OBJECT(no), "clicked",
		GTK_SIGNAL_FUNC(process_no), &yes_no); 
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
		GTK_SIGNAL_FUNC(process_cancel), &yes_no);
	gtk_signal_connect(GTK_OBJECT(dialog), "delete_event", GTK_SIGNAL_FUNC(delete_yesno), NULL); 
	gtk_signal_connect_object(GTK_OBJECT(yes), "clicked",
		GTK_SIGNAL_FUNC(gtk_grab_remove), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(no), "clicked",
		GTK_SIGNAL_FUNC(gtk_grab_remove), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(cancel), "clicked",
		GTK_SIGNAL_FUNC(gtk_grab_remove), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(yes), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(no), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(cancel), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(dialog));
	gtk_signal_connect_object(GTK_OBJECT(dialog), "destroy",
		GTK_SIGNAL_FUNC(g_main_loop_quit), (void *) loop);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), yes, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), no, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), cancel, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	GTK_WIDGET_SET_FLAGS(yes, GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(no, GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
	switch (default_choice) {
		case TRUE: 
			gtk_widget_grab_default(yes);
			break;
		case FALSE:
			gtk_widget_grab_default(no);
			break;
		case CANCEL_YESNO:
			gtk_widget_grab_default(cancel);
	}
	gtk_widget_show_all(dialog);
	gtk_grab_add(dialog);
	g_main_run (loop);
	g_main_destroy(loop);
	return yes_no;
}

int findnextrec (FILE *file,char *rec)
{
	/* Find next record, skipping comments */
	int i;
	while (1) {
		if (!fgets(rec,250,file))
			return FALSE;
		/* Find next char which is not blank */
		for (i=0;i<strlen(rec);i++)
			if (rec[i]!=' ') break;
		if (rec[i]!='#')
			break;
	}
	return TRUE;
}

char *remove_spaces_both_ends (char *str) {
	// Remove spaces at both ends of a string
	// Allocates a new pointer
	int max_length, i, j;
	char *s;
	max_length = strlen(str);
	if (!max_length)
		return str;
	i = max_length-1;
	// Removing spaces at the end
	while ( (i>=0) && (str[i]==' ') ) {
		i--;
	}
	
	i++;
//	printf("STR[%d]: %d\n",i,str[i]);
	
	s = (char *) x_malloc(i+1, "char (s in remove_spaces_both_ends)");
	if (i)
		strncpy(s,str,i);
	s[i]='\0';
	if (!i) // empty string
		return s;
	
	max_length = i;
	// Removing spaces at the beginning
	i = 0;
	while ( (i<=max_length) && (s[i]==' ') )
		i++;

	if (i) { // There is something to remove
		for (j=0; (j+i)<=(max_length+1) ; j++)
			s[j] = s[i+j];
		return (char *) x_realloc(s, j+1, "char (s - realloc in remove_spaces_both_ends)");
	}
	else
		return s;
}


char *strupp(char *my_string) {
//	Converts my_string to uppercase
	int i;
	char *buf;
	buf = (char *) x_malloc(strlen(my_string)+1, "char (buf in strupp)");
	for (i=0; i<strlen(my_string); i++)
		buf[i]=(char) toupper((int) my_string[i]);
	buf[i]='\0';
	return buf;
}

char *strlow(char *my_string) {
//	Converts my_string to lowercase
	int i;
	char *buf;
	buf = (char *) x_malloc(strlen(my_string)+1, "char (buf in strlow)");
	for (i=0; i<strlen(my_string); i++)
 		buf[i]=(char) tolower((int) my_string[i]);
	buf[i]='\0';
	return buf;
}

gboolean is_integer(gchar *buf) {
	//	Check if buf is a valid integer
	gint i;
	if (!strlen(buf))
		return FALSE;
	for (i = 0; i<strlen(buf); i++) {
		if ( (!i) && ((*(buf+i))=='-')) // Negative sign
			continue;  
		if (!isdigit(*(buf+i)))
			return FALSE;
	}
	return TRUE;
}

gboolean is_float(gchar *buf) {
	//	Check if buf is a valid float in the 9999,9... format
	//	Rough test - to be improved
	//	Doesn't check the 999 E+99 format
	//	We don't consider the locale, but assume
	//	that , or . are valid decimal separators
	//	No comma allowed for thousands separators!!
	gint i;
	gboolean beginning_spaces=TRUE, if_comma=FALSE;
//	printf("BUF: %s\n",buf);
	if (!strlen(buf))
		return FALSE;
	for (i = 0; i<strlen(buf); i++) {
//		printf("%d = %c\n", i, (char) *(buf+i));
		if (beginning_spaces) {
			if ((*(buf+i))==' ')
				continue;
		}
		if ( beginning_spaces && ((*(buf+i))=='-')) { // Negative sign
			beginning_spaces = FALSE;
			continue;
		}
		beginning_spaces = FALSE;
		if (!isdigit(*(buf+i))) {
			if (if_comma) // 2nd comma...
				return FALSE;
			if ( ((*(buf+i))!=',') && ( (*(buf+i))!='.') )
				return FALSE;
			else
				if_comma=TRUE;
		}
	}
	return TRUE;
}

gdouble string_to_double (gchar *buf) {
	// As the title says...
	// Added 2005-04-26, for processing the geomorphrc file without
	// considering the locale
	// We assume that , or . could be decimal separators
	// Doesn't work with the 999 E+99 format
	gdouble result=0.0;
	gint i, digits;
	gboolean beginning_spaces=TRUE, if_comma=FALSE, negative = FALSE;
	gchar *new_buf;
//	printf("BUF: %s\n",buf);
	if (!strlen(buf))
		return result;
		
	new_buf = (gchar *) x_malloc(1+(sizeof(gchar)*strlen(buf)), "gchar (new_buf in string_to_double)");
	
	digits = 0;
	for (i = 0; i<strlen(buf); i++) {
//		printf("%d = %c\n", i, (char) *(buf+i));
		if (beginning_spaces) {
			if ((*(buf+i))==' ')
				continue;
		}
		if ( beginning_spaces && ((*(buf+i))=='-'))  { // Negative sign
			beginning_spaces = FALSE;
			negative = TRUE;
			continue;
		}
		beginning_spaces = FALSE;
		if (!isdigit(*(buf+i))) {
			if (if_comma) { // 2nd comma, not supposed to, stop there
				x_free(new_buf);
				if (negative)
					result *= -1;
				return result;
			}
			if ( ((*(buf+i))!=',') && ( (*(buf+i))!='.') ) {
				x_free(new_buf);
				if (negative)
					result *= -1;
				return result;
			}
			else {
				if_comma=TRUE;
				*(new_buf+digits) = '\0';
				result = atof(new_buf);
				digits = 0;
			}
		}
		else {
			*(new_buf+digits) = *(buf+i); 
			digits++;
		}
	}
	*(new_buf+digits) = '\0';
	result += atof(new_buf) / pow(10.0,(gdouble)digits);
	x_free(new_buf);
	if (negative)
		result *= -1;
	return result;
}

GtkWidget * define_button_in_table (char *txt, GtkWidget *table, gint colfrom,
	gint colto, gint linefrom, gint lineto, gint pad)
{
	GtkWidget *button;
	button = gtk_button_new_with_label (_(txt));
	gtk_table_attach (GTK_TABLE (table), button, colfrom, colto, linefrom, lineto, 0, 0, pad, pad);
	gtk_widget_show(button);
	return button;
}

GtkWidget * define_check_button_in_table (char *txt, GtkWidget *table, gint colfrom,
	gint colto, gint linefrom, gint lineto, gint pad)
{
	GtkWidget *button;
	button = gtk_check_button_new_with_label (_(txt));
	gtk_table_attach (GTK_TABLE (table), button, colfrom, colto, linefrom, lineto, 0, 0, pad, pad);
	gtk_widget_show(button);
	return button;
}
GtkWidget * define_check_button_in_box (char *txt, GtkWidget *box, gint expand, gint fill, gint pad)
{
	GtkWidget *button;
	button = gtk_check_button_new_with_label (_(txt));
	gtk_box_pack_start(GTK_BOX(box), button, expand, fill, pad);
	gtk_widget_show(button);
	return button;
}
GtkWidget *define_label_in_box(char *txt, GtkWidget *box, gint expand, gint fill, gint pad)
{
	GtkWidget *label;
	label = gtk_label_new(_(txt));
	gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
	gtk_widget_show (label);
	gtk_box_pack_start(GTK_BOX(box), label, expand, fill, pad);
	return label;
}

GtkWidget * define_scale_in_table(GtkObject *adj, GtkWidget *table, gint colfrom, gint colto,
		gint linefrom, gint lineto, gint digits, gint pad)
{
	GtkWidget *scale;
    	scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_scale_set_digits (GTK_SCALE (scale), digits); 
	gtk_widget_set_usize(scale,80,0);
//	gtk_table_attach (GTK_TABLE (table), scale, colfrom, colto, linefrom, lineto, 
//		GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, pad, pad);
	gtk_table_attach_defaults (GTK_TABLE (table), scale, colfrom, colto, linefrom, lineto); 
	gtk_widget_show (scale);
	return(scale);
}

GtkWidget * define_scale_in_box(GtkObject *adj, GtkWidget *box,  gint digits, gint pad)
{
	GtkWidget *scale;
	if (GTK_IS_HBOX(GTK_OBJECT(box))) {
	    	scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
		gtk_widget_set_usize(scale,75,0);
	}
	else {
	    	scale = gtk_vscale_new (GTK_ADJUSTMENT (adj));
		gtk_widget_set_usize(scale,0,75);
	}
	gtk_scale_set_digits (GTK_SCALE (scale), digits); 
	gtk_box_pack_start (GTK_BOX (box), scale, TRUE, TRUE, pad); 
	gtk_widget_show (scale);
	return(scale);
}

GtkWidget *define_label_in_table_align (char *txt,GtkWidget *table, gint colfrom, gint colto, 
	gint linefrom, gint lineto, gint pad, gfloat horiz, gfloat vertic)
{
	GtkWidget *label,*lbl;
	label = gtk_label_new(_(txt));
	gtk_widget_show (label);
	gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
	if ((horiz==ALIGN_CENTER) && (vertic==ALIGN_CENTER))
		lbl = label;
	else
		lbl = align_widget(label,horiz,vertic);
	gtk_table_attach (GTK_TABLE (table), lbl, colfrom, colto, linefrom, lineto, 
		GTK_FILL, GTK_FILL, pad, pad);
	return label;
}

GtkWidget *define_label_in_table(char *txt,GtkWidget *table, gint colfrom, gint colto, 
	gint linefrom, gint lineto, gint pad)
{
	return define_label_in_table_align(txt, table, colfrom, colto,
		linefrom, lineto, pad, ALIGN_CENTER, ALIGN_CENTER);
}

GtkWidget * define_entry_in_table_align(char *txt,GtkWidget *table, gint colfrom, gint colto,
	gint linefrom, gint lineto, int length, int widget_len, gint pad, gfloat horiz, gfloat vertic) 
{	
	// "length" is in characters
	// "widget_len" is in pixels (seems to be)
	GtkWidget *entry, *entr;
	entry = gtk_entry_new_with_max_length(length);
	gtk_widget_show(entry);
	gtk_widget_set_usize(GTK_WIDGET(entry),widget_len,0);
	gtk_entry_set_text(GTK_ENTRY(entry),txt);
	if ((horiz==ALIGN_CENTER) && (vertic==ALIGN_CENTER))
		entr = entry;
	else
		entr = align_widget(entry,horiz,vertic);
	gtk_table_attach (GTK_TABLE (table), entr, 
		colfrom, colto, linefrom, lineto, 
		GTK_FILL, GTK_FILL, pad, pad); 
	return(entry);
}

GtkWidget * define_entry_in_table(char *txt,GtkWidget *table, gint colfrom, gint colto,
	gint linefrom, gint lineto, int length, int widget_len, gint pad) 
{	
	return define_entry_in_table_align(txt,table,colfrom,colto, linefrom, 
		lineto, length,widget_len,pad, ALIGN_CENTER, ALIGN_CENTER);
}

GtkWidget * define_entry_in_box(char *txt, GtkWidget *box, gint expand,
		gint fill, gint pad, int length, int widget_len)
{
	GtkWidget *entry;
	entry = gtk_entry_new_with_max_length(length);
	gtk_widget_set_usize(GTK_WIDGET(entry),widget_len,0);
	gtk_entry_set_text(GTK_ENTRY(entry),txt);
	gtk_box_pack_start(GTK_BOX(box), entry, expand, fill, pad);
	gtk_widget_show(entry);
	return(entry);
}

GtkWidget * define_radio_button_in_box (GtkWidget *box, GSList **group, char *txt,
	void (*callback_fn) (GtkWidget *, gpointer ),
	int if_default ) {

//	This form uses the label text as the callback data
//	Data is flowed from the widget to the application through a global variable
//	which is initialized in the callback function, from the label
	GtkWidget *button;
	button = gtk_radio_button_new_with_label (*group, _(txt));
    	gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
	*group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	if (if_default)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	gtk_widget_show(button);
	if (callback_fn)
		gtk_signal_connect (	GTK_OBJECT (button), 
					"toggled", 
					GTK_SIGNAL_FUNC(callback_fn),
					(gpointer) txt);
	return button;
}

GtkWidget * define_radio_button_in_box_with_data (GtkWidget *box, GSList **group, char *txt,
	void (*callback_fn) (GtkWidget *, gpointer ),
	gpointer callb_data,
	int if_default ) {

//	This forms implies that the callback data is the structure modified with the
//	information drawed from the radio button label
//	The callback function must tests the radio button label like this:
//		gchar *txt;
//		gtk_label_get(GTK_LABEL(GTK_BIN(radio_button)->child), &txt);

	GtkWidget *button;
	button = gtk_radio_button_new_with_label (*group, _(txt));
    	gtk_box_pack_start (GTK_BOX (box), button, TRUE, TRUE, 0);
	*group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	gtk_widget_show(button);
	if (callback_fn)
		gtk_signal_connect (	GTK_OBJECT (button), 
					"toggled", 
					GTK_SIGNAL_FUNC(callback_fn),
					callb_data);
	if (if_default) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
//		printf("Activating %s\n",txt);
	}
	return button;
}

GtkWidget * define_radio_button_in_table_with_data (GtkWidget *table, GSList **group, char *txt,
	void (*callback_fn) (GtkWidget *, gpointer ),
	gpointer callb_data, gint if_default, gint colfrom, gint colto, gint linefrom, gint lineto ) {

//	This forms implies that the callback data is the structure modified with the
//	information drawed from the radio button label
//	The callback function must tests the radio button label like this:
//		gchar *txt;
//		gtk_label_get(GTK_LABEL(GTK_BIN(radio_button)->child), &txt);

	GtkWidget *button;
	button = gtk_radio_button_new_with_label (*group, _(txt));
	
	gtk_table_attach (GTK_TABLE (table), button, colfrom, colto, linefrom, lineto, 0, 0, 0, 0); 

	*group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	if (if_default)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	gtk_widget_show(button);
	if (callback_fn)
		gtk_signal_connect (	GTK_OBJECT (button), 
					"toggled", 
					GTK_SIGNAL_FUNC(callback_fn),
					callb_data);
	return button;
}

int find_omenu_option (omenu_struct_type *omenu, char *label) {
//	Finds the option menu index from label
	int i;
	char *buf1, *buf2;
	buf1 = strupp(label);
	for (i=0; i<omenu->max_index; i++) {
		buf2 = strupp(omenu->list[i].lbl);
		if (!strcmp(buf1,buf2)) {
			x_free(buf2);
			break;
		}
		x_free(buf2);
	}
	x_free(buf1);
	if (i == omenu->max_index)
		return 0;
	else
		return i;
}

int find_omenu_option_num(omenu_struct_type *omenu, int value) {
//	Finds the option menu index from the numeric value
	int i;
	for (i=0; i<omenu->max_index; i++)
		if (value == omenu->list[i].num)
			break;
	if (i == omenu->max_index)
		return 0;
	else
		return i;
}

int find_omenu_num (omenu_list_type *list, int value, int max) {
//	Finds the option menu index from the numeric value
//	Same as preceding, but the omenu_struct_type is not known
	int i;
	for (i=0; i<max; i++) {
		if (value == list[i].num)
			break;
	}
	if (i == max)
		return 0;
	else {
// printf("FIND_OMENU_NUM - valeur cherchée: %d;  i: %d; v. cour: %d\n",value,i, list[i].num);
		return i;
	}
}

void menu_choice (omenu_list_type *item_list)
{
	/* Initialise current label and index */
	((omenu_struct_type *) item_list->parent_omenu)->current_lbl = _(item_list->lbl);
	((omenu_struct_type *) item_list->parent_omenu)->current_index = 
		find_omenu_option(
			(omenu_struct_type *) item_list->parent_omenu,
			item_list->lbl );
// printf("ITEM_LIST->lbl: %s\n",item_list->lbl);
//	/* Is there a function pointer to execute ? */
//	Done in define_menu_item 2001.12.16
//	if (item_list->callback_fn)
//		(*item_list->callback_fn)((gpointer *) item_list->lbl);
}

GtkWidget * define_menu_item (omenu_struct_type *omenu, omenu_list_type *omenu_list, 
	GSList **group, GtkWidget *menu)
{
	GtkWidget *menuitem;
	menuitem = gtk_radio_menu_item_new_with_label
		(*group,_(omenu_list->lbl));
	*group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM(menuitem));
	gtk_menu_append(GTK_MENU(menu),menuitem);
	gtk_widget_show(menuitem);
	gtk_signal_connect_object(GTK_OBJECT(menuitem), "activate",
		GTK_SIGNAL_FUNC(menu_choice), (gpointer) omenu_list);
	if (omenu_list->callback_fn) {
		// Is there a function pointer to execute ? 
		if (!omenu->data) 
		// Old version:  the data was the label
			gtk_signal_connect_object(GTK_OBJECT(menuitem), "activate",
				GTK_SIGNAL_FUNC(omenu_list->callback_fn), 
				(gpointer) omenu_list->lbl);
		else 
			gtk_signal_connect_object(GTK_OBJECT(menuitem), "activate",
				GTK_SIGNAL_FUNC(omenu_list->callback_fn), 
				(gpointer) omenu);
	}
	return menuitem;
}

GtkWidget * define_omenu (omenu_struct_type *omenu_str, int max, int def)
{
	/* Defines an option menu and sets the default value */
	int i,j=0;
	GSList *group=NULL;
	GtkWidget *omenu, *menu;
	omenu = gtk_option_menu_new();
	menu = gtk_menu_new();

	for(i=0;i<max;i++) {
		if (i==def) 
			j=i;
		omenu_str->list[i].menuitem = 
			define_menu_item (omenu_str,&omenu_str->list[i], &group, menu);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU(omenu),menu);
	gtk_widget_show(omenu);
	/* Toggle, show, initialise default value */
	gtk_option_menu_set_history(GTK_OPTION_MENU(omenu),j);
	gtk_menu_item_activate(GTK_MENU_ITEM(omenu_str->list[j].menuitem)); 
	omenu_str->current_lbl = _(omenu_str->list[j].lbl);
	return omenu;
}

int create_omenu (omenu_struct_type **new_omenu,
	omenu_list_type *list,
	int max_index,
	int def_index) 
{

	// Creates an Option Menu Structure from an undefined
	// omenu_struct_type pointer, a defined list of options
	// and a few indexes - maximum + default

	int i;
	if (NULL == ((*new_omenu) = 
		(omenu_struct_type *) x_malloc(sizeof(omenu_struct_type), "omenu_struct_type (new_omenu in create_omenu)"))) 
		return FALSE;

	(*new_omenu)->list = list;

	(*new_omenu)->max_index = max_index;

	(*new_omenu)->def_index = def_index;

	(*new_omenu)->data = NULL;

	// We need to know the parent pointer in the child structure
	for (i=0; i<max_index; i++) 
		(*new_omenu)->list[i].parent_omenu = *new_omenu;

	(*new_omenu)->omenu_ptr = define_omenu(*new_omenu, max_index, def_index);

	return TRUE;
}


int create_omenu_with_callb_data (omenu_struct_type **new_omenu,
	omenu_list_type *list,
	int max_index,
	int def_index,
	gpointer data) 
{

	// Creates an Option Menu Structure from an undefined
	// omenu_struct_type pointer, a defined list of options
	// and a few indexes - maximum + default

	int i;
	if (NULL == ((*new_omenu) = 
		(omenu_struct_type *) x_malloc(sizeof(omenu_struct_type),"omenu_struct_type (new_omenu in create_omenu_with_callb_data)" ))) 
		return FALSE;

	(*new_omenu)->list = list;

	(*new_omenu)->max_index = max_index;

	(*new_omenu)->def_index = def_index;

	(*new_omenu)->data = data;

	// We need to know the parent pointer in the child structure
	for (i=0; i<max_index; i++) 
		(*new_omenu)->list[i].parent_omenu = *new_omenu;

	(*new_omenu)->omenu_ptr = define_omenu(*new_omenu, max_index, def_index);

	return TRUE;
}

void set_option_menu_from_label (GtkWidget *omenu, gchar *label) {
//	Written 2002.09
//	A lot simpler than the preceding functions
//	(no need of a complex option menu struct, all data can be found
//	in the option menu... when one finally understands the widget :-( )
//	2005-02: seems to work with GTK2 (at least when the menu item is a label!)
	GtkWidget *menu;
	GList *node;
	gchar *txt,*lbl;
	lbl = remove_spaces_both_ends(label);
	if (GTK_IS_LABEL(GTK_BIN(omenu)->child)) {
		gtk_label_get(GTK_LABEL(GTK_BIN(omenu)->child), &txt);
		if (!strcmp(txt,lbl))  // Nothing to do, the label is already set
			return;	
	}
	menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(omenu));
	node = GTK_MENU_SHELL(menu)->children;
	while (node) {
		if (GTK_IS_LABEL(GTK_BIN(node->data)->child)) {
			gtk_label_get(GTK_LABEL(GTK_BIN(node->data)->child), &txt);
			if (!strcmp(txt,lbl)) {
				break;
			}
		} // If the child is not a label (typically it would be NULL),
		 // we skip it because the menu item is the current one
		node = node->next;
	}
	if (node)	{	// Label found
		gtk_menu_item_activate(GTK_MENU_ITEM(node->data));
		gtk_option_menu_set_history(GTK_OPTION_MENU(omenu),
			g_list_index(GTK_MENU_SHELL(menu)->children, node->data));
	}
	else {		// Label not found, revert to default 
		gtk_menu_item_activate(GTK_MENU_ITEM(GTK_MENU_SHELL(menu)->children->data));
		gtk_option_menu_set_history(GTK_OPTION_MENU(omenu),0);
	}
	free(lbl);
}

int my_exec(char *com, char **argum, int (*exec_to_use) (const char *, char *arg[])) {

/* "Wrapper" for execution of a command with execvp or execv
   Control returned to parent process (uses fork)
   argum must end with a NULL!!
   ... and preferably begin with the command to execute
   (we tested it!)
*/

int i=0;
pid_t pid;
char **buf=NULL, *bufmsg=NULL;

// printf("*Argum: %s == %s : %d\n" ,*argum,com, strcmp(*argum,com));

// Arguments must begin with the command to execute 
if (strcmp(*argum,com)) { // No
	while (*(argum+i)!=NULL) i++;
	buf = (char **) x_calloc((i+2),sizeof(*argum), "*argum (buf in my_exec)");
// printf("Allocation: %d:%d\n",i+2,(i+2)*sizeof(*argum));
	*buf = com;
	while (i+1) {
		*(buf+i+1) = *(argum+i);
// printf("I: %d; BUF: %s; ARGUM: %s\n", i, *(buf+i+1), *(argum+i));
		i--;
	} 
}
else {
	buf = argum;
}

// printf("com: %s; Buf 0: %s, Buf 1: %s, Buf 2: %s, Buf 3: %s, Buf 4: %s; Buf 5: %s\n", com, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

pid = fork (); 

// printf("PID: %d\n",pid);

if (pid == 0) {  
 // Child process OK, execute the command
	if ((*exec_to_use) (com, buf)<0) {  // Command not OK
		bufmsg = (char *) x_malloc(5+strlen(strerror(errno))+
			strlen(com), "char (bufmsg in my_exec)");
		sprintf(bufmsg,"%s: %s",com,strerror(errno));

		my_msg(bufmsg,FAIL);

		x_free(bufmsg);
		printf("%s: %s\n", com, strerror(errno));
		_exit(-1);	
	}
} 
else 	
	if (pid < 0) { 
	/* Error, no child process, Parent process OK */
		my_msg(strerror(errno),FAIL);
	}
	else {	// pid > 0, Parent process OK, child OK
		// Remove zombie process, if any
// printf("PID: %d\n",pid);
		waitpid(pid, NULL, WNOHANG);
	}

if (buf && (buf != argum)) x_free(buf);
	
return pid; 
}

int execution(char *file, char **argum) {
	return my_exec(file, argum, (gpointer) execvp);
}

// int execution(char *path, char **argum) {
//	return my_exec(path, argum, (gpointer) execv);
//}

int log2i(gint x) {
	gint i;
	i = 0;
	while (x) {
		i++;
		x = x>>1;
	}
	return(i-1);
}

void set_sensitive_callb (GtkWidget *wdg, gpointer widget) {
	// Enable "widget" when an event reaches "wdg"
	gtk_widget_set_sensitive(GTK_WIDGET(widget),TRUE);
}

void unset_sensitive_callb (GtkWidget *wdg, gpointer widget) {
	// Disable "widget" when an event reaches "wdg"
	gtk_widget_set_sensitive(GTK_WIDGET(widget),FALSE);
}

void show_if_true(GtkWidget *check_button, gpointer widget) {
//	Show widget if button is true, hide if false
	if (GTK_TOGGLE_BUTTON(check_button)->active)
		gtk_widget_show(GTK_WIDGET(widget));
	else
		gtk_widget_hide(GTK_WIDGET(widget));
}
void hide_if_true(GtkWidget *check_button, gpointer widget) {
//	Hide widget if button is true, show if false
	if (GTK_TOGGLE_BUTTON(check_button)->active)
		gtk_widget_hide(GTK_WIDGET(widget));
	else
		gtk_widget_show(GTK_WIDGET(widget));
}

void sensitive_if_true(GtkWidget *check_button, gpointer widget) {
//	Make widget sensitive if button is true, make unsensitive if false
	gtk_widget_set_sensitive(GTK_WIDGET(widget), GTK_TOGGLE_BUTTON(check_button)->active);
}
void unsensitive_if_true(GtkWidget *check_button, gpointer widget) {
//	Make widget sensitive if button is false, make sensitive if true
	gtk_widget_set_sensitive(GTK_WIDGET(widget), !GTK_TOGGLE_BUTTON(check_button)->active);
}

GtkWidget *align_widget(GtkWidget *wdg, gfloat horizontal, gfloat vertical) {
//	Aligns a widget, returns the GTK_ALIGNMENT created for this purpose
//	No expansion control ("scale")
	GtkWidget *align_wdg;
	align_wdg = gtk_alignment_new(horizontal,vertical,0.0,0.0);
	gtk_container_add(GTK_CONTAINER(align_wdg), wdg);
	gtk_widget_show(GTK_WIDGET(align_wdg));
	return align_wdg;
}

GtkWidget *frame_new(gchar *label, gint pad) {
	//	Standard boring frame creation
	GtkWidget *frame;
	frame = gtk_frame_new(_(label));
	gtk_widget_show(GTK_WIDGET(frame));
	gtk_frame_set_label_align(GTK_FRAME(frame),0,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), pad);
	return frame;
}

gint adjust_adj_callb (GtkAdjustment *adj_trigger, gpointer adj_toadjust) {
	GtkAdjustment *adj;
	adj = (GtkAdjustment *) adj_toadjust;
	gtk_adjustment_set_value (adj, gtk_adjustment_get_value (GTK_ADJUSTMENT(adj_trigger)));
	return TRUE; // Don't trigger other adj. changes
}

void synchro_scr_win (GtkWidget *scr_win1, GtkWidget *scr_win2) {
	// Synchronizes scrolled windows
	// Windows should be of the same size
	GtkAdjustment *vadj1, *vadj2, *hadj1, *hadj2;
	vadj1 = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(scr_win1));
	hadj1 = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(scr_win1));
	vadj2 = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(scr_win2));
	hadj2 = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(scr_win2));
	gtk_signal_connect (GTK_OBJECT (vadj1), "value_changed",
		(GtkSignalFunc) adjust_adj_callb, (gpointer) vadj2);
	gtk_signal_connect (GTK_OBJECT (vadj2), "value_changed",
		(GtkSignalFunc) adjust_adj_callb, (gpointer) vadj1);
	gtk_signal_connect (GTK_OBJECT (hadj1), "value_changed",
		(GtkSignalFunc) adjust_adj_callb, (gpointer) hadj2);
	gtk_signal_connect (GTK_OBJECT (hadj2), "value_changed",
		(GtkSignalFunc) adjust_adj_callb, (gpointer) hadj1);
}

/* Create a new hbox with an image and a label packed into it
 * and return the box. */
// Adapted from the GTK+ 2.x tutorial

GtkWidget *xpm_label_box (GtkWidget *window, gchar **xpm, gchar *label_text )
{
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *image;

    /* Create box for image and label */
    box = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (box), 2);

    /* Now on to the image stuff */
    image = create_widget_from_xpm (window,xpm) ;

    /* Create a label for the button */
    label = gtk_label_new (label_text);

    /* Pack the image and label into the box */
    gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 3);
    gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 3);

    gtk_widget_show (image);
    gtk_widget_show (label);

    return box;
}

