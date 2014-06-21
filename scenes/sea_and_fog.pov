
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions
#include "skies.inc"

#declare rock_color = 0.8*<0.72,0.56,0.3> ;
#declare rock_normal = 3.0 ;

#include "geomorph.inc"

#declare light=<1,0.97,0.94>;

#declare sun= sphere {<0,1,0>,100
         pigment{rgb 3*light}
         finish {ambient 1}
         }

global_settings {ambient_light light}

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
		warp { turbulence 0.4 octaves 1 }
		scale 1.0/0.15
}

#declare sand1 = texture {
	pigment {colour rgb <0.95,0.85,0.75>}
        	normal {
               average
               normal_map {
                          [0.5 wvs 1.5]
                          [0.5 wrinkles 1 scale 0.0025]
                          }
                }
		finish  {
				ambient 0.15
				diffuse 0.8
                specular 0.0
				}
   }

#declare P_Cloud5 =
pigment {
    bozo
    turbulence 0.65
    octaves 6
    omega 0.3
    lambda 4
    color_map {
        [0.0   color light transmit 0.25]
        [0.3, 1.001 color Clear
                    color Clear]
    }
    scale <6, 1, 6>
}
#declare rock1 =
     texture {
	pigment {
		average
		pigment_map {
			[0.7 rock1_pigment]
			[0.3 color rock_color]
		}
	}
	normal {
		average
		normal_map  {
			[0.7 wrinkles rock_normal scale 0.002]
			[0.2 pigment_pattern {rock1_pigment} rock_normal]
		}
	}
	finish {ambient 0.1}
	scale rock_scale
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
        [0 color rgb <0.8,0.8,1>]
        [0.15 color <0.2,0.4,0>+Blue]
        [1 color <0.2,0.4,0>+Blue]
      }
      scale 2
      translate 0
    }
  }

// Clouds sphere - sky_sphere is the background

plane { y,1000
    texture {
        pigment {
                granite
                turbulence 0.1
                color_map {
                [0.4 rgbt <0,0,0,1>]
                [1.0 rgb 1]
                } 
        }
        // P_Cloud5
        finish {ambient 1}
        scale 15000
        rotate 35*y
    }
    no_shadow
    hollow
}

#declare Fog =
fog
{
  fog_type   2
  distance 3
  color      rgbt <0.6,0.6,0.8,0.05 >
  fog_offset 0.3
  fog_alt 0.25
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

#declare sea_bottom = plane {y, 0
    texture {  sand1  pigment {color rock_color} rotate y*45
    }
}

#declare water = plane {y, 0
    texture {
        pigment {color rgbt<0.0,0.4,0.8,0.35>}

       normal {bumps 0.1 translate clock scale 3*<2.5,1,1> rotate <0,-30,0>}
       finish {specular 0.9 roughness 0.05
             reflection .5 ambient 0.1 diffuse 0.3
             refraction 1 ior 1.33 caustics 1}
    }
}

/**********************************************************************************************/

object {hf  translate y*-0.01 texture {rock1 scale 0.25} scale <1.0,1.4,1.0>}

object {sea_bottom rotate y*rot_y translate <0,-0.01,0> scale 0.005  }

object {water translate 0.01 scale 0.001}

fog {Fog}

light_source {Sun  translate <2000,2000,-1000>}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
