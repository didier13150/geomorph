/* dialog_utilities.c:  various dialog utilities for hf management
 *
 * Copyright (C) 2003-2004 Patrice St-Gelais
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

#include "dialog_utilities.h"
#include "hf.h"
#include "hf_calc.h"
#include "globals.h"
#include "gl_preview.h"

//	Icons for merge mode
#include "../icons/raise.xpm"
#include "../icons/dig.xpm"
#include "../icons/smooth.xpm"

gint optimize_on_mouse_click (GtkWidget *scale, gpointer data) {
	// Optimization
	// As of 2005-03, we only postpone the MesaGL preview drawing
	// up to the mouse button release
	// Future options: calculate only in the display buffer (for huge HF),
	// then apply the whole calculation on mouse release
	gl_preview_optimize (scale, data);
	return FALSE;
}

void optimize_on_mouse_release (GtkWidget *scale, gboolean *refresh_on_release) {
	g_signal_connect (G_OBJECT (scale), "button_press_event",
			(GtkSignalFunc) gboolean_set_false_event, refresh_on_release);
	g_signal_connect (G_OBJECT (scale), "button_release_event",
			(GtkSignalFunc) gboolean_set_true_event, refresh_on_release);
}

GtkWidget *define_frame_with_hiding_arrows (gchar *label, GtkWidget *embedded_widget, GtkWidget *parent_window, gboolean display_on_start) {
	
	GtkWidget *frame, *hbox;
	
	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);
	define_label_in_box (label, hbox,FALSE, FALSE, DEF_PAD);
	gtk_box_pack_start( GTK_BOX(hbox), align_widget (hideshow_dialog_new(parent_window, GTK_ORIENTATION_HORIZONTAL, embedded_widget,NULL), 1.0, 0.5), TRUE, TRUE, 0);
	gtk_frame_set_label_widget (GTK_FRAME(frame), hbox);
	gtk_frame_set_label_align(GTK_FRAME(frame),0,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), DEF_PAD*0.5);
	
	gtk_container_add(GTK_CONTAINER(frame),embedded_widget);
	
	if (!display_on_start)
		gtk_widget_hide(embedded_widget);
	
	return frame;
}

GtkWidget *define_options_frame_with_hiding_arrows (gchar *label, GtkWidget *embedded_widget, GtkWidget *parent_window, gboolean display_on_start) {
	
	GtkWidget *frame, *hbox;
	gchar *buf,*lbl;
	
	lbl = _(label);
	buf = (gchar *) x_malloc(2+strlen(_("Options for"))+strlen(lbl), "const gchar (buf in define_frame_with_hiding_arrows)");
	strcpy(buf,_("Options for"));
	strcat(buf," ");
	strcat(buf,lbl);
	
	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox);
	define_label_in_box (buf, hbox,FALSE, FALSE, DEF_PAD);
	gtk_box_pack_start( GTK_BOX(hbox), align_widget (hideshow_dialog_new(parent_window, GTK_ORIENTATION_HORIZONTAL, embedded_widget,NULL), 1.0, 0.5), TRUE, TRUE, 0);
	gtk_frame_set_label_widget (GTK_FRAME(frame), hbox);
	gtk_frame_set_label_align(GTK_FRAME(frame),0,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), DEF_PAD);
	
	gtk_container_add(GTK_CONTAINER(frame),embedded_widget);
	
	if (!display_on_start)
		gtk_widget_hide(embedded_widget);
	return frame;
}

GtkWidget *options_frame_new(gchar *label) {
//	Creates an empty frame with text preceded by "Options for", 
//	ready for adding options dialog 
	GtkWidget *frame;
	gchar *buf,*lbl;

	lbl = _(label);
	buf = (gchar *) x_malloc(2+strlen(_("Options for"))+strlen(lbl), "const gchar (buf in options_frame_new)");
	strcpy(buf,_("Options for"));
	strcat(buf," ");
	strcat(buf,lbl);
	frame = gtk_frame_new(buf);
	gtk_widget_show(frame);
	gtk_frame_set_label_align(GTK_FRAME(frame),0,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), DEF_PAD);

	return frame;
}

GtkWidget *options_frame_new_pad(gchar *label, gint pad) {
//	Creates an empty frame with text preceded by "Options for", 
//	ready for adding options dialog
	GtkWidget *frame;
	gchar *buf,*lbl;

	lbl = _(label);
	buf = (gchar *) x_malloc(2+strlen(_("Options for"))+strlen(lbl), "const gchar (buf in options_frame_new_pad)");
	strcpy(buf,_("Options for"));
	strcat(buf," ");
	strcat(buf,lbl);
	frame = gtk_frame_new(buf);
	gtk_widget_show(frame);
	gtk_frame_set_label_align(GTK_FRAME(frame),0,0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), pad);

	return frame;
}

/* "Change seed" function model
   Usually provided by the calling app, and passed as argument to seed_dialog_new
gint change_seed(GtkWidget *entry, gpointer data) {
	if (!data)
		return FALSE;
	*((gint*)data) =(gint) atoi((char *)gtk_entry_get_text(GTK_ENTRY(entry)));
//	printf("SEED: %d\n",*((gint*)data));
	return FALSE;
}
*/

