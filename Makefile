#!/bin/bash

CXX=g++
CXXFLAGS=-Wall -g -pthread

TARGETS=socket_server socket_client

all: $(TARGETS)

socket_server: socket_server.o socket_common.o
	$(CXX) $(CXXFLAGS) -o socket_server socket_server.o socket_common.o

socket_client: socket_client.o socket_common.o
	$(CXX) $(CXXFLAGS) -o socket_client socket_client.o socket_common.o

socket_server.o: socket_server.cpp socket_server.h
	$(CXX) $(CXXFLAGS) -c socket_server.cpp

socket_client.o: socket_client.cpp socket_client.h
	$(CXX) $(CXXFLAGS) -c socket_client.cpp

socket_common.o: socket_common.cpp socket_common.h
	$(CXX) $(CXXFLAGS) -c socket_common.cpp

clean:
	rm -r $(TARGETS) *.o *.exe