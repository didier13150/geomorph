/* hf_calc.h - headers for hf transformation utilities
 *
 * Copyright (C) 2001-2010 Patrice St-Gelais
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _HF_CALC
#define _HF_CALC 1

#include "globals.h"
#include "hf.h"
#include "merge.h"
#include <math.h>

//	TEST... looking for a way to display the 16 bits lower heights 
//	in a 8 bits scale...  See hf_16_to_8 in hf.c
#ifndef BLACK_THRESHOLD
#define BLACK_THRESHOLD 5.0
#endif

// Data types id
#ifndef GDOUBLE_ID
#define GDOUBLE_ID 0
#endif

#ifndef HF_TYPE_ID
#define HF_TYPE_ID 1
#endif

// Same as HF_TYPE
#ifndef UNSIGNED_SHORT_ID
#define UNSIGNED_SHORT_ID 1
#endif

#ifndef GINT_ID
#define GINT_ID 2
#endif

#ifndef UNSIGNED_CHAR_ID
#define UNSIGNED_CHAR_ID 3
#endif

#ifndef PIX8_RGB_ID
#define PIX8_RGB_ID 4
#endif

#ifndef PIX16_RGB_ID
#define PIX16_RGB_ID 5
#endif

// "E"
#ifndef CONST_E
#define CONST_E 2.718281828
#endif

#define hf_type_avrg(v1,v2) (hf_type)((((glong)(v1))+(glong)(v2))/2)
//	Kind of bell curve, for using in "shape filters" and so on

// A cleaned version of the formula:
// 1 / pow(  base, pow(2*index/size ,exp) / exp) 
// The derived exponent to "base" is divided by "exp"
// When base == CONST_E and exp == 2, 
// the formula gives a centered but not reduced gaussian (normal) bell
// One has to divide the result by pow(PI,0.5) to get a centered and reduced bell
// Actually we divide the independent variable (index) by half the radius 
// so that the standard deviation is equal to half the radius, and we cover 95% of
// the values with the whole radius (given exp==2)
// The non reduced formula gives 1.0 at x==0.

//	How to control the functions:  
//		exponent = "hardness" of the edges - greater = steepier
//		size = relative width of the bell... start with something like half the actual radius (exponent = 2)
//		it's probably not necessary to fiddle with "base" (check filters!)

#define BELL(base,exp,index,size) (gfloat)(1.0/ pow((gfloat)(base),  pow(2.0*((gfloat)(index))/(gfloat)(size), (exp) ) / (exp)   )  )

#define BELLD(base,exp,index,size) (gdouble)(1.0/ pow((gdouble)(base),  pow(2.0*((gdouble)(index))/(gdouble)(size), (exp) ) / (exp)   )  )

//	Wrapping... ("tiling")
#define WRAP(x,max) (((x)+(max))%(max))

//	Case where x could be negative
#define WRAP2(x,max) ( ((x)<0) ? ((max)-WRAP(ABS(x),(max))) : WRAP((x),(max))  )

//	DWRAP is a float version of WRAP2
#define DOUBLEWRAP(x,max) ( ((x)<(max)) ? (x) : (x)-( ((gdouble) (max)) * floor((x)/(max)) ) )
#define DWRAP(x,max)  ( ((x)<0) ? ((max)-DOUBLEWRAP(ABS(x),(max))) : DOUBLEWRAP((x),(max))  )

//	An alternative to WRAP for processing overflowed indexes...
//	X should be in base 0, and ranges from 0 to size-1
//	Old version is valid only between -(size-1) and (2*size-1)
// #define REBOUND(x,size) ( ((x)>=size) ? (2*size-(x)-1) : ABS(x) )
// New version 2004-04-11, valid for any value (indexes follow a "triangle" wave)
#define REBOUND(x,size) ( ((ABS(x)%(2*size))<size) ? (ABS(x)%size) :  (2*size - (ABS(x)%(2*size)) - 1))

//	This macro returns the length of the intersection between two segments of an axis
//	Allows calculating the area of the intersection between two rectangles
//	No intersection = negative result - so we MAX the result to 0
#define INTERSECT(x1,size1,x2,size2) (MAX(0,MIN((x1)+(size1),(x2)+(size2))-MAX((x1),(x2))))

//	Redefinition of shift allowing negative values
//	 - interpreted as the reverse shift
#define RIGHT_SHIFT(value,shift) ( ((shift)>=0) ? (value)>>(shift) : (value)<<(-(shift)) )
#define LEFT_SHIFT(value,shift) ( ((shift)>=0) ? (value)<<(shift) : (value)>>(-(shift)) )

// 	As the name says...
#define FILL_MAP(map,size,value) for(;size-1;size--) *(map+size-1)=value

#define HF_BLACK 0x0000
#define HF_WHITE 0xFFFF

//	Merge operations, for filters and paint strokes

//	MULTIPLY: transform the multiplier as a ratio between 0.0 and 1.0
#ifndef MULTIPLY
	#define MULTIPLY 1
#endif

//	MULTIPLY2: transform the multiplier as a ratio between 1.0 and 2.0
#ifndef MULTIPLY2
	#define MULTIPLY2 2
#endif

#ifndef ADD
	#define ADD 3
#endif
#ifndef SUBTRACT
	#define SUBTRACT 4
#endif
#ifndef MIN_MERGE
	#define MIN_MERGE 5
#endif
#ifndef MAX_MERGE
	#define MAX_MERGE 6
#endif
#ifndef XOR_MERGE
	#define XOR_MERGE 7
#endif

#ifndef POWER_OP
	#define POWER_OP 8
#endif
#ifndef LOG_OP
	#define LOG_OP 9
#endif
#ifndef EXP_OP
	#define EXP_OP 10
#endif
#ifndef BASE_OP
	#define BASE_OP 11
#endif
#ifndef SINE_OP
	#define SINE_OP 12
#endif
#ifndef SMOOTH_OP
	#define SMOOTH_OP 13
#endif
#ifndef ALT_MERGE
	#define ALT_MERGE 14
#endif

#ifndef MERGE_HARDNESS
	#define MERGE_HARDNESS 0.15
#endif

//	Cache of normalized gaussian vectors, generated by normalized_bell_new
#ifndef GAUSS_LIST_LENGTH
	#define GAUSS_LIST_LENGTH 37
#endif

#ifndef TILING_AUTO
#define TILING_AUTO 0
#endif
#ifndef TILING_YES
#define TILING_YES 1
#endif
#ifndef TILING_NO
#define TILING_NO 2
#endif

//	Parameters for overflow index processing
//	in functions like interpolate and similar

//	WRAP: max+1 -> 0, max+2 ->1...; -1->max, -2->max-1
//	REBOUND: max+1 -> max-1, -1->1, -2->2...
//	ZERO: outbound values set to 0
//	IDLE: max+1, max+2... -> max

#ifndef OVERFLOW_WRAP
#define OVERFLOW_WRAP 1
#endif
#ifndef OVERFLOW_REBOUND
#define OVERFLOW_REBOUND 2
#endif
#ifndef OVERFLOW_ZERO
#define OVERFLOW_ZERO 3
#endif
#ifndef OVERFLOW_IDLE
#define OVERFLOW_IDLE 4
#endif
#ifndef OVERFLOW_OFFSET
#define OVERFLOW_OFFSET 5
#endif
#ifndef OVERFLOW_SCALE
#define OVERFLOW_SCALE 6
#endif

#ifndef NORTH
#define NORTH 0
#endif

#ifndef SOUTH
#define SOUTH 1
#endif

#ifndef EAST
#define EAST 2
#endif

#ifndef WEST
#define WEST 3
#endif


//	The distance matrix (lookup table)
//	We avoid recalculating distances from hf center for each use
//	by storing them.  Could become big for, say, 4096x4096 HF!
//	We store only one quadrant and translate the coordinates with CENTER_XY!

//	The most important rule:
//	At any moment, hf_size for which the distance matrix is computed should be >= 
//		than the hf_size for which the current filter is computed, which should be >=
//		than the hf_size

typedef struct {
	gint hf_size;	// HF size the current structure is able to process
	gint size;	// Vector size = square of the quadrant size
			// Example:  256 for a 16 pixels wide square for a 32 pixels wide HF
	gfloat *distances;  // malloc(size*sizeof(gfloat))
} dist_matrix_struct;

//	This macro translates x,y coordinates relative to 0,0 
//	into absolute coordinates relative to the image center, for calculating distances
//	The result is a X or Y index in a dist_matrix_struct
//	Since we have an even number of pixels, the center is 2 pixels wide
#define CENTER_XY(xy,max)  (((xy)>=((max)>>1))?(xy)-((max)>>1):((max)>>1)-(xy)-1)

//	This macro "vectorizes" x,y coordinates, allowing the use of a * pointer instead of a [x,y] table 
#define VECTORIZE(x,y,max) (((y)*(max))+(x))

//	Absolute value
#ifndef ABS
	#define ABS(v) (((v)<0)?-(v):(v))
#endif

//	Getting the distance between 2 arbitrary points
#define DIST(dm,x1,y1,x2,y2) (*((dm)->distances+VECTORIZE(ABS((x2)-(x1)),ABS((y2)-(y1)),(dm)->hf_size>>1)))
//	Straight version, without lookup table
#define POWDIST(x1,y1,x2,y2) ((x2)-(x1))*((x2)-(x1))+((y2)-(y1))*((y2)-(y1))
#define DIST2(x1,y1,x2,y2) sqrt(POWDIST(x1,y1,x2,y2))

//	Scalar product of 2 vectors given by dots P1, P2, P3
//	Vector 1 = P1-P2; Vector 2 = P3-P2
//	Typically: for calculating the angle of a path
//	If scalar product < 0, angle > 90
//	If scalar product == 0, angle = 90
#define SCALAR_PRODUCT(x1,y1,x2,y2,x3,y3) (((x1)-(x2))*((x3)-(x2))+((y1)-(y2))*((y3)-(y2)))
// Same, with normalized vectors
#define NORM_SCALAR_PRODUCT(x1,y1,x2,y2,x3,y3) (((gdouble)SCALAR_PRODUCT(x1,y1,x2,y2,x3,y3))/((gdouble)(DIST2(x1,y1,x2,y2)*DIST2(x2,y2,x3,y3))))

// Get the absolute slope value from degrees, for a given HF (1 pixel wide)
#ifndef dget_absolute_slope
#define dget_absolute_slope(deg,max) ((gdouble)(tan(PI*((gdouble)deg/180.0))*((gdouble)MAX_HF_VALUE)/(gdouble)max))
#endif

#ifndef iget_absolute_slope
#define iget_absolute_slope(deg,max) ((gint)dget_absolute_slope(deg,max))
#endif

#ifndef alloc_double_hf
#define alloc_double_hf(max) ((gdouble *)malloc(max*max*sizeof(gdouble)))
#endif

#ifndef calloc_double_hf
#define calloc_double_hf(max) ((gdouble *)calloc(max*max,sizeof(gdouble)))
#endif

#ifndef alloc_hf_type
#define alloc_hf_type(max) ((hf_type *)malloc(max*max*sizeof(hf_type)))
#endif

#ifndef calloc_hf_type
#define calloc_hf_type(max) ((hf_type *)calloc(max*max,sizeof(hf_type)))
#endif

#ifndef xalloc_double_hf
#define xalloc_double_hf(max,string) ((gdouble *)x_malloc(max*max*sizeof(gdouble),string))
#endif

#ifndef xcalloc_double_hf
#define xcalloc_double_hf(max,string) ((gdouble *)x_calloc(max*max,sizeof(gdouble),string))
#endif

#ifndef xalloc_hf_type
#define xalloc_hf_type(max,string) ((hf_type *)x_malloc(max*max*sizeof(hf_type),string))
#endif

#ifndef xcalloc_hf_type
#define xcalloc_hf_type(max,string) ((hf_type *)x_calloc(max*max,sizeof(hf_type),string))
#endif

#ifndef hf_min_merge

#define hf_min_merge(out,in1,in2,length) gint _i;for (_i=0;_i<length;_i++) *(out+_i)=MIN(*(in1+_i),*(in2+_i))

#endif

#ifndef H_AXIS
#define H_AXIS 0
#endif

#ifndef V_AXIS
#define V_AXIS 1
#endif

// 	Standard vector with room for length
typedef struct {
	gfloat x;
	gfloat y;
	gfloat z;
	gfloat l;
} vector;

//	Prototypes

void hf_merge(	hf_struct_type *from_hf,
		hf_struct_type *into_hf,
		gint x, gint y,		// "from_hf" is centered at "into_hf(x,y)"
		gint merge_action, 	// ADD, SUBTRACT
		gboolean if_tiles,
		gfloat hf_displacement,
		gboolean normalize);

// Same as hf_merge, decomposing the hf_struct_type* of the pen and the background
void hf_merge_buf (hf_type *hf_pencil, gint pen_max_x, gint pen_max_y,
		hf_type *hf, gint max_x, gint max_y,
		int x, int y,
		gint pen_merge,
		gboolean pen_tiles,
		gfloat h_displacement,
		gboolean normalize) ;

void generalized_merge( gpointer map, 
	gint map_data_type,
	gint size_x, gint size_y,
	gpointer hf, 
	gint hf_data_type, 
	gint max_x, gint max_y, 
	gint x, gint y,
	gint merge_type,
	gboolean wrap,
	gboolean square_symmetry);
	
void interpolated_merge (gpointer map,
	gint map_data_type, 
	gint size_x, gint size_y, 
	gpointer hf, 
	gint hf_data_type, 
	gint max_x, gint max_y,
	gdouble x, 
	gdouble y,
	gint merge_type, 
	gboolean wrap, 
	gboolean square_symmetry);
	
void hf_slide(hf_struct_type *hf, gint slideh, gint slidev) ;

void interpolate (gdouble dbx, gdouble dby, gpointer grid,
	gint x_size, gint y_size, gpointer return_value_ptr, gint data_type,
	gint overflow);

void translate_real_forward_mapping (gpointer source_grid,
	gpointer output_grid,
	gint data_type,
	gint x_size, gint y_size,
	gint x,gint y,
	gdouble ox, gdouble oy) ;

void translate_forward_mapping (gpointer source_grid,
	gpointer output_grid,
	gint data_type,
	gint x_size, gint y_size,
	gint x,gint y,
	gint ix, gint iy) ;

// Old version - use "interpolate" instead
hf_type interpolate2 (gdouble dbx, gdouble dby, hf_type *hf, gint x_size, gint y_size);

hf_type interpolate_get_xy (gdouble x, gdouble y, hf_type *hf,
	gint x_size, gint y_size,
	gdouble (*get_x) (gdouble, gpointer), gdouble (*get_y) (gdouble, gpointer),
	gpointer arg_get_x, gpointer arg_get_y);

void vector_interpolate (gdouble dbx, gpointer vector, gint x_size,
	gpointer return_value_ptr, gint data_type, gint overflow) ;

void rotate (gdouble dx, gdouble dy, gpointer map_in, gpointer map_out,
	gint hsize, gint vsize, gint data_type, gint overflow);

void rotate_sin_cos (gdouble sin, gdouble cos, gpointer map_in, gpointer map_out, gint hsize, gint vsize, gint data_type, gint overflow);
	
void hf_rotate (hf_type *hf_in, hf_type *hf_out, gint hf_size, gint angle, 
	gboolean wrap, gint overflow);
void hf_fast_rotate (hf_type *hf_in, hf_type *hf_out, gint hf_size, gint angle);

void pow2_scale_grid (gpointer in, gpointer out, gint max_x, gint max_y, gint log_ratio, gint data_type_in, gint data_type_out);

void pow2_scale_uchar_grid_with_mask (gpointer in, unsigned char *out, gint max_x, gint max_y, gint log_ratio, gint data_type_in, gdouble *mask, gpointer mask_value_ptr);

hf_struct_type * hf_scale (hf_struct_type *hf_in, gint log_ratio);

void hf_horizontal_mirror(hf_struct_type *hf);
void hf_vertical_mirror(hf_struct_type *hf);
void hf_circular_projection(hf_type *hf_in, hf_type *hf_out, gint hf_size);
void hf_revert(hf_struct_type *hf);

void norm(vector *v);

// void hf_save(hf_struct_type *, gchar *);

dist_matrix_struct *dist_matrix_new(gint hf_size);
void dist_matrix_init(dist_matrix_struct *, gint hf_size);
gdouble *normalized_bell_new(gint radius);
void convolve_normalized_vector (gpointer in,gpointer out, gint max_x, gint max_y, gboolean wrap,
		gint radius, gdouble *vector, gint data_type) ;
void map_convolve (hf_type *map, gint map_max_x, gint map_max_y,
	hf_type *background, gint max_x, gint max_y, gint cx, gint cy,
	gboolean wrap, gint level, gdouble **gauss_list, gboolean square_symmetry) ;

gboolean intersect_rect (gint startx_rect, gint starty_rect, gint endx_rect, gint endy_rect,
	gint x0, gint y0, gint x1, gint y1, 
	gdouble *startx, gdouble *starty, gdouble *endx, gdouble *endy);

void fill_area (hf_type *buffer_in, hf_type *buffer_out, gint xmax, gint ymax,
	hf_type filling_value, hf_type range, gint x, gint y);

gpointer hexagonal_row_sampling_with_type (gpointer hf, gint hf_size, 
	gboolean wrap, gboolean even, gboolean shift_right, gint data_type, gint axis) ;
		
hf_type *hexagonal_row_sampling (hf_type *hf, gint hf_size, 
	gboolean wrap, gboolean even, gboolean shift_right) ;
	
void add_spread_3x3 (hf_type *hf, gint max, gint x, gint y, gint value, gboolean wrap);

void hf_clamp (hf_struct_type *hf, hf_type min, hf_type max);
void hf_clamp_buffer (hf_type *buffer, gint length, 
		hf_type newmin, hf_type newmax);
void hf_double_clamp (hf_struct_type *hf, hf_type min, hf_type max, gdouble *buffer);
void double_clamp (hf_type *buf, gint max_x, gint max_y, hf_type newmin, hf_type newmax, gdouble *dbuf);
void hf_type_to_double (hf_type *hf_buf, gint max_x, gint max_y, gdouble *doublebuf);
void hf_type_to_double_scaled (hf_type *hf_buf, gint max_x, gint max_y, gdouble *doublebuf);

void hf_subtract(hf_type *hf1, hf_type *hf2, hf_type *result, gint length, gint behaviour);

#endif // _HF_CALC