gint refresh_seed(GtkWidget *button, gpointer entry) {
	gchar *buf;
	buf = (gchar *) x_malloc(11, "const gchar (buf in refresh_seed)");
	sprintf(buf,"%10d",rand());
//	printf("New seed: %s\n",buf);
	//	Changing the entry here calls the entry call back
	gtk_entry_set_text(GTK_ENTRY(entry),buf);
	return TRUE;
}

gint refresh_seed_from_seedbox (GtkWidget *wdg, gpointer hbox) {
	GList *list;
//	The hbox contains one entry, which is the field to update
	for (list = gtk_container_children(GTK_CONTAINER(hbox)); list != NULL; list = list->next) {
  		if (GTK_IS_ENTRY(GTK_OBJECT(list->data))) {
			refresh_seed (wdg, (gpointer) list->data);
		}
	}
	return TRUE;
}

GtkWidget *seed_dialog_new_lbl(gpointer data, gint first_seed,
	gint (*change_seed) (GtkWidget *, gpointer), gboolean if_lbl, gint box  ) {
	
//	Creates a generic random seed control dialog, in a hbox
//	The function "change_seed" deals with the update of the variable
//	containing the seed and computing following the update;
//	it must be provided, because seed_dialog_new doesn't know
//	the different structures where the seed variable appears

	GtkWidget *hbox, *button, *wtmp;
	gchar *buf;

	if (box==HORIZONTAL_BOX) 
		hbox = gtk_hbox_new(FALSE,0);
	else
		hbox = gtk_vbox_new(FALSE,0);
		
	gtk_widget_show(GTK_WIDGET(hbox));
	if (if_lbl)
		define_label_in_box("Seed", hbox, FALSE, FALSE, DEF_PAD);

	button = gtk_button_new_with_label (_("Refresh"));
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, DEF_PAD);
	
	buf = (gchar *) x_malloc(11, "const gchar (buf in seed_dialog_new)");
	sprintf(buf,"%10d",first_seed);
	wtmp = define_entry_in_box(buf, hbox, TRUE, TRUE, DEF_PAD, 11, 11);

	if (change_seed)
		gtk_signal_connect (GTK_OBJECT (wtmp), "changed",
			       (GtkSignalFunc) change_seed, data);
			       
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			       (GtkSignalFunc) refresh_seed, wtmp);

	return hbox;	
}

GtkWidget *seed_dialog_new(gpointer data, gint first_seed,
		gint (*change_seed) (GtkWidget *, gpointer) ) {
	return seed_dialog_new_lbl (data, first_seed, change_seed, TRUE, HORIZONTAL_BOX);
}

static gint plus_30 (GtkWidget *wdg, gpointer data) {
	gint value;
	value = (gint) GTK_ADJUSTMENT(data)->value;
	value = (value + 30) % 360;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
	return FALSE;
}
static gint plus_45 (GtkWidget *wdg, gpointer data) {
	gint value;
	value = (gint) GTK_ADJUSTMENT(data)->value;
	value = (value + 45) % 360;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
	return FALSE;
}
static gint plus_90 (GtkWidget *wdg, gpointer data) {
	gint value;
	value = (gint) GTK_ADJUSTMENT(data)->value;
	value = (value + 90) % 360;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
	return FALSE;
}

void rotate_buttons_new(GtkWidget *box, gpointer adj) {
//	Returns +30, +60, +90 degrees buttons in the given hbox or vbox
//	The buttons activate an adjustment, which is passed to the callbacks functions

	GtkWidget *button;
	button = gtk_button_new_with_label ("+30");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_30, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("+45");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_45, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("+90");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_90, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
}

static gint plus_1_unbounded (GtkWidget *wdg, gpointer data) {
	// No limit test (the adjustement is automatically limited to its 
	// maximum value by GTK)
	gint value;
	value = (gint) GTK_ADJUSTMENT(data)->value;
	value += 1;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
	return FALSE;
}

