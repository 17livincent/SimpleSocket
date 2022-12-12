/**
 * @file socket_common.cpp
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief
 */

#include <iostream>
#include <cstring>

#include "socket_common.h"

void SocketUser::print_buffer(char* buffer, int32_t len) {
    for(int32_t i = 0; i < len; i++) {
        std::cout << buffer[i];
    }
}