/**
 * @file test_server.cpp
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Create a and run a SocketServer object.
 */

#include "socket_server.h"

#include <thread>

int main(int argc, char** argv) {
    // Init buffers
    char recv_buffer[DEFAULT_MAX_CONNECTIONS * DEFAULT_RECV_BUFFER_LEN];
    char send_buffer[DEFAULT_MAX_CONNECTIONS * DEFAULT_SEND_BUFFER_LEN];

    // Create server
    SocketServer server = SocketServer(DEFAULT_MAX_CONNECTIONS, (char*)recv_buffer, DEFAULT_RECV_BUFFER_LEN, (char*)send_buffer, DEFAULT_SEND_BUFFER_LEN, 3005, &default_process_req_handler);
    
    bool setup_status = server.skt__socket_setup();

    if(setup_status == true) {
        server.skt__set_active(true);

        std::thread th_server_cli(&SocketServer::th_cl_capt_user_input, &server);

        server.skt__run_instances();

        th_server_cli.join();
    }

    return 0;
}