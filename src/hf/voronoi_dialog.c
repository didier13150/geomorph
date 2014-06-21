/* voronoi_dialog.c:  dialogs for voronoi diagrams (cracks networks)
 *
 * Copyright (C) 2006 Patrice St-Gelais
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

#include <string.h>
#include "voronoi.h"
#include "hf.h"
#include "img_process_dialog.h"
#include "hf_wrapper.h"
#include "dialog_utilities.h"
#include "view_area.h"
#include "voronoi_dialog.h"


/************************* Constructors / destructors *************************/

voronoi_dialog_struct *voronoi_dialog_struct_new () {
	voronoi_dialog_struct * vds;
	vds = (voronoi_dialog_struct *) x_malloc(sizeof(voronoi_dialog_struct), "voronoi_dialog_struct");
	vds->dialog = NULL;
	vds->accept = NULL;
	vds->adj_cell_size = NULL;
	vds->adj_random_variation = NULL;
	vds->gener_noise = NULL;
	vds->crack_width_type = NULL;
	vds->adj_min_width = NULL;
	vds->adj_max_width = NULL;
	vds->wdg_edges_level = NULL;
	vds->adj_edges_level = NULL;
	vds->noise_preview = NULL;
	vds->preview_buf = xcalloc_hf_type(NOISE_PREVIEW_SIZE, "hf_type (vds->preview_buf in voronoi_dialog.c)");
	
	return vds;
}

void voronoi_dialog_struct_free (voronoi_dialog_struct* vds) {
	if (vds)
		x_free(vds);
}

/****************************************************************************/

void apply_voronoi (hf_wrapper_struct *hfw) {

//	gint t1;
	if (!hfw->hf_struct->tmp_buf)
		hf_backup(hfw->hf_struct);
	if (!hfw->hf_struct->result_buf)
		hfw->hf_struct->result_buf = 
			(hf_type *) x_malloc(sizeof(hf_type) * hfw->hf_struct->max_x  * hfw->hf_struct->max_y, "hf_type (result_buf in apply_voronoi)");
	if (!hfw->hf_struct->tmp2_buf)
		hfw->hf_struct->tmp2_buf = 
			(hf_type *) x_malloc(sizeof(hf_type) * hfw->hf_struct->max_x  * hfw->hf_struct->max_y, "hf_type (tmp2_buf in apply_voronoi)");
//	t1 = clock();
	// Convention:
	// 1. tmp_buf: original HF
	// 2. tmp2_buf: distance HF
	// 3. result_buf: crackled HF (base level + black lines == cracks)
	// 4. hf_buf: output = crackled HF + distance HF (for lifting edges)

	set_watch_cursor(hfw);	
	hf_voronoi(hfw->hf_struct, hfw->hf_options->img->voronoi);
	
	voronoi_adjust_edges (hfw->hf_struct->result_buf, hfw->hf_struct->hf_buf, hfw->hf_struct->tmp2_buf, hfw->hf_options->img->voronoi->edges_level,  hfw->hf_struct->max_x, hfw->hf_struct->max_y);
	
	unset_watch_cursor(hfw);
// printf("TEMPS DE CRAQUÈLEMENT: %d\n",clock() - t1);
}

void apply_voronoi_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (hfw->if_calculated)
		return;
	apply_voronoi(hfw);
	(*hfw->if_modified) = TRUE;
	hfw->if_calculated = TRUE;
	begin_pending_record(hfw,"Cracks network",accept_fn,reset_fn);
	gtk_widget_set_sensitive(GTK_WIDGET(hfw->hf_options->img->voronoi_dialog->accept),TRUE);
	draw_hf(hfw);
	hfw->if_calculated =  FALSE;
}

static gint change_voronoi_seed(GtkWidget *entry, gpointer data) {
	hf_wrapper_struct *hfw;
	if (!data)
		return FALSE;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	if (!hfw->hf_options->img->voronoi)
		return FALSE;
	if ( ! strlen( gtk_entry_get_text(GTK_ENTRY(entry))))
		return FALSE;
	if (hfw->hf_options->img->voronoi->seed == 
		(unsigned int) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry))))
		return FALSE;
	hfw->hf_options->img->voronoi->seed = (gint) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry)));
//	printf("SEED: %d\n",hfw->hf_options->img->voronoi->seed);
	hfw->if_calculated =  FALSE;
	apply_voronoi_callb (NULL, data);
	return FALSE;
}

