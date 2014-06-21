/* gl_preview.c - managing the OpenGL preview
 *
 * Copyright (C) 2002, 2005, 2010 Patrice St-Gelais
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
 
/* From gtkglarea_demo.c
 * Compile command:
 *
 * gcc gtkglarea_demo.c `gtk-config --libs --cflags` -lMesaGL -lgtkgl
 *
Height field display inspired from Xavier Michelon articles 
in Linux Magazine France, n. 33-34.
 ************/
 
 /* 2005-11: change library from gtkglarea to gtkglext
 */

#include <locale.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include "gl_preview.h"
// hf_calc.h for data types and some macros
#include "hf_calc.h"

#include "../icons/rotate_y.xpm"
#include "../icons/rotate_x.xpm"
#include "../icons/vertical.xpm"
#include "../icons/horizontal.xpm"
#include "../icons/depth.xpm"

// Ligths parameters
gfloat l0pos[] = {-2.0, 1.5, 3.0};
gfloat l0dif[] = {0.8, 0.8, 0.8};
gfloat water_colour[] = {0.2,0.4,0.6,0.6};
gfloat terrain_colour[] = {0.75, 0.5, 0.3, 1.0}; 
gfloat l1pos[] = {2.0, 1.0, -1.0};
gfloat l1dif[] = {0.5, 0.5, 0.5};
gfloat mspec[] = {0.05, 0.05, 0.05};
gfloat mshiny=5;

#define BASE_DISTANCE 0.5
// #define ROT_X_BASE 180
#define ROT_X_BASE 270
// #define ROT_Y_BASE 90
#define ROT_Y_BASE 270 // Values for ROT_X and ROT_Y made consistent with 0 Povray values

// Aspect ratio now defined in globals.h - thisappinit.c
// #define ASPECT_RATIO 1.3333333333

#define MAX_GL_SIZE 511
#define GL_SIZE_STEP 32
#define MIN_GL_SIZE 127
#define MESH_SIZE_MIN 32
#define MESH_SIZE_MAX 1024
#define HEIGHT_SCALE 0.75
#define BASE_TRANSLATE_Y -0.15

gboolean FLAG_CAMERA_CHANGE = TRUE;

// Camera_def is the default camera, from which others are reset
// Camera[NBCAMERAS] is used for keeping the current parameters between each new document
extern camera_struct camera_def;

extern camera_struct cameras[NBCAMERAS] ;

extern gint PREVIEW_SIZE, MESH_SIZE, DRAWING_MESH_SIZE;
extern gboolean SHOW_SUBDIALOG, DIRECT_UPDATE, DIRECT_RENDERING;
extern gfloat ASPECT_RATIO;
/*************************************************************************************************/

void gl_draw (GtkWidget *gl_area) {
//	New drawing function for GTK 2.0
	if (!gl_area)
		return;
	gtk_widget_queue_draw(GTK_WIDGET(gl_area));
//	We force the update, for a more fluid display when the pen is used
	if (gl_area->window)
		gdk_window_process_updates(gl_area->window, FALSE);
}

static void gl_reset_callb (GtkWidget *wdg, gpointer data) {

	// Callback for the reset button
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	camera_struct *camera;
	camera = gl_preview->cameras[gl_preview->current_camera_id];
	copy_camera (camera, &camera_def);
	set_camera (gl_preview, camera);
}

/*************************************************************************************************/

gl_defaults_struct *gl_defaults_init( ) {

	// Public method, called before creating any document
	gint i;
	gl_defaults_struct *gl_def;
	gl_def = (gl_defaults_struct *) x_malloc(sizeof(gl_defaults_struct), "gl_defaults_struct");
	gl_def->view_size= PREVIEW_SIZE;
	gl_def->mesh_size = MESH_SIZE;
	gl_def->drawing_mesh_size = DRAWING_MESH_SIZE;
	gl_def->direct_upd = DIRECT_UPDATE;
	gl_def->show_subdialog = SHOW_SUBDIALOG;
	init_cameras();
	for (i=0; i<NBCAMERAS; i++)
		gl_def->cameras[i] = &cameras[i];
	return gl_def;
}

/*****************************************************************************/
/*                                                                           */
/* Function: glarea_button_release (GtkWidget*, GdkEventButton*)             */
/*                                                                           */
/* This function handles button-release events for the GtkGLArea into which  */
/* we are drawing.                                                           */
/*                                                                           */
/*****************************************************************************/

gint glarea_button_release (GtkWidget* widget, GdkEventButton* event, gpointer data) {

  gl_preview_struct *gl_hf = (gl_preview_struct *) data;
  if (event->button == 1) {
  	gl_hf->mouse_dragging1 = FALSE;
    return TRUE;
  }

  if (event->button == 2) {
  	gl_hf->mouse_dragging2 = FALSE;
    return TRUE;
  }

  if (event->button == 3) {
  	gl_hf->mouse_dragging3 = FALSE;
    return TRUE;
  }
  return FALSE;
}

/*****************************************************************************/
/*                                                                           */
/* Function: glarea_button_press (GtkWidget*, GdkEventButton*)               */
/*                                                                           */
/* This function handles button-press events for the GtkGLArea into which we */
/* are drawing.                                                              */
/*                                                                           */
/*****************************************************************************/

gint glarea_button_press (GtkWidget* widget, GdkEventButton* event, gpointer data) {

  gl_preview_struct *gl_hf = (gl_preview_struct *) data;

  if (event->button == 1) {
  	gl_hf->mouse_dragging1 = TRUE;
    return TRUE;
  }

  if (event->button == 2) {
  	gl_hf->mouse_dragging2 = TRUE;
    return TRUE;
  }
  if (event->button == 3) {
  	gl_hf->mouse_dragging3 = TRUE;
    return TRUE;
  }
  return FALSE;
}

/*****************************************************************************/
/*                                                                           */
/* Function: glarea_motion_notify (GtkWidget*, GdkEventMotion*)              */
/*                                                                           */
/* This function handles motion events for the GtkGLArea into which we are   */
/* drawing                                                                   */
/*                                                                           */
/*****************************************************************************/

