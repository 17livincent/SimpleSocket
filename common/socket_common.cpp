/**
 * @file socket_common.cpp
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief
 */

#include <iostream>
#include <cstring>

#include "socket_common.h"

void SocketUser::th_cl_capt_user_input() {
    while(this->active) {
        skt__cl_capt_user_input();
    }
    std::cout << "EXITED th_cl_capt_user_input" << std::endl;
}

bool SocketUser::skt__cl_capt_user_input() {
    bool input_ok = false;

    input_buffer_mutex.lock();

    this->input_buffer_len = 0;
    memset(this->input_buffer, 0x0, INPUT_BUFFER_LEN);

    std::cin.getline(this->input_buffer, INPUT_BUFFER_LEN);
    std::cout << "GOT INPUT: " << this->input_buffer << std::endl;

    input_ok = process_user_input();

    input_buffer_mutex.unlock();

    return input_ok;
}

void print_buffer(char* buffer, int32_t len) {
    for(int32_t i = 0; i < len; i++) {
        std::cout << buffer[i];
    }
}