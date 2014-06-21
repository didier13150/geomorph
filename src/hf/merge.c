/* merge.c - Merge utility functions (merging source - result)
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
#include "merge.h"
#include "hf_wrapper.h"

void merge_calc (merge_struct *mrg) {
	if (!mrg)
		return;
	simple_merge(mrg);
	if (mrg->display_fn)
		(*mrg->display_fn) (mrg->data);
}

merge_struct *merge_struct_new (gpointer hfw_adr, void (*display_fn) (gpointer)) {
	merge_struct *mrg;
	mrg = (merge_struct *) x_malloc(sizeof(merge_struct), "merge_struct");
	mrg->source1 = NULL;
	mrg->source2 = NULL;
	mrg->result = NULL;
	mrg->max_x = 0;
	mrg->max_y = 0;
	mrg->merge_op = ADD;
	mrg->display_fn = display_fn; // Post-merge function, typically for displaying the result
	mrg->data = hfw_adr; // data to display
	mrg->mix = 0;
	mrg->source_offset = 0;
	mrg->result_offset = 0;
	mrg->x_translate = 0;
	mrg->y_translate = 0;
	mrg->hardness = MERGE_HARDNESS;
	return mrg;
}

void set_merge_buffers (merge_struct *mrg, 
	gpointer source1, 
	gpointer source2, 
	gpointer result, 
	gint max_x, 
	gint max_y) {

	mrg->source1 = source1;
	mrg->source2 = source2;
	mrg->result = result;
	mrg->max_x = max_x;
	mrg->max_y = max_y;
};

void reset_merge_buffers (merge_struct *mrg) {
	mrg->source1 = NULL;
	mrg->source2 = NULL;
	mrg->result = NULL;
	mrg->max_x = 0;
	mrg->max_y = 0;
};

void simple_merge (merge_struct *mrg ) {
	//	Merging source1 and source2 in result, all HF being the same size
	//	"result" should be already allocated
	//	Usually called by mrg->calc_hf
	gint i, j, dx, dy, indx, max_x, max_y;
	gdouble s1, s2, r, p1, p2, percent, so1, so2, pivot=0.0, abs_pivot=0.0;
	hf_type hf_source, hf_result, lmin = 0, lmax = 0; // tmp;
	hf_type *source1, *source2, *result;
	static gdouble max_f = (gdouble) MAX_HF_VALUE;
	//	Merging:  1st version worked with a "percent" value, 
	//	0% meaning keep only source1, 100% meaning keep only source2
	//	50 % meaning keep 50% of both
	//	It's now "mix", -100 meaning source1, 100 meaning source2, 0 meaning 50-50
	// 2005-06-23: 	now source1_offset and source2_offset are relative values
	// 		ranging from -100% to +100%
// printf("Simple merge: %p; %p, %p, %p\n", mrg, mrg->source1,mrg->source2, mrg->result);
	if ((!mrg->source1) || (!mrg->source2) || (!mrg->result))
		return;
	source1 = mrg->source1;
	source2 = mrg->source2;
	result = mrg->result;
	max_x = mrg->max_x;
	max_y = mrg->max_y;
	percent = ( ((gdouble) mrg->mix) + 100.0) / 2.0;
	// For MIN_MERGE and MAX_MERGE
	p1 = MIN(100.0 - (gdouble) mrg->mix,100.0) / 100.0;
	p2 = MIN(100.0 + (gdouble) mrg->mix,100.0) / 100.0;
	dx = (mrg->x_translate * max_x) / 100;
	dy = - (mrg->y_translate * max_y) / 100;
	so1 =  1.0+(((gdouble) mrg->source_offset)/100.0);
	so2 =  1.0+(((gdouble) mrg->result_offset)/100.0);
// printf ("SIMPLE_MERGE;  Source1: %p; Source 2: %p; Result: %p; DX: %d; DY: %d; PERCENT: %f; P1: %5.3f, P2: %5.3f; Source1_offset: %8.5f; Source2_offset: %8.5f; \n", source1, source2, result, dx, dy, percent, p1, p2, so1, so2);
// printf ("SIMPLE_MERGE: %p;  Source1 - tmp_buf: %p; Source 2 - result_buf: %p; Result - hf_buf: %p; \n",mrg,source1, source2, result);
	// Calculate combined max and min for "altitude average"
	if (mrg->merge_op==ALT_MERGE) {
		lmin = MAX_HF_VALUE;
		lmax = 0;
		pivot = ((gdouble) mrg->mix)/100.0;
		abs_pivot = ABS(pivot);
		for (i=0; i<(max_y*max_x); i++) {
			
			if (pivot>=0)
				hf_source = (hf_type) (so1 * (gdouble) *(source1 + i));
			else
				hf_source =  (hf_type) (so2 * (gdouble) *(source2 + i));

			if (lmin > hf_source)
				lmin = hf_source;
			else {
				if (lmax < hf_source)
					lmax = hf_source;
			}
		}
	}
	for (i=0; i<max_y; i++) 
	  for (j=0; j<max_x; j++) {
		indx = VECTORIZE(j,i,max_x);
		s1 =  so1 * (gdouble) *(source1 + indx);
		s2 =  so2 * (gdouble) *(source2 + VECTORIZE(WRAP(j+dx,max_x),
					WRAP2(i+dy,max_y), max_x));
		switch (mrg->merge_op) {
//	When results fall outside the 0-0xFFFF range, the slope is reverted
		case ADD:
			r = ABS((((100.0-percent) * s1) + (percent * s2) ) / 100.0);
			*(result + indx) = (hf_type) ((r>max_f)?(max_f+max_f-r):r);
			break;
		case SUBTRACT:
			// Because this is a subtraction, percent has no effect on s1
			*(result + indx) = (hf_type) MIN(max_f, ABS(MAX(0,0.5*s1 - ((percent * s2) / 100.0)) ) );
			break;
		case MULTIPLY:	
			*(result + indx) = (hf_type) MIN(max_f,((s1*(100.0-percent)) + (percent* (s1 * s2) / max_f)) / 100.0);
			break;
		case MIN_MERGE:
		
		//	*(result + indx) = (hf_type) MIN(max_f,MAX(0.0,MIN(p1*s1,p2*s2)));
		//	2005-09-01 Formula changed to avoid a black result when mix=-100
			*(result + indx) = (hf_type) ((1.0-p2)*s1 + (1.0-p1)*s2
			 + (1.0-ABS(mrg->mix)/100.0)*MIN(max_f,MAX(0.0,MIN(s1,s2))));
			break;
		case MAX_MERGE:
			*(result + indx) = (hf_type) MAX(0.0,MIN(max_f,MAX(p1*s1,p2*s2)));
			break;
		case ALT_MERGE:
			if (pivot>=0)
				r = ((s1 / (lmax - lmin))/ (2*mrg->hardness)) + 0.5 - (abs_pivot/(2*mrg->hardness));
			else
				r = ((s2 / (lmax - lmin))/ (2*mrg->hardness)) + 0.5 - (abs_pivot/(2*mrg->hardness));

			r = MAX(0.0, MIN(r,1.0));

			if (pivot>=0.0)
				*(result + indx) = ((hf_type)(r*s1)) + ((hf_type) ((1.0-r)*s2));
			else
				*(result + indx) = ((hf_type)(r*s2)) + ((hf_type) ((1.0-r)*s1));
				
			break;
		case XOR_MERGE:
			hf_source = (hf_type) MIN(max_f,MAX(0.0,(100.0-percent) * s1));
			hf_result = (hf_type) MIN(max_f,MAX(0.0,percent * s2));
			*(result + indx) = hf_source ^ hf_result;
			break;
		}
	}	// end of merging for...
}

