/*
 * Ethernet.cpp
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 14.08.2017
 *      Author: Marcel Honegger
 */

#include "Ethernet.h"

using namespace std;

/**
 * Creates an <code>Ethernet</code> object.
 */
Ethernet::Ethernet() {}

/**
 * Deletes the <code>Ethernet</code> object.
 */
Ethernet::~Ethernet() {}

/**
 * Returns if an ethernet link is present or not.
 * @return <code>false</code> if no ethernet link is present, <code>true</code> if an ethernet link is present.
 */
bool Ethernet::link() {
    
    return false;
}

/**
 * Gives the ethernet MAC address of this hardware interface.
 * @param macAddress A 6 byte array to copy the ethernet MAC address into.
 */
void Ethernet::address(uint8_t macAddress[6]) {}

/**
 * Sends an ethernet frame to transmit on the network.
 * @param destinationMACAddress an array of 6 bytes representing the MAC address
 * of the destination of this frame.
 * @param etherType the ether type value of this frame, i.e. <code>ETHERTYPE_IPV4</code>
 * for an IPv4 packet.
 * @param data a buffer with the payload of this frame.
 * @param length the size of the given buffer pointed to by <code>data</code>, given in [bytes].
 * @return the number of bytes actually sent.
 */
uint16_t Ethernet::send(uint8_t destinationMACAddress[6], uint16_t etherType, uint8_t data[], uint16_t length) {
    
    return 0;
}

/**
 * Receives an ethernet frame from the network.
 * @param sourceMACAddress a buffer of 6 bytes to store the MAC address of the source of the frame.
 * @param etherType the ether type value of the received frame.
 * @param data a buffer to write the payload of the received frame into.
 * @param length the size of the given buffer pointed to by <code>data</code>.
 * @return the number of received bytes, or 0 if no frame was received.
 */
uint16_t Ethernet::receive(uint8_t sourceMACAddress[6], uint16_t& etherType, uint8_t data[], uint16_t length) {
    
    return 0;
}
