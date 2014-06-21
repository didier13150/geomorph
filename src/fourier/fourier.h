/**************************************************************************

    Copyright (C) Patrice St-Gelais, december 2005
         patrstg@users.sourceforge.net
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#ifndef FOURIER_H
#define FOURIER_H
#include <complex.h>
#include <glib.h>

typedef struct {

// Gfloat data convert to integer coordinates
// Gdouble data: to multiply with double data from complex grids

	gint filter_type; // See #defines
	gfloat size_x; // Linear size of the boxed area, around the center, in 0,1 %, from 0,1 to 100,0 %
	gfloat size_y; // In 0,1 %, from 0,1 to 100,0 %
	gboolean xy_synchro;
	gfloat translate_x; // In 0,1 %, from -100,0 % to 100,0 %
	gfloat translate_y; // In 0,1 %, from -100,0 % to 100,0 %
	gfloat band_width; // In 0,1 %, from 0,1 % to 100,0 % of the filter size
	gdouble amplitude; // In 0,1 %, from 50% to 200 %
} fourier_filter_struct;

typedef struct {
	double complex *in; // Frequency spectrum
	double complex *out; // Filtered frequency spectrum
	double complex *finv; // Inverse Fourier transform
	gint max_x; // Size
	gint max_y;
	// The next values are required to scale the complex grid to a displayable format
	// They must be computed when running the Fast Fourier Tranform
	gdouble r_max;
	gdouble r_min;
	gdouble i_max;
	gdouble i_min;
	// Norm or module (pow(r*r+i*i,0.5))
	gdouble n_max;
	gdouble n_min;
	gint current_view; // REAL, IMAGINARY or MODULE
	unsigned short int *tmp_buf; // For storing the current view
	unsigned short int *tmp_buf_inv; // For storing the current inverse view
	fourier_filter_struct *ffs;
	gdouble *mask; // Instanciation of the filter (max_x*max_y)
} fourier_struct;

// Filters

#ifndef NO_FOURIER_FILTER
#define NO_FOURIER_FILTER 0
#endif

#ifndef F_BOX
#define F_BOX 1
#endif

#ifndef F_BOX_INVERTED
#define F_BOX_INVERTED 2
#endif

#ifndef F_EMPTY_BOX
#define F_EMPTY_BOX 3
#endif

#ifndef F_EMPTY_BOX_INVERTED
#define F_EMPTY_BOX_INVERTED 4
#endif

#ifndef F_BELL
#define F_BELL 5
#endif

#ifndef F_BELL_INVERTED
#define F_BELL_INVERTED 6
#endif

#ifndef F_EMPTY_BELL
#define F_EMPTY_BELL 7
#endif

#ifndef F_EMPTY_BELL_INVERTED
#define F_EMPTY_BELL_INVERTED 8
#endif

// For filter management

#ifndef FILTER_DIRECT
#define FILTER_DIRECT 1
#endif

#ifndef FILTER_INVERTED
#define FILTER_INVERTED 2
#endif

// Conversion type, for convert_complex

#ifndef MODULE
#define MODULE 0
#endif

#ifndef REAL
#define REAL 1
#endif

#ifndef IMAGINARY
#define IMAGINARY 2
#endif

// Fourier transform type

#ifndef INVERSE
#define INVERSE 0
#endif

#ifndef DIRECT
#define DIRECT 1
#endif

// Buffer to process

#ifndef F_INPUT_BUFFER
#define F_INPUT_BUFFER 0
#endif

#ifndef F_OUTPUT_BUFFER
#define F_OUTPUT_BUFFER 1
#endif
/************************************************************************************/

fourier_struct *fourier_struct_new ( );
void fourier_struct_free (fourier_struct *fs);

// In fourier_struct_init, data_type describes the type of input_grid
// Basically this function computes fs->in with a direct FFT 
// and stores max_x, max_y
void fourier_struct_init (fourier_struct *fs, gpointer input_grid, gint max_x, gint max_y, gint data_type);

void fourier_compute_inverse (fourier_struct *fs);

// Converting fs->out to displayable / exportable formats
// data_type is #defined in hf.h

void convert_complex (fourier_struct *fs, gpointer output_grid, gint data_type, gint conversion_type, gint buffer_type);
void convert2module (fourier_struct *fs, gpointer output_grid, gint data_type, gint buffer_type);
void convert2real (fourier_struct *fs, gpointer output_grid, gint data_type,  gint buffer_type);
void convert2imag (fourier_struct *fs, gpointer output_grid, gint data_type, gint buffer_type);

void convert_finv (fourier_struct *fs, gpointer output_grid, gint data_type);

// Filter management functions
fourier_filter_struct *fourier_filter_new();
void fourier_filter_free(fourier_filter_struct *ffs);
// Initializes filter with default data (relative - percent data, independent of the image size)
void fourier_filter_init(fourier_filter_struct *ffs,
	gint filter_type, 
	gfloat size_x, gfloat size_y, 
	gfloat translate_x, gfloat translate_y,
	gfloat band_width, gdouble amplitude);

// Returns a grid to multiply/merge with the Fourier transform
gdouble *fourier_filter_bell_grid (fourier_filter_struct*ffs, gint max_x, gint max_y);

// This function computes fs->out, applying the current fourier filter
void fourier_apply_filter (fourier_struct *fs);

void instantiate_filter (fourier_filter_struct *ffs, gint max_x, gint max_y, gdouble *mask);

#endif
