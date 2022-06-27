/*
 * GrivixAutoCharge.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 05.11.2021
 *      Author: Marcel Honegger
 */

#ifndef GRIVIX_AUTO_CHARGE_H_
#define GRIVIX_AUTO_CHARGE_H_

#include <cstdlib>
#include <string>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <pthread.h>
#include "Module.h"
#include "RealtimeThread.h"
#include "CANMessage.h"

class CAN;

/**
 * The Grivix Auto Charge class is a device driver that reads distance information
 * from an ultra wide band receiver board with a CAN bus interface. It implements
 * a Module device driver with one analog input channel.
 * <br/>
 * The following example shows how to use this device driver:
 * <pre><code>
 * PCI pci;
 * PCANpci can(pci, 0, 0);  <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * can.frequency(500000);
 *
 * GrivixAutoCharge autoCharge(can);  <span style="color:#008000">// create a driver for the Grivix Auto Charge</span>
 * AnalogIn distance(autoCharge, 0);
 *
 * float xDistance = distance.read();  <span style="color:#008000">// read a distance in [cm]</span>
 * </code></pre>
 * Also see the documentation about the AnalogIn class for more information.
 */
class GrivixAutoCharge : public Module, RealtimeThread {

    public:

                    GrivixAutoCharge(CAN& can);
        virtual     ~GrivixAutoCharge();
        float       readAnalogIn(uint16_t number);

    private:

        static const size_t     STACK_SIZE = 16*1024;   // stack size of private thread polling for CAN messages in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        static const double     PERIOD;                 // period of private thread in [s]

        static const uint32_t   X_DISTANCE_BITMASK = 0x007FC000;  // bitmasks for decoding the distance value

        CAN&        can;
        float       analogIn;

        void        run();
};

#endif /* GRIVIX_AUTO_CHARGE_H_ */
