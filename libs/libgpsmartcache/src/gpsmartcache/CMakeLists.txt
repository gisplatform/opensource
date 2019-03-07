# GObject Introspection -->
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/poppler)
  include(FindGObjectIntrospection)
  include(MacroOptionalFindPackage)

  macro_optional_find_package(GObjectIntrospection 0.9.12)

  if(INTROSPECTION_FOUND)
    include(GObjectIntrospectionMacros)

    set(GpSmartCache_gir_IDENTIFIER_PREFIXES Gp)
    set(GpSmartCache_gir_SYMBOL_PREFIXES gp)

    if(DEFINED GP_SMART_CACHE_ENABLE_GUI)
      set(GpSmartCache_gir_INCLUDES Gtk-3.0)
      set(GpSmartCache_gir_INCLUDES_MY GpCore-2.0 GpCoreGui-2.0)
    else(DEFINED GP_SMART_CACHE_ENABLE_GUI)
      set(GpSmartCache_gir_INCLUDES GLib-2.0 GObject-2.0)
    endif(DEFINED GP_SMART_CACHE_ENABLE_GUI)

    set(GpSmartCache_gir_LIBS gpsmartcache)
    set(GpSmartCache_gir_HEADERS ../../inc/gp-smartcache.h)
    set(GpSmartCache_gir_SOURCES gp-smartcache.c sysinfo.c)

    set(INTROSPECTION_GIRS GpSmartCache)
    gir_add_introspections(INTROSPECTION_GIRS)
  else()
    message(WARNING "Failed to find GObjectIntrospection")
  endif ()
# GObject Introspection <--


# Target.
add_definitions( -DG_LOG_DOMAIN="SMARTCACHE" )
add_library(gpsmartcache SHARED gp-smartcache.c sysinfo.c)
target_link_libraries( gpsmartcache ${GLIB2_LIBRARIES} )

if(DEFINED GP_SMART_CACHE_ENABLE_GUI)
  target_link_libraries(gpsmartcache ${GTK3_LIBRARIES} gpcore gpcoregui)
  set_property(TARGET gpsmartcache PROPERTY COMPILE_DEFINITIONS
    GP_SMART_CACHE_ENABLE_GUI=1
    PACKAGE="${PROJECT_NAME}"
    GETTEXT_PACKAGE="${PROJECT_NAME}")
  add_translations(".")
endif(DEFINED GP_SMART_CACHE_ENABLE_GUI)

