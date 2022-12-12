/**
 * @file common.h
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Shared code
 */

#ifndef COMMON
#define COMMON

#include <netinet/in.h>
#include <mutex>

// Default socket params
#define DOMAIN AF_INET
#define COMM_TYPE SOCK_STREAM
#define PORT 3005

// Default buffer lengths
#define DEFAULT_RECV_BUFFER_LEN 4096
#define DEFAULT_SEND_BUFFER_LEN 4096

class SocketUser {
    public:
        // Socket file descriptor
        int socket_fd;
        // SocketServer address
        struct sockaddr_in server_address;

        // Mutex for the socket itself
        std::mutex socket_mutex;

        int32_t recv_buffer_max_len = 0;
        int32_t send_buffer_max_len = 0;

        /**
         * @brief Print contents of recv_buffer
         * 
         */
        void print_buffer(char* buffer, int32_t len);
};

#endif  // COMMON