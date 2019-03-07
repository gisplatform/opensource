# GObject Introspection -->
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../misc/cmake/poppler)
  include(FindGObjectIntrospection)
  include(MacroOptionalFindPackage)

  macro_optional_find_package(GObjectIntrospection 0.9.12)

  if(INTROSPECTION_FOUND)
    include(GObjectIntrospectionMacros)

    set(GpRpc_gir_IDENTIFIER_PREFIXES Gp)
    set(GpRpc_gir_SYMBOL_PREFIXES gp)

    set(GpRpc_gir_INCLUDES GObject-2.0 GLib-2.0)

    set(GpRpc_gir_LIBS gprpc)
    file(GLOB GpRpc_gir_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../../inc/gp-*h")
    file(GLOB GpRpc_gir_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*c")

    set(INTROSPECTION_GIRS GpRpc)
    gir_add_introspections(INTROSPECTION_GIRS)
  else()
    message(WARNING "Failed to find GObjectIntrospection")
  endif ()
# GObject Introspection <--


add_library( gprpc SHARED
             gp-rpc.c
             gp-rpc-server.c
             gp-rpc-common.c
             gp-rpc-manager.c
             gp-rpc-data.c
             gp-rpc-auth.c
             gp-rpc-auth-cram.c
             gp-rpc-auth-cram-server.c
             urpc-server.c
             urpc.c
             trpc-server.c
             trpc.c
             srpc-common.c
             srpc-server.c
             srpc.c )

if( WIN32 )
  add_definitions( -D_WIN32_WINNT=0x0501 )
endif( WIN32 )

#
#Translate
set_property( TARGET gprpc PROPERTY COMPILE_DEFINITIONS GETTEXT_PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}" )
add_translations(.)

target_link_libraries(gprpc ${GLIB2_LIBRARIES} m)

if( UNIX )
  target_link_libraries( gprpc rt )
endif( UNIX )

