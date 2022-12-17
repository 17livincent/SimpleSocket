/**
 * @file test_client.cpp
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Create and run a SocketClient object.
 */

#include "socket_client.h"

#include <iostream>

int main(int argc, char** argv) {
    // Init buffers
    char recv_buffer[DEFAULT_RECV_BUFFER_LEN];
    char send_buffer[DEFAULT_SEND_BUFFER_LEN];

    // Create client
    SocketClient client = SocketClient(recv_buffer, DEFAULT_RECV_BUFFER_LEN, send_buffer, DEFAULT_SEND_BUFFER_LEN);

    bool setup_status = client.skt__setup();

    if(setup_status == true) {
        // Connect to server
        bool connect_status = client.skt__connect();
        std::cout << "CLIENT ESTABLISHED CONNECTION " << connect_status << std::endl;

        if(connect_status) {
            client.skt__set_active(true);

            std::thread th_client_cli(&SocketClient::th_cl_capt_user_input, &client);

            client.skt__run();

            th_client_cli.join();
        }
    }

    std::cout << "DONE " << client.skt__get_active() << std::endl;

    return 0;
}