set( GeoMorph_FOURIER_SOURCES
     fft.c
     fourier.c
     fourier_dialog.c

)

set( GeoMorph_FOURIER_HEADERS
     fft.h
     fourier.h
     fourier_dialog.h

)

include_directories (${GeoMorph_SOURCE_DIR}
                     ${GeoMorph_BINARY_DIR}
                     ${GTK2_INCLUDE_DIRS}
                     ${GTKGLEXT_INCLUDE_DIRS}
)
add_library (fourier STATIC 
             ${GeoMorph_FOURIER_HEADERS}
             ${GeoMorph_FOURIER_SOURCES}
)

target_link_libraries( fourier
                       ${GTK2_LIBRARIES}
                       ${GTKGLEXT_LIBRARIES}
                       ${PNG_LIBRARIES}
                       ${OPENGL_LIBRARIES}
)
