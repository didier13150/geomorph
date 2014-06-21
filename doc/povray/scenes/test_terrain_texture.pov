#include "colors.inc"
#include "textures.inc"
#include "camera.inc"

// #declare rock_color=<0.5,0.5,0.2>;
// #include "maprock1.inc"
#include "painted_desert_map3.inc"

#declare rock_normal=1.0;
#declare rock_color=<0.7,0.6,0.3>;
#declare rock_scale=1.0;
#declare rock_pigment_scale=1.0;
#declare adhesion=0.7;
#declare forest_color_density=1.0;
#declare bushes_color_density=1.0;
#declare forest_normal_scale=1;
#declare bushes_normal_scale=1;
#declare forest_normal=1;
#declare bushes_normal=1;
#declare snow_density=1.0;
#declare snow_color=<0.95,0.95,1.0>;
#declare snow_height=0.5;
#declare UseGranite=0;
#include "slope_patterns.inc"

#declare fog_distance=70.0;
#declare fog_transparency=0.8;
#declare fog_altitude=0.08;
#declare sky_density=1;

#declare granite_density=1.5;
#declare granite1_grain_scale=5.5;
#declare grass_scale = 1.0;
#include "geomorph_txtr.inc"

#if (file_exists("global_settings.inc"))
	#include "global_settings.inc"
#end

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
 	light_source { <-130, 0.75*200, -10> color White }
 #declare light2 =
	light_source { <130, 300, 5>   color 0.4*White }


#declare strata = pigment {gradient y color_map{rock_map} warp {turbulence 0.1}}

#declare hf=

height_field {
	png "test.png"
	smooth
	scale < 1.0, 0.3, 1.0 >
	translate <-.5, 0.0, -.5>

/*	
object {hf 
texture {
	pigment {gradient y
		color_map {ground_map}
		translate y*-0.1
		turbulence 0.1
		scale 0.35
	}
	}
}
*/

/*******************************************************************/
object {hf texture {forest_rock_snow_bushes}}

light_source {light1 rotate y*rot_y}
light_source {light2 rotate y*rot_y}

camera {maincamera
		translate <0+tr_x,0.05+tr_y,tr_z>
		rotate <rot_x,0+rot_y,0+rot_z> }
		
/*******************************************************************/

/*
#declare Haze =
fog
{
  fog_type   2
  distance fog_distance
  color      rgbt <0.6,0.6,0.8,fog_transparency >
  fog_offset fog_altitude
  fog_alt fog_altitude
  turbulence 0.8
}

#declare sky_gradient = 
pigment {
	gradient y
	color_map {
		[0 color rgb sky_density*<0.8,0.8,1>]
		[0.1 color sky_density*(<0.45,0.5,1>)]
		[0.2 color sky_density*(<0.2,0.4,1>)]
		[1 color sky_density*(<0.2,0.4,1>)]
	}
}


sky_sphere {pigment {sky_gradient scale <1.0, 1.0, 1.0> } }

plane { y,1000
    texture {
        pigment {
                granite
                turbulence 0.5
    	    octaves 6
 	    omega 0.1
	    lambda 2
                color_map {
                [0.3 rgbt <0,0,0,1>]
                [1.0 rgb 1]
                }
        }
	translate z*0.0
	rotate y*-35
        finish {ambient 0.8 }
        scale 11000
    }
    no_shadow
    hollow
    rotate x*rot_x
}

fog {Haze}
*/