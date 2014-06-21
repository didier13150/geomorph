/* gl_preview.h - headers for OpenGL preview
 *
 * Copyright (C) 2002, 2010 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         www.oricom.ca/patrice.st-gelais
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

#ifndef _GLPREVIEW
#define _GLPREVIEW 1

#include "globals.h"
#include "camera.h"

typedef struct {
	gfloat x;
	gfloat y;
	gfloat z;
	gfloat nx;
	gfloat ny;
	gfloat nz;
	char nb;
} vertex;

typedef struct {
	gint view_size;
	gint mesh_size;
	gint drawing_mesh_size;
	gint scale;
	gboolean direct_upd;
	gboolean show_subdialog;
	gboolean use_water;
	gfloat water_level; // 1.0 == 65535
	camera_struct *cameras[NBCAMERAS];
} gl_defaults_struct;

typedef struct {
	gboolean mouse_dragging1;
	gboolean mouse_dragging2;
	gboolean mouse_dragging3;
	gpointer grid; 	// Height field to display
			// Copy of a pointer allocated by the calling app
	gint max_x;	// Size of the grid
	gint max_y;
	gint data_type;	// Grid data type (HF_TYPE_ID, GINT_ID, GDOUBLE_ID...)
	gint last_x;	// Last mouse position, any button
	gint last_y;	// (for managing mouse dragging)
	gint view_size;	// Current size of the viewport (== size of the GL hf in pixels)
			// Default:  1/2 the default HF size, 2x the mesh size
	gint mesh_size;
	gint drawing_mesh_size;
	vertex *hf;
//	gint scale;	// The mesh size, related to the HF size:
			// 4, 3, 2, 1, 0 = 6, 12, 25, 50, 100 %
			// mesh_size = hf_size >> scale
			// Default : 2 -> 25%
	gint mesh_size_backup; // Backup for main mesh size when using low res mesh
	gint hf_list;
	gint current_camera_id;	// From 0 to NBCAMERAS-1 - updated by a radio button
	camera_struct *cameras[NBCAMERAS];
	gboolean refresh_on_mouse_down;
	GtkObject *adj_rot_y;
	GtkObject *adj_rot_x;
	GtkObject *adj_distance;
	GtkObject *adj_translate_x;
	GtkObject *adj_translate_y;
	GtkObject *adj_angle_w;
	GtkWidget *gl_dialog;
	GtkWidget *main_box;
	GtkWidget *refresh_button;
	GtkWidget *direct_upd;
	GtkWidget *gl_area;
	GtkWidget *details_percent_lbl;
	GtkWidget *if_water_chkbox;	// Check button for use_water
	gboolean *use_water;	// From the hf_struct
	GtkObject *adj_water_level;
	gfloat *water_level;	// 1.0 == 65535; from the hf_struct
	gl_defaults_struct *defaults; // Copy of the main gl_defaults pointer
} gl_preview_struct;

//	Private methods

GtkWidget *create_glarea  (gpointer callb_data, gint min_size);
gint       glarea_button_release (GtkWidget*, GdkEventButton*, gpointer);
gint       glarea_button_press   (GtkWidget*, GdkEventButton*, gpointer);
gint       glarea_motion_notify  (GtkWidget*, GdkEventMotion*, gpointer);
gint       glarea_draw           (GtkWidget*, GdkEventExpose*,gpointer);
gint       glarea_reshape        (GtkWidget*, GdkEventConfigure*, gpointer);
gint       glarea_init           (GtkWidget*, gpointer);
gint       glarea_destroy        (GtkWidget*, gpointer);
void 	gl_preview_free (gl_preview_struct *gl_preview);

/****************************************************************************************/

// Public methods

void gl_area_update(gl_preview_struct *) ; 
gl_defaults_struct *gl_defaults_init( );

gl_preview_struct *create_gl_preview_dialog (gl_defaults_struct *gl_def, GtkWidget *window, gpointer input_grid, gint max_x, gint max_y, gint data_type, gboolean *use_water_ptr, gfloat *water_level_ptr);

void gl_update_details_display (gl_preview_struct *gl_preview);

void	gl_details_min (gl_preview_struct* glp);
void 	gl_details_restore (gl_preview_struct* glp);

void gl_set_input_grid (gl_preview_struct *glp, gpointer grid, gint max_x, gint max_y, gint data_type);
void gl_save_water_inc (gl_preview_struct *gl_preview);
void gl_set_water_level (gl_preview_struct *gl_preview);

// Yet-to-be-determined-status method

void 	refresh_gl_callb(GtkWidget *refresh_button, gpointer hfw);
#endif // _GLPREVIEW
