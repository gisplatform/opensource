
add_executable( gtrace-test gtrace-test.c )
target_link_libraries( gtrace-test ${GLIB2_LIBRARIES} gptrace )
