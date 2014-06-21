/* render.c - height field rendering interface
 *
 * Copyright (C) 2001, 2006 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         http://geomorph.sourceforge.net
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

// get_current_dir_name() is only prototyped if _GNU_SOURCE is defined
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>
#include "render.h"
#include "../utils/utils.h"
#include "../utils/menus_n_tools.h"
#include "hf_wrapper.h"
#include "camera.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

// Toolbar for rendering commands

#include "../icons/run2.xpm"
#include "../icons/run2_with_choice.xpm"
#include "../icons/stop.xpm"

extern gchar *HF_OUTPUT_FOR_RENDERING, *OUTPUT_PREFIX, *TMP_DIR;

#define NBRENDERCOM 3

command_item_struct render_commands[NBRENDERCOM] = {
  {"Render",  "Choose & Run", "Choose a scene (.pov) and render it", 0, (gchar **) run2_with_choice_xpm, GDK_LEFT_PTR, run_with_choice_callb,NULL, NULL, FALSE},
 {"Render",  "Run", "Render the current scene (.pov)", 0, (gchar **) run2_xpm, GDK_LEFT_PTR, run_callb,NULL, NULL, FALSE},
 {"Render",  "Stop", "Stop the scene rendering", 0, (gchar **) stop_xpm, GDK_LEFT_PTR, stop_callb,NULL, NULL, FALSE}
};

#define NB_SCENE_EXT 2
omenu_list_type scene_ext_list[NB_SCENE_EXT] = {
	{NULL,0,NULL,gettext_noop("POV"),filetype_sel_callback_fn},
	{NULL,0,NULL,gettext_noop("<no extension>"),filetype_sel_callback_fn},
};

render_struct *render_struct_new () {
	return (render_struct *) x_calloc(sizeof(render_struct),1, "render_struct");
}

void init_render_struct(render_struct *rs, gchar **dirname, gchar **filename, gboolean *if_creation, gboolean *if_modified, gl_preview_struct *glp, hf_type *grid, gint max_x, gint max_y) {
//	printf("INIT_RENDER_STRUCT\n");
	rs->dirname = dirname; // HF_DIR
	rs->filename = filename; // HF name
	rs->if_creation = if_creation;
	rs->if_modified = if_modified;
	if (glp) // As it is related to a dialog, we don't nullify a valid value
		rs->gl_preview = glp;
	rs->grid = grid;
	rs->max_x = max_x;
	rs->max_y = max_y;
//	printf("INIT_RENDER_STRUCT: RS: DIRNAME: %s; FILENAME: %s, GRID (hf_buf): %p, max_x: %d, max_y: %d\n",*rs->dirname, *rs->filename, rs->grid, rs->max_x, rs->max_y); 
}

void set_render_buffer (render_struct *rs, hf_type *buffer, gint max_x, gint max_y) {
	rs->grid = buffer;
	rs->max_x = max_x;
	rs->max_y = max_y;
//	printf("Set render buffer: GRID (hf_buf): %p, max_x: %d, max_y: %d\n", rs->grid, rs->max_x, rs->max_y);
}

void set_gl_render (render_struct *rs, gl_preview_struct *glp) {
//	printf("Set GL render: %d\n",glp);
	rs->gl_preview = glp;
}

GtkWidget *create_render_bar (GtkWidget *window, GtkTooltips *tooltips, render_struct *rs) {
	GtkWidget *bar;
//	printf("HFO_PTR in CREATE_RENDER_BAR: %d - %x\n",hfo_ptr, hfo_ptr);
	bar = standard_toolbar_new(NBRENDERCOM, render_commands,
			tooltips,
			window,
			(gpointer) rs,
			GTK_ORIENTATION_HORIZONTAL,
			GTK_TOOLBAR_ICONS,
			FALSE);
	return bar;
}

void save_render_options_inc (render_struct *rs) {
//	Private method, added 2007-01-01
//	Save some render options in render_options.inc, in the current directory
//	those that are not camera related
//	and that cannot be specified on the command line
	FILE *fin;
	gchar *msg_buf, str_format[100], *ext;
	if (!(fin=fopen("render_options.inc","wb"))) {
		msg_buf = (gchar *) x_malloc(sizeof(gchar) + strlen("render_options.inc") + strlen(_("Not able to open '%s' file for writing"))+5, "const gchar (msg_buf - open file for writing)");
		sprintf(msg_buf,_("Not able to open '%s' file for writing"),"render_options.inc");
		my_msg(msg_buf,WARNING);
	}
	else {
//		loc = setlocale(LC_NUMERIC,""); // Povray uses "." as decimal separator instead of ","
//		setlocale(LC_NUMERIC,"C");
		if (*rs->if_creation || *rs->if_modified) {
			// The PNG file to render is written in a temporary directory
			fprintf(fin,"#declare hf_to_render = %c%s%c;\n",'\"',HF_OUTPUT_FOR_RENDERING,'\"');
			fprintf(fin,"#declare temporary_directory = %c%s%c;\n",'\"',TMP_DIR,'\"');
		}
		else
			// If the current height field has not been changed, we render the source file
			fprintf(fin,"#declare hf_to_render = %c%s%c;\n",'\"',*rs->filename,'\"');
		fprintf(fin,"#declare current_directory = %c%s%c;\n",'\"',*rs->dirname,'\"');
		ext = strstr(*rs->filename,".png");
		if (ext) {
			sprintf(str_format, "#declare fname_radix = %%c%%%ld.%lds%%c;\n", ext-*rs->filename,ext-*rs->filename);
//			printf("STR_FORMAT: %s\n",str_format);
			fprintf(fin,str_format,'\"',*rs->filename,'\"');
		}
//		setlocale(LC_NUMERIC,loc);
		fclose(fin);
	}
}

void render_pov(render_struct *rs) {
	// buf is the string array passed to the exec function
	// It MUST have one index more than the number of required arguments,
	// and the last index MUST be NULL
	static char *buf[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	
	save_render_options_inc (rs);
	
	// 2004-12-23 Arguments separated for compatibility with MDK10.1
	buf[0] = RENDERER;
	buf[1] = (gchar *) malloc(strlen(SCENE)+5);
	buf[2] = (gchar *) malloc(15);
	buf[3] = (gchar *) malloc(15);
	buf[4] = (gchar *) malloc(strlen(*rs->filename)+strlen(OUTPUT_PREFIX)+5);
	buf[5] = (gchar *) malloc(strlen(POV_DIR)+5);
	buf[6] = OTHER_RENDER_OPTIONS;
	sprintf(buf[1],"+I%s",SCENE);
	sprintf(buf[2],"+W%05d",RENDER_WIDTH);
	sprintf(buf[3],"+H%05d",RENDER_HEIGHT);
	sprintf(buf[4],"+O%s%s",OUTPUT_PREFIX,*rs->filename);
	sprintf(buf[5],"+L%s",POV_DIR);
	
//	printf("CURRENT_DIR: %s\n",get_current_dir_name());
//	printf("POV: %s str: %s; W: %s; H: %s; Output: %s; POV_DIR: %s; RENDER_OPTIONS: %s\n",buf[0], buf[1],buf[2],buf[3], buf[4], buf[5], buf[6]);
//	printf("HF_OUTPUT_FOR_RENDERING: %s\n",HF_OUTPUT_FOR_RENDERING);

	rs->povray_pid = execution(buf[0],buf);
	
//	printf("PID: %d\n",rs->povray_pid);
	if (buf[1]) {
		free(buf[1]);
		buf[1]=NULL;
	}
	if (buf[2]) {
		free(buf[2]);
		buf[2]=NULL;
	}
	if (buf[3]) {
		free(buf[3]);
		buf[3]=NULL;
	}
	if (buf[4]) {
		free(buf[4]);
		buf[4]=NULL;
	}
	if (buf[5]) {
		free(buf[5]);
		buf[5]=NULL;
	}
}

void run_callb(GtkWidget *wdg, gpointer render_struct_ptr) {
//	Render a povray image, display only
//	First we save the image under HF_OUTPUT_FOR_RENDERING
//	(usually test.png) in the current directory
	gchar *dir;
	gfloat ar;
	render_struct *rs = (render_struct *) render_struct_ptr;
	if (wdg)
		if (GTK_IS_TOGGLE_BUTTON(wdg))
			if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
				return;
	if (!rs)
		return;
	dir = (gchar *) get_current_dir_name();
	chdir(TMP_DIR);
	// If the current height field has not been changed, we render the source file
//	printf("if_creation: %d; if_modified: %d; (x,y): (%d,%d)\n",*rs->if_creation, *rs->if_modified, rs->max_x, rs->max_y);
	if (*rs->if_creation || *rs->if_modified)  {
//		printf("\n*************\n\nSAVING in %s/%s\n\n*************\n",TMP_DIR, HF_OUTPUT_FOR_RENDERING);
		write_png (HF_OUTPUT_FOR_RENDERING, 16, (guchar *) rs->grid, rs->max_x, rs->max_y);
	}
		
	// Patch 2005-05 - Deduce aspect ratio from image size,
	// backup the original (which is the preview W/H ratio), maybe for the future...
/*
printf("RS: %d\n",rs);
printf("RS->gl_preview: %d\n",rs->gl_preview);
printf("RS->gl_preview->cameras: %d\n",rs->gl_preview->cameras);
printf("RS->gl_preview->current_camera_id: %d\n",rs->gl_preview->current_camera_id);
*/
	ar = rs->gl_preview->cameras[rs->gl_preview->current_camera_id]->aspect_ratio;
	rs->gl_preview->cameras[rs->gl_preview->current_camera_id]->aspect_ratio = ((gfloat) RENDER_WIDTH) / (gfloat) RENDER_HEIGHT;
	
	gl_save_camera_inc (rs->gl_preview->cameras[rs->gl_preview->current_camera_id]);
	gl_save_water_inc (rs->gl_preview);
	rs->gl_preview->cameras[rs->gl_preview->current_camera_id]->aspect_ratio = ar;
	render_pov(rs);
	chdir(dir);
	free(dir);
}

