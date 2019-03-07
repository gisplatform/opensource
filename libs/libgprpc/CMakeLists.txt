cmake_minimum_required( VERSION 2.6 )

project( libgprpc )

include( FindPkgConfig )

#
# Cmake-txt
include(${PROJECT_SOURCE_DIR}/../../misc/cmake/main.txt)
include(${PROJECT_SOURCE_DIR}/../../misc/cmake/glib2.txt)
include(${PROJECT_SOURCE_DIR}/../../misc/cmake/elementary/Translations.cmake)
include(${PROJECT_SOURCE_DIR}/../../misc/cmake/arch.cmake)

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

#
# Compilers options.
if( CMAKE_COMPILER_IS_GNUCC )
  set( CMAKE_C_FLAGS_RELEASE "-O2 -Wall" )
  set( CMAKE_C_FLAGS_DEBUG "-g -Wall" )
else( CMAKE_COMPILER_IS_GNUCC )
  message( FATAL_ERROR "Unsupported compiler" )
endif( CMAKE_COMPILER_IS_GNUCC )

# Additional tuning for Windows platform
if( WIN32 )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mthreads" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mthreads" )
endif( WIN32 )

#
# Output directories.
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )

if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )

#
# Project include and link directories.
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/inc )
link_directories( ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

#
# Target.
add_subdirectory( src/gprpc )
enable_testing()
add_subdirectory( src/test )
