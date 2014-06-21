/* Main.c - standard application frame
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

// get_current_dir_name() is only prototyped if _GNU_SOURCE is defined
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "./main.h"
#include "./globals.h"
#include "./app.h"
#include "../utils/config_dialog.h"
#include "../hf/camera.h"
#include <unistd.h>
// Gtkglext - 2005-11
#include <gdk/gdkgl.h> 
#include <gtk/gtkgl.h>
#include "../icons/GeoMorph_16x16.xpm"
#include "../icons/GeoMorph_32x32.xpm"
#include "../icons/GeoMorph_48x48.xpm"

// Global variables for no_mem_exit
static app_struct *MAIN_APP = NULL ; 
static gboolean INTERFACE_ACTIVE = FALSE;

static void no_mem_exit (gchar *struct_name) {
	// Used with the x_alloc.h functions
	my_msg ("No more memory to allocate for %s; trying to exit normally\n",WARNING);
	if (MAIN_APP && INTERFACE_ACTIVE) {
		app_quit (NULL, MAIN_APP);
	}
	else
		exit (5);
}

/* Returns a char * that contains the path to the geomorph data dir
   NOTE: This is a pointer to a statically allocated block of memory
   and should not be modified or freed */

gchar *get_data_dir() {
#ifdef GEOMORPHDATADIR
  return GEOMORPHDATADIR;
#else
  return "/usr/local/share/geomorph/" ## VERSION;
#endif
}

void create_config_file(gchar *rc_file) {
	gchar *buf, *command;
	buf = (gchar *) x_malloc(strlen(rc_file)+1+strlen(_("Creation of the option file %s")), "const gchar (buf - Creation of the option file MSG)");
	sprintf(buf,_("Creation of the option file %s"),rc_file);
	my_msg(buf,INFO);
	command = concat_dname_fname(get_data_dir(),"install-step2-rcfile");
	if (system(command))
		my_msg(_("Fatal error during the creation of the option file"),ABORT);
	if (command) x_free(command);
}

gchar *find_config_file() {
//	Returns the config file name (name OPTION_FILE, defined in globals.h),
//	with its full path
	gchar *default_dir, *path_n_file, *buf, *command;
	gboolean shortcuts;

//	We try:
//	1. in the current directory. (./OPTION_FILE = ./geomorphrc)
//	2. if not found, we try ./DEF_DIR_NAME/OPTION_FILE (./geomorph/geomorphrc)
//	3. if not found, we try $HOME/OPTION_FILE (~/geomorphrc)
//	4. if not found, we try in $HOME/DEF_DIR_NAME (~/geomorph/geomorphrc) 
//	--> if OPTION_FILE was not found, and 
//	if $HOME/DEF_DIR_NAME doesn't exist, we create the required files
//	The directory where OPTION_FILE is found becomes the default one (DEF_DIR)

	// Try ./OPTION_FILE
	default_dir = (gchar *) get_current_dir_name();
	path_n_file = concat_dname_fname(default_dir,OPTION_FILE);
	if (!filexists(path_n_file)) { // Try ./DEF_DIR_NAME/OPTION_FILE
		default_dir = concat_dname_fname(default_dir,DEF_DIR_NAME);
		if (path_n_file)
			x_free(path_n_file);
		path_n_file = concat_dname_fname(default_dir,OPTION_FILE);
		if (!filexists(path_n_file)) { // Try in the home directory - ~/OPTION_FILE
			default_dir = getenv("HOME");
			if (path_n_file)
				x_free(path_n_file);
			path_n_file = concat_dname_fname(default_dir, OPTION_FILE);
			if (!filexists(path_n_file)) { // Try ~/DEFAULT_DIR/OPTION_FILE
				default_dir = concat_dname_fname(default_dir,DEF_DIR_NAME);
				if (path_n_file) x_free(path_n_file);
				path_n_file = concat_dname_fname(default_dir, OPTION_FILE);
				if (directory_exists(default_dir)) {
					if (!filexists(path_n_file)) {
						// Big problem, create a default rc file
						create_config_file(path_n_file);
					}
				}
				else {
					// Geomorph directory ("default_dir") doesn't exist
					// Create one with default scenes and rc file
					buf = (gchar *) x_malloc(strlen(_("Creation of the default working directory: %s ?"))+1+strlen(default_dir), "const gchar (buf - Creation of the default working directory MSG)");
					sprintf(buf,_("Creation of the default working directory: %s ?"),default_dir);
					if (!yes_no(buf,TRUE))
						exit(0);
//					if (buf) x_free(buf);
					shortcuts = yes_no(_("Add shortcuts on the KDE and GNOME desktops?"), TRUE);
					command = concat_dname_fname(get_data_dir(),"install-step1-dir");
					if (system(command))
						my_msg(_("Fatal error during the creation of the default working directory"),ABORT);
					if (command) x_free(command);
					create_config_file(path_n_file);
					command = concat_dname_fname(get_data_dir(),"install-step3-menu");
					if (system(command))
						my_msg(_("Error during the creation of Geomorph in the Gnome or KDE menu"),WARNING);
					if (command) x_free(command);
					if (shortcuts) {
						command = concat_dname_fname(get_data_dir(),"install-step4-desktop");
						if (system(command))
							my_msg(_("Error during the creation of one shortcut or both"),WARNING);
//						if (buf) x_free(buf);
						if (command) x_free(command);
					}
				if (path_n_file) x_free(path_n_file);
				}
			}
		}
	}
	return add_filesep(default_dir);
}

