/**
 * @file server.h
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Socket server
 * 
 */

#ifndef SOCKET_SERVER
#define SOCKET_SERVER

#include <sys/socket.h>
#include <vector>

#include "socket_common.h"

// Max concurrent socket connections
const int DEFAULT_MAX_CONNECTIONS = 2;

const int ADDRESS_SIZE = sizeof(struct sockaddr_in);
const int OPT_SIZE = sizeof(int);

class SocketServer : public SocketUser {
    public:
        const int opt = 1;
        const int addrlen = sizeof(struct sockaddr_in);

        // SocketServer status flag
        volatile bool active = false;
        // Shutdown flag
        volatile bool shutdown = false;

        // Max live connections
        uint8_t max_connections = 0;

        // Socket receive timeout
        struct timeval to_time;

        // Array of all connections
        // Each entry is a thread running server_instance()
        std::thread* server_instances;

        // Send and receive buffers for each server instance
        char* instance_recv_buffers;
        char* instance_send_buffers;
        int32_t* instance_recv_buffer_len;
        int32_t* instance_send_buffer_len;

        /**
         * @brief Construct a new Socket Server object
         * 
         * @param max_connections number of max connections
         * @param recv_buffer receive buffer pointer
         * @param recv_buffer_max_len max length of receive buffer
         * @param send_buffer send buffer pointer
         * @param send_buffer_max_len max length of send buffer
         */
        SocketServer(uint8_t max_connections, char* recv_buffer, int32_t recv_buffer_max_len, char* send_buffer, int32_t send_buffer_max_len);

        /**
         * @brief Destroy the Socket Server object
         * 
         */
        ~SocketServer();

        /**
         * @brief PUBLIC: Create, set, and bind socket.
         * 
         * @return true on success
         * @return false on failure
         */
        bool skt__socket_setup();

        /**
         * @brief Check for and accept a new connection,
         * and then run the server session until it ends.
         * Afterward, poll for a new connection.
         * 
         * @param instance_id an numerical ID of the server instance
         * 
         */
        void server_instance(const uint8_t instance_id);

        /**
         * @brief Serve a client until the connection closes.
         * Each client connection has its own such thread.
         * 
         * @param instance_id of this server instance
         * @param socket of this server instance
         */
        void server_session(const uint8_t instance_id, const int socket);

        /**
         * @brief Close the current connection.
         * 
         * @param curr_socket the descriptor value returned by accept()
         */
        void socket_close(int curr_socket);

        /**
         * @brief PUBLIC: Set active flag to stop receiving and sending.
         * 
         * @param active 
         */
        void skt__set_active(bool active);

        /**
         * @brief Activate instance threads.
         * 
         */
        void skt__run_instances();

        /**
         * @brief Start shutdown process.
         * 
         */
        void set_shutdown();
};

#endif  // SOCKET_SERVER