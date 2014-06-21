
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions
#include "skies.inc"

// Water level ranges from 0 to 65535 - it's in height field world
// Notice that all height field are scaled down on the Y axis by 0.3

#ifndef (waterlevel)
#declare waterlevel = 10300.0*0.3/65535.0;
#end

// Higher value of adhesion: vegetation "sticks" on greater slopes
#declare adhesion = 0.35;
#declare rock_color = (<0.72,0.56,0.3>+<0.0,0.1,0.1>) ;
#declare sky_density=0.8;
#declare water_density=0.5;
#declare fog_distance=5;
#declare fog_transparency=0.1;
#declare fog_altitude=0.15;
#declare snow_density=1.3; // Default is 1.0
#declare snow_turbulence=0.015; // Default is 0.025
#declare snow_height = 0.4; // Default is 0.5

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

#declare Sun =
light_source
{
  0*x // light's position (translated below)
  color rgb light  // light's color
  looks_like {sun}
}

 
sky_sphere {
 pigment {
      gradient y
      color_map {
        [0 color rgb sky_density*<0.8,0.8,1>]
        [0.15 color sky_density*(<0.5,0.65,0>+Blue)]
        [0.5 color sky_density*(<0.3,0.5,0>+Blue)]
        [1.0 color rgb sky_density*<0.8,0.8,1>]
      }
	scale <1,0.5,1>
    }
        pigment {
                granite
                turbulence 0.15
                color_map {
                [0.3 rgbt <0,0,0,1>]
 //               [0.7 rgbt <0,0,0,1>] // For smaller clouds
                [0.85 rgb <0.7,0.7,0.8>]
                [1.0 rgb 0.2+<0.7,0.7,0.8>]
                }
		rotate x*10
		scale <1.5,0.4,1.5>
        }
	rotate <0,180,0.0>
 }

#declare Haze =
fog
{
  fog_type   2
  distance fog_distance
  color      rgbt <0.6,0.6,0.8,fog_transparency >
  fog_offset fog_altitude
  fog_alt 0.05
  turbulence 0.8
}

#declare hf =
height_field
{
	png main_terrain
   	smooth
 //  	water_level 0.05 // truncate/clip below N (0.0 ... 1.0)
	scale < 1.0, 0.3, 1.0 >
	translate <-.5, 0.0, -.5>
}

#if (file_exists(water_map))
	//	Water HF
	
	#declare water = 
	
	height_field
	{
		png water_map
		smooth
		scale < 1.0, 0.3, 1.0 >
		translate <-.5, 0.0, -.5>
	texture {
		pigment {color rgbt<water_density*0.0,water_density*0.15,water_density*0.35,0.1>}
	
	normal {bumps 0.0001 scale <0.003,1,0.001> }
	finish {specular 0.1 roughness 0.05
		reflection 0.3 ambient 0.05 diffuse 0.8}
	}
	}
#end

/*******************************************************************************************/

// object {hf scale <1,1,1> translate y*0 texture {gradient_forest_snow_with_wet_rock scale <0.5,1.0,0.5> } }
 object {hf scale <2.0,1,2.0> translate y*0 texture {gradient_forest_snow_with_wet_rock scale <0.5,1,0.5> } }


#ifdef (water)
//	object {water scale <1,0.98,1>}
	object {water scale <2.0,0.98,2.0>}
#end

fog {Haze}

light_source {Sun  translate <2000,2000,-2000>
		rotate <rot_x,0+rot_y,0+rot_z>}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
