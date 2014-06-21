/* hf_wrapper.c - functions for managing the height field creation and display
 *
 * Copyright (C) 2001, 2005 Patrice St-Gelais
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

 // Modified 2005-01-09 for removing double buffering, not required with GTK2
 // Modified 2006-01-01 for cleaning the file / code structure
 
#include "globals.h"
#include "hf_wrapper.h"
#include "hf_dialog_options.h"
#include "hf_creation_dialog.h"
#include "hf_filters.h"
#include "hf_filters_dialog.h"
#include "render.h"
#include "gl_preview.h"
#include "dialog_utilities.h"
#include "../utils/scale_buttons.h"

pix8_rgb rgb_mask = { 0x10, 0x0F, 0x00};

extern gboolean INTEGRATED_INTERFACE;

gint gl_no_refresh_callb (GtkWidget *wdg, GdkEventButton *event, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return TRUE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw)
		return TRUE;
	if (!hfw->gl_preview)
		return TRUE;
	hfw->gl_preview->refresh_on_mouse_down = FALSE;
	return FALSE;
}

gint gl_restore_refresh_callb (GtkWidget *wdg, GdkEventButton *event, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return TRUE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw)
		return TRUE;
	if (!hfw->gl_preview)
		return TRUE;
	hfw->gl_preview->refresh_on_mouse_down = TRUE;
	gl_area_update(hfw->gl_preview);
	return FALSE;
}

void gl_preview_optimize (GtkWidget *scale, gpointer data) {
	// Connects a widget (typically a scale) to callbacks for decreasing
	// the density of texels when dragging the mouse
	hf_wrapper_struct *hfw;
	if (!data) return;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;	
	// Don't apply for a pen or a similar HF
	// If hfw is null, we assume we are initializing a dialog, and it's a HFMAIN
	if (hfw && (hfw->type != HFMAIN)) 
		return;
	gtk_signal_connect (GTK_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) gl_no_refresh_callb, data);
	gtk_signal_connect (GTK_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) gl_restore_refresh_callb, data);
}

gint hf_default_size () {
	gint s;
	if (HF_SIZE_IN_PIXELS)
//	"Floor" the given HF_SIZE to the nearest power of 2
		return pow(2,log2i(HF_SIZE_IN_PIXELS));
	s = log2i(HF_SIZE_IN_SCREEN_PERCENT*gdk_screen_width()/100);
	s = pow(2,s);
	return s;
}

gint get_display_scale (gint hf_size) {
//	Guess display scale from HF size and current display size...
//	or take the value given in the RC file
	static gint def_size = 0;
	if (DOC_DISPLAY_SIZE) {
		if (hf_size>DOC_DISPLAY_SIZE)
			return log2i(hf_size / DOC_DISPLAY_SIZE);
		else 
			return 0;
	}
	else {
		if (!def_size)
			def_size =  hf_default_size();
		if (hf_size > def_size)
			return log2i(hf_size / def_size);
		else
			return 0;
	}
}
void display_16_to_8 (hf_wrapper_struct *hfw)
{
	// Transforms 16 bits HF to 8 bits hf, for displaying
	// or saving in 8 bits formats
	gint i, j, max_x, max_y, factor, index, scaled_index;
	hf_struct_type *hf = (hf_struct_type *) hfw->hf_struct;
	max_x = RIGHT_SHIFT(hf->max_x,hfw->display_scale);
	max_y = RIGHT_SHIFT(hf->max_y,hfw->display_scale);
	if (hfw->if_rgb && hf->select_buf)
		factor = 3;
	else {
		factor = 1;
		hfw->if_rgb = FALSE;
	}
//	We reallocate the memory in case of a size change
	// 2008-08: why is it not scaled? something to check here!
	realloc_buf8(hfw);

	if (hfw->display_scale)
		for (i = 0; i < max_x; i++)
			for (j = 0; j < max_y; j++) {
				index = j*max_y + i;
				scaled_index = (LEFT_SHIFT(i,hfw->display_scale)) + (LEFT_SHIFT(j,hfw->display_scale))*hf->max_x;
				if (factor==1)
					*(hfw->hf_buf8 + index) = *(hf->hf_buf + scaled_index) >> 8;
				else {
					*(hfw->hf_buf8 + 3*index) = *(hfw->hf_buf8 + 3*index + 1) = *(hfw->hf_buf8 + 3*index + 2) = *(hf->hf_buf + scaled_index) >> 8;

					if (*(hf->select_buf + scaled_index) > MIN_FILLING_VALUE) {

						*(hfw->hf_buf8 + index*3)  = rgb_mask.r + *(hfw->hf_buf8 + index*3);
						*(hfw->hf_buf8 + index*3+1) = rgb_mask.g + *(hfw->hf_buf8 + index*3+1);
						*(hfw->hf_buf8 + index*3+2) = rgb_mask.b + *(hfw->hf_buf8 + index*3+2);
					}
				}
			}
	else
		for (i = 0; i<hf->max_x*hf->max_y; i++) {
			if (factor==1)
				*(hfw->hf_buf8 + i) = *(hf->hf_buf + i) >> 8;
			else { // R-G-B	
				*(hfw->hf_buf8 + 3*i) = *(hfw->hf_buf8 + 3*i + 1) = *(hfw->hf_buf8 + 3*i + 2) = *(hf->hf_buf + i) >> 8;
				if (*(hf->select_buf + i) > MIN_FILLING_VALUE) {
					*(hfw->hf_buf8 + i*3)  = rgb_mask.r + *(hfw->hf_buf8 + i*3);
					*(hfw->hf_buf8 + i*3+1) = rgb_mask.g + *(hfw->hf_buf8 + i*3+1);
					*(hfw->hf_buf8 + i*3+2) = rgb_mask.b + *(hfw->hf_buf8 + i*3+2);
				}
			}
		}
}

void display_16_to_8_partial (hf_wrapper_struct *hfw, gint from_x, gint to_x, gint from_y, gint to_y) {
	// Same as display_16_to_8, but only for a subwindow
	// We don't reallocate the buffer, but allocate it if NULL
	// Only the beginning of the buffer is used
	// Suitable for drawing only (more efficient)
	// from_x, from_y, to_x, to_y are in display world, not in hf world
	gint i, j, delta_x, delta_y, max_x, max_y, factor, index, scaled_index;
	hf_struct_type * hf = (hf_struct_type *) hfw->hf_struct;
	max_x = RIGHT_SHIFT(hf->max_x,hfw->display_scale);
	max_y = RIGHT_SHIFT(hf->max_y,hfw->display_scale);

	if (hfw->if_rgb && hf->select_buf)
		factor = 3;
	else {
		factor = 1;
		hfw->if_rgb = FALSE;
	}
	if (!hfw->hf_buf8) {
		alloc_buf8(hfw);
	}
	if ( (!(to_x-from_x)) || (!(to_y-from_y)))
		return;
	// Be sure that from_x <= to_x, and from_y <= to_y
	if (from_x > to_x) {
		i = from_x;
		from_x = to_x;
		to_x = i;
	}
	if (from_y > to_y) {
		i = from_y;
		from_y = to_y;
		to_y = i;
	}
	to_x = MIN(to_x,max_x);
	to_y = MIN(to_y,max_y);
	delta_x = to_x - from_x;
	delta_y = to_y - from_y;
//	printf("DELTA_X: %d; DELTA_Y: %d\n",delta_x, delta_y);
	for (i = 0 ; i < delta_x; i++)
		for (j = 0; j < delta_y; j++) {
			index = j*delta_x + i;
			if (hfw->display_scale)
				scaled_index = (LEFT_SHIFT(i+from_x,hfw->display_scale)) + (LEFT_SHIFT(j+from_y,hfw->display_scale))*hf->max_x;
			else
				scaled_index = (j+from_y)*hf->max_x + i + from_x;	
			if (factor==1)
				*(hfw->hf_buf8 + index) = *(hf->hf_buf + scaled_index) >> 8;
			else {
				*(hfw->hf_buf8 + 3*index) = *(hf->hf_buf + scaled_index) >> 8;
				*(hfw->hf_buf8 + 3*index + 1) = *(hf->hf_buf + scaled_index) >> 8;
				if (*(hf->select_buf + scaled_index)> MIN_FILLING_VALUE)
				// We keep 75% of the Blue channel
				// This gives a light yellow
					*(hfw->hf_buf8 + index*3 + 2) = (*(hf->hf_buf + scaled_index) >> 9) + (*(hf->hf_buf + scaled_index) >> 10);
				else
					*(hfw->hf_buf8 + index*3 + 2) = *(hf->hf_buf + scaled_index) >> 8;
			}
		}

}
/*
//	TEST:  trying to boost higher frequencies (1, 2, 4 and 8 pixels period)
//	UPDATE 2003-03:  should try equalization (kind of)
void display_16_to_8(hf_struct_type *hf)
{
	// Transforms 16 bits HF to 8 bits hf, for displaying
	// or saving in 8 bits formats
	gint i, j,max_x, max_y;
	gfloat val,minval,divider;
	minval = 256.0*BLACK_THRESHOLD;
	divider = 256.0+BLACK_THRESHOLD;
	max_x = RIGHT_SHIFT(hf->max_x,hf->display_scale);
	max_y = RIGHT_SHIFT(hf->max_y,hf->display_scale);
//	We reallocate the memory in case of a size change
	hf->hf_buf8 = (unsigned char *)  x_realloc (hf->hf_buf8, max_x * max_y * sizeof(unsigned char), "unsigned char (hf_buf8 in display_16_to_8)");

// printf("HF->DISPLAY_SCALE in HF_16_TO_8: %d; max_x: %d; max_y: %d \n",hf->display_scale, max_x, max_y);
	if (hf->display_scale)
		for (i = 0; i < max_x; i++)
			for (j = 0; j < max_y; j++) {
				val = 	((gfloat)*(hf->hf_buf + (LEFT_SHIFT(i,hf->display_scale)) +
					(LEFT_SHIFT(j,hf->display_scale))*hf->max_x) + minval )/divider;
				*(hf->hf_buf8 + j*max_y + i) = (unsigned char) val;
// printf("j*max_y+i:  %d = %d\n", j*max_y+i, RIGHT_SHIFT(i,hf->display_scale) + RIGHT_SHIFT(j,hf->display_scale)*hf->max_x);
			}
	else
		for (i = 0; i<hf->max_x*hf->max_y; i++) {
			val = (((gfloat) *(hf->hf_buf + i)) + minval )/ divider;
			*(hf->hf_buf8 + i) = (unsigned char) val;
		}
}
*/
void draw_hf (hf_wrapper_struct *hfw) {
	hf_struct_type *hf;
	hf = hfw->hf_struct;
//	if (hfw->type==HFMAIN)
//		printf("DRAW_HF: hfw: %p -> hf: %p -> hf_buf: %p; creation_mode: %d; if_modified: %d\n",hfw,hf, hf->hf_buf, hfw->creation_mode, *hfw->if_modified);
	if (hfw->apply_filter)
	    if (hfw->hf_options->fd_struct) {
		if (hfw->hf_options->fd_struct->current_filter) {
			if (hfw->hf_struct->max_x > hfw->hf_options->dist_matrix->hf_size)
				dist_matrix_init(hfw->hf_options->dist_matrix, hfw->hf_struct->max_x);

			filter_apply(hfw->hf_struct,
				hfw->hf_options->fd_struct->current_filter,
				hfw->hf_options->dist_matrix,
				hfw->hf_options->fd_struct->revert_filter,
				hfw->hf_options->fd_struct->filter_level,
				hfw->hf_options->fd_struct->merge_oper);
	    	}
	}
// 	printf ("hf->max_x: %d; hf->display_scale: %d; SHIFT: %d\n",hf->max_x,hf->display_scale, RIGHT_SHIFT(hf->max_x,hf->display_scale));
	// "gtk_widget_queue_draw_area" triggers an expose_event for hf_wrapper->area:
	gtk_widget_queue_draw_area(hfw->area, 0, 0, RIGHT_SHIFT(hf->max_x,hfw->display_scale), RIGHT_SHIFT(hf->max_y,hfw->display_scale) );
		
	// Redraw immediately, otherwise using the pen is too annoying (irregular)
	if (hfw->area->window)
		gdk_window_process_updates(hfw->area->window, FALSE);
	if (hfw->gl_preview) {
		gl_set_input_grid (hfw->gl_preview, hfw->hf_struct->hf_buf, hfw->hf_struct->max_x, hfw->hf_struct->max_y, HF_TYPE_ID);
// 	printf("GL_AREA_UPDATE de %d:  %s\n", hfw, *hfw->filename);
		gl_area_update(hfw->gl_preview);
	}
	
	// Emit expose event required for size change during creation
	if (hfw->creation_mode) {
		emit_expose_event(hfw->area);
	}
}