static gint minus_1_unbounded (GtkWidget *wdg, gpointer data) {
	gint value;
	value = (gint) GTK_ADJUSTMENT(data)->value;
	value -= 1;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
	return FALSE;
}

void plus_minus_1_buttons (GtkObject *adj, GtkWidget *box) {
	// Adds +/- buttons to a scale adjustement
	// For using when the scale is slow because it controls a heavy process
	
	GtkWidget *button;
	button = gtk_button_new_with_label ("+");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_1_unbounded, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("-");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) minus_1_unbounded, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
}

static gint plus_1 (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), MIN(GTK_ADJUSTMENT(data)->upper,GTK_ADJUSTMENT(data)->value + 1.0));
	return FALSE;
}

static gint plus_10 (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), MIN(GTK_ADJUSTMENT(data)->upper,GTK_ADJUSTMENT(data)->value + 10.0));
	return FALSE;
}
static gint minus_1 (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), MAX(GTK_ADJUSTMENT(data)->lower,GTK_ADJUSTMENT(data)->value - 1.0));
	return FALSE;
}

static gint minus_10 (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), MAX(GTK_ADJUSTMENT(data)->lower,GTK_ADJUSTMENT(data)->value - 10.0));
	return FALSE;
}

void percent_buttons_new_with_callb (GtkWidget *box, gpointer adj, void (*callb_fn) (GtkWidget *, gpointer), gpointer data) {
//	Returns 0, +1, -1, +10, -10 buttons in the given box
//	Same concept as rotate_buttons_new, but for percent values instead of degrees
//	(or any range bounded by 0 and 100)
	GtkWidget *button;

	button = gtk_button_new_with_label ("0");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) zero_callb, adj );
	if (callb_fn && data)
		g_signal_connect (G_OBJECT (button), "clicked", GTK_SIGNAL_FUNC(callb_fn), (gpointer) data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("+1");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_1, adj );
	if (callb_fn && data)
		g_signal_connect (G_OBJECT (button), "clicked", GTK_SIGNAL_FUNC(callb_fn), (gpointer) data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("-1");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) minus_1, adj );
	if (callb_fn && data)
		g_signal_connect (G_OBJECT (button), "clicked", GTK_SIGNAL_FUNC(callb_fn), (gpointer) data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
		
	button = gtk_button_new_with_label ("+10");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_10, adj );
	if (callb_fn && data)
		g_signal_connect (G_OBJECT (button), "clicked", GTK_SIGNAL_FUNC(callb_fn), (gpointer) data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("-10");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) minus_10, adj );
	if (callb_fn && data)
		g_signal_connect (G_OBJECT (button), "clicked", GTK_SIGNAL_FUNC(callb_fn), (gpointer) data);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

}

void percent_buttons_new (GtkWidget *box, gpointer adj) {
	percent_buttons_new_with_callb (box, adj, NULL, NULL);
}

static gint minus_1_neg (GtkWidget *wdg, gpointer data) {
	gint value;
	value = (gint) GTK_ADJUSTMENT(data)->value;
	value = MAX(-100,value - 1);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
	return FALSE;
}

static gint minus_10_neg (GtkWidget *wdg, gpointer data) {
	gint value;
	value = (gint) GTK_ADJUSTMENT(data)->value;
	value = MAX(-100,value - 10);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
	return FALSE;
}

void percent_buttons_negative_new (GtkWidget *box, gpointer adj) {
//	Returns 0, +1, -1, +10, -10 buttons in the given box
//	Range bounded by -100 and +100
//	Same concept as rotate_buttons_new, but for percent values instead of degrees
//	(or any range bounded by -100 and 100)

	GtkWidget *button;

	button = gtk_button_new_with_label ("0");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) zero_callb, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("+1");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_1, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("-1");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) minus_1_neg, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
		
	button = gtk_button_new_with_label ("+10");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_10, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("-10");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) minus_10_neg, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

}
static void zero_percent (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), 0.0);
}

static void hundred_percent (GtkWidget *wdg, gpointer data) {
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), (gfloat) MAX_HF_VALUE);
}

static void plus_10_percent (GtkWidget *wdg, gpointer data) {
	long int value;
	value = (long int) GTK_ADJUSTMENT(data)->value;
	value = (value + (long int) (0.1* (gfloat) MAX_HF_VALUE)) % (long int) MAX_HF_VALUE;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
}

static void plus_25_percent (GtkWidget *wdg, gpointer data) {
	long int value;
	value = (long int) GTK_ADJUSTMENT(data)->value;
	value = (value + (long int) (0.25* (gfloat) MAX_HF_VALUE)) % (long int) MAX_HF_VALUE;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(data), value);
}

void uniform_percent_buttons_new(GtkWidget *box, gpointer adj) {
//	Returns 0, +10, +25 % buttons in the given hbox or vbox
//	The buttons activate an adjustment, which is passed to the callbacks functions
//	It "wraps" (90 + 25 = 15%)

	GtkWidget *button;
	button = gtk_button_new_with_label (" 0 ");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) zero_percent, adj );
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
	button = gtk_button_new_with_label ("+10%");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_10_percent, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
	button = gtk_button_new_with_label ("+25%");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) plus_25_percent, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
	button = gtk_button_new_with_label ("100%");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	       (GtkSignalFunc) hundred_percent, adj);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, DEF_PAD);
}


void set_tiling_auto (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint *) data) = TILING_AUTO;
}
void set_tiling_yes (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint *) data) = TILING_YES;
}
void set_tiling_no (GtkWidget *wdg, gpointer data) {
	if (data)
		*((gint *) data) = TILING_NO;
}

GtkWidget *tiling_control (gpointer data) {
	// Creates a row of radio buttons,
	// allowing to override or not the tiling property of the HF
	// "data" is a gint pointer, whose values can be:
	// TILING_AUTO, TILING_YES, TILING_NO

	GtkWidget *hbox;
	GSList *group = NULL;
	gint tiling;
	tiling = (gint) * (gint *) data;

	hbox = gtk_hbox_new(FALSE,DEF_PAD);
	gtk_widget_show(GTK_WIDGET(hbox));

	define_label_in_box ("Tiling",hbox,FALSE,FALSE,DEF_PAD*0.5);
	define_radio_button_in_box_with_data (hbox, &group, "Auto",
		set_tiling_auto, data, (tiling==TILING_AUTO)) ;
	define_radio_button_in_box_with_data (hbox, &group, "Yes",
		set_tiling_yes, data,  (tiling==TILING_YES)) ;
	define_radio_button_in_box_with_data (hbox, &group, "No",
		set_tiling_no, data, (tiling==TILING_NO)) ;
	return hbox;
}

gint effect_subtract_callb (GtkWidget *button, gpointer data) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		*((gint *) data) = SUBTRACT;
	return FALSE;
}

gint effect_add_callb (GtkWidget *button, gpointer data) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		*((gint *) data) = ADD;
	return FALSE;
}

gint effect_smooth_callb (GtkWidget *button, gpointer data) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	//	Nothing to do if we are just unactivating the button
		*((gint *) data) =  SMOOTH_OP;
	return FALSE;
}

GtkWidget *effect_toolbar (GtkWidget *window, gpointer data) {
	// Creates a toolbar with drawing effect control (raise / subtract / smooth)

	GtkWidget *hbox, *toolbar, *wdg;

	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(GTK_WIDGET(hbox));

//	Effect - merge mode (mountains / valleys / smoothing)

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar),GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),GTK_TOOLBAR_ICONS);
	gtk_widget_show(GTK_WIDGET(toolbar));
	wdg = gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,NULL,_("Raise"),
		_("Raise a mountain"),NULL,
		create_widget_from_xpm(window,raise_xpm),
		GTK_SIGNAL_FUNC(effect_add_callb), data);
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("Dig"),
		_("Dig a valley"),NULL,
		create_widget_from_xpm(window,dig_xpm),
		GTK_SIGNAL_FUNC(effect_subtract_callb), data);
	gtk_toolbar_append_element(GTK_TOOLBAR(toolbar),
		GTK_TOOLBAR_CHILD_RADIOBUTTON,wdg,_("Smooth"),
		_("Smooth"),NULL,
		create_widget_from_xpm(window,smooth_xpm),
		GTK_SIGNAL_FUNC(effect_smooth_callb), data);

	define_label_in_box("Effect",hbox,FALSE,FALSE,DEF_PAD);
	gtk_box_pack_start(GTK_BOX(hbox),toolbar, FALSE, FALSE, DEF_PAD);

	return hbox;
}

