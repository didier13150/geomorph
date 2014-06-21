/* voronoi.c - Functions for calculating voronoi diagrams,
 *	       exclusive of the dialogs
 *
 * Copyright (C) 2006 Patrice St-Gelais
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

#include "hf.h"
#include "img_process.h"
#include "voronoi.h"

// Variables for list management
#ifndef BEGIN_LIST
#define BEGIN_LIST -10
#endif
#ifndef END_LIST
#define END_LIST -1
#endif
#ifndef REALLOC_BLOCK
#define REALLOC_BLOCK 10000
#endif

static gint l_begin, l_end, l_allocated_nodes, l_max, l_current, last_node=BEGIN_LIST;
static x_y_l * ordered_list=NULL;
static gint calls=0, nsteps=0;
#define NBSUBINDEX 5000
#define FACTORSUBINDEX 5.0
static gint *subindex = NULL;

voronoi_struct *voronoi_struct_new (gdouble cell_size, gint distribution_type, gint random_variation, gint hf_use, gboolean gener_noise, gint noise_level, gint crack_width_type, gint min_width, gint max_width, gint edges_level ) {
	voronoi_struct *vs;
	vs = (voronoi_struct *) x_calloc(1,sizeof(voronoi_struct), "voronoi_struct");
	vs->cell_size = MIN(50.0, MAX(0.0, cell_size));
	if ((distribution_type == UNIFORM) || (distribution_type == CENTERED) || (distribution_type == REGULAR) )
		vs->distribution_type = distribution_type;
	else
		distribution_type = UNIFORM;
	vs->random_variation = MIN(100,MAX(0,random_variation));
	if (hf_use==USE_AS_GUIDE)
		vs->hf_use = hf_use;
	else
		vs->hf_use = USE_AS_NOISE;
	if (gener_noise)
		vs->gener_noise = TRUE;
	else
		vs->gener_noise = FALSE;
	vs->noise_level = MIN(100,MAX(0,noise_level));
	if (crack_width_type == FROM_DISTANCE)
		vs->crack_width_type = crack_width_type;
	else
		vs->crack_width_type = FIXED_WIDTH;
	if (min_width>max_width)
		min_width = max_width;
	vs->min_width = MIN(10,MAX(0,min_width));
	vs->max_width = MIN(10,MAX(0,max_width));
	vs->edges_level = MIN(100,MAX(0,edges_level));
	vs->scale = SCALE_1X;
	vs->noise_opt = subdiv1_opt_new ();
	return vs;	
};

voronoi_struct *voronoi_struct_new_with_defaults () {
	return voronoi_struct_new (10.0, UNIFORM, 50, USE_AS_NOISE, TRUE, 50, FIXED_WIDTH, 1, 2, 25);
}

void voronoi_struct_free (voronoi_struct *vs) {
	if (!vs)
		return;
	x_free(vs);
}

x_y * create_site_list (gint max_x, gint max_y, gdouble size, gint distribution, gint random_var, gint *length) {
	// Creates a pixel list in a max_x * max_y grid, randomly distributed
	// size = average diameter of each cell
	// Distribution = UNIFORM: uniformly random distribution
	// Distribution = CENTERED: more pixels in the center
	x_y *l=NULL;
	guint i,n, quantumx, quantumy;
	gint x,y, randomx, randomy;
	gdouble newsize;
	// Note: size is assumed to be a % of the dimension
	if (distribution==REGULAR)
		n = 2 * (guint) (100.0 / size / 2.0);
	else
		n = (guint) (100.0 / size);
	newsize = 100.0 / (gdouble) n;
	quantumx = (gint) (((gdouble) max_x) * newsize / 100.0);
	quantumy = (gint) (((gdouble) max_y) * newsize / 100.0);
	// For random variation of pixels placement in REGULAR:
	// Randomx and randomy are upper boundaries for variation, in pixels
	// 0 % of quantum = 0 % noise
	randomx = quantumx * 5 * random_var / 100;
	randomy = quantumy * 5 * random_var / 100;
//	printf("SIZE: %7.4f; NEWSIZE: %7.4f; NODES: %d; RANDOMX: %d; RANDOMY: %d;\n",size, newsize, n*n, randomx, randomy);
	l = (x_y *) x_malloc(sizeof(x_y)*n*n, "x_y (l in create_site_list in voronoi.c)");
	
	for (i=0; i<(n*n); i++) {
		switch (distribution) {
		case CENTERED: // 2006-01: quadratic distribution
			(l+i)->x = (gint) pow((gdouble) ( rand()%((max_x>>1)*(max_x>>1))),0.5);
			(l+i)->y = (gint) pow((gdouble) ( rand()%((max_y>>1)*(max_y>>1))),0.5);
			(l+i)->x *= 2*(rand()%2) - 1;
			(l+i)->y *= 2*(rand()%2) - 1;
			(l+i)->x = ((l+i)->x+max_x)%max_x;
			(l+i)->y = ((l+i)->y+max_x)%max_x;
			break;
		case UNIFORM:
			(l+i)->x = rand()%max_x;
			(l+i)->y = rand()%max_y;
			break;
		case REGULAR: // Kind of hexagonal
			x = i%n;
			y = i/n;
			if (!(y%2)) {// Translate half the size 
				(l+i)->x = WRAP(x * quantumx + (quantumx>>1), max_x);
//				printf("Translate %d\n",x);
			}
			else
				(l+i)->x = x * quantumx;
			(l+i)->y = y * quantumy;
			// Random factor
			if (randomx)
				(l+i)->x = WRAP((l+i)->x+rand()%randomx, max_x);
			if (randomy)
				(l+i)->y = WRAP((l+i)->y+rand()%randomy, max_y);
//			printf("X,Y: %d,%d; x,y: %d,%d\n",x,y,(l+i)->x,(l+i)->y);
		}
	}
	
	(*length) = n*n;
	return l;
}

void voronoi_adjust_edges (hf_type *in, hf_type *out, hf_type *dist, gint edges_level, gint max_x, gint max_y) {
	// We lift the edges of the cells by adding the distance from
	// the cell centers to the master grid
	gint i;
	gdouble value, ratio;
	ratio = ((gdouble) edges_level) / 100.0;
//	printf("VORONOI_ADJUST_EDGES\n");
	for (i=0; i<(max_x*max_y); i++) {
		if (!*(in+i)) {
			*(out+i) = 0;
			continue;
		}
		value = ratio * (gdouble) *(dist+i);
		*(out+i) = *(in+i) + (hf_type) value;
	}
}

void voronoi_dist_clamp (hf_type *hf_buf, gint max_x, gint max_y, hf_type newmin, hf_type newmax, gdouble *dbuf) {
	// From hf_calc::double_clamp
	// We raise the input to its square
	gdouble ratio, min, max=0.0, newmind, sum=0.0;
	static gdouble maxd = (gdouble) MAX_HF_VALUE;
	gint i;
	newmind = (gdouble) newmin;
	min = maxd;
	for (i=0; i<max_x*max_y; i++) {
		sum+=*(dbuf+i);
		if (*(dbuf+i)<min)
			min = *(dbuf+i);
		if (*(dbuf+i)>max)
			max = *(dbuf+i);
	}
//	printf("MIN: %f; MAX: %f; AVRG: %f\n",min,max, sum / (gdouble) (max_x*max_y));
	min *= min;
	max *= max;
	ratio = ((gdouble) ABS(newmax-newmin)) / (max - min);
//	printf("MIN: %f; MAX: %f; RATIO: %f\n",min,max,ratio);
	for (i=0; i< max_x*max_y; i++) {
		*(hf_buf+i) = (hf_type) ( (ratio * ( (*(dbuf+i) * *(dbuf+i)) - min) + newmind));
	}
}


void reset_list (gint length) {
	if (ordered_list)
		x_free(ordered_list);
	ordered_list = (x_y_l*) x_calloc(sizeof(x_y_l), length, "x_y_l (ordered_list in reset_list in voronoi.c)");
	l_allocated_nodes = length;
	last_node = BEGIN_LIST;
	calls = 0;
	
	if (subindex)
		x_free(subindex);
		
	subindex = (gint *) x_calloc(NBSUBINDEX,sizeof(gint), "gint (subindex in reset_list)");
	
//	printf("Allocating %d nodes\n",length);
}

void add_node (gint x, gint y, gdouble weigth) {
	// We insert the node before the first location 
	// whose weigth > weigth to add
	gint index;
	calls++;

	if (last_node==BEGIN_LIST)
		last_node = l_current;
		
	index = *(subindex + MIN(NBSUBINDEX-1,(gint) (weigth*FACTORSUBINDEX)));
	
	if (!index) 
		index =  *(subindex + MIN(NBSUBINDEX-1, 10 * (gint) (weigth*FACTORSUBINDEX/10.0)));
	if (!index) 
		index = last_node;
		
	// Index (last found target) is kept between each call
	// Generally the node to find is close to the last we've found
	// If weigth of index is > weigth to look for, we search backward
	// If weigth of index is <, we search forward
	if (weigth > (ordered_list+index)->weigth) { // Search forward
		while ( ((ordered_list+index)->next != END_LIST) && (weigth > (ordered_list+index)->weigth ) ) {
			index = (ordered_list+index)->next;
			nsteps++;

		}
	}
	else { // Search backward
		while ( ((ordered_list+index)->prec != BEGIN_LIST) && (weigth < (ordered_list+index)->weigth ) ) {
			index = (ordered_list+index)->prec;
			nsteps++;
		}
		if (index != BEGIN_LIST) {
			if ((ordered_list+index)->next != END_LIST)
				index = (ordered_list+index)->next;
		}
		else
			index = l_begin;
	}
	l_max = l_max + 1;
	if (l_allocated_nodes<=l_max) {
		l_allocated_nodes += REALLOC_BLOCK;
		ordered_list = (x_y_l*) x_realloc(ordered_list, l_allocated_nodes*sizeof(x_y_l), "x_y_l (ordered_list - realloc)");
	}
	(ordered_list+l_max-1)->x = x;
	(ordered_list+l_max-1)->y = y;
	(ordered_list+l_max-1)->weigth = weigth;
	// index == -1?? (END_LIST)
	if ( ((ordered_list+index)->next==END_LIST) && ((ordered_list+index)->weigth < weigth) ) {
		// Add a node at the end
		// Index points to the last node
//		printf("Insert %d at the end - index: %d\n",l_max-1, index);
		(ordered_list+l_max-1)->prec = index;
		(ordered_list+l_max-1)->next = END_LIST;
		(ordered_list+index)->next = l_max-1;
		l_end = l_max;
	}
	else {
		// Insert the node before the current one
//		printf("Insert %d before %d\n",l_max-1,index);
		(ordered_list+l_max-1)->prec = (ordered_list+index)->prec;
		(ordered_list+l_max-1)->next = index;
		if ((ordered_list+index)->prec != BEGIN_LIST)
			(ordered_list+(ordered_list+index)->prec)->next = l_max-1;
		(ordered_list+index)->prec = l_max-1;
	}
//	if (index==l_begin)
//		printf("******************* Insert %d before l_begin (%d)\n",l_max-1,l_begin);
	last_node = index;
	*(subindex+MIN(NBSUBINDEX-1,(gint) (weigth*FACTORSUBINDEX))) = index;
	if (!*(subindex+MIN(NBSUBINDEX-1, 10 * (gint) (weigth*FACTORSUBINDEX/10.0))))
		*(subindex + MIN(NBSUBINDEX-1, 10 * (gint) (weigth*FACTORSUBINDEX/10.0))) = index;
}

void delete_node (gint index) {
	// Index is in base 0
	// The nodes are not physically destroyed
	// We only change the pointers to "shortcut" the current node
	
	// Change the next pointer in the preceding node
	if (index != l_begin) {
		if (index == l_end) {
			(ordered_list+(ordered_list+index)->prec)->next = END_LIST;
			l_end = (ordered_list+index)->prec;
		}
		else {
			(ordered_list+(ordered_list+index)->prec)->next = (ordered_list+index)->next;
		}
	}
	
	// Change the preceding pointer in the next node
	if (index != l_end) {
		if (index == l_begin) {
			(ordered_list+(ordered_list+index)->next)->prec = BEGIN_LIST;
			l_begin = (ordered_list+index)->next;
		}
		else {
			(ordered_list+(ordered_list+index)->next)->prec = (ordered_list+index)->prec;
		}
	}
}

void expand_seeds (gdouble *dist, gint *sites, gint *nodes, hf_type *noise_in, hf_type *guide_in, gint max_x, gint max_y, gint length, gdouble nlevel) {
	
	hf_type site, *guide=NULL, *noise=NULL;
	gint n, x, y, xx, yy, index, shift;
	gdouble dx, dy, wx, wy, weigth, new_dist, base_noise, nnoise;
	x_y_l pixel;
	
//	printf ("LENGTH: %d; NLEVEL: %7.3f\n",length, nlevel);	
	// 2. Expand the seeds up to common borders with areas defined by neighbour seeds
	// We loop through the list until all the current pixels
	// are seen, and there are no new neighbour pixels to add
	// This step outputs 1 main structure:
	// dist = distances matrix - the maxima (crests) define the borders
	
	if (noise_in) {
		noise = (hf_type *) x_malloc(sizeof(hf_type)*max_x*max_y, "hf_type (noise_in in expand_seeds in voronoi.c)");
		memcpy(noise, noise_in, sizeof(hf_type)*max_x*max_y);
	}
	if (guide_in) {
		guide = (hf_type *) x_malloc(sizeof(hf_type)*max_x*max_y, "hf_type (guide in expand_seeds in voronoi.c)");
		memcpy(guide, guide_in, sizeof(hf_type)*max_x*max_y);	
	}
	
	// l_* variables are global, static
	
	// l_max is always the next node to initialize (we're in base 0!)
	l_max = length;
		
	l_begin = 0;
	l_end = l_max;
	l_current = 0;
			
//	printf("Begin: %d; End: %d; Current: %d; Max: %d\n", l_begin, l_end, l_current, l_max);
	// l_current is always the address of the node with the lowest weight (dist+noise)
	
	while (l_current!=END_LIST) {

		memcpy(&pixel,ordered_list + l_current,sizeof(x_y_l));
		index = (WRAP2(pixel.y,max_y)*max_x) + WRAP2(pixel.x,max_x);
		
		// The current site is always the one with the lowest distance
		// Pixels visited as neighbours (frontier) have negative site number
		// We remove the processed site from the frontier (not anymore negative)
		
		*(sites+index) = ABS(*(sites+index));
		
		// Process each neighbour (crux shape)
		// (x-1, y), (x, y-1), (x, y+1), (x+1, y)
		// We add the neighbours to the heap
		// For each neighbour, check if it's in a region
		// Cases:
		// (1) It's not in any region (site # == 0)
		//	==> add it to the frontier
		// (2) It's in a region with the same #, but negative
		//	==> it's already listed as a neighbour, don't add it
		// (3) It's in the same region, positive number
		//	==> already processed, don't add it
		// (4) It's in another region, positive number
		// 	If the weigth is higher, add it to the frontier

		weigth = pixel.weigth;
		(ordered_list+l_current)->weigth = 0.0;
			
		shift = ABS(pixel.y)%2; // We shift indexes for odd rows (even rows have been shifted right half a pixel)
		
		for (n=1; n<7; n++) {
			switch (n) {
		//
		//             5 | 6
		//           4 | 0 | 1
		//             3 | 2
		//

			case 1:
				xx = pixel.x + 1;
				yy = pixel.y;
				dx = 1.0;
				dy = 0.0;
				break;
			case 2:
				xx = pixel.x + shift;
				yy = pixel.y + 1;
				dx = 0.5;
				dy = 1.0;
				break;
			case 3:
				xx = pixel.x - 1 + shift;
				yy = pixel.y + 1;
				dx = -0.5;
				dy = 1.0;
				break;		
			case 4:
				xx = pixel.x - 1;
				yy = pixel.y;
				dx = -1.0;
				dy = 0.0;
				break;	
			case 5:
				xx = pixel.x - 1 + shift;
				yy = pixel.y - 1;
				dx = -0.5;
				dy = -1.0;
				break;
			case 6:
				xx = pixel.x + shift;
				yy = pixel.y - 1;
				dx = 0.5;
				dy = -1.0;
			}
			x = WRAP2(xx,max_x);
			y = WRAP2(yy,max_y);
			// Test if pixel not assigned or assigned to a different site
			if (ABS(*(sites + x + y*max_x))==*(sites+index))
				continue;
			wx = pixel.wx + dx;
			wy = pixel.wy + dy;
			if (*(sites + x + y*max_x) != *(sites+index)) {
				site = ABS(*(sites+index))-1; // "site" is a base-0 index, while *sites contains site IDs (base 1)
				base_noise = 0.0;
				if (noise)
					base_noise =  *(noise+x+y*max_x) ;
				if (guide)
					base_noise +=  5.0 * *(guide+x+y*max_x);
				nnoise = nlevel * base_noise  / 100.0;
				
				new_dist = sqrt (wx*wx + wy*wy);
				
				if (!*(sites + x + y*max_x)) { // New assignment
					// Assign site, evaluate distance, add to frontier
					// Site number is negative for pixels which
					// are part of a frontier
					*(sites + x + y*max_x) = -*(sites+index);
					*(dist + x + y*max_x) = DIST2(xx,yy,(ordered_list+site)->x, (ordered_list+site)->y);
					if (guide && *(guide + x + y*max_x))
						*(dist + x + y*max_x) += 5.0;
	
					// Add to frontier
					add_node ( xx, yy, new_dist + nnoise );
	
					(ordered_list+l_max-1)->wx = wx + dx;
					(ordered_list+l_max-1)->wy = wy + dy;
					*(nodes + x + y*max_x) = l_max-1;
				}
				else { // Already assigned
					// 2 cases:
					// 1. It's a negative site (belongs to a frontier)
					//  ==> assign to the current frontier
					// 2. It's a positive site
					//  ==> do nothing, it's "frozen"

					if ((*(sites + x + y*max_x) < 0) && ((new_dist+nnoise) < weigth)) {
						*(sites + x + y*max_x) = -*(sites+index);
						*(dist + x + y*max_x) =  DIST2(xx,yy,(ordered_list+site)->x, (ordered_list+site)->y);
						if (guide && *(guide + x + y*max_x))
							*(dist + x + y*max_x) += 5.0;
						// It must be "unwrapped"
						(ordered_list + *(nodes + x + y*max_x))->x = xx;
						(ordered_list + *(nodes + x + y*max_x))->y = yy;

						(ordered_list+*(nodes + x + y*max_x))->wx = wx + dx;
						(ordered_list+*(nodes + x + y*max_x))->wy = wy + dy;
						(ordered_list+*(nodes + x + y*max_x))->weigth = new_dist + nnoise;
					}
				
				}
			}
		}
		l_current = (ordered_list+l_current)->next;
		if (l_current != END_LIST)
			delete_node((ordered_list+l_current)->prec);
		
	} // End while
	
	if (noise)
		x_free(noise);
	if (guide)
		x_free(guide);

}

void assign_seeds (gint *nodes, gint *sites, gint max_x, gint max_y, x_y *seed_list, gint length) {
	
	gint n;

	// We manage our own ordered list
	// (using GSList makes no sense from a performance standpoint!)
	
	// Each pixel is visited at least once, so we allocate
	// a full max_x*max_y ordered list struct.
	// Then we reallocate by blocks of 10000 pixels when needed
	
	reset_list(max_x*max_y);

	// 1. Assign the sites seed

	for (n=0; n<length; n++) {
		// We start with the list of seeds
		(ordered_list+n)->prec = n-1;
		(ordered_list+n)->next = n+1;
		(ordered_list+n)->x = (seed_list+n)->x;
		(ordered_list+n)->y = (seed_list+n)->y;
		(ordered_list+n)->weigth = 0.0;
		// Base 1 (no 0 allowed as site number - 0 means not assigned)
		*(sites + (seed_list+n)->y*max_x + (seed_list+n)->x) = n+1;
		// We don't assign distance, since it's already 0
//		printf("Site #%d: (%d, %d)\n",n+1, (seed_list+n)->x, (seed_list+n)->y);
		*(nodes + (seed_list+n)->y*max_x + (seed_list+n)->x) = n;
	}
	
	(ordered_list+n-1)->next = END_LIST; // End of the list
	ordered_list->prec = BEGIN_LIST; // First node
}

void draw_cracks (hf_type *output, gdouble *dist, gint *sites, gint max_x, gint max_y, gint max_width, gint crack_width_type) {

	gint x;	
	gdouble *v;
	// Draw cracks as 1-pixel width lines
	// Cracks are crests (distance maxima separating areas - areas borders)
	if (max_width>0) {
		v = (gdouble *) x_malloc(sizeof(gdouble)*max_x*max_y, "gdouble (v in draw_cracks in voronoi.c)");
		// We add an epsilon to force inequality at sites border
		for (x=0; x<(max_x*max_y); x++)
			*(v+x) = *(dist+x) + ((gdouble) *(sites+x)) / 5.0 ;
		
		if (crack_width_type==FIXED_WIDTH) {
			// In the output: lines (edge values) are initialized at 0, the balance at VORONOI_HEIGHT
			dfind_maximum (v, output, max_x, max_y, 0, VORONOI_HEIGHT);
		}
		else { // FROM_DISTANCE: edges values == distance, balance = 0
			dfind_maximum (v, output, max_x, max_y, 0, 0);	
		}
		x_free(v);
	}
	else // No width - initialize *output to VORONOI_HEIGHT
		for (x=0; x<(max_x*max_y); x++)
			*(output+x) = VORONOI_HEIGHT;
}

void widen_cracks (hf_type *output, gint max_x, gint max_y, gint min_width, gint max_width, gint crack_width_type) {	

	hf_type *tmp;
	gint x;
	
	// Widen the cracks
	if (max_width>1) {
		tmp = (hf_type *) x_malloc(sizeof(hf_type)*max_x*max_y, "hf_type (tmp in widen_cracks in voronoi.c)");
		memcpy(tmp,output,sizeof(hf_type)*max_x*max_y);
		if (crack_width_type==FIXED_WIDTH) {
			min_max_spread (tmp, output, max_x, max_y, max_width, TRUE);
		}
		else { // FROM_DISTANCE: we "spread" the maxima
			spread_over_max (tmp, output, max_x, max_y, TRUE, min_width, max_width, 0, 0, TENT_FUNC);
			for (x=0; x<(max_x*max_y); x++)
				if (*(output+x)>0)
					*(output+x) = 0;
				else
					*(output+x) = VORONOI_HEIGHT;			
		}
		x_free(tmp);
	}
}

void voronoi (hf_type *output, hf_type *output_dist, hf_type *noise_in, hf_type *guide_in, gint max_x, gint max_y, gdouble size, gint distribution, gint random_var, gint crack_width_type, gint min_width, gint max_width, gboolean gener_noise, gint noise_level, gboolean dist_required) {

	gint *sites=NULL, *nodes=NULL, length;
	gdouble *dist=NULL;
	x_y *seed_list=NULL;
	
	// Distances matrix - the maxima define the areas borders
	dist = (gdouble *) x_calloc(sizeof(gdouble),max_x*max_y, "gdouble (dist in voronoi)");
	// Sites matrix - each site has a different number, from 1 to "length"
	// Used to test if a pixel is already assigned to a site
	sites = (gint *) x_calloc(sizeof(gint),max_x*max_y, "gint (sites in voronoi)");	
	// Nodes matrix - index of each pixel in the ordered list
	nodes = (gint *) x_calloc(sizeof(gint),max_x*max_y, "gint (nodes in voronoi)");
		
	seed_list = create_site_list (max_x, max_y, size, distribution, random_var, &length);
	
//	printf("LENGTH: %d; SIZE: %7.3f\n",length, size);
	
	// << assign_seeds>
	// Output: 
	// ordered_list (static, global, preallocated), 
	// nodes (preallocated)
	// sites (preallocated)
	assign_seeds (nodes, sites, max_x, max_y, seed_list, length);
	
	// << expand_seeds >>
	// Input:
	// Output: ordered_list, dist, sites
	
	// 2006-11-14: The noise level is scaled relatively to the cell size
	// Reference: cell size of 10% is 1.
	expand_seeds (dist, sites, nodes, noise_in, guide_in, max_x, max_y, length, ((gdouble) noise_level) / 100.0);
	
	draw_cracks (output, dist, sites, max_x, max_y, max_width, crack_width_type);
	
	widen_cracks (output, max_x, max_y, min_width, max_width, crack_width_type);

	// Adjust distance, so that it can be added with the crack array for
	// simulating the edges lifting of each area
	if (dist_required)
		voronoi_dist_clamp (output_dist, max_x, max_y, 0, MAX_HF_VALUE-VORONOI_HEIGHT, dist);
	
	if (seed_list)
		x_free(seed_list);
	if (sites)
		x_free(sites);
	if (dist)
		x_free(dist);
	if (nodes)
		x_free(nodes);
	
}

void voronoi_revert(hf_type *output, hf_type *input, gint length) {
	// Special revert function
	// Black -> white
	// Others ==
	gint i;
	for (i=0; i<length; i++) {
		if (!*(input+i))
			*(output+i) = MAX_HF_VALUE;
		else
			*(output+i) = *(input+i);
	}
}
void voronoi_undo_revert(hf_type *output, hf_type *input, gint length) {
	// See the preceding function
	gint i;
	for (i=0; i<length; i++) {
		if (*(input+i)==MAX_HF_VALUE)
			*(output+i) = 0;
		else
			*(output+i) = *(input+i);
	}
}

void hf_voronoi (hf_struct_type *hf, voronoi_struct *vs) {
	
	gdouble scaled_cell_size;
	gint scaled_min_width, scaled_max_width;
	hf_type *tmp=NULL, *tmp_cracks=NULL, *guide, *noise;
	
	// Convention:
	// 1. tmp_buf: original HF
	// 2. tmp2_buf: distance HF
	// 3. result_buf: crackled HF
	// 4. hf_buf: output = crackled HF + distance HF (for lifting edges)
	// hf_buf is computed outside this function
	
	if (!hf->tmp_buf)
		hf_backup(hf);
	if (!hf->result_buf) {
		hf->result_buf = xalloc_hf_type(hf->max_x, "hf_type (result_buf in hf_voronoi)");
	}
	if (!hf->tmp2_buf) {
		hf->tmp2_buf = xalloc_hf_type(hf->max_x, "hf_type (tmp2_buf in hf_voronoi)");
	}
	
	// scaled_size is required for managing the multiple scale option
	// The biggest scale (4x or 2x) should be < 50 %
	// So, we scale down the original cell size to get it rigth, when required
	
	switch (vs->scale) {
	case SCALE_1X:
		scaled_cell_size = vs->cell_size;
		scaled_min_width = vs->min_width;
		scaled_max_width = vs->max_width;
		break;
	case SCALE_1X_2X:
		scaled_cell_size = MIN(vs->cell_size*2.0,49.0);
		scaled_min_width = MIN(vs->min_width*2,10);
		scaled_max_width = MIN(vs->max_width*2,10);
		break;
	case SCALE_1X_2X_4X:
		scaled_cell_size = MIN(vs->cell_size*4.0,49.0);
		scaled_min_width = MIN(vs->min_width*4,10);
		scaled_max_width = MIN(vs->max_width*4,10);
		break;
	default:
		scaled_cell_size = vs->cell_size;
	}
	
	srand(vs->seed);
	
	if (vs->hf_use==USE_AS_NOISE) {
		noise = hf->tmp_buf;
		guide = NULL;
	}
	else { // USE_AS_GUIDE
		guide = hf->tmp_buf;
		if (vs->gener_noise) {
			tmp = xalloc_hf_type(hf->max_x, "hf_type (tmp in hf_voronoi)");
			calc_subdiv1 (tmp, hf->max_x, hf->max_y, vs->noise_opt);
			noise = tmp;
		}
		else {
			noise = NULL;
		}
	}
	
	voronoi (hf->result_buf, hf->tmp2_buf, noise, guide, hf->max_x, hf->max_y, scaled_cell_size, vs->distribution_type, vs->random_variation, vs->crack_width_type, scaled_min_width, scaled_max_width, vs->gener_noise, vs->noise_level, (vs->scale==SCALE_1X));
	
	if (vs->distribution_type==REGULAR) {	
		// Improve edges (remove some line artifacts)	
		if (scaled_max_width>0) {
			if (tmp) x_free(tmp);
			tmp = xalloc_hf_type(hf->max_x, "hf_type (tmp in hf_voronoi)");
			memcpy(tmp,hf->result_buf,sizeof(hf_type)*hf->max_x*hf->max_y);
			improve_edges(tmp, hf->result_buf, hf->max_x, hf->max_y, TRUE, VORONOI_HEIGHT-1);
		}
		if (tmp)
			x_free(tmp);
		return;
	}
		
	// 2x scale
	if (vs->scale!=SCALE_1X) {
		tmp_cracks = xalloc_hf_type(hf->max_x, "hf_type (tmp_cracks in hf_voronoi)");
		voronoi_revert(tmp_cracks,hf->result_buf,hf->max_x*hf->max_y);
		voronoi (hf->result_buf, hf->tmp2_buf, noise, tmp_cracks, hf->max_x, hf->max_y, scaled_cell_size / 2.0, vs->distribution_type, vs->random_variation, vs->crack_width_type, scaled_min_width / 2, scaled_max_width / 2, vs->gener_noise, vs->noise_level, vs->scale==SCALE_1X_2X);
		// Merge hf->result_buf and tmp_cracks in hf->result_buf
		voronoi_undo_revert(tmp_cracks,tmp_cracks,hf->max_x*hf->max_y);
		hf_min_merge(hf->result_buf, hf->result_buf, tmp_cracks, hf->max_x*hf->max_x);
	}
	
	// 4x scale
	if (vs->scale==SCALE_1X_2X_4X) {
		tmp_cracks = xalloc_hf_type(hf->max_x, "hf_type (tmp_cracks in hf_voronoi)");
		voronoi_revert(tmp_cracks,hf->result_buf,hf->max_x*hf->max_y);
		voronoi (hf->result_buf, hf->tmp2_buf, noise, tmp_cracks, hf->max_x, hf->max_y, scaled_cell_size / 4.0, vs->distribution_type, vs->random_variation, vs->crack_width_type, scaled_min_width / 4, scaled_max_width / 4, vs->gener_noise, vs->noise_level,vs->scale==SCALE_1X_2X_4X);
		// Merge hf->result_buf and tmp_cracks in hf->result_buf
		voronoi_undo_revert(tmp_cracks,tmp_cracks,hf->max_x*hf->max_y);
		hf_min_merge(hf->result_buf, hf->result_buf, tmp_cracks, hf->max_x*hf->max_x);
	}
	// Improve edges (remove some line artifacts)	
	if (scaled_max_width>0) {
		if (tmp) x_free(tmp);
		tmp = xalloc_hf_type(hf->max_x, "hf_type (tmp in hf_voronoi)");
		memcpy(tmp,hf->result_buf,sizeof(hf_type)*hf->max_x*hf->max_y);
		improve_edges(tmp, hf->result_buf, hf->max_x, hf->max_y, TRUE, VORONOI_HEIGHT-1);
	}
	if (tmp)
		x_free(tmp);
	if (tmp_cracks)
		x_free(tmp_cracks);
};
