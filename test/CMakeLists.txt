cmake_minimum_required(VERSION 3.22)

add_executable(test_client test_client.cpp)
target_link_libraries(test_client client)

add_executable(test_server test_server.cpp)
target_link_libraries(test_server server)
