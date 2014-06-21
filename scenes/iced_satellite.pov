// ==== Standard POV-Ray Includes ====
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions

#include "stars.inc"
#include "uranus.inc"

#include "geomorph.inc"

#declare maincamera =
camera
{
   location <0,  0, -0.1>
   up y
   right aspect_ratio*x
   angle gl_angle
   look_at <0,0,0 >
}

#declare Sun = light_source {0 color White}

#declare Sky =
sphere { <0,0,0> 9900000  // avec clock,0.005)*10000-10002:  9990000
// milky_way is a gradient on y axis
       texture {pigment {milky_way
		translate <0,1,0>
                        rotate <0,0,20>
                        scale 0.5*9900000}
               }
       hollow
       finish {ambient 0.4 diffuse 0}
}

// The ground

#declare norm = 
	normal {
		crackle 2.0
		turbulence 0.2
	}

// The layered texture avoids reflexions in dark cracks
#declare ice1 = pigment {color rgbt <0, 0.25, 0.55, 0.5>}
#declare ice2 = pigment {color rgbt <0, 0.3, 0.65, 0.5>}
#declare ice3 = pigment {color rgbt <0, 0.3, 0.75, 0>}

#declare ground =
plane {y, 0

    texture { pigment {ice3}
	finish {specular 0.8 roughness 0.05 reflection 0.4 ambient 0.1 diffuse 1}
	normal {norm}
    }
    texture {
	pigment {
		crackle
            	pigment_map {
			[0.0 rgb <0.0,0.1,0.2>]
			[0.03 rgb <0.0,0.1,0.2>]
                    		[0.06 ice1] // 0.1
                    		[0.08 ice2] // 0.3
                    		[1.0 rgbt <0,0,0,1>]
                    	}
          		turbulence 0.2
	}
	finish {ambient 0.1 diffuse 1}
	normal {norm}
    }
}

#declare ground_test = plane {y, 0 texture {pigment{ice1}} }

#declare hf =
height_field
{
	png main_terrain
   	smooth
	scale <1.0,0.3,1.0>
	translate <-0.5,0.0,-0.5>
	texture  {pigment {color rgb <0.2,0.3,0.4>}}
}
/***************************************************************************************************/

object {hf rotate <0,0,0> translate <0.0,-0.005, 0.0> scale <1.0,1.35,1.0> }
object {ground translate <0,0,0> scale 0.1}
//	For testing purpose (a lot faster!):
// object {ground_test translate <0,0,0> scale 0.1}
object {Sky rotate <0,rot_y,0>}
object {uranus scale 68 translate <300, 425, 2000> rotate <0,rot_y,0>}
light_source {Sun translate <-20, 30, -50>  rotate <0,rot_y,0>}
camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }



