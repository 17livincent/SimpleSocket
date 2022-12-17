cmake_minimum_required(VERSION 3.22)

add_library(server socket_server.cpp)

target_link_libraries(server common)

target_include_directories(server PUBLIC ./)