void draw_hf_ptr (gpointer hfw_ptr_ptr) {
	hf_wrapper_struct *hfw;
//	printf("Draw_hf_ptr 1\n");
	if (hfw_ptr_ptr) {
		hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) hfw_ptr_ptr;
		if (hfw) {
			
			draw_hf (hfw);
//			printf("Draw_hf_ptr 2; hf->tmp_buf: %p; hf->result_buf: %p, hf->hf_buf: %p\n", hfw->hf_struct->tmp_buf, hfw->hf_struct->result_buf,hfw->hf_struct->hf_buf);
		}
	}
}

void draw_hf_partial (hf_wrapper_struct *hfw, gint fromx, gint tox, gint fromy, gint toy,
		gboolean update_gl_area) {
	//	Version of draw_hf drawing in a subset of the area
	//	from_x, to_x, from_y, to_y defined in the display world, not in the hf world
	hf_struct_type *hf;
	hf = hfw->hf_struct;

//	printf("DRAW_HF partial: (%d, %d) - (%d, %d) \n", fromx, fromy, tox, toy);
	
	gtk_widget_queue_draw_area(hfw->area, fromx, fromy, tox - fromx, toy - fromy);
	// Redraw immediately, otherwise using the pen is too annoying (irregular)
	if (hfw->area->window)
		gdk_window_process_updates(hfw->area->window, FALSE);
	if (update_gl_area && hfw->gl_preview) {
		gl_set_input_grid (hfw->gl_preview, hfw->hf_struct->hf_buf, hfw->hf_struct->max_x, hfw->hf_struct->max_y, HF_TYPE_ID);
// 	printf("GL_AREA_UPDATE de %d:  %s\n", hfw, *hfw->filename);
		gl_area_update(hfw->gl_preview);
	}
}

