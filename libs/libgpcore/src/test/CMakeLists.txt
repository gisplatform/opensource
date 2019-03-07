add_executable(activity_button_test activity_button_test.c)
target_link_libraries(activity_button_test ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES} gpcore gpcoregui)
set_property(TARGET activity_button_test PROPERTY COMPILE_DEFINITIONS PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}")

vala_precompile(DISPATCHER_TEST_VALA_C
  dispatcher_test.vala
PACKAGES
  gio-2.0
  gp-core-2.0
OPTIONS
  --thread)

add_executable(chunk_test chunk_test.c)
target_link_libraries(chunk_test ${GLIB2_LIBRARIES} gpcore)
add_test(NAME chunk_test COMMAND chunk_test)

add_executable(dispatcher_test ${DISPATCHER_TEST_VALA_C})
target_link_libraries(dispatcher_test ${GLIB2_LIBRARIES} gpcore)
add_test(NAME dispatcher_test COMMAND dispatcher_test -n 3)

add_executable(dumper_test dumper_test.c)
target_link_libraries(dumper_test ${GLIB2_LIBRARIES} gpcore)
add_test(NAME dumper_test COMMAND dumper_test)

vala_precompile(LANDLE_CLIENT_VALA_C
  landle_client.vala
PACKAGES
  gp-core-2.0
  gp-core-gui-2.0
  gtk+-3.0
OPTIONS
  --thread)

add_executable(landle_client ${LANDLE_CLIENT_VALA_C})
target_link_libraries(landle_client ${GTK3_LIBRARIES} gpcore gpcoregui)
install(TARGETS landle_client RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin) #< DEB/RPM
set_property(TARGET landle_client PROPERTY COMPILE_DEFINITIONS GETTEXT_PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}")

vala_precompile(LANDLE_SERVER_VALA_C
  landle_server.vala
PACKAGES
  gp-core-2.0
  gp-core-gui-2.0
  gtk+-3.0
OPTIONS
  --thread)

add_executable(landle_server ${LANDLE_SERVER_VALA_C})
target_link_libraries(landle_server ${GTK3_LIBRARIES} gpcore gpcoregui)
install(TARGETS landle_server RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin) #< DEB/RPM
set_property(TARGET landle_server PROPERTY COMPILE_DEFINITIONS GETTEXT_PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}")

vala_precompile(TREEVIEW_LISTSTORE_VALA_C
  treeview_liststore.vala
PACKAGES
  gio-2.0
  gp-core-2.0
  gp-core-gui-2.0
  gtk+-3.0
OPTIONS
  --thread)

add_executable(treeview_liststore ${TREEVIEW_LISTSTORE_VALA_C})
target_link_libraries(treeview_liststore ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES} gpcore gpcoregui)
set_property(TARGET treeview_liststore PROPERTY COMPILE_DEFINITIONS GETTEXT_PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}")

vala_precompile(MAGNETIC_DECLINATION_VALA_C
  magnetic_declination.vala
CUSTOM_VAPIS
  ../libgpcore/src/test/magnetic_model_and_geo_point.vapi
PACKAGES
  glib-2.0
  gtk+-3.0
  gp-core-2.0
OPTIONS
  --thread)

add_executable(magnetic_declination ${MAGNETIC_DECLINATION_VALA_C} magnetic_model.c)
target_link_libraries(magnetic_declination ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES} gpcore)

add_executable(menu_tree_test menu_tree_test.c)
target_link_libraries(menu_tree_test ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES} gpcore gpcoregui)

vala_precompile(CP_VALA_C
  cp.vala
PACKAGES
  gio-2.0
  glib-2.0
  gp-core-2.0
OPTIONS
  --thread)

add_executable(cp ${CP_VALA_C})
target_link_libraries(cp ${GLIB2_LIBRARIES} gpcore)

vala_precompile(GUICP_VALA_C
  guicp.vala
PACKAGES
  gio-2.0
  glib-2.0
  gtk+-3.0
  gp-core-2.0
OPTIONS
  --thread)

add_executable(guicp ${GUICP_VALA_C})
target_link_libraries(guicp ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES} gpcore)

vala_precompile(DRAWBACKGROUND_VALA_C
  draw_background_test.vala
PACKAGES
  gtk+-3.0
  gp-core-2.0
  gp-core-gui-2.0
OPTIONS
  --thread)

add_executable(draw_background_test ${DRAWBACKGROUND_VALA_C})
target_link_libraries(draw_background_test ${GTK3_LIBRARIES} gpcore gpcoregui)

vala_precompile(RMRF_VALA_C
  rmrf.vala
PACKAGES
  gio-2.0
  glib-2.0
  gp-core-2.0
OPTIONS
  --thread)

add_executable(filenames_sort filenames_sort.c)
target_link_libraries(filenames_sort ${GLIB2_LIBRARIES} gpcore)
add_test(NAME filenames_sort COMMAND filenames_sort)

add_executable(rmrf ${RMRF_VALA_C})
target_link_libraries(rmrf ${GLIB2_LIBRARIES} gpcore)

if(UNIX)
  add_executable(rumorista rumorista_test.c)
  target_link_libraries(rumorista ${GLIB2_LIBRARIES} ${GTK3_LIBRARIES} gpcore  ${PULSE_SIMPLE_LIBRARIES})
endif(UNIX)

add_executable(screen_doctor screen_doctor.c)
target_link_libraries(screen_doctor ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES} gpcore gpcoregui)

add_executable(spinner spinner.c)
target_link_libraries(spinner ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES})
set_property(TARGET spinner PROPERTY COMPILE_DEFINITIONS PACKAGE="${PROJECT_NAME}" G_LOG_DOMAIN="${PROJECT_NAME}")

add_executable(trunk_test trunk_test.c)
target_link_libraries(trunk_test ${GLIB2_LIBRARIES} gpcore)
add_test(NAME trunk_test COMMAND trunk_test)

add_executable(palette_test palette_test.c)
target_link_libraries(palette_test ${GTK3_LIBRARIES} ${GLIB2_LIBRARIES} gpcore gpcoregui)

add_executable(window_present_test window_present_test.c)
target_link_libraries(window_present_test ${GLIB2_LIBRARIES} ${GTK3_LIBRARIES})
add_test(NAME window_present_test COMMAND window_present_test)

