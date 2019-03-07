add_executable( cifrotest cifrotest.c dummyrenderer.c )
add_executable( cifroscopetest cifroscopetest.c )
add_executable( cifrocurvetest cifrocurvetest.c )

target_link_libraries( cifrotest ${GTK3_LIBRARIES} gpcifro m )
target_link_libraries( cifroscopetest ${GTK3_LIBRARIES} gpcifro m )
target_link_libraries( cifrocurvetest ${GTK3_LIBRARIES} gpcifro m )
