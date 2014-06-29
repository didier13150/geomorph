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

IF (NOT DATADIR)
  SET(DATADIR ${CMAKE_INSTALL_PREFIX}/share/geomorph/${GEOMORPH_VERSION})
ENDIF(NOT DATADIR)
message(STATUS "Datadir: ${DATADIR}")
SET(LOCALE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/locale")

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
               


find_package(X11 REQUIRED)
find_package(Gettext REQUIRED)
find_package(GTK2 2.6 REQUIRED gtk)
find_package(GTKGLext REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
#find_package(JPEG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

include_directories(${CMAKE_BINARY_DIR})

if(NOT (${CMAKE_VERSION} VERSION_LESS 2.8.3))
  set(PRINT_SUMMARY ON)
  include(FeatureSummary)
endif()

add_subdirectory(src)
#add_subdirectory(scenes)
add_subdirectory(po)

if(PRINT_SUMMARY)
  feature_summary(WHAT ALL)
endif()
