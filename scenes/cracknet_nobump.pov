
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions   
#include "skies.inc"

#declare mud_color = <0.9,0.8,0.6>;
// #declare mud_color = <0.9,0.7,0.7>;
#declare rock_color = mud_color;
#declare sand_color = rock_color;
#declare rock_turbulence = 0.2;
#declare rock_normal = 0.75;
// #declare mud_strength = 300; // 20 ... 500 - ex. try 300 for an edge raising of 25
#declare mud_strength = 70; // 20 ... 500 - ex. try 300 for an edge raising of 25
#declare global_ambient = 0.2;

#include "geomorph.inc"

#declare light=<1,0.97,0.94>;

#declare sun= sphere {<0,1,0>,100
         pigment{rgb 3*light}
         finish {ambient 1}
         }

global_settings {ambient_light light}

#declare maincamera =
camera
{
	location	<0,  0, -0.1>	// <X Y Z>
	angle 	gl_angle
	up		y              	// which way is +up <X Y Z>
	right		aspect_ratio*x         	// which way is +right <X Y Z> and aspect ratio
	look_at	<0, 0, 0> 	// point center of view at this point <X Y Z>
}


sky_sphere {
 pigment {
      gradient y
      color_map {
        [0 color rgb <0.8,0.8,1.0>]
        [0.15 color rgb <0.5,0.6,1.0>]
        [0.3 color <0.2,0.4,1.0>]
        [1 color <0.2,0.4,1.0>]
      }
    }
  }

// Clouds - sky_sphere is the background

plane { y,1000
    texture {

        pigment {
                granite
                turbulence 0.5
    	    octaves 6
 	    omega 0.1
	    lambda 2
                color_map {
                [0.55 rgbt <0,0,0,1>]
                [1.0 rgb 1]
                }
        }
	translate z*0.1
        finish {ambient 1}
        scale 18000
        rotate -15*y
    }
    no_shadow
    hollow
}

#declare Sun =
light_source
{
  0 // light's position (translated below)
  color rgb light  // light's color
  looks_like {sun}
}

// If ground_map exists, we assume that main_terrain is the background
// Else, we assume that main_terrain is a ground map 
// and that there is no hf in the background

#if (file_exists(ground_map))
	#declare hf_ground =
		height_field
		{
		png ground_map
		smooth
		//  water_level 0.001 // truncate/clip below N (0.0 ... 1.0)
			scale < 1.0, 0.3, 1.0 >
			translate <-.5, 0.0, -.5>
		}
		
	#declare hf_background =
		height_field
		{
		png main_terrain
		smooth
		  water_level 0.001 // truncate/clip below N (0.0 ... 1.0)
			scale < 1.0, 0.3, 1.0 >
			translate <-.5, 0.0, -.5>
		}
#else

	#declare hf_ground =
		height_field
		{
		png main_terrain
		smooth
		//   water_level 0.001 // truncate/clip below N (0.0 ... 1.0)
			scale < 1.0, 0.3, 1.0 >
			translate <-.5, 0.0, -.5>
		}
		
#end

#declare Fog =
fog
{
  fog_type   2
  distance 5
  color      rgbt <0.7,0.7,0.7,0.01 >
  fog_offset 0.05
  fog_alt 0.15
  turbulence 0.8
}

#declare ground_fog =
fog
{
  fog_type   2
  distance 0.4
  color      rgbt <0.9,0.9,0.7,0.001 >
  fog_offset 0.005
  fog_alt 0.01
  turbulence 0.8
}

#declare norm_hf = 
	normal {granite 0.2 scale <0.05,0.025,0.05>}
		
#declare mud =
texture {
	pigment {color mud_color} normal {norm_hf} finish {ambient 0.05}
  } // texture
  
/**********************************************************************************************/
// Texture "rock1" is from slope_patterns.inc
#ifdef (hf_background)
	object {hf_background translate y*0.0 texture {cliff_bands scale 1.0 } scale 1.5*<1,0.7,1> }
#end

object {hf_ground texture {mud scale 1.0} translate z*-0.35 translate y*-0.14 scale y*0.5 scale x*0.78 scale z*0.78}

fog {Fog}
// fog {ground_fog}

light_source {Sun  translate <-10000, 15000,5000>   rotate y*rot_y}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
