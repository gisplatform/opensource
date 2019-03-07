# GObject Introspection -->
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/poppler)
  include(FindGObjectIntrospection)
  include(MacroOptionalFindPackage)

  macro_optional_find_package(GObjectIntrospection 0.9.12)

  if(INTROSPECTION_FOUND)
    include(GObjectIntrospectionMacros)

    set(GpNein_gir_IDENTIFIER_PREFIXES Gp)
    set(GpNein_gir_SYMBOL_PREFIXES gp)

    set(GpNein_gir_INCLUDES GObject-2.0 GLib-2.0 Gio-2.0)

    set(GpNein_gir_LIBS gpnein)
    set(GpNein_gir_HEADERS ../../inc/gp-nein.h)
    set(GpNein_gir_SOURCES nein.c)

    set(INTROSPECTION_GIRS GpNein)
    gir_add_introspections(INTROSPECTION_GIRS)
  else()
    message(WARNING "Failed to find GObjectIntrospection")
  endif ()
# GObject Introspection <--

# Target.
add_library(gpnein SHARED nein.c)
target_link_libraries(gpnein ${GLIB2_LIBRARIES})

if(WIN32)
  target_link_libraries(gpnein ws2_32 iphlpapi)
endif(WIN32)

