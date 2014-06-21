
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions
#include "skies.inc"

#declare rock_color = <0.82,0.76,0.5>+<0.1,-0.1,-0.1> ;
#declare rock_turbulence = 0.05;
#declare sky_density=0.8;
#declare forest_color_density=1.2;
#declare water_density=0.5;
#declare fog_distance=10;
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
        [0.15 color sky_density*(<0.3,0.5,0>+Blue)]
        [1 color sky_density*(<0.3,0.5,0>+Blue)]
      }
    }
        pigment {
                granite
                turbulence 0.2
                color_map {
                [0.4 rgbt <0,0,0,1>]
 //               [0.7 rgbt <0,0,0,1>] // For smaller clouds
                [0.95 rgb <0.7,0.7,0.8>]
                [1.0 rgb <0.7,0.7,0.8>]
                } 
        }
	scale <1.0,0.4,1.0>
	rotate <0.0,-10,0.0>
 }

#declare Haze =
fog
{
  fog_type   2
  distance fog_distance
  color      rgbt <0.8,0.8,0.8,fog_transparency >
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

//	Dark water
#declare water = plane {y, 0
    texture {
        pigment {color rgbt<0.0,water_density*0.15,water_density*0.35,0.2>}

       normal {bumps 0.1 translate clock scale 3*<2.5,1,1> rotate <0,-30,0>}
       finish {specular 0.9 roughness 0.05
             reflection .65 ambient 0.1 diffuse 1}
    }
}
/**********************************************************************************************/

object {hf  translate y*0 texture {slope_rock_bushes translate y*-0.05 scale 0.7 } }

object {water scale 0.0005 translate 0.0002*y }

fog {Haze}

light_source {Sun  translate <0,2000,-1000>}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
