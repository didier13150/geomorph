/* erode.h - headers for erosion and cresting functions
 *
 * Copyright (C) 2001-2005 Patrice St-Gelais
 *         patrstg@users.sourceforge.net
 *         www.oricom.ca/patrice.st-gelais
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

#ifndef _ERODE
#define _ERODE 1

#include "craters.h"

#define MAX_DROPS 200000

typedef struct {
	gint drops;
	gint threshold; // Slope threshold
	gint strength; // Part of slope flowed
	gint hardness; // 25 to 100 - channel depth - lower = shallower and softer
	gboolean gener_frames;
	gboolean refresh;
	gint interval; // 100 to 20000 - Interval between refresh
	gboolean old_algorithm;
} rain_erosion_struct;

// Gravity: for cresting or relaxing
typedef struct {
	gint steps;
	gint threshold;
} gravity_struct;

typedef struct {
	gint direction;
	gint steps;
	gint threshold;
} oriented_gravity_struct;

// "Ribbed" erosion
typedef struct {
	gint radius;
	gint smooth_ribs;
	gboolean auto_contrast;
} whimsical_erosion_struct;

typedef struct {
	gint qty;
	draw_crater_struct *crater_str;
	gint display_interval;
	gint peak_threshold;
	gint slope_threshold;
} craters_erosion_struct;

rain_erosion_struct *rain_erosion_struct_new (gint drops, gint threshold, gint strength, gint hardness, gboolean progressive_refresh, gint interval, gboolean old_algorithm);
void rain_erosion_struct_free (rain_erosion_struct *);
void hf_rain_erosion_hex_old (hf_struct_type *hf, rain_erosion_struct *res, gpointer (*display_function) (gpointer), gpointer display_data);

gravity_struct *gravity_struct_new (gint steps, gint slope_threshold);
void gravity_struct_free (gravity_struct *);

oriented_gravity_struct *oriented_gravity_struct_new (gint steps, gint slope_threshold, gint direction);
void oriented_gravity_struct_free (oriented_gravity_struct *);

whimsical_erosion_struct *whimsical_erosion_struct_new (gint radius, gint smooth_ribs, gboolean auto_contrast);
void whimsical_erosion_struct_free (whimsical_erosion_struct *);

craters_erosion_struct *craters_erosion_struct_new (gint qty, gint display_interval, gint peak_threshold, gint slope_threshold);
void craters_erosion_struct_free (craters_erosion_struct *);

void hf_slope (hf_struct_type *hf);
void hf_eroded_ribs (hf_type *hf_in, hf_type *hf_out, gint max);
void hf_eroded_ribs_hex (hf_struct_type *hf);
// The "hex" erosion (with hexagonal sampling) gives a more natural look
// void hf_rain_erosion (hf_struct_type *hf, gint steps, gint threshold);
void hf_rain_erosion_hex (hf_struct_type *hf, rain_erosion_struct *res, gpointer (*display_function) (gpointer), gpointer display_data);
void hf_crests (hf_struct_type *hf);
// 2 versions of crests erosion, 
// the "hex" variety giving better quality (less artifacts on smooth terrains)
// void hf_crests_erosion (hf_struct_type *hf, gint steps, gint threshold);
void hf_crests_erosion_hex (hf_struct_type *hf, gint steps, gint slope_threshold);
// The last one gives smoothed tops
// void hf_crests_erosion1 (hf_struct_type *hf, gint steps, gint thresh);

void hf_relax_hex (hf_struct_type *hf, gint steps, gint slope_threshold);
void hf_oriented_relax (hf_struct_type *hf, gint steps, gint slope_threshold, gint direction);
void hf_water_erosion_hex (hf_struct_type *hf, gint steps, gint slope_threshold, gint strength);

#endif // _ERODE