void gener_hf_from_ptr_ptr (gpointer hfw_ptr_ptr) {
	if (hfw_ptr_ptr)
		if (* (hf_wrapper_struct **) hfw_ptr_ptr)
			gener_hf(* (hf_wrapper_struct **)hfw_ptr_ptr);
}

void gener_hf (hf_wrapper_struct *hfw) {

	if (!hfw)
		return;
	if (hfw->creation_mode)
		if (hfw->hf_options->current_calculation) {
			(*hfw->hf_options->current_calculation) (hfw->hf_struct, hfw->hf_options->current_options);
//	if (hfw->type==HFMAIN)
//		printf("Calculation on %d == %x in GENER_HF\n",hfw, hfw);
		}
	if (hfw->dependent_process)
		if (hfw->dependent_process_data)
			(*hfw->dependent_process) (hfw->dependent_process_data);
	draw_hf(hfw);
}

static gint configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	hf_wrapper_struct *hf_wrapper;
	hf_wrapper = (hf_wrapper_struct *) data;

//	if (hf_wrapper->type==HFMAIN)
//		printf("CONFIGURE_EVENT for HFW == %d; Creation mode: %d\n", hf_wrapper, hf_wrapper->creation_mode);

//	We initialize the drawable:

	if (GTK_IS_DRAWING_AREA(hf_wrapper->area)) 
		gdk_draw_rectangle(hf_wrapper->area->window,
			hf_wrapper->area->style->black_gc,TRUE, 0, 0,
			hf_wrapper->area->allocation.width,
			hf_wrapper->area->allocation.height);
	else
		return TRUE;

	
	if (hf_wrapper->creation_mode) {
		gener_hf(hf_wrapper);
	}
	/*
	else  {		// Probably an area size change - removed 2005-11-09
		draw_hf(hf_wrapper);
	}
	*/
	hf_wrapper->size_change_request = FALSE;

    return TRUE;
}

