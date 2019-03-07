#
# Test
add_executable( gsmartcachetest gsmartcachetest.c )
target_link_libraries( gsmartcachetest gpsmartcache ${GLIB2_LIBRARIES} )
add_test(NAME gsmartcachetest COMMAND gsmartcachetest )

#
# System info
add_executable(sysinfo sysinfo.c )
target_link_libraries(sysinfo gpsmartcache ${GLIB2_LIBRARIES})


if(DEFINED GP_SMART_CACHE_ENABLE_GUI)
  set(_gui_pkgs
    gtk+-3.0
    gp-core-2.0
    gp-core-gui-2.0
  )
else(DEFINED GP_SMART_CACHE_ENABLE_GUI)
  set(_gui_pkgs)
endif(DEFINED GP_SMART_CACHE_ENABLE_GUI)

#
# Vala test (closure, cleaning, etc.)
vala_precompile(VALA_C
  vala_test.vala
PACKAGES
  gp-smart-cache-2.0
  ${_gui_pkgs}
OPTIONS
  --thread)
add_executable(vala_test ${VALA_C})
target_link_libraries(vala_test gpsmartcache  ${GLIB2_LIBRARIES})


