/* uniform.c - generates a white / grey / black uniform pixmap
 *
 * Copyright (C) 2001 Patrice St-Gelais
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
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "uniform.h"

void uniform (hf_struct_type *hf, gpointer data) 
{
	gint i;
	hf_type value;
	value = (hf_type) * (hf_type *) data;
	hf->hf_buf = (hf_type *) x_realloc(hf->hf_buf,hf->max_x * hf->max_y * sizeof(hf_type), "hf_type (hf_buf in uniform)");
	for (i = 0; i < hf->max_x*hf->max_y; i++)
//	D�grad� pour test
//		*(hf->hf_buf + i) = (i*hf->max_x) % (hf->max_x*hf->max_x);		
		*(hf->hf_buf + i) = value;
}















