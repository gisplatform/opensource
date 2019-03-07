cmake_minimum_required( VERSION 2.6 )

include( FindPkgConfig )

project( libgpcore )


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
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/test )
link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

include_directories(${INCLUDE_DIRECTORIES} ${PROJECT_SOURCE_DIR}/../include/)

link_directories(${PROJECT_SOURCE_DIR}/bin/)


# Vala support -->
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/elementary)
  find_package(Vala REQUIRED)
  include(ValaPrecompile)
# Vala support <--

include(${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/gresources/GlibCompileResourcesSupport.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/main.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/arch.cmake)
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/glib2.txt )

# Поддержка звука только на Linux'e, причем за исключением встроенного образа для Intel.
if(UNIX AND NOT DEFINED EMBED_BUILD_INTEL)
  include(../../misc/cmake/pulse_simple.cmake)
  set(pulse_vapis ../../vapi/pulse/libpulse.vapi ../../vapi/pulse/libpulse-simple.vapi)
  set(pulse_valas Rumorista.vala)
  set(pulse_pkgs posix)
endif(UNIX AND NOT DEFINED EMBED_BUILD_INTEL)


# Отключается при сборке образа для Intel
if( NOT DEFINED EMBED_BUILD_INTEL )
  include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/gtk3.txt )
  include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/no_win32_console.txt )
endif( NOT DEFINED EMBED_BUILD_INTEL )

include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/elementary/Translations.cmake )
add_translations("src/core" "src/gui" "src/test")

add_subdirectory(src/core)

# Отключается при сборке образа для Intel
if( NOT DEFINED EMBED_BUILD_INTEL )
  add_subdirectory(src/gui)

  enable_testing()
  add_subdirectory(src/test)
endif( NOT DEFINED EMBED_BUILD_INTEL )
