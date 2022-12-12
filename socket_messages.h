/**
 * @file socket_messages.h
 * @author Vincent Li (li.vincent0gmail.com)
 * @brief Definitions of socket message types and formats.
 * 
 */

#ifndef SOCKET_MESSAGES
#define SOCKET_MESSAGES

typedef enum {
    MSG_INVALID = 0xA0,
    MSG_STOP = 0xA1,
    MSG_ACK = 0xA2,
} message_type_t;

const uint8_t MSG_TYPE_LEN = sizeof(uint8_t);

const message_type_t msg_invalid = MSG_INVALID;
const message_type_t msg_stop = MSG_STOP;
const message_type_t msg_ack = MSG_ACK;

#endif  // SOCKET_MESSAGES