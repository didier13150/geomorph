/**************************************************************************

    File converted from C++ to C99 by Patrice St-Gelais, december 2005
    From fft1d.cc, fft2d.cc  - The 1D and 2D FFT algorithms, and pow2.h
    Copyright (C) 1998  Antonio Larrosa Jimenez (kfourier)
    

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
    Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

***************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include "fft.h"
#include "math.h"
#include <glib.h>

#include "../hf/hf.h"
#include "../hf/hf_calc.h"

double threshold=64.0;

static double CosArray[28]=
{ /* cos (-2pi/N) for N=2,4,8,...,16384 */
-1.00000000000000,  0.00000000000000,  0.70710678118655,
 0.92387953251129,  0.98078528040323,  0.99518472667220,
 0.99879545620517,  0.99969881869620,  0.99992470183914,
 0.99998117528260,  0.99999529380958,  0.99999882345170,
 0.99999970586288,  0.99999992646572,
 /* cos (2pi/N) for N=2,4,8...16384 */
-1.00000000000000,  0.00000000000000,  0.70710678118655,
 0.92387953251129,  0.98078528040323,  0.99518472667220,
 0.99879545620517,  0.99969881869620,  0.99992470183914,
 0.99998117528260,  0.99999529380958,  0.99999882345170,
 0.99999970586288,  0.99999992646572
};

static double SinArray[28]=
{ /* sin (-2pi/N) for N=2,4,8,...,16384 */
 0.00000000000000, -1.00000000000000, -0.70710678118655,
-0.38268343236509, -0.19509032201613, -0.09801714032956,
-0.04906767432742, -0.02454122852291, -0.01227153828572,
-0.00613588464915, -0.00306795676297, -0.00153398018628,
-0.00076699031874, -0.00038349518757,
 /* sin (2pi/N) for N=2,4,8,...,16384 */
 0.00000000000000,  1.00000000000000,  0.70710678118655,
 0.38268343236509,  0.19509032201613,  0.09801714032956,
 0.04906767432742,  0.02454122852291,  0.01227153828572,
 0.00613588464915,  0.00306795676297,  0.00153398018628,
 0.00076699031874,  0.00038349518757
};

int is2pow(int i)
{
int i_bits=(int)ceil(log(i-1)/log(2)); // 2005-12-10 - subtract 1 -> 512 must not give 1024, but 512!
int j=(int)pow(2,i_bits);
return j;
}

u_int ShuffleIndex(u_int i, int WordLength)
{
u_int NewIndex;
u_char BitNr;
NewIndex=0;
for (BitNr=0;BitNr<=WordLength-1;BitNr++)
  {
  NewIndex=NewIndex << 1;
  if ((i&1)!=0) 
  	NewIndex = NewIndex+1;
  i = i >> 1;
  };
  return NewIndex;
}

void Shuffle2Arr(double complex *a, int length, int bits)
{
u_int indexOld,indexNew;
double complex temp;

for (indexOld=0; indexOld <= length-1 ;indexOld++)
  {
  indexNew = ShuffleIndex(indexOld,bits);
  if (indexNew > indexOld)
     {
     temp=a[indexOld];
     a[indexOld]=a[indexNew];
     a[indexNew]=temp;
     };
  };

} 

void FFT(double complex *data, int Pwr, int Dir)
{
int N, Section, AngleCounter, FlyDistance, FlyCount;
int index1,index2;
double complex temp, giro;
double scale;
double complex Q;

N = (int) pow(2.0,(double) Pwr);

Shuffle2Arr(data,N,Pwr);

if (Dir==DIRECT) AngleCounter=0;
	else AngleCounter=14;
Section=1;
while (Section<N)
  {
  FlyDistance = 2*Section;
  giro = CosArray[AngleCounter] + I*SinArray[AngleCounter];
  Q=1.0+I*0.0;
  for (FlyCount=0;FlyCount<=Section-1;FlyCount++)
     {
     index1=FlyCount;
     do
       {
       index2 = index1 + Section;
       temp= Q*data[index2];
       data[index2]=data[index1]-temp;
       data[index1]=data[index1]+temp;
       index1=index1+FlyDistance;
       } while (index1 <= (N-1));
     Q=Q*giro;

     };
  Section*=2;
  AngleCounter++;
  };
if (Dir==INVERSE)
  {
  scale = 1.0/N;
  for (index1=0;index1<=N-1;index1++)
     {
     data[index1]*=scale;
     };
  };

}