void run_with_choice_callb(GtkWidget *wdg, gpointer render_struct_ptr) {
//	Render a povray image after choosing a scene file
	gchar *path_n_file, *dir;
	if (wdg)
		if (GTK_IS_TOGGLE_BUTTON(wdg))
			if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
				return;

	
	dir = (gchar *) get_current_dir_name();
	chdir(POV_DIR);

	path_n_file =
		GetFilename(	scene_ext_list,
				NB_SCENE_EXT,
				_("Open"),
				POV_DIR,
				EXISTING_FILE,
				scene_ext_list[0].lbl);
	if (path_n_file)
		SCENE = path_n_file;
	else
		return;
	POV_DIR = get_dir_name(path_n_file,FILESEP);
	// It could be dangerous to free SCENE before reinitializing it
	// It could contain a default #define value (check this... small memory leakage here).
	// However path_n_file is always "malloced" in GetFilename
	run_callb(wdg, render_struct_ptr);

	chdir(dir);
}

void stop_callb(GtkWidget *wdg, gpointer render_struct_ptr) {
	render_struct *rs = (render_struct *) render_struct_ptr;
	if (!rs)
		return;
	if (!wdg)
		return;
	if (GTK_IS_TOGGLE_BUTTON(wdg))
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;
	if (rs->povray_pid<=0)
		return;
	kill((pid_t) rs->povray_pid, SIGKILL);
	rs->povray_pid = 0;
}
