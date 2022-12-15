/**
 * @file common.h
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Shared code
 */

#ifndef COMMON
#define COMMON

#include <netinet/in.h>
#include <mutex>
#include <string>

// Default socket params (TCP on port 3005)
#define DOMAIN AF_INET
#define COMM_TYPE SOCK_STREAM
#define PORT 3005

// SocketServer IP address
const std::string SERVER_IP = "127.0.0.1";

// Default buffer lengths
#define DEFAULT_RECV_BUFFER_LEN 4096
#define DEFAULT_SEND_BUFFER_LEN 4096
#define INPUT_BUFFER_LEN 1024

// CLI commands
const std::string STOP_CMD = "STOP";

class SocketUser {
    public:
        // Socket file descriptor
        int socket_fd;
        // SocketServer address
        struct sockaddr_in server_address;

        // Mutex for the socket itself
        std::mutex socket_mutex;

        // User input buffer to control client/server
        std::mutex input_buffer_mutex;
        char input_buffer[INPUT_BUFFER_LEN] = {0};
        uint32_t input_buffer_len = 0;

        int32_t recv_buffer_max_len = 0;
        int32_t send_buffer_max_len = 0;

        /**
         * @brief Print contents of recv_buffer
         * 
         */
        void print_buffer(char* buffer, int32_t len);
};

#endif  // COMMON