gint expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{

	gint w,h;
	hf_wrapper_struct *hfw;

	hfw = (hf_wrapper_struct *) data;
	
  	if (event->count > 0) {
    		return TRUE;
  	}

	if (hfw->size_change_request)
		return TRUE;		// A configure event must be done before

//	printf("EXPOSE_EVENT: X: %d; Y: %d; W: %d;  H: %d;  hfw: %d; area size: %d\n ",event->area.x, event->area.y, event->area.width, event->area.height, hfw, hfw->area_size);
// 	printf("FILE: %s; AREA SIZE: %d\n",*hfw->filename,hfw->area_size);

	if ((!event->area.width) && (!event->area.height))
		return TRUE;
	if (GTK_IS_DRAWING_AREA(hfw->area)) {
	
//	We decide here what part of the HF we refresh, before drawing

		if ( (!event->area.width) || (!event->area.height))
			return TRUE;
		if ( ((event->area.width>=hfw->area_size) &&
			(event->area.height>=hfw->area_size) &&
			(event->area.x==0) && (event->area.y==0) ) ) {
//			(event->area.x==0) && (event->area.y==0) ) || (hfw->hf_struct->if_rgb) ) {
			// Full refresh
			// Patched 2005-10-28 - partial refresh not working with rgb images
			// We fallback to full redraw
			// Unpatched 2005-10-29
//			printf("FULL REFRESH\n");
			display_16_to_8(hfw);
			if (hfw->if_rgb) 
				gdk_draw_rgb_image(hfw->area->window,
					hfw->area->style->fg_gc[GTK_WIDGET_STATE (hfw->area)],
					0,0,hfw->area_size, hfw->area_size,
					GDK_RGB_DITHER_NONE,
					hfw->hf_buf8,
					3*hfw->area_size);
			else
				gdk_draw_gray_image(hfw->area->window,
					hfw->area->style->fg_gc[GTK_WIDGET_STATE (hfw->area)],
					0,0,hfw->area_size, hfw->area_size,
					GDK_RGB_DITHER_NONE,
					hfw->hf_buf8,
					hfw->area_size);
		}
		else {
			w = MIN(event->area.x+event->area.width,hfw->area_size) - event->area.x;
			h = MIN(event->area.y+event->area.height,hfw->area_size) - event->area.y ;
			display_16_to_8_partial (hfw,
				event->area.x, event->area.x+w,
				event->area.y,event->area.y + h);

//			printf("PARTIAL REFRESH from (%d,%d) to (%d,%d)\n",event->area.x, event->area.y,event->area.x+w,event->area.y + h);
			
			if (hfw->if_rgb) 
				gdk_draw_rgb_image(hfw->area->window,
					hfw->area->style->fg_gc[GTK_WIDGET_STATE (hfw->area)],
					event->area.x,
					event->area.y,
					w,
					h,
					GDK_RGB_DITHER_NONE,
					hfw->hf_buf8,
					3*w );
			else
				gdk_draw_gray_image(hfw->area->window,
					hfw->area->style->fg_gc[GTK_WIDGET_STATE (hfw->area)],
					event->area.x,
					event->area.y,
					w,
					h,
					GDK_RGB_DITHER_NONE,
					hfw->hf_buf8,
					w );

		}

		control_line_draw (hfw->control_line, hfw->area->window,
			hfw->area_size, hfw->area_size);

		gdk_window_set_cursor(widget->window, hfw->hf_options->current_cursor);
	}
	// GL refresh doesn't seem to be required here (?)
	// if (hfw->gl_preview)
	//	gtk_widget_queue_draw(GTK_WIDGET(hfw->gl_preview->gl_area));
	//    return FALSE;
    return TRUE;
}

void hf_wrapper_display(gpointer data) {
//	Redraws the HF after resizing the drawing area (typically:  for undo / redo)
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) data;
//	This does not always automatically redraw the HF at the desired scale?
//	toggle_scale_button(hfw->sbs);
//	Accepting transformations like brightness / contrast - avoiding unseen inconsistencies!
//	printf("HF_WRAPPER_DISPLAY: %s .. %p;\n", *hfw->filename, hfw);
	if (hfw->hf_options->need_tmp_buf) {
		accept_callb(NULL,&data);
	}
//	Be sure that we put the HF in a state allowing a new drawing
	hfw->hf_options->flag_uniform_done = FALSE;
	if (hfw->tiling_ptr)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hfw->tiling_wdg), *hfw->tiling_ptr);

	init_render_struct(hfw->hf_options->render_str, 
		hfw->dirname, 
		hfw->filename, 
		&hfw->creation_mode,
		hfw->if_modified,
		hfw->gl_preview,
		hfw->hf_struct->hf_buf,
		hfw->hf_struct->max_x,
		hfw->hf_struct->max_y);

//	The HF could be displayed 2 times ?
	draw_hf(hfw);
	// GTK 2.x: we must repeat this to display the preview when activating a new doc window 
	if (hfw->gl_preview) {
 //	printf("GL_AREA_UPDATE de %d:  %s\n", hfw, *hfw->filename);
		gl_area_update(hfw->gl_preview);
	}
}


gpointer hf_wrapper_copy(gpointer data_from, gpointer data_to, gint operation) {
//	Copy of a hf_wrapper, including the embedded hf_struct,
//	for stacking in the history (undo / redo stack) or replacing a structure with another
//	If data_to is null, we allocate it and return the newly allocated pointer
	hf_wrapper_struct *hfw, *hfwcopy;
	gint old_scale;
	gboolean flag_swap;
	hfw = (hf_wrapper_struct *)data_from;
	if ((operation==NONE) || (operation==UNDO))
		flag_swap = TRUE;
	else // operation==REDO
		flag_swap = FALSE;
	if (!data_to) {
		hfwcopy = (hf_wrapper_struct *) x_malloc(sizeof(hf_wrapper_struct), "hf_wrapper_struct (hfwcopy)");
//		printf("data_to==NULL; HF_WRAPPER_COPY, de %p sur MALLOC: %p\n",hfw, hfwcopy);
	}
	else {
		hfwcopy = data_to;
		hf_free(hfwcopy->hf_struct);
		old_scale = hfwcopy->display_scale;
//		printf("data_to!=NULL; HF_WRAPPER_COPY, de %p sur %p\n",hfw, hfwcopy);
	}
	memcpy(hfwcopy, hfw, sizeof(hf_wrapper_struct));
	hfwcopy->hf_struct = hf_copy_new(hfw->hf_struct,flag_swap);
//	The copy would certainly be reused in edit mode, not creation mode
	hfwcopy->creation_mode = FALSE;
//	hfwcopy->hf_buf8 = NULL;
	if (data_to) { // We are probably restoring (undoing)
		hfwcopy->display_scale = old_scale;
		alloc_buf8(hfwcopy);
		hfwcopy->area_size = RIGHT_SHIFT(hfwcopy->hf_options->hf_size,hfwcopy->display_scale) ;
	}
	return (gpointer) hfwcopy;
}

