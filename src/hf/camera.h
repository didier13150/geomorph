/* camera.h - headers for OpenGL settings translatable to Povray camera
 *
 * Copyright (C) 2003 Patrice St-Gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _CAMERA
#define _CAMERA 1

#ifndef NBCAMERAS
#define NBCAMERAS 5
#endif

typedef struct {
	gboolean init;
	gint rot_x; // Rotation around X axis
	gint rot_y; // Rotation around vertical (Y) axis
	gfloat distance; // Distance from observer to origin ("lookat_z")
	gfloat translate_x;
	gfloat translate_y;
	gint angle_w;
	gfloat aspect_ratio;
} camera_struct;

// Public methods

void gl_save_camera_inc (camera_struct *);
void init_cameras ( );
void init_default_camera ( );
void print_default_camera ( );
void set_perspective (camera_struct *camera);
void set_camera_callb (GtkWidget *wdg, gpointer data);
void set_camera (gpointer glp, camera_struct *camera) ;
void copy_camera(camera_struct *camera_out, camera_struct *camera_in);

#endif // _CAMERA
