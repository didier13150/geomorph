/* hf.c - 	height field creator/destructor, input / output
 *			 	and memory management
 *
 * Copyright (C) 2001, 2003, 2010 Patrice St-Gelais
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
#include "hf.h"
#include "hf_calc.h"
#include "camera.h"
#include "merge.h"

#include <png.h>
#include <locale.h>

#define CAM_RECORD_LENGTH 41
#define CAM_RECORD_FORMAT "%3d %3d %6.3f %6.3f %6.3f %3d %7.5f"
#define CAM_SCAN_FORMAT "%d %d %f %f %f %d %f"
#define WATER_LEVEL_LENGTH 10
#define WATER_LEVEL_FORMAT "%6.3f"

#ifndef DEFAULT_WATER_LEVEL
#define DEFAULT_WATER_LEVEL 0.2
#endif

static merge_struct mrg = {ADD, NULL, NULL, 0, 0, 0, 0, FALSE, 0.0};

static gchar cam1[CAM_RECORD_LENGTH],
		cam2[CAM_RECORD_LENGTH],
		cam3[CAM_RECORD_LENGTH],
		cam4[CAM_RECORD_LENGTH],
		cam5[CAM_RECORD_LENGTH]; 
		//global_water_level[WATER_LEVEL_LENGTH];

//	Ad hoc global structure for holding comments

#define NBPNGCOMMENTS 9 // Software ID, Tiling, use water, water level + 5 cameras
#define NBKEYS_BEFORE_CAMERAS 4
static png_text *pngcomments= NULL;

extern camera_struct camera_def;
extern camera_struct cameras[NBCAMERAS];

// This struct is required to write / read comments in the png file
// Here iTXt is not supported (size of the struct is 16 bytes)
// (important for initialization...)

// typedef struct png_text_struct
// {
//   int  compression;       /* compression value:
//                             -1: tEXt, none
//                              0: zTXt, deflate
//                              1: iTXt, none
//                              2: iTXt, deflate  */
//   png_charp key;          /* keyword, 1-79 character description of "text" */
//   png_charp text;         /* comment, may be an empty string (ie "")
//                              or a NULL pointer */
//   png_size_t text_length; /* length of the text string */
// #ifdef PNG_iTXt_SUPPORTED
//   png_size_t itxt_length; /* length of the itxt string */
//   png_charp lang;         /* language code, 0-79 characters
//                              or a NULL pointer */
//   png_charp lang_key;     /* keyword translated UTF-8 string, 0 or more
//                              chars or a NULL pointer */
// #endif
// } png_text;


gint sscanf_camera (gchar *text, gint index) {
	gint nbargs;
	nbargs = sscanf (text,CAM_SCAN_FORMAT,
		&cameras[index].rot_x, &cameras[index].rot_y, &cameras[index].distance,
		&cameras[index].translate_x, &cameras[index].translate_y,
		&cameras[index].angle_w, &cameras[index].aspect_ratio);

/* printf("Arguments lus dans <<%s>>: %d / 7\n",text, nbargs);
printf("INDEX: %2d; ROT_X: %3d; ROT_Y: %3d; DIST: %6.3f; TR_X: %6.3f; TR_Y: %6.3f; ANGLE: %3d; RATIO: %5.3f\n",
		index, cameras[index].rot_x, cameras[index].rot_y, cameras[index].distance,
		cameras[index].translate_x, cameras[index].translate_y,
		cameras[index].angle_w, cameras[index].aspect_ratio);
*/
	return nbargs;
}

gint snprintf_camera (gchar *buf, gint index, camera_struct *cam) {
	gint nbchars;
	nbchars = snprintf(buf,CAM_RECORD_LENGTH,CAM_RECORD_FORMAT,
		cam->rot_x, cam->rot_y, cam->distance,
		cam->translate_x, cam->translate_y, cam->angle_w, cam->aspect_ratio);
	pngcomments[index+NBKEYS_BEFORE_CAMERAS].text = buf;
// printf("CAM%1d: %s; LNG: %d\n",index, buf, strlen(buf));
	return nbchars;
}

