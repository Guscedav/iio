/*
 * CAN.cpp
 * Copyright (c) 2014, ZHAW
 * All rights reserved.
 *
 *  Created on: 23.12.2014
 *      Author: Marcel Honegger
 */

#include "CANMessage.h"
#include "CAN.h"

using namespace std;

/**
 * Creates a CAN object.
 */
CAN::CAN() {}

/**
 * Deletes the CAN object.
 */
CAN::~CAN() {}

/**
 * Sets the frequency of the CAN bus, given in [Hz].
 * @param hz the frequency of the CAN bus, given in [Hz].
 */
void CAN::frequency(uint32_t hz) {}

/**
 * Writes a CAN message for transmission on the CAN bus.
 * This method must be implemented by a specific CAN driver.
 * @param canMessage a CAN message object to transmit.
 * @return 0 if this write command failed, 1 otherwise.
 */
int32_t CAN::write(CANMessage canMessage) {
    
    return 0;
}

/**
 * Reads a CAN message received from the CAN bus.
 * This method must be implemented by a specific CAN driver.
 * @param canMessage a reference to a CAN message object to overwrite.
 * @return 0 if no message was received, 1 if a message could be read successfully.
 */
int32_t CAN::read(CANMessage& canMessage) {
    
    return 0;
}
