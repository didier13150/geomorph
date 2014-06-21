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

#include <string.h>
#include "thisappinit.h"

// Include files peculiar to this application
#include "../hf/hf.h"
#include "../hf/hf_tools_dialog.h"
#include "../hf/hf_creation_dialog.h"
#include "../hf/hf_wrapper.h"
#include "../hf/hf_dialog_options.h"
// #include "povini/povini.h"
// #include "povini/povini_dialog.h"

#define NBHFEXT 1
omenu_list_type hf_extension_list[NBHFEXT] = {
	{ NULL,0,NULL,"PNG",filetype_sel_callback_fn },
};

#define NBPOVINIEXT 1
omenu_list_type povini_extension_list[NBPOVINIEXT] = {
	{ NULL,0,NULL,"INI",filetype_sel_callback_fn },
};

//	NULL,1,NULL,"POV",filetype_sel_callback_fn,

// Toolbar icons for document type
#define NBDOCTYPES 1
#include "../icons/GeoMorph.xpm"
// #include "../icons/povini.xpm"

doc_type_struct doc_type_list[NBDOCTYPES] = { {
	DOC_TYPE_TYPE, // type_id 
	"HeightField", // type (type description - gchar *)
	NBHFEXT, // number of extensions
	hf_extension_list, // extension list (omenu_list_type *)
	NULL, // gchar *def_dir: default directory for this type
	NULL, // creation_dialog - created on-the-fly
	NULL, // gpointer widget_list - optional
	NULL, // gpointer options_dialogs - generated fy options_dialog_new
	NULL, // GtkWidget* tools_dialog
	hf_creation_dialog_new,
	hf_options_dialog_new,
	hf_options_dialog_free,
	hf_tools_dialog_new,
	hf_wrapper_new,
	NULL, 	// should be read, for now replaced by "..._new"
	hf_wrapper_free,
	hf_wrapper_copy,
	NULL, 			// close
	hf_wrapper_display,
	hf_wrapper_save,
	NULL,			// print
	NULL, 			// specific undo
	NULL, 			// specific redo
	NULL, 			// cut
	NULL,			// copy
	NULL, 			// paste
	NULL, 			// paste special
	NULL, 			// run
	(gpointer) hf_dialog_defaults,	// defaults
	hf_stats,
	hf_commit_creation,
	hf_get_last_action,
	hf_get_icon,
	(gpointer) set_creation_container,
	NULL // undo / redo stack struct
 }
// ,
// { ".INI Povray", NBPOVINIEXT, povini_extension_list, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
//	NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
//	NULL, NULL, NULL,NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, NULL, NULL }
};

command_item_struct doc_type_item[NBDOCTYPES] = {
{ "Document type", "HeightField", "Height field",0, (gchar **) GeoMorph_xpm, GDK_LEFT_PTR, choose_hf, NULL, NULL,TRUE },
// { "Document type", ".INI Povray", ".INI options file POVRAY 3.1",0, (gchar **) povini_xpm, GDK_LEFT_PTR, choose_povini,NULL, NULL,FALSE }
};

/*****************************************************************************/

//	Allowed options for "geomorphrc"
//	We use the functions developped for another project
//	without initializing the read and write functions...
//	See ../utils/config_rc_file.h and ... .c
//	Needs some revision... (maybe more like a form definition, in XML?)

#define NB_OPTION_BLOCKS 10

#define NB_FILE_OPT 6
#define NB_INTERFACE_OPT 17
#define NB_APP_OPT 3
#define NB_RENDER_OPT 7
#define NB_3DPREVIEW_OPT 6
#define NB_CAMERA_OPT 7

#define NB_CAMERAS 5

option_file_type allowed_options = {NB_OPTION_BLOCKS, NULL};

option_block_type block[NB_OPTION_BLOCKS] =
	{ {"files", "Directory and files", NB_FILE_OPT, NULL},
	{"interface","Interface", NB_INTERFACE_OPT, NULL},
	{"application", "Application", NB_APP_OPT, NULL},
	{"rendering","3D rendering", NB_RENDER_OPT, NULL},
	{"3d_preview","3D preview", NB_3DPREVIEW_OPT, NULL},
	{"Camera_1","Camera 1", NB_CAMERA_OPT, NULL},
	{"Camera_2","Camera 2", NB_CAMERA_OPT, NULL},
	{"Camera_3","Camera 3", NB_CAMERA_OPT, NULL},
	{"Camera_4","Camera 4", NB_CAMERA_OPT, NULL},
	{"Camera_5","Camera 5", NB_CAMERA_OPT, NULL}
	};

