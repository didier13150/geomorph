
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions   
#include "skies.inc"

#include "camera.inc"

#declare rock_color = <0.72,0.5,0.3>;

#include "maprock1.inc"	// Generated rockmap

#declare light=1.25*<1,0.97,0.94>;

#declare sun= sphere {<0,1,0>,100
         pigment{rgb 3*light}
         finish {ambient 1}
         }

global_settings { assumed_gamma 1.45 ambient_light light}

#declare wvs = normal {

	gradient x
	slope_map {
		[0    <0.5, 1>]   // start in middle and slope up
		[0.4 <1.0, 0>]   // flat slope at top of wave
		[0.65  <0.5,-1>]   // slope down at mid point
		[0.85 <0.0, 0>]   // flat slope at bottom
		[1    <0.5, 1>]   // finish in middle and slope up
	}
		scale 0.15
		warp { turbulence 0.5 octaves 1 }
		scale 1.0/0.15
}

#declare sand1 = texture {
	pigment {colour rgb <0.95,0.85,0.75>}
        normal {
               average
               normal_map {
                          [0.5 wvs 0.5]
                          [0.5 wrinkles 2 scale 0.0025]
                          }
                }
		finish  {
			ambient 0.15
			diffuse 0.8
                		specular 0.0
				}
   }

#declare rock1 =

          pigment {
                  wood
                  color_map { rock_map }
                  rotate <0,0,0>
	      turbulence 0.1
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

#declare desert_plane = plane {y, 0
    texture {  sand1
	pigment {color 0.8*rock_color}
	rotate <0,45,0>
    }
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

#declare hf =
height_field
{
png "test.png"

   smooth
//   water_level 0.001 // truncate/clip below N (0.0 ... 1.0)
	scale < 1.0, 0.3, 1.0 >
	translate <-.5, 0.0, -.5>
}

#declare Fog =
fog
{
  fog_type   2
  distance 10
  color      rgbt <0.6,0.4,0.2,0.01 >
  fog_offset 0.3
  fog_alt 0.3
  turbulence 0.8
}

/**********************************************************************************************/

object {hf translate y*-0.0075 texture {pigment {color rock_color } } scale <1,0.7,1>  }

object {desert_plane rotate y*rot_y scale 0.003 translate <0,0,0> }

fog {Fog}

light_source {Sun  translate <2000,2000,-1000>}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