gint hf_set_display_scale(GtkWidget *wdg, gpointer data) {
//	Applied after getting the scale setting from the clicked button
//	(non generic part of the process)
	gchar *txt;
	GtkAdjustment *vadj,*hadj;
	GtkRequisition rq;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return TRUE;
	
	if (!GTK_IS_WIDGET(hfw->draw_area_frame))
		return TRUE;
//	printf("HF_SET_DISPLAY_SCALE\n");
	hfw->display_scale = get_scale(hfw->sbs);
	hfw->area_size = RIGHT_SHIFT(hfw->hf_struct->max_x, hfw->display_scale);
//	Area size changes trigger a callback which reallocates and computes the buffers
//	2005-03: now that the draw area is in a scrolled window, we must reallocate
//	hf_buf8, because it could be only partially exposed, and the function
//	which writes into hf_buf8 doesn't reallocate it, for performance considerations
//	(this function was designed for use when drawing with a pen).

	realloc_buf8(hfw);

	gtk_widget_set_size_request (hfw->area, hfw->area_size, hfw->area_size);
	gtk_widget_set_size_request (hfw->draw_area_viewport, hfw->area_size, hfw->area_size);

	if (hfw->area_size<=hfw->hf_struct->max_x)
		return TRUE;

	// The image is centered in the scrolled window
	gtk_widget_size_request(GTK_WIDGET(hfw->draw_area_frame),&rq);
//	printf("WIDTH: %d; HEIGHT: %d\n",rq.width, rq.height);
	vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(hfw->scrolled_window));
	hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(hfw->scrolled_window));
	gtk_adjustment_set_value (vadj, (gdouble) ((hfw->area_size>>1) - (rq.height>>1)));
	gtk_adjustment_set_value (hadj, (gdouble) ((hfw->area_size>>1) - (rq.width>>1)));

	return TRUE;
}
 
void create_standard_hf_display (hf_wrapper_struct *hf_wrapper) {

	GtkWidget *vbox, *hbox, *frame, *label, *scrolled_window;
	GSList *group = NULL;
	GtkAdjustment *vadj,*hadj;
	gchar *txt;

//	printf("CREATE_STANDARD_HF_DISPLAY\n");
//	Standard HF
	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox));
	gtk_container_add (GTK_CONTAINER(hf_wrapper->window), vbox);
	
//	hbox for scale buttons and mouse position

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, DEF_PAD*0.5);

//	frame for mouse position
	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE, FALSE, DEF_PAD*0.5);
	hf_wrapper->mouse_position = gtk_label_new("   0,0   ");
	gtk_widget_show(hf_wrapper->mouse_position);
	gtk_widget_set_size_request (hf_wrapper->mouse_position, 120, 0);
	gtk_container_add(GTK_CONTAINER(frame),hf_wrapper->mouse_position);
	
	if (hf_wrapper->tiling_ptr) {
		hf_wrapper->tiling_wdg = define_check_button_in_box ("Tiling", hbox, FALSE, FALSE, DEF_PAD);
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON ( hf_wrapper->tiling_wdg ),
			*hf_wrapper->tiling_ptr);
		gtk_signal_connect (GTK_OBJECT(hf_wrapper->tiling_wdg), "toggled",
			GTK_SIGNAL_FUNC(toggle_check_button_callb), hf_wrapper->tiling_ptr);
	}

	hf_wrapper->sbs = scale_buttons_new_with_callback (GTK_ORIENTATION_HORIZONTAL, SCALE_ID_12_5, SCALE_ID_400, hf_set_display_scale, (gpointer) hf_wrapper);
	hf_wrapper->sbs->current = hf_wrapper->display_scale;

	gtk_box_pack_start(GTK_BOX(hbox), hf_wrapper->sbs->box, FALSE, FALSE, DEF_PAD*0.5);

	txt = (gchar *) x_malloc(12, "const gchar (txt in create_standard_hf_display)");
	sprintf(txt,"%d x %d",hf_wrapper->hf_options->hf_size, hf_wrapper->hf_options->hf_size);
	hf_wrapper->draw_area_frame = gtk_frame_new(txt);
	gtk_widget_show(hf_wrapper->draw_area_frame);
	gtk_widget_set_size_request (hf_wrapper->draw_area_frame, hf_wrapper->area_size+SCALE_PAD, hf_wrapper->area_size+SCALE_PAD);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (scrolled_window);
	
	vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(scrolled_window));
	hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW(scrolled_window));

	// We process the viewport separately, instead of using
	// gtk_scrolled_window_add_with_viewport
	// to be able to rescale the area when its size changes
	hf_wrapper->draw_area_viewport = gtk_viewport_new (hadj, vadj);
	gtk_widget_show(hf_wrapper->draw_area_viewport);
	gtk_widget_set_size_request (hf_wrapper->draw_area_viewport, hf_wrapper->area_size, hf_wrapper->area_size);

	gtk_container_add (GTK_CONTAINER(hf_wrapper->draw_area_viewport), hf_wrapper->area);
	gtk_container_add (GTK_CONTAINER(scrolled_window), hf_wrapper->draw_area_viewport);

	hf_wrapper->scrolled_window = scrolled_window;

	gtk_container_add (GTK_CONTAINER(hf_wrapper->draw_area_frame), scrolled_window);

//	hbox for main view and GL preview
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	gtk_box_pack_start(GTK_BOX(hbox),  hf_wrapper->draw_area_frame, TRUE, TRUE,0);

	hf_wrapper->gl_preview =
		create_gl_preview_dialog(
			hf_wrapper->hf_options->gl_defaults, 
			hf_wrapper->window, 
			hf_wrapper->hf_struct->hf_buf, 
			hf_wrapper->hf_struct->max_x, 
			hf_wrapper->hf_struct->max_y, 
			HF_TYPE_ID, 
			&hf_wrapper->hf_struct->use_water, 
			&hf_wrapper->hf_struct->water_level);
	if (hf_wrapper->gl_preview && hf_wrapper->gl_preview->gl_dialog) {
		gl_set_water_level (hf_wrapper->gl_preview);
		gtk_box_pack_start(GTK_BOX(hbox),
			align_widget(hf_wrapper->gl_preview->gl_dialog,0.5,0.5),
			FALSE, FALSE, DEF_PAD*0.5);
	}
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, DEF_PAD*0.5);

	gtk_signal_connect (GTK_OBJECT (hf_wrapper->area), "motion_notify_event",
		(GtkSignalFunc) motion_notify, hf_wrapper);
