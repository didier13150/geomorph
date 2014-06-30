set( GeoMorph_APP_SOURCES
     app.c
     doctype.c
     document.c
     main.c
     stack.c
     thisappinit.c
)

set( GeoMorph_APP_HEADERS
     app.h
     doc.h
     globals.h
     main.h
     stack.h
     thisappinit.h
)

include_directories (${GeoMorph_SOURCE_DIR}
                     ${GeoMorph_BINARY_DIR}
                     ${PNG_INCLUDE_DIRS}
                     ${OPENGL_INCLUDE_DIR}
                     ${GTK2_INCLUDE_DIRS}
                     ${GTKGLEXT_INCLUDE_DIRS}
)
                     
add_executable( geomorph
                ${GeoMorph_APP_SOURCES}
                ${GeoMorph_APP_HEADERS}
)

target_link_libraries( geomorph
                       hf
                       fourier
                       utils
                       ${GTK2_LIBRARIES}
                       ${PC_GTKGLEXT_LIBRARIES}
                       ${PNG_LIBRARIES}
                       ${OPENGL_LIBRARIES}
)

install(TARGETS geomorph DESTINATION bin )
