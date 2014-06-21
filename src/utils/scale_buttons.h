/* scale_buttons.h - headers for scale buttons
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _SCALE_BUTTONS
#define _SCALE_BUTTONS 1

#include <gtk/gtk.h>

//	Scale buttons labels
#ifndef SCALE_12_5
	#define SCALE_12_5 "12,5"
#endif
#ifndef SCALE_25
	#define SCALE_25 "25"
#endif
#ifndef SCALE_50
	#define SCALE_50 "50"
#endif
#ifndef SCALE_100
	#define SCALE_100 "100"
#endif
#ifndef SCALE_200
	#define SCALE_200 "200"
#endif
#ifndef SCALE_400
	#define SCALE_400 "400"
#endif
#ifndef SCALE_800
	#define SCALE_800 "800"
#endif

//	Scale buttons ID
#ifndef SCALE_ID_12_5
	#define SCALE_ID_12_5 3
#endif
#ifndef SCALE_ID_25
	#define SCALE_ID_25 2
#endif
#ifndef SCALE_ID_50
	#define SCALE_ID_50 1
#endif
#ifndef SCALE_ID_100
	#define SCALE_ID_100 0
#endif
#ifndef SCALE_ID_200
	#define SCALE_ID_200 -1
#endif
#ifndef SCALE_ID_400
	#define SCALE_ID_400 -2
#endif
#ifndef SCALE_ID_800
	#define SCALE_ID_800 -3
#endif

#ifndef get_scale
	#define get_scale(scale) scale->current
#endif

typedef struct {
	GtkOrientation orientation;
	GtkWidget *box;
	GSList *group;
	gint lower;
	gint upper;
	gint current;
} scale_buttons_struct;

void toggle_scale_button (scale_buttons_struct *sbs);
void set_display_scale_from_label(GtkWidget *wdg, gpointer data);
void set_current_button (scale_buttons_struct *sbs, gint current);

scale_buttons_struct *scale_buttons_new (GtkOrientation orient, gint lower, gint upper);

scale_buttons_struct *scale_buttons_new_with_callback (GtkOrientation orient, gint lower, gint upper, gpointer callback_fn, gpointer external_data);

void scale_buttons_free (scale_buttons_struct *sbs);

#endif // _SCALE_BUTTONS
