/* hf_creation_dialog.c - height field creation interface functions
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "hf_creation_dialog.h"
#include "render.h"
#include "hf.h"
#include "hf_wrapper.h"
#include "gl_preview.h"

#define NBSIZES 6
void size_callb (gpointer data,GtkWidget *wdg);

omenu_list_type sizes_list[NBSIZES] = {
	{NULL,128,NULL,"128",size_callb},
	{NULL,256,NULL,"256",size_callb},
	{NULL,512,NULL,"512",size_callb},
	{NULL,1024,NULL,"1024",size_callb},
	{NULL,2048,NULL,"2048",size_callb},
	{NULL,4096,NULL,"4096",size_callb},
};

// Toolbar icons for HF creation algorithm
#define NBHFTYPES 4
#include "../icons/uniform.xpm"
#include "../icons/subdiv1.xpm"
#include "../icons/subdiv2.xpm"
// #include "../icons/waves.xpm"
// #include "../icons/waves1.xpm"
#include "../icons/surfadd.xpm"

extern gchar* DEFAULT_TERRAIN;

command_item_struct hftypes[NBHFTYPES] = {
{"Type", "Uniform", "Uniform color",0, (gchar **) uniform_xpm, GDK_LEFT_PTR, hfuniform_callb, NULL, NULL,TRUE},
{"Type", "Subdiv1", "Subdivision 1",0, (gchar **) subdiv1_xpm, GDK_LEFT_PTR, hfsubdiv1_callb,NULL, NULL,FALSE},
{"Type", "Subdiv2", "Subdivision 2",0, (gchar **) subdiv2_xpm, GDK_LEFT_PTR, hfsubdiv2_callb, NULL, NULL,FALSE},
{"Type", "SurfSum", "Progressive sum of surfaces",0, (gchar **) surfadd_xpm, GDK_LEFT_PTR, hfsurfadd_callb, NULL, NULL,FALSE}
};

/***************************************************************************/

void set_size_menu_from_size (gpointer hfo_ptr, gint size) {
	hf_options_struct *hfo = (hf_options_struct *) hfo_ptr;
	static gchar label[5];
	if (hfo->creation_widgets && hfo->creation_widgets->size_menu) {
		sprintf(label, "%-4u", (guint) size);
//		printf("LABEL: %s\n",label);
		set_option_menu_from_label (hfo->creation_widgets->size_menu->omenu_ptr, label);
	}
}

void size_callb (gpointer data,GtkWidget *wdg) {
	// Sets size when creating a new HF
	gint hf_size;
	static gchar area_label[12];
	omenu_struct_type *opt_menu;
	hf_wrapper_struct *hfw;
	hf_options_struct *hfo;
	opt_menu = (omenu_struct_type *) data;
//	printf("DATA in SIZE_CALLB: %d;\n",data);
	hfo = (hf_options_struct *) opt_menu->data;
//	printf("HFO in SIZE_CALLB: %d;\n",hfo);
	if (!hfo)
		return;

	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **)  hfo->hfw_ptr_ptr;

	hf_size = opt_menu->list[opt_menu->current_index].num;
	if (!hfw)
		return;
	if (!hfw->hf_struct)
		return;
	if (hf_size == hfw->hf_struct->max_x)
		//	No change!
		return;

	sprintf(area_label, "%d x %d", hf_size, hf_size);
	gtk_frame_set_label(GTK_FRAME(hfw->draw_area_frame), area_label);

	hf_free(hfw->hf_struct);
	hfw->hf_options->hf_size = hf_size;
	hfw->hf_struct = (hf_struct_type *) hf_new(hf_size);
	hfw->display_scale = get_display_scale(hf_size);
	hfw->hf_buf8 = (unsigned char *) x_malloc((RIGHT_SHIFT(hf_size, hfw->display_scale)) * (RIGHT_SHIFT(hf_size,hfw->display_scale))*sizeof(unsigned char), "unsigned char (hfw->buf8 in size_callb)");
	hfw->if_rgb = FALSE;
	hfw->area_size = RIGHT_SHIFT(hf_size,hfw->display_scale);
	set_current_button (hfw->sbs, hfw->display_scale);
	hfw->size_change_request = TRUE;
	
	gtk_widget_set_size_request (hfw->draw_area_frame, hfw->area_size+SCALE_PAD, hfw->area_size+SCALE_PAD);