gint glarea_motion_notify (GtkWidget* widget, GdkEventMotion* event, gpointer data) {

  int x;
  int y;

  GdkModifierType state;
  camera_struct *camera;

  gl_preview_struct *gl_hf = (gl_preview_struct *) data;

  camera = gl_hf->cameras[gl_hf->current_camera_id];

  if (event->is_hint) {
    gdk_window_get_pointer(event->window, &x, &y, &state);
  }
  else {
    	x = event->x;
    	y = event->y;
    	state = event->state;
  }

  if (state & GDK_BUTTON1_MASK) {

	if (gl_hf->mouse_dragging1) {
		camera->rot_y += (gint) ((360.0 / (gfloat) gl_hf->view_size) * (gfloat) (x - gl_hf->last_x));
		camera->rot_y = (360+camera->rot_y) % 360;
		gtk_adjustment_set_value(
			GTK_ADJUSTMENT(gl_hf->adj_rot_y), camera->rot_y);
		camera->rot_x += (gint) ((360.0 / (gfloat) gl_hf->view_size) * (gfloat) (y - gl_hf->last_y));
		camera->rot_x = (360+camera->rot_x) % 360;
		gtk_adjustment_set_value(
			GTK_ADJUSTMENT(gl_hf->adj_rot_x), camera->rot_x);
	}
  }

  if (state & GDK_BUTTON2_MASK) {
		camera->translate_x +=
			((gfloat) (x - gl_hf->last_x)) / (gfloat) gl_hf->view_size;
		camera->translate_x = MAX(-1.0,MIN(camera->translate_x,1.0));
		gtk_adjustment_set_value(
			GTK_ADJUSTMENT(gl_hf->adj_translate_x), camera->translate_x);
		camera->translate_y -=
			((gfloat) (y - gl_hf->last_y)) / (gfloat) gl_hf->view_size;
		camera->translate_y = MAX(-1.0,MIN(camera->translate_y,1.0));
		gtk_adjustment_set_value(
			GTK_ADJUSTMENT(gl_hf->adj_translate_y), camera->translate_y);
  }

  if (state & GDK_BUTTON3_MASK) {
		camera->distance +=
			((gfloat) (x - gl_hf->last_x)) / (gfloat) gl_hf->view_size;
		camera->distance -=
			((gfloat) (y - gl_hf->last_y)) / (gfloat) gl_hf->view_size;
		camera->distance= MAX(0.0,MIN(camera->distance,5.0));
		gtk_adjustment_set_value(
			GTK_ADJUSTMENT(gl_hf->adj_distance), camera->distance);
  }
  gl_hf->last_x = x;
  gl_hf->last_y = y;
//  gtk_widget_queue_draw(GTK_WIDGET(gl_hf->gl_area));
  gl_draw(GTK_WIDGET(gl_hf->gl_area));
  return TRUE;

}

/*****************************************************************************/
/*                                                                           */
/* Function: glarea_reshape (GtkWidget*, GdkEventConfigure*)                 */
/*                                                                           */
/* This function performs the operations needed to maintain the viewing area */
/* of the GtkGLArea. This should be called whenever the size of the area     */
/* is changed.                                                               */
/*                                                                           */
/*****************************************************************************/

gint glarea_reshape (GtkWidget* widget, GdkEventConfigure* event, gpointer data) {

  
	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
	gint w = widget->allocation.width;
	gint h = widget->allocation.height;
	gfloat aspect_ratio, actual_aspect_ratio = ((gfloat) w)/ (gfloat) h;
	static gfloat epsilon = 0.01;
  	gl_preview_struct *glp = (gl_preview_struct *) data;
	aspect_ratio = glp->cameras[glp->current_camera_id]->aspect_ratio;

// printf("*************RESHAPE\n");

//	Keep the right ratio for the display area
	
	/*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return FALSE;
	
	if (ABS(aspect_ratio - actual_aspect_ratio) < epsilon) // Ratios are equal
		glViewport (0,0, w, h);
	else
		if (actual_aspect_ratio > aspect_ratio) {
			// Area widget too wide, we translate the viewport on the X axis
			glViewport ( (gint) ((w-(gint) (aspect_ratio*(gfloat) h)) / 2.0), 0,
				(gint) (aspect_ratio*(gfloat) h),h);
		}
		else { 	// Area widget too narrow, we translate the viewport on the Y axis
			glViewport ( 0,(gint) ((h-(gint)( ((gfloat) w)/aspect_ratio) ) / 2.0),
				w,(gint) ( ((gfloat) w)/aspect_ratio) );
		}
	
	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
	
	return (TRUE);

}

/*****************************************************************************/
/*                                                                           */
/* Function: glarea_init (GtkWidget*)                                        */
/*                                                                           */
/* This function is a callback for the realization of the GtkGLArea widtget. */
/*                                                                           */
/*****************************************************************************/

gint glarea_init (GtkWidget* widget, gpointer data) {

//	printf ("GLAREA_INIT - Realize Event\n");

  	gl_preview_struct *gl_hf = (gl_preview_struct *) data;

	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  /*** OpenGL BEGIN ***/
	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
		my_msg(_("Not able to display the Open GL preview"),WARNING);
		return FALSE;
	}
	
	glClearColor(0.0,0.0,0.0,1.0);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT, GL_FILL);
//	glDisable(GL_CULL_FACE);

	set_perspective (gl_hf->cameras[gl_hf->current_camera_id]);

//	Lights
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0dif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, l0dif);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, l1dif);
	glLightfv(GL_LIGHT1, GL_SPECULAR, l1dif);
	
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;

//	Materials
	glMaterialfv(GL_FRONT, GL_SPECULAR, mspec);
	glMaterialf(GL_FRONT, GL_SHININESS, mshiny);

	glShadeModel (GL_SMOOTH);

	if (gdk_gl_drawable_is_double_buffered (gldrawable))
 		gdk_gl_drawable_swap_buffers (gldrawable);
	else
		glFlush ();

	gdk_gl_drawable_gl_end (gldrawable);
	
	return TRUE;

}

/*****************************************************************************/
/*                                                                           */
/* Function: glarea_destroy (GtkWidget*)                                     */
/*                                                                           */
/* This function is a callback for the main GtkGLArea. It should             */
/* delete any data structures stored in the GtkGLArea.                       */
/*                                                                           */
/*****************************************************************************/

gint glarea_destroy (GtkWidget* widget, gpointer data) {

  	gl_preview_struct *glp = (gl_preview_struct *) data;

  /* Insert any required cleanup */
  /* code here.                  */
 
//	GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
	gl_preview_free(glp);
//	gdk_gl_context_destroy (glcontext);
	return FALSE;

}