double complex *Allocate_complex_array(int l)
{
double complex *data;
// printf("FFT - Allocate %d values\n",l);
data=(double complex *) x_calloc(l,sizeof(double complex), "double complex (data in allocate_complex_array in fft.c)");
if (data==NULL)
   {
   printf("FFT - No memory\n");
   exit(1);
   };
return data;
}

double complex * FFT2D(double complex *in, double complex *out, int w,int h, int Dir)
{
return FFT2D_with_minmax(in, out, w, h, Dir, NULL, NULL, NULL, NULL, NULL, NULL);
}

double complex * FFT2D_with_minmax(double complex *in, double complex *out, int w,int h, int Dir, double *rmin, double *rmax, double *imin, double *imax, double *nmin, double *nmax)
{
// Computing a Fast Fourier Transform, with min / max values for future scaling (display)
// printf("FFT - Preparing data ...\n");
double complex *temp, *temp2;
double value;
int i,j;
int w_bits,h_bits,testminmax=TRUE;

if ( (!rmin) && (!rmax) && (!imin) && (!imax) && (!nmin) && (!nmax))
	testminmax=FALSE;
else {
	if (rmin)
		(*rmin) = HUGE_VAL;
	if (imin)
		(*imin) = HUGE_VAL;
	if (rmax)
		(*rmax) = -HUGE_VAL;
	if (imax)
		(*imax) = -HUGE_VAL;
	if (nmax)
		(*nmax) = 0.0;
	if (nmin)
		(*nmin) = HUGE_VAL;
}

w_bits=(int)(ceil(log(w-1)/log(2)));
h_bits=(int)(ceil(log(h-1)/log(2)));
// printf("w : %d,%d, h : %d,%d\n",w,w_bits,h,h_bits);
temp=Allocate_complex_array(w*h);

if (!out)
	out = Allocate_complex_array(w*h);

for (i=0;i<h;i++)
   {
   for (j=0;j<w;j++)
      {
      temp[j+(i*w)]=in[j+(i*w)];
      };
   };

// printf("FFT - Doing Horiz FFT ...\n");

for (i=0;i<h;i++)
   {
//   printf("Row %d\n",i);
   FFT(temp+(i*w),w_bits,Dir);
   };

// printf("FFT - Preparing 2nd data ...\n");

// Columns

temp2=Allocate_complex_array(w*h);
for (i=0;i<w;i++)
   {
   for (j=0;j<h;j++)
      {
      temp2[j+(i*h)]=temp[i+(j*w)];
//      if (i==10) printf("Ligne %d... R: %f; I: %f;\n",creal(temp[i+(j*w)]),cimag(temp[i+(j*w)]));
      };
   };

// printf("FFT - Doing Vert FFT ...\n");

for (i=0;i<w;i++)
   {
//   printf("Column %d\n",i);
   FFT(temp2+(i*h),h_bits,Dir);
   };

// printf("FFT - Moving to OUT ...\n");

for (i=0;i<h;i++)
   {
   for (j=0;j<w;j++) {
	  out[j+(i*w)]=temp2[i+(j*h)];
	  if (testminmax) {
	  	if (rmin || rmax) {
			value = creal(temp2[i+(j*h)]);
			if (rmin && ((*rmin)>value))
				(*rmin) = value;
			if (rmax && ((*rmax)<value))
				(*rmax) = value;
		}
		if (imin || imax) {
	    		value = cimag(temp2[i+(j*h)]);
			if (imin && ((*imin)>value))
				(*imin) = value;
			if (imax && ((*imax)<value))
				(*imax) = value;
		}
	    	if (nmin || nmax) {
			value = creal(temp2[i+(j*h)])*creal(temp2[i+(j*h)]) + cimag(temp2[i+(j*h)])*cimag(temp2[i+(j*h)]);
			if (nmin && ((*nmin)>value))
				(*nmin) = value;
			if (nmax && ((*nmax)<value))
				(*nmax) = value;
		}
	  }
	}
   }

if (nmin)
	(*nmin) = pow(*nmin,0.5);
if (nmax)
	(*nmax) = pow(*nmax,0.5);

// printf("FFT - Deallocating memory ...\n");

x_free(temp);
x_free(temp2);

return out;
}