void edges_level_callb (GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	
	voronoi_adjust_edges (hfw->hf_struct->result_buf, hfw->hf_struct->hf_buf, hfw->hf_struct->tmp2_buf, hfw->hf_options->img->voronoi->edges_level,  hfw->hf_struct->max_x, hfw->hf_struct->max_y);

	draw_hf(hfw);
}

void set_voronoi_defaults(img_dialog_struct *img) {
	gtk_widget_set_sensitive(GTK_WIDGET(img->voronoi_dialog->wdg_edges_level),FALSE);
//	gtk_adjustment_set_value( 
//			GTK_ADJUSTMENT(img->adj_erode_radius), 0);
}

void set_uniform_distrib (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = UNIFORM;
}
void set_centered_distrib (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = CENTERED;
}
void set_regular_distrib (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = REGULAR;
}

void set_scale_1x (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = SCALE_1X;
}
void set_scale_1x_2x (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = SCALE_1X_2X;
}
void set_scale_1x_2x_4x (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = SCALE_1X_2X_4X;
}

void set_use_as_noise (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = USE_AS_NOISE;
}
void set_use_as_guide (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = USE_AS_GUIDE;
}

void set_fixed_width (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = FIXED_WIDTH;
}
void set_from_distance (GtkWidget *wdg, gpointer data) {
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
	(*(gint *) data) = FROM_DISTANCE;
}

void check_min_width (GtkWidget *wdg, gpointer data) {
	// Make sure that min<=max when max is modified
	// (we modify min)
	img_dialog_struct *img = (img_dialog_struct *) data;
	if (img->voronoi->min_width>img->voronoi->max_width) {
		img->voronoi->min_width = img->voronoi->max_width;
		gtk_adjustment_set_value( 		GTK_ADJUSTMENT(img->voronoi_dialog->adj_min_width), img->voronoi->min_width);
	}
}

void check_max_width (GtkWidget *wdg, gpointer data) {
	// Make sure that min<=max when min is modified
	// (we modify max)
	img_dialog_struct *img = (img_dialog_struct *) data;
	if (img->voronoi->min_width>img->voronoi->max_width) {
		img->voronoi->max_width = img->voronoi->min_width;
		gtk_adjustment_set_value( 		GTK_ADJUSTMENT(img->voronoi_dialog->adj_max_width), img->voronoi->max_width);
	}
}

void compute_noise_preview (gpointer img_pointer, unsigned char *buf8, gint width, gint height) {
	guint i;
	img_dialog_struct *img = (img_dialog_struct *) img_pointer;
	// printf("COMPUTE_NOISE_PREVIEW\n");
	calc_subdiv1 (img->voronoi_dialog->preview_buf, width, height, img->voronoi->noise_opt);
	for (i=0; i<(width*height); i++) {
		*(buf8+i) = (unsigned char) (*(img->voronoi_dialog->preview_buf+i)>>8);
	}
}

gint change_noise_seed (GtkWidget *entry, gpointer img_pointer) {
	img_dialog_struct *img = (img_dialog_struct *) img_pointer;
	// printf("COMPUTE_NOISE_PREVIEW\n");
	 img->voronoi->noise_opt->seed =(gint) atoi((char *) gtk_entry_get_text(GTK_ENTRY(entry)));
	calc_subdiv1 (img->voronoi_dialog->preview_buf, NOISE_PREVIEW_SIZE, NOISE_PREVIEW_SIZE, img->voronoi->noise_opt);
	draw_area (img->voronoi_dialog->noise_preview);
	return FALSE;
}

GtkWidget *voronoi_dialog_new (gpointer data) {

	GtkWidget *vbox, *vbox2, *vbox3, *guide_box, *hbox, *hbox1, *hbox2, *frame, *frame2, *button, *wdg, *check_box, *lblmax, *lblwidth, *button_uniform, *preview_box, *scale;
	GtkObject *adj;
	GSList *group = NULL, *group1 = NULL, *group2 = NULL, *group3 = NULL;
	voronoi_struct *vs;
	voronoi_dialog_struct *vds;
	hf_wrapper_struct *hfw;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	vs = hfw->hf_options->img->voronoi;
	vds = hfw->hf_options->img->voronoi_dialog;
	
	frame = options_frame_new("Cracks network");

	vbox = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(vbox));

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Average cell size", hbox, FALSE,TRUE, DEF_PAD);
 	vds->adj_cell_size = gtk_adjustment_new (
		vs->cell_size,0.0, 50.0, 0.1, 0.1, 0.0);
	define_scale_in_box(vds->adj_cell_size,hbox,1,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (vds->adj_cell_size), "value_changed", GTK_SIGNAL_FUNC (gdouble_adj_callb), (gpointer) &vs->cell_size);
	gtk_signal_connect (GTK_OBJECT (vds->adj_cell_size), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);

  	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), 
		seed_dialog_new(data, vs->seed, change_voronoi_seed), 
		TRUE, TRUE, 0);
		