GtkWidget* create_glarea (gpointer gl_preview_ptr, gint view_size) {

  GtkWidget* glarea;
  GdkGLConfig *glconfig;

  /*
   * Configure OpenGL-capable visual. 
   * IMPORTED from the gtkglext gears.c example 2005-11
   */

  /* Try double-buffered visual */
  glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB    |
					GDK_GL_MODE_DEPTH  |
					GDK_GL_MODE_DOUBLE);
  if (glconfig == NULL)
    {
        g_print ("*** Cannot find the double-buffered visual.\n");
        g_print ("*** Trying single-buffered visual.\n");

      /* Try single-buffered visual */
      glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB   |
					    GDK_GL_MODE_DEPTH);
      if (glconfig == NULL)
	{
//		my_msg(_("Not able to display the Open GL preview"),WARNING);
    		my_msg(_("Error creating GtkGLArea!\nPreview not available!"),WARNING);
    		return NULL;
	}
    }

  /* Now, create the GtkGLArea using the attribute list that  */
  /* we defined above.                                        */

  glarea = gtk_drawing_area_new ();
//  gtk_widget_set_size_request (drawing_area, 300, 300);

  /* Set OpenGL-capability to the widget. */
  gtk_widget_set_gl_capability (glarea,
				glconfig,
				NULL,
				DIRECT_RENDERING,
				GDK_GL_RGBA_TYPE);

  gtk_widget_set_events(GTK_WIDGET(glarea),
                        GDK_EXPOSURE_MASK|
                        GDK_BUTTON_PRESS_MASK|
			GDK_BUTTON_RELEASE_MASK|
			GDK_POINTER_MOTION_MASK|
                        GDK_POINTER_MOTION_HINT_MASK);

  /* button_release_event - The user has released one of the  */
  /*                        mouse buttons in the window.      */

  gtk_signal_connect (GTK_OBJECT(glarea), "button_release_event",
                      GTK_SIGNAL_FUNC(glarea_button_release), gl_preview_ptr);

  /* button_press_event - The user has pressed one of the     */
  /*                      mouse buttons in the window.        */

  gtk_signal_connect (GTK_OBJECT(glarea), "button_press_event",
                      GTK_SIGNAL_FUNC(glarea_button_press), gl_preview_ptr);
  
  /* motion_notify_event - The mouse is moving in the window. */

  gtk_signal_connect (GTK_OBJECT(glarea), "motion_notify_event",
                      GTK_SIGNAL_FUNC(glarea_motion_notify), gl_preview_ptr);
  
  /* expose_event - The window was exposed and the contents   */
  /*                need to be redrawn.                       */

  gtk_signal_connect (GTK_OBJECT(glarea), "expose_event",
                      GTK_SIGNAL_FUNC(glarea_draw), gl_preview_ptr);
  
  /* configure_event - The window has been resized. You will  */
  /*                   probably want to call your reshape     */
  /*                   function here.                         */

  gtk_signal_connect (GTK_OBJECT(glarea), "configure_event",
                      GTK_SIGNAL_FUNC(glarea_reshape), gl_preview_ptr);
  
  /* realize - The window has been created, this is where you */
  /*           can hook up your initialization routines.      */

  gtk_signal_connect (GTK_OBJECT(glarea), "realize",
                    GTK_SIGNAL_FUNC(glarea_init), gl_preview_ptr);

  /* destroy - The window has received a destroy event, this  */
  /*           is where you should do any cleanup that needs  */
  /*           to happen, such as de-allocating data objects  */
  /*           that you have added to your GtkGLArea.         */

  gtk_signal_connect (GTK_OBJECT(glarea), "destroy",
                     GTK_SIGNAL_FUNC (glarea_destroy), gl_preview_ptr);

  gtk_widget_show (GTK_WIDGET(glarea));

  gtk_widget_set_usize(GTK_WIDGET(glarea), view_size, view_size/ASPECT_RATIO);

  return (glarea);

}

void draw_vertex(gint index, vertex *hf) {
	glNormal3fv(&hf[index].nx);
	glVertex3fv(&hf[index].x);
}

gint create_hf_list (gl_preview_struct *hf) {
//	Creates the HF list for the current HF
	gint list, i, j;
	list = glGenLists(1);
//	printf("**************CREATE_HF_LIST\n");
	glNewList(list, GL_COMPILE);
	glColor4fv(terrain_colour);
	for (i=0; i<hf->mesh_size-1; i++)
		for (j=0; j<hf->mesh_size-1; j++) {
			glBegin(GL_TRIANGLES);
			// Triangle 1
		//	glEdgeFlag(TRUE);
			draw_vertex(VECTORIZE(i,j,hf->mesh_size),hf->hf);
			draw_vertex(VECTORIZE(i+1,j,hf->mesh_size),hf->hf);
		//	glEdgeFlag(FALSE);
			draw_vertex(VECTORIZE(i+1,j+1,hf->mesh_size),hf->hf);
			// Triangle 2
			draw_vertex(VECTORIZE(i,j,hf->mesh_size),hf->hf);
		//	glEdgeFlag(TRUE);
			draw_vertex(VECTORIZE(i+1,j+1,hf->mesh_size),hf->hf);
			draw_vertex(VECTORIZE(i,j+1,hf->mesh_size),hf->hf);
			glEnd();
		}

	glEndList();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	return list;
}

gint create_lights () {
	gint lights;
	lights = glGenLists(1);
	glNewList (lights, GL_COMPILE);
	glDisable(GL_LIGHTING);
	glColor4f(1.0,1.0,1.0,1.0);
	glPointSize(6.0);
		glBegin(GL_POINTS);
		glVertex3fv(l0pos);
		glVertex3fv(l1pos);
	glEnd();

	glLineWidth(1.0);
	glBegin(GL_LINES);
		glVertex3fv(l0pos);
		glVertex3f(0.0,0.0,0.0);
		glVertex3fv(l1pos);
		glVertex3f(0.0,0.0,0.0);
	glEnd();

	glEnable(GL_LIGHTING);

	glEndList();

	return lights;
}

/*****************************************************************************/
/*                                                                           */
/* Function: glarea_draw (GtkWidget*, GdkEventExpose*)                       */
/*                                                                           */
/* This is the function that should render your scene to the GtkGLArea. It   */
/* can be used as a callback to the 'Expose' event.                          */
/*                                                                           */
/*****************************************************************************/

