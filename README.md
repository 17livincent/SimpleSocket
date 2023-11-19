# SimpleSocket

This is a multithreaded, TCP socket client and server implementation written in C++.  This package is being developed on Ubunth 22.04 and is not guaranteed to work on other OS's.  Ports to other platforms will at the minimum, require switching the socket and thread libraries and functions.

## How to
### Build and Run
Run ```./pre_build.sh``` to set up the CMake build.  Step into the newly-created ```build/``` and run ```make``` to compile everything.  The resulting executables are ```test_server``` and ```test_client```.  These are working implementations of the server and client.  The client sends data from the command line to the server, and the server echos.

### Customize
Sources files ```test_server.cpp``` and ```test_client.cpp``` show a good starting point for how the server and client are initialized and run.

The ```SocketServer``` constructor requires parameters to define the maximum number of active connections AKA server instance threads, along with send and receive buffer lengths.  The buffer lengths can also be set for the ```SocketClient```.

The ```SocketServer```'s ```process_req_handler``` or ```default_process_req_handler()``` is the function used to handle specific message types from the ```SocketClient```.  They are of type ```enum message_type_t``` in ```socket_messages.h```.  These messages types are expected to be the first byte in a written recv_buffer.

Additionally, the ```SERVER_IP``` can be changed in ```socket_common.h```.

## Overall Features

### Client
- API to read and send data
- Defined sizes for receive and send buffers
- Can manually close connection with server after receiving "STOP"
- Can take in data from the command line
- Threads to receive and send data

### Server
- Continuously polls for connections
- Supports simultaneous live connections (defined maximum)
    - One thread per server instance
    - When a connection becomes supported by a server instance, sends an ACK to client
    - Each live connection has its own send and receive buffers
- Can manually close all connections and halt by getting "STOP"
- CLI to control the server

## Development Resources
https://www.geeksforgeeks.org/socket-programming-cc/
https://www.geeksforgeeks.org/multithreading-in-cpp/?ref=lbp