void write_cameras_in_png (camera_struct *cams[]) {
	// We store the cameras as fixed length strings
	// At this point, keys in "pngcomments" are supposed to be initialized

	snprintf_camera (cam1, 0, cams[0]);
	snprintf_camera (cam2, 1, cams[1]);
	snprintf_camera (cam3, 2, cams[2]);
	snprintf_camera (cam4, 3, cams[3]);
	snprintf_camera (cam5, 4, cams[4]);
}

void init_png_comments () {
	gint i;
	static gchar *keys[] = {"CAMERA1","CAMERA2",
		"CAMERA3","CAMERA4","CAMERA5" };

	static gchar *tmp,*loc;
	tmp = setlocale(LC_NUMERIC,NULL); // Standardize on "." as a decimal separator
	loc = (gchar *) malloc(strlen(tmp)+1);
	strcpy(loc,tmp);
//	printf("LOCALE: %s\n",loc);
	setlocale(LC_NUMERIC,"C");
	
	pngcomments = (png_text *) x_calloc(sizeof(png_text),NBPNGCOMMENTS, "png_text");
	pngcomments[0].compression =  PNG_TEXT_COMPRESSION_NONE;
	pngcomments[0].key =  "Software";
	pngcomments[0].text =  VERSION_NAME;
	pngcomments[1].compression =  PNG_TEXT_COMPRESSION_NONE;
	pngcomments[1].key = "Tiling";
	pngcomments[1].text = "Y";
	pngcomments[2].compression =  PNG_TEXT_COMPRESSION_NONE;
	pngcomments[2].key = "Use water";
	pngcomments[2].text = "N";
	pngcomments[3].compression =  PNG_TEXT_COMPRESSION_NONE;
	pngcomments[3].key = "Water level";
	pngcomments[3].text = (gchar *) malloc(WATER_LEVEL_LENGTH);
	snprintf(pngcomments[3].text, WATER_LEVEL_LENGTH, WATER_LEVEL_FORMAT, DEFAULT_WATER_LEVEL);

	for (i=NBKEYS_BEFORE_CAMERAS; i<NBPNGCOMMENTS; i++) {
		pngcomments[i].key = keys[i-NBKEYS_BEFORE_CAMERAS];
		pngcomments[i].compression = PNG_TEXT_COMPRESSION_NONE;
	}

	snprintf_camera (cam1, 0, &cameras[0]);
	snprintf_camera (cam2, 1, &cameras[1]);
	snprintf_camera (cam3, 2, &cameras[2]);
	snprintf_camera (cam4, 3, &cameras[3]);
	snprintf_camera (cam5, 4, &cameras[4]);

//	printf("LOCALE: %s\n",loc);
	setlocale(LC_NUMERIC,loc);
	free(loc);
#ifdef PNG_iTXt_SUPPORTED
	for (i=0; i<NBPNGCOMMENTS; i++)
		pngcomments[i].lang = NULL;
#endif

}

void hf_write( hf_struct_type *hf, gchar *path_n_file, camera_struct *cameras[]) {
	gboolean tiling;	
	static gchar *tmp,*loc;
	tmp = setlocale(LC_NUMERIC,NULL); // Standardize on "." as a decimal separator
//	printf("LOCALE: %s\n",loc);
	loc = (gchar *) malloc(strlen(tmp)+1);
	strcpy(loc,tmp);
	setlocale(LC_NUMERIC,"C");
	if (hf->if_tiles)
		tiling = *hf->if_tiles;
	else
		tiling = TRUE;
	if (!pngcomments)
		init_png_comments();
	write_cameras_in_png (cameras);
	if (tiling)
		pngcomments[1].text = "Y";
	else
		pngcomments[1].text = "N";
	if (hf->use_water)
		pngcomments[2].text = "Y";
	else
		pngcomments[2].text = "N";
//	Added 2010-10-08
	snprintf (pngcomments[3].text, WATER_LEVEL_LENGTH, WATER_LEVEL_FORMAT, hf->water_level);
	setlocale(LC_NUMERIC,loc);
	free(loc);
	write_png_with_text(path_n_file, 16, (gpointer) hf->hf_buf, hf->max_x, hf->max_y,
		pngcomments, NBPNGCOMMENTS);
}

