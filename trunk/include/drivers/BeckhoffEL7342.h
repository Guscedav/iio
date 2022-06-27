/*
 * BeckhoffEL7342.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 17.12.2021
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL7342_H_
#define BECKHOFF_EL7342_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Beckhoff EL7342 DC motor terminal.
 * The EL7342 is a module that allows to control 2 DC motors.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel7342.png" width="285"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL7342 module for 2 DC motors</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL7342 beckhoffEL7342(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create EL7342 driver</span>
 *
 * AnalogOut velocity0(beckhoffEL7342, 0);  <span style="color:#008000">// create the analog outputs</span>
 * AnalogOut velocity1(beckhoffEL7342, 1);
 * DigitalOut enable0(beckhoffEL7342, 0);   <span style="color:#008000">// create the digital outputs</span>
 * DigitalOut enable1(beckhoffEL7342, 1);
 * EncoderCounter counter0(beckhoffEL7342, 0);
 * EncoderCounter counter1(beckhoffEL7342, 1);
 *
 * enable0 = true;    <span style="color:#008000">// enable the first motor controller</span>
 * velocity0 = 0.5;   <span style="color:#008000">// set the velocity to 50%</span>
 * int32_t position0 = counter0; <span style="color:#008000">// read the actual position</span>
 *
 * enable1 = true;    <span style="color:#008000">// enable the second motor controller</span>
 * velocity1 = -0.25; <span style="color:#008000">// set the velocity to 25% in negative direction</span>
 * int32_t position1 = counter1; <span style="color:#008000">// read the actual position</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL7342 : public Module, CoE::SlaveDevice {

    public:

                    BeckhoffEL7342(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~BeckhoffEL7342();
        void        writeAnalogOut(uint16_t number, float value);
        void        writeDigitalOut(uint16_t number, bool value);
        int32_t     readEncoderCounter(uint16_t number);

    private:

        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1000;
        static const uint16_t   MAILBOX_OUT_SIZE = 128;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x1080;
        static const uint16_t   MAILBOX_IN_SIZE = 128;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_OUT_SIZE = 16;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1200;
        static const uint16_t   BUFFERED_IN_SIZE = 16;

        EtherCAT&   etherCAT;           // reference to EtherCAT stack
        CoE&        coe;                // reference to CANopen over EtherCAT driver
        Mutex       mutex;              // mutex to lock critical sections
        uint16_t    controlChannel1;    // output buffer for channel 1
        uint16_t    controlChannel2;    // output buffer for channel 2
        int16_t     velocityChannel1;   // output buffer for channel 1
        int16_t     velocityChannel2;   // output buffer for channel 2
        int32_t     positionChannel1;   // input buffer for position value 1
        int32_t     positionChannel2;   // input buffer for position value 2

        EtherCAT::Datagram*     txPDO;
        EtherCAT::Datagram*     rxPDO;

        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL7342_H_ */
