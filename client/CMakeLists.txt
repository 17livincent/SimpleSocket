cmake_minimum_required(VERSION 3.22)

add_library(client socket_client.cpp)

target_link_libraries(client common)

target_include_directories(client PUBLIC ./)