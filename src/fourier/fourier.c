/* fourier.c - Functions for managing the Geomorph Fourier Explorer, 
 * 	       exclusive of the dialog
 *
 * Copyright (C) 2005 Patrice St-Gelais
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
#include "fourier.h"
#include "fft.h"
#include <complex.h>
#include "../hf/hf.h"
#include "../hf/hf_calc.h"

#define SCALE_MODULE(value) pow((value),0.1)
#define SCALE_REAL(value) pow((value),0.5)
#define SCALE_IMAGINARY(value) pow((value),2.0)

fourier_struct *fourier_struct_new ( ) {
	fourier_struct *fs;
	fs = (fourier_struct *) x_calloc(1,sizeof(fourier_struct), "fourier_struct");
	fs->r_max = fs->i_max = pow(G_MAXDOUBLE/2.0,0.5); // Because the module <= G_MAXDOUBLE
	fs->r_min = fs->i_min = -fs->max_x;
	fs->n_max = G_MAXDOUBLE; // The norm or module is always positive
	fs->n_min = 0.0;
	fs->current_view = MODULE;
	fs->mask = NULL;
	fs->ffs = fourier_filter_new();
	fourier_filter_init (fs->ffs, NO_FOURIER_FILTER, 10.0, 10.0, 0.0, 0.0, 100.0, 100.0);
	fs->tmp_buf = NULL;
	fs->tmp_buf_inv = NULL;
	return fs;	
}

void fourier_struct_free (fourier_struct *fs) {
	if (!fs)
		return;
	if (fs->in)
		x_free(fs->in);
	if (fs->out)
		x_free(fs->out);
	if (fs->tmp_buf)
		x_free(fs->tmp_buf);
	if (fs->tmp_buf_inv)
		x_free(fs->tmp_buf_inv);
	fourier_filter_free(fs->ffs);
	fs->in = NULL; 
	fs->out = NULL; 
	fs->tmp_buf = NULL;
	fs->tmp_buf_inv = NULL;
	x_free(fs);
}

// In fourier_struct_init, data_type describes the type of input_grid
// Basically this function computes fs->in with a direct FFT
// and store max_x, max_y
// Important: max_x and max_y must be a power of 2 (Geomorph standard)

void fourier_struct_init (fourier_struct *fs, gpointer input_grid, gint max_x, gint max_y, gint data_type) {
	gint i;
	if ( (!fs->in) || (fs->max_x!=max_x) || (fs->max_y!=max_y)) {
		if (fs->in)
			x_free(fs->in);
		if (fs->out)
			x_free(fs->out);
		if (fs->finv)
			x_free(fs->finv);
		fs->out = (double complex *) x_calloc(sizeof(double complex), max_x*max_y, "double_complex (fs->out in fourier_struct_init)");
		fs->in = (double complex *) x_calloc(sizeof(double complex), max_x*max_y, "double_complex (fs->in in fourier_struct_init)");
		fs->finv = (double complex *) x_calloc(sizeof(double complex), max_x*max_y, "double_complex (fs->finv in fourier_struct_init)");
		fs->max_x = max_x;
		fs->max_y = max_y;
		fs->tmp_buf = (unsigned short int*) x_malloc(sizeof(unsigned short int)* fs->max_x * fs->max_y, "unsigned short int (fs->tmp_buf in fourier_struct_init)");
		fs->tmp_buf_inv = (unsigned short int*) x_malloc(sizeof(unsigned short int)* fs->max_x * fs->max_y, "unsigned short int (fs->tmp_buf_inv in fourier_struct_init)");
	}
	// fs->in is used as a temporary buffer, containing
	// a double complex version of the input grid
	for (i=0; i<(max_x*max_y); i++) { 
		switch (data_type) {
		case GDOUBLE_ID:
			*(fs->in+i) = (double) *(((gdouble *) input_grid)+i);
			break;
		case HF_TYPE_ID:
			*(fs->in+i) = (double) *(((hf_type *) input_grid)+i);
			break;
		case GINT_ID:
			*(fs->in+i) = (double) *(((gint *) input_grid)+i);
			break;
		case UNSIGNED_CHAR_ID:
			*(fs->in+i) = (double) *(((unsigned char *) input_grid)+i);
		}
	}
	FFT2D_with_minmax(fs->in, fs->out, max_x, max_y, DIRECT, &fs->r_min, &fs->r_max, &fs->i_min, &fs->i_max, &fs->n_min, &fs->n_max);
	// fs->in should contain the unfiltered Fourier transform
	swap_buffers((gpointer *)&fs->in, (gpointer *)&fs->out);
	// Initialize the mask (filter)
	if (fs->mask)
		x_free(fs->mask);
	fs->mask = (gdouble *) x_malloc(sizeof(gdouble) * max_x*max_y, "gdouble (fs->mask in fourier_struct_init)");
	for (i=0; i<fs->max_x*fs->max_y; i++)
		*(fs->mask+i) = 1.0;
}

void fourier_compute_inverse (fourier_struct *fs) {
	// Returns back to the original image, 
	// by computing an inverse Fourier transform
	// Here we need the minimum value, to adjust the output
	// No scaling is required, though
	// We get negative values when we remove frequencies
	// The resulting range is always <= the original range
	gint i;
	gdouble min;
	FFT2D_with_minmax(fs->out, fs->finv, fs->max_x, fs->max_y, INVERSE, &min, NULL, NULL, NULL, NULL, NULL);
	// Adjust the output to get only positive values
//	printf("MIN - FINV: %f\n",min);
	if (min>=0.0)
		return;
	for (i=0; i<(fs->max_x*fs->max_y); i++)
		*(fs->finv+i) = *(fs->finv+i)-min;
}
	
// Converting fs->out to displayable / exportable formats
// data_type is #defined in hf.h

void convert_complex (fourier_struct *fs, gpointer output_grid, gint data_type, gint conversion_type, gint buffer_type) {
	// Converts a complex buffer to a displayable format
	// We must scale the data, and translate it to center the 0 frequency
	gint i, j ,half_x, half_y, index, indexw;
	gdouble ratio,min,max,value, scaled_gap; //max_val=0.0,min_val=0.0;
	double complex *buffer;
	if (buffer_type==F_INPUT_BUFFER)
		buffer = fs->in;
	else
		buffer = fs->out;

	half_x = fs->max_x>>1;
	half_y = fs->max_y>>1;
	switch (conversion_type) {
	case MODULE:
		min = fs->n_min;
		max = fs->n_max;
		scaled_gap = SCALE_MODULE(max - min);	
//		printf("MODULE --- Min: %f; Max: %f; Scaled gap: %f; Ratio: %f\n", min, max, scaled_gap, ratio);
		break;
	case IMAGINARY:
		min = fs->i_min;
		max = fs->i_max;
		scaled_gap = SCALE_IMAGINARY(max - min);	
//		printf("IMAGINARY --- Min: %f; Max: %f; Scaled gap: %f; Ratio: %f\n", min, max, scaled_gap, ratio);
		break;
	default: // Real
		min = fs->r_min;
		max = fs->r_max;
		scaled_gap = SCALE_REAL(max - min);	
//		printf("REAL --- Min: %f; Max: %f; Scaled gap: %f; Ratio: %f\n", min, max, scaled_gap, ratio);
	}
	switch (data_type) {
	case GDOUBLE_ID: // No scaling takes place for double
		ratio = 1.0;
		break;
	case GINT_ID:
		ratio = (((gdouble) G_MAXINT) - ((gdouble) G_MININT)) / scaled_gap; 
		break;
	case HF_TYPE_ID: // Gint allows negative values
		ratio = ((gdouble) MAX_HF_VALUE) / scaled_gap; 
		break;
	default: // case UNSIGNED_CHAR_ID:
		ratio = ((gdouble) 0xFF) / scaled_gap;
	}
	
//	printf("Min: %f; Max: %f; Scaled gap: %f; Ratio: %f\n", min, max, scaled_gap, ratio);
	for (i=0; i<fs->max_y; i++) 
	   for (j=0; j<fs->max_x;j++) { 
	   	index = i*fs->max_x + j;
		indexw = (WRAP(i+half_y,fs->max_y)*fs->max_x)+WRAP(j+half_x,fs->max_x);
		switch (conversion_type) {
		case MODULE:
			value = SCALE_MODULE(cnorm(buffer[indexw]) - min) * ratio;
			break;
		case IMAGINARY:
			value = SCALE_IMAGINARY(cimag(buffer[indexw]) - min) * ratio;
			break;
		default: // Real
			value = SCALE_REAL(creal(buffer[indexw]) - min) * ratio;
		}
/*
		if (value<min_val)
			min_val = value;
		if (value>max_val)
			max_val = value;
*/
		switch (data_type) {
		case GDOUBLE_ID:
			*(((gdouble *) output_grid)+index) = value;
			break;
		case HF_TYPE_ID:
			*(((hf_type *) output_grid)+index) = (hf_type) MIN( (gdouble) MAX_HF_VALUE, MAX(0.0,value));
			break;
		case GINT_ID:
			*(((gint *) output_grid)+index) = (gint) MIN( (gdouble) G_MAXINT, MAX(G_MININT,value));
			break;
		default: // case UNSIGNED_CHAR_ID:
			*(((unsigned char *) output_grid)+index) = (unsigned char) MIN( (gdouble) 0xFF, MAX(0.0,value));
			break;
		}
		
	}
