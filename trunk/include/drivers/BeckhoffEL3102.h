/*
 * BeckhoffEL3102.h
 * Copyright (c) 2020, ZHAW
 * All rights reserved.
 *
 *  Created on: 09.01.2020
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL3102_H_
#define BECKHOFF_EL3102_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Beckhoff EL3102 analog input module.
 * The EL3102 is a module that offers 2 analog inputs in the range ±10 V at a resolution of 16 bits.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel3102.png" width="280"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL3102 module with 2 analog input channels</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL3102 beckhoffEL3102(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create EL3102 driver</span>
 * 
 * AnalogIn analogIn0(beckhoffEL3102, 0);  <span style="color:#008000">// create the analog inputs</span>
 * AnalogIn analogIn1(beckhoffEL3102, 1);
 * 
 * if (analogIn0 > 0.5) {  <span style="color:#008000">// check the input 0</span>
 *     ...
 * }
 *
 * float value = analogIn1;  <span style="color:#008000">// read the input 1</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL3102 : public Module, CoE::SlaveDevice {
    
    public:
        
                    BeckhoffEL3102(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~BeckhoffEL3102();
        float       readAnalogIn(uint16_t number);
        
    private:
        
        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1800;
        static const uint16_t   MAILBOX_OUT_SIZE = 246;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x18f6;
        static const uint16_t   MAILBOX_IN_SIZE = 246;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1000;
        static const uint16_t   BUFFERED_OUT_SIZE = 0;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_IN_SIZE = 6;
        static const uint16_t   NUMBER_OF_ANALOG_INPUTS = 2;
        
        EtherCAT&   etherCAT;                               // reference to EtherCAT stack
        CoE&        coe;                                    // reference to CANopen over EtherCAT driver
        Mutex       mutex;                                  // mutex to lock critical sections
        int16_t     inputBuffer[NUMBER_OF_ANALOG_INPUTS];   // buffer for input values
        
        EtherCAT::Datagram*     txPDO;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL3102_H_ */
