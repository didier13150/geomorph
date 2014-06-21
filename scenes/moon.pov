
// ==== Standard POV-Ray Includes ====
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions

#include "geomorph.inc"

global_settings { number_of_waves 1}

camera
{
   location  <0.0 , 0 ,-10>
  look_at   <0.0 , 0.0 , 0>
  up y*image_height
  right x*image_width
  angle 60
}

// "shadowless" for seeing bumps in shadow

light_source {
  <-20,40,-20> color White
    area_light <4, 0, 0>, <0, 0, 4>, 4, 4}
light_source {
  <-20, 40, -20> color 0.15*White shadowless}

sphere {
  <0, 0, 0>
  3.9

texture {
	pigment {color White}
	normal {
        bump_map
                {
                  png main_terrain
                  map_type 0 // 0=planar, 1=spherical, 2=cylindrical, 5=torus
                  interpolate 2 // 0=none, 1=linear, 2=bilinear, 4=normalized distance
                  bump_size 20 // 0...3
                 } // bump_map
                 rotate <0,0,0>
                 scale <1,1,1>
        } // normal
        finish {ambient <0.1,0.1,0.2>}
	scale 18*<0.25,0.25,1>
  } // texture
scale 1
rotate <0,0,0>
}

