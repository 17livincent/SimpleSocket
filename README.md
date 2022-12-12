# SimpleSocket

A TCP socket client and server implementation written in C++.

## Client
- API to read and send data
- Defined sizes for receive send buffers
- Can manually close connection by sending STOP to server
- Currently takes in data from the command line

## Server
- Continuously polls for connections
- Supports simultaneous live connections (defined maximum)
    - When a connection becomes supported by a server instance, sends an ACK to client
    - Each live connection has its own send and receive buffers

## Resources
https://www.geeksforgeeks.org/socket-programming-cc/

TODOS:
- Give each server instance its own send and receive threads
    - Is this required?
    - Will server ever send data without the client asking for it?
    - Currently using TCP
- Create a test execuutable that uses server and clients