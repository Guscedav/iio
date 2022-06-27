/*
 * HokuyoUST10LX.cpp
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 27.07.2017
 *      Author: Marcel Honegger
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "HokuyoUST10LX.h"

using namespace std;

/**
 * Creates a Hokuyo UST-10LX device driver object, configures the TCP/IP communication
 * and starts the measurements with the laser scanner.
 * @param ipAddress the IP address of the laser scanner, i.e. "192.168.0.10".
 * @param portNumber the port number of the laser scanners TCP/IP server (must be 10940).
 */
HokuyoUST10LX::HokuyoUST10LX(string ipAddress, uint16_t portNumber) {
    
    // create TCP/IP socket
    
    clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1) {
        throw runtime_error("HokuyoUST10LX: couldn't create network socket.");
    }
    
    // configure receive timeout
    
    timeval tv;
    tv.tv_sec = RECEIVE_TIMEOUT/1000;
    tv.tv_usec = (RECEIVE_TIMEOUT%1000)*1000;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (void*)&tv, sizeof(tv)) < 0) {
        throw runtime_error("HokuyoUST10LX: couldn't configure receive timeout.");
    }
    
    // connect to server
    
    sockaddr_in serverSocket;
    memset((int8_t*)&serverSocket, 0, sizeof(serverSocket));
    serverSocket.sin_family = AF_INET;
    serverSocket.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    serverSocket.sin_port = htons(portNumber);
    if (connect(clientSocket, (sockaddr*)&serverSocket, sizeof(serverSocket)) < 0) {
        throw runtime_error("HokuyoUST10LX: failed to connect with server.");
    }
    
    // initialize laser scanner
    
    writeLine("BM");
    readLine();
    readLine();
    readLine();
}

/**
 * Creates a Hokuyo UST-10LX device driver object, configures the TCP/IP communication
 * socket and starts the measurements with the laser scanner.
 * @param interfaceAddress the local interface to bind the communication socket to.
 * @param ipAddress the IP address of the laser scanner, i.e. "192.168.0.10".
 * @param portNumber the port number of the laser scanners TCP/IP server (must be 10940).
 */
HokuyoUST10LX::HokuyoUST10LX(string interfaceAddress, string ipAddress, uint16_t portNumber) {
    
    // create TCP/IP socket
    
    clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1) {
        throw runtime_error("HokuyoUST10LX: couldn't create network socket.");
    }
    
    // configure receive timeout
    
    timeval tv;
    tv.tv_sec = RECEIVE_TIMEOUT/1000;
    tv.tv_usec = (RECEIVE_TIMEOUT%1000)*1000;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (void*)&tv, sizeof(tv)) < 0) {
        throw runtime_error("HokuyoUST10LX: couldn't configure receive timeout.");
    }
    
    // configure reuse of port
    
    int32_t buffer = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void*)&buffer, sizeof(buffer)) < 0) {
        throw runtime_error("HokuyoUST10LX: couldn't configure reuse of port.");
    }
    
    // bind local socket
    
    sockaddr_in localSocket;
    memset((int8_t*)&localSocket, 0, sizeof(localSocket));
    localSocket.sin_family = AF_INET;
    localSocket.sin_addr.s_addr = inet_addr(interfaceAddress.c_str());
    localSocket.sin_port = htons(portNumber);
    bind(clientSocket, (sockaddr*)&localSocket, sizeof(localSocket));
    //if (bind(clientSocket, (sockaddr*)&localSocket, sizeof(localSocket)) < 0) {
    //    throw runtime_error("HokuyoUST10LX: couldn't bind socket.");
    //}
    
    // connect to server
    
    sockaddr_in serverSocket;
    memset((int8_t*)&serverSocket, 0, sizeof(serverSocket));
    serverSocket.sin_family = AF_INET;
    serverSocket.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    serverSocket.sin_port = htons(portNumber);
    if (connect(clientSocket, (sockaddr*)&serverSocket, sizeof(serverSocket)) < 0) {
        throw runtime_error("HokuyoUST10LX: failed to connect with server.");
    }
    
    // initialize laser scanner
    
    writeLine("BM");
    readLine();
    readLine();
    readLine();
}

/**
 * Deletes the Hokuyo UST-10LX device driver object and closes the TCP/IP socket.
 */
