/**************************************************************************

    Headers converted from kfourier by Patrice St-Gelais, december 2005
    Copyright (C) 1998  Antonio Larrosa Jimenez

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    Send comments and bug fixes to antlarr@arrakis.es
    or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain

***************************************************************************/
#ifndef FFT_H
#define FFT_H
#include <stdlib.h>
#include <complex.h>

#ifndef u_int
	#define u_int unsigned int
#endif

#ifndef u_char
	#define u_char unsigned char
#endif

// Module computation

#define cnorm(c) sqrt(pow(creal(c),2.0)+pow(cimag(c),2.0))

// How we apply the fourier transform

#ifndef INVERSE
#define INVERSE 0
#endif

#ifndef DIRECT
#define DIRECT 1
#endif

#ifndef MAX_HF_VALUE
#define MAX_HF_VALUE 0xFFFF
#endif

int is2pow(int i);
// Returns i if i is a power of 2 or the smaller power of 2
// that is larger than i

void FFT(double complex *data, int Pwr, int Dir); // 1D FFT

// For the next functions: if out is NULL, the functions allocate it and return the pointer
// Otherwise, they do nothing and return the out pointer

double complex * FFT2D(double complex *in,double complex *out, int w,int h, int Dir);

double complex * FFT2D_with_minmax(double complex *in, double complex *out, int w,int h, int Dir, double *rmin, double *rmax, double *imin, double *imax, double *nmin, double *nmax);

double complex * FFT2D_real_i2c(u_int *in, double complex *out, int w,int h, int Dir);

u_int * complex_to_u_int (double complex *in, u_int *out, int w_in, int h_in, int w_out, int h_out);
u_int * complex_to_u_int_clamp (double complex *in, u_int *out, int w_in, int h_in, int w_out, int h_out);
u_int* complex_to_u_int_clamp_translate (double complex *in, u_int *out, int w_in, int h_in, int w_out, int h_out, int tr_x, int tr_y);

#endif
