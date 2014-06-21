// ==== Standard POV-Ray Includes ====
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions

#declare mapuranus = color_map {
[ 0 color <.085,.276,.246> ]
[ .042 color <.036,.427,.066> ]
[ .056 color <.085,.284,.156> ]
[ .097 color <.002,.277,.136> ]
[ .120 color <.081,.361,.181> ]
[ .132 color <.039,.307,.149> ]
[ .141 color <.060,.427,.201> ]
[ .176 color <.027,.353,.109> ]
[ .251 color <.065,.506,.197> ]
[ .270 color <.073,.302,.217> ]
[ .299 color <.063,.505,.247> ]
[ .302 color <.085,.276,.246> ]
[ .309 color <.053,.502,.105> ]
[ .349 color <.002,.412,.024> ]
[ .350 color <.031,.330,.104> ]
[ .398 color <.026,.547,.021> ]
[ .426 color <.002,.503,.137> ]
[ .483 color <.017,.477,.179> ]
[ .506 color <.055,.422,.011> ]
[ .519 color <.010,.291,.111> ]
[ .529 color <.017,.339,.155> ]
[ .565 color <.079,.443,.207> ]
[ .583 color <.014,.365,.189> ]
[ .729 color <.065,.422,.169> ]
[ .818 color <.091,.289,.141> ]
[ .835 color <.056,.421,.101> ]
[ .853 color <.079,.472,.175> ]
[ .872 color <.053,.317,.058> ]
[ .882 color <.001,.268,.135> ]
[ .959 color <.094,.392,.128> ]
[ .977 color <.022,.453,.231> ]
[ 1 color <.085,.276,.246> ]
}

#declare piguranus = pigment {
        gradient y
        color_map {mapuranus}
        phase -0.1
        scale 3
        turbulence 0.05 omega 0.6 lambda 4
        }

#declare uranus =         
union {
// torus {MAJOR, MINOR} // (in the X-Z plane)
// MAJOR = float value giving the major radius
// MINOR = float specifying the minor radius
// The major radius extends from the center of the hole
// to the mid-line of the rim while the minor radius
// is the radius of the cross-section of the rim.
torus  // Anneaux
{
  0.90,
  0.08
  texture {
          pigment {
                  wood
                  color_map {
                  [0 color rgbt <0.2,0.4,0.1,0.9>]
                  [0.1 color rgbt <0,0.1,0.3,1>]
                  [0.2 color rgbt <0,0.6,0.1,0.9>]
                  [0.3 Clear]
                  [0.5 Clear]
                  [0.5 color rgbt <0.3,0.6,0,0.92>]
                  [0.6 color rgbt <0.1,0.1,0.5,0.95>]
                  [0.7 color rgbt <0.3,0.5,0,1>]
                  [0.8 Clear]
                  [0.85 Clear]
                  [0.9 color rgbt <0.3,0.5,0.6,0.9>]
                  [1 color <0.3,0.05,0>]
                  }
                  scale 0.7
                  phase 0.88
                  rotate <-90,0,0>
           }
        finish {ambient 1 diffuse 1}
  }
  scale <5,0.1,5>
}

sphere {<0,0,0> 1
       texture {
               pigment {
                      granite
                      pigment_map {
                      [0 piguranus]
                      [0.8 piguranus translate 0.05 scale <10,1,1>]
                      [1 piguranus]
                      }
               }
        }
        scale 2.5
        finish {ambient 0.3 diffuse 1}
}
rotate <-10,0,80>

} // Union


