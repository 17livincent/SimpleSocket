cmake_minimum_required(VERSION 3.22)

add_library(common socket_common.cpp)

target_include_directories(common PUBLIC ./)