// printf("CONNECT hfw->area in HF_WRAPPER_NEW\n");
	gtk_signal_connect (GTK_OBJECT (hf_wrapper->area), "button_press_event",
		(GtkSignalFunc) button_press, hf_wrapper);
	gtk_signal_connect (GTK_OBJECT (hf_wrapper->area), "button_release_event",
		(GtkSignalFunc) button_release, hf_wrapper);

	toggle_scale_button(hf_wrapper->sbs);
	
	init_render_struct(hf_wrapper->hf_options->render_str, 
		hf_wrapper->dirname, 
		hf_wrapper->filename, 
		&hf_wrapper->creation_mode,
		hf_wrapper->if_modified,
		hf_wrapper->gl_preview,
		hf_wrapper->hf_struct->hf_buf,
		hf_wrapper->hf_struct->max_x,
		hf_wrapper->hf_struct->max_y);

	if (hf_wrapper->gl_preview && hf_wrapper->gl_preview->gl_area)
		gtk_widget_realize(hf_wrapper->gl_preview->gl_area);

	return;

}

void set_dependent_process (hf_wrapper_struct *hfw, void (*dependent_process) (gpointer), gpointer dependent_process_data) {
	hfw->dependent_process = dependent_process;
	hfw->dependent_process_data = dependent_process_data;
}

gpointer hf_wrapper_new (
		gchar **dirname,
		gchar **filename,
		GtkWidget *window,
		GtkWidget *creation_container, 
		gpointer hf_options, 
		gboolean *if_modified, 
		gboolean*flag_history,
		gboolean (*registering_fn) (gpointer),
		gpointer registering_data,
		gchar *path_n_file,
		gboolean *fname_tochoose,
		gint hf_category) {
	// Builds the main HF window and the HF struct
	// "window" is the document window
	//	NOTE:  for a HFW which is a pen, "window" is a box where to put the preview pixmap
	// "creation_container" is the creation dialog - allows adding some options
	//	For a HF pen, it is the HF draw dialog
	// hf_options is inherited from doc_type_struct
	// path_n_file, if not NULL, is the file name from which we try to read the file
	// fname_tochoose:  set to TRUE if we have to do a conversion when reading the file,
	//	because the file would be opened under a generic name (DOCUMENTn)

	gint hf_size;
	gchar *msg=NULL;
	hf_wrapper_struct *hf_wrapper;

	hf_wrapper = (hf_wrapper_struct *) x_malloc(sizeof(hf_wrapper_struct), "hf_wrapper_struct");
// printf("hf_wrapper in hf_wrapper_new: %p\n",hf_wrapper);
	hf_wrapper->filename = filename;
	hf_wrapper->dirname = dirname;
	hf_wrapper->window = window;
	hf_wrapper->to_register_in_history = flag_history;  // For undo / redo stack
	hf_wrapper->registering_fn = registering_fn;
	hf_wrapper->registering_data = registering_data;
	hf_wrapper->if_calculated = FALSE;
	hf_wrapper->creation_container = creation_container;
	hf_wrapper->hf_options = (hf_options_struct *)hf_options;
	hf_wrapper->tiling_wdg = NULL;
	hf_wrapper->tiling_ptr = (gboolean *) x_malloc(sizeof(gboolean), "hf_wrapper->tiling_ptr");
	*hf_wrapper->tiling_ptr = TRUE;
	hf_wrapper->mouse_position = NULL;
	hf_wrapper->size_change_request = TRUE;
	hf_wrapper->control_line = NULL;
	hf_wrapper->sbs = NULL;
	hf_wrapper->draw_area_frame = NULL;
	hf_wrapper->draw_area_viewport = NULL;
	hf_wrapper->dependent_process = NULL;
	hf_wrapper->dependent_process_data = NULL;
	if (hf_category)
		hf_wrapper->type = hf_category; // HFMAIN, HFPEN, HFPRIMITIVE, HFNOISE
	else
		hf_wrapper->type = HFMAIN;
	//	Initialize the current filter dialog struct to the creation filter struct
	hf_wrapper->hf_options->fd_struct =
		hf_wrapper->hf_options->creation_fd_struct;
	hf_wrapper->apply_filter = FALSE;

	if (!hf_wrapper->hf_options->hf_size) {
		switch (hf_wrapper->type) {
		case HFMAIN: 
		//	The pixmap area is best-guessed from the X screen size
			hf_size = hf_default_size();
			break;
		case HFPEN:
			hf_size = MAX_PEN_PREVIEW;
			break;
		default:
			//	It's any other preview
			hf_size = MAX_FILTER_PREVIEW;
		}
		hf_wrapper->hf_options->hf_size = hf_size;
	}
	else
		hf_size = hf_wrapper->hf_options->hf_size;

	if (!path_n_file) {
	//	We are creating a new document from scratch
		// We reset the size menu for consistency (only required with
		// non integrated interface)
		if (!INTEGRATED_INTERFACE)
			set_size_menu_from_size ((gpointer) hf_wrapper->hf_options, hf_size);
		hf_wrapper->hf_struct = hf_new(hf_size);
		hf_set_tiling_ptr (hf_wrapper->hf_struct, hf_wrapper->tiling_ptr);
	//	hf_wrapper->last_creation_action initialized in hf_dialog_defaults
	//	(when toggling the default button)

		hf_wrapper->last_action = _("New document");
		hf_wrapper->creation_mode = TRUE;
	}
	else {
	//	We are reading an existing document
		hf_wrapper->hf_struct = hf_read(path_n_file, fname_tochoose, &msg, hf_wrapper->tiling_ptr);

		if (*fname_tochoose)
			(*if_modified) = TRUE;

		if (msg)
			my_msg(msg, WARNING);
		if (!hf_wrapper->hf_struct) {	// Something had gone bad! (file unreadable?)
			x_free(hf_wrapper);
			return NULL;
		}
		else {
			hf_wrapper->hf_options->hf_size = hf_wrapper->hf_struct->max_x;
			hf_size = hf_wrapper->hf_options->hf_size ;
		}
		hf_backup(hf_wrapper->hf_struct);
		hf_wrapper->hf_options->last_creation_action = _("Reading");
		hf_wrapper->last_action = _("Reading");
		hf_wrapper->creation_mode = FALSE;
		if (hf_wrapper->hf_options->img)
			hf_wrapper->hf_options->fd_struct = hf_wrapper->hf_options->img->fd_struct;
		else
			hf_wrapper->hf_options->fd_struct = NULL;
	}

	hf_wrapper->display_scale = get_display_scale(hf_size);
	hf_wrapper->if_rgb = FALSE;
	alloc_buf8(hf_wrapper);
	hf_wrapper->area = gtk_drawing_area_new();
	//	For mouse movements
	gtk_widget_set_events (hf_wrapper->area, GDK_EXPOSURE_MASK
				| GDK_LEAVE_NOTIFY_MASK
				| GDK_BUTTON_PRESS_MASK
				| GDK_POINTER_MOTION_MASK
				| GDK_POINTER_MOTION_HINT_MASK
				| GDK_BUTTON_RELEASE_MASK);

	hf_wrapper->area_size = RIGHT_SHIFT(hf_size,hf_wrapper->display_scale) ;
	hf_wrapper->gl_preview = NULL;

	hf_wrapper->mouse_dragging = FALSE;

	//	The next &flag is inherited from the document wrapper in the calling app
	hf_wrapper->if_modified = if_modified;

	if (hf_wrapper->type==HFMAIN) {
 //		printf("%d == HF_MAIN\n",hf_wrapper);
		create_standard_hf_display (hf_wrapper);
	}
	else {
	//	This HF is a pen or another preview
	//	"window" is a box or frame where to put the preview pixmap
	//	We don't track the mouse movements

		gtk_container_add(GTK_CONTAINER(window),
			align_widget(hf_wrapper->area,0.5,0.5));
 //		printf("%d == HF_PEN or HF_PRIMITIVE\n",hf_wrapper);
	}

	// 2006-09-27 gtk_drawing_area_size is supposed to be deprecated
	// Unfortunately, gtk_widget_set_size_request make the H and V scales
	// disappear, when the area size is larger than the window (??)
	gtk_drawing_area_size( GTK_DRAWING_AREA(hf_wrapper->area),
		hf_wrapper->area_size ,hf_wrapper->area_size );
//	gtk_widget_set_size_request( GTK_WIDGET(hf_wrapper->area),
//		hf_wrapper->area_size ,hf_wrapper->area_size );
	gtk_widget_show(hf_wrapper->area);

	hf_wrapper->exp_handler =
		gtk_signal_connect(GTK_OBJECT(hf_wrapper->area), "expose_event", (GtkSignalFunc) expose_event, (gpointer) hf_wrapper);
// printf("**** HANDLER de %d: %d\n",hf_wrapper,hf_wrapper->exp_handler);
	gtk_signal_connect(GTK_OBJECT(hf_wrapper->area), "configure_event",
		(GtkSignalFunc) configure_event, (gpointer) hf_wrapper);

	return (gpointer) hf_wrapper;
}