//	printf("MIN VAL: %7.2f; MAX VAL: %7.2f\n",min_val, max_val);
}

void convert2module (fourier_struct *fs, gpointer output_grid, gint data_type, gint buffer_type) {
	convert_complex (fs, output_grid, data_type, MODULE, buffer_type);
}

void convert2real (fourier_struct *fs, gpointer output_grid, gint data_type, gint buffer_type) {
	convert_complex (fs, output_grid, data_type, REAL, buffer_type);
}

void convert2imag (fourier_struct *fs, gpointer output_grid, gint data_type, gint buffer_type) {
	convert_complex (fs, output_grid, data_type, IMAGINARY, buffer_type);
}


void convert_finv (fourier_struct *fs, gpointer output_grid, gint data_type) {
	// Convert the Fourier inverse buffer to some displayable data type
	// For all integer types (all except GDOUBLE), we assume that
	// the values are in the 0 - 0xFFFF range
	// *** Needs to be generalized in the future
	gint i,j,indx;
	gdouble value;
	for (i=0; i<fs->max_y; i++) 
	   for (j=0; j<fs->max_x;j++) { 
	   	indx = i*fs->max_x + j;
		value = fs->finv[indx];
		switch (data_type) {
		case GDOUBLE_ID:
			*(((gdouble *) output_grid)+indx) = value;
			break;
		case HF_TYPE_ID:
			*(((hf_type *) output_grid)+indx) = MIN( (gdouble) MAX_HF_VALUE, MAX(0.0,value));
			break;
		case GINT_ID:
			*(((gint *) output_grid)+indx) = MIN( (gdouble) G_MAXINT, MAX(G_MININT,value));
			break;
		default: // case UNSIGNED_CHAR_ID:
			*(((unsigned char *) output_grid)+indx) = MIN( (gdouble) 0xFF, MAX(0.0,value/256.0));
			break;
		}
		
	}
}

