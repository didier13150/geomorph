/* craters.c - Functions for creating and processing craters
 *
 * Copyright (C) 2003 Patrice St-Gelais
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

#include "craters.h"
#include "hf.h"
#include "hf_calc.h"

//	Load crater maps

float peak_crater[256] = 
#include "./peak.map"

float irregular_crater[256] = 
#include "./irregular.map"

float standard_crater[256] = 
#include "./standard.map" 

#define MAX_RADIAL_NOISE 180
static gdouble radial_noise[MAX_RADIAL_NOISE];

draw_crater_struct *crater_struct_new() {
	// Used for "crater pen" and also as a basis for automatic crater generation
	draw_crater_struct *d;
	d = (draw_crater_struct *) x_malloc(sizeof(draw_crater_struct), "draw_crater_struct");
	d->type = STANDARD_CRATER ;
	d->diameter = 20;	// % of HF size
	d->random_diameter = FALSE;
	d->diam_bound1 = 5;
	d->diam_bound2 = 40;
	d->default_depth = TRUE;
	d->depth = 20;		// % of diameter
	d->radial_noise = 4;	// Random variation of the diameter, from 0% to 10%
	d->wrap = TILING_AUTO;
	d->preview = NULL;
	d->if_smooth = TRUE;
	d->surface_noise = 2;
	d->distribution = 2;
	d->seed = rand();
	return d;
}

void crater_struct_free(draw_crater_struct *dcs) {
	if (!dcs)
		return;
	if (dcs->preview)
		view_struct_free(dcs->preview);
	x_free(dcs);
}

void init_radial_noise() {
	gint i;
	// Radial noise:  from 0 to 15%
	for (i=0; i<MAX_RADIAL_NOISE; i++) {
//		radial_noise[i] =  ((gdouble) (rand()%15)) / 100.0;
		radial_noise[i] =  ((gdouble) (rand()%25)) / 100.0;
// printf("RADIAL_NOISE[%d]: %6.3f\n",i,radial_noise[i]);
	}
}

gint get_depth (draw_crater_struct *dcs, gint diameter, gint hf_size) {
	// Calculate the absolute crater depth
	gint depth = 10;
	if (dcs->default_depth)
		switch (dcs->type) {
		case STANDARD_CRATER:
			depth = 25;
			break;
		case PEAK_CRATER:
			depth = 15;
			break;
		case IRREGULAR_CRATER:
			depth = 12;
		}
	else
		depth = dcs->depth;
	//	Absolute depth is measured from the ground level
	//	It is not measured from the rim top
	//	In the map, the ground level is supposed to be 0 and the bottom of the crater -1
	//	The depth is in % - so the absolute depth should be (relatively to the max. height 0xFFFF)
	depth = depth * diameter * 0xFFFF / (100 * hf_size);
	return depth;
}

gint *get_sqrmap (draw_crater_struct *dcs, gint diam, gint depth) {
//	Build a square map from a crater map, ready to be merged (+) with a hf
//	The function allocates the map
	gfloat *map, noise;
	gint *gintmap, *sqrmap;
	gint i,x,y, dist, distmax, int_noise, quadrant;
	gdouble mid, ratio, ddistmax, radial_noise_factor, ddepth, h;
	mid = -1.0 + (gdouble) (diam / 2);
	switch (dcs->type) {
	case STANDARD_CRATER:
		map = standard_crater;
		break;
	case PEAK_CRATER:
		map = peak_crater;
		break;
	case IRREGULAR_CRATER:
		map = irregular_crater;
	default:
		map = irregular_crater;
	}
	noise = ((gfloat) dcs->radial_noise) / 100.0;
	int_noise = (gint) (noise * diam);
	// The radial noise requires the angle computation
	// We use the atan() function to get the angle in radians
	// We must scale the atan() result for "spreading" it on one quadrant
	radial_noise_factor = (MAX_RADIAL_NOISE>>1) / PI;

	//	Convert the map to gint and scale it to the radius 
	//	of the biggest circle contained in width x height
	distmax = 1+(diam>>1);
	ddistmax = (gdouble) distmax;
	ddepth = (gdouble) depth;
	gintmap = (gint *) x_malloc(sizeof(gint) * distmax, "gint (gintmap in get_sqrmap in craters.c)");
	sqrmap = (gint *) x_calloc(diam * diam,sizeof(gint), "gint (sqrmap in get_sqrmap in craters.c)");
	ratio = ((gdouble) MAP_LENGTH-1 ) / ddistmax;
// printf("NOISE: %6.3f;  RADIAL_NOISE_FACTOR: %6.3f, DISTMAX: %d; RATIO: %f; MID: %f; \n",noise,radial_noise_factor,distmax,ratio,mid);
	for (i=0; i<distmax; i++) {
		gintmap[i] = depth * *(map+ MAP_LENGTH -1-(gint) ( ((gdouble) i)*ratio) );
	}
	for (y=0; y<diam; y++) {
		for (x=0; x<diam; x++) {
			if ( ((x-mid)>=0) && ((y-mid)>=0))
				quadrant = 1;
			else if ( ((x-mid)<0) && ((y-mid)>=0))
				quadrant = 2;
			else if ( ((x-mid)<0) && ((y-mid)<0))
				quadrant = 3;
			else // if ( ((x-mid)>=0) && ((y-mid)<0))
				quadrant = 4;

// printf("QUADRANT: %d;  ",quadrant);
			dist = (gint) DIST2((gdouble) x, (gdouble) y, mid, mid);

			if (dist<distmax) {
				if (dcs->type==IRREGULAR_CRATER)
					h = MAX(( (gdouble) gintmap[dist] )  / ddepth + 0.8, 0.0);
				else
					h = MAX(( (gdouble) gintmap[dist] )  / ddepth + 0.25, 0.0);
			}
				else
			h = 0.0;
// printf("(%d,%d) - gintmap[%d]: %d;  h: %6.3f; ",x,y,dist, gintmap[dist],h);
			// Random height noise
			// Depends only on distance from center and depth, not on angle
			if (int_noise) {
				dcs->seed = rand();
				dist = dist + (gint) (4.0 * (h+0.25) * (gdouble) (dcs->seed%int_noise));
			}
			if (dist>=distmax) {
				*(sqrmap + y*diam + x) = 0;
				continue;
			}

			// XY radial noise (quadratic - high deviations unfrequent)

			ratio = radial_noise[MAX(0,MIN(MAX_RADIAL_NOISE-1, quadrant *
				(gint) (radial_noise_factor*ABS(atan((y-mid)/(x-mid+1))))))];
// printf("Ratio: %6.3f; Index radial_noise: %d; ", ratio, MAX(0,MIN(MAX_RADIAL_NOISE-1, quadrant *
//				(gint) (radial_noise_factor*ABS(atan((y-mid)/(x-mid+1)))))));
			dist = (gint) (((gdouble) dist) * (1.0 + noise * 100.0 * h  * ratio * ratio ));
			if (dist>=distmax) {
				*(sqrmap + y*diam + x) = 0;
				continue;
			}

// printf("New dist: %d\n",dist);
		// With height radial noise
			*(sqrmap + y*diam + x) = (gint) ((1.0 + noise * 100.0 * h * ratio) * 
						(gdouble) gintmap[dist] ) ;

//			*(sqrmap + y*diam + x) = gintmap[dist] ;
		}
	}
	x_free(gintmap);
	return sqrmap;
}

hf_struct_type *get_smoothmap (gint diam, gint depth) {
//	Build a smoothing map with a gaussian bell, ready for convolve_normalized_vector
//	The map is a HF, which should be freed with hf_free
//	... optimisation required ... (caching maps?)
	hf_struct_type *smooth_map;
	gdouble *bell;
	hf_type *hfmap;
	gint i,x,y, dist, distmax;
	gdouble mid, ratio;
	// "diam" should be even
	diam = 2 * diam>>1;
	mid = -1.0 + ((gdouble) diam) / 2.0;
	smooth_map = (hf_struct_type *) hf_new(diam);
	//	Convert the map to gint and scale it to the radius 
	//	of the biggest circle contained in width * height
	distmax = diam>>1;
	bell = normalized_bell_new(distmax);
	hfmap = (hf_type *) x_malloc(sizeof(hf_type) * distmax, "hf_type (hf_map in get_smoothmap in craters.c)");
	ratio = ((gdouble) 0xFFFF) / bell[0]; 
// printf("DISTMAX: %d; MID: %f; RATIO: %f \n",distmax,mid, ratio);
	for (i=0; i<distmax; i++) {
		hfmap[i] = (hf_type) (ratio * *(bell + i));
// printf("HFMAP[%d]: %d\n",i, hfmap[i]);
	}
	for (y=0; y<diam; y++) {
		for (x=0; x<diam; x++) {
			dist = (gint) DIST2((gdouble) x, (gdouble) y, mid, mid);
			if (dist>=distmax)
				*(smooth_map->hf_buf + y*diam + x) = 0 ;
			else
				*(smooth_map->hf_buf + y*diam + x) = hfmap[dist];
// printf("*(smooth_map(%d,%d) : %d\n",x,y, *(smooth_map->hf_buf + y*diam + x) );
		}
	}
	x_free(bell);
	x_free(hfmap);
	return smooth_map;
}

gint * smooth_under_crater (hf_type *buf, gint max, gint x, gint y, 
		gint diameter, gboolean wrap, gboolean subtract) {
	//	Smooth the region under the crater before applying it
	//	Fast version (hopefully), faster than using values in a map as convolution radii
	//	"buf":  the hf buffer to smooth, of size "max"
	//	"(x,y)": the center of the crater, typically the mouse position when drawing
	//	"diameter":  the diameter to smooth, may be more than the crater diameter
	//	if (subtract)
	//		Returns the difference between the smoothed image and the original
	//		(after applying the crater, we can remove the lowest frequencies
	//		in this difference, than add it to the crater for keeping more natural surface noise)
	//	if (!subtract)
	//		Returns NULL
	//	Algorithm:
	//	We average the N-W-E-S points on the circle delimited by "diameter" (cross),
	//	then put this value in the center.
	//	Each pixel between the boundaries of the circle and half the radius is averaged
	//	with the center value, proportionnally to the distance (actually non linear but gaussian)
	//	Each pixel between the center and half the radius is made equal to the center
	gint i,j, x0,y0,x1,y1, idx;
	hf_type center;
	gint *rest;
	gdouble dist, dbx, dby, dradius, dval, dval2, prop; // dmax, 
	if (subtract)
		rest = (gint *) x_calloc(diameter*diameter, sizeof(gint), "gint (rest in smooth_under_crater)");
	else
		rest = NULL;
	
	if (wrap) {
		x0 = x - (diameter>>1);
		x1 = x0 + diameter;
		y0 = y - (diameter>>1);
		y1 = y0 + diameter;
	}
	else {
		x0 = MAX(0, x - (diameter>>1 ) );
		x1 = MIN(max, x0 + diameter);
		y0 = MAX(0, y - (diameter>>1) );
		y1 = MIN(max, y0 + diameter);
	}
		
	// We average (x,y0) + (x1,y) + (x,y1) + (x0,y) in (x,y)

	dval = 	(gdouble) *(buf +VECTORIZE(x,WRAP2(y0,max),max) ) +
		(gdouble) *(buf +VECTORIZE(WRAP2(x1,max),y,max) ) +
		(gdouble) *(buf +VECTORIZE(x,WRAP2(y1,max),max) ) +
		(gdouble) *(buf +VECTORIZE(WRAP2(x0,max),y,max) ) ;

	center = (hf_type) (dval / 4.0);
	
// printf("AVERAGE: %u; (x,y): (%d,%d); (x0,y0): (%d,%d); (x1,y1): (%d,%d); \n",center,x,y,x0,y0,x1,y1);
	dbx = (gdouble) x;
	dby = (gdouble) y;
	dradius = (gdouble) diameter / 2.0;
	//dmax = (gdouble) diameter / 4.0;
	for (i=x0; i<x1; i++) {
		for (j=y0; j<y1; j++) {
			idx = VECTORIZE(WRAP2(i,max),WRAP2(j,max),max) ;
			dist = DIST2( dbx, dby, (gdouble) i, (gdouble) j);
			if (dist>dradius)
				continue;
			dval = (gdouble) *(buf +idx);
			// Equivalent of "wide filter"
			if (dist)
				prop = BELL(6.0, 5.0, dist, dradius);
			else
				prop = 1.0;
			dval2 = (1-prop) * dval  + prop * center;
			if (subtract)	
				*(rest+VECTORIZE(i-x0,j-y0,diameter) ) = 
					(gint) *(buf +idx) - 
					(gint) dval2 ;
			*(buf +idx ) = (hf_type) MAX(0,MIN(dval2,65535.0));
		}
		if (dist>dradius)
			continue;
	}
	return rest;
	
}

gboolean can_be_drawn (draw_crater_struct *dcs, gint diameter, 
		gint x, gint y, gint slope_threshold,
 		hf_type *buf,gint max, gint wrap) {
//	Check if a crater of diameter "diam" can be drawn
//	centered at (x,y) in hf_buf, given slope_threshold
//	"slope_threshold" is a percent of the max height (0xFFFF)
//	We only check the difference between the min and max of
//	the surface delimited by the crater

	gint i,j, x0,y0,x1,y1, idx;	
	hf_type vmin, vmax;
	long int dif;
	if (wrap) {
		x0 = x - (diameter>>1);
		x1 = x0 + diameter;
		y0 = y - (diameter>>1);
		y1 = y0 + diameter;
	}
	else {
		x0 = MAX(0, x - (diameter>>1 ) );
		x1 = MIN(max, x0 + diameter);
		y0 = MAX(0, y - (diameter>>1) );
		y1 = MIN(max, y0 + diameter);
	}
	vmin = 0xFFFF;
	vmax = 0;
	for (i=x0; i<x1; i++) {
		for (j=y0; j<y1; j++) {
			idx = VECTORIZE(WRAP2(i,max),WRAP2(j,max),max) ;
			if (*(buf+idx) < vmin)
				vmin = *(buf+idx);
			if (*(buf+idx) > vmax)
				vmax = *(buf+idx);
		}
	}
// printf("VMIN: %u; VMAX: %u\n",vmin,vmax);
	dif = (long int) (vmax-vmin);
// printf("DIF: %d; SL: %d\n",dif, ((long int) slope_threshold * (long int) 0xFFFF) / 100);
	if (dif>((long int) slope_threshold * (long int) 0xFFFF) / 100)
		return FALSE;
	else
		return TRUE;
}

gint get_random_diameter (draw_crater_struct *dcs) {
	gint dif, diam;	
	// Diameter is random between boundaries 1 and 2
	dif = ABS(dcs->diam_bound1 - dcs->diam_bound2);
	dcs->seed = rand();
	diam = dcs->seed%dif;
	if (diam && (dcs->distribution>1) ) {
		diam = pow( (gdouble) diam, (gdouble) dcs->distribution) /
			pow( (gdouble) dif , (gdouble) dcs->distribution-1.0);
	}
	diam = MIN(dcs->diam_bound1,dcs->diam_bound2) + diam;
	return diam;
}

gint *dig_crater (draw_crater_struct *dcs, hf_struct_type *hf, 
	gint diam, gint x, gint y, gboolean return_remainder, gboolean wrap,
	gint *s_map_diam, gdouble **gauss_list ) {

//	Common processes for draw_crater and draw_many_craters
//	Formal output:  a remainder of the smoothing process (surface noise)
//	Also, the diameter of the smooth map is returned in *s_map_diam
//	for the draw_crater function (size of the remainder to add afterwards)

	gint *sqrmap, *remainder=NULL, depth, smooth_radius, smooth_map_diam;
	gdouble *v;

//	1. Determine the depth from the relative %
	depth = get_depth (dcs,diam,hf->max_x);

//	2. Calculate the square map and apply depth to it
	sqrmap = get_sqrmap (dcs, diam, depth);

//	3. Smooth the map to remove aliasing, if no noise added after
	// Best smoothing (empirical) = log2i(diam)
	smooth_radius = log2i(diam);
	if (gauss_list) {
		if (!gauss_list[smooth_radius])
			gauss_list[smooth_radius] = normalized_bell_new(smooth_radius);
		v = gauss_list[smooth_radius];
	}
	else
		v = normalized_bell_new(smooth_radius);
	convolve_normalized_vector (sqrmap,
			sqrmap, 
			diam, 
			diam, 
			FALSE, 
			smooth_radius, 
			v,
			GINT_ID);
	
//	4. Prepare the surface

	// We need preferably an even smooth map diameter
	smooth_map_diam = ( ((gint) (1.9* (gfloat) diam))>>1)<<1;
// printf("SMOOTH_MAP_DIAM: %d\n",smooth_map_diam);

	remainder = smooth_under_crater (hf->hf_buf, hf->max_x, x, y, 
		smooth_map_diam, wrap, dcs->surface_noise);

//	5. Apply the crater map
	generalized_merge(sqrmap, GINT_ID, diam,diam, hf->hf_buf,  HF_TYPE_ID, hf->max_x, hf->max_y, x, y,ADD, wrap, FALSE);

	x_free(sqrmap);

	if (s_map_diam)
		*s_map_diam = smooth_map_diam;

	return remainder;
}

void draw_crater (draw_crater_struct *dcs, hf_struct_type *hf, gint x, gint y, gdouble **gauss_list) {
//	Draw one crater from "dcs" into "hf" at (x,y)
//	1. Determine diameter
//	2. Determine height
//	3. Calculate the square map from the "line" map
//	4. Apply depth to the square map
//	5. Smooth the map, if no noise added after
//	6. Apply radial noise
//	7. Prepare the surface
//	    if the surface is to be smoothed before applying the map
//		if some surface noise is to be kept
//			smooth up the radius of the noise to keep
//			subtract to obtain the noise
//		Smooth up to the radius required to erase details
//		Add the surface noise, if required
//	8. Apply the map (add)

	gint diam, *remainder, *remainder2, smooth_map_diam, i;
	gdouble *v;
	gboolean wrap;
// printf ("DRAW_CRATER\n");
	if (hf->if_tiles==NULL)
		wrap = (dcs->wrap==TILING_YES) || (dcs->wrap==TILING_AUTO);
	else
		wrap = (dcs->wrap==TILING_YES) || ((dcs->wrap==TILING_AUTO) && *hf->if_tiles);

//	1. Determine diameter in pixels, from diameter in % of hf size
	if (dcs->random_diameter) {
		diam = (get_random_diameter(dcs) * hf->max_x) / 100;
	}
	else
		diam = (dcs->diameter * hf->max_x) / 100;
// printf("Diam: %d\n",diam);

//	2 to 8
	remainder = dig_crater (dcs, hf, diam, x, y, (gboolean) dcs->surface_noise, 
			wrap, &smooth_map_diam, gauss_list);

//	9. Apply the remainder,
//	    after removing the lower frequencies (surface noise = higher frequencies)
	if (remainder) {
		if (gauss_list) {
			if (!gauss_list[dcs->surface_noise])
				gauss_list[dcs->surface_noise] = normalized_bell_new(dcs->surface_noise);
			v = gauss_list[dcs->surface_noise];
		}
		else
			v = normalized_bell_new(dcs->surface_noise);
		remainder2 = (gint *) x_malloc(sizeof(gint) * smooth_map_diam*smooth_map_diam, "gint (remainder2 in draw_crater)");
		convolve_normalized_vector (remainder,
			remainder2, 
			smooth_map_diam,
			smooth_map_diam, 
			FALSE,
			dcs->surface_noise, 
			v,
			GINT_ID);
		// Subtract
		for (i=0; i<smooth_map_diam*smooth_map_diam; i++) {
//			printf("remainder(%d,%d): %d\n",i%smooth_map_diam, (gint) (i / smooth_map_diam), *(remainder+i));
			*(remainder+i) = *(remainder+i) - *(remainder2+i);
		}
		generalized_merge(remainder, GINT_ID, smooth_map_diam, smooth_map_diam, hf->hf_buf, HF_TYPE_ID, hf->max_x, hf->max_y, x, y,ADD, wrap, FALSE);

		x_free(remainder);
		x_free(remainder2);
	}
}

gint draw_many_craters (draw_crater_struct *dcs, 
			hf_struct_type *hf,
			gint qty, 
			gint peak_threshold,
			gint slope_threshold,
			gdouble **gauss_list) {

//	Draw "qty" craters randomly in "hf"
//	dcs:  width, depth, noise parameters
//		dcs->seed:  input to the srand() function (allows predictable results)
//	peak_threshold:  diameter (% of hf) over which craters have a central peak
//	slope_threshold:  slope over which a crater is not drawn
//			not used yet (2003.03.16)
//	gauss_list:  list of gaussian maps (cache for convolutions)
//	Returns:  number of craters drawn
//	This number would be different from qty if the slope_threshold is too low
//
//	Steps:
//	1.  If we keep some noise surface, smooth the whole HF and keep the remainder
//	2.  Loop over "qty"
//	2.1  Generate x.y coordinates of the next crater to draw, with the random generator
//	2.2  Determine it's diameter from the boundaries given to the random generator, in % of hf size
//	2.3  Determine the type of crater, from diameter in % (with or without peak - no irregular crater here)
//	2.4  Determine diameter in pixels, from diameter in % of hf size
//	2.5  Determine the depth in hf_type values (range 0 ... 0xFFFF)
//	2.6  Calculate the crater map, applying depth and noise to it
//	2.7  Prepare the surface where we apply the crater (smooth)
//	2.8  Merge the map with the prepared surface
//	2.9  Free the square map
//	3.  Merge the surface noise (remainder of smoothing),
//	     Free the remainder, if required

//	2.5 to 2.8 are common to "draw_crater" and "draw_many_craters"

	gint i, j, x, y, diam, qty_drawn;
	gdouble *v;
	long int value,*remainder=NULL;
	gboolean wrap;

	if (hf->if_tiles==NULL)
		wrap = (dcs->wrap==TILING_YES) || (dcs->wrap==TILING_AUTO);
	else
		wrap = (dcs->wrap==TILING_YES) || ((dcs->wrap==TILING_AUTO) && *hf->if_tiles);

// printf("SEED: %d\n",dcs->seed);
	srand(dcs->seed);

	qty_drawn = 0;
	// 1. Process noise surface
	if (dcs->surface_noise) {	
		// IMPORTANT:  we assume that "result_buf" has been initialized before entering this function
		// with the result of the last "craters rain", when the "repeat button" is clicked
		if (gauss_list) {
			if (!gauss_list[dcs->surface_noise])
				gauss_list[dcs->surface_noise] = 
					normalized_bell_new(dcs->surface_noise);
			v = gauss_list[dcs->surface_noise];
		}
		else
			v = normalized_bell_new(dcs->surface_noise);
		convolve_normalized_vector ( hf->hf_buf,
			hf->hf_buf,
			hf->max_x,
			hf->max_y,
			(hf->if_tiles==NULL) ? TRUE : (*hf->if_tiles),
			dcs->surface_noise,
			v,
			HF_TYPE_ID);
		// Keeping the remainder (the noise to add after the process)
		remainder = (long int *) x_malloc(hf->max_x*hf->max_y*sizeof(long int), "long int (remainder in draw_many_craters)");
		for (i=0; i<hf->max_x*hf->max_y; i++)
			*(remainder+i) = ((long int) *(hf->result_buf+i)) - (long int) *(hf->hf_buf+i);
	}
	qty_drawn = 0;
	for (i=0; i<qty; i++) {
		for (j=0; j<MAX_ATTEMPTS; j++) {
			// 2.1 Find size of the next crater to draw, in %, between the boundaries
			dcs->diameter = get_random_diameter(dcs);
			// 2.2 Determine the type of crater, with or without peak
			if (dcs->diameter>peak_threshold)
				dcs->type = PEAK_CRATER;
			else
				dcs->type = STANDARD_CRATER;
			// 2.3 Calculate diameter in pixels
			diam = (dcs->diameter * hf->max_x) / 100;
// printf("Diam: %d\n",diam);	
			// 2.4 Find x,y coordinates of the crater center
			dcs->seed = rand();
			value = dcs->seed % (hf->max_x * hf->max_y);
			x = value % hf->max_y;
			y = value / hf->max_x;
			if (slope_threshold && (slope_threshold<MAX_SLOPE_THRESHOLD)) {
			//	Test wheter the crater can be drawn
				if (!can_be_drawn(dcs, diam, x,y,slope_threshold, hf->hf_buf, hf->max_x,wrap)) {
					continue;
				}
			}
			// 2.5 - 2.6 Calculate depth, crater map; prepare surface, apply the map
			dig_crater (dcs, hf, diam, x, y, FALSE, wrap, NULL, gauss_list);
			qty_drawn++;
			break;
		}
	}
	// 3. Post-process the noise surface
	if (remainder) {
		for (i=0; i<hf->max_x*hf->max_y; i++) {
			value = MAX(0,MIN(0xFFFF, ((long int) *(hf->hf_buf+i) ) +*(remainder+i) ));
			*(hf->hf_buf+i) = (hf_type) value;
		}
		x_free(remainder);
	}
	return qty_drawn;
}


