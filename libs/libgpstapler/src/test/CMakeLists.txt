add_library(duller duller.c )
add_library(muddy muddy.c)
add_library(weigher weigher.c)
add_executable(pixmantest pixmantest.c)
add_executable(staplertest staplertest.c)
add_executable(treeview_dnd_test treeview_dnd_test.c)
add_executable(treeview_dnd_test2 treeview_dnd_test2.c)
add_executable(viewertestduller viewertestduller.c)

target_link_libraries(pixmantest ${GTK3_LIBRARIES} pixman-1 m)
target_link_libraries(staplertest ${GTK3_LIBRARIES} duller muddy weigher gpstapler gpcore gpcoregui)
target_link_libraries(muddy ${GTK3_LIBRARIES} gpcore)
target_link_libraries(weigher gpcore)

#if( UNIX )
#  target_link_libraries(staplertest profiler)
#endif( UNIX )

target_link_libraries(treeview_dnd_test ${GTK3_LIBRARIES})
target_link_libraries(treeview_dnd_test2 ${GTK3_LIBRARIES})
target_link_libraries(viewertestduller ${GTK3_LIBRARIES} duller gpstapler)

