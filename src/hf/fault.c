/* fault.c:  Drawing a fault in a height field
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

#include <string.h>
 #include "line.h"
 #include "fault.h"
 #include "hf.h"
 #include "hf_calc.h"
 #include "img_process.h"
 #include "fill.h"

void init_fault_buffer (fault_struct *f, gint hf_size) {

	// Make sure the fault buffer is large enough for the current HF
	if (hf_size == f->buf_size) 
		return;
	f->buf_size = hf_size;
	f->buffer = (hf_type *)
		x_realloc(f->buffer, sizeof(hf_type) * f->buf_size * f->buf_size, "hf_type (f->buffer in init_fault_buffer)");
}

void prepare_fault_result_buf (fault_struct *f, hf_struct_type *hf) {
//	Public function
//	Used when beginning a fault, changing the altitude difference
//	or separating planes (cracks)

	hf_type diff;

// printf("PREPARE_RESULT_BUF\n");

	memcpy(hf->result_buf, hf->tmp_buf, hf->max_x * hf->max_y * sizeof(hf_type));

	// "Clamp" the HF values, removing the absolute altitude difference,
	// so that we can merge - add the HF with the line buffer later

	diff = (hf_type) ABS(( ( (gfloat) MAX_HF_VALUE) * (gfloat) f->altitude_difference) / 100.0);
	if (diff < (MAX_HF_VALUE - hf->max)) // We have enough room
		return;
	if (diff < (MAX_HF_VALUE - hf->max + hf->min)) // We'll have enough room if we clamp the min to 0
		hf_clamp_buffer (hf->result_buf, hf->max_x * hf->max_y, 0, hf->max - hf->min);
	else
		hf_clamp_buffer (hf->result_buf, hf->max_x * hf->max_y, 0, MAX_HF_VALUE - diff);
		
	f->buffer_reset_required = FALSE;
}

void begin_fault (fault_struct *f, hf_struct_type *hf) {

//	"begin_fault" is called when the user clicks on the HF to draw the control line

// printf("BEGIN_FAULT\n");
	hf_min_max (hf);

	init_fault_buffer (f, hf->max_x);

	// We need 2 result buffers
	// The original HF (hf_buf) is copied in tmp_buf
	// result_buf is allocated, if not already available
	// It will contain the "clamped" tmp_buf, before the merge with the line buffer

	if (!hf->tmp_buf)
		hf_backup(hf);
	if (!hf->result_buf) {
		hf->result_buf = (hf_type *) x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (hf->result_buf in begin_fault)");
	}

	prepare_fault_result_buf (f, hf);

}

void compute_fault (fault_struct *f, hf_struct_type *hf,
	gint x0, gint y0, gint x1, gint y1,
	dist_matrix_struct *dm, gdouble **gauss_list) {

//	Computing the fault
//	1. Draw the polyline fault in the fault buffer
//	   (it was only drawn in a display buffer before)
//	2. Raise & lower the planes each side of the line, in the buffer
//	   This gives an "altitude difference map"
//	3. Smooth the buffer with the given radius
//	4. Merge the buffer with hf->result_buf, put the result in hf->hf_buf
//	The calling function must unactivate the control line
//	and activate the "accept" button

//	f->mode = FAULT_PREVIEW (without smoothing) or FAULT_FINAL (with)

//	static gchar *CORNERS[] = {"TOP_LEFT","TOP_RIGHT","BOTTOM_RIGHT",
//		"BOTTOM_LEFT","LEFT_RIGHT","TOP_BOTTOM"}; // For testing purposes

	gint diff,  dx, dy, x, y, i, corner, minx, miny, maxx,maxy, ixs, iys;

	glong lvalue;

	gdouble ddist, startx=0.0, starty=0.0, endx=0.0, endy=0.0, ratio, dxs, dys;
	// Ratio = ratio by which we multiply dx,dy to get the perpendicular separation

	hf_type abs_diff, *sep_buf=NULL, *buf1=NULL, *buf2=NULL;

	static hf_type value = 0x0001;

	dx = x1-x0;
	dy = y1-y0;

	if ( (dx==dy) && !dx)
		return;
// printf("COMPUTE_FAULT - MODE: %d\n",f->mode);

// printf("BUF_SIZE: %d; HF_SIZE: %d\n",f->buf_size, hf->max_x);

	if (f->buffer_reset_required)
		prepare_fault_result_buf (f,hf); // Resets the flag to FALSE
	if (f->separation)
		f->buffer_reset_required = TRUE; // For the next "compute_fault" call


	// 1. Initialize the buffer to its default value
	for (i=0; i<f->buf_size*f->buf_size; i++) {
		*(f->buffer+i) = 0;
	}

	// 2. Extend the control line (mouse controlled)
	//     to find where the polyline intersects with the HF edges
	if (! intersect_rect (0,0,hf->max_x-1, hf->max_y-1, x0, y0, x1, y1,
		&startx, &starty, &endx, &endy) )
			return;

	ddist = DIST2(startx, starty, endx,endy);
	set_line_scale (f->fractal_line->polyline, ddist, ddist);

	// 3. Compute the absolute difference between the planes
	diff = (gint) ( ( (gfloat) MAX_HF_VALUE) * (gfloat) f->altitude_difference) / 100.0;
	abs_diff = (hf_type) ABS(diff);

//	printf("\n***********LINE (%d,%d) - (%d,%d) from (%5.2f, %5.2f) to (%5.2f, %5.2f)\n",x0, y0, x1, y1, startx, starty, endx, endy);
	// 4. Draw the polyline
	draw_n_transform_all_segments (f->fractal_line->polyline,
		endx-startx, endy-starty,
		startx, starty,
		(gpointer) f->buffer,
		f->buf_size,
		f->buf_size,
		(gpointer) &value,
		HF_TYPE_ID,
		OVERFLOW_ZERO);
		
	if (f->separation) {
		sep_buf = (hf_type *) x_malloc(sizeof(hf_type) * f->buf_size * f->buf_size, "hf_type (sep_buf in compute_fault)");
		memcpy(sep_buf, f->buffer,sizeof(hf_type) * f->buf_size * f->buf_size);
	}

	// 5. Apply the altitude difference
	// Lower plane is always 0
	// Difference ranges from -50% to +50%
	// When difference is positive, the top or left plane is higher
	// When difference is negative, the bottom or right plane is higher

	miny = (gint) MIN(starty,endy);
	minx = (gint) MIN(startx,endx);
	maxy = (gint) MAX(starty+0.5,endy+0.5);
	maxx = (gint) MAX(startx+0.5,endx+0.5);
	if ( (!miny) && (!minx) )
		corner = TOP_LEFT;
	else
		if ( (!miny) && (maxx==f->buf_size-1) )
			corner = TOP_RIGHT;
		else
			if ( (!minx) && (maxy==f->buf_size-1) )
				corner = BOTTOM_LEFT;
			else
				if (  (maxx==f->buf_size-1) &&
					(maxy==f->buf_size-1) )
					corner = BOTTOM_RIGHT;
				else
					if ( (!miny) &&  (maxy==f->buf_size-1) )
						corner = LEFT_RIGHT;
					else
						corner = TOP_BOTTOM;

	if ( (corner==TOP_LEFT) || (corner==LEFT_RIGHT) ||
		(corner==TOP_BOTTOM) || (corner==BOTTOM_RIGHT) )
		if (diff>0)
			fill_area (f->buffer,f->buffer, f->buf_size, f->buf_size, abs_diff, 0, 0, 0);
		else
			fill_area (f->buffer, f->buffer, f->buf_size, f->buf_size, abs_diff, 0, f->buf_size-1, f->buf_size-1);
	else
		if (corner==TOP_RIGHT)
			if (diff>0)
				fill_area (f->buffer,f->buffer, f->buf_size, f->buf_size, abs_diff, 0, f->buf_size-1, 0);
			else
				fill_area (f->buffer, f->buffer, f->buf_size, f->buf_size, abs_diff, 0, 0, 0);
		else // BOTTOM_LEFT
			if (diff>0)
				fill_area (f->buffer, f->buffer,f->buf_size, f->buf_size, abs_diff, 0, 0, f->buf_size-1);
			else
				fill_area (f->buffer,f->buffer, f->buf_size, f->buf_size, abs_diff, 0, f->buf_size-1,f->buf_size-1);

// printf( "(minx,miny): (%d,%d); (maxx,maxy): (%d,%d); corner: %s\n",minx,miny,maxx,maxy,CORNERS[corner]);

	// Updated 2004-09-09 for processing separation (cracks)

	// If f->mode == FAULT_PREVIEW, we merge the fault buffer and the HF, then separate
	// If f->mode == FAULT_FINAL, we separate both the fault buffer and the HF,
	// then smooth the fault buffer, then merge

	if (f->separation) {
		// 5.5 Calculate delta x and delta y resulting from the separation
		//	Initialize the separation buffer sep_buf with the polyline

		// 1. Find the length of the vector perpendicular to the control line
		// 2. Scale down this vector to f->separation, giving dxs,dys, in pixel units
		ratio = ((gdouble) f->separation) / sqrt( (gdouble) (dx*dx + dy*dy) )  ;
		dxs = ratio * (gdouble) dy;
		dys = ratio * (gdouble) dx;

// printf("(dx,dy): (%d,%d); (dxs,dys): (%5.2f, %5.2f); ratio: %5.2f\n",dx,dy,dxs,dys,ratio);

// printf("(startx,starty): (%5.2f,%5.2f); (endx,endy): (%5.2f,%5.2f); CORNER: %s\n",startx,starty,endx,endy,CORNERS[corner]);

		// For final computation, we try to get values in both planes, so that smoothing will work
		if (f->mode==FAULT_FINAL)
			for (i=0; i<f->buf_size*f->buf_size; i++)
				*(f->buffer+i) += MAX_HF_VALUE>>2;

		// We choose the corner to move so that the move doesn't create "blank" areas
		// The delta sign depends on where the control line cuts the edges
		switch (corner) {
			case TOP_LEFT:
				dxs = - ABS(dxs);
				dys = - ABS(dys);
				fill_area (sep_buf,sep_buf, f->buf_size, f->buf_size, value, 0, 0, 0);
				break;
			case TOP_RIGHT:
				dxs = ABS(dxs);
				dys = - ABS(dys);
				fill_area (sep_buf,sep_buf, f->buf_size, f->buf_size, value, 0, f->buf_size-1, 0);
				break;
			case BOTTOM_LEFT:
				dxs = - ABS(dxs);
				dys = ABS(dys);
				fill_area (sep_buf,sep_buf, f->buf_size, f->buf_size, value, 0, 0, f->buf_size-1);
				break;
			case BOTTOM_RIGHT:
				dxs = ABS(dxs);
				dys = ABS(dys);
				fill_area (sep_buf,sep_buf, f->buf_size, f->buf_size, value, 0, f->buf_size-1, f->buf_size-1);
				break;
			case LEFT_RIGHT:
				dxs = (gdouble) f->separation; // Temporary solution - we'll "stretch" one plane in the future
				dys = 0.0;
				fill_area (sep_buf,sep_buf, f->buf_size, f->buf_size, value, 0, f->buf_size-1, f->buf_size>>1);
				break;
			default: // TOP_BOTTOM
				dys = (gdouble) f->separation;
				dxs = 0.0;
				fill_area (sep_buf,sep_buf, f->buf_size, f->buf_size, value, 0, f->buf_size>>1, f->buf_size-1);

		}

		// Integer values for preview (separation without interpolation)
		ixs = (gint) floor(dxs+0.5);
		iys = (gint) floor(dys+0.5);

		// Separate the source and the control buffers, given sep_buf

// printf("(DX,DY): (%d, %d); (DXS, DYS): (%5.2f, %5.2f);\n", dx, dy, dxs,dys);
		if (f->mode == FAULT_FINAL) {
//		if (FALSE) {
			buf1 = (hf_type *) x_calloc(f->buf_size*f->buf_size, sizeof(hf_type), "hf_type (buf1 in compute_fault)");
			buf2 = (hf_type *) x_calloc(f->buf_size*f->buf_size, sizeof(hf_type), "hf_type (buf2 in compute_fault)");
			// We separate then we merge
			for (x=0; x < f->buf_size; x++) {
				for (y=0; y< f->buf_size; y++) {
					i =VECTORIZE(x,y,f->buf_size);
					if (*(sep_buf+i) ) {
						translate_real_forward_mapping (
							f->buffer,
							buf1,
							HF_TYPE_ID,
							f->buf_size, f->buf_size,
							x,y,
							dxs,dys);

						translate_real_forward_mapping (
							hf->result_buf,
							buf2,
							HF_TYPE_ID,
							f->buf_size, f->buf_size,
							x,y,
							dxs,dys);

					} // if sep_buf
					else { // No translation!
						*(buf1+i) = *(f->buffer+i);
						*(buf2+i) = *(hf->result_buf+i);
					} // if !sep_buf
				} // for y
			} // for x

		swap_buffers ((gpointer *) &f->buffer, (gpointer *) &buf1);
		swap_buffers ((gpointer *) &hf->result_buf, (gpointer *) &buf2);

	} // End FAULT_FINAL

	else {
		// FAULT_PREVIEW
		// We merge then we separate

		// Merge
		for (i=0; i<(f->buf_size*f->buf_size); i++)
			*(hf->result_buf+i) = *(hf->result_buf+i) + *(f->buffer+i);
		// We need an empty buffer
		for (i=0; i<f->buf_size*f->buf_size; i++) {
			*(hf->hf_buf+i) = 0;
		}
		// Separate (no interpolation)
		for (x=0; x < f->buf_size; x++) {
			for (y=0; y< f->buf_size; y++) {
				i =VECTORIZE(x,y,f->buf_size);
				if (*(sep_buf+i) ) {

					translate_forward_mapping (
						hf->result_buf,
						hf->hf_buf,
						HF_TYPE_ID,
						f->buf_size, f->buf_size,
						x,y,
						ixs,iys);

				} // if sep_buf
				else { // No translation!
					*(hf->hf_buf+i) = *(hf->result_buf+i);
				} // if !sep_buf
			} // for y
		} // for x
	} // End FAULT_PREVIEW

	} // End with separation

	else {

	}


	// 6. Smooth the line buffer
	if (f->smoothing && (f->mode==FAULT_FINAL) ) {
		swap_buffers ((gpointer*) &hf->hf_buf, (gpointer*) &f->buffer);
		hf_smooth (hf, f->smoothing,dm, FALSE, gauss_list);
		swap_buffers ((gpointer*) &hf->hf_buf, (gpointer*) &f->buffer);
	}

	// 7. Merge the line buffer with the HF

	if (f->separation) {
		if (f->mode==FAULT_FINAL) {
			for (i=0; i<(f->buf_size*f->buf_size); i++) {
				lvalue = ((glong) *(hf->result_buf+i)) + ((glong) *(f->buffer+i))
					- (glong) (MAX_HF_VALUE>>2) ;
				*(hf->hf_buf+i) = (hf_type) MAX(0,lvalue);
			}
		}
	}
	else
		for (i=0; i<(f->buf_size*f->buf_size); i++)
			*(hf->hf_buf+i) =*(hf->result_buf+i) + *(f->buffer+i);

//	memcpy(hf->hf_buf, f->buffer, f->buf_size * f->buf_size * sizeof(hf_type)); // Test

//	The next function draws the line in the buffer, for testing purposes
/*
	draw_n_transform_all_segments (f->fractal_line,
		endx-startx, endy-starty,
		startx, starty,
		(gpointer) hf->hf_buf,
		f->buf_size,
		f->buf_size,
		(gpointer) &value,
		HF_TYPE_ID,
		OVERFLOW_ZERO);
*/
	if (sep_buf) {
		x_free(sep_buf);
		if (buf1)
			x_free(buf1);
		if (buf2)
			x_free(buf2);
	}
}