// Filter management functions

fourier_filter_struct *fourier_filter_new() {
	return (fourier_filter_struct *) x_calloc (sizeof(fourier_filter_struct),1, "fourier_filter_struct");
}

void fourier_filter_free(fourier_filter_struct *ffs) {
	if (!ffs)
		return;
	x_free(ffs);
}

// Initializes filter with default data (relative - percent data, independent of the image size)
void fourier_filter_init(fourier_filter_struct *ffs, 
	gint filter_type,
	gfloat size_x, gfloat size_y, 
	gfloat translate_x, gfloat translate_y,
	gfloat band_width, gdouble amplitude) {
	
	ffs->size_x = MIN(100.0,MAX(0.1,size_x));
	ffs->size_y = MIN(100.0,MAX(0.1,size_y));
	ffs->translate_x = MIN(100.0,MAX(-100.0,translate_x));
	ffs->translate_y = MIN(100.0,MAX(-100.0,translate_y));
	ffs->band_width = MIN(100.0,MAX(0.0,band_width));
	ffs->amplitude = MIN(200.0,MAX(50.0,amplitude));
	ffs->xy_synchro = TRUE;
	switch (filter_type) { // Basic existence check
		case F_BOX:
		case F_BOX_INVERTED:
		case F_EMPTY_BOX:
		case F_EMPTY_BOX_INVERTED:
		case F_BELL:
		case F_BELL_INVERTED:
		case F_EMPTY_BELL:
		case F_EMPTY_BELL_INVERTED:
			ffs->filter_type = filter_type;
			break;
		default:
			ffs->filter_type = NO_FOURIER_FILTER;
		
	}
	ffs->filter_type = filter_type;
}

