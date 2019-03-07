#
# Test.
vala_precompile(VALA_C
  nein_test.vala
PACKAGES
  gio-2.0
  gp-nein-2.0
OPTIONS
  --thread)
add_executable(nein_test ${VALA_C})
target_link_libraries(nein_test gpnein ${GLIB2_LIBRARIES})

