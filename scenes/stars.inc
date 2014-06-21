
#declare etoiles0=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.6 Clear]
                   [0.8 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.15
            }
  #declare etoiles1=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.93 Clear]
                   [0.96 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.15
            }          
 #declare etoiles2=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.96 Clear]
                   [0.98 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.3
            }          
#declare etoiles3=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.98 Clear]
                   [0.99 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.45
            }
#declare etoiles4=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.99 Clear]
                   [0.993 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.6
            }
#declare etoiles5=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.993 Clear]
                   [0.998 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.6
            }
#declare etoiles6=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.9985 Clear]
                   [0.999 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.75
            }

#declare etoiles7=
             pigment {
                   granite
                   color_map {
                   [0.0 Clear]
                   [0.999 Clear]
                   [0.9995 White]
                   [1.0 White]
                   }
                   frequency 500
                   phase 0.75
            }


#declare noir = pigment {color Clear}

#declare voie_lactee =
           pigment {
                   gradient y
                   pigment_map {
                   [0.0 etoiles7]
                   [0.2 etoiles6]
                   [0.3 etoiles5]
                   [0.38 etoiles4]
                   [0.44 etoiles3]
                   [0.48 etoiles2]
                   [0.5 etoiles1]
                   [0.52 etoiles2]
                   [0.56 etoiles3]
                   [0.62 etoiles4]
                   [0.7 etoiles5]
                   [0.8 etoiles6]
                   [0.9 etoiles7]
                   }
                   turbulence 0.4
           }
#declare milky_way = voie_lactee

#declare galaxy_center = 
	pigment {
		spherical
               	pigment_map {
                   		[0.0 Clear]
                   		[0.1 etoiles6]
                  		[0.2 etoiles5]
                 		[0.3 etoiles4]
                  		[0.4 etoiles3]
                   		[0.5 etoiles2]
                   		[0.6 etoiles1]
                   		[0.8 etoiles0]
			[1.0 White]
                   	}
         }

#declare galaxy_center1 = 
	pigment {
		spherical
		pigment_map {
			[0.0 Clear]
			[0.3 rgb <1,0,0>]
//			[0.5 rgb <0.1,0.1,0.1>]
//			[0.9 rgb <0.5,0.5,0.5>]
			[0.7 rgb <0,1,0>]
			[1.0 rgb <0,1,0>]
//			[1.0 White]
		}
		turbulence 0.1
	}

#declare galaxy_center3 = 
	pigment {
		spherical
		pigment_map {
			[0.0 Clear]
			[0.5 rgbt <0.1,0.1,0.1,1>]
			[0.8 White]
			[1.0 White]
		}
		scale 1
	}

#declare galaxy_arm_pigment_map = 
               	pigment_map {
                  		[0.0 etoiles7]
                   		[0.05 etoiles6]
                   		[0.1 etoiles5]
                   		[0.17 etoiles4]
                   		[0.25 etoiles3]
                   		[0.35 etoiles2]
                   		[0.45 etoiles1]
                   		[0.55 etoiles1]
                   		[0.65 etoiles2]
                   		[0.75 etoiles3]
                   		[0.83 etoiles4]
                   		[0.9 etoiles5]
                   		[0.95 etoiles6]
                   	}

#declare galaxy_pigment1 = 	
	pigment {
		spiral1 3
		pigment_map {galaxy_arm_pigment_map}
           		rotate <90,0,0>
		scale 1.25
	}
#declare galaxy_pigment2 = 	
	pigment {
		spiral1 2
		pigment_map {galaxy_arm_pigment_map}
           		rotate <90,0,0>
		scale 1.25
	}

#declare galaxy_pigment3 = 	
	pigment {
		spiral1 6
           		rotate <90,0,0>
		scale 0.8
	}

#declare galaxy_elliptic = 
disc
{
  <0, 1, 0>  // center position
  y,         // normal vector
  2,       // outer radius (4.5)

texture {
	pigment {galaxy_center1}
	scale 2	// 3.5
	finish {ambient 1}
   }
}

#declare galaxy_spiral1 = 
disc
{
  <0, 1, 0>  // center position
  y,         // normal vector
  4.5,       // outer radius

texture {
	pigment {galaxy_center }
	scale 3.5
	finish {ambient 1}
} 
texture {
	pigment {
		spherical
		scale 4
		pigment_map {
			[0.0 Clear]
			[1.0 galaxy_pigment1]
		}
	}
	finish {ambient 1}
} 
}

#declare galaxy_spiral2 = 
disc
{
  <0, 1, 0>  // center position
  y,         // normal vector
  4.5,       // outer radius

texture {
	pigment {galaxy_center}
	scale 3.5
	finish {ambient 1}
} 
texture {
	pigment {
		spherical
		scale 4
		pigment_map {
			[0.0 Clear]
			[1.0 galaxy_pigment2]
		}
	}
	finish {ambient 1}
} 
}

#declare galaxy_spiral3 = 
disc
{
  <0, 1, 0>  // center position
  y,         // normal vector
  4,       // outer radius

texture {
	pigment {
		spherical
		scale 4
		pigment_map {
			[0.0 Clear]
			[1.0 galaxy_pigment3]
		}
	}
	finish {ambient 0.5}
} 

texture {
	pigment {galaxy_center3}
	scale 4
	finish {ambient 1}
} 

}