option_type options0[NB_FILE_OPT] =
	{{"def_dir", NULL,NULL,"dir", NULL, "Default directory"},
	{"hf_dir", NULL,NULL,"dir", NULL,"Default directory (Height fields)"},
	{"pov_dir", NULL,NULL,"dir", NULL, "Default directory (Povray scenes)"},
	{"tmp_dir", NULL,NULL,"dir", NULL, "Default directory (temporary files)"},
	{"doc_dir",NULL,NULL,"dir",NULL, "Default directory (documentation)"},
	{"doc_reader", NULL, NULL,"file", NULL, "Default HTML reader"}
	};
	
option_type options1[NB_INTERFACE_OPT] =
	{
	{"interface_style", NULL,NULL, "char", "Gimp style#Integrated", "Interface style"},
	{"menu_in_doc_window", NULL,NULL, "char",
		"TRUE#FALSE", "Display the main menu in each document window"},
	{"icons_in_doc_window", NULL,NULL, "char",
		"TRUE#FALSE", "Display the main menu icons in each document window"},
	{"pad", NULL, NULL,
		"integer", "0-20", "Dialog padding (pixels)"},
	{"hf_size_in_screen_percent", NULL, NULL,
		"integer", "10-100", "Height field default size (% of display)"},
	{"hf_size_in_pixels", NULL, NULL,
		"integer", "128#256#512#1024#2048#4096", "Height field default size (pixels)"},
	{"doc_display_size",NULL,NULL,
		"integer","128#256#512#1024#2048#4096", "Default image display size (pixels)"},
	{"max_pen_preview",NULL,NULL,
		"integer", "2-255", "Maximum pen preview size (pixels)"},
	{"filter_preview_size",NULL,NULL,
		"integer", "16#32#64#128#256", "Filter preview size (pixels)"},
	{"noise_preview_size",NULL,NULL,
		"integer", "16#32#64#128#256", "Noise preview size (pixels)"},
	{"main_bar_x",NULL,NULL,
		"integer", "0-100", "Horizontal position of the main menu (% of display)"},
	{"main_bar_y",NULL,NULL,
		"integer", "0-100", "Vertical position of the main menu (% of display)"},
	{"creation_window_x",NULL,NULL,
		"integer", "0-75", "Horizontal position of the creation window (%)"},
	{"creation_window_y",NULL,NULL,
		"integer", "0-75", "Vertical position of the creation window (%)"},
	{"tools_window_x",NULL,	NULL,
		"integer", "0-75", "Horizontal position of the tools window (%)"},
	{"tools_window_y",NULL,	NULL,
		"integer", "0-75", "Vertical position of the tools window (%)"},
	{"display_doc_offset",NULL,	NULL,
		"integer", "0-20", "Horizontal offset of new documents (%)"}
	};
option_type options2[NB_APP_OPT] =
	{
	{"max_history", NULL,NULL, "integer", "1-25", "Default history size (undo / redo)"},
	{"default_terrain",NULL, NULL, "char", "Uniform#Subdiv1#Subdiv2#SurfSum", "Default terrain"},
	{"default_seed",NULL, NULL, "integer", NULL, "Default seed for random generator"}
	};
	
option_type options3[NB_RENDER_OPT] =
	{{"renderer", NULL, NULL,"char", NULL, "3D rendering tool (Povray...)"},
	{"render_width", NULL,NULL, "integer",
		"160#320#480#640#800#1024#1280#1600", "Default 3D rendering width"},
	{"render_height", NULL, NULL,"integer",
		"120#240#360#480#600#768#1024#1200", "Default 3D rendering height"},
	{"scene", NULL, NULL,"file", NULL, "Default rendering scene (.pov...)"},
	{"hf_output_for_rendering", NULL, NULL,"file", NULL, "PNG output file name for default POV scene"},
	{"output_prefix", NULL, NULL,"char", NULL, "Povray output prefix"},
	{"other_render_options", NULL, NULL,"char", NULL, "Other command line rendering options"}
	};
	