gboolean test_power_2(gint hf_size) {
//	Returns 1 if hf_size is a power of 2 between 128 and 4096 included
	switch (hf_size) {
		case 128:	break;
		case 256:	break;
		case 512:	break;
		case 1024:	break;
		case 2048:	break;
		case 4096:	break;
		default:
			return FALSE;
	}
	return TRUE;
}

static unsigned char *expand_8_to_16(unsigned char *img_buf, gint width, gint height) {
//	Simple expansion of a greyscale 8 bits image to 16 bits image
//	Future improvement:  maybe add noise in the lower weight byte??
	unsigned char *new_img;
	gint i;
	new_img = (unsigned char *) x_calloc(width*height,2, "unsigned char (new_img)");
	for (i=0; i<width*height; i++) {
	//	Lower byte must appear 1st ... (I'm missing something here...)
		*(new_img+i*2+1) = *(img_buf+i);
	}
	x_free(img_buf);
	return new_img;
}

static unsigned char *rgb_to_grey(unsigned char *img_buf, gint width, gint height, gint depth, gboolean alpha) {
/*	Extract from libpng, explaining the method
The default values approximate those recommended in the Charles
Poynton's Color FAQ, <http://www.inforamp.net/~poynton/>
Copyright (c) 1998-01-04 Charles Poynton poynton@inforamp.net

    Y = 0.212671 * R + 0.715160 * G + 0.072169 * B

Libpng approximates this with

    Y = 0.21268 * R    + 0.7151 * G    + 0.07217 * B

which can be expressed with integers as

    Y = (6969 * R + 23434 * G + 2365 * B)/32768
*/
	unsigned long int r,g,b,y;
	pix8_rgb *pix8 = 0;
	pix16_rgb *pix16 = 0;
	pix8_rgba *pix8a = 0;
	pix16_rgba *pix16a = 0;
	gint i;
	unsigned char *new_img, *new_img8;
	unsigned short int *new_img16;
	if (depth==8) {	
		if (alpha)
			pix8a = (pix8_rgba *) img_buf;
		else
			pix8 = (pix8_rgb *) img_buf;
		new_img8 = (unsigned char *) x_calloc(width*height,1, "unsigned char (new_img8)");
		for (i=0; i<width*height; i++) {
			if (alpha) {
				r = (unsigned long int) pix8a->r;
				g = (unsigned long int) pix8a->g;
				b = (unsigned long int) pix8a->b;
				pix8a++;
			}
			else {
				r = (unsigned long int) pix8->r;
				g = (unsigned long int) pix8->g;
				b = (unsigned long int) pix8->b;
				pix8++;
			}
			y = (6969 * r + 23434 * g + 2365 * b)>>15;
			*(new_img8+i) = y;
		}
		new_img = (unsigned char *) new_img8;
	}
	else
		if (depth==16) {
			if (alpha)
				pix16a = (pix16_rgba *) img_buf;
			else
				pix16 = (pix16_rgb *) img_buf;
			new_img16 = (unsigned short int *) x_calloc(width*height, sizeof(unsigned short int), "unsigned short int (new_img16)");
			for (i=0; i<width*height; i++) {
				if (alpha) {
					r = (unsigned long int) pix16a->r;
					g = (unsigned long int) pix16a->g;
					b = (unsigned long int) pix16a->b;
					pix16a++;
				}
				else {
					r = (unsigned long int) pix16->r;
					g = (unsigned long int) pix16->g;
					b = (unsigned long int) pix16->b;
					pix16++;
				}
				y = (6969 * r + 23434 * g + 2365 * b)>>15;
				*(new_img16+i) = y;
			}
			new_img = (unsigned char *) new_img16;
		}
		else {
			x_free(img_buf);
			return NULL;
		}
	x_free(img_buf);
	return new_img;
}

