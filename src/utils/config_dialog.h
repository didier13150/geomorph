/* config_dialog.h - headers for options file GTK interface
 *
 * Copyright (C) 2002 Patrice St-Gelais
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

#ifndef _CONFIG_DIALOG
#define _CONFIG_DIALOG 1

#include <stdio.h>
#include <gtk/gtk.h>
#include "config_rc_file.h"

//	Widget types
#ifndef GENERIC_ENTRY
#define GENERIC_ENTRY 0
#endif
#ifndef FILE_ENTRY
#define FILE_ENTRY 1
#endif
#ifndef DIR_ENTRY
#define DIR_ENTRY 2
#endif
#ifndef OPT_MENU
#define OPT_MENU 3
#endif
#ifndef SCALE_ENTRY
#define SCALE_ENTRY 4
#endif

typedef struct {	// Callback struct for GTK interface
		// 1 instance = 1 file
	char **path_n_file;		// Last file loaded
	GtkWidget *window;		// Window where the file is displayed
					// (we have to change the title after loading)
	option_file_type **current_opt_ptr;	// Options to display
					// Indirect reference because we have
					// to reinit them when reading a new file
	option_file_type *allowed_opt;	// Directory of allowed options
					// Usually includes comments
	gboolean save_comments;	// TRUE := we save descriptions in allowed_opt
					// in the current file
	gboolean if_modified;		// TRUE := ask to save
	void (*process_options) (option_file_type*);  // Allows to reinit the options 
					// after reading a new file
	
} rc_callback_data;

typedef struct {	// 2e callback struct
		// 1 instance = 1 line in the file = 1 option
	option_type *opt;
	gchar *format;
	gint lng;	// Total length, including decimals, excluding \0
	gint dec; // Decimals
} opt_callb_data;

GtkWidget* options_dialog_new( char **path_n_file,
			option_file_type **current_opt,
			option_file_type *allowed_opt,
			void (*process_options) (option_file_type *) );

#endif // _CONFIG_DIALOG






