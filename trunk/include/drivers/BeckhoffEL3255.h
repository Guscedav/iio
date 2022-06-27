/*
 * BeckhoffEL3255.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 22.11.2021
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL3255_H_
#define BECKHOFF_EL3255_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Beckhoff EL3255 analog input module.
 * The EL3255 is a module that offers 5 analog inputs for potentiometers with a resolution of 16 bits.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel3255.png" width="290"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL3255 module with 5 analog input channels</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL3104 beckhoffEL3255(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create EL3255 driver</span>
 *
 * AnalogIn analogIn0(beckhoffEL3255, 0);  <span style="color:#008000">// create the analog inputs</span>
 * AnalogIn analogIn1(beckhoffEL3255, 1);
 * AnalogIn analogIn2(beckhoffEL3255, 2);
 * AnalogIn analogIn3(beckhoffEL3255, 3);
 * AnalogIn analogIn4(beckhoffEL3255, 4);
 *
 * if (analogIn0 > 0.5) {  <span style="color:#008000">// check the input 0</span>
 *     ...
 * }
 *
 * float value = analogIn2;  <span style="color:#008000">// read the input 2</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL3255 : public Module, CoE::SlaveDevice {

    public:

                    BeckhoffEL3255(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~BeckhoffEL3255();
        float       readAnalogIn(uint16_t number);

    private:

        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1000;
        static const uint16_t   MAILBOX_OUT_SIZE = 128;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x1080;
        static const uint16_t   MAILBOX_IN_SIZE = 128;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_OUT_SIZE = 0;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1180;
        static const uint16_t   BUFFERED_IN_SIZE = 20;
        static const uint16_t   NUMBER_OF_ANALOG_INPUTS = 5;

        EtherCAT&   etherCAT;                               // reference to EtherCAT stack
        CoE&        coe;                                    // reference to CANopen over EtherCAT driver
        Mutex       mutex;                                  // mutex to lock critical sections
        int16_t     inputBuffer[NUMBER_OF_ANALOG_INPUTS];   // buffer for input values

        EtherCAT::Datagram*     txPDO;

        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL3255_H_ */