HokuyoUST10LX::~HokuyoUST10LX() {
    
    close(clientSocket);
}

/**
 * Gets the distance measurements.
 * @param distance an array of 1081 distance measurements, given in [m].
 */
void HokuyoUST10LX::get(float distance[]) {
    
    mutex.lock();
    
    // request data from laser scanner
    
    writeLine("GE0000108000");
    readLine();
    readLine();
    readLine();
    
    // read response
    
    string buffer;
    
    for (uint16_t i = 0; i < 102; i++) {
        string s = readLine();
        s.pop_back();
        buffer += s;
    }
    readLine();
    
    // process data from buffer
    
    for (uint16_t i = 0; i < LENGTH; i++) {
        
        // get distance of points
        
        uint32_t c0 = static_cast<uint32_t>(buffer[6*i+0]) & 0xFF;
        uint32_t c1 = static_cast<uint32_t>(buffer[6*i+1]) & 0xFF;
        uint32_t c2 = static_cast<uint32_t>(buffer[6*i+2]) & 0xFF;
        
        uint32_t d = (((c0-0x30) & 0x3F) << 12) | (((c1-0x30) & 0x3F) << 6) | (((c2-0x30) & 0x3F) << 0);
        
        distance[i] = static_cast<float>(d)/1000.0f;
    }
    
    mutex.unlock();
}

/**
 * Gets the distance and reflectance measurements.
 * @param distance an array of 1081 distance measurements, given in [m].
 * @param reflectance an array of 1081 reflectance measurements, given as a relative value between 0.0 and 1.0.
 */
void HokuyoUST10LX::get(float distance[], float reflectance[]) {
    
    mutex.lock();
    
    // request data from laser scanner
    
    writeLine("GE0000108000");
    readLine();
    readLine();
    readLine();
    
    // read response
    
    string buffer;
    
    for (uint16_t i = 0; i < 102; i++) {
        string s = readLine();
        s.pop_back();
        buffer += s;
    }
    readLine();
    
    // process data from buffer
    
    for (uint16_t i = 0; i < LENGTH; i++) {
        
        // get distance and reflectance of points
        
        uint32_t c0 = static_cast<uint32_t>(buffer[6*i+0]) & 0xFF;
        uint32_t c1 = static_cast<uint32_t>(buffer[6*i+1]) & 0xFF;
        uint32_t c2 = static_cast<uint32_t>(buffer[6*i+2]) & 0xFF;
        
        uint32_t d = (((c0-0x30) & 0x3F) << 12) | (((c1-0x30) & 0x3F) << 6) | (((c2-0x30) & 0x3F) << 0);
        
        distance[i] = static_cast<float>(d)/1000.0f;
        
        uint32_t c3 = static_cast<uint32_t>(buffer[6*i+3]) & 0xFF;
        uint32_t c4 = static_cast<uint32_t>(buffer[6*i+4]) & 0xFF;
        uint32_t c5 = static_cast<uint32_t>(buffer[6*i+5]) & 0xFF;
        
        uint32_t r = (((c3-0x30) & 0x3F) << 12) | (((c4-0x30) & 0x3F) << 6) | (((c5-0x30) & 0x3F) << 0);
        
        reflectance[i] = static_cast<float>(r)/262143.0f; // 18 bit value, relative value without unit
    }
    
    mutex.unlock();
}

/**
 * Writes a line of ASCII text to the socket.
 * The given text is terminated with an LF character (0x0A).
 * @param line the line of ASCII text to write.
 */
void HokuyoUST10LX::writeLine(string line) {
    
    line += 0x0A; // terminate with LF
    
    if (write(clientSocket, line.c_str(), line.size()) < 0) throw runtime_error("HokuyoUST10LX: error writing to socket.");
}

/**
 * Reads a line of ASCII text from the socket until an LF character is read.
 * @return the line of ASCII text, without the terminating LF character.
 */
string HokuyoUST10LX::readLine() {
    
    string line;
    int8_t ch = 0;
    bool endOfLine = false;
    
    while (!endOfLine) {
        int32_t available = read(clientSocket, &ch, 1);
        if ((available > 0) && (ch != 0x0A)) line += ch;
        else endOfLine = true;
    }
    
    return line;
}