// Returns a grid to multiply/merge with the Fourier transform
gdouble *fourier_filter_bell_grid (fourier_filter_struct*ffs, gint max_x, gint max_y){
	return NULL;
}

gboolean test_inside_box (gint x, gint y, gint start_x, gint start_y, gint length_x, gint length_y, gint max_x, gint max_y, gint translate_x, gint translate_y) {
	// Test if (x,y) is inside the box delimited by 
	// (start_x+translate_x, start_y+translate_y) and
	// (start_x+length_x+translate_x, start_y+length_y+translate_y)
	// We are in a "wrapping" world - we add max_x/y to deal with
	// negative numbers (negative translation being possible)
	gint xx,yy;
	xx = x%max_x;
	yy = y%max_y;
	if (	(xx>((max_x+start_x+translate_x)%max_x)) &&
		(xx<=((max_x+start_x+translate_x+length_x)%max_x)) && 
		(yy>((max_y+start_y+translate_y)%max_y)) &&
		(yy<=((max_y+start_y+translate_y+length_y)%max_y))
		) {
		return TRUE;
	}
	return FALSE;
}

void instantiate_filter_coordinates (gfloat size, gfloat band_width, gint max, gint *start, gint *length, gint *band, gint *start_band) {
	// Convert the box size in relative value (%) to absolute coordinates
	// Length is always odd, so that it is symmetrical around its pivot,
	// usually 0,0
//	printf("Instantiate coordinates\n");
	(*length) = MIN(max-1,1 + (2 * (gint) (((gfloat) max)*size/200.0)));
	(*start) = (gint) (((gfloat) max)*((100.0-size)/200.0));
	if (0.0 == size) // boundary case...
		(*start)--;
	(*band) = (gint) ((band_width/100.0) * (gdouble) (max  - *length));
//	(*start_band) = (*start) + (((*length)-(*band))/2);
	(*start_band) = (*start) - ((*band)/2);
//	printf("SIZE: %7.2f; MAX: %d; START: %d; LENGTH: %d\n",size,max,*start,*length); 
//	printf("Band width: %f; band: %d; start_band: %d\n",band_width, *band, *start_band);
}

void instantiate_filter (fourier_filter_struct *ffs, gint max_x, gint max_y, gdouble *mask) {
	// Instantiate filter as a mask
	// Mask values range from 0.0 to +1.0 (0 % to +100 %) X amplitude
	gint start_x, start_y, length_x, length_y, translate_x, translate_y;
	gint x, y, band_x, band_y, start_band_x, start_band_y;
	gdouble amplitude;
	gboolean test;
	if (!mask)
		return;
	// We move the FT by half the size of the grid when displaying it,
	// so that the 0,0 point is in the center of the image (lowest frequency)
	// The filter box is centered on this point, for the user
	// We add max_x/y to deal with negatives translation values (always>=-max - remember that we are in a "wrapping" world)

	// Translate Y is reverted (more intuitive)
	translate_x = max_x + (gint) (((gfloat) max_x)*ffs->translate_x/100.0);
	translate_y = max_y - (gint) (((gfloat) max_y)*ffs->translate_y/100.0);

	start_x = start_y = start_band_x = start_band_y = 0;
	length_x = max_x;
	length_y = max_y;
	instantiate_filter_coordinates(ffs->size_x, ffs->band_width, max_x, &start_x, &length_x, &band_x, &start_band_x);
	instantiate_filter_coordinates(ffs->size_y, ffs->band_width, max_y, &start_y, &length_y, &band_y, &start_band_y);
	
//	printf("INSTANTIATE_FILTER: MAX (X,Y): (%d,%d), Start: (%d,%d), Length: (%d,%d), Start band: (%d,%d); Band width: (%d,%d); \n", max_x, max_y, start_x, start_y, length_x, length_y, start_band_x, start_band_y, band_x, band_y);
	
	amplitude = 1.0 - (ffs->amplitude / 100.0);
	
	switch (ffs->filter_type) {
		case F_BOX:
			for (y=0;y<max_y;y++)
				for (x=0;x<max_x;x++) {

					if (test_inside_box (x, y, start_x, start_y, length_x, length_y, max_x, max_y, translate_x, translate_y)) 
						*(mask+y*max_x+x) = 1.0;
					else
						*(mask+y*max_x+x) = amplitude;
				}
			break;
		case F_BOX_INVERTED:
			for (y=0;y<max_y;y++)
				for (x=0;x<max_x;x++) {
					if (test_inside_box (x, y, start_x, start_y, length_x, length_y, max_x, max_y, translate_x, translate_y))
						*(mask+y*max_x+x) = amplitude;
					else
						*(mask+y*max_x+x) = 1.0;
				}
			break;
		case F_EMPTY_BOX: 
			// "Banded" filter
			for (y=0;y<max_y;y++)
				for (x=0;x<max_x;x++) {
	
					test = test_inside_box ( x, y, start_band_x, start_band_y, MIN(length_x+band_x,max_x-1), MIN(length_y+band_y,max_y-1), max_x, max_y, translate_x, translate_y);

					if ( test && !test_inside_box (x, y, start_x, start_y, length_x, length_y, max_x, max_y, translate_x, translate_y))
						*(mask+y*max_x+x) = 1.0;
					else
						*(mask+y*max_x+x) = amplitude;
				}
			break;
		case F_EMPTY_BOX_INVERTED:
			// "Banded" filter
			for (y=0;y<max_y;y++)
				for (x=0;x<max_x;x++) {
					
					test = !test_inside_box ( x, y, start_band_x, start_band_y, MIN(length_x+band_x,max_x-1), MIN(length_y+band_y,max_y-1), max_x, max_y, translate_x, translate_y);
					
					if ( test || test_inside_box (x, y, start_x, start_y, length_x, length_y, max_x, max_y, translate_x, translate_y) )
						*(mask+y*max_x+x) = 1.0;
					else
						*(mask+y*max_x+x) = amplitude;
				}
			break;
		case F_BELL:
		//	break;
		case F_BELL_INVERTED:
		//	break;
		case F_EMPTY_BELL:
		//	break;
		case F_EMPTY_BELL_INVERTED:
		//	break;
		default: // Unknown value = NO_FOURIER_FILTER

			for (y=0;y<max_x*max_y;y++)
				*(mask+y) = 1.0;
	} // Switch end
};