//	printf("SIZE_CALLB: HFW: %d; HF_SIZE: %d; DISPLAY_SCALE: %d; AREA_SIZE: %d\n", hfw, hf_size, hfw->display_scale, hfw->area_size);
	
	init_render_struct(hfw->hf_options->render_str, 
		hfw->dirname, 
		hfw->filename, 
		&hfw->creation_mode,
		hfw->if_modified,
		hfw->gl_preview,
		hfw->hf_struct->hf_buf,
		hfw->hf_struct->max_x,
		hfw->hf_struct->max_y);

	gl_set_input_grid (hfw->gl_preview, hfw->hf_struct->hf_buf, hfw->hf_struct->max_x, hfw->hf_struct->max_y, HF_TYPE_ID);
	gl_update_details_display (hfw->gl_preview);
	gtk_drawing_area_size(GTK_DRAWING_AREA(hfw->area), 
		hfw->area_size, hfw->area_size);
}

GtkWidget *hf_types_toolbar_new(GtkWidget *window, GtkTooltips *tooltips, 
		GtkWidget** tb_widget, gpointer hf_options_ptr) {
	GtkWidget *hbox;
	GtkWidget *tb;
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_label_in_box("Type", hbox, FALSE, FALSE, DEF_PAD*0.5);
	tb = standard_toolbar_new(NBHFTYPES,
		hftypes,
		tooltips,
		window,
		hf_options_ptr,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), tb, FALSE, FALSE, DEF_PAD*0.5);
	if (tb_widget)
		*tb_widget = tb;
	return hbox;
}

GtkWidget * hf_creation_dialog_new(GtkWidget *window, GtkTooltips *tooltips, 
		gpointer hf_options_ptr, gpointer widget_list_ptr) {
	// Initializes the dialog for creating height fields
	// Four parts, from top to bottom:
	//	1. Kind / type of HF
	//	2. Shape filters
	//	3. Generic options (size...)
	//	4. Options for the current HF
	// Returns a vbox container, allowing to add / substract particular controls
	// "window" only needed for heritance of characteristics in child toolbar(s)
	// "tooltips" needed for packing tooltips, could have only one for the whole app.
	// gpointer hf_options_ptr_pgtr:  is normally the address of the current hf_options pointer 
	// (hf_options_struct **)

	GtkWidget *dialog, *hbox, *vbox, *frame;
	gint default_hf_size;
	omenu_struct_type *opt_menu;
	hf_creation_wdg_struct *wdg_str;
//	printf("HFO PTR in HF_CREATION_DIALOG_NEW: %d - %x\n",hf_options_ptr, hf_options_ptr);
	wdg_str = (hf_creation_wdg_struct *) x_calloc(sizeof(hf_creation_wdg_struct),1, "hf_creation_wdg_struct");
	*((hf_creation_wdg_struct **) widget_list_ptr) = wdg_str;

	((hf_options_struct *)hf_options_ptr)->creation_widgets = wdg_str;

	frame = options_frame_new("HeightField");

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);

	//	1st generic option:  HF size - a power of 2
	//	Default:  some % of screen width
	
	hbox = gtk_hbox_new(FALSE, DEF_PAD);
	gtk_widget_show(hbox);
	define_label_in_box("Size", hbox, FALSE, FALSE, DEF_PAD);

	default_hf_size = hf_default_size();
	
	create_omenu_with_callb_data (&opt_menu, sizes_list,
		NBSIZES, 
		find_omenu_num(sizes_list, default_hf_size, NBSIZES),
		hf_options_ptr);
	gtk_box_pack_start(GTK_BOX(hbox), opt_menu->omenu_ptr, FALSE, FALSE, 0);

	wdg_str->size_menu = opt_menu;

 	gtk_box_pack_start (GTK_BOX (hbox), create_render_bar (window, tooltips, ((hf_options_struct *)hf_options_ptr)->render_str), FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox),
		hf_types_toolbar_new(window, tooltips, &wdg_str->hf_types_toolbar,hf_options_ptr), FALSE, FALSE, 0);

	//	Other generic options ?

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	dialog = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(dialog), frame, FALSE, FALSE, 0);
	gtk_widget_show(dialog);

	return dialog;
}

void hf_commit_creation (gpointer data) {
//	Commit the filter
	filter_dialog_struct *fds;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) data;
//	printf("HFW, HFW->creation_mode in hf_commit_creation: %d, %d\n",data,hfw->creation_mode);
// printf("HFW->window->window: %d->%d\n",hfw->window,hfw->window->window);
	hfw->creation_mode = FALSE;  // Supposed to be done in hf_wrapper_copy...
//	hfw->hf_options->current_calculation = NULL;
	hfw->apply_filter = FALSE;

//	Backup the filter definition and nullify it - we 1st commit the HF without the filter
	if (hfw->hf_options->fd_struct) {
		set_filter_defaults (hfw->hf_options->fd_struct);
	}
	
	record_action(hfw, hfw->hf_options->last_creation_action);
	if (hfw->hf_options->fd_struct) {
		fds = hfw->hf_options->fd_struct;
		if (fds->current_filter) {
		    filter_apply(hfw->hf_struct,
			fds->current_filter,
			hfw->hf_options->dist_matrix,
			fds->revert_filter,
			fds->filter_level,
			fds->merge_oper);
//	Now we commit the HF with the filter
		record_action(hfw,_("Shape filter"));

		}
	}
//	If we are in the shape filter subdialog in the tools dialog:
	if (hfw->hf_options->img)
		hfw->hf_options->fd_struct = hfw->hf_options->img->fd_struct;
	else
		hfw->hf_options->fd_struct = NULL;

//	"expose_event" seems required to display the cursor ?
	emit_expose_event(hfw->area);
}

// HF creation

void hfuniform_callb(GtkWidget *wdg, gpointer hf_options_ptr) {
	// Display / connect callback for uniform hf (a simple scale from 0 to 0xFFFF)

	hf_wrapper_struct *hfw;
	hf_options_struct *hfo = (hf_options_struct *) hf_options_ptr;

	if (!hfo)
		return;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;

// printf("IN hfuniform_callb:  hfo: %d;\n", hfo);

	//  Options dialog
	if (!hfo->uniform_dialog) {
		// NB hfw->hf_options->uniform_value allocated in hf_options_new()
		hfo->uniform_dialog = uniform_dialog_new(hfo,
			*hfo->uniform_value);
		gtk_box_pack_start(GTK_BOX(hfo->creation_container), hfo->uniform_dialog, FALSE, FALSE, 0);
	}
	if (hfo->current_dialog && GTK_IS_WIDGET(hfo->current_dialog))
		gtk_widget_hide(hfo->current_dialog);
	gtk_widget_show(hfo->uniform_dialog);
	hfo->last_creation_action = _("Uniform");
	hfo->current_dialog = hfo->uniform_dialog;
	hfo->current_calculation = (gpointer) uniform;
	hfo->current_callb = hfuniform_callb;
	hfo->current_options = (gpointer) hfo->uniform_value;
	hfo->flag_uniform_done = FALSE;
	if (hfo->hfw_ptr_ptr) {
		hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) hfo->hfw_ptr_ptr;
		if (hfw)
			gener_hf(hfw);
	}
}

void hfsubdiv1_callb(GtkWidget *wdg, gpointer hf_options_ptr) {
	// Display / connect callback
	// Simple subdivision, square / diamond dots

	hf_wrapper_struct *hfw;
	hf_options_struct *hfo = (hf_options_struct *) hf_options_ptr;
	if (!hfo)
		return;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;

// printf("IN hfsubdiv1_callb:  hfo: %d;\n", hfo);

	//  Options dialog - we create it at the 1st pass
	if (!hfo->subdiv1_dialog) {
	//	We create the struct which will receive the data
		hfo->subdiv1_options = subdiv1_opt_new();
	//	Dialog creation (with some defaults from subdiv1_opt_new())
		hfo->subdiv1_dialog = subdiv1_dialog_new(hfo);
		gtk_box_pack_start(GTK_BOX(hfo->creation_container), hfo->subdiv1_dialog, FALSE, FALSE, 0);
	}
	if (hfo->current_dialog && GTK_IS_WIDGET(hfo->current_dialog))
		gtk_widget_hide(hfo->current_dialog);
	gtk_widget_show(hfo->subdiv1_dialog);
	hfo->last_creation_action = _("Subdiv1");
	hfo->current_dialog= hfo->subdiv1_dialog;
	hfo->current_calculation = (gpointer) subdiv1;
	hfo->current_callb = hfsubdiv1_callb;
	hfo->current_options = (gpointer) hfo->subdiv1_options;
	if (hfo->hfw_ptr_ptr) {
		hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) hfo->hfw_ptr_ptr;
		if (hfw)
			gener_hf(hfw);
	}
}

