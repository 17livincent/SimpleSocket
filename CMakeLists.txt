cmake_minimum_required(VERSION 3.22)

project(SimpleSocket VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(test)