static unsigned char *remove_alpha_from_grey (unsigned char *img_buf, gint width, gint height, gint depth) {

	gint i;
	unsigned char *new_img, *new_img8;
	unsigned short int *new_img16, *img_buf16;

	if (depth==8) {	
		new_img8 = (unsigned char *) x_calloc(width*height,1, "unsigned char (new_img8)");
		for (i=0; i<width*height; i++) {
			*(new_img8+i) = *(img_buf+2*i);
		}
		new_img = (unsigned char *) new_img8;
	}
	else 	if (depth==16) {
			new_img16 = (unsigned short int *) x_calloc(width*height, sizeof(unsigned short int), "unsigned short int (new_img16)");
			img_buf16 = (unsigned short int *) img_buf;
			for (i=0; i<width*height; i++) {
				*(new_img16+i) = *(img_buf16+2*i);
			}
			new_img = (unsigned char *) new_img16;
		}
		else {
			x_free(img_buf);
			return NULL;
		}
	x_free(img_buf);
	return new_img;
}

static gpointer hf_create(hf_type *image_buf, gint hfsize, gboolean tiling, gboolean *tiling_ptr, gboolean use_water, gfloat water_level ) {
//	Create a new HF with the image data given,
//	typically after a file read

	hf_struct_type *hf;
	hf = (hf_struct_type *) x_malloc(sizeof(hf_struct_type), "hf_struct_type");
	hf->max_x = hfsize;
	hf->max_y = hfsize;
	if (image_buf)
		hf->hf_buf = image_buf; 
	else
		hf->hf_buf = (hf_type *) x_malloc(hfsize * hfsize *sizeof(hf_type), "hf_type (hf->hf_buf)");
	hf->tmp_buf = NULL;
	hf->tmp2_buf = NULL;
	hf->result_buf = NULL;
	hf->select_buf = NULL;
	hf->layer_buf = NULL;
	hf->if_layer = FALSE;
	hf->min = 0;
	hf->max = MAX_HF_VALUE;
	hf->avrg = MAX_HF_VALUE>>1;

	hf->if_tiles = tiling_ptr;
	// If no tiling_ptr, tiling is assumed to be TRUE
	if (tiling_ptr)
		*hf->if_tiles = tiling;
	hf->use_water = use_water;
	hf->water_level = water_level;
	return (gpointer) hf;
}