void fourier_apply_filter (fourier_struct *fs) {
	
// This function computes fs->out, applying the current fourier filter
// Depending on it, we apply the filter to the whole complex matrix,
// to the real part or to the imaginary part

	gint x, y, i, im, half_x, half_y;

	if (!fs->out) // Not supposed to happen at this point...
		fs->out = (double complex *) x_calloc(sizeof(double complex), fs->max_x*fs->max_y, "double complex (fs->out in fourier_apply_filter)");

// printf("FOURIER_APPLY_FILTER; filter_type : %d\n", fs->ffs->filter_type);

	if ((fs->ffs->filter_type==NO_FOURIER_FILTER) || !(fs->mask)) {
		memcpy(fs->out, fs->in, sizeof(double complex)*fs->max_x*fs->max_y);
		return;
	}
	half_x = fs->max_x>>1;
	half_y = fs->max_y>>1;
	switch (fs->current_view) {
		case REAL:
			for (x=0; x<fs->max_x; x++)
				for (y=0; y<fs->max_y; y++) {
					i = x + y*fs->max_x;
					// We need to translate back the mask
					// before applying it to the
					// untranslated fourier buffer
					im = ((x+half_x)%fs->max_x) + ((y+half_y)%fs->max_y)*fs->max_x;		
					*(fs->out+i) = *(fs->mask+im) * creal(*(fs->in+i)) + I * cimag(*(fs->in+i));
				}
			break;
		case IMAGINARY:
			for (x=0; x<fs->max_x; x++)
				for (y=0; y<fs->max_y; y++) {
					i = x + y*fs->max_x;
					im = ((x+half_x)%fs->max_x) + ((y+half_y)%fs->max_y)*fs->max_x;		
					*(fs->out+i) = creal(*(fs->in+i)) + *(fs->mask+im) * I * cimag(*(fs->in+i));
				}
			break;
		case MODULE:
		default:
			for (x=0; x<fs->max_x; x++)
				for (y=0; y<fs->max_y; y++) {
					i = x + y*fs->max_x;
					im = ((x+half_x)%fs->max_x) + ((y+half_y)%fs->max_y)*fs->max_x;		
					*(fs->out+i) = *(fs->in+i) * *(fs->mask+im);
			}
	}	
}
