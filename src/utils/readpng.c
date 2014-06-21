   /*  A simple PNG file reader, with default options
 *
 * Copyright (C) 2002 Patrice St-Gelais
 * patrstg@users.sourceforge.net
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
#include <png.h>
#include "utils.h"

/* Check to see if a file is a PNG file using png_sig_cmp().  png_sig_cmp()
 * returns zero if the image is a PNG and nonzero if it isn't a PNG.
 *
 * The function check_if_png() shown here, but not used, returns nonzero (true)
 * if the file can be opened and is a PNG, 0 (false) otherwise.
 *
 * If this call is successful, and you are going to keep the file open,
 * you should call png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK); once
 * you have created the png_ptr, so that libpng knows your application
 * has read that many bytes from the start of the file.  Make sure you
 * don't call png_set_sig_bytes() with more than 8 bytes read or give it
 * an incorrect number of bytes read, or you will either have read too
 * many bytes (your fault), or you are telling libpng to read the wrong
 * number of magic bytes (also your fault).
 *
 * Many applications already read the first 2 or 4 bytes from the start
 * of the image to determine the file type, so it would be easiest just
 * to pass the bytes to png_sig_cmp() or even skip that if you know
 * you have a PNG file, and call png_set_sig_bytes().
 */

void free_png_text (png_text **txt, gint nbkeys) {
	// Freeing the strings in *txt and then free *txt
	// Required after a read_png_with_text, if nbkeys>0
	gint i;
	if (*txt) {
		for (i=0; i<nbkeys; i++) {
			if ((*txt)[i].key)
				free((*txt)[i].key);
			if ((*txt)[i].text)
				free((*txt)[i].text);
		}
		free (*txt);
	}
}

gboolean read_png_header(gchar *file_name,  gint *depth, 
		gint *width, gint *height, gint *color_type, gint *interlace_type) 
{

//	Opens the file, reads the PNG header, close the file
//      	File_name:  supplied, with path;
//      	depth:  pre-allocated pointer, for returning depth (normally 8 or 16 bits)
//      	width and height:  pre-allocated integer pointer, for returning read parameters
//	Returns FALSE if the file cannot be read
   png_structp png_ptr;
   png_infop info_ptr;
   png_uint_32 w, h;
   FILE *fp;

   if ((fp = fopen(file_name, "rb")) == NULL)
      return FALSE;
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL,NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return FALSE;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return FALSE;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return FALSE;
   }

   png_init_io(png_ptr, fp);

   /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).  REQUIRED
    */
   png_read_info(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &w, &h, depth, color_type,
       interlace_type, NULL, NULL);

	(*width) = (gint) w;
	(*height) = (gint) h;
	fclose(fp);
	return TRUE;
}

gboolean read_png_with_text (gchar *file_name, 
		gint *depth, 
		unsigned char **image_buf, 
		gint *width, 
		gint *height,
		gint *color_type, 
		gint *interlace_type,
		png_text **text,
		gint *nbkeys)
{

//      image_buf:  unallocated image_buffer;  size in bytes: depth * width * height
//	Returns FALSE if the file is not readable

   png_structp png_ptr;
   png_infop info_ptr;
   FILE *fp;
   png_uint_32 w, h;
   png_bytep row_pointer;
   png_text *comments_buf;
   gint row_length, pass, number_passes, y;

   if ((fp = fopen(file_name, "rb")) == NULL)
      return FALSE;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return FALSE;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return FALSE;
   }

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return FALSE;
   }

   png_init_io(png_ptr, fp);

   /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).  REQUIRED
    */
   png_read_info(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &w, &h, depth, color_type,
       interlace_type, NULL, NULL);

   (*width) = w;
   (*height) = h;

   /* Expand paletted colors into true RGB triplets */
   if ((*color_type) == PNG_COLOR_TYPE_PALETTE) {
      png_set_expand(png_ptr);
      (*color_type) = PNG_COLOR_TYPE_RGB;
   }

   /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
   else if ((*color_type) == PNG_COLOR_TYPE_GRAY && (*depth) < 8) {
      png_set_expand(png_ptr);
      (*depth) = 8;
   }
   
// Change byte order from big endian to little endian
   if ((*depth)==16)
	png_set_swap(png_ptr);

   /* Turn on interlace handling.  REQUIRED if you are not using
    * png_read_image().  To see how to handle interlacing passes,
    * see the png_read_row() method below:
    */
   number_passes = png_set_interlace_handling(png_ptr);
   row_length = png_get_rowbytes(png_ptr, info_ptr);
   row_pointer = (png_bytep) malloc(row_length);
   (*image_buf) = (unsigned char *) malloc((*height)*row_length);

  for (pass = 0; pass < number_passes; pass++)
   {
      for (y = 0; y < (*height); y++)
      {
         png_read_row(png_ptr, row_pointer, NULL);
         memcpy((*image_buf)+row_length*y,row_pointer,row_length);
      }
   }
   free(row_pointer);
   /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
   png_read_end(png_ptr, info_ptr);

if (text && nbkeys) {
   // Reading comments, whether they are at the beginning or the end
   png_get_text (png_ptr, info_ptr, &comments_buf, nbkeys);

   // Comments_buf is allocated by libpng and so would be freed, so we must copy it
   (*text) = (png_text *) malloc((*nbkeys)*sizeof(png_text));
   for (y=0; y<*nbkeys; y++) {
	(*text)[y].key = (char *) malloc(1+strlen(comments_buf[y].key));
	strcpy((*text)[y].key,comments_buf[y].key);
	(*text)[y].text = (char *) malloc(1+strlen(comments_buf[y].text));
	strcpy((*text)[y].text,comments_buf[y].text);
	(*text)[y].text_length = comments_buf[y].text_length;
   }
}
   /* clean up after the read, and free any memory allocated - REQUIRED */
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

   fclose(fp);
   return TRUE;
}


gboolean read_png(gchar *file_name, gint *depth, unsigned char **image_buf, gint *width, gint *height,
		gint *color_type, gint *interlace_type)
{
   return read_png_with_text (file_name, depth, image_buf, width, height,
		color_type, interlace_type, NULL, NULL);
}



