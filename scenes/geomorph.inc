// Initializations for Geomorph scenes, from version 0.40
// 2nd version (ships with Geomorph 0.41)

#if (file_exists("global_settings.inc"))
	#include "global_settings.inc"
#end

// Camera position, from the preview dialog
#include "tmp/camera.inc"

// Rock maps generated with colmap (colmap.c)
#ifndef (rock_color)
	#declare rock_color=<0.75,0.5,0.3>;
#end
#include "maprock1.inc"
#include "maprock_redmud.inc"

// Basic textures
#include "geomorph_txtr.inc"
#include "slope_patterns.inc"

// Options other than camera
#if (file_exists("tmp/water.inc"))
	#include "tmp/water.inc"
#end
#if (file_exists("render_options.inc"))
	#include "render_options.inc"
#end

// The height field we render
// The water map is taken from the directory of the current height field

#ifndef (hf_to_render)
	#define hf_to_render="test.png";
#end

#ifdef (current_directory)
	#ifdef (temporary_directory)
		#declare main_terrain=concat(temporary_directory,hf_to_render);
	#else
		#declare main_terrain=concat(current_directory,hf_to_render);
	#end
	#declare water_map=concat(current_directory,"hf_water.png");
	#declare background_map=concat(current_directory,"hf_background.png");
	#declare ground_map=concat(current_directory,"ground_map.png");
	#declare crater_map=concat(current_directory,"craters.png");
#else
	#declare main_terrain=hf_to_render;
	#declare water_map="water_map.png";
	#declare ground_map="ground_map.png";
	#declare background_map="hf_background.png";
	#declare crater_map="craters.png";
#end
