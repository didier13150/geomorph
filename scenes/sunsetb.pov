// Sunset
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions   
#include "skies.inc"

// #declare light=<1,0.97,0.94>;
#declare light=<1,0.9,0.8>;
global_settings {ambient_light light}

#declare rock_color = <0.72,0.56,0.3> ;

#include "geomorph.inc"

// #declare horiz_sky_color = <0.6,0.7,1.0>;
#declare horiz_sky_color = <0.9,0.5,0.4>;

#declare sand_plane =
plane {y,0
	texture {
		pigment {colour rock_color }
		finish  { ambient 0.15 diffuse 0.8 specular 0.0 }
		scale 0.5
   	}
}

#declare sun= sphere {<0,1,0>,300
         pigment{rgb light + <0,0.1,0>}
         finish {ambient 1}
         }

#declare maincamera =
camera
{
	location	<0,  0, -0.1>	// <X Y Z>
	angle 	gl_angle
	up		y              	// which way is +up <X Y Z>
	right		aspect_ratio*x         	// which way is +right <X Y Z> and aspect ratio
	look_at	<0, 0, 0> 	// point center of view at this point <X Y Z>
}


#declare water = plane {y, 0
    texture {
       pigment {color rgbt<0.0,0.1,0.6,0.7> }
       normal {bumps 0.1 scale 3*<2.5,1,1> rotate <0,-30,0>}
       finish {specular 0.1 roughness 0.05
             reflection 0.8 ambient 0.0 diffuse 0.4
             refraction 1 ior 1.33 caustics 1}
    }
}

sky_sphere {
 pigment {
      gradient y
      color_map {

        [0 color rgb <115,50,30>/255]
        [0.01 color <150,65,30>/255]
        [0.02 color <190,115,45>/255]
        [0.03 color <220,165,95>/255]
        [0.04 color <205,160,120>/255]
        [0.05 color <165,145,135>/255]
        [0.06 color <125,125,140>/255]
        [0.07 color <95,95,120>/255]
        [0.10 color <65,60,115>/255]
        [1.0 color <30,25,80>/255]

      }
      scale 2
    }
  }

// Clouds plan / sphere - sky_sphere is the background

plane { y,1000
// sphere { <0,0,0>, 10000
   texture {
        pigment {
                granite 
                turbulence 0.1
                color_map {
                [0.4 rgbt <0,0,0,1>]
                [1.0 rgb 0.9]
                } 
        }
        finish {ambient 1}
//        scale <4000,2000,4000> // for sphere
	scale <20000,5000,20000> // for plane
        rotate 35*y
    }
    no_shadow
    hollow
}

#declare Sun =
light_source
{
  0*x // light's position (translated below)
  color rgb light  // light's color
  looks_like {sun}
}

#declare hf =
height_field
{
	png main_terrain
	smooth
	scale < 1.0, 0.3, 1.0 >
	translate <-.5, 0.0, -.5>
}

#declare Fog =
fog
{
  fog_type   2
  distance 0.7
  color rgb <150,65,30>/200 transmit 0.075
  fog_offset 0.075
  fog_alt 0.05
  turbulence 30.0 // Very high value, because the camera is near the HF
}

/**********************************************************************************************/

object {hf translate y*-0.01 texture {pigment {color rock_color } } }

object {sand_plane rotate y*rot_y scale 0.015 translate <0,0,0> }

object {water scale 0.001*<1,1,1> translate y*0.005}

fog {Fog}

light_source {Sun scale 4 translate <4000,2000, 30000>}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z>
	}