hf_struct_type * hf_read(gchar *path_n_file, gboolean *fname_tochoose, gchar **msg, gboolean *tiling_ptr) {

//	Reads a PNG HF file and transforms it so it becomes a 16 bits power2 x power2 image
//	path_n_file:  the file to read
//	fname_tochoose:  we set this variable to TRUE if the image is transformed and we
//			want to keep the original file intact - the displayed file
//			would be called "documentN"
//	msg:  	message to return, if something goes wrong - the "methods" of the
//		current .c file are not supposed to deal with the human interface!
	gint depth, width, height, color_type, interlace_type, i;
	unsigned char *img_buf=NULL;
	png_text *comments=NULL;
	gint nbkeys;
	gchar *tmp,*loc;
	gboolean tiling = FALSE, use_water = FALSE, alpha = FALSE;
	gfloat water_level;

	if (!read_png_header(path_n_file,  &depth, &width, &height,
			&color_type, &interlace_type) )  {
		(*msg) = _("PNG file unreadable");
		return NULL;
	}

	if (!read_png_with_text(path_n_file, &depth, &img_buf, &width, &height,
			&color_type, &interlace_type, &comments, &nbkeys)) {
		(*msg) = _("PNG file unreadable");
		return NULL;
	}
	if (width!=height)  {
		(*msg) = _("Geomorph requires a square image");
		x_free(img_buf);
		return NULL;
	}
	if (!test_power_2(width)) {
		(*msg) = _("Geomorph requires a square image nwith edges of 128, 256, 512, 1024, 2048 or 4096 pixels");
		x_free(img_buf);
		return NULL;
	}

	if ((color_type!=PNG_COLOR_TYPE_GRAY) &&
		(color_type!=PNG_COLOR_TYPE_GRAY_ALPHA) &&
		(color_type!=PNG_COLOR_TYPE_RGB_ALPHA) &&
		(color_type!=PNG_COLOR_TYPE_RGB) ) {
			(*msg) = _("PNG file unreadable");
			return NULL;
	}

	if ((color_type==PNG_COLOR_TYPE_GRAY_ALPHA) || (color_type==PNG_COLOR_TYPE_RGB_ALPHA))
		alpha = TRUE;

	if ( (color_type == PNG_COLOR_TYPE_RGB) || (color_type == PNG_COLOR_TYPE_RGB_ALPHA)) {
		(*msg) = _("Color image converted to B&W under a new name");
		(*fname_tochoose) = TRUE;
		img_buf = rgb_to_grey(img_buf, width, height, depth, alpha);
		if (!img_buf) {
			(*msg) = _("Not able to convert this image to B&W");
			return NULL;
		}
	}
	else if (color_type==PNG_COLOR_TYPE_GRAY_ALPHA) {
	// Remove alpha channel if 8 bits or 16 bits grayscale
		(*msg) = _("Image converted to 16 bits B&W without alpha under a new name");
		(*fname_tochoose) = TRUE;
		img_buf = remove_alpha_from_grey (img_buf, width, height, depth);
		if (!img_buf) {
			(*msg) = _("Not able to convert this image to B&W");
			return NULL;
		}
	}
	if (depth!=16) {
		(*msg) = _("8 bits image converted to 16 bits under a new name");
		(*fname_tochoose) = TRUE;
//		We suppose weird depths (1, 2, 4...) are already converted in read_png
		img_buf = expand_8_to_16(img_buf, width, height);
	}
	if (nbkeys && comments) {
		tmp = setlocale(LC_NUMERIC,NULL); // Standardize on "." as a decimal separator
		loc = (gchar *) malloc(strlen(tmp)+1);
		strcpy(loc,tmp);
		setlocale(LC_NUMERIC,"C");
		for (i=0; i<nbkeys; i++) {
 // printf("KEY: %s; TEXT: %s\n",comments[i].key, comments[i].text);
			if (!strcmp(comments[i].key,"Tiling")) {
				if (!strcmp(comments[i].text,"Y"))
					tiling = TRUE;
			}
			if (!strcmp(comments[i].key,"Use water")) {
				if (!strcmp(comments[i].text,"Y"))
					use_water = TRUE;
			}
			if (!strcmp(comments[i].key,"Water level")) {
//				printf("sscanf (comments[i].text, , water_level)");
				sscanf (comments[i].text, "%f", &water_level);
// 				printf("Water level in HF_READ: %6.3f\n",water_level);
			}
			else if (!strcmp(comments[i].key,"CAMERA1"))
				sscanf_camera (comments[i].text,0);
			else if (!strcmp(comments[i].key,"CAMERA2"))
				sscanf_camera (comments[i].text,1);
			else if (!strcmp(comments[i].key,"CAMERA3"))
				sscanf_camera (comments[i].text,2);
			else if (!strcmp(comments[i].key,"CAMERA4"))
				sscanf_camera (comments[i].text,3);
			else if (!strcmp(comments[i].key,"CAMERA5"))
				sscanf_camera (comments[i].text,4);
		} // enf for
		setlocale(LC_NUMERIC,loc);
		free(loc);
		free_png_text(&comments,nbkeys);
	} // end if (nbkeys...)
	return hf_create((hf_type *) img_buf, width, tiling, tiling_ptr, use_water, water_level);
}

gpointer hf_new(gint hfsize) {
//	Creates and initializes an HF with default values
//	The image data is not known
// 	We presume we are generating a new (tiling) HF
	hf_struct_type *hf;
	hf = (hf_struct_type *) hf_create(NULL, hfsize, TRUE, NULL, TRUE, DEFAULT_WATER_LEVEL);
	return hf;
}

void hf_set_tiling_ptr (hf_struct_type *hf, gboolean *tiling_ptr) {
	hf->if_tiles = tiling_ptr;
}

