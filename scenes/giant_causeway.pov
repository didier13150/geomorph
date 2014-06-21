
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions
#include "skies.inc"

#declare rock_color = 2*<0.3,0.35,0.3> ;
#declare rock_normal=1.0;
#declare sky_density=1;
#declare water_density=0.4;
#declare fog_distance=5;
#declare fog_transparency=0.1;
#declare fog_altitude=0.15;

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
    }
        pigment {
                granite
                turbulence 0.2
                color_map {
                [0.3 rgbt <0,0,0,1>]
 //               [0.7 rgbt <0,0,0,1>] // For smaller clouds
                [0.95 rgb <0.7,0.7,0.8>]
                [1.0 rgb <0.7,0.7,0.8>]
                } 
		scale <1.0,0.4,1.0>
        }
	rotate <0.0,-10,0.0>
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
		pigment {color rgbt water_density* <0.0,0.15,0.35,0.8>}
	
	normal {bumps 0.0001 scale <0.003,1,0.001> }
	finish {specular 1 roughness 0.005
		reflection 0.2 ambient 0.1 diffuse 0.8}
	}
	}
#end

/*******************************************************************************************/

// object {hf scale <1,1,1> translate y*0 texture {rock3 scale 0.6}}
object {hf scale <1.7,0.7,1.7> translate y*0 texture {rock3 scale 0.6}}

#ifdef (water)
//	object {water scale <1,1,1>}
	object {water scale <1.7,0.7,1.7>}
#end

fog {Haze}

light_source {Sun  translate <0,2000,3000>}
// Second light source, for backlit scenes
light_source {<0,10000,-3000> color rgb light*0.2 }

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
