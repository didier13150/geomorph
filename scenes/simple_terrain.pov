#include "colors.inc"
#include "textures.inc"

#declare rock_color = <0.75,0.5,0.3>;

#include "geomorph.inc"

#declare maincamera =
camera
{
	location	<0,  0, -0.1>	// <X Y Z>
	angle 	gl_angle
	up		y              	// which way is +up <X Y Z>
	right		aspect_ratio*x         	// which way is +right <X Y Z> and aspect ratio
	look_at	<0, 0, 0> 	// point center of view at this point <X Y Z>
}

// Lights positions updated 2004-09-14 for trying to get more 
// consistency with the GL preview

 #declare light1 =
 	light_source { <-130, 150, -10> color White }
// GL: <-2, 1.5, 3>
 #declare light2 =
	light_source { <130, 300, 5>   color 0.5*White }
// <0.25,0.1,0.0> }
// GL: <2, 1, -1>

#declare water = plane {y, 0
    texture {
        pigment {color rgbt<0.2,0.4,0.6,0.5>}

       normal {bumps 0.001 translate clock scale <6,2,2> rotate <0,-30,0>}
       finish {
	     specular 0.1 roughness 0.05
             reflection 0.6 ambient 0.1 diffuse 0.7
             refraction 1.0 ior 1.33 caustics 1.0
	}
    }
}

object
{
height_field {
	png main_terrain
	smooth
	water_level 0.001
}
	scale < 1.0, 0.33333, 1.0 >
	translate <-.5, 0.0, -.5>
	texture
		{
		pigment { color rock_color }
		}
}

#ifdef (preview_water_level)
object {water scale <0.003,1.0,0.003> translate 0.33333*preview_water_level }
#end

light_source {light1 rotate y*rot_y}
light_source {light2 rotate y*rot_y}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
