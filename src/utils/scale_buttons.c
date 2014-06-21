/* scale_buttons.c - scale buttons class
 *
 * Copyright (C) 2007 Patrice St-Gelais
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
#include <string.h>
#include "scale_buttons.h"
#include "utils.h"
#include "x_alloc.h"

void toggle_scale_button (scale_buttons_struct *sbs) {
//	Toggle the appropriate radio button from the current display scale
	gchar *lbl, *txt;
	GSList *node;
	if (!sbs)
		return;
//	Find label from scale:
	switch (sbs->current) {
		case 0:
			lbl = SCALE_100;
			break;
		case 1:
			lbl = SCALE_50;
			break;
		case 2:
			lbl = SCALE_25;
			break;
		case 3:
			lbl = SCALE_12_5;
			break;
		case -1:
			lbl = SCALE_200;
			break;
		case -2:
			lbl = SCALE_400;
			break;
		case -3:
			lbl = SCALE_800;
	}
	if (!sbs->group)
		return;
	for (node = sbs->group; node; node = node->next) {
		gtk_label_get(GTK_LABEL(GTK_BIN(node->data)->child), &txt);
		if (!strcmp(txt,lbl))
			break;
	}
	if (node) {
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(node->data), TRUE);
	}
}

void set_display_scale_from_label(GtkWidget *wdg, gpointer data) {
//	Sets the display scale variable from the label of a radio button
//	Scales on the buttons are in %
//	Internally, they are a binary shift (>>)
	gchar *txt;
	//GtkAdjustment *vadj,*hadj;
	//GtkRequisition rq;
	scale_buttons_struct *sbs;
	sbs = (scale_buttons_struct *) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wdg)))
		return;
//	printf("HF_SET_DISPLAY_SCALE\n");
	gtk_label_get(GTK_LABEL(GTK_BIN(wdg)->child), &txt);
	if (!strcmp(SCALE_100, txt))
		sbs->current = 0;
	else
	if (!strcmp(SCALE_50, txt))
		sbs->current = 1;
	else
	if (!strcmp(SCALE_25, txt))
		sbs->current = 2;
	else
	if (!strcmp(SCALE_12_5, txt))
		sbs->current = 3;
	else
	if (!strcmp(SCALE_200, txt))
		sbs->current = -1;
	else
	if (!strcmp(SCALE_400, txt))
		sbs->current = -2;
	else
	if (!strcmp(SCALE_800, txt))
		sbs->current = -3;

}

scale_buttons_struct *scale_buttons_new_with_callback (GtkOrientation orient, gint lower, gint upper, gpointer callback_fn, gpointer external_data) {
	GtkWidget *label, *button;
	scale_buttons_struct *sbs;
	sbs = (scale_buttons_struct *) x_calloc(sizeof(scale_buttons_struct),1, "scale_buttons_struct");
	if (orient==GTK_ORIENTATION_HORIZONTAL)
		sbs->box = gtk_hbox_new (FALSE, 0);
	else
		sbs->box = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (sbs->box);
	sbs->group=NULL;
	sbs->lower = lower;
	sbs->upper = upper;
	sbs->current = 0;
	label = gtk_label_new(_("Scale % "));
	gtk_widget_show (label);
	gtk_box_pack_start(GTK_BOX(sbs->box), label, TRUE, TRUE, DEF_PAD*0.5);
	if (lower>=SCALE_ID_12_5) {
		button = define_radio_button_in_box_with_data (sbs->box, &sbs->group, 
			SCALE_12_5, set_display_scale_from_label, sbs, FALSE);
		if (callback_fn && external_data)
			gtk_signal_connect ( GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(callback_fn),
				external_data);
	}
	if ((lower>=SCALE_ID_25)&& (upper<=SCALE_ID_25)) {
		button = define_radio_button_in_box_with_data (sbs->box, &sbs->group,
			SCALE_25, set_display_scale_from_label, sbs, FALSE) ;
		if (callback_fn && external_data)
			gtk_signal_connect ( GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(callback_fn),
				external_data);
	}
	if ((lower>=SCALE_ID_50) && (upper<=SCALE_ID_50)) {
		button = define_radio_button_in_box_with_data (sbs->box, &sbs->group, 
			SCALE_50, set_display_scale_from_label, sbs, FALSE) ;
		if (callback_fn && external_data)
			gtk_signal_connect ( GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(callback_fn),
				external_data);
	}
	if ((lower>=SCALE_ID_100) && (upper<=SCALE_ID_100)) {
		button = define_radio_button_in_box_with_data (sbs->box, &sbs->group, 
			SCALE_100, set_display_scale_from_label, sbs, TRUE) ;
		if (callback_fn && external_data)
			gtk_signal_connect ( GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(callback_fn),
				external_data);
	}
	if ((lower>=SCALE_ID_200) && (upper<=SCALE_ID_200)) {
		button = define_radio_button_in_box_with_data (sbs->box, &sbs->group, 
			SCALE_200, set_display_scale_from_label, sbs, FALSE) ;
		if (callback_fn && external_data)
			gtk_signal_connect ( GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(callback_fn),
				external_data);
	}
	if ((lower>=SCALE_ID_400) && (upper<=SCALE_ID_400)) {
		button = define_radio_button_in_box_with_data (sbs->box, &sbs->group,
			SCALE_400, set_display_scale_from_label, sbs, FALSE) ;
		if (callback_fn && external_data)
			gtk_signal_connect ( GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(callback_fn),
				external_data);
	}
	if ((lower>=SCALE_ID_800) && (upper<=SCALE_ID_800)) {
		button = define_radio_button_in_box_with_data (sbs->box, &sbs->group, 
			SCALE_800, set_display_scale_from_label, sbs, FALSE) ;
		if (callback_fn && external_data)
			gtk_signal_connect ( GTK_OBJECT (button), 
				"toggled", 
				GTK_SIGNAL_FUNC(callback_fn),
				external_data);
	}
	return sbs;
}

scale_buttons_struct *scale_buttons_new (GtkOrientation orient, gint lower, gint upper) {
	return scale_buttons_new_with_callback (orient, lower, upper, NULL, NULL);
}

void scale_buttons_free (scale_buttons_struct *sbs) {
	if (sbs)
		free (sbs);
}

void set_current_button (scale_buttons_struct *sbs, gint current) {
	// UPPER is negative, LOWER is positive...
	if (!sbs)
		return;
	sbs->current = MAX(sbs->upper,MIN(sbs->lower,current));
	toggle_scale_button (sbs);
}
