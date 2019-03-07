cmake_minimum_required( VERSION 2.8 )

project(float2hex)

include(FindPkgConfig)

include( ../../misc/cmake/main.txt )

#
# Output directories.
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
   set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )

if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
   set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )

#
# Target.
add_executable(float2hex float2hex.c)


#
# DEB/RPM для INTEL
if( DEFINED EMBED_BUILD_INTEL )
  install(TARGETS float2hex RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)
endif( DEFINED EMBED_BUILD_INTEL )
