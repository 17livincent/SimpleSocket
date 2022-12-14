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

SocketServer::SocketServer(uint8_t max_connections, char* recv_buffer, int32_t recv_buffer_max_len, char* send_buffer, int32_t send_buffer_max_len) {
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

    // Create thread instance array
    this->server_instances = new std::thread[max_connections];
}

SocketServer::~SocketServer() {
    delete this->instance_recv_buffer_len;
    delete this->instance_send_buffer_len;

    delete instance_fds;

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

        // Wait until a new connection is accepted
//        socket_mutex.lock();
        int new_socket = accept(this->socket_fd, (struct sockaddr*)&this->server_address, (socklen_t*)&this->addrlen);
//        socket_mutex.unlock();

        if(new_socket != -1) {
            this->instance_fds[instance_id] = new_socket;
            // Send ACK to client
//            socket_mutex.lock();
            int send_size = send(new_socket, (void*)&msg_ack, MSG_TYPE_LEN, 0);
//            socket_mutex.unlock();

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
    char* instance_send_buffer = &instance_send_buffers[instance_id * recv_buffer_max_len];

    bool instance_running = true;

    while(this->active && instance_running) {
        // After connection, send and read

//        socket_mutex.lock();
        instance_recv_buffer_len[instance_id] = ::read(socket, (void*)instance_recv_buffer, recv_buffer_max_len);
//        socket_mutex.unlock();

        std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED: " << instance_recv_buffer_len[instance_id] << " ";
        print_buffer(instance_recv_buffer, instance_recv_buffer_len[instance_id]);
        std::cout << std::endl;

        if(instance_recv_buffer_len[instance_id] > 0) {
            // Process request...

            switch((uint8_t)instance_recv_buffer[0]) {
                case ((uint8_t)MSG_STOP):
                    // Recieved STOP message
                    std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED STOP" << std::endl;
                    instance_running = false;
                    break;
                default:
                    break;
            }

            // Send back
            instance_send_buffer_len[instance_id] = instance_recv_buffer_len[instance_id];
            memcpy((void*)instance_send_buffer, (void*)instance_recv_buffer, instance_recv_buffer_len[instance_id]);

//            socket_mutex.lock();
            send(socket, (void*)instance_send_buffer, instance_send_buffer_len[instance_id], 0);
//            socket_mutex.unlock();
            
            std::cout << "INSTANCE #" << (int)instance_id <<" ECHOED" << std::endl;
        }
        else {
            std::cout << "INSTANCE #" << (int)instance_id << " CONNECTION CLOSED" << std::endl;
            instance_running = false;
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
//    socket_mutex.lock();
    close(curr_socket);
//    socket_mutex.unlock();
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