/* fault.h - headers for fault.c - drawing faults in a height field
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <gtk/gtk.h>
#include "hf.h"
#include "hf_calc.h"
#include "line.h"

#ifndef _FAULT
#define _FAULT 1

#ifndef TOP_LEFT
#define TOP_LEFT 0
#endif

#ifndef TOP_RIGHT
#define TOP_RIGHT 1
#endif

#ifndef BOTTOM_RIGHT
#define BOTTOM_RIGHT 2
#endif

#ifndef BOTTOM_LEFT
#define BOTTOM_LEFT 3
#endif

#ifndef LEFT_RIGHT
#define LEFT_RIGHT 4
#endif

#ifndef TOP_BOTTOM
#define TOP_BOTTOM 5
#endif


 #ifndef FAULT_PREVIEW_SIZE
 #define FAULT_PREVIEW_SIZE 100
 #endif

 // The STEP option in compute_fault

 #ifndef FAULT_PREVIEW
 #define FAULT_PREVIEW 0
 #endif

 #ifndef FAULT_FINAL
 #define FAULT_FINAL 1
 #endif

typedef struct {
	fractal_line_struct *fractal_line;
	gint smoothing;	// From 0 to 20 pixels; default 4 (?)
	gint altitude_difference; // -50% to +50%, on 0xFFFF
	gint separation; 	// Separation between the 2 sides of the crack - 0 to 100 pixels
				// Default 0 (it's a fault)
	hf_type *buffer; // Buffer containing the fault line
	gint buf_size;		// Edge size of the buffer ( the buffer is square -
				// it must be >= to the hf size we are drawing in)
	gboolean buffer_reset_required;
	gint mode; // FAULT_PREVIEW or FAULT_FINAL
} fault_struct;

fault_struct *fault_pen_new();

void fault_pen_free (fault_struct *f);

void init_fault_buffer (fault_struct *f, gint hf_size);

void begin_fault (fault_struct *f, hf_struct_type *hf);

void compute_fault (fault_struct *f, hf_struct_type *hf,
	gint x0, gint y0, gint x1, gint y1,
	dist_matrix_struct *dm, gdouble **gauss_list);

void prepare_fault_result_buf (fault_struct *f, hf_struct_type *hf);

#endif // _FAULT