option_type options4[NB_3DPREVIEW_OPT] =
	{{"mesh_size", NULL, NULL,"integer", "128#256#512#1024#2048#4096", "3D mesh size (in nodes on one edge of the HF)"},
	{"drawing_mesh_size", NULL, NULL, "integer",
		"32#64#128#256#512#1024", "Default 3D mesh size when drawing (in nodes)"},
	{"preview_size", NULL,NULL, "integer", "128-512", "Default preview size (in pixels)"},
	{"direct_update", NULL,NULL, "char",
		"TRUE#FALSE", "Automatic update (FALSE => the user must click to refresh)"},
	{"show_subdialog", NULL,NULL, "char",
		"TRUE#FALSE", "Show the camera controls on startup"},
	{"direct_rendering", NULL,NULL, "char",
		"TRUE#FALSE", "Hardware acceleration for OpenGL (DRI)"}};

option_type options5[NB_CAMERA_OPT] = {
	{"camera_y_rotation", NULL,NULL, "integer", "0-360", "Default camera right-left rotation (Y)"},
	{"camera_x_rotation", NULL,NULL, "integer", "0-360", "Default camera back-front rotation (X)"},
	{"camera_distance", NULL,NULL, "float", "0.00-5.00", "Default relative distance of the camera"},
	{"camera_height", NULL, NULL, "float", "-1.00-1.00", "Default camera height"},
	{"right_left_displacement", NULL, NULL, "float", "-1.00-1.00", "Default camera right-left displacement"},
	{"vision_angle", NULL,NULL, "integer", "5-100", "Default field of view, in degrees"},
	{"aspect_ratio", NULL,NULL, "float", "0.500-2.000", "Default aspect ratio (width / length)"}

	};
	
gint MAX_PEN_PREVIEW, MAX_FILTER_PREVIEW, MAX_NOISE_PREVIEW, DRAW_PREVIEW_SIZE;
gint HF_SIZE_IN_SCREEN_PERCENT, HF_SIZE_IN_PIXELS, DOC_DISPLAY_SIZE;
gint RENDER_WIDTH, RENDER_HEIGHT;
gchar *RENDERER, *SCENE, *OTHER_RENDER_OPTIONS, *DEFAULT_TERRAIN,
	*HF_DIR, *POV_DIR, *HF_OUTPUT_FOR_RENDERING, *OUTPUT_PREFIX, *TMP_DIR;
	
// GL preview (added 2005-03, direct_rendering 2005-11)
gint PREVIEW_SIZE, MESH_SIZE, DRAWING_MESH_SIZE;
gboolean SHOW_SUBDIALOG, DIRECT_UPDATE, DIRECT_RENDERING;
gint DEFAULT_ANGLE[NB_CAMERAS], DEFAULT_ROT_X[NB_CAMERAS], DEFAULT_ROT_Y[NB_CAMERAS];
gfloat DISTANCE_INIT[NB_CAMERAS], ASPECT_RATIO[NB_CAMERAS], RIGHT_LEFT_DISPL[NB_CAMERAS], HEIGHT[NB_CAMERAS]; 

/*****************************************************************************/

option_file_type *init_allowed_options() {
	allowed_options.option_block = (option_block_type *) &block;
	allowed_options.option_block[0].option_list = (option_type *) &options0;
	allowed_options.option_block[1].option_list = (option_type *) &options1;
	allowed_options.option_block[2].option_list = (option_type *) &options2;
	allowed_options.option_block[3].option_list = (option_type *) &options3;
	allowed_options.option_block[4].option_list = (option_type *) &options4;
	allowed_options.option_block[5].option_list = (option_type *) &options5;
	allowed_options.option_block[6].option_list = copy_options ((option_type *) &options5, NB_CAMERA_OPT);
	allowed_options.option_block[7].option_list = copy_options ((option_type *) &options5, NB_CAMERA_OPT);
	allowed_options.option_block[8].option_list = copy_options ((option_type *) &options5, NB_CAMERA_OPT);
	allowed_options.option_block[9].option_list = copy_options ((option_type *) &options5, NB_CAMERA_OPT);
	return &allowed_options;
}

