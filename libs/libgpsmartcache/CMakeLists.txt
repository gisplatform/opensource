cmake_minimum_required( VERSION 2.6 )

project(libgpsmartcache)

include( FindPkgConfig )

if( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

#
# Compilers options.
if( CMAKE_COMPILER_IS_GNUCC )
   set( CMAKE_C_FLAGS_RELEASE "-O2 -Wall" )
   set( CMAKE_C_FLAGS_DEBUG "-g -Wall" )
endif( CMAKE_COMPILER_IS_GNUCC )

#
# Output directories.
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )

if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )

if(NOT DEFINED EMBED_BUILD_INTEL)
  set(GP_SMART_CACHE_ENABLE_GUI TRUE)
endif(NOT DEFINED EMBED_BUILD_INTEL)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/arch.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/glib2.txt )

if(DEFINED GP_SMART_CACHE_ENABLE_GUI)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/gtk3.txt)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/elementary/Translations.cmake)

  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/libgpcore/bin)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
endif(DEFINED GP_SMART_CACHE_ENABLE_GUI)

# Vala support -->
  if( NOT DEFINED EMBED_BUILD_INTEL )
    list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/elementary)
    find_package(Vala REQUIRED)
    include(ValaVersion)
    include(ValaPrecompile)
  endif( NOT DEFINED EMBED_BUILD_INTEL )
# Vala support <--

#
# Project include and link directories.
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/inc )
link_directories( ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

#
# Target.
add_subdirectory( src/gpsmartcache )
if( NOT DEFINED EMBED_BUILD_INTEL )
  enable_testing()
  add_subdirectory( src/test )
endif( NOT DEFINED EMBED_BUILD_INTEL )