int main( int   argc,
          char *argv[] )
{
	app_struct *main_app;
	gint i;
	GList *icon_list=NULL;
	option_file_type *opt=NULL, *allowed_opt=NULL;
	gboolean file_on_cmd = FALSE, file_on_cmd_processed = FALSE;
	gchar *msg = NULL,*option_file = NULL, *buf, *dir, *default_dir=NULL;

	bindtextdomain("geomorph",LOCALEDIR);
	bind_textdomain_codeset("geomorph", "UTF-8");
	textdomain("geomorph");
  
	gtk_set_locale();

	gtk_init(&argc, &argv);
	gdk_rgb_init();

	if (!gtk_gl_init_check (&argc, &argv)) {
		my_msg(_("Not able to display the Open GL preview"),WARNING);
	}

	x_alloc_init_with_exit (no_mem_exit);

	icon_list = g_list_append(icon_list, (gpointer) gdk_pixbuf_new_from_xpm_data ((const gchar**) GeoMorph_16x16_xpm));	
	icon_list = g_list_append(icon_list, (gpointer) gdk_pixbuf_new_from_xpm_data ((const gchar**) GeoMorph_32x32_xpm) );	
	icon_list = g_list_append(icon_list, (gpointer) gdk_pixbuf_new_from_xpm_data ((const gchar**) GeoMorph_48x48_xpm));
	gtk_window_set_default_icon_list (icon_list);

// printf("\nLOCALEDIR: %s; LANGUAGE: %s\n",LOCALEDIR, getenv("LANGUAGE"));

	allowed_opt = init_allowed_options();
	
	if (argc > 1) {
		for (i=1; i<argc; i++) {
		// Arguments supposed to be file names to open as a document or
		// as an "rc" file
		// Here we check for RC file
			if ( !strcmp("rc",argv[i]+strlen(argv[i])-2) )  {
				opt = load_config_file (argv[i], allowed_opt,&msg);
				option_file = argv[i];
			}
			else {
				file_on_cmd = TRUE;
			}	
		}
	}
	if (msg)
		my_msg(msg,WARNING);

//	Loading config file
//	First find the file
	default_dir = find_config_file();

	if (chdir(default_dir)) { // Working if return code is 0
	//	Not supposed to get here!!
		buf = x_malloc(strlen(default_dir)+strlen(_("Unable to change directory to %s"))+1, "const gchar (buf - change directory in main.c)");
		sprintf(buf,_("Unable to change directory to %s"),default_dir);
		my_msg(buf, ABORT);
	}

	DEF_DIR = default_dir;
	
	if (!opt)	{ // Options not already loaded (from file in command line)
		opt = load_config_file(OPTION_FILE, allowed_opt, &msg);
	}
	if (!option_file) // option_file name was not given on the command line
		option_file = OPTION_FILE;
	if (!strchr(option_file, FILESEP))	{ // Append the directory name if required
		dir = (gchar *) get_current_dir_name();
		buf = x_malloc(strlen(dir)+strlen(option_file)+2, "gchar - Option file name)");
		strcpy(buf,dir);
		buf[strlen(dir)] = FILESEP;
		buf[strlen(dir)+1] = '\0';
		strcat(buf,option_file);
		option_file = buf;
	}

	if (msg)
		my_msg(msg,ABORT);
	
	process_options(allowed_opt);
	process_specific_options(allowed_opt);  // Function given by thisappinit.h
		
//	Reset default directory as initialized by config file
	if (chdir(DEF_DIR)) // Working if return code is 0, switch back to old value if not
		DEF_DIR = default_dir;

	init_default_camera();
	
	main_app = app_new (_("GeoMorph 0.x - a height field creator for Linux"), opt, allowed_opt, option_file);

	// MAIN_APP: global variable, added 2008-11
	// Think about using only a global variable
	MAIN_APP = main_app;
	if (file_on_cmd)
		for (i=1; i<argc; i++) {
			// Try to open any file which is not a RC file, in sequence
			if ( strcmp("rc",argv[i]+strlen(argv[i])-2) ) {
				if (!strchr(argv[i], FILESEP))	{ // Append the directory name if required
					dir = (gchar *) get_current_dir_name();
					buf = x_malloc(strlen(dir)+strlen(option_file)+2, "gchar (buf - Option file name 2)");
					strcpy(buf,dir);
					buf[strlen(dir)] = FILESEP;
					buf[strlen(dir)+1] = '\0';
					strcat(buf,argv[i]);
					main_app->file_on_cmdline = buf;
				}
				else
					main_app->file_on_cmdline = argv[i];
				open_callb(NULL,(gpointer) main_app);
				file_on_cmd_processed = TRUE;
			}
		}
	
	// If no file found on command line, we create a new document

	if (!file_on_cmd_processed)
			app_show(main_app);

	INTERFACE_ACTIVE = TRUE;

	gtk_main();

	app_free(main_app);

	gtk_exit(0) ;
	
	x_alloc_release ();
	return 0;
}
