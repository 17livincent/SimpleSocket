/**
 * @file client.h
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Socket client
 */

#ifndef SOCKET_CLIENT
#define SOCKET_CLIENT

#include <sys/socket.h>
#include <string>
#include <mutex>
#include <thread>

#include "socket_common.h"

const uint32_t INPUT_BUFFER_LEN = 4096;
const int ADDRESS_SIZE = sizeof(struct sockaddr);

// SocketServer IP address
const std::string SERVER_IP = "127.0.0.1";

const std::string STOP_CMD = "STOP";

class SocketClient : public SocketUser {
    public:
        // User input buffer
        std::mutex input_buffer_mutex;
        char input_buffer[INPUT_BUFFER_LEN] = {0};
        uint32_t input_buffer_len = 0;

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
         */
        SocketClient(char* recv_buffer, char* send_buffer);

        /**
         * @brief Construct a new Socket Client object
         * 
         * @param recv_buffer receive buffer pointer
         * @param recv_buffer_max_len max length of receive buffer
         * @param send_buffer send buffer pointer
         * @param send_buffer_max_len max length of send buffer
         */
        SocketClient(char* recv_buffer, int32_t recv_buffer_max_len, char* send_buffer, int32_t send_buffer_max_len);

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
         * @brief PUBLIC: Wait for and capture user input from the command line.
         * 
         */
        bool skt__cl_capt_user_input();

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
        // SocketClient status flag
        volatile bool active = false;

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