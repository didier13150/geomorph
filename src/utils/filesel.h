/* filesel.h - File selection dialog
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "./utils.h"

#ifndef _FILESEL
#define _FILESEL 1

//	DEF_PAD supposed to be declared in the calling application
// #ifndef DEF_PAD
// #define DEF_PAD 5
// #endif
extern gint DEF_PAD;

#define NEW_FILE 0
#define EXISTING_FILE 1
#define NO_CHECK 2

#ifndef FILESEP
#define FILESEP '/'
#endif

/*	Main function prototype
	ff_list:  	option menu list which contains all allowed file
			formats, as defined by the extension (label = extension);
	max_index:	number of option menu items in ff_list;
	sTitle:		window title;
	default_dir_file:	default directory or file; directory ends with /;
	write_or_read:	values:  NEW_FILE, EXISTING_FILE, NO_CHECK;
	extension:	default extension asked for, can be NO_EXTENSION

To use this function, you must define:
	a omenu_list_type containing the extensions menu
	a #define string NO_EXTENSION, in whatever language you want
And these messages, in whatever language too:
	a #define string FORMAT_OPTIONS,
		which is the title of the option menu
	a #define string INCOHERENT_EXTENSION,
		displayed when the user type an extension which is not the one
		chosen in the menu
	a #define string FILE_NOT_EXISTS, used when an existing file is required
		and no one is found (e.g. when reading)
	a #define string FILE_EXISTS, used when a non-existent file is found and
		the file given exists (e.g. when saving)
*/

char *GetDirname (omenu_list_type *ff_list, int max_index,  
	char *sTitle, char *default_dir_file, 
	int write_or_read, char *default_extension);
	
char *GetFilename (omenu_list_type *ff_list, int max_index,  
	char *sTitle, char *default_dir_file, 
	int write_or_read, char *default_extension);

void filetype_sel_callback_fn(gpointer user_data, GtkWidget *emitting_widget);

#endif // _FILESEL



