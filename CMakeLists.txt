#  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 or (at your option)
#  version 3 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

project(GeoMorph)
include(version.cmake)

cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckCCompilerFlag)
#add_gcc_compiler_flags("-Wall -Wextra -Wundef -Wpointer-arith")
#add_gcc_compiler_flags("-fno-common -fstack-protector --param=ssp-buffer-size=4")
#add_gcc_compiler_flags("-Wall -Wextra -Wundef -Wpointer-arith -Wno-long-long")
#add_gcc_compiler_flags("-Wformat=2 -Wmissing-format-attribute")
#add_gcc_compiler_flags("-fvisibility=hidden")
#add_gcc_compiler_cflags("-Wchar-subscripts -Wwrite-strings")
set(CMAKE_C_FLAGS "-Wall -Wformat -Werror=format-security -Werror=implicit-function-declaration")

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -Wcast-align")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-add-needed -Wl,--as-needed -Wl,--no-undefined")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-add-needed -Wl,--as-needed")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,relro")
endif()

if (NOT DATADIR)
  set(DATADIR "${CMAKE_INSTALL_PREFIX}/share/geomorph/${GEOMORPH_VERSION}")
endif(NOT DATADIR)
message(STATUS "Data dir (DATADIR): ${DATADIR}")

if (NOT LOCALEDIR)
  set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
endif(NOT LOCALEDIR)
message(STATUS "Locale dir (LOCALEDIR): ${LOCALEDIR}")

if (NOT DESKTOPDIR)
  set(DESKTOPDIR "${CMAKE_INSTALL_PREFIX}/share/geomorph")
endif(NOT DESKTOPDIR)
message(STATUS "Desktop dir (DESKTOPDIR): ${DESKTOPDIR}")

if (NOT ICONSDIR)
  set(ICONSDIR "${CMAKE_INSTALL_PREFIX}/share/geomorph")
endif(NOT ICONSDIR)
message(STATUS "Icons dir (ICONSDIR): ${ICONSDIR}")

configure_file(${GeoMorph_SOURCE_DIR}/config.h.cmake
               ${GeoMorph_BINARY_DIR}/config.h)

configure_file(${GeoMorph_SOURCE_DIR}/install-step1-dir.cmake
               ${GeoMorph_BINARY_DIR}/install-step1-dir
               @ONLY)
               
configure_file(${GeoMorph_SOURCE_DIR}/install-step2-rcfile.cmake
               ${GeoMorph_BINARY_DIR}/install-step2-rcfile
               @ONLY)
               
configure_file(${GeoMorph_SOURCE_DIR}/install-step3-menu.cmake
               ${GeoMorph_BINARY_DIR}/install-step3-menu
               @ONLY)
               
configure_file(${GeoMorph_SOURCE_DIR}/install-step4-desktop.cmake
               ${GeoMorph_BINARY_DIR}/install-step4-desktop
               @ONLY)
               
configure_file(${GeoMorph_SOURCE_DIR}/install-user.cmake
               ${GeoMorph_BINARY_DIR}/install-user
               @ONLY)
               
configure_file(${GeoMorph_SOURCE_DIR}/update-rc.cmake
               ${GeoMorph_BINARY_DIR}/update-rc
               @ONLY)
               
configure_file(${GeoMorph_SOURCE_DIR}/v0_30_new_sections.cmake
               ${GeoMorph_BINARY_DIR}/v0_30_new_sections
               @ONLY)
               
configure_file(${GeoMorph_SOURCE_DIR}/geomorph.desktop.cmake
               ${GeoMorph_BINARY_DIR}/geomorph.desktop
               @ONLY)
               


find_package(X11 REQUIRED)
find_package(Gettext REQUIRED)
find_package(GTK2 2.6 REQUIRED gtk)
find_package(GTKGLext REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
#find_package(JPEG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
find_library(MATH_LIB NAMES m)

include_directories(${CMAKE_BINARY_DIR})

if(NOT (${CMAKE_VERSION} VERSION_LESS 2.8.3))
  set(PRINT_SUMMARY ON)
  include(FeatureSummary)
endif()

add_subdirectory(src)
add_subdirectory(scenes)
add_subdirectory(po)
add_subdirectory(doc)

if(PRINT_SUMMARY)
  feature_summary(WHAT ALL)
endif()

set( GeoMorph_DOCS     
     ${GeoMorph_SOURCE_DIR}/AFAIRE
     ${GeoMorph_SOURCE_DIR}/FAQ
     ${GeoMorph_SOURCE_DIR}/FAQ-fr
     ${GeoMorph_SOURCE_DIR}/LISEZMOI.md
     ${GeoMorph_SOURCE_DIR}/README.md
     ${GeoMorph_SOURCE_DIR}/TODO
     ${GeoMorph_SOURCE_DIR}/geomorphrc_de
     ${GeoMorph_SOURCE_DIR}/geomorphrc_en
     ${GeoMorph_SOURCE_DIR}/geomorphrc_fr
     ${GeoMorph_SOURCE_DIR}/splash.jpg     
     ${GeoMorph_BINARY_DIR}/install-step1-dir
     ${GeoMorph_BINARY_DIR}/install-step2-rcfile
     ${GeoMorph_BINARY_DIR}/install-step3-menu
     ${GeoMorph_BINARY_DIR}/install-step4-desktop
     ${GeoMorph_BINARY_DIR}/install-user
     ${GeoMorph_BINARY_DIR}/update-rc
     ${GeoMorph_BINARY_DIR}/v0_30_new_sections
)

install(FILES ${GeoMorph_DOCS} DESTINATION ${DATADIR} )
install(FILES ${GeoMorph_BINARY_DIR}/geomorph.desktop DESTINATION ${DESKTOPDIR} )
install(FILES ${GeoMorph_SOURCE_DIR}/GeoMorph.xpm DESTINATION ${ICONSDIR} )
