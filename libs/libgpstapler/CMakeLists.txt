cmake_minimum_required( VERSION 2.6 )

include( FindPkgConfig )

project( libgpstapler )
add_definitions(-DGETTEXT_PACKAGE=\"${CMAKE_PROJECT_NAME}\")

# Vala support -->
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/elementary)
find_package(Vala REQUIRED)
include(ValaVersion)
include(ValaPrecompile)
# Vala support <--

include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/main.txt )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/arch.cmake )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/glib2.txt )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/gtk3.txt )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/require_library.txt )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/libtiff.txt )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/libgeotiff.txt )

if ( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif ( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )

if ( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif ( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libgpsmartcache/inc )
link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libgpsmartcache/bin )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libgpcifro/inc )
link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libgpcifro/bin )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")
link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libgpcore/bin )

#
# Project include and link directories.
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/inc )
link_directories( ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

#
# Target.
add_subdirectory( src/gpstapler )
add_subdirectory( src/test )