//	******** Distribution of cells

	//	Box for radio buttons
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox2));
	
	frame2 = define_frame_with_hiding_arrows ("Cell distribution", vbox2, hfw->hf_options->tools_window, FALSE);
	
	gtk_box_pack_start(GTK_BOX(vbox), frame2, TRUE, TRUE, 0);
	
	// We need a scale for the REGULAR option, for controlling the perturbation
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Random variation", hbox, FALSE,TRUE, DEF_PAD);
 	vds->adj_random_variation = gtk_adjustment_new (
		vs->random_variation,0.0, 100.0, 1, 1, 0.0);
	wdg = define_scale_in_box(vds->adj_random_variation,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (vds->adj_random_variation), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &vs->random_variation);
	gtk_signal_connect (GTK_OBJECT (vds->adj_random_variation), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);
		
	// We need a row of buttons for controlling the scale (multiple or not)
	
	hbox1 = gtk_hbox_new(FALSE,DEF_PAD);
	define_label_in_box("Scale", hbox1, FALSE,TRUE, DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox1));
	define_radio_button_in_box_with_data (hbox1, &group1, "1X", set_scale_1x, &vs->scale, (vs->scale==SCALE_1X)) ;
	define_radio_button_in_box_with_data (hbox1, &group1, "1X+2X", set_scale_1x_2x, &vs->scale, (vs->scale==SCALE_1X_2X)) ;
	define_radio_button_in_box_with_data (hbox1, &group1, "1X+2X+4X", set_scale_1x_2x_4x, &vs->scale, (vs->scale==SCALE_1X_2X_4X)) ;
	
	button = 
	define_radio_button_in_box_with_data (vbox2, &group, "Uniform / random",
		set_uniform_distrib, &vs->distribution_type,
		(vs->distribution_type==UNIFORM)) ;
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) hide_if_true, (gpointer) hbox);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) show_if_true, (gpointer) hbox1);
	button = 
	define_radio_button_in_box_with_data (vbox2, &group, "Centered / random",
		set_centered_distrib, &vs->distribution_type,
		(vs->distribution_type==CENTERED)) ;
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) hide_if_true, (gpointer) hbox);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) show_if_true, (gpointer) hbox1);
	button = 
	define_radio_button_in_box_with_data (vbox2, &group, "Regular / perturbated",
		set_regular_distrib, &vs->distribution_type,
		(vs->distribution_type==REGULAR)) ;
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) show_if_true, (gpointer) hbox);
	gtk_signal_connect (GTK_OBJECT (button), "clicked", (GtkSignalFunc) hide_if_true, (gpointer) hbox1);
	
	// At the end, We add the random variation scale for REGULAR
	// and the radio buttons for the multiple scale control

  	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);  	gtk_box_pack_start (GTK_BOX (vbox2), hbox1, FALSE, FALSE, 0);
	
	if (vs->distribution_type!=REGULAR)
		gtk_widget_hide(GTK_WIDGET(hbox));
	else
		gtk_widget_hide(GTK_WIDGET(hbox1));
	
	/*****************************************************/
	
	// Now specify how the current HF should be used
	
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox2));
	
	frame2 = define_frame_with_hiding_arrows ("Use of current height field", vbox2, hfw->hf_options->tools_window, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0.5*DEF_PAD);
	
	// Vbox3: "USE AS GUIDE" dialog (for adding noise)
	
	guide_box = gtk_vbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(guide_box));
	
	check_box = define_check_button_in_box ("Gener noise anyway?", guide_box, 0, 0, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_box),vs->gener_noise);
	gtk_signal_connect (GTK_OBJECT(check_box), "toggled",
		GTK_SIGNAL_FUNC(toggle_check_button_callb), (gpointer) &vs->gener_noise);
	
	// Preview box
	preview_box = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(preview_box));
	
	vds->noise_preview =
		view_struct_new(NOISE_PREVIEW_SIZE,NOISE_PREVIEW_SIZE,(gpointer) compute_noise_preview,(gpointer)  hfw->hf_options->img);
	gtk_box_pack_start(GTK_BOX(preview_box), vds->noise_preview->area, TRUE, TRUE, DEF_PAD *0.5);
	
	vbox3 =  seed_dialog_new_lbl(hfw->hf_options->img, vs->noise_opt->seed, change_noise_seed, FALSE, VERTICAL_BOX );
	
	adj = gtk_adjustment_new (0, -4, 4, 1, 1, 0.1);
	
    	scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
	gtk_widget_set_usize(scale,75,0);
	gtk_scale_set_digits (GTK_SCALE (scale), 0); 
	gtk_box_pack_start (GTK_BOX (vbox3), scale, TRUE, TRUE, DEF_PAD*0.5); 
	gtk_widget_show (scale);
	
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(gint_adj_callb), &vs->noise_opt->roughness);
	gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		GTK_SIGNAL_FUNC(draw_area_callb), vds->noise_preview);
		
	gtk_box_pack_start(GTK_BOX(preview_box), vbox3, TRUE, TRUE, DEF_PAD*0.5);
	
	gtk_box_pack_start(GTK_BOX(guide_box), preview_box, FALSE, FALSE, 0.5*DEF_PAD);
	
	gtk_signal_connect (GTK_OBJECT (check_box), "toggled",
	       (GtkSignalFunc) show_if_true, (gpointer) preview_box);
	       	
	if (vs->gener_noise==FALSE)
		gtk_widget_hide(GTK_WIDGET(preview_box));
		
	// Scale for the noise level
	
	hbox2 = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox2));

	define_label_in_box("Noise level", hbox2, FALSE,TRUE, DEF_PAD);
 	vds->adj_noise_level = gtk_adjustment_new (
		vs->noise_level,0.0, 100.0, 1, 1, 0.0);
	wdg = define_scale_in_box(vds->adj_noise_level,hbox2,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (vds->adj_noise_level), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &vs->noise_level);
	gtk_signal_connect (GTK_OBJECT (vds->adj_noise_level), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated); 
		
	gtk_signal_connect (GTK_OBJECT (check_box), "toggled",
	       (GtkSignalFunc) show_if_true, (gpointer) hbox2);
      
	//	hbox for NOISE vs GUIDE radio buttons
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));
	
	button = define_radio_button_in_box_with_data (hbox, &group2, "As noise",
		set_use_as_noise, &vs->hf_use, (vs->hf_use==USE_AS_NOISE)) ;
	gtk_signal_connect (GTK_OBJECT (button), "toggled",
	       (GtkSignalFunc) hide_if_true, (gpointer) check_box);
	// Always show the noise scale in that case
	gtk_signal_connect_object(GTK_OBJECT(button), "toggled",
		GTK_SIGNAL_FUNC(gtk_widget_show), (gpointer) wdg);
	       
	button = define_radio_button_in_box_with_data (hbox, &group2, "As guide",
		set_use_as_guide, &vs->hf_use, (vs->hf_use==USE_AS_GUIDE)) ;
	gtk_signal_connect (GTK_OBJECT (button), "toggled",
	       (GtkSignalFunc) show_if_true, (gpointer) guide_box);
	
	if (vs->hf_use==USE_AS_NOISE)
		gtk_widget_hide(GTK_WIDGET(guide_box));
       
  	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0); 
	
	gtk_box_pack_start (GTK_BOX (vbox2), guide_box, FALSE, FALSE, 0);     
	
	gtk_box_pack_start (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);
	
	/*********************** Crack width **************************/
		
	vbox2 = gtk_vbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(vbox2));
	
	frame2 = define_frame_with_hiding_arrows ("Crack width", vbox2, hfw->hf_options->tools_window, FALSE);
	
	gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0.5*DEF_PAD);
			
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));
	
	button_uniform = define_radio_button_in_box_with_data (hbox, &group3, "Uniform", set_fixed_width, &vs->crack_width_type, vs->crack_width_type==FIXED_WIDTH) ;
	       
	button = define_radio_button_in_box_with_data (hbox, &group3, "From distance",set_from_distance, &vs->crack_width_type, vs->crack_width_type==FROM_DISTANCE) ;

	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
	
	// Width: in pixels for "Uniform", relative for "Varies with distance"
			
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Min", hbox, FALSE,TRUE, DEF_PAD);
 	vds->adj_min_width = gtk_adjustment_new (
		vs->min_width,0.0, 10.0, 1, 1, 0.0);
	wdg = define_scale_in_box(vds->adj_min_width,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (vds->adj_min_width), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &vs->min_width);
	gtk_signal_connect (GTK_OBJECT (vds->adj_min_width), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);
		  	
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
		       
	gtk_signal_connect (GTK_OBJECT (button_uniform), "toggled",
	       (GtkSignalFunc) hide_if_true, (gpointer) hbox);
	if (vs->crack_width_type==FIXED_WIDTH)
		gtk_widget_hide(GTK_WIDGET(hbox));
		
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	lblmax = define_label_in_box("Max", hbox, FALSE,TRUE, DEF_PAD);	
	lblwidth = define_label_in_box("Width", hbox, FALSE,TRUE, DEF_PAD);
 	vds->adj_max_width = gtk_adjustment_new (
		vs->max_width,0.0, 10.0, 1, 1, 0.0);
	wdg = define_scale_in_box(vds->adj_max_width,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (vds->adj_max_width), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &vs->max_width);
	gtk_signal_connect (GTK_OBJECT (vds->adj_max_width), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);
  	
	// Consistency controls (min should be <= max)
	
	gtk_signal_connect (GTK_OBJECT (vds->adj_min_width), "value_changed", GTK_SIGNAL_FUNC (check_max_width), (gpointer) hfw->hf_options->img);
	gtk_signal_connect (GTK_OBJECT (vds->adj_max_width), "value_changed", GTK_SIGNAL_FUNC (check_min_width), (gpointer) hfw->hf_options->img);
	
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, FALSE, FALSE, 0);
		       
	gtk_signal_connect (GTK_OBJECT (button_uniform), "toggled",
	       (GtkSignalFunc) hide_if_true, (gpointer) lblmax);		       
	gtk_signal_connect (GTK_OBJECT (button_uniform), "toggled",
	       (GtkSignalFunc) show_if_true, (gpointer) lblwidth);
	       
	if (vs->crack_width_type==FIXED_WIDTH)
		gtk_widget_hide(GTK_WIDGET(lblmax));
	
	button = gtk_button_new_with_label (_("Apply"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) apply_voronoi_callb, data);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(vbox), align_widget(button,0.5,0.5), FALSE, FALSE, DEF_PAD);

	/************************ Edge level control ************************/
	
	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box("Edge raising", hbox, FALSE,TRUE, DEF_PAD);
 	vds->adj_edges_level = gtk_adjustment_new (
		vs->edges_level,0.0, 100.0, 1, 1, 0.0);
	vds->wdg_edges_level = define_scale_in_box(vds->adj_edges_level,hbox,0,DEF_PAD);
	gtk_signal_connect (GTK_OBJECT (vds->adj_edges_level), "value_changed", GTK_SIGNAL_FUNC (gint_adj_callb), (gpointer) &vs->edges_level);
	gtk_signal_connect (GTK_OBJECT (vds->adj_edges_level), "value_changed", GTK_SIGNAL_FUNC (edges_level_callb), data);
	gtk_signal_connect (GTK_OBJECT (vds->adj_edges_level), "value_changed", GTK_SIGNAL_FUNC (gboolean_set_false), (gpointer) &hfw->if_calculated);
  	
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	// We activate when apply is clicked
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) set_sensitive_callb, (gpointer) vds->wdg_edges_level);
  
	gtk_widget_set_sensitive (GTK_WIDGET(vds->wdg_edges_level),FALSE);
	// Controls
		
	gtk_box_pack_start (GTK_BOX (vbox), reset_accept_buttons_new (data, &vds->accept), FALSE, FALSE, DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	return frame;
}

void voronoi_callb(GtkWidget *wdg, gpointer data) {
	hf_wrapper_struct *hfw;
	voronoi_dialog_struct *vds;
	hfw = (hf_wrapper_struct *) * (hf_wrapper_struct **) data;
	
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg))) {
		accept_callb(wdg,data);
		return;
	}
	
	vds = hfw->hf_options->img->voronoi_dialog;
	if (!vds->dialog) {
		vds->dialog = voronoi_dialog_new(data);
		gtk_container_add(GTK_CONTAINER(hfw->hf_options->img->img_dialog),
			vds->dialog );
	}
	if (hfw->hf_options->img->current_subdialog)
		gtk_widget_hide(hfw->hf_options->img->current_subdialog);
	hfw->hf_options->img->current_subdialog = vds->dialog;
	hfw->hf_options->img->set_fn = (gpointer) set_voronoi_defaults;
	hfw->hf_options->img->accept_wdg = vds->accept;
	gtk_widget_show(hfw->hf_options->img->current_subdialog);
}
