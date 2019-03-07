
add_executable( manager-test manager-test.c )
add_executable( data-test data-test.c )
add_executable( auth-test auth-test.c )
add_executable( grpc-test grpc-test.c )

add_test(NAME gprpc-manager-test COMMAND manager-test)
add_test(NAME gprpc-data-test COMMAND data-test)

add_test(NAME gprpc-test-shm COMMAND grpc-test "shm://gpt-rpc-test-shm")
add_test(NAME gprpc-test-tcp COMMAND grpc-test "tcp://localhost:60123")
add_test(NAME gprpc-test-udp COMMAND grpc-test "shm://localhost:60123")

add_test(NAME unknown-test-shm COMMAND grpc-test -nr 1 "shm://gpt-rpc-test-shm")
add_test(NAME unknown-test-tcp COMMAND grpc-test -nr 1 "tcp://localhost:60123")
add_test(NAME unknown-test-udp COMMAND grpc-test -nr 1 "shm://localhost:60123")

target_link_libraries( manager-test ${GRPC_GLIB2_LIBRARIES} gprpc )
target_link_libraries( data-test ${GRPC_GLIB2_LIBRARIES} gprpc )
target_link_libraries( auth-test ${GRPC_GLIB2_LIBRARIES} gprpc )
target_link_libraries( grpc-test ${GRPC_GLIB2_LIBRARIES} gprpc )