double complex * FFT2D_real_i2c(u_int *in, double complex *out, int w,int h, int Dir)
{
// Input image in integers, output FFT in complex (padded)
double complex *input;
int ww,hh,i,j;
ww=is2pow(w); // Padding
hh=is2pow(h);
// printf("FFT - Converting real to complex array using ww: %d X hh: %d from w: %d X h: %d... \n", ww, hh, w, h);
input=Allocate_complex_array(ww*hh);
for (i=0;i<h;i++)
   {
   for (j=0;j<w;j++) {
 //     printf("i: %d; j: %d; input_idx: %d; in_idx: %d;\n",i,j,(i*ww)+j,(i*w)+j);
      *(input+(i*ww)+j)=(double) *(in+(i*w)+j);
      }
   };

// printf("FFT2D will start...\n");
return FFT2D(input,out,ww,hh,Dir);

x_free(input);
}


u_int* complex_to_u_int (double complex *in, u_int *out, int w_in, int h_in, int w_out, int h_out) {
// Convert from a complex array to an integer array
// We suppose that the complex array has power of 2 sizes
// and that they can have been padded
// (w_out, h_out) gives the original size
// Allocates the OUT pointer if NULL, always returns it
int i,j;
// printf("FFT - Converting complex to unsigned integer array... \n");
if (!out)
	out = (u_int *) x_calloc(w_out*h_out, sizeof(u_int), "u_int (out in complex_to_u_int in fft.c)");
for (i=0;i<h_out;i++)
   for (j=0;j<w_out;j++)
   	out[w_out*i+j] = (u_int) MIN((double) MAX_HF_VALUE,MAX(0.0,creal(in[i*w_in+j])));
//   	out[w_out*i+j] = (u_int) cimag(in[i*w_in+j]);
return out;
}


u_int* complex_to_u_int_clamp (double complex *in, u_int *out, int w_in, int h_in, int w_out, int h_out) {
return complex_to_u_int_clamp_translate (in, out, w_in, h_in, w_out, h_out,0,0);
}

u_int* complex_to_u_int_clamp_translate (double complex *in, u_int *out, int w_in, int h_in, int w_out, int h_out, int tr_x, int tr_y) {
// Convert from a complex array to an integer array
// We suppose that the complex array has power of 2 sizes
// and that they can have been padded
// (w_out, h_out) gives the original size
// Allocates the OUT pointer if NULL, always returns it
// This version adjusts the output to the unsigned int range (2 bytes)
// It also translates the output - typically: use x == w_out>>1 and y == h_out>>1 to center
int i,j;
static double maxd = (double) 0xFFFF;
double rmin, rmax, rratio=1.0; // iratio=1.0, imax, imin;
// printf("FFT - Converting complex to unsigned integer array... \n");
if (!out)
	out = (u_int *) x_calloc(w_out*h_out, sizeof(u_int), "u_int (out in complex_to_u_int_clamp_translate in fft.c)");
// Calculate max and min, for normalizing the data
rmin=cnorm(*in);
rmax=rmin;
//imin=cimag(*in);
//imax=imin;
for (i=1;i<(w_in*h_in);i++) {
	if (cnorm(*(in+i))>rmax)
		rmax = cnorm(*(in+i));
	if (cnorm(*(in+i))<rmin)
		rmin = cnorm(*(in+i));
/*
	if (cimag(*(in+i))>imax)
		imax = cimag(*(in+i));
	if (cimag(*(in+i))<imin)
		imin = cimag(*(in+i));
*/
}
rratio = maxd / pow(rmax-rmin,0.25);
// iratio = maxd / log(imax-imin);
// printf("Rmin: %f; Rmax: %f; rratio: %f; iratio: %f\n",rmin,rmax,rratio,iratio);
for (i=0;i<h_out;i++)
   for (j=0;j<w_out;j++)
   	out[w_out*WRAP(i+tr_y,h_out)+WRAP(j+tr_x,w_out)] = (u_int) MIN((double) MAX_HF_VALUE,MAX(0.0,(pow(cnorm(in[i*w_in+j]) - rmin,0.25)*rratio)));
//   	out[w_out*i+j] = (u_int) (log(cimag(in[i*w_in+j]) - imin)*iratio);
return out;
}