gint glarea_draw (GtkWidget* widget, GdkEventExpose* event, gpointer data) {

//	Rotates, translate, etc. the scene but does not refresh the mesh

	gint i;
	camera_struct *camera;
  	gl_preview_struct *gl_hf = (gl_preview_struct *) data;
	camera = gl_hf->cameras[gl_hf->current_camera_id];
  /* Draw only on the last expose event. */

//	printf("************* GL DRAWING 1 *************\n");

  if (event->count > 0) {
    return(TRUE);
  }

//	printf("************* GL DRAWING 2 **************\n");
  
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

    /* Clear the drawing color buffer and depth buffers */
    /* before drawing.                                  */

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	Drawing instructions
 	glLoadIdentity();
	gluLookAt(0.0,0.0,camera->distance + BASE_DISTANCE,0.0,0.0,0.0,0.0,1.0,0.0);
	glLightfv(GL_LIGHT0,GL_POSITION,l0pos);
	glLightfv(GL_LIGHT1,GL_POSITION,l1pos);

//	glCallList(lights_list);

	glTranslatef(camera->translate_x, camera->translate_y+BASE_TRANSLATE_Y,0.0);
	glRotatef(camera->rot_x+ROT_X_BASE,1.0,0.0,0.0);
	glRotatef(camera->rot_y+ROT_Y_BASE,0.0,0.0,1.0);
	glCallList(gl_hf->hf_list);

	// A very simple water plane
	if (gl_hf->use_water && *gl_hf->use_water) {
		glColor4fv(water_colour);
		glBegin(GL_POLYGON);
			glVertex3f(1.5,-1.5,*gl_hf->water_level*HEIGHT_SCALE);
			glVertex3f(1.5,1.5,*gl_hf->water_level*HEIGHT_SCALE);
			glVertex3f(-1.5,1.5,*gl_hf->water_level*HEIGHT_SCALE);
			glVertex3f(-1.5,-1.5,*gl_hf->water_level*HEIGHT_SCALE);
		glEnd();
	}

//	glCallList(normals_list);

    // Back up the cameras
    if (FLAG_CAMERA_CHANGE) {
	for(i=0; i<NBCAMERAS; i++) {
		copy_camera(&cameras[i],gl_hf->cameras[i]);
	}
	FLAG_CAMERA_CHANGE = FALSE;
    }
    if (gdk_gl_drawable_is_double_buffered (gldrawable))
    	gdk_gl_drawable_swap_buffers (gldrawable);
    else
    	glFlush ();

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/
  return (TRUE);

}

void create_vertices(gl_preview_struct *gl_hf) {
//	Create the Open GL vertices from the HF buffer
	gint i,j,indx;
	unsigned int lag;
	vertex *p1, *p2, *p3, *p4;
	vertex v1,v2,v3;
	gfloat step,incx, incy, incz, norm, value, height_scale;
	
	switch (gl_hf->data_type) {
		case GDOUBLE_ID:
			height_scale = 1.0 / HEIGHT_SCALE;
			break;
		case HF_TYPE_ID:
			height_scale = 65535.0 / HEIGHT_SCALE;
			break;
		case GINT_ID:
			height_scale = 65535.0 / HEIGHT_SCALE;
			break;
		default: // UNSIGNED_CHAR_ID
			height_scale = 256.0 / HEIGHT_SCALE;
	}

	step = 2.0 / (gl_hf->mesh_size-1);
	if (gl_hf->hf)
		x_free(gl_hf->hf);
	gl_hf->hf = (vertex *) x_malloc(sizeof(vertex) * (gl_hf->mesh_size+1) * (gl_hf->mesh_size+1), "vertex");
//	printf("CREATE_VERTICES - MESH_SIZE: %d; step: %6.4f; Height scale: %5.3f\n", gl_hf->mesh_size, step, height_scale);
//	Initializing normals
	for (i=0; i<gl_hf->mesh_size; i++)
		for (j=0; j<gl_hf->mesh_size; j++) {
			gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)].nx = 0.0;
			gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)].ny = 0.0;
			gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)].nz = 0.0;
		}
// printf("********* Normals initialized *************\n");

//	Filling vertices array
// 	**** MODIFY TO ALLOW NON SQUARE GRIDS ****
	lag = log2i(gl_hf->max_x)-log2i(gl_hf->mesh_size);
// printf("***************MAX_X: %d;  MESH_SIZE: %d; LAG: %d\n", gl_hf->max_x, gl_hf->mesh_size, lag);
	for (i=0; i<gl_hf->mesh_size; i++)
		for (j=0; j<gl_hf->mesh_size; j++) {
			gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)].x = -1+i*step;
			gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)].y = -1+j*step;
			indx = VECTORIZE(j<<lag,i<<lag,gl_hf->max_x);
			switch (gl_hf->data_type) {
				case GDOUBLE_ID:
					value = (gfloat) *(((gdouble*) gl_hf->grid)+indx);
					break;
				case HF_TYPE_ID:
					value = (gfloat) *(((hf_type*) gl_hf->grid)+indx);
					break;
				case GINT_ID:
					value = (gfloat) *(((gint*) gl_hf->grid)+indx);
					break;
				default: // UNSIGNED_CHAR_ID
					value = (gfloat) *(((unsigned char*) gl_hf->grid)+indx);
			}
			gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)].z = value / height_scale;

		}

// printf("********* Vertices array filled *************\n");

	for (i=0; i<gl_hf->mesh_size-1; i++)
		for (j=0; j<gl_hf->mesh_size-1; j++) {
			p1 = &gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)];
			p2 = &gl_hf->hf[VECTORIZE(i+1,j,gl_hf->mesh_size)];
			p3 = &gl_hf->hf[VECTORIZE(i+1,j+1,gl_hf->mesh_size)];
			p4 = &gl_hf->hf[VECTORIZE(i,j+1,gl_hf->mesh_size)];
			// v1, v2, v3:  vectors used to calculate normals to triangles
			v1.x = p2->x - p1->x;
			v1.y = p2->y - p1->y;
			v1.z = p2->z - p1->z;
			v2.x = p3->x - p1->x;
			v2.y = p3->y - p1->y;
			v2.z = p3->z - p1->z;
			v3.x = p4->x - p1->x;
			v3.y = p4->y - p1->y;
			v3.z = p4->z - p1->z;
			// Vector product = unnormalized normal
			incx = v2.y*v1.z - v1.y*v2.z;
			incy = v2.z*v1.x - v1.z*v2.x;
			incz = v2.x*v1.y - v1.x*v2.y;
			norm = sqrt(incx*incx+incy*incy+incz*incz);
			incx = incx / norm;
			incy = incy / norm;
			incz = incz / norm;
			p1->nx -= incx;
			p1->ny -= incy;
			p1->nz -= incz;
			p2->nx -= incx;
			p2->ny -= incy;
			p2->nz -= incz;
			p3->nx -= incx;
			p3->ny -= incy;
			p3->nz -= incz;
			incx = v3.y*v2.z - v2.y*v3.z;
			incy = v3.z*v2.x - v2.z*v3.x;
			incz = v3.x*v2.y - v2.x*v3.y;
			p1->nx -= incx;
			p1->ny -= incy;
			p1->nz -= incz;
			p3->nx -= incx;
			p3->ny -= incy;
			p3->nz -= incz;
			p4->nx -= incx;
			p4->ny -= incy;
			p4->nz -= incz;
		}
