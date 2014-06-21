// "Slope patterns" textures
// Not suitable for Povray version older than 3.5
// Offers rock, snow, forest / grass

#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions   

#ifndef (hf_scale) // Standard HF scale on the Y axis
	#declare hf_scale=0.3 ;
#end

#ifndef (Transparent)
#declare Transparent = rgbt 1 ;
#end

#ifndef (rock_color) // For maprock1.inc
	#declare rock_color = <0.7,0.6,0.3> ;
#end

#include "maprock1.inc"	// Generated rockmap, using rock_color

#ifndef (snow_density)
#declare snow_density = 1.0 ;
#end

#ifndef (snow_color)
#declare snow_color = <0.95,0.95,1.0> ;
#end

#ifndef (snow_height)
#declare snow_height = 0.5;
#end

#ifndef (forest_color_density)
	#declare forest_color_density=1.0;
#end
#ifndef (bushes_color_density)
	#declare bushes_color_density=forest_color_density;
#end

#ifndef (rock_turbulence)
	#declare rock_turbulence=0.1;
#end

#ifndef (rock_normal)
	#declare rock_normal=1.0;
#end

#ifndef (rock_scale)
	#declare rock_scale=1.0;
#end

#ifndef (rock_pigment_scale)
	#declare rock_pigment_scale=1.0;
#end

#ifndef (adhesion)
	#declare adhesion=0.5;
#end

#ifndef (forest_normal)
	#declare forest_normal=1.0;
#end
#ifndef (forest_normal_scale)
	#declare forest_normal_scale=1.0;
#end

#ifndef (bushes_normal)
	#declare bushes_normal=1.0;
#end
#ifndef (bushes_normal_scale)
	#declare bushes_normal_scale=1.0;
#end

// Check if we should use Granite instead of Bumps 
// for the forest and bushes textures
#declare Normal = normal {bumps} ;
#ifdef (UseGranite)
	#if (UseGranite=1)
		#declare Normal = normal {granite} ;
		#declare bushes_normal = bushes_normal*0.7 ;
		#declare forest_normal = forest_normal*0.7 ;
		#declare bushes_normal_scale = bushes_normal_scale*8 ;
		#declare forest_normal_scale = forest_normal_scale*5 ;
	#end
#end

// **************** Building blocks *****************//

#declare rock1_pigment =
          	pigment {
                 gradient y
                  color_map { rock_map }
		warp {turbulence rock_turbulence octaves 2}
		scale rock_pigment_scale*0.1
           }

#declare rock1_normal = 
	normal {
		average
		normal_map  {
			[0.7 wrinkles rock_normal scale 0.005]
			[0.3 pigment_pattern {rock1_pigment} ]
		}
	}
	
#declare rock1 =
     texture {
	pigment {rock1_pigment}
	normal {rock1_normal}
	finish {ambient 0.1}
	scale rock_scale
  }

#declare normal_bushes = 
	normal {
		Normal bushes_normal*0.3
		scale 0.004*bushes_normal_scale
	}

#declare normal_forest = 
	normal {
		Normal forest_normal*0.6
		scale 0.01*forest_normal_scale
	}

#declare pigment_forest = 
	pigment {color rgb forest_color_density*<0.12 0.24 0.057>}
	
#declare forest =
   texture {
	pigment {pigment_forest}
	finish {ambient 0.1}
	normal {normal_forest}
}

#declare pigment_bushes = pigment {color rgb bushes_color_density*<0.3 0.44 0.12>}

#declare bushes =
   texture {
	pigment {pigment_bushes}
	finish {ambient 0.1}
	normal {normal_bushes}
}

#declare normal_snow =	normal {bumps 0.05
	scale 0.003 }

#declare snow = 
   texture {pigment {color rgb snow_density*snow_color}
	normal {normal_snow}
	}

#declare pigment_top_snow = 
pigment {
	gradient y
	pigment_map {
		[0.0 Transparent]
		[0.15+0.6*snow_height Transparent]
		[0.3+0.6*snow_height color rgb  snow_density*snow_color]
		[1.0 color rgb  snow_density*snow_color]
	}	
	// The turbulence creates some overflow between the summits and the floor,
	// this is why we need a slight overscaling and a translation before scaling
	translate y*-0.08
	scale <1,hf_scale+0.1,1>
	warp {turbulence 0.025
    		octaves 5
    		omega 0.9
    		lambda 4}
}

