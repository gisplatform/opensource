
cmake_minimum_required( VERSION 2.6 )

include( FindPkgConfig )

project( libgpcifro )

include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/main.txt )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/arch.cmake )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/glib2.txt )
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/gtk3.txt )

if ( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
   set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif ( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )

if ( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
   set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif ( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/inc )
link_directories( ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

#
# Target.
add_subdirectory( src/gpcifroarea )
add_subdirectory( src/test )