//	Nomalizing normals
	for (i=0; i<gl_hf->mesh_size; i++)
		for (j=0; j<gl_hf->mesh_size; j++) {
			p1 = &gl_hf->hf[VECTORIZE(i,j,gl_hf->mesh_size)];
			norm = sqrt(p1->nx*p1->nx + p1->ny*p1->ny + p1->nz*p1->nz);
			p1->nx = p1->nx / norm;
			p1->ny = p1->ny / norm;
			p1->nz = p1->nz / norm;
// printf("(%d,%d): (x,y,z): (%f, %f, %f);  (nx,ny,nz): (%f,%f,%f); \n",i,j,p1->x,p1->y,p1->z,p1->nx,p1->ny,p1->nz);
		}

// printf("********* Normals normalized *************\n");

}

void refresh_gl_callb(GtkWidget *refresh_button, gpointer data) {
  	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
//	gint t1;
//	printf("Refresh_gl_callb - 1\n");
	if (!gl_preview)
		return;
	if (!gl_preview->gl_area)
		return;
	if (!gl_preview->refresh_on_mouse_down)
		return;
//	printf("Refresh_gl_callb - 2\n");
//	t1 = clock();
//	printf("(X,Y): (%d,%d); Mesh size: %d;\n",gl_preview->max_x, gl_preview->max_y, gl_preview->mesh_size);
	if (glIsList(gl_preview->hf_list))
		glDeleteLists(gl_preview->hf_list,1);
//	if (glIsList(lights_list))
//		glDeleteLists(lights_list,1);
	create_vertices(gl_preview);
	gl_preview->hf_list = create_hf_list(gl_preview);
//	lights_list = create_lights();
//	Expose the gl_area widget
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
//	printf("Mesh size: %d; preview refresh delay: %d\n",gl_preview->mesh_size, clock() - t1);
}

void gl_area_update(gl_preview_struct *gl_preview) {
//	Public method, allowing automatic refresh of the GL display
	if (!gl_preview)
		return;
	gl_set_water_level (gl_preview);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gl_preview->direct_upd))) {;
		FLAG_CAMERA_CHANGE = TRUE;
		refresh_gl_callb(NULL, gl_preview);
	}
}

static void gl_area_redraw (GtkWidget *wdg, gpointer data) {
//	Simple redraw, for instance when changing the water plane height
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
}

