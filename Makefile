#!/bin/bash

CXX=g++
CXXFLAGS=-Wall -g -pthread

TARGETS=run_server run_client

all: $(TARGETS)

run_server: test_server.o socket_server.o socket_common.o
	$(CXX) $(CXXFLAGS) -o run_server test_server.o socket_server.o socket_common.o

run_client: test_client.o socket_client.o socket_common.o
	$(CXX) $(CXXFLAGS) -o run_client test_client.o socket_client.o socket_common.o

socket_server: socket_server.o socket_common.o
	$(CXX) $(CXXFLAGS) -o socket_server socket_server.o socket_common.o

socket_client: socket_client.o socket_common.o
	$(CXX) $(CXXFLAGS) -o socket_client socket_client.o socket_common.o

test_server.o: test_server.cpp socket_server.h socket_common.h
	$(CXX) $(CXXFLAGS) -c test_server.cpp

test_client.o: test_client.cpp socket_client.h socket_common.h
	$(CXX) $(CXXFLAGS) -c test_client.cpp

socket_server.o: socket_server.cpp socket_server.h socket_messages.h
	$(CXX) $(CXXFLAGS) -c socket_server.cpp

socket_client.o: socket_client.cpp socket_client.h socket_messages.h
	$(CXX) $(CXXFLAGS) -c socket_client.cpp

socket_common.o: socket_common.cpp socket_common.h
	$(CXX) $(CXXFLAGS) -c socket_common.cpp

clean:
	rm -r $(TARGETS) *.o *.exe