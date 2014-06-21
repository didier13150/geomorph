// Povray scene provided with the Geomorph package
// (c) Patrice St-Gelais, 2001-2007
// To use this scene, you have to define a crater map first 
// in craters.png, in your working directory

// ==== Standard POV-Ray Includes ====
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions

#include "stars.inc"

#declare rock_color = <0.72,0.56,0.3> ;

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

// The Sky
#declare Sky =
sphere { <0,0,0> 9900000
// milky_way is a gradient on y axis

       texture {pigment {milky_way
		translate <0,0.75,0>
                        scale 0.45*9900000}
               }

       hollow
       finish {ambient 0.4 diffuse 0}
}

#declare water = plane {y, 0
    texture {
 //       pigment {color rgbf<0.0,0.2,0.5,0.6>} // with filtering transparency
 	pigment {color rgb 1.5*<0.0,0.2,0.5>} // Opaque

       normal {bumps 0.1 scale <2.5,0.2,1> rotate <0,0,0>}
       finish {specular 0.9 roughness 0.05
             reflection .9 ambient 0.05 diffuse 0.2 }
    }
}

#declare Sun = light_source { 0 color 0.7*White shadowless}

#declare moon =
sphere {  <0, 0, 0>, 1
	texture {
		pigment {color <1.0,0.9,0.8>}
		normal {
			bump_map {
				png crater_map
				map_type 0 // 0=planar, 1=spherical, 2=cylindrical, 5=torus
				interpolate 2 // 0=none, 1=linear, 2=bilinear, 4=normalized distance
				bump_size 90
			} // bump_map
			scale <1,1,1>
		} // normal
		finish {ambient <0.1,0.1,0.2>}
		scale 1.5
	} // texture
}

#declare hf =
height_field
{
	png main_terrain
   	smooth
	scale <1.0,0.3,1.0>
	translate <-0.5,0.0,-0.5>
   	texture  {pigment {color rock_color}}
}

/***************************************************************************************************/

object {hf rotate <0,0,0> translate <0.0,-0.01, 0.0> scale <1.0,1.0,1.0> }

object {moon scale 160 rotate <0,0,0> translate <-125,270,2000>  rotate <0,rot_y,0>}

// Try this for a "moonset", with the moon centered:
// object {moon scale 300 rotate <0,0,0> translate <0,0,2000>  rotate <0,rot_y,0>}

object {water scale 0.02 translate <0.0,0.015,0.0>}

object {Sky rotate <0,rot_y,0>}

light_source {Sun  translate <-3500, 2500,0> rotate <0,rot_y,0>}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
