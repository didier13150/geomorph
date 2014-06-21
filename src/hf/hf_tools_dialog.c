/* hf_tools_dialog.c - height field tools interface functions
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

#include "hf.h"
#include "hf_wrapper.h"
#include "hf_tools_dialog.h"
#include "img_process_dialog.h"
#include "draw_tools_dialog.h"
#include "../utils/menus_n_tools.h"
#include "render.h"

extern command_item_struct *render_commands;

static gboolean flag_true=TRUE;

// Toolbar icons for main HF tools
#define NBHFTOOLS 2
// Modal dialogs
#define NBMODALTOOLS 1
#include "../icons/transform.xpm"
#include "../icons/pencil.xpm"
#include "../icons/fourier2.xpm"
// #include "../icons/merge_dialog.xpm"
// #include "../icons/scale.xpm"

command_item_struct hftools[NBHFTOOLS] = {
{"Edit", 	  "Transform", "Filters and other transformations", 0, (gchar **) transform_xpm, GDK_LEFT_PTR, transform_callb,NULL, NULL, TRUE},
{"Edit", 	  "Draw", "Drawing tools", 0, (gchar **) pencil_xpm, GDK_PENCIL, draw_tools_callb,NULL, NULL,FALSE}
};

command_item_struct modaltools[NBMODALTOOLS] = {
{"Edit", 	  "Fourier", "The Fourier Explorer", 0, (gchar **) fourier2_xpm, GDK_LEFT_PTR, fourier_callb, NULL, NULL, FALSE}
// {"Edit", 	  "Merge", "Merge Dialog", 0, (gchar **) merge_dialog_xpm, GDK_LEFT_PTR, transform_callb, NULL, NULL, FALSE},
// {"Edit", 	  "Scale", "Scale and crop dialog", 0, (gchar **) scale_xpm, GDK_LEFT_PTR, transform_callb, NULL, NULL,FALSE}
};

GtkWidget * hf_tools_dialog_new (GtkWidget *window, GtkTooltips *tooltips,
	gpointer hf_opt, gpointer data) {
	// Initializes the dialog for editing height fields
	// Two parts, from top to bottom:
	//	1. Kind of edition:  paint, transform, run / stop (not edition, but...)
	//	2. Toolbar / options for the current edit button
	// Returns a vbox container, allowing to add / substract particular controls
	// "window" only needed for heritance of characteristics in child toolbar(s)
	// ... could be the application main window
	// "tooltips" needed for packing tooltips, could have only one for the whole app.
	// gpointer data:  is normally the address of the current hf_wrapper pointer (hf_wrapper **)

	GtkWidget *dialog, *hbox, *tb, *tmp;
	hf_options_struct *hf_options;
	hf_options = (hf_options_struct *) hf_opt;
	dialog = gtk_vbox_new(FALSE, DEF_PAD);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(GTK_WIDGET(hbox));
	define_label_in_box("Edit", hbox, FALSE, FALSE, DEF_PAD*0.5);
	tb = standard_toolbar_new(NBHFTOOLS, hftools,
		tooltips,
		window,
		data,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		TRUE);
	// Modal windows: non toggleable buttons
	gtk_box_pack_start(GTK_BOX(hbox), tb, FALSE, FALSE, DEF_PAD);
	tb = standard_toolbar_new(NBMODALTOOLS, modaltools,
		tooltips,
		window,
		data,
		GTK_ORIENTATION_HORIZONTAL,
		GTK_TOOLBAR_ICONS,
		FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), tb, FALSE, FALSE, DEF_PAD);
	gtk_box_pack_start(GTK_BOX(dialog), hbox, FALSE, FALSE, DEF_PAD);

	gtk_box_pack_start(GTK_BOX(hbox), create_render_bar (window,tooltips,hf_options->render_str), FALSE, FALSE, DEF_PAD);

	gtk_widget_show(dialog);

	//	Sub-dialogs creation

	hf_options->draw =
		draw_tools_new(NULL,hf_options->dist_matrix, hf_options->gauss_list);
	hf_options->draw->draw_tools_dialog =	draw_tools_dialog_new(window, tooltips, hf_options->draw, hf_opt);

	gtk_container_add(GTK_CONTAINER(dialog), hf_options->draw->draw_tools_dialog);

	tmp = img_process_dialog_new(window, tooltips, data);
	hf_options->img = img_struct_new(NULL, data);
	hf_options->img->img_dialog = tmp;
	gtk_container_add(GTK_CONTAINER(dialog), tmp);

	//	The default is the transform dialog

	hf_options->current_tools = hf_options->img->img_dialog;
	gtk_widget_show(hf_options->img->img_dialog);
	gtk_widget_hide(hf_options->draw->draw_tools_dialog);

	hf_options->context = TRANSFORM_CONTEXT;
	hf_options->current_cursor = hf_options->default_cursor;

//	Default img subdialog - brightness / contrast
	hf_options->img->br_contr_dialog = br_contr_dialog_new(hf_options->img, data);
	gtk_container_add(GTK_CONTAINER(hf_options->img->img_dialog),
		hf_options->img->br_contr_dialog );
	hf_options->img->current_subdialog = hf_options->img->br_contr_dialog;
	hf_options->img->accept_wdg = hf_options->img->br_contr_accept ;
	hf_options->img->set_fn = (gpointer) set_br_contr_defaults;

	hf_options->tools_dialog = dialog;

	return dialog;
}

void transform_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw) return;
//	printf("ENTERING TRANSFORM\n");
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		// We are leaving the "transform" context, commit the changes
//	printf("LEAVING TRANSFORM\n");
			hfw->hf_options->need_tmp_buf = FALSE;
			hf_commit_or_reset (hfw->hf_options);
			return;
		}
	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->default_cursor;
	gdk_window_set_cursor(hfw->area->window, hfw->hf_options->current_cursor);
	if (hfw->hf_options->current_tools)
		gtk_widget_hide(hfw->hf_options->current_tools);
	if (hfw->hf_options->img) {
		hfw->hf_options->current_tools = hfw->hf_options->img->img_dialog;
		gtk_widget_show(hfw->hf_options->current_tools);
	}
	hf_backup(hfw->hf_struct);
	hfw->hf_options->need_tmp_buf = TRUE;
	hfw->hf_options->context = TRANSFORM_CONTEXT;
}

void fourier_post_processing (hf_wrapper_struct *hfw) {
	begin_pending_record (hfw, _("Fourier transform"), accept_fn, reset_fn);
	commit_pending_record (hfw);
	hf_backup(hfw->hf_struct);
	hf_wrapper_display(hfw);
	*(hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
// printf("Fourier Post Processing\n");
}

void fourier_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw) return;
	// This is a modal dialog, so we don't change the original context
// printf("ENTERING FOURIER\n");
	commit_pending_record(hfw);
	hf_backup(hfw->hf_struct);
//	printf("HFW->HF_STRUCT->HF_BUF: %d\n",hfw->hf_struct->hf_buf);

	init_render_struct(hfw->hf_options->fourier->render_str, 
		hfw->dirname, 
		hfw->filename, 
		&flag_true,
		&flag_true,
		NULL,
		NULL,0,0); // 4 last values are initialized in a fourier_dialog callback

	activate_fourier_dialog(hfw->hf_options->fourier, hfw->hf_struct->tmp_buf, hfw->hf_struct->hf_buf, hfw->hf_struct->max_x, hfw->hf_struct->max_y,HF_TYPE_ID, (gpointer) fourier_post_processing, (gpointer) hfw);
}

void draw_tools_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw) return;

//	printf("ENTERING DRAW TOOLS\n");
	if (wdg)
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		// We are leaving the "draw" context, commit the changes
//	printf("LEAVING DRAW TOOLS\n");
			hfw->hf_options->need_tmp_buf = FALSE;
			hf_commit_or_reset (hfw->hf_options);
			return;
		}

	commit_pending_record(hfw);
	hfw->hf_options->current_cursor = hfw->hf_options->pencil_cursor;
	gdk_window_set_cursor(hfw->area->window, hfw->hf_options->current_cursor);

	if (hfw->hf_options->current_tools)
		gtk_widget_hide(hfw->hf_options->current_tools);

	if (hfw->hf_options->draw) {
		if (hfw->hf_options->draw->draw_tools_dialog) {
			hfw->hf_options->current_tools = hfw->hf_options->draw->draw_tools_dialog;
			gtk_widget_show(hfw->hf_options->current_tools);
		}
		else
			return;
	}

	hf_backup(hfw->hf_struct);
	hfw->hf_options->need_tmp_buf = TRUE;
	hfw->hf_options->context = DRAW_TOOLS_CONTEXT;
}

