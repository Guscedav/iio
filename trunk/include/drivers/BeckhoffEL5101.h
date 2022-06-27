/*
 * BeckhoffEL5101.h
 * Copyright (c) 2020, ZHAW
 * All rights reserved.
 *
 *  Created on: 09.01.2020
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL5101_H_
#define BECKHOFF_EL5101_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Beckhoff EL5101 encoder counter interface module.
 * The EL5101 counts the increments of one quadrature encoder counter with differential RS 422 signals.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel5101.png" width="330"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL5101 module for incremental encoder counters</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL5101 encoderCounter(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create EL5101 driver</span>
 * 
 * int16_t position = encoderCounter.getPosition();  <span style="color:#008000">// read the current position value</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL5101 : public CoE::SlaveDevice {
    
    public:
        
                    BeckhoffEL5101(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~BeckhoffEL5101();
        int16_t     readPosition();
        
    private:
        
        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1800;
        static const uint16_t   MAILBOX_OUT_SIZE = 48;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x1880;
        static const uint16_t   MAILBOX_IN_SIZE = 48;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1000;
        static const uint16_t   BUFFERED_OUT_SIZE = 3;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_IN_SIZE = 5;
        
        EtherCAT&   etherCAT;       // reference to EtherCAT stack
        CoE&        coe;            // reference to CANopen over EtherCAT driver
        Mutex       mutex;          // mutex to lock critical sections
        uint8_t     status;         // buffer for input value
        uint16_t    value;          // buffer for input value
        uint16_t    latch;          // buffer for input value
        
        EtherCAT::Datagram*     rxPDO;
        EtherCAT::Datagram*     txPDO;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL5101_H_ */
