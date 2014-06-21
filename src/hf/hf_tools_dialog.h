/* hf_tools_dialog.h - headers for height field tools dialog class
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

#ifndef _HF_TOOLS_DIALOG
#define _HF_TOOLS_DIALOG 1

#include <gtk/gtk.h>

//	Context (dialog ID)

#ifndef DRAW_TOOLS_CONTEXT
	#define DRAW_TOOLS_CONTEXT 1
#endif
#ifndef TRANSFORM_CONTEXT
	#define TRANSFORM_CONTEXT 2
#endif
#ifndef FOURIER_CONTEXT
	#define FOURIER_CONTEXT 3
#endif

GtkWidget* hf_tools_dialog_new (GtkWidget *window, GtkTooltips *tooltips,
	gpointer hf_options, gpointer callb_data);

void draw_tools_callb(GtkWidget *,gpointer );
void transform_callb(GtkWidget *, gpointer );
void fourier_callb(GtkWidget *, gpointer );

#endif // _HF_TOOLS_DIALOG
