/* Initialization of structures peculiar to GeoMorph
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

#include <gtk/gtk.h>
#include <gdk/gdkrgb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Generic include files
#include "globals.h"
#include "../utils/menus_n_tools.h"
#include "doc.h"
#include "app.h"

/******************* SPECIFIC PARAMETERS ****************/

//	Maximum size of some preview areas, in pixels
#define MAXIMUM_PEN_PREVIEW 150
#define MAXIMUM_FILTER_PREVIEW 32
#define MAXIMUM_NOISE_PREVIEW 64


// Terrain size
#define DEFAULT_HF_SIZE_IN_SCREEN_PERCENT 40

// Renderer
#define RENDER_W 320
#define RENDER_H 240
#define DEFAULT_RENDERER "povray"
#define DEFAULT_SCENE "geomorph/simple_terrain.pov"
#define DEFAULT_RENDER_OPTIONS "+P +D -F +L/usr/local/share/geomorph/"VERSION"/scenes"

// OpenGL preview
#define GL_DEFAULT_ANGLE 55
#define GL_DISTANCE_INIT 2.5
#define GL_DEFAULT_ROT_X 50
#define GL_DEFAULT_ROT_Y 0
#define GL_ASPECT_RATIO 1.3333333333
#define GL_SHOW_SUBDIALOG TRUE
#define GL_DIRECT_RENDERING FALSE
#define GL_DIRECT_UPDATE TRUE
#define GL_PREVIEW_SIZE 256
#define GL_MESH_SIZE 128
#define GL_DRAWING_MESH_SIZE 64
#define GL_HEIGHT 0.0
#define GL_RIGHT_LEFT_DISPL 0.0

// Application parameter(s)
#define DEF_TERRAIN "Subdiv2"

/******************* EXTERN (GLOBAL) VARIABLES **************/
//	The extern variables store the parameters from the RC file
//	Values not found in the RC file default to values
//	#defined a few lines before

//	For specific parameters, initialized in thisappinit.c or app.c
extern gint MAX_PEN_PREVIEW, MAX_FILTER_PREVIEW, MAX_NOISE_PREVIEW;

extern gint HF_SIZE_IN_SCREEN_PERCENT, HF_SIZE_IN_PIXELS, DOC_DISPLAY_SIZE;
extern gint RENDER_WIDTH, RENDER_HEIGHT;
extern gchar *RENDERER, *SCENE, *OTHER_RENDER_OPTIONS, *DEFAULT_TERRAIN,
		*HF_DIR, *POV_DIR, *DOC_DIR, *DOC_READER;

// GL preview (added 2005-03, 2005-11 for direct rendering)
extern gint DEFAULT_ANGLE[], DEFAULT_ROT_X[], DEFAULT_ROT_Y[], PREVIEW_SIZE, 
	MESH_SIZE, DRAWING_MESH_SIZE;
extern gboolean SHOW_SUBDIALOG, DIRECT_UPDATE, DIRECT_RENDERING;
extern gfloat DISTANCE_INIT[], ASPECT_RATIO[], RIGHT_LEFT_DISPL[], HEIGHT[]; 

// Prototypes

types_wrapper * instantiate_types(GtkWidget *main_window);
void choose_hf (GtkWidget *, gpointer data);
void choose_povini (GtkWidget *, gpointer data);
option_file_type *init_allowed_options (void);
void process_specific_options(option_file_type *opt);
