//	Rocks, sand, grass textures for Geomorph tutorials
//	(c) Patrice St-Gelais 2004
//	Feel free to reuse!

#ifndef (Transparent)
#declare Transparent = rgbt 1 ;
#end


// Sand

#declare ondes = normal {
                gradient x sine_wave
                turbulence 0.6 octaves 1
                rotate y*90
				}

#declare sand1 = texture {
		pigment {colour rgb <0.95,0.85,0.75>
				}
        normal {
               average
               normal_map {
                          [0.5 ondes 1.5]
                          [0.5 wrinkles 1 scale 0.0025]
                          }
                }
		finish  {
				ambient 0.15
				diffuse 0.8
                specular 0.0
				}
   }

// Grass, Forest: pigment and normal, usable on a plane or on a HF
// p_grass2 gives a wave, n_grass gives the related normal

#ifndef (factorcolor)
#declare factorcolor = 0.8 ;
#end

#ifndef (factorscale)
#declare factorscale = 4 ;
#end

#declare p_grass1 =
            pigment {
                    granite
                    color_map {
                    [0.0 factorcolor*DarkGreen]
                    [0.23 factorcolor*DarkGreen]
                    [0.23 factorcolor*ForestGreen]
                    [0.66 factorcolor*ForestGreen]
                    [1.0 factorcolor*LimeGreen]
                    }
                    scale factorscale*<0.05,0.05,1>
             }
#declare p_grass2 =
            pigment {
                    bumps scale 0.5
                    pigment_map {
                    [0.0 p_grass1 rotate 45*y]
                    [0.3 p_grass1 rotate 22*y]
                    [0.6 p_grass1]
                    [1.0 p_grass1 rotate -30*y]
                    }
  	            warp { turbulence 0.1 }
            }
#declare n_grass =
            normal {
                   bumps scale 0.5
                   normal_map {
                   [0.0 granite scale factorscale*<0.05,0.05,1> rotate 45*y]
                   [0.3 granite scale factorscale*<0.05,0.05,1> rotate 22*y]
                   [0.6 granite scale factorscale*<0.05,0.05,1> ]
                   [1.0 granite scale factorscale*<0.05,0.05,1> rotate -30*y]
                   }
	           warp { turbulence 0.1 }
            }

// Pink granite with veins (cracks)
#declare granite_1 =
        pigment {
                    crackle
                    pigment_map {
                    [0.0 rgb <0.4,0.23,0.15>]
                    [0.05 rgb <0.7,0.7,0.55>]
                    [0.2 rgb <0.9,0.73,0.65>]
                    [1.0 rgb <1,0.83,0.75>]
                    }
                    warp { turbulence 0.5 }
                    scale 0.5
            }

#declare n_granite_1 = normal {crackle 0.1 turbulence 0.5 scale 0.5}

// Usual pink granite (grainy texture)
#declare granite_2 =
        pigment {
                    wrinkles
                    pigment_map {
                    [0.0 rgb 0.2*<0.4,0.23,0.15>]
                    [0.05 rgb 0.4*<0.7,0.7,0.55>]
                    [0.2 rgb 0.6*<0.9,0.73,0.65>]
                    [1.0 rgb 0.8*<1,0.83,0.75>]
                    }
                    warp { turbulence 0.5 }
		    scale 0.2
            }

// Gradient for mountains

#declare grad_mount1 =
 texture {
    gradient y
    texture_map {
      [0.0  granite_1 scale 0.2]
      [0.1  granite_1 scale 0.2]
      [0.25  p_grass1 scale <0.1,1,0.1>]
      [1.0  p_grass1 scale <0.1,1,0.1>]
    }
   warp { turbulence 0.1 }
  }


// Gradient for shore rocks, with an underwater part
// We stack 4 textures:
// 1. A pink granite
// 2. A band of algae
// 3. Grass on the top of the rocks
// 4. A mask for darkening the wet rocks and giving some bumps
// Then we add on the top a gradient finish, for a shiny wet part

#declare shore_granite_base =
 // 1. Granite

 // texture { pigment {rgb <0.9,0.73,0.65>} // Flat granite pink

 texture { pigment { granite_2 scale 0.05 } }

  // 2. Grass on the top
 texture {
      pigment {
            gradient y
            pigment_map {
              [0.0  Transparent]
              [0.4  Transparent]
	      [0.45 color rgb <0.2,0.2,0.1> ] // Some humus
              [0.6  p_grass2 ]
              [1.0  p_grass2 ] 
            }
   	  warp { turbulence 0.1
           omega 0.8 	// How large is the step between octaves, default 0.5
  	   lambda 3		// How large is the random move, default 2.0
	   }
      }
  }
/*
// 4. A band of algae over the wet and shiny line
 texture {
      pigment {
            gradient y
            pigment_map {
              [0.0  Transparent]
              [0.2  color 1.2*ForestGreen transmit 0.4 ]
              [0.3  Transparent]
            }
      }
  }
*/
  // 5. Darkening the wet rocks
   texture {
	pigment {
                gradient y
                pigment_map {
			[0.0 color rgb <0,0,0.05> transmit 0.3]
                        [0.22 color rgb <0,0,0.05> transmit 0.5]
                        [0.3 color rgb <0,0,0> transmit 1]
                        [1 color rgb <0,0,0> transmit 1]
                }
	}
  }

  
// end shore_granite_base


// Add some specular light (it's wet!)
// and a band of algae

#declare algae_light =
	texture {pigment { color rgb <0.5,0.5,0.25> }   }
#declare algae_medium =
	texture {pigment { color rgb <0.32,0.4,0.2> }   }
#declare algae_dark =
	texture {pigment { color rgb <0.15,0.3,0.15> }   }

#declare shore_granite =
      texture {
                gradient y
                texture_map {
                        [0.0 shore_granite_base  ]
                        [0.16 shore_granite_base
 				finish {phong 0.2 phong_size 50} ]
                        [0.17 algae_light finish {phong 0}  ]
                        [0.175 algae_medium finish {phong 0}  ]
                        [0.195 algae_dark finish {phong 0}  ]
                        [0.22 shore_granite_base
			// phong = 0.0 to 1.0, 0.0 = no highlight, 1.0 = complete saturation with the light color
			// phong_size = 1.0 to 250.0; high value = small highlight = shinier
 				finish {phong 0.4 phong_size 25} ]
                        [0.25 shore_granite_base ]
                        [1 shore_granite_base  ]
                }
		translate y*-0.05 // Avoid some cycle
 	warp { turbulence 0.03
           omega 0.8 	// How large is the step between octaves, default 0.5 - + gives "mixed up" bands
  	   lambda 3		// How large is the random move, default 2.0 - ++ gives a "granular" effect
	   }
	   }