void fault_pen_free (fault_struct *f) {
	if (f) {
		if (f->fractal_line)
			fractal_polyline_free (f->fractal_line);
		x_free (f);
		}
}

fault_struct *fault_pen_new() {
	fault_struct *f;
	gint i;
	f = (fault_struct *) x_malloc(sizeof(fault_struct), "fault_struct");
	f->fractal_line = fractal_polyline_new (7, // steps
		0,50, 	// random_x, random_y
		1,0, 		// width, random_w
		123456, 	// seed
		2.0, 		// distribution
		FALSE,	// if_cracks
		50, 2,	// cracks_depth, cracks_width
		1);		// cracks_branching_steps (for future enhancement)

	f->smoothing = 4;
	f->altitude_difference = 25;
	f->separation = 0;
	f->buffer_reset_required = TRUE;
	f->mode = FAULT_FINAL;
	f->buffer =(hf_type *) x_malloc(FAULT_PREVIEW_SIZE*FAULT_PREVIEW_SIZE * sizeof(hf_type), "hf_type (f->buffer in fault_pen_new)") ;
	for (i=0; i<FAULT_PREVIEW_SIZE*FAULT_PREVIEW_SIZE; i++)
		*(f->buffer+i) = MAX_HF_VALUE;
	f->buf_size = FAULT_PREVIEW_SIZE;
	return f;
}
