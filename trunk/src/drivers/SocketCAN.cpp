/*
 * SocketCAN.cpp
 * Copyright (c) 2020, ZHAW
 * All rights reserved.
 *
 *  Created on: 17.12.2020
 *      Author: Marcel Honegger
 */

#if defined __QNX__

#else

#include <iostream>
#include <cstring>
#include <typeinfo>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#endif

#include "SocketCAN.h"

using namespace std;

const int32_t SocketCAN::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-10;
const double SocketCAN::PERIOD = 0.0005;

/**
 * Creates a CAN device driver object and initializes the CAN socket.
 * @param socketName a file descriptor of the CAN socket to use, like 'can0' or 'can1'.
 */
SocketCAN::SocketCAN(string socketName) : RealtimeThread("SocketCAN", STACK_SIZE, PRIORITY, PERIOD) {
    
    #if defined __QNX__
    
    #else
    
    canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    
    cout << "SocketCAN: canSocket=" << canSocket << endl;
    
    ifreq ifr;
    strcpy(ifr.ifr_name, socketName.c_str());
    int32_t result = ioctl(canSocket, SIOCGIFINDEX, &ifr);
    
    cout << "SocketCAN: ioctl(canSocket, SIOCGIFINDEX, &ifr)=" << result << endl;
    
    int32_t flags = fcntl(canSocket, F_GETFL, 0);
    result = fcntl(canSocket, F_SETFL, flags | O_NONBLOCK);
    
    cout << "SocketCAN: fcntl(canSocket, F_SETFL, flags | O_NONBLOCK)=" << result << endl;
    
    sockaddr_can address;
    address.can_family = AF_CAN;
    address.can_ifindex = ifr.ifr_ifindex;
    result = bind(canSocket, (sockaddr*)&address, sizeof(address));
    
    cout << "SocketCAN: bind(...)=" << result << endl;
    
    #endif
    
    // start handler

    start();
}

/**
 * Deletes the CAN device driver object and releases all allocated resources.
 */
SocketCAN::~SocketCAN() {

    // stop handler

    stop();
}

/**
 * Writes a CAN message for transmission on the CAN bus.
 * This method stores a copy of the given CAN message in a software transmit buffer.
 * @param canMessage a CAN message object to transmit.
 * @return 0 if this write command failed, 1 otherwise.
 */
int32_t SocketCAN::write(CANMessage canMessage) {

    if (messagesToTransmit.size() < BUFFER_SIZE) {

        mutex.lock();

        messagesToTransmit.push_back(canMessage);

        mutex.unlock();

        return 1;

    } else {

        return 0;
    }
}

/**
 * Reads a CAN message received from the CAN bus.
 * @param canMessage a reference to a CAN message object to overwrite.
 * @return 0 if no message was received, 1 if a message could be read successfully.
 */

int32_t SocketCAN::read(CANMessage& canMessage) {

    if (receivedMessages.size() > 0) {

        mutex.lock();

        CANMessage canMessageOnStack = receivedMessages.front();
        canMessage.id = canMessageOnStack.id;
        for (uint8_t i = 0; i < 8; i++) canMessage.data[i] = canMessageOnStack.data[i];
        canMessage.len = canMessageOnStack.len;
        canMessage.type = canMessageOnStack.type;
        receivedMessages.pop_front();

        mutex.unlock();

        return 1;

    } else {

        return 0;
    }
}

/**
 * This method is the handler of this CAN device driver.
 */
void SocketCAN::run() {

    while (waitForNextPeriod()) {
        
        #if defined __QNX__
        
        #else
        
        // tries to write a message from the software transmit buffer to the CAN socket interface

        if (messagesToTransmit.size() > 0) {

            mutex.lock();

            CANMessage canMessage(messagesToTransmit.front());
            
            can_frame frame;
            frame.can_id = static_cast<int32_t>(canMessage.id);
            if (canMessage.type == CANRemote) frame.can_id |= CAN_RTR_FLAG;
            frame.can_dlc = static_cast<int32_t>(canMessage.len);
            for (int32_t i = 0; i < frame.can_dlc; i++) frame.data[i] = canMessage.data[i];
            
            int32_t written = ::write(canSocket, &frame, sizeof(can_frame));
            
            if (written > 0) {
                
                // remove message from buffer
                
                messagesToTransmit.pop_front();
                
            } else {
                
                // leave message in buffer
            }
            
            mutex.unlock();
        }

        // tries to read messages from the CAN socket interface
        
        can_frame frame;
        
        int32_t bytesRead = ::read(canSocket, &frame, sizeof(frame));
        
        while (bytesRead > 0) {
            
            CANMessage canMessage;
            
            canMessage.id = static_cast<uint32_t>(frame.can_id & CAN_SFF_MASK);
            canMessage.type = (frame.can_id & CAN_RTR_FLAG) > 0 ? CANRemote : CANData;
            canMessage.len = static_cast<uint8_t>(frame.can_dlc);
            
            for (uint8_t i = 0; i < canMessage.len; i++) {
                
                canMessage.data[i] = frame.data[i];
            }
            
            if (receivedMessages.size() < BUFFER_SIZE) {

                mutex.lock();

                receivedMessages.push_back(canMessage);

                mutex.unlock();

            } else {

                // software receive buffer is full, discard message
            }
            
            bytesRead = ::read(canSocket, &frame, sizeof(frame));
        }
        
        #endif
    }
}