control_line_struct *control_line_new (GtkWidget *area,
	gint x0, gint y0, gint x1, gint y1) {
	// A control line has huge dots at each end which
	// can be moved with the mouse (see the "fault" dialog)
	control_line_struct *cl;
	cl = (control_line_struct *) x_malloc(sizeof(control_line_struct), "control_line_struct");
	cl->x0 = x0;
	cl->y0 = y0;
	cl->x1 = x1;
	cl->y1 = y1;
	cl->tolerance = MOUSE_TOLERANCE;
	cl->active = TRUE;
	cl->mouse_on_0 = FALSE;
	cl->mouse_on_1 = FALSE;
	cl->revert_ends = FALSE;
	cl->gc = gdk_gc_new (area->window);
	gdk_gc_copy(cl->gc,area->style->black_gc);
	gdk_gc_set_function (cl->gc, GDK_COPY);
	return cl;
}

void control_line_update (control_line_struct *cl,
	gint x0, gint y0, gint x1, gint y1) {

	if (!cl)
		return;
	cl->x0 = x0;
	cl->y0 = y0;
	cl->x1 = x1;
	cl->y1 = y1;
}

void control_line_update0 (control_line_struct *cl,gint x0, gint y0) {

	if (!cl)
		return;
	cl->x0 = x0;
	cl->y0 = y0;
}

void control_line_update1 (control_line_struct *cl,gint x1, gint y1) {

	if (!cl)
		return;
	cl->x1 = x1;
	cl->y1 = y1;
}

gboolean test_control_line (control_line_struct *cl,gint x, gint y) {

//	Test whether or not (x,y) is "over" (x0,y0) or (x1,y1), given the tolerance
//	Return TRUE if one of the ends is active
//	(x,y) is typically a mouse position, and we test if one end should be dragged

	gint dx, dy, d1;
	dx = ABS(x - cl->x0);
	dy = ABS(y - cl->y0);
	d1 = dx + dy;
	if ( (dx<=cl->tolerance) && (dy<=cl->tolerance))
		cl->mouse_on_0 = TRUE;
	else
		cl->mouse_on_0 = FALSE;

	dx = ABS(x - cl->x1);
	dy = ABS(y - cl->y1);
	if ( (dx<=cl->tolerance) && (dy<=cl->tolerance))
		cl->mouse_on_1 = TRUE;
	else
		cl->mouse_on_1 = FALSE;

	if ( cl->mouse_on_0 && cl->mouse_on_1 ) {
		// When both points are inside the tolerance zone from the mouse cursor,
		// Choose the nearest
		if ( (dx+dy) > d1)
			cl->mouse_on_0 = FALSE;
	}

	return (cl->mouse_on_0 || cl->mouse_on_1);
}

void control_line_activate (control_line_struct *cl) {
	if (!cl)
		return;
	cl->active = TRUE;
}

void control_line_unactivate (control_line_struct *cl) {
	if (!cl)
		return;
	cl->active = FALSE;
}

void control_line_undrag (control_line_struct *cl) {
	if (!cl)
		return;
	cl->mouse_on_0 = FALSE;
	cl->mouse_on_1 = FALSE;
}

void control_line_draw (control_line_struct *cl,
	GdkDrawable *dw, gint xmax, gint ymax) {

	static GdkColor red = {0,0xFFFF,0,0};
	static GdkColormap *cmap = NULL;

	if (!cl)
		return;
	if (!cl->active)
		return;

	gdk_gc_set_function (cl->gc, GDK_COPY);

	if (!cmap) {
		cmap = gdk_colormap_get_system();
	}

	gdk_colormap_alloc_color (cmap,&red, TRUE, TRUE);
	gdk_gc_set_foreground (cl->gc, &red);

	gdk_gc_set_line_attributes (cl->gc, 1,
		GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);

	gdk_draw_line (dw, cl->gc,
		MAX(0,MIN(cl->x0,xmax)),
		MAX(0,MIN(cl->y0,ymax)),
		MAX(0,MIN(cl->x1,xmax)),
		MAX(0,MIN(cl->y1,ymax)) );

	// Draw a 7x7 dot at both ends

	gdk_gc_set_line_attributes (cl->gc, 7,
		GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);

	gdk_draw_line (dw, cl->gc,
		MAX(0,MIN(cl->x0-4,xmax)),
		MAX(0,MIN(cl->y0,ymax)),
		MAX(0,MIN(cl->x0+3,xmax)),
		MAX(0,MIN(cl->y0,ymax)) );

	gdk_draw_line (dw, cl->gc,
		MAX(0,MIN(cl->x1-4,xmax)),
		MAX(0,MIN(cl->y1,ymax)),
		MAX(0,MIN(cl->x1+3,xmax)),
		MAX(0,MIN(cl->y1,ymax)) );
}

