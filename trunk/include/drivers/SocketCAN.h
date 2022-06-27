/*
 * SocketCAN.h
 * Copyright (c) 2020, ZHAW
 * All rights reserved.
 *
 *  Created on: 17.12.2020
 *      Author: Marcel Honegger
 */

#ifndef SOCKET_CAN_H_
#define SOCKET_CAN_H_

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <deque>
#include <stdint.h>
#include "Mutex.h"
#include "CAN.h"
#include "CANMessage.h"
#include "RealtimeThread.h"

/**
 * This class implements a specific CAN device driver for the socket CAN interface of Linux operating systems.
 * The socket CAN interface is a generic interface for CAN communication on linux systems, implemented as a
 * networking stack like Ethernet communication.
 * <br/>
 * To check the availability of CAN interfaces, type the following command into a terminal:
 * <code><pre>
 * % ip link show
 * </pre></code>
 * This returns a list of all available networking interfaces, including CAN interfaces. The CAN sockets are
 * numbered and called can0, can1 and so on. To configure and initialize a CAN interface, execute the following
 * commands:
 * <code><pre>
 * % sudo ip link set can0 type can bitrate 1000000
 * % sudo ip link set can0 up
 * </pre></code>
 * This will configure the baudrate of the CAN bus named 'can0' to 1 MBit/s.
 * <br/><br/>
 * When the CAN interface is configured, this device driver may be used to transmit and receive CAN messages.
 */
class SocketCAN : public CAN, public RealtimeThread {
    
    public:
        
                        SocketCAN(std::string socketName);
        virtual         ~SocketCAN();
        int32_t         write(CANMessage canMessage);
        int32_t         read(CANMessage& canMessage);
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread polling for messages in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        static const double     PERIOD;                 // period of private thread in [s]
        
        static const uint16_t   BUFFER_SIZE = 64;       // size of the software transmit and receive buffers
        
        std::deque<CANMessage>  messagesToTransmit;     // buffer with CAN messages to transmit
        std::deque<CANMessage>  receivedMessages;       // buffer with received CAN messages
        Mutex                   mutex;                  // mutex to lock critical sections
        int32_t                 canSocket;              // socket id for CAN communication
        
        void            transmit(CANMessage canMessage);
        int32_t         receive(CANMessage& canMessage);
        void            run();
};

#endif /* SOCKET_CAN_H_ */