void process_specific_options(option_file_type *options) {
//	Process some specific defaults from the RC file
//	DEFAULT_PAD, MAXIMUM_HISTORY... are #defined in globals.h
//	DEF_PAD, MAX_HISTORY are declared "extern" in globals.h as well
	gchar *buf, *buf2, *current_block;
	gboolean invalid;
	gint i;

	MAX_PEN_PREVIEW = MAXIMUM_PEN_PREVIEW;
	if ((buf = get_option(options,"interface","max_pen_preview"))) {
		if (is_integer(buf)) {
			MAX_PEN_PREVIEW = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - MAX_PEN_PREVIEW)");
		sprintf ( buf,"%-4d", MAX_PEN_PREVIEW);
		put_option(options,"interface","max_pen_preview" , buf);
	}
		
	MAX_FILTER_PREVIEW = MAXIMUM_FILTER_PREVIEW;
	if ((buf = get_option(options,"interface","max_filter_preview"))) {
		if (is_integer(buf)) {
			MAX_FILTER_PREVIEW = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - MAX_FILTER_PREVIEW)");
		sprintf ( buf,"%-4d", MAX_FILTER_PREVIEW);
		put_option(options,"interface","max_filter_preview" ,buf);
	}
		
	MAX_NOISE_PREVIEW = MAXIMUM_NOISE_PREVIEW;
	if ((buf = get_option(options,"interface","noise_preview_size"))){
		if (is_integer(buf)) {
			MAX_NOISE_PREVIEW = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - MAX_NOISE_PREVIEW)");
		sprintf ( buf,"%-4d", MAX_NOISE_PREVIEW);
		put_option(options,"interface","noise_preview_size" ,buf);
	}
	
	DOC_DISPLAY_SIZE = 0;	// Use default from screen size
	if ((buf = get_option(options,"interface","doc_display_size"))){
		if (is_integer(buf)) {
			DOC_DISPLAY_SIZE = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - DOC_DISPLAY_SIZE)");
		sprintf ( buf,"%-4d",DOC_DISPLAY_SIZE );
		put_option(options,"interface","doc_display_size" ,buf);
	}
		
	HF_SIZE_IN_SCREEN_PERCENT = DEFAULT_HF_SIZE_IN_SCREEN_PERCENT;
	if ((buf = get_option(options,"interface","hf_size_in_screen_percent"))){
		if (is_integer(buf)) {
			HF_SIZE_IN_SCREEN_PERCENT = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - HF_SIZE_IN_SCREEN_PERCENT)");
		sprintf(buf,"%-4d",HF_SIZE_IN_SCREEN_PERCENT );
		put_option(options,"interface","hf_size_in_screen_percent" ,buf);
	}
		
	HF_SIZE_IN_PIXELS = 0; 	// Use default from screen size
	if ((buf = get_option(options,"interface","hf_size_in_pixels"))){
		if (is_integer(buf)) {
			HF_SIZE_IN_PIXELS = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - HF_SIZE_IN_PIXELS)");
		sprintf(buf,"%-4d",HF_SIZE_IN_PIXELS );
		put_option(options,"interface","hf_size_in_pixels" ,buf);
	}

	if ((buf = get_option(options,"application","default_terrain")) && strlen(buf))
		DEFAULT_TERRAIN = buf;
	else {
		DEFAULT_TERRAIN = DEF_TERRAIN;	
		put_option ( options,"application","default_terrain", DEF_TERRAIN );

	}
	
	RENDER_WIDTH = RENDER_W;
	if ((buf = get_option(options,"rendering","render_width"))){
		if (is_integer(buf)) {
			RENDER_WIDTH = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - RENDER_WIDTH)");
		sprintf(buf,"%-4d", RENDER_WIDTH);
		put_option(options, "rendering","render_width",buf);
	}
		
	RENDER_HEIGHT = RENDER_H;	
	if ((buf = get_option(options,"rendering","render_height"))){
		if (is_integer(buf)) {
			RENDER_HEIGHT = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - RENDER_HEIGHT)");
		sprintf(buf,"%-4d", RENDER_HEIGHT);
		put_option(options, "rendering","render_height",buf);
	}
	
	if ((buf = get_option(options,"rendering","renderer")) && strlen(buf))
			RENDERER = buf;
	else {
		RENDERER = DEFAULT_RENDERER;
		put_option ( options,"rendering","renderer", RENDERER );
	}
		
	if ((buf = get_option(options,"rendering","scene")) && strlen(buf))
		SCENE = buf;
	else {
		SCENE = DEFAULT_SCENE;
		put_option ( options,"rendering","scene", SCENE );
	}
		
	if ((buf = get_option(options,"rendering","other_render_options")) && strlen(buf))
		OTHER_RENDER_OPTIONS = buf;
	else {
		OTHER_RENDER_OPTIONS = DEFAULT_RENDER_OPTIONS;
		put_option(options,"rendering","other_render_options", OTHER_RENDER_OPTIONS );
	}
		
	if ( (buf = get_option(options,"files","hf_dir")))
		HF_DIR = add_filesep(buf);
	else {
		HF_DIR = DEF_DIR;
		put_option(options,"files","hf_dir",HF_DIR);
	}
	
	if ((buf = get_option(options,"rendering","hf_output_for_rendering")) && strlen(buf))
		HF_OUTPUT_FOR_RENDERING = buf;
	else {
		HF_OUTPUT_FOR_RENDERING = "test.png";
		put_option(options,"rendering","hf_output_for_rendering", HF_OUTPUT_FOR_RENDERING);
	}
		
	if ((buf = get_option(options,"rendering","output_prefix")) && strlen(buf)) {
		OUTPUT_PREFIX = buf;
	}
	else {
		OUTPUT_PREFIX = "_";
		put_option(options,"rendering","output_prefix", OUTPUT_PREFIX);
	}
		
	if ((buf = get_option(options,"files","pov_dir")) && strlen(buf) )
		POV_DIR = add_filesep(buf);
	else {
		POV_DIR = add_filesep(DEF_DIR);
		put_option(options,"files","pov_dir",POV_DIR);
	}
//	printf("*** POV_DIR: %s\n",POV_DIR);
//	Add the directory to the default SCENE, if SCENE has no path

	if (SCENE[0] != FILESEP) {
		buf = (gchar *) x_malloc(strlen(POV_DIR)+2+strlen(SCENE), "gchar (buf - POV_DIR + SCENE)");
		buf2 = add_filesep(POV_DIR);
		sprintf(buf,"%s%s",buf2,SCENE);
		SCENE = buf;
		if (buf2 != POV_DIR)
			x_free(buf2);
	}
		
	if ((buf = get_option(options,"files","tmp_dir")) && strlen(buf) )
		TMP_DIR = add_filesep(buf);
	else {
		TMP_DIR = add_filesep(DEF_DIR);
		put_option(options,"files","tmp_dir",TMP_DIR);
	}
//	printf("TMP_DIR: %s; DEF_DIR: %s; POV_DIR: %s; SCENE: %s\n",TMP_DIR, DEF_DIR, POV_DIR, SCENE);
	MESH_SIZE = GL_MESH_SIZE;
	if ((buf = get_option(options,"3d_preview","mesh_size"))){
		if (is_integer(buf)) {
			MESH_SIZE = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - MESH_SIZE)");
		sprintf(buf,"%-4d", MESH_SIZE);
		put_option(options,"3d_preview","mesh_size",buf);
	}
			
	DRAWING_MESH_SIZE = GL_DRAWING_MESH_SIZE;	
	if ((buf = get_option(options,"3d_preview","drawing_mesh_size"))){
		if (is_integer(buf)) {
			DRAWING_MESH_SIZE = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - DRAWING_MESH_SIZE)");
		sprintf(buf,"%-4d", DRAWING_MESH_SIZE);
		put_option(options,"3d_preview","drawing_mesh_size",buf);
	}
		
	PREVIEW_SIZE = GL_PREVIEW_SIZE;	
	if ((buf = get_option(options,"3d_preview","preview_size"))){
		if (is_integer(buf)) {
			PREVIEW_SIZE = atol(buf);
			invalid = FALSE;
		}
		else
			invalid = TRUE;
	}
	if ((!buf) || invalid) {
		buf = (gchar *) x_malloc(5, "gchar (buf - PREVIEW_SIZE)");
		sprintf(buf,"%-4d", PREVIEW_SIZE);
		put_option(options,"3d_preview","preview_size",buf);
	}
		
	if ((buf = get_option(options,"3d_preview","direct_update"))){
		if (!strcmp(buf,"TRUE"))
			DIRECT_UPDATE = TRUE;
		else
			DIRECT_UPDATE = FALSE;
	}
	else {
		DIRECT_UPDATE = GL_DIRECT_UPDATE;
		if (DIRECT_UPDATE)
			put_option (options,"3d_preview","direct_update","TRUE");
		else
			put_option (options,"3d_preview","direct_update","FALSE");
	}
				
	if ((buf = get_option(options,"3d_preview","show_subdialog"))){
		if (!strcmp(buf,"TRUE"))
			SHOW_SUBDIALOG = TRUE;
		else
			SHOW_SUBDIALOG = FALSE;
	}
	else {
		SHOW_SUBDIALOG = GL_SHOW_SUBDIALOG;
		if (SHOW_SUBDIALOG)
			put_option (options,"3d_preview","show_subdialog","TRUE");
		else
			put_option (options,"3d_preview","show_subdialog","FALSE");
	}
	
				
	if ((buf = get_option(options,"3d_preview","direct_rendering"))){
		if (!strcmp(buf,"TRUE"))
			DIRECT_RENDERING = TRUE;
		else
			DIRECT_RENDERING = FALSE;
	}
	else {
		DIRECT_RENDERING = GL_DIRECT_RENDERING;
		if (DIRECT_RENDERING)
			put_option (options,"3d_preview","direct_rendering","TRUE");
		else
			put_option (options,"3d_preview","direct_rendering","FALSE");
	}
	
	current_block = (gchar *) x_malloc(1+strlen("Camera_N"), "gchar (current_block - Camera_N)");
	for (i=0; i<NB_CAMERAS; i++) {
	
		sprintf(current_block, "Camera_%1d", i+1);
		
		DEFAULT_ANGLE[i] = GL_DEFAULT_ANGLE;			
		if ((buf = get_option(options, current_block,"vision_angle"))){
			if (is_integer(buf)) {
				DEFAULT_ANGLE[i] = atol(buf);
				invalid = FALSE;
			}
			else
				invalid = TRUE;
		}
		if ((!buf) || invalid) {
			buf = (gchar *) x_malloc(5, "gchar (buf - DEFAULT_ANGLE)");
			sprintf(buf,"%4d", DEFAULT_ANGLE[i]);
			put_option(options,current_block,"vision_angle",buf);
		}
			
		DEFAULT_ROT_X[i] = GL_DEFAULT_ROT_X;
		if ((buf = get_option(options, current_block,"camera_x_rotation"))){
			if (is_integer(buf)) {
				DEFAULT_ROT_X[i] = atol(buf);
				invalid = FALSE;
			}
			else
				invalid = TRUE;
		}
		if ((!buf) || invalid) {
			buf = (gchar *) x_malloc(5,"gchar (buf - DEFAULT_ROT_X)" );
			sprintf(buf,"%4d",DEFAULT_ROT_X[i]);
			put_option(options, current_block,"camera_x_rotation",buf);
		}
			
		DEFAULT_ROT_Y[i] = GL_DEFAULT_ROT_Y;		
		if ((buf = get_option(options, current_block ,"camera_y_rotation"))){
			if (is_integer(buf)) {
				DEFAULT_ROT_Y[i] = atol(buf);
				invalid = FALSE;
			}
			else
				invalid = TRUE;
		}
		if ((!buf) || invalid) {
			buf = (gchar *) x_malloc(5,"gchar (buf - DEFAULT_ROT_Y)" );
			sprintf(buf,"%4d",DEFAULT_ROT_Y[i]);
			put_option(options, current_block ,"camera_y_rotation",buf);
		}
		
		DISTANCE_INIT[i] = GL_DISTANCE_INIT;
		if ((buf = get_option(options, current_block,"camera_distance"))){
			if (is_float(buf)) {
//				DISTANCE_INIT[i] = atof(buf);
				DISTANCE_INIT[i] = string_to_double(buf);
				invalid = FALSE;
			}
			else
				invalid = TRUE;
		}
		if ((!buf) || invalid) {
			buf = (gchar *) x_malloc(10, "gchar (buf - camera_distance)");
			sprintf(buf,"%9.5f",DISTANCE_INIT[i]);
			put_option(options, current_block ,"camera_distance",buf);
		}
		
		RIGHT_LEFT_DISPL[i] = GL_RIGHT_LEFT_DISPL;
		if ((buf = get_option(options, current_block,"right_left_displacement"))){
			if (is_float(buf)) {
//				RIGHT_LEFT_DISPL[i] = atof(buf);
				RIGHT_LEFT_DISPL[i] = string_to_double(buf);
				invalid = FALSE;
			}
			else
				invalid = TRUE;
		}
		if ((!buf) || invalid) {
			buf = (gchar *) x_malloc(10, "gchar (buf - RIGHT_LEFT_DISPL)");
			sprintf(buf,"%9.5f",RIGHT_LEFT_DISPL[i]);
			put_option(options, current_block ,"right_left_displacement",buf);
		}
			
		HEIGHT[i] = GL_HEIGHT;
		if ((buf = get_option(options, current_block,"camera_height"))){
			if (is_float(buf)) {
//				HEIGHT[i] = atof(buf);
				HEIGHT[i] = string_to_double(buf);
				invalid = FALSE;
			}
			else
				invalid = TRUE;
		}
		if ((!buf) || invalid) {
			buf = (gchar *) x_malloc(10, "gchar (buf - camera_height)");
			sprintf(buf,"%9.5f",HEIGHT[i]);
			put_option(options, current_block ,"camera_height",buf);
		}
	
		
		ASPECT_RATIO[i] = GL_ASPECT_RATIO;		
		if ((buf = get_option(options, current_block ,"aspect_ratio"))){
			if (is_float(buf)) {
//				ASPECT_RATIO[i] = atof(buf);
				ASPECT_RATIO[i] = string_to_double(buf);
				invalid = FALSE;
			}
			else
				invalid = TRUE;
		}
		if ((!buf) || invalid) {
			buf = (gchar *) x_malloc(10, "gchar (buf - aspect_ratio)");
			sprintf(buf,"%9.7f",ASPECT_RATIO[i]);
			put_option(options, current_block ,"aspect_ratio",buf);
		}
	}	// End FOR NB_CAMERAS

		
}

