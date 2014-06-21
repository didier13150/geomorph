/* Writes a PNG file, with default options
 *
 * Copyright (C) 2000 Patrice St-Gelais
 * patrstg@users.sourceforge.net
 * www.oricom.ca/patrice.st-gelais
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

#include <png.h>
#include <stdlib.h>
#include "zlib.h" 
#include "./utils.h"

/* write a png file */
// Accepts RGB, 8 bits greyscale or 16 bits greyscale

void write_png_with_text (char *file_name, 
	int depth, 
	unsigned char *image_buf, 
	int width, 
	int height,
	png_text *text,
	int nbkeys)
{
   FILE *fp;
   png_uint_32 k;
   png_structp png_ptr;
   png_infop info_ptr;
   png_bytep *row_pointers;

   /* open the file */
   fp = fopen(file_name, "wb");
   if (fp == NULL) {
      my_msg(_("Not able to create the PNG file"),FAIL);
      return;
   }

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
	my_msg(_("Not able to create the PNG data structure"),FAIL);
      return;
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
	my_msg(_("Not able to create the PNG data structure"),FAIL);
      return;
   }

   /* Set error handling.    */

   if (setjmp (png_jmpbuf (png_ptr)))
   {
      /* If we get here, we had a problem writing the file */
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
	my_msg(_("Not able to write the PNG file"),FAIL);
      return;
   }

/* set up the output control - using standard C streams */
   png_init_io(png_ptr, fp);

   /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */
	if (depth == 24)

	   png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
      		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	else 
		if (depth == 16) {
	   		png_set_IHDR(png_ptr, info_ptr, width, height, 16, PNG_COLOR_TYPE_GRAY,
      				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		}
		else
	   		png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_GRAY,
      				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   /* Optional gamma chunk is strongly suggested if you have any guess
    * as to the correct gamma of the image.
    */
   png_set_gAMA(png_ptr, info_ptr, 2.2);

    /* set the zlib compression level */
    png_set_compression_level(png_ptr,
        Z_DEFAULT_COMPRESSION);

//	Write comments from the text array given in the function header

  if (nbkeys && text) { 
	png_set_text (png_ptr, info_ptr, text, nbkeys);
   }

   /* Write the file header information.  REQUIRED */
   png_write_info(png_ptr, info_ptr);

   /* For 16 bits gray images:  transforms little endian to big endian... */
   if (depth == 16)
   	png_set_swap(png_ptr);

   /* The easiest way to write the image (you may have a different memory
    * layout, however, so choose what fits your needs best).  You need to
    * use the first method if you aren't handling interlacing yourself.
    */

   row_pointers = (png_bytep *) malloc(sizeof(png_bytep *)*height);
   for (k = 0; k <(png_uint_32) height; k++) 
     row_pointers[k] = ((png_byte*) image_buf) + k*width*(depth >> 3);

   /* write out the entire image data in one call */
   png_write_image(png_ptr, row_pointers);

   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

   /* close the file */
   fclose(fp);

   /* that's it */
   return;
}


void write_png(char *file_name, int depth, unsigned char *image_buf, int width, int height)
{
	write_png_with_text(file_name, depth, image_buf, width, height, NULL, 0);

}






