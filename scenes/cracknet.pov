// cracnet.pov
// A Povray scene provided with Geomorph
// For using it: 
// 1. Define a crack network and save it as "ground_map.png"
//    It would be used as a normal map on an infinite plane.
// 2. Your current height field would be used as a background height field

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
                [0.6 rgbt <0,0,0,1>]
                [1.0 rgb 1]
                }
        }
	translate z*0.1
        finish {ambient 1}
        scale 18000
        rotate -25*y
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

	#declare hf_map =
	normal {
		bump_map
			{
			png ground_map
			map_type 0 // 0=planar, 1=spherical, 2=cylindrical, 5=torus
			interpolate 2 // 0=none, 1=linear, 2=bilinear, 4=normalized distance
			bump_size mud_strength // 0...3
			} // bump_map
			rotate <90,0,0>
			scale <1,1,1>
		}
		
	#declare hf_background =
		height_field
		{
		png main_terrain
		smooth
		//   water_level 0.001 // truncate/clip below N (0.0 ... 1.0)
			scale < 1.0, 0.3, 1.0 >
			translate <-.5, 0.0, -.5>
		}
#else

	#declare hf_map =
	normal {
		bump_map
			{
			png main_terrain
			map_type 0 // 0=planar, 1=spherical, 2=cylindrical, 5=torus
			interpolate 2 // 0=none, 1=linear, 2=bilinear, 4=normalized distance
			bump_size mud_strength // 0...3
			} // bump_map
			rotate <90,0,0>
			scale <1,1,1>
		}
		
#end

#declare Fog =
fog
{
  fog_type   2
  distance 3
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
	normal {
		average
		normal_map {
		[1.0 hf_map]
		[0.2 granite 0.2 scale <0.05,0.1,0.05>]
		} 
		rotate x*-90
	}
		
#declare mud =
texture {
#if (file_exists(ground_map))
	image_pattern {png ground_map}
#else
	image_pattern {png main_terrain}
#end
	texture_map {
	[0 pigment {color 0.05*mud_color} normal {norm_hf} finish {ambient global_ambient}]
	[0.0001 pigment {color mud_color} normal {norm_hf} finish {ambient global_ambient}]
	[1 pigment {color mud_color} normal {norm_hf} finish {ambient global_ambient}]
	}
	rotate x*90
  } // texture
  
/**********************************************************************************************/
// Texture "rock1" is from slope_patterns.inc
#ifdef (hf_background)
	object {hf_background translate y*-0.0005 texture {cliff_bands scale 1.0 } scale <1,0.7,1> }
#end

// plane {y,0 texture {mud rotate 0*y translate <-0.3,0,-0.5> scale 0.5*<1.0,2.0,1.0> } }
plane {y,0 texture {mud rotate 0*y translate <-0.3,0,-0.5> scale 0.42*<0.3,0.5,0.3> } }

fog {Fog}
fog {ground_fog}

// light_source {Sun  translate <1000,5000,-8000>   rotate y*rot_y}

light_source {Sun  translate <-10000, 15000,5000>   rotate y*rot_y}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
