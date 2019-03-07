# GObject Introspection -->
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/poppler)
  include(FindGObjectIntrospection)
  include(MacroOptionalFindPackage)

  macro_optional_find_package(GObjectIntrospection 0.9.12)

  if(INTROSPECTION_FOUND)
    include(GObjectIntrospectionMacros)

    set(GpTrace_gir_IDENTIFIER_PREFIXES Gp)
    set(GpTrace_gir_SYMBOL_PREFIXES gp)

    set(GpTrace_gir_INCLUDES GObject-2.0 GLib-2.0)

    set(GpTrace_gir_LIBS gptrace)
    set(GpTrace_gir_HEADERS ../../inc/gp-trace.h)
    set(GpTrace_gir_SOURCES gp-trace.c)

    set(INTROSPECTION_GIRS GpTrace)
    gir_add_introspections(INTROSPECTION_GIRS)
  else()
    message(WARNING "Failed to find GObjectIntrospection")
  endif ()
# GObject Introspection <--

add_library( gptrace SHARED gp-trace.c )
target_link_libraries( gptrace ${GLIB2_LIBRARIES} )

