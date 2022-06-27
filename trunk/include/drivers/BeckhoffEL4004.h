/*
 * BeckhoffEL4004.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 22.11.2021
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL4004_H_
#define BECKHOFF_EL4004_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Beckhoff EL4004 analog output module.
 * The EL4004 is a module that offers 4 analog outputs in the range 0...10 V at a resolution of 12 bits.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel4004.png" width="320"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL4004 module with 4 analog output channels</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL4004 beckhoffEL4004(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create EL4004 driver</span>
 * 
 * AnalogOut analogOut0(beckhoffEL4004, 0);  <span style="color:#008000">// create the analog outputs</span>
 * AnalogOut analogOut1(beckhoffEL4004, 1);
 * AnalogOut analogOut2(beckhoffEL4004, 2);
 * AnalogOut analogOut3(beckhoffEL4004, 3);
 * 
 * analogOut0 = 0.25;     <span style="color:#008000">// set the output to 2.5 V</span>
 * analogOut1 = 0.75;     <span style="color:#008000">// set the output to 7.5 V</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL4004 : public Module, CoE::SlaveDevice {
    
    public:
        
                    BeckhoffEL4004(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~BeckhoffEL4004();
        void        writeAnalogOut(uint16_t number, float value);
        
    private:
        
        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1000;
        static const uint16_t   MAILBOX_OUT_SIZE = 128;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x1080;
        static const uint16_t   MAILBOX_IN_SIZE = 128;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_OUT_SIZE = 8;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1180;
        static const uint16_t   BUFFERED_IN_SIZE = 0;
        static const uint16_t   NUMBER_OF_ANALOG_OUTPUTS = 4;

        EtherCAT&   etherCAT;                               // reference to EtherCAT stack
        CoE&        coe;                                    // reference to CANopen over EtherCAT driver
        Mutex       mutex;                                  // mutex to lock critical sections
        int16_t     outputBuffer[NUMBER_OF_ANALOG_OUTPUTS]; // buffer for output values
        
        EtherCAT::Datagram*     rxPDO;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL4004_H_ */
