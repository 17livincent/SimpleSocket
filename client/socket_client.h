/**
 * @file client.h
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Socket client
 */

#ifndef SOCKET_CLIENT
#define SOCKET_CLIENT

#include <sys/socket.h>
#include <mutex>
#include <thread>

#include "socket_common.h"

class SocketClient : public SocketUser {
    public:
        static const int ADDRESS_SIZE = sizeof(struct sockaddr);

        // Receive buffer shared with caller
        std::mutex recv_buffer_mutex;
        char* recv_buffer;
        int32_t recv_buffer_len = 0;

        // Send buffer shared with caller
        std::mutex send_buffer_mutex;
        char* send_buffer;
        int32_t send_buffer_len = 0;

        // If the connection has been acknowledged by the server
        // Can send data only when this is true
        bool connection_ackd = false;

        /**
         * @brief Construct a new Socket Client object
         * Buffers have default lengths.
         * 
         * @param recv_buffer pointer to buffer for received data
         * @param send_buffer pointer to buffer to send data
         * @param port server port
         */
        SocketClient(char* recv_buffer, char* send_buffer, uint16_t port);

        /**
         * @brief Construct a new Socket Client object
         * 
         * @param recv_buffer receive buffer pointer
         * @param recv_buffer_max_len max length of receive buffer
         * @param send_buffer send buffer pointer
         * @param send_buffer_max_len max length of send buffer
         * @param port server port
         */
        SocketClient(char* recv_buffer, int32_t recv_buffer_max_len, char* send_buffer, int32_t send_buffer_max_len, uint16_t port);

        /**
         * @brief Construct a new Socket Client object
         * 
         * @param client 
         */
        SocketClient(const SocketClient &client);

        /**
         * @brief Create the socket.
         * 
         * @return true on success
         * @return false on failure
         */
        bool skt__setup();

        /**
         * @brief Connect this client to the server.
         * 
         * @return true on success
         * @return false on failure
         */
        bool skt__connect();

        /**
         * @brief PUBLIC: Set active flag to stop receiving and sending.
         * 
         * @param active 
         */
        void skt__set_active(bool active);

        /**
         * @brief PUBLIC: Return active status
         * 
         * @return true 
         * @return false 
         */
        bool skt__get_active();

        /**
         * @brief PUBLIC: Start the send and receive threads.
         * 
         * @return true Active and successful
         * @return false otherwise
         */
        bool skt__run();

        /**
         * @brief Close the client connection.
         * 
         */
        void socket_close();

        /**
         * @brief Start shutdown process.
         * 
         */
        void shutdown();

        /**
         * @brief Read the input buffer and do the requested action based on the contents.
         * 
         * @return true did something
         * @return false otherwise
         */
        bool process_user_input();

        /**
         * @brief PUBLIC: Populate the send_buffer to send data.
         * 
         * @param data_buffer Data to send
         * @param datalen Length of buffer
         */
        void skt__send_data(const char* data_buffer, uint32_t datalen);

        /**
         * @brief PUBLIC: Copy data from the client recv_buffer to the given buffer, if there is any.
         * The data could have multiple messages concatenated together.
         * 
         * @param data_buffer Buffer to copy read data to
         * @return int32_t read data length
         */
        int32_t skt__read_data(const char* data_buffer);

        /**
         * @brief Continuously poll and send user input.
         * 
         */
        void th_send_data();

        /**
         * @brief Continuously receive data.
         * 
         */
        void th_receive_data();

        /**
         * @brief PUBLIC: Acquire mutex for recv_buffer
         * 
         */
        void skt__lock_recv_buffer();

        /**
         * @brief PUBLIC: Release mutex for recv_buffer
         * 
         */
        void skt__unlock_recv_buffer();

        /**
         * @brief PUBLIC: Acquire mutex for send_buffer
         * 
         */
        void skt__lock_send_buffer();

        /**
         * @brief PUBLIC: Release mutex for send_buffer
         * 
         */
        void skt__unlock_send_buffer();

    private:
        // Receiving thread
        std::thread th_recv;
        // Sending thread
        std::thread th_send;

        /**
         * @brief Send data in send_buffer.
         * 
         */
        void send_data();

};

#endif  // SOCKET_CLIENT