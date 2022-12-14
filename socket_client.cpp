/**
 * @file client.cpp
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief
 */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>

#include "socket_client.h"
#include "socket_common.h"
#include "socket_messages.h"

std::string hello = "CLIENT SAYS HELLO!";

SocketClient::SocketClient(char* recv_buffer, char* send_buffer) {
    this->recv_buffer = recv_buffer;
    this->send_buffer = send_buffer;
    this->recv_buffer_max_len = DEFAULT_RECV_BUFFER_LEN;
    this->send_buffer_max_len = DEFAULT_SEND_BUFFER_LEN;

}

SocketClient::SocketClient(char* recv_buffer, int32_t recv_buffer_max_len, char* send_buffer, int32_t send_buffer_max_len) {
    this->recv_buffer = recv_buffer;
    this->send_buffer = send_buffer;
    this->recv_buffer_max_len = recv_buffer_max_len;
    this->send_buffer_max_len = send_buffer_max_len;
}

bool SocketClient::skt__setup() {
    bool status = true;
    std::cout << "CLIENT setup" << std::endl;

    // Create socket
    this->socket_fd = ::socket(DOMAIN, COMM_TYPE, 0);
    std::cout << "CLIENT FD: " << this->socket_fd << std::endl;
    if(this->socket_fd == -1) {
        std::cout << "CREATE SOCKET FAILED" << std::endl;
        status = false;
    }
    else {
        std::cout << "CREATE SOCKET DONE" << std::endl;
    }

    // Set socket timeout
    struct timeval to_time;
    to_time.tv_sec = 0;
    to_time.tv_usec = 1 * 1000000; // 1s
    setsockopt(this->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&to_time, sizeof(timeval));

    // Set destination address
    this->server_address.sin_family = AF_INET;
    this->server_address.sin_port = htons(PORT);
    if(inet_pton(AF_INET, SERVER_IP.c_str(), &this->server_address.sin_addr) != 1) {
        std::cout << "ADDRESS CONVERSION FAILED" << std::endl;
        status = false;
    }
    else {
        std::cout << "ADDRESS CONVERSION DONE" << std::endl;
    }

    return status;
}

bool SocketClient::skt__connect() {
    int connect_status = connect(this->socket_fd, (struct sockaddr*)&this->server_address, ADDRESS_SIZE);

    return (connect_status == 0);
}

void SocketClient::skt__set_active(bool active) {
    this->active = active;
}

bool SocketClient::skt__get_active() {
    return this->active;
}

bool SocketClient::skt__run() {
    bool status = false;

    if(this->active) {
        std::cout << "ACTIVE" << std::endl;
        this->th_recv = std::thread(&SocketClient::th_receive_data, this);
        this->th_send = std::thread(&SocketClient::th_send_data, this);
        this->th_recv.join();
        this->th_send.join();

        status = true;
    }

    socket_close();

    return status;
}

void SocketClient::socket_close() {
    close(this->socket_fd);
}

void SocketClient::skt__send_data(const char* data_buffer, uint32_t datalen) {
    this->skt__lock_send_buffer();

    memcpy((void*)this->send_buffer, (void*)data_buffer, datalen);
    this->send_buffer_len = this->input_buffer_len;

    this->skt__unlock_send_buffer();
}

int32_t SocketClient::skt__read_data(const char* data_buffer) {
    int32_t read_len = this->recv_buffer_len;

    if(data_buffer && read_len) {
        skt__lock_recv_buffer();

        memcpy((void*)data_buffer, (void*)this->recv_buffer, this->recv_buffer_len);
        this->recv_buffer_len = 0;
    
        skt__unlock_recv_buffer();
    }

    return read_len;
}

void SocketClient::th_cl_capt_user_input() {
    while(this->active) {
        skt__cl_capt_user_input();
    }
    std::cout << "EXITED th_cl_capt_user_input" << std::endl;
}

bool SocketClient::skt__cl_capt_user_input() {
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

bool SocketClient::process_user_input() {
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
            if(connection_ackd) {
                // Fill send buffer
                skt__send_data((char*)&msg_stop, MSG_TYPE_LEN);
            }
            else {
                this->skt__set_active(false);
                ::shutdown(this->socket_fd, SHUT_RD);
            }
        }
        else {
            skt__send_data(this->input_buffer, this->input_buffer_len);
        }
    }

    return input_ok;
}

void SocketClient::send_data() {
    if(this->active) {
        if(this->send_buffer_len > 0) {
            int size_sent = send(this->socket_fd, this->send_buffer, this->send_buffer_len, 0);

            std::cout << "CLIENT SENT " << size_sent << std::endl;

            // Clear send flag
            this->send_buffer_len = 0;
        }
    }
}

void SocketClient::th_send_data() {
    while(this->active) {
        // Send data only if the connection is acknowledged and there is something to send
        this->skt__lock_send_buffer();

        if(this->send_buffer_len > 0) {//if(this->skt__cl_capt_user_input()) {
            if(this->connection_ackd) {
                this->send_data();
            } 
        }

        this->skt__unlock_send_buffer();
    }
    std::cout << "EXITED th_send_data" << std::endl;
}

void SocketClient::th_receive_data() {
    std::cout << "CLIENT WAITING FOR ACK" << std::endl;
    while(this->active) {
        this->skt__lock_recv_buffer();

        this->recv_buffer_len = read(this->socket_fd, this->recv_buffer, this->recv_buffer_max_len);
        // Possible that recv_buffer has multiple messages sent from server

        if(this->recv_buffer_len > 0) {
            std::cout << "CLIENT RECEIVED: " << this->recv_buffer_len << " ";
            print_buffer(this->recv_buffer, this->recv_buffer_len);
            std::cout << std::endl;

            // Valid recv_buffer data here...

            // Receive ACK
            if(this->recv_buffer[0] == (char)MSG_ACK) {
                std:: cout << "CLIENT RECEIVED ACK" << std::endl;
                connection_ackd = true;
            }
        }
        else if(this->recv_buffer_len == 0) {
            // When connection closed by server
            std::cout << "CONNECTION CLOSED" << std::endl;
            this->skt__set_active(false);
            connection_ackd = false;
        }
        else {
            // Nothing read
        }

        this->skt__unlock_recv_buffer();
    }
    std::cout << "EXITED th_receive_data" << std::endl;
}

void SocketClient::skt__lock_recv_buffer() {
    recv_buffer_mutex.lock();
}

void SocketClient::skt__unlock_recv_buffer() {
    recv_buffer_mutex.unlock();
}

void SocketClient::skt__lock_send_buffer() {
    send_buffer_mutex.lock();
}

void SocketClient::skt__unlock_send_buffer() {
    send_buffer_mutex.unlock();
}
