vala_precompile(VALA_C
  AsyncTiler.vala
  Tasks.vala
  Tiler.vala
  MixTiler.vala
GENERATE_GIR
  GpTiler-2.0
GENERATE_VALADOC
  gp-tiler-2.0
GENERATE_VAPI
  gp-tiler
GENERATE_HEADER
  gp-tiler
PACKAGES
  gp-core-2.0
  gp-core-gui-2.0
  gp-smart-cache-2.0
  glib-2.0
  gio-2.0
  gtk+-3.0
OPTIONS
  --thread)

add_library(gpstapler SHARED gp-getup.c layer.c gp-stapler.c gp-tile-viewer.c gp-streamer.c ${VALA_C})
vala_postcompile(gpstapler GIR_NAME GpTiler GIR_VER 2.0)

# GObject Introspection -->
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/poppler)
  include(FindGObjectIntrospection)
  include(MacroOptionalFindPackage)

  macro_optional_find_package(GObjectIntrospection 0.9.12)

  if(INTROSPECTION_FOUND)
    include(GObjectIntrospectionMacros)

    set(GpStapler_gir_IDENTIFIER_PREFIXES Gp)
    set(GpStapler_gir_SYMBOL_PREFIXES gp)

    set(GpStapler_gir_INCLUDES GObject-2.0 GLib-2.0 Gtk-3.0)
    set(GpStapler_gir_INCLUDES_MY GpSmartCache-2.0 GpCifro-2.0 GpTiler-2.0 GpCore-2.0 GpCoreGui-2.0)

    set(GpStapler_gir_LIBS gpstapler)
    file(GLOB GpStapler_gir_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../../inc/gp-*h")
    file(GLOB GpStapler_gir_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "gp-*c")

    set(INTROSPECTION_GIRS GpStapler)
    gir_add_introspections(INTROSPECTION_GIRS)
  else()
    message(WARNING "Failed to find GObjectIntrospection")
  endif ()
# GObject Introspection <--

if(GEOTIFF_FOUND)
  set_property(TARGET gpstapler PROPERTY COMPILE_DEFINITIONS PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}" GEOTIFF_FOUND=1)
  target_link_libraries(gpstapler ${GTK3_LIBRARIES} ${LIBTIFF_LIBRARIES} gpcore gpcoregui gpsmartcache gpcifro pixman-1 geotiff )
else(GEOTIFF_FOUND)
  set_property(TARGET gpstapler PROPERTY COMPILE_DEFINITIONS PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}")
  target_link_libraries(gpstapler ${GTK3_LIBRARIES} ${LIBTIFF_LIBRARIES} gpcore gpcoregui gpsmartcache gpcifro pixman-1)
endif(GEOTIFF_FOUND)




