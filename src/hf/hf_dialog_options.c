/* hf_dialog_options.c - "Pot-pourri" of functions and structures
 *	for managing the HF dialogs - created from hf_wrapper.c on 2006-01-01
 *
 * Copyright (C) 2001, 2006, 2010 Patrice St-Gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
#include "hf.h"
#include "hf_dialog_options.h"
#include "hf_wrapper.h"
#include "hf_filters_dialog.h"
#include "hf_creation_dialog.h"
#include "hf_tools_dialog.h"
#include "gl_preview.h"
#include "render.h"
#include "draw_tools_dialog.h"
#include "img_process_dialog.h"

extern gchar *DEFAULT_TERRAIN;

void set_creation_container (hf_options_struct *hfo, GtkWidget *creation_container) {
	gboolean already_set=FALSE;
	if (hfo->creation_container)
		already_set = TRUE;
	if (!(creation_container && hfo))
		return;
// printf("HFO->CREATION_CONTAINER in SET_CREATION_CONTAINER: %d->%d\n",hfo,creation_container);
	if (GTK_WIDGET(creation_container))
		if (GTK_IS_CONTAINER(creation_container))
			hfo->creation_container = creation_container;
		else
			printf("SET_CREATION_CONTAINER: %ld is not a container\n", (long int) creation_container);
	else
		printf("SET_CREATION_CONTAINER: %ld is not a widget\n", (long int) creation_container);
	if (!already_set)
		gtk_box_pack_start(GTK_BOX(creation_container), hfo->fd_struct->filter_dialog, FALSE, FALSE, 0);
//		gtk_container_add(GTK_CONTAINER(creation_container), hfo->fd_struct->filter_dialog);
}

void hf_dialog_defaults (hf_options_struct *hfo) {
/*
	printf("DEFAULT_TERRAIN: %s\n",DEFAULT_TERRAIN);
	printf("HFO in DIALOG DEFAULTS: %d\n",hfo);
	printf("CREATION_WIDGETS in DIALOG DEFAULTS: %d\n",hfo->creation_widgets);
	printf("HF_TYPES_TOOLBAR in DIALOG DEFAULTS: %d\n",hfo->creation_widgets->hf_types_toolbar);
*/
	set_toolbar_button_from_label(hfo->creation_widgets->hf_types_toolbar, DEFAULT_TERRAIN, FALSE);
	set_toolbar_button_from_label(hfo->creation_widgets->hf_types_toolbar, DEFAULT_TERRAIN, TRUE);
}

gpointer hf_options_dialog_new(GtkWidget *window,
		GtkWidget *creation_container,
		GtkTooltips *tooltips, 
		gpointer widget_list, 
		gpointer data,
		stack_struct *stack,
		gchar *doc_type, gpointer all_docs,
		GList *(*get_doc_list_from_type) (gpointer all_docs, gchar *doc_type)) {
	// Options structure related to the tools dialog,
	// initialized once for all the documents
	// doc_type, all_docs, get_doc_list_from_type:  required for building a list of other available open documents
	gint i;
	hf_options_struct *hf_options;
	hf_options = (hf_options_struct *) x_malloc(sizeof(hf_options_struct), "hf_options_struct");
	hf_options->creation_container = creation_container;
	hf_options->tools_window = window;
	hf_options->tools_dialog = NULL;
	hf_options->hfw_ptr_ptr = data;
	hf_options->creation_widgets = (hf_creation_wdg_struct *) widget_list;
	hf_options->all_docs = all_docs;
	hf_options->doc_type = doc_type;
	hf_options->get_doc_list_from_type = get_doc_list_from_type;
	hf_options->current_tools = NULL;
	hf_options->context = TRANSFORM_CONTEXT;
	hf_options->last_creation_action = NULL;
	hf_options->tooltips = tooltips;
	hf_options->current_callb = NULL;
	hf_options->current_calculation = NULL;
	hf_options->current_options = NULL;
	hf_options->current_dialog = NULL ;
	hf_options->subdiv1_dialog = NULL;
	hf_options->subdiv2_dialog = NULL;
	hf_options->subdiv2_direct_upd = TRUE;
	hf_options->waves_dialog = NULL;
	hf_options->surfadd_dialog = NULL;
	hf_options->uniform_value = (hf_type *) x_malloc(sizeof(hf_type), "hf_type (hf_options->uniform_value)");
	(*hf_options->uniform_value) = 32767;
	hf_options->uniform_dialog = NULL;
	hf_options->subdiv1_options = NULL;
	hf_options->subdiv2_options = NULL;
	hf_options->surfadd_options = NULL;
	hf_options->waves_options = NULL;
	hf_options->wide_filter = NULL;
	hf_options->medium_filter1 = NULL;
	hf_options->medium_filter2 = NULL;
	hf_options->sharp_filter = NULL;
	hf_options->image_filter = NULL;
	hf_options->gauss_filter = NULL;
	hf_options->dist_matrix = dist_matrix_new(0);
	hf_options->creation_fd_struct = filter_dialog_new(window,tooltips,data);
	hf_options->fd_struct = hf_options->creation_fd_struct;
	hf_options->doc_list = NULL;
//	The default is to add the main filters dialog to the creation container
	if (creation_container)
		gtk_box_pack_start(GTK_BOX(creation_container), hf_options->fd_struct->filter_dialog, FALSE, FALSE, 0);
	hf_options->flag_uniform_done = FALSE;
	set_filter_defaults(hf_options->fd_struct);

	hf_options->draw = NULL;

// 	Cursors
	hf_options->default_cursor = gdk_cursor_new(GDK_LEFT_PTR);
	hf_options->current_cursor = hf_options->default_cursor;
	hf_options->pencil_cursor = gdk_cursor_new(GDK_PENCIL);
	hf_options->watch_cursor = gdk_cursor_new(GDK_WATCH);
	hf_options->fleur_cursor = gdk_cursor_new(GDK_FLEUR);
	hf_options->hf_size = 0;	// Initialized by hf_wrapper_new and size_callb
	hf_options->img = NULL;
	hf_options->need_tmp_buf = FALSE;	// Flag:  if we need temp buffer for brightness / contrast
						// useful after an undo...

//	OpenGL preview
	hf_options->gl_defaults = gl_defaults_init ( );

	for (i=0; i<GAUSS_LIST_LENGTH; i++)
		hf_options->gauss_list[i] = NULL;

	// Context management functions and data
	// The pointer field is inherited from app.h
	hf_options->stack = stack;

	hf_options->primit_surf = NULL;
	hf_options->fourier = fourier_dialog_new();
	hf_options->render_str = render_struct_new();

	set_stack_specific_fn (hf_options->stack, (gpointer) hf_specific_commit_or_reset);

	return (gpointer) hf_options;
}

