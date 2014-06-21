/* camera.c - Managing the Povray camera in preview mode, from OpenGL settings
 *
 * Copyright (C) 2003 Patrice St-Gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "gl_preview.h"

/* These variables are now defined in globals.h and the RC file

#define DISTANCE_INIT 2.5
#define DEFAULT_ANGLE 55 // Povray standard (width), translates to ~38 in GL standard (height)
// #define DEFAULT_ROT_Y 180
#define DEFAULT_ROT_Y 0
// #define DEFAULT_ROT_X 140
#define DEFAULT_ROT_X 50
#define ASPECT_RATIO 1.3333333333

*/

extern gint DEFAULT_ANGLE[], DEFAULT_ROT_X[], DEFAULT_ROT_Y[];
extern gfloat DISTANCE_INIT[], ASPECT_RATIO[], RIGHT_LEFT_DISPL[], HEIGHT[]; 

// Camera_def is the default camera, from which others are reset
// Camera[NBCAMERAS] is used for keeping the current parameters between each new document

camera_struct camera_def = {FALSE, 0, 0, 0.0, 0.0, 0.0, 0, 0.0} ;

camera_struct cameras[NBCAMERAS] ;

/*************************************************************************************************/
// Camera management

//	How it works (2003-10-12)
//	1. The preview settings (translation and rotation of the HF) are translated
//	    into Povray camera parameters (rotation of the camera around the HF,
//	    negative translation). Povray lights are not rotated nor translated, as of v. 0.12.
//	2. Each document has 5 cameras.
//	3. Each camera can be reset to default values (in "camera_def").
//	4. The 5 cameras of a newly created document are copied from the last active document.
//	5. The 5 cameras of the last active document are saved in a the geomorph_session file
//
//	camera1, camera2... camera5 are used as buffers for "transmitting" the cameras settings
//	between the current document and the newly created ones
//	==> each time we activate a document, its cameras must be copied into these buffers

void copy_camera (camera_struct *camera_out, camera_struct *camera_in) {
	memcpy(camera_out, camera_in, sizeof(camera_struct));
}

void init_default_camera ( ) {
	if (camera_def.init)
		return;
	camera_def.init = TRUE;
	camera_def.rot_x = DEFAULT_ROT_X[0];
	camera_def.rot_y = DEFAULT_ROT_Y[0];
	camera_def.distance = DISTANCE_INIT[0];
	camera_def.translate_x = RIGHT_LEFT_DISPL[0];
	camera_def.translate_y = HEIGHT[0];
	camera_def.angle_w = DEFAULT_ANGLE[0];
	camera_def.aspect_ratio = ASPECT_RATIO[0];
} 

void init_cameras ( ) {
//	Method initializing the camera buffers (array of NBCAMERAS cameras)
//	with the default camera
	gint i;
	for (i=0; i<NBCAMERAS; i++) {
	//	Old way: initialize cameras from the default one
//		copy_camera (&cameras[i],&camera_def);
	//	Now we can have default values for all the 5 cameras
		cameras[i].init = TRUE;
		cameras[i].rot_x = DEFAULT_ROT_X[i];
		cameras[i].rot_y = DEFAULT_ROT_Y[i];
		cameras[i].distance = DISTANCE_INIT[i];
		cameras[i].translate_x = RIGHT_LEFT_DISPL[i];
		cameras[i].translate_y = HEIGHT[i];
		cameras[i].angle_w = DEFAULT_ANGLE[i];
		cameras[i].aspect_ratio = ASPECT_RATIO[i];
	}
}

void print_default_camera () {

	printf("DEFAULT CAMERA:\nINIT: %d;\nROT_X: %d;\nROT_Y: %d;\nDISTANCE: %5.3f;\n", camera_def.init,   camera_def.rot_x, camera_def.rot_y, camera_def.distance);
	printf("TRANSLATE_X: %5.3f;\nTRANSLATE_Y: %5.3f;\nANGLE_W: %d;\nASPECT_RATIO: %5.3f;\n", camera_def.translate_x,   camera_def.translate_y, camera_def.angle_w, camera_def.aspect_ratio);

	printf("DEFAULT_VALUES: \nROT_X: %d;\nROT_Y: %d;\nDISTANCE: %5.3f;\nANGLE_W: %d;\nASPECT_RATIO: %5.3f;\n", DEFAULT_ROT_X[0], DEFAULT_ROT_Y[0], DISTANCE_INIT[0], DEFAULT_ANGLE[0], ASPECT_RATIO[0]);
}

