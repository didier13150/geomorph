/* line.c:  Functions for drawing lines and polylines in a hf_struct
 *
 * Copyright (C) 2004 Patrice St-Gelais
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

#include <string.h>
#include "line.h"
#include "hf.h"
#include "hf_calc.h"

line_struct *line_new (gint max_seg, gint tip_size) {
//	Allocator for an irregular line struct (polyline)
	line_struct *l;
	l = (line_struct *) x_malloc(sizeof(line_struct), "line_struct");
	l->max_seg = max_seg;
	l->alloc_seg = max_seg;
	l->cur_seg = -1;
	l->segments = (segment_struct *) x_calloc(max_seg,sizeof(segment_struct), "segment_struct");
	l->size = tip_size;
	l->transl_x = 0.0;
	l->transl_y = 0.0;
	l->scale_x = 1.0;
	l->scale_y = 1.0;
// printf("MAX_SEG: %d\n",max_seg);
	return l;
}

void line_free (line_struct *l) {
	if (l)
		x_free(l);
}

void extend_line (line_struct *l, gint new_max_seg) {
//	Extend line by reallocating the segments
	l->max_seg = new_max_seg;
	if (new_max_seg>l->alloc_seg) {
		l->segments = (segment_struct *) x_realloc(l->segments, new_max_seg * sizeof(segment_struct), "segment_struct");
		l->alloc_seg = new_max_seg;
	}
}

void reset_line (line_struct *l, gint max_seg, gint tip_size) {
	l->cur_seg = -1;
	l->max_seg = MIN(l->alloc_seg,max_seg);
	l->size = tip_size;
}

void set_line_scale (line_struct *l, gdouble sizex, gdouble sizey) {
	// Set the polyline scale from the image size (or some slanted line)
	// The internal struct of the polyline is normalized to fit between 0 and 1
	l->scale_x = sizex;
	l->scale_y = sizey;
}

void set_line_translation (line_struct *l, gint x, gint y, gint sizex, gint sizey) {
	l->transl_x = ((gdouble) x) / (gdouble) sizex;
	l->transl_y = ((gdouble) x) / (gdouble) sizey;
}

void add_segment (line_struct *l, gdouble x0, gdouble y0, gdouble x1, gdouble y1) {
//	Add a segment to an irregular line struct
	l->cur_seg++;
	if (l->cur_seg>=l->max_seg)
		extend_line (l, l->cur_seg+1);
	(l->segments+l->cur_seg)->x0 = x0;
	(l->segments+l->cur_seg)->y0 = y0;
	(l->segments+l->cur_seg)->x1 = x1;
	(l->segments+l->cur_seg)->y1 = y1;
// printf("ADDING SEGMENT %d: (%5.2f,%5.2f) - (%5.2f,%5.2f)\n",l->cur_seg,x0,y0,x1,y1);
}

fractal_line_struct *fractal_polyline_new ( gint steps,
	gint random_x, gint random_y,
	gint width, gint random_w,
	gint seed, gdouble distribution,
	gboolean if_cracks, gint cracks_depth, gint cracks_width,
	gint cracks_branching_steps )
{
	fractal_line_struct *f;
	f = (fractal_line_struct *) x_malloc(sizeof(fractal_line_struct), "fractal_line_struct");
	f->steps = steps;
	f->random_x = random_x;
	f->random_y = random_y;
	f->seed = seed;
	f->distribution = distribution;
	f->width = width;
	f->random_w = random_w;
	f->if_cracks = if_cracks;
	f->cracks_depth = cracks_depth;
	f->cracks_width = cracks_width;
	f->cracks_branching_steps = cracks_branching_steps;
	f->polyline =  line_new ((gint) pow(2.0,(gdouble) f->steps),width);
	return f;
}

void fractal_polyline_free (fractal_line_struct *f) {
	if (f) {
		if (f->polyline)
			line_free(f->polyline);
		x_free(f);
	}
}

void divide_n_draw(fractal_line_struct *f, gint level, gdouble x0, gdouble y0,
	gdouble x1, gdouble y1, gdouble length, gdouble cos, gdouble sin) {
	// Divide a line recursively, given a slanted control line
	// Level: from max to 0 (0 = highest frequency)
	// (x0,y0) - (x1,y1): segment to divide
	// length: total line length
	// cos, sin: inclination of the control line
	gdouble dx, dy, percent, x2, y2, ddist, ddx, ddy, var_y, factor;
	static gdouble big_value = 1000000.0, big_value_invert = 0.000001;
	if (!level)
		add_segment(f->polyline,
			x0 / length,
			y0 / length,
			x1 / length,
			y1 / length);
	else {
		if ( f->if_cracks && (level==f->cracks_width) ) {
			factor = (rand()%5) ? 1.0 : 0.1 * length * ( (gdouble) f->cracks_depth ) / 100.0;
//			printf("FACTOR: %5.2f\n",factor);
		}
		else
			factor = 1.0;
		ddx = x1 - x0;
		ddy = y1 - y0;
		ddist = sqrt(ddx*ddx + ddy*ddy);
		if (!ddist)
			return;
		// 1. Find the subdivision point - depends on random_x
		// If random_x is 0, lines are cut exactly in half
		dx = ddx / 2.0;
		dy = ddy / 2.0;
		if (f->random_x)
			percent = 1.0 + 0.01 * (((gdouble) (rand()%f->random_x)) - ((gdouble) f->random_x) / 2.0);
		else
			percent = 1.0;
		x2 = x0 + percent * dx;
		y2 = y0 + percent * dy;

		// 2. Move the subdivision point under or over the line - depends on f->random_y
		// If random_y is 0, the result is always a straight line
		// The variation range is equal to the length of a standard segment
		// We'll subtract half this variation to get the actual one, over or under the line

		var_y = factor * ddist * 0.01 * (gdouble) f->random_y;
// printf("RANDOM_Y: %d; VAR_Y before distribution: %5.2f; DDIST: %5.2f; DISTRIB: %5.2f\n",f->random_y,var_y, ddist, f->distribution);
// printf("RANDOM_X: %d; STEPS: %d; SEED: %d\n",f->random_x,f->steps, f->seed);
		var_y = pow(var_y/(ddist * factor), f->distribution ) * factor * ddist ;
// printf("VAR_Y after distribution: %5.2f\n",var_y);

		if (var_y>=big_value_invert)
			var_y = (rand()%(glong)(big_value * var_y)) / big_value - (var_y/2.0);

		// We must "rotate" the move
		// The angle used is the stroke angle, not the angle of the last subdivision
		// Otherwise, the line is too irregular
		if ( f->if_cracks && (level==f->cracks_width) ){
			dx = ABS(var_y) * ddy / ddist; // sin - angle of the last subdivision
			dy = - ABS(var_y) * ddx / ddist; // cos
		}
		else {
			dx = sin * var_y;
			dy = cos * var_y;
		}
//		dx = - ((gdouble) var_y) * ddy / ddist; // sin - angle of the last subdivision
//		dy = ((gdouble) var_y) * ddx / ddist; // cos
// printf("VAR_Y-2: %d; (ddx,ddy): (%5.2f, %5.2f); (dx,dy): (%5.2f, %5.2f) \n",var_y, ddx, ddy, dx, dy);
		x2 = x2 + dx;
		y2 = y2 + dy;
// printf("LEVEL: %d; DDIST: %5.2f; (x0,y0): (%d,%d); (x2,y2): (%d,%d); (x1,y1): (%d,%d)\n",level, ddist, x0, y0, (gint) x2, (gint) y2, x1, y1);

		divide_n_draw(f, level-1, x0, y0, x2, y2, length, cos, sin);
		divide_n_draw(f, level-1, x2, y2, x1, y1, length, cos, sin);

	}
}

void init_fractal_polyline (fractal_line_struct *f,
	gint sizex, gint sizey,
	gint x0, gint y0,
	gint x1, gint y1) {

	// When using the fault or crack pens,
	// create a fractal line for the preview window or for initializing the drawing buffer

	gdouble cos, sin, ddist;

	// Subdivide and draw the line
	srand (f->seed);
	ddist = DIST2(x0,y0,x1,y1);
	sin = -((gdouble) (y1-y0)) / ddist;
	cos = ((gdouble)(x1-x0)) / ddist;;

	reset_line (f->polyline, (gint) pow(2.0,(gdouble) f->steps),1);
	set_line_translation (f->polyline, x0, y0, sizex, sizey);
	set_line_scale (f->polyline,(gdouble) (sizex-1), (gdouble) (sizey-1));

	divide_n_draw (f, f->steps, 0.0, 0.0,
		(gdouble) (x1-x0), (gdouble) (y1-y0), ddist, cos, sin);
}

void write_value (gpointer buf, gint index, gpointer value_ptr,
	gint data_type) {

//	Write a value " *value_ptr" of type "data_type" in array "buf" at "index"
	switch (data_type) {
		case GINT_ID:
			*(((gint *) buf) + index) = *((gint *) value_ptr);
			break;
		case HF_TYPE_ID:
//			printf("INDEX: %d\n", index);
			*(((hf_type *) buf) + index) = *((hf_type *) value_ptr);
			break;
		case GDOUBLE_ID:
			*(((gdouble *) buf) + index) = *((gdouble *) value_ptr);
			break;
		case UNSIGNED_CHAR_ID:
			*(((unsigned char *) buf) + index) = *((unsigned char *) value_ptr);
			break;
		default:
			printf(_("Unexpected option in %s; contact the programmer!"),"write_value");
			printf("\n");
			return;
	}
}

void draw_thin_segment_in_buf (segment_struct *s, gdouble trx, gdouble try,
	gpointer buf, gint max_x, gint max_y, gpointer value_ptr, gint data_type) {

	//	Draw a line in "buf" of size (max_x,max_y) with value *value_ptr
	//	of type "data_type", given translation (trx,try)
	//	Bresenham algorithm, from Foley's / van Dam's Computer Graphics
	gint x0,x1,y0,y1,d, x, y, dx, dy, incrE, incrNE, ix, iy, xs, ys;
	gboolean tx=TRUE, ty=TRUE;
// printf("DRAWING F (%5.2f, %5.2f) - (%5.2f, %5.2f); TRX: %5.2f; TRY: %5.2f;\n",s->x0,s->y0,s->x1,s->y1, trx, try);
	x0 = (gint) (0.5+s->x0 + trx);
	y0 = (gint) (0.5+s->y0 + try);
	x1 = (gint) (0.5+s->x1+ trx);
	y1 = (gint) (0.5+s->y1+ try);
	x0 = MAX(0,MIN(x0,max_x-1));
	y0 = MAX(0,MIN(y0,max_y-1));
	x1 = MAX(0,MIN(x1,max_x-1));
	y1 = MAX(0,MIN(y1,max_y-1));

// printf("DRAWING (%d,%d) - (%d,%d) in %d X %d\n",x0,y0,x1,y1, max_x, max_y);
	dx = ABS(x1 - x0);
	dy = ABS(y1 - y0);
	if (y1>=y0) {
		y = y0;
		ys = y1;
	}
	else {
		y = max_y - y0;
		ys = max_y - y1;
		ty = FALSE;
	}
	if (x1>=x0) {
		x = x0;
		xs = x1;
	}
	else {
		x = max_x - x0;
		xs = max_x - x1;
		tx = FALSE;
	}
	if (dx >= dy) { // Slope <= 1
		d = 2*dy - dx;
		incrE = 2*dy;
		incrNE = 2*(dy-dx);
		if (tx)
			ix = x;
		else
			ix = max_x - x;
		if (ty)
			iy = y;
		else
			iy = max_y - y;
		write_value (buf,VECTORIZE(ix,iy,max_x),value_ptr, data_type);
//		printf("Drawing (%d,%d) in (%d,%d) to (%d,%d)\n",ix,iy,x0,y0,x1,y1);
		while ( x < xs) {
			if (d<=0)
				d += incrE;
			else {
				d += incrNE;
				y++;
			}
			x++;
			if (tx)
				ix = x;
			else
				ix = max_x - x;
			if (ty)
				iy = y;
			else
				iy = max_y - y;
			write_value (buf,VECTORIZE(ix,iy,max_x),value_ptr, data_type);
//			printf("Drawing (%d,%d) in (%d,%d) to (%d,%d)\n",ix,iy,x0,y0,x1,y1);
		}
	} // End dx >= dy (slope <= 1)
	else { // Beginning slope > 1
		d = 2*dx - dy;
		incrE = 2*dx;
		incrNE = 2*(dx-dy);
		if (tx)
			ix = x;
		else
			ix = max_x - x;
		if (ty)
			iy = y;
		else
			iy = max_y - y;
		write_value (buf,VECTORIZE(ix,iy,max_x),value_ptr, data_type);
//		printf("Drawing (%d,%d) in (%d,%d) to (%d,%d)\n",ix,iy,x0,y0,x1,y1);
		while ( y < ys) {
			if (d<=0)
				d += incrE;
			else {
				d += incrNE;
				x++;
			}
			y++;
			if (tx)
				ix = x;
			else
				ix = max_x - x;
			if (ty)
				iy = y;
			else
				iy = max_y - y;
			write_value (buf,VECTORIZE(ix,iy,max_x),value_ptr, data_type);
//			printf("Drawing (%d,%d) in (%d,%d) to (%d,%d)\n",ix,iy,x0,y0,x1,y1);
		}
	}
}

void transform_segment (segment_struct *s,
	gdouble scale_x, gdouble scale_y, gdouble ddist, gdouble cos, gdouble sin) {

	// Steps:
	//	1. Rotate
	//	2. Scale (to fit the whole original line in the slanted line drawn by the mouse)
	// Translation is done in the drawing function

	gdouble x_rot;

// printf("Transforming  (%7.4f, %7.4f) - (%7.4f, %7.4f) \n", s->x0, s->y0, s->x1, s->y1);

	if (ddist>0) {

		x_rot = s->x0 * cos - s->y0 * sin;
		s->y0 = s->y0 * cos + s->x0 * sin;
		s->x0 = x_rot;

		x_rot = s->x1 * cos - s->y1 * sin;
		s->y1 = s->y1 * cos + s->x1 * sin;
		s->x1 = x_rot;

	}

// printf("After rotation (%7.4f, %7.4f) - (%7.4f, %7.4f) \n", s->x0, s->y0, s->x1, s->y1);

	s->x0 = scale_x * s->x0;
	s->y0 = scale_y * s->y0;
	s->x1 = scale_x * s->x1;
	s->y1 = scale_y * s->y1;

// printf("After scaling with %7.4f, %7.4f = (%7.4f, %7.4f) - (%7.4f, %7.4f)\n", scale_x, scale_y, s->x0, s->y0, s->x1, s->y1);
}

void draw_n_transform_all_segments (line_struct *l, gdouble dx, gdouble dy,
	gdouble unscaled_transl_x, gdouble unscaled_transl_y,
	gpointer buf, gint max_x, gint max_y, gpointer value_ptr,
	gint data_type, gint overflow) {

//	Draw an irregular line segment by segment, from (0,0) to (dx,dy)
//	Apply scaling, rotations and translations
//	2004-01-30 The "size" (tip) parameter is not recognized yet

	static segment_struct s;
	gdouble ddist, cos, sin;

	ddist =  sqrt( pow(dx ,2.0) + pow(dy, 2.0) ) ;

	//if (ddist>0) {
		cos =  dx / ddist;
		sin = dy / ddist;
	//}

	for (l->cur_seg=0; l->cur_seg<l->max_seg; l->cur_seg++) {
		memcpy(&s, l->segments+l->cur_seg, sizeof(segment_struct));

		transform_segment (&s, l->scale_x, l->scale_y,
			ddist, cos, sin);
		draw_thin_segment_in_buf (&s,
			unscaled_transl_x, unscaled_transl_y,
			buf, max_x, max_y, value_ptr, data_type);
	}
}
/*
void draw_n_transform_all_segments (line_struct *l, gdouble dx, gdouble dy,
	gdouble unscaled_transl_x, gdouble unscaled_transl_y,
	gpointer buf, gint max_x, gint max_y, gpointer value_ptr,
	gint data_type, gint overflow) {

//	Draw an irregular line segment by segment, from (0,0) to (dx,dy)
//	Apply scaling, rotations and translations
//	2004-01-30 The "size" (tip) parameter is not recognized yet

	static segment_struct s;
	gdouble ddist, cos, sin;

	ddist =  sqrt( pow(dx ,2.0) + pow(dy, 2.0) ) ;

	if (ddist>0) {
		cos =  dx / ddist;
		sin = dy / ddist;
	}

// printf ("In draw_an_transform_all_segments - DX: %5.2f; DY: %5.2f; TR_X: %5.2f; TR_Y: %5.2f\n",dx, dy, unscaled_transl_x, unscaled_transl_y);

	for (l->cur_seg=0; l->cur_seg<l->max_seg; l->cur_seg++) {
		memcpy(&s, l->segments+l->cur_seg, sizeof(segment_struct));

if ( (l->cur_seg==0) || (l->cur_seg == (l->max_seg-1)) )
printf ("\nDRAWING SEGMENT       : (%5.2f, %5.2f) - (%5.2f, %5.2f)\n", s.x0, s.y0, s.x1, s.y1);
printf ("DRAWING SEGMENT - NR: (%5.2f, %5.2f) - (%5.2f, %5.2f)\n",
				(0.5 + (l->scale_x*s.x0) + unscaled_transl_x),
				(0.5 + (l->scale_y*s.y0) + unscaled_transl_y),
				(0.5 + (l->scale_x*s.x1)+ unscaled_transl_x),
				(0.5 + (l->scale_y*s.y1)+ unscaled_transl_y) );

		transform_segment (&s, l->scale_x, l->scale_y,
			ddist, cos, sin);
		draw_thin_segment_in_buf (&s,
			unscaled_transl_x, unscaled_transl_y,
			buf, max_x, max_y, value_ptr, data_type);

printf("TRANSL_X: %5.2f, TRANSL_Y: %5.2f\n",unscaled_transl_x, unscaled_transl_y);

if ( (l->cur_seg==0) || (l->cur_seg == (l->max_seg-1)) )
printf ("DRAWING SEGMENT - R : (%5.2f, %5.2f) - (%5.2f, %5.2f)\n",
				(0.5 + s.x0 + unscaled_transl_x),
				(0.5 + s.y0 + unscaled_transl_y),
				(0.5 + s.x1+ unscaled_transl_x),
				(0.5 + s.y1+ unscaled_transl_y) );

	}
}

*/
