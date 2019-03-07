# GObject Introspection -->
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/poppler)
  include(FindGObjectIntrospection)
  include(MacroOptionalFindPackage)

  macro_optional_find_package(GObjectIntrospection 0.9.12)

  if(INTROSPECTION_FOUND)
    include(GObjectIntrospectionMacros)

    set(GpCifro_gir_IDENTIFIER_PREFIXES Gp)
    set(GpCifro_gir_SYMBOL_PREFIXES gp)

    set(GpCifro_gir_INCLUDES Gtk-3.0)

    set(GpCifro_gir_LIBS gpcifro)

    file(GLOB GpCifro_gir_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../../inc/gp-*h")
    file(GLOB GpCifro_gir_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "gp-*c")

    set(INTROSPECTION_GIRS GpCifro)
    gir_add_introspections(INTROSPECTION_GIRS)
  else()
    message(WARNING "Failed to find GObjectIntrospection")
  endif ()
# GObject Introspection <--

add_library( gpcifro SHARED
             gp-cifroarea.c
             gp-icarenderer.c
             gp-icastaterenderer.c
             icaaxis.c
             gp-icascope.c
             gp-icacurve.c
             gp-icacustom.c
             cairosdline.c
             gp-cifroscope.c
             gp-cifrocurve.c
             gp-cifrocustom.c )

target_link_libraries( gpcifro ${GTK3_LIBRARIES} )