void gl_save_camera_inc (camera_struct * camera) {
//	Public method
//	Save the current camera parameters in "camera.inc" in the current directory

	FILE *fin;
	gchar *loc, *tmp, *msg_buf;
	if (!(fin=fopen("camera.inc","wb"))) {
		msg_buf = (gchar *) x_malloc(sizeof(gchar) + strlen("camera.inc") + strlen(_("Not able to open '%s' file for writing"))+5 , "const gchar (msg_buf - open file for writing)");
		sprintf(msg_buf,_("Not able to open '%s' file for writing"),"camera.inc");
		my_msg(msg_buf,WARNING);
	}
	else {
		tmp = setlocale(LC_NUMERIC,NULL); // Povray uses "." as decimal separator instead of ","
		loc = (gchar *) malloc(strlen(tmp)+1);
		strcpy(loc,tmp);
		setlocale(LC_NUMERIC,"C");
		fprintf(fin,"#declare gl_angle = %d;\n",camera->angle_w);
		fprintf(fin,"#declare aspect_ratio = %10.7f;\n",camera->aspect_ratio);
		fprintf(fin,"#declare tr_x = %10.7f;\n",-camera->translate_x/2.0);
		fprintf(fin,"#declare tr_y = %10.7f;\n",-camera->translate_y/2.0);
		fprintf(fin,"#declare tr_z = %10.7f;\n", -(1.35+(camera->distance-2.5)/2.0));
		fprintf(fin,"#declare rot_x = %d;\n",camera->rot_x);
		fprintf(fin,"#declare rot_y = %d;\n",camera->rot_y);
		fprintf(fin,"#declare rot_z = 0;\n");
		setlocale(LC_NUMERIC,loc);
		free(loc);
		fclose(fin);
	}
}

void set_perspective (camera_struct *camera) {

//	Set perspective
	gfloat angle;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// camera->angle_w is in the Povray standard (horizontal)
	// gluPerspective() takes a vertical angle
	angle = atanf( tanf(((gfloat) camera->angle_w)*PI/360.0)/camera->aspect_ratio);
	gluPerspective(angle*360.0/PI,camera->aspect_ratio,0.1,20.0);

	glMatrixMode(GL_MODELVIEW);
}

void set_camera (gpointer glp, camera_struct *camera) {
	gl_preview_struct *gl_preview = (gl_preview_struct *) glp;
	gtk_adjustment_set_value(
		GTK_ADJUSTMENT(gl_preview->adj_rot_y), camera->rot_y);
	gtk_adjustment_set_value(
		GTK_ADJUSTMENT(gl_preview->adj_rot_x), camera->rot_x);
	gtk_adjustment_set_value(
		GTK_ADJUSTMENT(gl_preview->adj_distance), camera->distance);
	gtk_adjustment_set_value(
		GTK_ADJUSTMENT(gl_preview->adj_translate_x), camera->translate_x);
	gtk_adjustment_set_value(
		GTK_ADJUSTMENT(gl_preview->adj_translate_y), camera->translate_y);
	gtk_adjustment_set_value(
		GTK_ADJUSTMENT(gl_preview->adj_angle_w), camera->angle_w);
	set_perspective (camera);
	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area));
}

void set_camera_callb (GtkWidget *wdg, gpointer data) {
//	Sets the camera from the label of a radio button
	gchar *txt;
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;

	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	gtk_label_get(GTK_LABEL(GTK_BIN(wdg)->child), &txt);

	gl_preview->current_camera_id = atoi(txt);
	if ( (gl_preview->current_camera_id<1) || (gl_preview->current_camera_id>NBCAMERAS))
		gl_preview->current_camera_id = 1;

	gl_preview->current_camera_id--;	 // current_camera_id is a C index

	set_camera (gl_preview, gl_preview->cameras[gl_preview->current_camera_id]);
}