/*****************************************************************************/
types_wrapper* instantiate_types(GtkWidget *main_window) {

	doc_type_list[0].def_dir = (gchar *) x_malloc(strlen(HF_DIR)+1, "gchar (def_dir - HF_DIR)");
	strcpy(doc_type_list[0].def_dir,HF_DIR);
	/*doc_type_list[1].def_dir = (gchar *) x_malloc(strlen(POV_DIR)+1, "gchar (def_dir - POV_DIR)");
	strcpy(doc_type_list[1].def_dir,POV_DIR);*/
	return types_wrapper_new(main_window, doc_type_list, doc_type_item, NBDOCTYPES);

}

// These callback functions are connected in "type_choice_dialog_new"

void choose_hf (GtkWidget *wdg, gpointer data) {

	app_struct *app;
	app = (app_struct *)data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
	//	Nothing to do if we are just unactivating the button
		return;
	initype (app->types, 
			app->window, 
			&doc_type_list[0], 
			&app->docs->current_doc,
			&app->docs->current_doc_data,
			app->stack,
			app->docs);
}

void choose_povini (GtkWidget *wdg, gpointer data) {
	app_struct *app;
	app = (app_struct *)data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
	//	Nothing to do if we are just unactivating the button
		return;

	initype (app->types,
			app->window, 
			&doc_type_list[1], 
			&app->docs->current_doc,
			&app->docs->current_doc_data,
			app->stack,
			app->docs);
}

