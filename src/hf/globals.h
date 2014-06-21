/* Globals.h - shared definitions
 * One application = a menu / tool bar + a list of documents
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

#ifndef _GLOBALS
#define _GLOBALS 1
#include <libintl.h>
#include <stdlib.h>

#include "../utils/utils.h"
#include "../utils/filesel.h"
#include "../utils/config_rc_file.h"
#include "../utils/config_dialog.h"
#include "../utils/x_alloc.h"

#ifndef VERSION
	#define VERSION "0.60"
#endif
#ifndef VERSION_NAME
	#define VERSION_NAME "geomorph-0.60"
#endif

// Subdir with scenes and option file, normally in $HOME
// DEF_DIR_NAME must be without path
#define DEF_DIR_NAME "geomorph"
#define OPTION_FILE "geomorphrc"

// Notice about directories
//
// DEF_DIR_NAME: default user directory name
// DEF_DIR: the directory where geomorph is executed
// HF_DIR: the directory where the user height fields are saved,
//	usually the same as DEF_DIR and POV_DIR
// TMP_DIR: temporary directory, with full path
// 	Should be read / write
// 	Contains: temporary PNG and POV - INC files, for rendering
//	Could be the same for all the applications in the package
// POV_DIR: directory containing the POV and INC files provided
// 	with the application
//	It could the directory where the POV scripts are
//	copied so that the user can modify them,
//	typically /home/<username>/geomorph,
//	or where the read-only scripts for the whole system
//	are installed, typically /usr/local/share/geomorph/<version>/scenes
// SCENE: the current scene, with full path name
// 
/****************** GENERAL PARAMETERS ****************/

#define DEF_INTEGRATED_INTERFACE TRUE

#define DEFAULT_PAD 6

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

//	Undo / redo stack max. length
#define MAXIMUM_HISTORY 10

#ifndef FILESEP
#define FILESEP '/'
#endif

#ifndef PI
#define PI ((gdouble) 3.14159265358979)
#endif

// Windows positions (in % of screen size)
#define MAIN_MENU_BAR_X 3
#define MAIN_MENU_BAR_Y 3
#define TOOLS_WINDOW_X 3
#define TOOLS_WINDOW_Y 15
#define CREATION_WINDOW_X 3
#define CREATION_WINDOW_Y 15
#define DEFAULT_DOC_OFFSET 4

// HTML docs directory & reader
#define DOCUMENT_DIR "geomorph.sourceforge.net"
#define DEFAULT_DOC_READER "firefox"

// This one should be in thisappinit.h...

#define MAX_HF_SIZE 4096

/******************* EXTERN (GLOBAL) VARIABLES **************/
//	The extern variables store the parameters from the RC file
//	Values not found in the RC file default to values
//	#defined a few lines before

//	For general parameters, initialized in main.c
extern gchar *DEF_DIR;
extern gint DEF_PAD, MAX_HISTORY,MAIN_BAR_X, MAIN_BAR_Y;
extern gint TOOLS_X, TOOLS_Y, CREATION_X, CREATION_Y;
extern guint DEFAULT_SEED;

//	For specific parameters, initialized in thisappinit.c
extern gint MAX_PEN_PREVIEW, MAX_FILTER_PREVIEW, MAX_NOISE_PREVIEW;
extern gint HF_SIZE_IN_SCREEN_PERCENT, HF_SIZE_IN_PIXELS, DOC_DISPLAY_SIZE;
extern gint RENDER_WIDTH, RENDER_HEIGHT;
extern gchar *RENDERER, *SCENE, *OTHER_RENDER_OPTIONS, 
		*HF_DIR, *POV_DIR, *DOC_DIR, *DOC_READER;

//	This one is initialized in utils.c, but can be overridden in thisappinit.c
extern gint COMMIT_BEHAVIOR;

#endif // _GLOBALS