#declare normal_top_snow = 
normal {
	gradient y
	normal_map {
		[0.0 Normal 0]
		[0.15+0.6*snow_height Normal 0]
		[0.3+0.6*snow_height Normal 0.05 scale 0.003]
		[1.0 Normal 0.05 scale 0.003]
	}
	translate y*-0.08
	scale hf_scale+0.1
	warp {turbulence 0.025
    		octaves 5
    		omega 0.9
    		lambda 4}
}

#declare top_snow = 
texture {
	pigment {pigment_top_snow}
	normal {normal_top_snow}
}

#declare gradient_rock_snow  =
texture {rock1}
texture {top_snow}

#declare pigment_forest_bushes =	
pigment {
	gradient y
	pigment_map {
		[0.0 pigment_forest]
		[0.3 pigment_forest]
		[0.5 pigment_bushes]
		[1.0 pigment_bushes]
	}
	translate y*-0.075
	scale <1,hf_scale+0.08,1>
}

#declare normal_forest_bushes =
normal {
	gradient y
	normal_map {
		[0.0 normal_forest]
		[0.3 normal_forest]
		[0.5 normal_bushes]
		[0.6 normal_bushes]
		[1.0 normal_bushes]
	}
	translate y*-0.075
	scale hf_scale+0.08
}

#declare gradient_forest_bushes  =
texture { pigment {pigment_forest_bushes} normal {normal_forest_bushes}
	finish {ambient 0.1}}
	
#declare gradient_forest_snow  =
texture {gradient_forest_bushes}
texture {top_snow}


// **************** End of building blocks *****************//

// *************** Final textures ******************//

#declare slope_snow_rock = 
 texture {
	slope { <0,-1,0>, 0, 0.5}
	texture_map {
		[0.0 snow finish {diffuse 0.8}]
		[0.28+(adhesion*0.7) snow finish {diffuse 0.8}]
		[0.3+(adhesion*0.7) rock1]
		[1.0 rock1]
	}
}

#declare slope_forest_rock = 
 texture {
	slope { <0,-1,0>, 0, 0.5}
	texture_map {
		[0.0 forest scale 0.5]
		[0.25+(adhesion*0.6) forest scale 0.5]
		[0.3+(adhesion*0.6) bushes scale 0.5]
		[0.4+(adhesion*0.6) bushes scale 0.5]
		[0.45+(adhesion*0.6) rock1]
		[1.0 rock1]
	}
}

#declare slope_rock_bushes = 
 texture {
	slope { <0,-1,0>, 0, 0.5}
	texture_map {
		[0.0 bushes scale 0.5]
		[0.28+(adhesion*0.7) bushes scale 0.5]
		[0.3+(adhesion*0.7) rock1]
		[1.0 rock1]
	}
}
	
//	Two level texture:  forest / rock - snow / rock
#declare forest_rock_snow = 
 texture {
	slope { <0,-1,0>, 0, 0.5 }
	texture_map {
		[0.0 gradient_forest_snow]
		[0.28+(adhesion*0.7) gradient_forest_snow]
		[0.3+(adhesion*0.7) rock1]
		[1.0 rock1]
	}
}

#declare forest_rock_snow_bushes = 
 texture {
	slope { <0,-1,0>, 0, 0.5 }
	texture_map {
		[0.0 gradient_forest_snow]
		[0.25+(adhesion*0.5) gradient_forest_snow]
		[0.3+(adhesion*0.5) bushes]
		[0.4+(adhesion*0.5) bushes]
		[0.45+(adhesion*0.5) rock1]
		[1.0 rock1]
	}
}

//	Top covered with snow
//	Near the mountain foot, the snow is only on lower slope
#declare rock_top_snow = 
texture {rock1}
texture { 
	pigment { 
		
		slope { <0,-1,0>, 0, 0.5 }
		pigment_map {
		[0.0 color rgb snow_density*snow_color ]
		[0.28+(adhesion*0.7) color rgb  snow_density*snow_color]
		[0.3+(adhesion*0.7) Transparent]
		[1.0 Transparent]
		}
	}

}
texture {top_snow}