void hf_wrapper_free (gpointer data, gboolean free_interface) {

	hf_wrapper_struct *hfw;
	if (!data) return;
	hfw = (hf_wrapper_struct *) data;
//	printf("HF_WRAPPER_FREE: %p -- H: %d; size: %d; tiling_ptr: %p\n", hfw, hfw->exp_handler, hfw->hf_struct->max_x, hfw->tiling_ptr);
	if (free_interface) { // FALSE if we are freeing an instance in the undo stack
//		printf("Disconnect exposure signal\n");
		gtk_signal_disconnect(GTK_OBJECT(hfw->area), hfw->exp_handler);
		if (hfw->sbs)
			scale_buttons_free(hfw->sbs);
		if (hfw->tiling_ptr)
			x_free (hfw->tiling_ptr);
	}

	if (hfw->hf_struct)
		hf_free(hfw->hf_struct);

	x_free(hfw);
}

gchar *hf_get_last_action (gpointer data) {

	return ((hf_wrapper_struct *)data)->last_action;
}

static void free_buf (guchar *pixels, gpointer data) {
	// **buf == *pixels
	guchar **buf;
	buf = (guchar **) data;
	if (*buf) {
		x_free(*buf);
		*buf = NULL;
	}
}

GtkWidget *hf_get_icon (gpointer hf_wrapper, gint size_x, gint size_y) {
	// Returns a small icon, representing the document
	hf_wrapper_struct *hfw;
	GtkWidget *area;
	GdkPixbuf *pixbuf;
	guchar *buf;
	gint i,j, len_x;
	gfloat ratio_x, ratio_y;

	hfw = (hf_wrapper_struct * ) hf_wrapper;
	size_x =  MIN(size_x,128);
	size_y =  MIN(size_y,128);
	buf = (guchar *) x_malloc(3*size_x*size_y*sizeof(guchar), "const guchar (buf in hf_get_icon)");
	
	ratio_x = ((gfloat) hfw->hf_struct->max_x) / (gfloat) size_x;	
	ratio_y = ((gfloat) hfw->hf_struct->max_y) / (gfloat) size_y;
	
	len_x = (gint) (hfw->hf_struct->max_x*ratio_y);
	
	for (i=0; i<size_x; i++)
		for (j=0; j<size_y; j++) {
			*(buf + j*size_x*3 + i*3) = (*(hfw->hf_struct->hf_buf + (j*len_x) + (gint) (i*ratio_x))) >>8;
			*(buf + j*size_x*3 + i*3 + 1) = *(buf + j*size_x*3 + i*3);
			*(buf + j*size_x*3 + i*3 + 2) = *(buf + j*size_x*3 + i*3);
	}
	pixbuf = gdk_pixbuf_new_from_data (buf, GDK_COLORSPACE_RGB, FALSE, 8, size_x, size_y, 3*size_x, free_buf, (gpointer) &buf);
	area = gtk_image_new_from_pixbuf( pixbuf );
	
	gtk_widget_show(area);
	
	return area;
}