static void rot_x_upd (GtkWidget *wdg, gpointer data) {
//	Rotation around X axis
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	camera_struct *camera = gl_preview->cameras[gl_preview->current_camera_id];
	if (camera->rot_x == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	camera->rot_x = (gint) GTK_ADJUSTMENT(wdg)->value;
	FLAG_CAMERA_CHANGE = TRUE;
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
//	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area));
}

static void rot_y_upd (GtkWidget *wdg, gpointer data) {
//	Rotation around vertical axis (Y)
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	camera_struct *camera = gl_preview->cameras[gl_preview->current_camera_id];
	if (camera->rot_y == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	camera->rot_y = (gint) GTK_ADJUSTMENT(wdg)->value;
	FLAG_CAMERA_CHANGE = TRUE;
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
//	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area));
}

static void distance_upd (GtkWidget *wdg, gpointer data) {
//	Distance (depth / viewpoint translation on Z axis)
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	camera_struct *camera = gl_preview->cameras[gl_preview->current_camera_id];
	if (camera->distance == (gfloat) GTK_ADJUSTMENT(wdg)->value)
		return;
	camera->distance = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	FLAG_CAMERA_CHANGE = TRUE;
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
//	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area));
}
static void translate_x_upd (GtkWidget *wdg, gpointer data) {
//	Translation on X axis
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	camera_struct *camera = gl_preview->cameras[gl_preview->current_camera_id];
	if (camera->translate_x == (gfloat) GTK_ADJUSTMENT(wdg)->value)
		return;
	camera->translate_x = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	FLAG_CAMERA_CHANGE = TRUE;
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
//	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area));
}
static void translate_y_upd (GtkWidget *wdg, gpointer data) {
//	Translation on Y axis (vertical)
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	camera_struct *camera = gl_preview->cameras[gl_preview->current_camera_id];
	if (camera->translate_y == (gfloat) GTK_ADJUSTMENT(wdg)->value)
		return;
	camera->translate_y = (gfloat) GTK_ADJUSTMENT(wdg)->value;
	FLAG_CAMERA_CHANGE = TRUE;
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
//	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area));
}

static void angle_w_upd (GtkWidget *wdg, gpointer data) {
//	Angle variation
//	Specified in the widget as horizontal (the Povray standard),
//	must be transformed to vertical in OpenGL
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	camera_struct *camera = gl_preview->cameras[gl_preview->current_camera_id];
	if (camera->angle_w == (gint) GTK_ADJUSTMENT(wdg)->value)
		return;
	camera->angle_w = (gint) GTK_ADJUSTMENT(wdg)->value;
	set_perspective(camera);
	FLAG_CAMERA_CHANGE = TRUE;
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
//	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area));
}

static void size_up_callb (GtkWidget *wdg, gpointer data) {
//	Area resize
	gl_preview_struct *gls = (gl_preview_struct *) data;
	gls->view_size =
		MIN(gls->view_size + GL_SIZE_STEP , MAX_GL_SIZE);
	gls->defaults->view_size = gls->view_size ;
	gtk_widget_set_usize(GTK_WIDGET(gls->gl_area),
		gls->view_size,gls->view_size/ASPECT_RATIO);
}

static void size_down_callb (GtkWidget *wdg, gpointer data) {
//	Area resize
	gl_preview_struct *gls = (gl_preview_struct *) data;
	gls->view_size =
		MAX(gls->view_size - GL_SIZE_STEP , MIN_GL_SIZE);
	gls->defaults->view_size = gls->view_size ;
	gtk_widget_set_usize(GTK_WIDGET(gls->gl_area),
		gls->view_size,gls->view_size/ASPECT_RATIO);
}

void gl_update_details_display (gl_preview_struct *gl_preview) {
	static gchar txt[6] = "     ";
//	Display details as % of HF size
	sprintf(txt, "%3d %%", (gint) (100.0*( ((gfloat) gl_preview->mesh_size) / (gfloat) gl_preview->max_x)) );
	gtk_label_set_text(GTK_LABEL(gl_preview->details_percent_lbl), txt);
}

static void details_up_callb (GtkWidget *wdg, gpointer data) {
//	Increase by 100% the number of polygons

	gl_preview_struct *gl_preview = (gl_preview_struct *) data;
	gl_preview->mesh_size = gl_preview->mesh_size << 1;
	if ( (gl_preview->mesh_size > MESH_SIZE_MAX) || 
			(gl_preview->mesh_size > gl_preview->max_x) ) {
		gl_preview->mesh_size = gl_preview->mesh_size_backup;
		return;
	}
		
	gl_preview->mesh_size_backup = gl_preview->mesh_size;
	gl_preview->defaults->mesh_size = gl_preview->mesh_size;
	gl_update_details_display (gl_preview);
	refresh_gl_callb(wdg,data);
}

static void details_down_callb (GtkWidget *wdg, gpointer data) {
//	Decrease by 50% the number of polygons
	gl_preview_struct *gl_preview = (gl_preview_struct *) data;

	gl_preview->mesh_size = gl_preview->mesh_size >> 1;
	
	if ( gl_preview->mesh_size < MESH_SIZE_MIN ) {
		gl_preview->mesh_size = gl_preview->mesh_size_backup;
		return;
	}
	
	gl_preview->mesh_size_backup = gl_preview->mesh_size;
	gl_preview->defaults->mesh_size = gl_preview->mesh_size;
	gl_update_details_display (gl_preview);
	refresh_gl_callb(wdg,data);
}

void gl_details_min (gl_preview_struct *gl_preview) {
	// Drop the mesh size to the minimum, for instance when drawing
	gl_preview->mesh_size_backup =  gl_preview->mesh_size;
	gl_preview->mesh_size = gl_preview->drawing_mesh_size;
}
void gl_details_restore (gl_preview_struct *gl_preview) {
	// Restore the mesh size to its normal value
	gl_preview->mesh_size =  gl_preview->mesh_size_backup;
}

gl_preview_struct *gl_preview_new (gl_defaults_struct *gl_def) {

	gl_preview_struct *gl_preview;
	gint i;

	gl_preview = (gl_preview_struct *) x_malloc(sizeof(gl_preview_struct), "gl_preview_struct");
	gl_preview->defaults = gl_def;
	gl_preview->mouse_dragging1 = FALSE;
	gl_preview->mouse_dragging2 = FALSE;
	gl_preview->mouse_dragging3 = FALSE;
	gl_preview->last_x = 0;
	gl_preview->last_y = 0;
	gl_preview->mesh_size = gl_def->mesh_size;
	gl_preview->mesh_size_backup = gl_def->mesh_size ;
	gl_preview->drawing_mesh_size = gl_def->drawing_mesh_size ;
	for (i=0; i<NBCAMERAS; i++) {
		gl_preview->cameras[i] = (camera_struct *) x_malloc(sizeof(camera_struct), "camera_struct");
		copy_camera(gl_preview->cameras[i], gl_def->cameras[i]);
	}
	gl_preview->current_camera_id = 0;
	gl_preview->hf_list = 0;
	gl_preview->hf = NULL;
	gl_preview->view_size = gl_def->view_size;
	gl_preview->gl_area = NULL;
	gl_preview->refresh_on_mouse_down = TRUE;

	gl_preview->grid = NULL;
	gl_preview->max_x = 0;
	gl_preview->max_y = 0;
	gl_preview->data_type = HF_TYPE_ID;
	gl_preview->if_water_chkbox = NULL;
	gl_preview->adj_water_level = NULL;
	gl_preview->use_water = NULL;
	gl_preview->water_level = NULL;

	return (gl_preview);
}

void gl_preview_free (gl_preview_struct *gl_preview) {
	gint i;
//	static GtkWidget *w;
// printf("GL_PREVIEW_FREE - 1; GL_AREA: %x = %d\n", gl_preview->gl_area,gl_preview->gl_area);
	if (gl_preview)
		if (gl_preview->cameras) {
			for (i=0; i<NBCAMERAS; i++) {
				if (gl_preview->cameras[i])
					x_free(gl_preview->cameras[i]);
			}
		if (gl_preview->hf)
			x_free(gl_preview->hf);
		x_free (gl_preview);
	}
// printf("GL_PREVIEW_FREE - 2\n");
}

void gl_set_input_grid (gl_preview_struct *glp, gpointer grid, gint max_x, gint max_y, gint data_type) {
	// Reinitializes the input buffer
	if (!grid)
		return;
	glp->grid = grid;
	glp->max_x = max_x;
	glp->max_y = max_y;
	glp->data_type = data_type;
	refresh_gl_callb (NULL, glp);
}

void gl_save_water_inc (gl_preview_struct *gl_preview) {
//	Public method
//	Save the current water plane parameters in "geomorph_water.inc" in the current directory
//	Eventually move in a scene management file

	FILE *fin;
	gchar *loc, *tmp, *msg_buf;
	if (!(fin=fopen("water.inc","wb"))) {
		msg_buf = (gchar *) x_malloc(sizeof(gchar) + strlen("water.inc") + strlen(_("Not able to open '%s' file for writing"))+5 , "const gchar (msg_buf - open file for writing)");
		sprintf(msg_buf,_("Not able to open '%s' file for writing"),"water.inc");
		my_msg(msg_buf,WARNING);
	}
	else {
		tmp = setlocale(LC_NUMERIC,NULL); // Povray uses "." as decimal separator instead of ","
		loc = (gchar *) malloc(strlen(tmp)+1);
		strcpy(loc,tmp);
		setlocale(LC_NUMERIC,"C");
		if (gl_preview->use_water && *gl_preview->use_water)
			fprintf(fin,"#declare preview_water_level = %5.2f;\n",*gl_preview->water_level);
		else
			fprintf(fin," ");
		setlocale(LC_NUMERIC,loc);
		free(loc);
		fclose(fin);
	}
}

void gl_set_water_level (gl_preview_struct *gl_preview) {
	if ((!gl_preview->water_level) || (!gl_preview->use_water)) {
		printf("No water plane dialog to set!\n");
		return;
	}
//	printf("gl_set_water_level: use_water = %d; water_level = %f\n",*gl_preview->use_water, *gl_preview->water_level);
	if (gl_preview->if_water_chkbox)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gl_preview->if_water_chkbox),*gl_preview->use_water);
	if (gl_preview->adj_water_level)
		gtk_adjustment_set_value (GTK_ADJUSTMENT(gl_preview->adj_water_level), *gl_preview->water_level);
}

