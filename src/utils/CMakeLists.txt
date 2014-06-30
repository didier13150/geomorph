set( GeoMorph_UTILS_SOURCES
     config_dialog.c
     config_rc_file.c
     filesel.c
     menus_n_tools.c
     readpng.c
     scale_buttons.c
     utils.c
     writepng.c
     x_alloc.c
)

set( GeoMorph_UTILS_HEADERS
     config_dialog.h
     config_rc_file.h
     filesel.h
     menus_n_tools.h
     scale_buttons.h
     utils.h
     x_alloc.h
)

include_directories (${GeoMorph_SOURCE_DIR}
                     ${GeoMorph_BINARY_DIR}
                     ${GTK2_INCLUDE_DIRS}
                     ${GTKGLEXT_INCLUDE_DIRS}
)
add_library (utils STATIC 
             ${GeoMorph_UTILS_HEADERS}
             ${GeoMorph_UTILS_SOURCES}
)

target_link_libraries( utils
                       ${GTK2_LIBRARIES}
                       ${GTKGLEXT_LIBRARIES}
                       ${PNG_LIBRARIES}
                       ${OPENGL_LIBRARIES}
)