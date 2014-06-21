/* waves_dialog.h:  data structures waves deformation functions dialogs
 *
 * Copyright (C) 2002 Patrice St-Gelais
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

#include "globals.h"

#ifndef _WAVES_DIALOG
#define _WAVES_DIALOG 1

typedef struct {
	GtkWidget *notebook_page;	// A box or table containing the other widgets
	GtkWidget *order_spinner;	// Computing order (operations are not commutative)
	GtkObject *adj_order;
	GtkWidget *shape_toolbar;	// SINE, SQUARE...
	GtkWidget *axis_toolbar; 		// CIRCULAR_WAVE, VERTICAL_WAVE, LATERAL_WAVE
	GtkWidget *angle_slider;		// From 0 to 360 degrees
	GtkObject *adj_angle;
	GtkWidget *period_slider;		// from 0 to 10... actual period = hf_size / pow(2,this period)
	GtkObject *adj_period;
	GtkWidget *amplitude_slider;	// 0 to 100% of max. height (width for LATERAL_WAVE)
	GtkObject *adj_amplitude;
	GtkWidget *phase_slider;		// 0 to 100% of actual period
	GtkObject *adj_phase;
	GtkWidget *randomness_slider;	// 0 to 100% of actual amplitude
	GtkObject *adj_randomness;
} wave_page_struct;

GtkWidget *wave_dialog_new(gpointer data);
void wave_page_free(wave_page_struct *wps);
void waves_callb(GtkWidget *wdg, gpointer data);

#endif // _WAVES_DIALOG



