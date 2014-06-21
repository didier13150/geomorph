/* fill.c:  Filling a HF with the fill bucket / pen
 *
 * Copyright (C) 2003-2005 Patrice St-Gelais
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
#include <stdlib.h>
 #include "hf.h"
 #include "hf_calc.h"
 #include "fill.h"

#define test_fill(value_to_test,v1,v2,select_mode,output_value) ( (value_to_test>=v1) && (value_to_test<v2) && ((select_mode!=SELECT_SUBTRACT) || (output_value>MIN_FILLING_VALUE)) )

// Local structure used as a "railing" to check for cycles in the recursive "fill bucket" algorithm
// ... because I'm not sure the algorithm is 100% safe
// We test only 1024x1024 HF or smaller - 2010-07-14 Not anymore true!
// 4 spans are kept for each Y
// From-To X are encoded with the direction in an unsigned long int value

#define SPAN_LIST_LENGTH 16384 
#define SPAN_MAX 16384
static unsigned long int span_list[SPAN_LIST_LENGTH];
gint span_count=0;

void fill_pen_free (fill_struct *f) {
	if (f) {
		reset_seeds (f);
		x_free (f);
	}
}

fill_struct *fill_pen_new(hf_type default_height, gint default_select_mode, gint default_percent) {
	fill_struct *f;
	f =  (fill_struct *) x_malloc(sizeof(fill_struct), "fill_struct");
	f->height = default_height;
	f->select_mode = default_select_mode;
	f->percent = default_percent; 
	f->seeds_list = NULL;
	f->noise_exponent = 1;
	return f;
}

void add_seed (fill_struct *f, gint x, gint y) {
	seed_coord *s;
//	printf("ADD_SEED: %d, %d\n",x,y);
	if (f->select_mode==SELECT_REPLACE)
		reset_seeds(f);
	s = (seed_coord*) x_malloc (sizeof(seed_coord), "seed_coord");
	s->x = x;
	s->y = y;
	s->select_mode = f->select_mode;
	f->seeds_list = g_list_append (f->seeds_list, (gpointer) s);
//	printf("List length in ADD_SEED: %d\n",g_list_length(f->seeds_list));
//	if (g_list_length(f->seeds_list)>10)
//		exit(0);
}

void reset_seeds (fill_struct *f) {
	// Resets the filling seeds list to a NULL list
	GList *node;
//	printf("RESET_SEEDS; list length: %d\n",g_list_length(f->seeds_list));
	for (node = f->seeds_list; node; node = node->next) {
		x_free((seed_coord *) node->data);
	}
	g_list_free(f->seeds_list);
	f->seeds_list = NULL;
}

unsigned long int encode_span (gint from, gint to, gint direction){
	unsigned long int code = direction; // We assume it's at least 32 bits long (there could be a portability problem here!)
	code = code | (((unsigned long int) from) << 2);
	code = code | (((unsigned long int) from) << 14);
	return code;
};

gboolean write_span (gint y, unsigned long int code) {
	// Returns TRUE if SPAN found, FALSE otherwise
	// If the span bucket is full, overwrite one chosen randomly
	gint i;
	for (i=0; i<4; i++) {
		if (span_list[(y*4)+i]==code) {
			return TRUE;
		}
		if (span_list[(y*4)+i])
			continue;
		span_list[(y*4)+i]=code;
		break;
	}
	if (i==4) { // Bucket full
		span_list[(y*4)+rand()%4]=code;
//		printf("BUCKET FULL for y=%d\n",y);
	}
	span_count++;
	return FALSE;
} ;

void fill_span_from_shadow (hf_type *buffer_in, hf_type *buffer_out, gint xmax, gint ymax,
	hf_type v1, hf_type v2, hf_type filling_value,
	gint y, gint x1, gint x2, gint direction, gint select_mode) {
	
	gint i, mark1, mark2;
	glong value_to_test;
	hf_type output_value;
	gboolean span_end, filled, to_fill;

	filled = FALSE;

//	printf("FILLING y = %d, from x = %d to %d, direction = %d; value: %d\n",y, x1, x2, direction, filling_value);

	if (write_span(y,encode_span(direction,x1,x2))) { 
	
//		printf("FILLING in DOUBLE y = %d, from x = %d to %d, direction = %s; value: %d; mode: %s\n",y, x1, x2, 
//		((direction==0)?"FILL_UP":((direction==1)?"FILL_DOWN":"FILL_BOTH")), filling_value,
//		 ((select_mode==0)?"SELECT_REPLACE":((select_mode==1)?"SELECT_ADD":"SELECT_SUBTRACT")));
		return;
	}
//	Direction: FILL_UP (North), FILL_DOWN (South), FILL_BOTH

	if ( direction==FILL_BOTH ) {

		if ( (y-1) >= 0 )
			fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1,v2,
				filling_value, y, x1, x2, FILL_UP, select_mode);
		if ( (y+1) <= ymax)
			fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1,v2,
				filling_value, y, x1, x2, FILL_DOWN, select_mode);
		return;
	}

	// 1. Calculate direction (FILL_UP or FILL_DOWN)

	if ( direction==FILL_UP )
		y--;
	else
		y++;

	// Test the boundaries in case the function is called
	// at the first recursivity level with direction <> FILL_BOTH
	if ( (y<0) || (y>=ymax) )
		return;

	// 2. Process span

	// 2.1 Backup original span
	mark1 = x1;
	mark2 = x2;

	// 2.2 For x1, if the filling test is true, extend the span as needed

	output_value = *(buffer_out + VECTORIZE(x1,y,xmax));
	if ( (buffer_out==buffer_in) || (select_mode==SELECT_SUBTRACT))
		value_to_test  = *(buffer_in + VECTORIZE(x1,y,xmax));
	else // select_mode==SELECT_ADD / REPLACE || buffer_in != buffer_out
		value_to_test = ((glong) *(buffer_in + VECTORIZE(x1,y,xmax))) + ((glong) output_value);

	if ( test_fill (value_to_test, v1, v2, select_mode, output_value) ) {
	
		// Filling test is true, we extend the span backwards, filling pixels
		for (mark1 = x1-1; mark1>=0; mark1--) {
				
			output_value = *(buffer_out + VECTORIZE(mark1,y,xmax));
			if ( (buffer_out==buffer_in) || (select_mode==SELECT_SUBTRACT))
				value_to_test  = *(buffer_in + VECTORIZE(mark1,y,xmax));
			else // select_mode==SELECT_ADD / REPLACE || buffer_in != buffer_out
				value_to_test = ((glong) *(buffer_in + VECTORIZE(mark1,y,xmax))) + ((glong) output_value);
				
			// Stop when the filling test is false
			if (! test_fill (value_to_test, v1, v2, select_mode, output_value)){
				break;
			}
			else
				*(buffer_out + VECTORIZE(mark1,y,xmax)) = filling_value;
		}
		mark1++;
	}
	else {
	//	If the filling test is false, narrow the span as needed
		while ( mark1<=mark2 ) {
			mark1++;
			
			output_value = 	*(buffer_out + VECTORIZE(mark1,y,xmax));	
			if ( (buffer_out==buffer_in) || (select_mode==SELECT_SUBTRACT))
				value_to_test  = *(buffer_in + VECTORIZE(mark1,y,xmax));
			else // select_mode==SELECT_ADD / REPLACE || buffer_in != buffer_out
				value_to_test = ((glong) *(buffer_in + VECTORIZE(mark1,y,xmax))) + ((glong) output_value);
			
			if ( test_fill (value_to_test, v1, v2, select_mode, output_value) ) 
				break;
		}
	}

	if (mark1>mark2)
		return;

	// Run from mark1 through mark2, filling pixels and calling recursively fill_from_shadow
	// for the current span, each time a discontinuity is encountered

	// At this point, the filling test is true for mark1
	// If mark1 < x1, then the mark1-x1 range is already filled, including x1

	span_end = FALSE;

	// 	We don't need to fill pixels before x1,
 	//	but we need mark1 to specify the shadow for the span on the next y
	i = MAX(x1,mark1);

	// We need to fill the shadow between x1 and mark1, if mark1<(x1-1)
	
	if (mark1<(x1-1))
		fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1,v2, filling_value, y, mark1, x1, ((direction==FILL_UP) ? FILL_DOWN : FILL_UP ), select_mode );
	
	to_fill = TRUE;
	while (i<=mark2) {
//		count++;
//		if (i==384)
//		printf("Count: %d; I: %d; Mark1: %d; Mark2: %d\n",count, i, mark1, mark2);
		if ( to_fill ) {
			*(buffer_out + VECTORIZE(i,y,xmax)) = filling_value;
			if (span_end) // begin a new span
				mark1 = i;
			span_end = FALSE;
			filled = TRUE;
		}
		else {
			// The first time the fill condition is false, a span has ended,
			// we must call "fill_from_shadow" for this span
			// After, we're looking for the beginning of a new span
			if ( (!span_end) && (i>x1)) {
				fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1, v2, filling_value, y, mark1, i-1, direction, select_mode);
				// If the new span boundary exceeds the old one by more than one pixel,
				// we start a filling movement on the other direction
				if ( (i-1) > x2 )
					fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1,v2, filling_value, y, x2+1, i-1,
					((direction==FILL_UP) ? FILL_DOWN : FILL_UP ), select_mode );
				span_end = TRUE;
				filled = TRUE;
			}
			mark1 = i+1;
			if (mark1>=x2)
				break;
		}
		i++;
		if (i==xmax)
			break;

		output_value = *(buffer_out + VECTORIZE(i,y,xmax));
		if ( (buffer_out==buffer_in) || (select_mode==SELECT_SUBTRACT))
			value_to_test  = *(buffer_in + VECTORIZE(i,y,xmax));
		else // select_mode==SELECT_ADD / REPLACE || buffer_in != buffer_out
			value_to_test = ((glong) *(buffer_in + VECTORIZE(i,y,xmax))) + ((glong) output_value);
					
		if ( test_fill (value_to_test, v1, v2, select_mode, output_value) ) {	
			to_fill = TRUE;
			if (i>x2) {
				mark2++; // We extend the span as necessary
				if (mark2==xmax) {
					mark2--;
					break;
				}
			}
		}
		else
			to_fill = FALSE;
	}

	// "Flush" the last span if required

	if (filled && ((i>mark2) || (i==xmax-1))) {
		fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1,v2,
			filling_value, y, mark1, mark2, direction, select_mode);

			// If the new span boundary exceeds the old one by more than one pixel,
			// we start a filling movement on the other direction
		if ( mark2 > x2 )
			fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1,v2, filling_value, y, x2+1, mark2, ((direction==FILL_UP) ? FILL_DOWN : FILL_UP ), select_mode );
	}
}

void fill_area (hf_type *buffer_in, hf_type *buffer_out, gint xmax, gint ymax,
	hf_type filling_value, hf_type range, gint x, gint y) {
//	printf("FILL AREA: xmax=%d; ymax=%d, filling_value=%d, range=%d, x=%d, y=%d\n", xmax, ymax, filling_value, range, x, y);
	fill_area_with_mode (buffer_in, buffer_out, xmax, ymax, filling_value, range, x, y, FILL_RANGE, SELECT_REPLACE);	
}
	
void fill_area_with_mode (hf_type *buffer_in, hf_type *buffer_out, gint xmax, gint ymax,
	hf_type filling_value, hf_type range, gint x, gint y, gint fill_mode, gint select_mode) {

//	Fill area in buffer_out around filling seed (x,y) in buffer_in with filling_value
//	if surrounding pixels value are in a ± range
//	1.	Determine first span
//	2.	Fill first span
//	3.	Fill top and bottom spans from shadow

//	buffer_in and buffer_out could be the same (ex. when drawing faults)
//	They would be different, for instance, when buffer_in is used as
//	a select tool

//	2005-04-20 Modified to allow filling up to a maximum

	gint i, x1, x2;
	hf_type v1, v2, value_to_test;	// v1, v2: value to replace (range)

	// Initialize our control structure
	if (xmax<=SPAN_MAX) {
		for (i=0; i<SPAN_LIST_LENGTH; i++)
			span_list[i]=0;
		span_count=0;
	}
		
	// Range & filling_value should be >= 1
	range = MAX(range,1);
	filling_value = MAX(filling_value,MIN_FILLING_VALUE);
	
	if (fill_mode==FILL_MAX) {
		v1 = 0;
		v2 = range;
	}
	else { // fill_mode == FILL_RANGE
		v2 = v1 = *(buffer_in + VECTORIZE(x,y,xmax));
		if (v1 >= range)
			v1 = v1 - range;
		else
			v1 = 0;
		if (range <= (MAX_HF_VALUE - v2))
			v2 = v2 + range;
		else
			v2 = MAX_HF_VALUE;
	}

	*(buffer_out + VECTORIZE(x,y,xmax)) = filling_value;

	// Find the first span
	for (x1=x-1; x1>=0; x1--) {
		value_to_test = *(buffer_in + VECTORIZE(x1,y,xmax));
		if (!test_fill(value_to_test,v1,v2,select_mode,*(buffer_out + VECTORIZE(x1,y,xmax)))) {
			break;
		}
		else
			*(buffer_out + VECTORIZE(x1,y,xmax)) = filling_value;
	} // end for x1
	x1++;

	for (x2=x+1; x2<xmax; x2++) {
		value_to_test = *(buffer_in + VECTORIZE(x2,y,xmax));
		if (!test_fill(value_to_test,v1,v2,select_mode,*(buffer_out + VECTORIZE(x2,y,xmax)))) {
			break;
		}
		else
			*(buffer_out + VECTORIZE(x2,y,xmax)) = filling_value;
	} // end for x2
	x2--;

	fill_span_from_shadow (buffer_in, buffer_out, xmax,ymax, v1,v2,
		filling_value, y, x1, x2, FILL_BOTH, select_mode);
		
//	printf("LAST COUNT: %d\n",span_count);
}

gboolean fill_from_selection (hf_type *input, hf_type *selection, hf_type *output, gint maxx, gint maxy, fill_struct *pen) {
	gint i;
	glong diff;
	gdouble h;
	if (!selection)
		return FALSE;
	h = (gdouble) pen->height;
	for (i=0; i<(maxx*maxy); i++) {
		*(output+i) = *(input+i);
		if (*(selection+i)) {
			// *selection contains the height to fill up to
			// where we need to fill, otherwise 0 or 1
			diff = *(selection+i) - *(input+i);
			if (pen->noise_exponent>1.0) {
				diff = (glong) (h * pow(((gdouble) diff) / h, pen->noise_exponent));
			}
			if (diff>0) {
				diff = (((glong) pen->percent) * diff) / 100;
				*(output+i) += diff;
			}		
		}
	}
	return TRUE;
}

void select_for_fill (fill_struct *pen, hf_struct_type *hf, gint x, gint y) {

	// Select an area in the "magic wand" style,
	// Then fill it given the "paint bucket" parameters (fill _struct *)
	
	hf_type filling_value, height;
	
	if (!hf->tmp_buf)
		hf_backup(hf);
		
	//	We use "select_buf" for storing the selection mask
	if (!hf->select_buf)
		hf->select_buf = (hf_type *) x_calloc(sizeof(hf_type) * hf->max_x * hf->max_y, 1, "hf_type (hf->select_buf)");

	switch (pen->select_mode) {
		case SELECT_ADD:
			height = filling_value = pen->height;
			break;
		case SELECT_SUBTRACT:
			filling_value = MIN_FILLING_VALUE;
			height = pen->height + 1; // pen->height is bounded by MAX_HF_VALUE-1
			break;
		case SELECT_REPLACE:
		default:
			height = filling_value = pen->height;
			hf_reset_buffer(hf->select_buf, hf->max_x, hf->max_y);
	}
	
	// Height must be >= 1 and <= MAX_HF_VALUE-1
	height = MIN(MAX_HF_VALUE-1,MAX(1,height));
	
	fill_area_with_mode (hf->tmp_buf, hf->select_buf, hf->max_x, hf->max_y, filling_value, height, x, y, FILL_MAX, pen->select_mode);

	// Uncomment the next line for testing	
//		memcpy (hf->hf_buf, hf->select_buf, hf->max_x*hf->max_y*sizeof(hf_type));

}

void refresh_select_from_seeds (fill_struct *f, hf_struct_type *hf) {
	// Recalculate the selection to fill, typically after a height change
	GList *node;
	seed_coord *s;
	gint select_bck;
	select_bck = f->select_mode;
	if (!g_list_length(f->seeds_list))
		return;
//	printf("REFRESH_SELECT_FROM_SEEDS; list length: %d\n",g_list_length(f->seeds_list));
	if (hf->select_buf)
		hf_reset_buffer(hf->select_buf, hf->max_x, hf->max_y);
	for (node = f->seeds_list; node; node=node->next) {
		s = (seed_coord *) node->data;
		f->select_mode = s->select_mode;
//		printf("SELECT_MODE: %d; x: %d; y: %d\n",s->select_mode, s->x, s->y);
		select_for_fill (f, hf, s->x, s->y);
	}
	f->select_mode = select_bck;
}