void hf_wrapper_save (gpointer data, gchar *path_n_file) {

	hf_wrapper_struct *hfw;
	if (!data) {
		my_msg(_("There is nothing to save!"),WARNING);
		return;
	}
	hfw = (hf_wrapper_struct *) data;
	if (hfw->hf_struct) {
		hf_write(hfw->hf_struct, path_n_file, hfw->gl_preview->cameras);
	}
	else
		my_msg(_("There is nothing to save!"),WARNING);
}

void hf_stats(gpointer data) {
	hf_wrapper_struct *hfw;
	view_struct *vs=NULL;
	guint HISTOGRAM_WIDTH = 384;
	guint HISTOGRAM_HEIGHT = 256;
	static gchar buf[200];
	hfw = (hf_wrapper_struct *) data;
	if (!hfw)
		return;
	hf_min_max (hfw->hf_struct);
	sprintf(buf,"%s ... MIN: %d;  MAX: %d;  %s: %d.",
		*hfw->filename,hfw->hf_struct->min, hfw->hf_struct->max,
		_("Average"), hfw->hf_struct->avrg);
	
	vs = view_struct_new (HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT, NULL, NULL);
	histogram(hfw->hf_struct, HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT, vs->buf8);
	
	my_msg_with_image("HISTOGRAM",buf,vs->area);
	view_struct_free(vs);
}

/**********************************************************************************/

// Undo / Redo stack management - think about a separated class in the future

void accept_fn (hf_wrapper_struct *hfw) {
	if (!hfw)
		return;
//	printf("********** ACCEPT_FN\n");
	hf_backup(hfw->hf_struct);
	commit_pending_record(hfw);
	// Resetting the parameter values (set_fn) could trigger a recalculation
	// This is not what we want
	hfw->if_calculated = TRUE;
	if (hfw->hf_options->img->set_fn)
		(*hfw->hf_options->img->set_fn) ((gpointer)hfw->hf_options->img);
//	Check if recomputing min-max is necessary or can be done on demand
//	hf_min_max(hfw->hf_struct);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->accept_wdg),FALSE);
	hfw->if_calculated = FALSE;
}

void reset_fn (hf_wrapper_struct *hfw) {
//	Cancels the current operation by copying the temp buffer (backuped)
// 	into the current (modified) one
//	printf("********** RESET_FN\n");
	if (!hfw)
		return;
	if (hfw->hf_options->img->set_fn)
		(*hfw->hf_options->img->set_fn) ((gpointer)hfw->hf_options->img);
	hf_restore (hfw->hf_struct);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->accept_wdg),FALSE);
	// Added 2007-01-02
	hfw->if_calculated = FALSE;
	draw_hf(hfw);
}

// Standard accept / refresh functions for drawing

void accept_draw_fn (hf_wrapper_struct *hfw) {
	if (!hfw)
		return;
//	printf("********** ACCEPT_DRAW_FN\n");
	hf_backup(hfw->hf_struct);
	commit_pending_record(hfw);
	hfw->if_calculated = FALSE;
}

void reset_draw_fn (hf_wrapper_struct *hfw) {
//	Cancels the current operation by copying the temp buffer (backuped)
// 	into the current (modified) one
//	printf("********** RESET_DRAW_CALLB\n");
	if (!hfw)
		return;
	hf_restore (hfw->hf_struct);
	hfw->if_calculated = FALSE;
	draw_hf(hfw);
}

void record_action (hf_wrapper_struct *data, gchar *action_lbl) {
//	Record action in the standard undo / redo stack
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) data;
// printf("RECORD ACTION: %s\n", _(action_lbl));
	if (hfw->type != HFMAIN)	// Don't record an HFPEN, for instance
		return;
	commit_pending_record (data);
//	if (hfw->init_history_flag)
//		(*hfw->init_history_flag) (hfw->window, hfw->to_register_in_history);
	hfw->last_action = _(action_lbl);
}

void begin_pending_record (hf_wrapper_struct *hfw, gchar *action_lbl,
	gpointer commit_fn,
	gpointer reset_fn) {

//	Anticipation of an action recording in the standard undo / redo stack
//	The signal for actually recording the action in the stack would be
//	emitted by commit_pending_record, after, for instance, a context change
//	Added 2004-03: initializes the functions required to commit / reset
//				the changes when we change context

//	printf("BEGIN_PENDING_RECORD: %s\n",action_lbl);
	if (hfw->to_register_in_history)
		(*hfw->to_register_in_history) = TRUE;
	hfw->last_action = _(action_lbl);
	set_commit_reset_fns (hfw->hf_options->stack, commit_fn, reset_fn, (gpointer) hfw);
}

void commit_pending_record (hf_wrapper_struct *hfw) {

//	Commits an anticipated action record, in the undo / redo stack
	if (*hfw->to_register_in_history)
		if (hfw->registering_fn && hfw->registering_data) {
//			printf("Executing registering_fn\n");
			(*hfw->registering_fn) (hfw->registering_data);
		}
}

/**********************************************************************************/

// Utilities requiring hf_wrapper_struct *

void set_watch_cursor (hf_wrapper_struct *hfw) {
	// For long processes...
	// We set it on all widgets / windows
	// The HF area has its own cursor (current->cursor)
	gdk_window_set_cursor(hfw->area->window, hfw->hf_options->watch_cursor);
	gdk_window_set_cursor(hfw->window->window, hfw->hf_options->watch_cursor);
	gdk_window_set_cursor(hfw->hf_options->tools_dialog->window, hfw->hf_options->watch_cursor);
	hfw->hf_options->current_cursor = hfw->hf_options->watch_cursor;
	gdk_flush();
}

void unset_watch_cursor (hf_wrapper_struct *hfw) {
	gdk_window_set_cursor(hfw->area->window, hfw->hf_options->default_cursor);
	gdk_window_set_cursor(hfw->window->window, hfw->hf_options->default_cursor);
	gdk_window_set_cursor(hfw->hf_options->tools_dialog->window, hfw->hf_options->default_cursor);
	hfw->hf_options->current_cursor = hfw->hf_options->default_cursor;
}
