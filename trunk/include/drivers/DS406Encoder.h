/*
 * DS406Encoder.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 15.10.2021
 *      Author: Marcel Honegger
 */

#ifndef DS406_ENCODER_H_
#define DS406_ENCODER_H_

#include <cstdlib>
#include <stdint.h>
#include "Module.h"
#include "CANopen.h"

/**
 * The DS406Encoder class is a device driver for CANopen encoder counters
 * that support the DS406 application profile.
 * <br/>
 * This device driver uses a CANopen object to communicate with the encoder counter.
 * It offers one method to read the current value of the encoder counter. The following
 * example shows how to use this device driver:
 * <pre><code>
 * PCI pci;
 * PCANpci pcanPCI(pci, 0, 0);                 <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(pcanPCI);                   <span style="color:#008000">// create a CANopen stack</span>
 * DS406Encoder encoder(canOpen, 41, 0.002);   <span style="color:#008000">// create a driver for node ID 41</span>
 *
 * EncoderCounter encoderCounter(encoder, 0);  <span style="color:#008000">// get the encoder counter channel</span>
 * ...
 *
 * int32_t position = encoderCounter;          <span style="color:#008000">// read the actual position</span>
 * </code></pre>
 * Also see the documentation about the CANopen and EncoderCounter classes for more information.
 */
class DS406Encoder : public Module, CANopen::Delegate {

    public:

                    DS406Encoder(CANopen& canOpen, uint32_t nodeID, double period);
        virtual     ~DS406Encoder();
        int32_t     readEncoderCounter(uint16_t number);

    private:

        CANopen&    canOpen;
        int32_t     positionValue;

        void        receiveObject(uint32_t functionCode, uint8_t object[]);
};

#endif /* DS406_ENCODER_H_ */
