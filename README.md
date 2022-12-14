# SimpleSocket

A TCP socket client and server implementation written in C++.  This package is being developed on Ubunth 20.04 and is not guaranteed to work on other OS's.

## Client
- API to read and send data
- Defined sizes for receive send buffers
- Can manually close connection with server after receiving "STOP"
- Currently takes in data from the command line

## Server
- Continuously polls for connections
- Supports simultaneous live connections (defined maximum)
    - When a connection becomes supported by a server instance, sends an ACK to client
    - Each live connection has its own send and receive buffers
- Can manually close all connections and halt by getting "STOP"

## Resources
https://www.geeksforgeeks.org/socket-programming-cc/