void hf_init_layer (hf_struct_type *hf) {
	if (hf->layer_buf)
		x_free (hf->layer_buf);
	hf->layer_buf = (hf_type *) x_calloc(hf->max_x*hf->max_y, sizeof(hf_type), "hf_type (hf->layer_buf)");
	hf->if_layer = TRUE;
}

void hf_cancel_layer (hf_struct_type *hf) {
	hf->if_layer = FALSE;
	if (hf->layer_buf)
		x_free (hf->layer_buf);
	hf->layer_buf = NULL;
}

void hf_init_result_buf_from_hf (hf_struct_type *hf) {
	if (!hf->result_buf)
		hf->result_buf = 
			(hf_type *) x_malloc(sizeof(hf_type) * hf->max_x  * hf->max_y, "hf_type (hf->result_buf)");
	memcpy(hf->result_buf, hf->hf_buf, sizeof(hf_type) * hf->max_x  * hf->max_y);
}

void hf_init_tmp2_buf_from_hf (hf_struct_type *hf) {
	if (!hf->tmp2_buf)
		hf->tmp2_buf = 
			(hf_type *) x_malloc(sizeof(hf_type) * hf->max_x  * hf->max_y, "hf_type (hf->tmp2_buf)");
	memcpy(hf->tmp2_buf, hf->hf_buf, sizeof(hf_type) * hf->max_x  * hf->max_y);
}

void hf_update_hf_with_layer (hf_struct_type *hf, gint merge_op) {
	//gint i;
	if (!hf->layer_buf)
		return;
	if (!hf->tmp2_buf)
		return;
	if (!hf->hf_buf)
		return;
	mrg.merge_op = merge_op;
	mrg.mix = 0;
	mrg.source_offset = 100;
	mrg.result_offset = 100;
	set_merge_buffers (&mrg, hf->tmp2_buf, hf->layer_buf, hf->hf_buf, hf->max_x, hf->max_y);
	simple_merge (&mrg);
}

hf_struct_type *hf_copy_new(hf_struct_type *hf, gboolean flag_swap) {
//	Copy a hf_struct onto a newly allocated hf_struct,
//	for instance for undo / redo purpose
	hf_struct_type *hfcopy;
	hfcopy = (hf_struct_type *) x_malloc(sizeof(hf_struct_type), "hf_struct_type (hfcopy)");
//	printf("HF_COPY, de %p sur MALLOC: %p; flag_swap: %d\n",hf, hfcopy, flag_swap);
	memcpy(hfcopy, hf, sizeof(hf_struct_type));
	hfcopy->hf_buf = (hf_type *) x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (hfcopy->hf_buf)");
	memcpy(hfcopy->hf_buf, hf->hf_buf, hf->max_x*hf->max_y*sizeof(hf_type));
	//	We swap buffers when the copy is intended for use for UNDO
	// 	because the data pointers are usually in use in some dialog - they should not vary
	//	When the copy does not replace an existing HF, usually flag_swap
	//	should be FALSE, or for the REDO operation
	//	See hf_wrapper_copy in hf_wrapper.c
//	printf("De hf->hf_buf: %p sur hfcopy->hf_buf: %p; flag_swap: %d\n",hf->hf_buf, hfcopy->hf_buf, flag_swap ); 
	if (flag_swap)
		swap_buffers((gpointer *)&hfcopy->hf_buf, (gpointer *)&hf->hf_buf);
//	printf("De hf->hf_buf: %p sur hfcopy->hf_buf: %p; flag_swap: %d\n",hf->hf_buf, hfcopy->hf_buf, flag_swap ); 
	if (hf->tmp_buf) {
		hfcopy->tmp_buf = (hf_type *)x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (hfcopy->tmp_buf)");
		memcpy(hfcopy->tmp_buf, hf->tmp_buf, hf->max_x*hf->max_y*sizeof(hf_type));
		if (flag_swap)
			swap_buffers((gpointer *)&hfcopy->tmp_buf, (gpointer *)&hf->tmp_buf);
	}
	if (hf->result_buf) {
		hfcopy->result_buf = (hf_type *)x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (hfcopy->result_buf)");
		memcpy(hfcopy->result_buf, hf->result_buf, hf->max_x*hf->max_y*sizeof(hf_type));
		if (flag_swap)
			swap_buffers((gpointer *)&hfcopy->result_buf, (gpointer *)&hf->result_buf);
	}
	if (hf->tmp2_buf) {
		hfcopy->tmp2_buf = (hf_type *)x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (hfcopy->tmp2_buf)");
		memcpy(hfcopy->tmp2_buf, hf->tmp2_buf, hf->max_x*hf->max_y*sizeof(hf_type));
		if (flag_swap)
			swap_buffers((gpointer *)&hfcopy->tmp2_buf, (gpointer *)&hf->tmp2_buf);
	}
	// select_buf is not required
	hfcopy->select_buf = NULL;
	// layer_buf is not required as well
	hfcopy->layer_buf = NULL;
	hfcopy->if_layer = FALSE;
	return hfcopy;
}

