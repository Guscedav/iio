/*
 * GrivixAutoCharge.cpp
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 05.11.2021
 *      Author: Marcel Honegger
 */

#include "CAN.h"
#include "GrivixAutoCharge.h"

using namespace std;

const int32_t GrivixAutoCharge::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-11;  // priority level of private thread
const double GrivixAutoCharge::PERIOD = 0.001;                                  // period of private thread in [s]

/**
 * Creates a Grivix Auto Charge device driver object and initializes local values.
 */
GrivixAutoCharge::GrivixAutoCharge(CAN& can) : RealtimeThread("GrivixAutoCharge", STACK_SIZE, PRIORITY, PERIOD), can(can) {

    // initialize local values

    analogIn = 0.0f;

    // start handler

    start();
}

/**
 * Deletes the Grivix Auto Charge device driver object and releases all allocated resources.
 */
GrivixAutoCharge::~GrivixAutoCharge() {

    // stop handler

    stop();
}

/**
 * This method reads the actual analog input. It is usually called by an AnalogIn object.
 * @param number the index number of the analog input, this must be 0.
 * @return the value of the analog input.
 */
float GrivixAutoCharge::readAnalogIn(uint16_t number) {

    return analogIn;
}

/**
 * This method is the handler of this device driver.
 */
void GrivixAutoCharge::run() {

    CANMessage canMessage;

    while (waitForNextPeriod()) {

        while (can.read(canMessage) != 0) {

            if (canMessage.type == CANData) {

                uint32_t id = canMessage.id;

                uint32_t data0 = static_cast<uint32_t>(canMessage.data[0]);
                uint32_t data1 = static_cast<uint32_t>(canMessage.data[1]);
                uint32_t data2 = static_cast<uint32_t>(canMessage.data[2]);
                uint32_t data3 = static_cast<uint32_t>(canMessage.data[3]);

                uint32_t data = data0 | (data1 << 8) | (data2 << 16) | (data3 << 24);

                uint16_t distance = static_cast<uint16_t>((data & X_DISTANCE_BITMASK) >> 14);
                if ((distance & 0x0100) == 0x0100) distance = distance | 0xFF00;

                analogIn = static_cast<float>(static_cast<int16_t>(distance));

            } else {

                // remote message received, ignore message
            }
        }
    }
}
