/*
 * BeckhoffEL7332.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 22.11.2021
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL7332_H_
#define BECKHOFF_EL7332_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Beckhoff EL7332 DC motor terminal.
 * The EL7332 is a module that allows to control 2 DC motors.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel7332.png" width="260"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL7332 module for 2 DC motors</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL7332 beckhoffEL7332(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create EL7332 driver</span>
 * 
 * AnalogOut velocity0(beckhoffEL7332, 0);  <span style="color:#008000">// create the analog outputs</span>
 * AnalogOut velocity1(beckhoffEL7332, 1);
 * DigitalOut enable0(beckhoffEL7332, 0);   <span style="color:#008000">// create the digital outputs</span>
 * DigitalOut enable1(beckhoffEL7332, 1);
 * 
 * enable0 = true;    <span style="color:#008000">// enable the first motor controller</span>
 * velocity0 = 0.5;   <span style="color:#008000">// set the velocity to 50%</span>
 * 
 * enable1 = true;    <span style="color:#008000">// enable the second motor controller</span>
 * velocity1 = -0.25; <span style="color:#008000">// set the velocity to 25% in negative direction</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL7332 : public Module, CoE::SlaveDevice {
    
    public:
        
                    BeckhoffEL7332(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~BeckhoffEL7332();
        void        writeAnalogOut(uint16_t number, float value);
        void        writeDigitalOut(uint16_t number, bool value);
        
    private:
        
        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1000;
        static const uint16_t   MAILBOX_OUT_SIZE = 128;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x1080;
        static const uint16_t   MAILBOX_IN_SIZE = 128;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_OUT_SIZE = 16;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1200;
        static const uint16_t   BUFFERED_IN_SIZE = 12;

        EtherCAT&   etherCAT;           // reference to EtherCAT stack
        CoE&        coe;                // reference to CANopen over EtherCAT driver
        Mutex       mutex;              // mutex to lock critical sections
        uint16_t    controlChannel1;    // output buffer for channel 1
        uint16_t    controlChannel2;    // output buffer for channel 2
        int16_t     velocityChannel1;   // output buffer for channel 1
        int16_t     velocityChannel2;   // output buffer for channel 2
        
        EtherCAT::Datagram*     txPDO;
        EtherCAT::Datagram*     rxPDO;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL7332_H_ */