void hf_options_dialog_free (gpointer hf_opt_ptr) {
	gint i;
	hf_options_struct *hf_opt = (hf_options_struct *) hf_opt_ptr;
	if (!hf_opt) return;
// printf("HF_OPTIONS_DIALOG_FREE-1\n");
	if (hf_opt->creation_widgets)
		x_free(hf_opt->creation_widgets);
	if (hf_opt->uniform_value)
		x_free(hf_opt->uniform_value);
	if (hf_opt->subdiv1_options)
		x_free(hf_opt->subdiv1_options);
	if (hf_opt->subdiv2_options)
		x_free(hf_opt->subdiv2_options);
	if (hf_opt->surfadd_options)
		x_free(hf_opt->surfadd_options);
	if (hf_opt->waves_options)
		x_free(hf_opt->waves_options);
	if (hf_opt->wide_filter)
		filter_free(hf_opt->wide_filter);
	if (hf_opt->medium_filter1)
		filter_free(hf_opt->medium_filter1);
	if (hf_opt->medium_filter2)
		filter_free(hf_opt->medium_filter2);
	if (hf_opt->sharp_filter)
		filter_free(hf_opt->sharp_filter);
	if (hf_opt->gauss_filter)
		filter_free(hf_opt->gauss_filter);
	for (i=0; i<GAUSS_LIST_LENGTH; i++)
		if (hf_opt->gauss_list[i])
			x_free(hf_opt->gauss_list[i]);
	if (hf_opt->img)
		img_dialog_free(hf_opt->img);
	if (hf_opt->draw)
		draw_tools_free(hf_opt->draw);
	if (hf_opt->fourier)
		fourier_dialog_free(hf_opt->fourier);
	if (hf_opt->render_str)
		x_free(hf_opt->render_str);
	x_free(hf_opt);
// printf("HF_OPTIONS_DIALOG_FREE-4\n");
}

void hf_specific_commit_or_reset (stack_struct *stack) {
	hf_wrapper_struct *hfw;
//	printf("HF_SPECIFIC_COMMIT_OR_RESET 1 on %p\n", stack->commit_data);
	if (stack->commit_data) {
		hfw = (hf_wrapper_struct *) stack->commit_data;
//		printf("HF_SPECIFIC_COMMIT_OR_RESET 2 on %p: hf_buf= %p\n",hfw,hfw->hf_struct->hf_buf);
		if (hfw->hf_options->img && hfw->hf_options->img->set_fn) 
			(*hfw->hf_options->img->set_fn) (hfw->hf_options->img);
		set_render_buffer (hfw->hf_options->render_str, hfw->hf_struct->hf_buf, hfw->hf_struct->max_x,hfw->hf_struct->max_y);
		hf_backup (hfw->hf_struct);
	}
}

// Transitional function (for backward compatibility) - 2010.12.05
void hf_commit_or_reset (hf_options_struct *hfo) {
	if (hfo && hfo->stack)
		commit_or_reset (hfo->stack);
}