static GtkWidget *gl_create_water_plane (gl_preview_struct *gl_preview) {

	GtkWidget *vbox, *scale;

	// 1. Create Vbox for water control
	vbox =  gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));

	if ((!gl_preview->use_water) || (!gl_preview->water_level))
	// Nothing to do if pointers are NULL
		return vbox;

	// 2. Pack water label + check button on/off + scale in Vbox

	define_label_in_box("Water", vbox, FALSE, TRUE, 0);
	gl_preview->if_water_chkbox = gtk_check_button_new ();
	gtk_widget_show (gl_preview->if_water_chkbox);
	gtk_box_pack_start (GTK_BOX(vbox), align_widget(gl_preview->if_water_chkbox,ALIGN_CENTER, ALIGN_CENTER), FALSE, FALSE, DEF_PAD*0.5);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gl_preview->if_water_chkbox), *gl_preview->use_water);

	gl_preview->adj_water_level = gtk_adjustment_new (*gl_preview->water_level, 0.0, 1.1, 0.01, 0.1, 0.1);
	g_signal_connect (G_OBJECT(gl_preview->adj_water_level), "value_changed",
		GTK_SIGNAL_FUNC(gfloat_adj_callb), gl_preview->water_level);
	g_signal_connect (G_OBJECT(gl_preview->adj_water_level), "value_changed",
		GTK_SIGNAL_FUNC(gl_area_redraw), gl_preview);
	scale = define_scale_in_box (gl_preview->adj_water_level, vbox, 2, DEF_PAD*0.5);
	gtk_range_set_inverted (GTK_RANGE(scale), TRUE);

	g_signal_connect (G_OBJECT(gl_preview->if_water_chkbox), "toggled", GTK_SIGNAL_FUNC(sensitive_if_true), scale);
	g_signal_connect (G_OBJECT(gl_preview->if_water_chkbox), "toggled", 
		GTK_SIGNAL_FUNC(toggle_check_button_callb), gl_preview->use_water);
	g_signal_connect (G_OBJECT(gl_preview->if_water_chkbox), "toggled", GTK_SIGNAL_FUNC(gl_area_redraw), gl_preview);

	return vbox;
}

gl_preview_struct *create_gl_preview_dialog (gl_defaults_struct *gl_def, GtkWidget *window, gpointer input_grid, gint max_x, gint max_y, gint data_type, gboolean *use_water_ptr, gfloat *water_level_ptr) {

	GtkWidget *vbox, *vbox2, *hbox, *table, *wdg;
	gint i;
	gchar buf[2]=" ";
	gl_preview_struct *gl_preview;
	camera_struct *camera;
	GSList *group = NULL;

// printf("CREATE_PREVIEW_DIALOG\n");

	gl_preview = gl_preview_new (gl_def);
	gl_preview->grid = input_grid;
	gl_preview->max_x = max_x;
	gl_preview->max_y = max_y;
	gl_preview->data_type = data_type;
	gl_preview->use_water = use_water_ptr;
	gl_preview->water_level = water_level_ptr;

	if (!gl_preview)
		return NULL;
		
	gl_preview->gl_area = create_glarea(gl_preview, gl_def->view_size);

	if (!gl_preview->gl_area) {
		gl_preview_free(gl_preview);
		return NULL;
	}
		
	gl_preview->gl_dialog = frame_new("Preview",DEF_PAD);

	gl_preview->main_box = gtk_vbox_new(FALSE,0);
	gtk_widget_show(gl_preview->main_box);
	gtk_container_add(GTK_CONTAINER(gl_preview->gl_dialog), gl_preview->main_box);

	// Pack gl area + water plane in the main box
	
	// Create Hbox to put gl_area + water control Vbox
	hbox =  gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	
	gtk_box_pack_start (GTK_BOX (hbox), gl_preview->gl_area,
		FALSE, FALSE, DEF_PAD*0.5);

	gtk_box_pack_start (GTK_BOX (hbox), 
		gl_create_water_plane (gl_preview), FALSE, FALSE, DEF_PAD);

	gtk_box_pack_start (GTK_BOX (gl_preview->main_box), 
		hbox, FALSE, FALSE, DEF_PAD);
	
	// Hbox for controls	
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	gl_preview->direct_upd = 
		define_check_button_in_box ("Auto update",hbox, FALSE,FALSE,0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gl_preview->direct_upd), 
		gl_def->direct_upd);

	gl_preview->refresh_button = gtk_button_new_with_label (_("Refresh"));
	gtk_signal_connect (GTK_OBJECT (gl_preview->refresh_button), "clicked",
	       (GtkSignalFunc) refresh_gl_callb, gl_preview);
	gtk_widget_show(gl_preview->refresh_button);	

  	gtk_box_pack_start (GTK_BOX (hbox), 
			gl_preview->refresh_button, FALSE, FALSE, 0);

  	gtk_box_pack_start (GTK_BOX (gl_preview->main_box), align_widget(hbox,0.5,0.5),
		FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	define_label_in_box("Size", hbox, FALSE,TRUE, 0);
	wdg = gtk_button_new_with_label(" + ");
	gtk_widget_show(GTK_WIDGET(wdg));
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
		GTK_SIGNAL_FUNC (size_up_callb), (gpointer) gl_preview);
  	gtk_box_pack_start (GTK_BOX (hbox), wdg, FALSE, FALSE, DEF_PAD*0.5);

	wdg = gtk_button_new_with_label(" - ");
	gtk_widget_show(GTK_WIDGET(wdg));
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
		GTK_SIGNAL_FUNC (size_down_callb), (gpointer) gl_preview);
  	gtk_box_pack_start (GTK_BOX (hbox), wdg, FALSE, FALSE, 0);

	define_label_in_box("  ", hbox, FALSE,TRUE, 0);
	define_label_in_box("Detail", hbox, FALSE,TRUE, 0);

	wdg = gtk_button_new_with_label(" + ");
	gtk_widget_show(GTK_WIDGET(wdg));
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
		GTK_SIGNAL_FUNC (details_up_callb), gl_preview);
  	gtk_box_pack_start (GTK_BOX (hbox), wdg, FALSE, FALSE, DEF_PAD*0.5);

	wdg = gtk_button_new_with_label(" - ");
	gtk_widget_show(GTK_WIDGET(wdg));
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
		GTK_SIGNAL_FUNC (details_down_callb), gl_preview);
  	gtk_box_pack_start (GTK_BOX (hbox), wdg, FALSE, FALSE, 0);

