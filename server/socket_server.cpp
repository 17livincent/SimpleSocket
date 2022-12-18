/**
 * @file server.cpp
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>

#include "socket_server.h"
#include "socket_common.h"
#include "socket_messages.h"

SocketServer::SocketServer(uint8_t max_connections, char* recv_buffer, int32_t recv_buffer_max_len, char* send_buffer, int32_t send_buffer_max_len) 
    : SocketServer(max_connections, recv_buffer, recv_buffer_max_len, send_buffer, send_buffer_max_len, &default_process_req_handler) {
}

SocketServer::SocketServer(uint8_t max_connections, char* recv_buffer, int32_t recv_buffer_max_len, char* send_buffer, int32_t send_buffer_max_len, void (*process_req_handler)(const SocketServer*, uint8_t, int)) {
    this->max_connections = max_connections;
    this->recv_buffer_max_len = recv_buffer_max_len;
    this->instance_recv_buffers = recv_buffer;
    this->send_buffer_max_len = send_buffer_max_len;
    this->instance_send_buffers = send_buffer;

    // Create buffer size arrays
    this->instance_recv_buffer_len = new int32_t[max_connections];
    this->instance_send_buffer_len = new int32_t[max_connections];

    // Create instance FD array
    this->instance_fds = new int[max_connections];

    // Create instance_running array
    this->instance_running = new bool[max_connections];

    // Create thread instance array
    this->server_instances = new std::thread[max_connections];

    // Set process request handler
    this->process_req_handler = process_req_handler;
}

SocketServer::~SocketServer() {
    delete this->instance_recv_buffer_len;
    delete this->instance_send_buffer_len;

    delete instance_fds;
    delete instance_running;

    delete[] this->server_instances;
}

bool SocketServer::skt__socket_setup() {
    bool status = true;
    std::cout << "SERVER setup" << std::endl;

    // 1. Create socket
    this->socket_fd = ::socket(DOMAIN, COMM_TYPE, 0);
    std::cout << "SERVER FD: " << this->socket_fd << std::endl;
    if(this->socket_fd == -1) {
        std::cout << "CREATE SOCKET FAILED" << std::endl;
        status = false;
    }
    else {
        std::cout << "CREATE SOCKET DONE" << std::endl;
    }

    // 2. Setsockopt
    if(setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &this->opt, OPT_SIZE) != 0) {
        std::cout << "SETSOCKOPT FAILED" << std::endl;
        status = false;
    }
    else {
        std::cout << "SETSOCKOPT DONE" << std::endl;
    }

    // Socket receive timeout
    this->to_time.tv_sec = 0;
    this->to_time.tv_usec = 1 * 1000000; // 1s

    // 3. Bind
    this->server_address.sin_family = AF_INET;
    this->server_address.sin_addr.s_addr = INADDR_ANY;
    this->server_address.sin_port = htons(PORT);
    if(bind(this->socket_fd, (struct sockaddr*)&this->server_address, ADDRESS_SIZE) != 0) {
        std::cout << "BIND FAILED" << std::endl;
        status = false;
    }
    else {
        std::cout << "BIND DONE" << std::endl;
    }

    // 4. Listen
    if(listen(this->socket_fd, 0) != 0) {
        std::cout << "LISTEN FAILED" << std::endl;
        status = false;
    }
    else {
        std::cout << "LISTEN DONE" << std::endl;
    }

    return status;
}

void SocketServer::th_server_instance(const uint8_t instance_id) {
    while(this->active) {
        std::cout << "INSTANCE #" << (int)instance_id << " READY FOR CONNECTION" << std::endl;

        this->instance_fds[instance_id] = -1;
        instance_running[instance_id] = false;

        // Wait until a new connection is accepted
        int new_socket = accept(this->socket_fd, (struct sockaddr*)&this->server_address, (socklen_t*)&ADDRESS_SIZE);

        if(new_socket != -1) {
            this->instance_fds[instance_id] = new_socket;
            instance_running[instance_id] = true;

            // Send ACK to client
            int send_size = send(new_socket, (void*)&msg_ack, MSG_TYPE_LEN, 0);

            std::cout << "INSTANCE #" << (int)instance_id <<" sent ACK " << send_size << std::endl;
            server_session(instance_id, new_socket);

            socket_close(new_socket);
        }
    }
    std::cout << "EXITED th_server_instance" << std::endl;
}

void SocketServer::server_session(const uint8_t instance_id, const int socket) {
    std::cout << "INSTANCE #" << (int)instance_id << " CONNECTED WITH CLIENT " << socket << std::endl;

    char* instance_recv_buffer = &instance_recv_buffers[instance_id * recv_buffer_max_len];

    while(this->active && instance_running[instance_id]) {
        // After connection, send and read

        instance_recv_buffer_len[instance_id] = ::read(socket, (void*)instance_recv_buffer, recv_buffer_max_len);

        std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED: " << instance_recv_buffer_len[instance_id] << " ";
        print_buffer(instance_recv_buffer, instance_recv_buffer_len[instance_id]);
        std::cout << std::endl;

        if(instance_recv_buffer_len[instance_id] > 0) {
            // Process request...
            process_req_handler(this, instance_id, socket);
        }
        else {
            std::cout << "INSTANCE #" << (int)instance_id << " CONNECTION CLOSED" << std::endl;
            instance_running[instance_id] = false;
        }
    }
}

void SocketServer::th_cl_capt_user_input() {
    while(this->active) {
        skt__cl_capt_user_input();
    }
    std::cout << "EXITED th_cl_capt_user_input" << std::endl;
}

bool SocketServer::skt__cl_capt_user_input() {
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

bool SocketServer::process_user_input() {
    bool input_ok = false;

    // Find '\0'
    bool found = false;
    uint32_t i = 0;

    while(!found && (i < INPUT_BUFFER_LEN)) {
        if(this->input_buffer[i] == '\0') {
            this->input_buffer_len = i + 1;
            found = true;
        }
        else {
            i++;
        }
    }

    if(!found || (i == 0)) {
        std::cout << "INVALID INPUT" << std::endl;
    }
    else {
        input_ok = true;

        // Process and send known message types
        std::string input = static_cast<std::string>(this->input_buffer);
        if(input == STOP_CMD) {
            std::cout << "RECEIVED STOP" << std::endl;
            this->shutdown();
        }
    }

    return input_ok;
}

void SocketServer::socket_close(int curr_socket) {
    close(curr_socket);
}

void SocketServer::skt__set_active(bool active) {
    this->active = active;
}

void SocketServer::skt__run_instances() {
    if(this->active) {
        // Activate threads
        for(uint8_t instance_id = 0; instance_id < this->max_connections; instance_id++) {
            this->server_instances[instance_id] = std::thread(&SocketServer::th_server_instance, this, instance_id);
        }

        // Join all
        for(uint8_t i = 0; i < this->max_connections; i++) {
            this->server_instances[i].join();
        }
    }
}

void SocketServer::shutdown() {
    // Shutdown all sockets
    ::shutdown(this->socket_fd, SHUT_RD);
    for(int i = 0; i < this->max_connections; i++) {
        ::shutdown(this->instance_fds[i], SHUT_RD);
    }
    
    this->skt__set_active(false);
}

void default_process_req_handler(const SocketServer* server, const uint8_t instance_id, const int socket) {
    char* instance_recv_buffer = &server->instance_recv_buffers[instance_id * server->recv_buffer_max_len];
    char* instance_send_buffer = &server->instance_send_buffers[instance_id * server->recv_buffer_max_len];

    switch((uint8_t)instance_recv_buffer[0]) {
        case ((uint8_t)MSG_STOP):
            // Recieved STOP message
            std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED STOP" << std::endl;
            server->instance_running[instance_id] = false;
            break;
        default:
            break;
    }

    // Send back
    server->instance_send_buffer_len[instance_id] = server->instance_recv_buffer_len[instance_id];
    memcpy((void*)instance_send_buffer, (void*)instance_recv_buffer, server->instance_recv_buffer_len[instance_id]);

    send(socket, (void*)instance_send_buffer, server->instance_send_buffer_len[instance_id], 0);
            
    std::cout << "INSTANCE #" << (int)instance_id <<" ECHOED" << std::endl;
}