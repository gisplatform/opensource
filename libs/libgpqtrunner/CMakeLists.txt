cmake_minimum_required( VERSION 2.6 )
project( qtrunner )
include( FindPkgConfig )

if( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

# Compilers options.
if( CMAKE_COMPILER_IS_GNUCC )
   set( CMAKE_C_FLAGS_RELEASE "-O2 -Wall" )
   set( CMAKE_C_FLAGS_DEBUG "-g -Wall" )
endif( CMAKE_COMPILER_IS_GNUCC )

# Архитектура.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/)
include(../../misc/cmake/arch.cmake)

# Qt bullshit.
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

# GLib stuff.
include( ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/glib2.txt )

#
# Output directories.
if( NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY )
   set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY )

if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
   set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )

if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
   set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )

# Project include and link directories.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
link_directories(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})



# GObject Introspection -->
 list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../misc/cmake/poppler)
 include(FindGObjectIntrospection)
 include(MacroOptionalFindPackage)

 macro_optional_find_package(GObjectIntrospection 0.9.12)

 if(INTROSPECTION_FOUND)
   include(GObjectIntrospectionMacros)

   set(GpQtRunner_gir_IDENTIFIER_PREFIXES Gp)
   set(GpQtRunner_gir_SYMBOL_PREFIXES gp)

   set(GpQtRunner_gir_INCLUDES GLib-2.0)

   set(GpQtRunner_gir_LIBS gpqtrunner)
   set(GpQtRunner_gir_HEADERS inc/gp-qtrunner.h)

   set(INTROSPECTION_GIRS GpQtRunner)
   gir_add_introspections(INTROSPECTION_GIRS)
 else()
   message(WARNING "Failed to find GObjectIntrospection")
 endif ()
# GObject Introspection <--

#
# Target.
add_subdirectory(src/gpqtrunner)

