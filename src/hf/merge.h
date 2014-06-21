 /* merge.h - Merge utility headers (merging source - result)
 *
 * Copyright (C) 2002, 2010 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
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

#ifndef _MERGE
#define _MERGE 1
#include "globals.h"
#include "hf.h"

#ifndef ADD
	#define ADD 91
#endif
#ifndef SUBTRACT
	#define SUBTRACT 92
#endif
#ifndef MIN_MERGE
	#define MIN_MERGE 93
#endif
#ifndef MAX_MERGE
	#define MAX_MERGE 94
#endif
#ifndef XOR_MERGE
	#define XOR_MERGE 95
#endif

#ifndef ALT_MERGE
	#define ALT_MERGE 96
#endif

#ifndef MERGE_HARDNESS
	#define MERGE_HARDNESS 0.15
#endif

typedef struct {
	gint merge_op;
// Post-merge function, typically for displaying the result, but can include other computations
	void (*display_fn) (gpointer); 
// Data to execute with display_fn, typically a hf_wrapper_struct **
	gpointer data;
	gpointer source1;
	gpointer source2;
	gpointer result;
	gint max_x;
	gint max_y;
	gint mix;
	gint source_offset;
	gint result_offset;
	gint x_translate;
	gint y_translate;		// TRUE when one of the parameters change
	gdouble hardness;		// For altitude merge only (added 2010-11)
} merge_struct;

void set_merge_buffers (
	merge_struct *mrg,
	gpointer source1, 
	gpointer source2, 
	gpointer result, 
	gint max_x, 
	gint max_y);

void reset_merge_buffers (merge_struct *mrg);

merge_struct *merge_struct_new(
	gpointer hfw_adr,
	void (*calc_hf) (gpointer));

void merge_calc (merge_struct *mrg);
	
void simple_merge  (merge_struct *mrg) ;

#endif // _MERGE