void hf_backup(hf_struct_type *hf) {
//	Backup the main buffer into a temporary one
	if (!hf->tmp_buf)
		hf->tmp_buf = (hf_type *)x_malloc(hf->max_x*hf->max_y*sizeof(hf_type), "hf_type (hf->tmp_buf in hf_backup)");
// printf("HF_BACKUP de %p sur %p\n",hf->hf_buf,hf->tmp_buf);
	memcpy(hf->tmp_buf, hf->hf_buf, hf->max_x*hf->max_y*sizeof(hf_type));
	// For consistency, between computations, hf_buf should == result_buf
	if (hf->result_buf)
		memcpy(hf->result_buf, hf->hf_buf, hf->max_x*hf->max_y*sizeof(hf_type));
	hf_min_max (hf);
}

void hf_restore(hf_struct_type *hf) {
	// Opposite of hf_backup...
// printf("HF_RESTORE de %d sur %d\n", hf->tmp_buf, hf->hf_buf);
	if (hf->tmp_buf)
		memcpy(hf->hf_buf, hf->tmp_buf, hf->max_x*hf->max_y*sizeof(hf_type));
	// Here result_buf should be the same as hf_buf - restore also cancel the last computation
	if (hf->result_buf)
		memcpy(hf->result_buf, hf->hf_buf, hf->max_x*hf->max_y*sizeof(hf_type));
}

void hf_free(hf_struct_type *hf) {
//	printf("HF_FREE de %p\n",hf);
	if (!hf) return;
	if (hf->hf_buf) {
		x_free(hf->hf_buf);
	}
	if (hf->tmp_buf) {
		x_free(hf->tmp_buf);
	}
	if (hf->tmp2_buf) {
		x_free(hf->tmp2_buf);
	}
	if (hf->result_buf) {
		x_free(hf->result_buf);
	}
	if (hf->select_buf) {
		x_free(hf->select_buf);
	}
	if (hf->layer_buf) {
		x_free(hf->layer_buf);
	}
	x_free(hf);
}

void hf_reset_buffer (hf_type *hf, gint maxx, gint maxy) {
	// Resets HF to 0
	gint x,y;
	if (!hf)
		return;
	for (x=0; x<maxx; x++) {
		for (y=0; y<maxy; y++)
			*(hf+x+y*maxx) = 0;
	}
}

void hf_min_max (hf_struct_type *hf) {
//	Refreshes the MIN, MAX. AVRG values of  the 16 bit greyscale buffer
	gint i;
	gfloat counter = 0.0;
	hf->min = *hf->hf_buf;
	hf->max = hf->min;
//	printf("HF_MIN_MAX_AVRG\n");
	for (i=0; i<hf->max_x*hf->max_y; i++) {
		if (hf->min>*(hf->hf_buf+i) )
			hf->min = *(hf->hf_buf+i);
		else
			if (hf->max<*(hf->hf_buf+i))
				hf->max = *(hf->hf_buf+i);
		counter += (gfloat) *(hf->hf_buf+i);
	}
	hf->avrg = (hf_type) (counter / i);
//	printf("Min: %d; Max: %d\n",hf->min,hf->max);
}