//	Display details as % of HF size
	
	gl_preview->details_percent_lbl = define_label_in_box (" ", hbox, FALSE,TRUE,DEF_PAD*0.5);

	gl_update_details_display (gl_preview);
		
  	gtk_box_pack_start (GTK_BOX (gl_preview->main_box), align_widget(hbox,0.5,0.5),
		FALSE, FALSE, 0);

	// Camera choice (radio buttons)

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	define_label_in_box ("Camera", hbox, 0, 0, DEF_PAD);
	for (i=0; i<NBCAMERAS; i++) {
		snprintf (buf, 2, "%1d", i+1);
		define_radio_button_in_box_with_data (hbox, &group, buf, set_camera_callb,
			(gpointer) gl_preview,(gl_preview->current_camera_id==i));
	}

	camera = gl_preview->cameras[gl_preview->current_camera_id];

	gtk_box_pack_start_defaults (GTK_BOX (gl_preview->main_box),hbox);

	table = gtk_table_new(5, 3, FALSE);
	gtk_widget_show(GTK_WIDGET(table));

	wdg = create_widget_from_xpm(window,rotate_y_xpm);
	gtk_table_attach (GTK_TABLE (table), wdg, 0, 1, 0, 1, 0, 0, 0, 0);
 	gl_preview->adj_rot_y =
		gtk_adjustment_new (camera->rot_y, 0.0, 360.0, 1.0, 1.0, 0.0);
	define_scale_in_table(gl_preview->adj_rot_y,table,1, 2, 0, 1, 0, 0);
	gtk_signal_connect (GTK_OBJECT (gl_preview->adj_rot_y ), "value_changed",
		GTK_SIGNAL_FUNC (rot_y_upd), (gpointer) gl_preview);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	rotate_buttons_new(hbox, (gpointer) gl_preview->adj_rot_y);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 0, 1, 0, 0, 0, 0);

	wdg = create_widget_from_xpm(window,rotate_x_xpm);
	gtk_table_attach (GTK_TABLE (table), wdg, 0, 1, 1, 2, 0, 0, 0, 0);
 	gl_preview->adj_rot_x =
		gtk_adjustment_new (camera->rot_x, 0.0, 360.0, 1.0, 1.0, 0.0);
	define_scale_in_table(gl_preview->adj_rot_x,table,1, 2, 1, 2, 0, 0);
	gtk_signal_connect (GTK_OBJECT (gl_preview->adj_rot_x ), "value_changed",
		GTK_SIGNAL_FUNC (rot_x_upd), (gpointer) gl_preview);

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);

	rotate_buttons_new(hbox, (gpointer) gl_preview->adj_rot_x);
	gtk_table_attach (GTK_TABLE (table), hbox, 2, 3, 1, 2, 0, 0, 0, 0);

	wdg = create_widget_from_xpm(window,depth_xpm);
	gtk_table_attach (GTK_TABLE (table), wdg, 0, 1, 2, 3, 0, 0, 0, 0);
 	gl_preview->adj_distance =
		gtk_adjustment_new (camera->distance, 0.0, 5.0, 0.01, 0.05, 0.001);
	define_scale_in_table(gl_preview->adj_distance,table,1, 3, 2, 3, 2, 0);
	gtk_signal_connect (GTK_OBJECT (gl_preview->adj_distance), "value_changed",
		GTK_SIGNAL_FUNC (distance_upd), (gpointer) gl_preview);

	wdg = create_widget_from_xpm(window,vertical_xpm);
	gtk_table_attach (GTK_TABLE (table), wdg, 0, 1, 3, 4, 0, 0, 0, 0);
 	gl_preview->adj_translate_y =
		gtk_adjustment_new (camera->translate_y, -1.0, 1.0, 0.01, 0.05, 0.001);
	define_scale_in_table(gl_preview->adj_translate_y,table,1, 3, 3, 4, 2, 0);
	gtk_signal_connect (GTK_OBJECT (gl_preview->adj_translate_y), "value_changed",
		GTK_SIGNAL_FUNC (translate_y_upd), (gpointer) gl_preview);

	wdg = create_widget_from_xpm(window,horizontal_xpm);
	gtk_table_attach (GTK_TABLE (table), wdg, 0, 1, 4, 5, 0, 0, 0, 0);
 	gl_preview->adj_translate_x =
		gtk_adjustment_new (camera->translate_x, -1.0, 1.0, 0.01, 0.05, 0.001);
	define_scale_in_table(gl_preview->adj_translate_x,table,1, 3, 4, 5, 2, 0);
	gtk_signal_connect (GTK_OBJECT (gl_preview->adj_translate_x), "value_changed",
		GTK_SIGNAL_FUNC (translate_x_upd), (gpointer) gl_preview);

	// vbox for table and related widgets (angle, reset)
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	gtk_box_pack_start_defaults(GTK_BOX(vbox), table);

	//	Angle
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_label_in_box ("Angle", hbox, 0, 0, DEF_PAD);
	gl_preview->adj_angle_w =
		gtk_adjustment_new (camera->angle_w, 5, 100, 1, 1, 0.1);
	gtk_signal_connect (GTK_OBJECT (gl_preview->adj_angle_w), "value_changed",
		GTK_SIGNAL_FUNC(angle_w_upd), (gpointer) gl_preview);
	define_scale_in_box (gl_preview->adj_angle_w, hbox, 0, DEF_PAD*0.5);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	// Reset all
	wdg = gtk_button_new_with_label (_("Reset"));
	gtk_signal_connect (GTK_OBJECT (wdg), "clicked",
	       (GtkSignalFunc) gl_reset_callb, (gpointer) gl_preview);
	gtk_widget_show(wdg);
  	gtk_box_pack_start (GTK_BOX (vbox), align_widget(wdg,0.5,0.5),
		FALSE, FALSE, DEF_PAD*0.5);

//	Small arrows for hiding / showing the dialog...
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox2);

	gtk_box_pack_start(GTK_BOX(vbox2),
		hideshow_dialog_new(window,
			GTK_ORIENTATION_HORIZONTAL, vbox, &gl_def->show_subdialog),
			FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), vbox, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (gl_preview->main_box), vbox2,
		FALSE, FALSE, 0);

//	gtk_widget_queue_draw(GTK_WIDGET(gl_preview->gl_area))
	gl_draw (GTK_WIDGET(gl_preview->gl_area));
	return gl_preview;
}