void hfsubdiv2_callb(GtkWidget *wdg, gpointer hf_options_ptr) {
	// Display / connect callback
	// Simple subdivision, square / diamond dots

	static gint frq = 50;
	hf_wrapper_struct *hfw;
	hf_options_struct *hfo = (hf_options_struct *) hf_options_ptr;
	if (!hfo)
		return;
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;

// printf("IN hfsubdiv2_callb:  hfo: %d;\n", hfo);

	//  Options dialog - we create it at the 1st pass
	if (!hfo->subdiv2_dialog) {
	//	We create the struct which will receive the data
		hfo->subdiv2_options = subdiv2_opt_new(&frq,1);
	//	Dialog creation (with some defaults from subdiv2_opt_new())
		hfo->subdiv2_dialog = subdiv2_dialog_new(hfo);
		gtk_box_pack_start(GTK_BOX(hfo->creation_container), hfo->subdiv2_dialog, FALSE, FALSE, 0);
	}
	if (hfo->current_dialog && GTK_IS_WIDGET(hfo->current_dialog))
		gtk_widget_hide(hfo->current_dialog);
	gtk_widget_show(hfo->subdiv2_dialog);
	hfo->last_creation_action = _("Subdiv2");
	hfo->current_dialog = hfo->subdiv2_dialog;
	hfo->current_calculation = (gpointer) subdiv2;
	hfo->current_callb = hfsubdiv2_callb;
	hfo->current_options = (gpointer) hfo->subdiv2_options;
	if (hfo->hfw_ptr_ptr) {
		hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) hfo->hfw_ptr_ptr;
		if (hfw)
			gener_hf(hfw);
	}
}

void hfsurfadd_callb(GtkWidget *wdg, gpointer hf_options_ptr) {
	// Progressive addition of surfaces

	hf_wrapper_struct *hfw;
	hf_options_struct *hfo = (hf_options_struct *) hf_options_ptr;
	if (!hfo)
		return;

	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
			return;

// printf("IN hfsurfadd_callb:  hfo: %d;\n", hfo);

	//  Options dialog - we create it at the 1st pass
	if (!hfo->surfadd_dialog) {
	//	We create the struct which will receive the data
		hfo->surfadd_options = surfadd_opt_new();
		hfo->surfadd_options->dist_matrix = hfo->dist_matrix;
	//	Dialog creation (with some defaults from surfadd_opt_new())
		hfo->surfadd_dialog = surfadd_dialog_new(hfo);
		hfo->surfadd_options->current_filter =
			&((hf_wrapper_struct *)hfo->primit_surf)->hf_options->fd_struct->current_filter;
		hfo->surfadd_options->revert_filter =
			&((hf_wrapper_struct *)hfo->primit_surf)->hf_options->fd_struct->revert_filter;
		hfo->surfadd_options->filter_merge =
			&((hf_wrapper_struct *)hfo->primit_surf)->hf_options->fd_struct->merge_oper;
		hfo->surfadd_options->filter_level =
			&((hf_wrapper_struct *)hfo->primit_surf)->hf_options->fd_struct->filter_level;
		gtk_container_add(GTK_CONTAINER(hfo->creation_container),
			hfo->surfadd_dialog);
	}
	if (hfo->current_dialog && GTK_IS_WIDGET(hfo->current_dialog))
		gtk_widget_hide(hfo->current_dialog);
	gtk_widget_show(hfo->surfadd_dialog);
	hfo->last_creation_action = _("SurfSum");
	hfo->current_dialog = hfo->surfadd_dialog;
	hfo->current_calculation = (gpointer) surfadd;
	hfo->current_callb = hfsurfadd_callb;
	hfo->current_options = (gpointer) hfo->surfadd_options;

	if (hfo->hfw_ptr_ptr) {
		hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) hfo->hfw_ptr_ptr;
		if (hfw)
			gener_hf(hfw);
	}
}
