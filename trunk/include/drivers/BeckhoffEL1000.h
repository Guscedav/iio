/*
 * BeckhoffEL1000.h
 * Copyright (c) 2018, ZHAW
 * All rights reserved.
 *
 *  Created on: 12.11.2018
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL1000_H_
#define BECKHOFF_EL1000_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for a range of Beckhoff EL1000 digital input modules.
 * The EL1000 are modules that offer 2, 4 or 8 digital inputs at a level of 24 V.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel1000.png" width="280"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL1012 module with 2 digital input channels</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL1000 beckhoffEL1000(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create EL1000 driver</span>
 * 
 * DigitalIn digitalIn0(beckhoffEL1000, 0);  <span style="color:#008000">// create the digital inputs</span>
 * DigitalIn digitalIn1(beckhoffEL1000, 1);
 * 
 * if (digitalIn0) {  <span style="color:#008000">// check the input 0</span>
 *     ...
 * }
 *
 * bool value = digitalIn1;  <span style="color:#008000">// read the input 1</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL1000 : public Module, CoE::SlaveDevice {
    
    public:
        
                    BeckhoffEL1000(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~BeckhoffEL1000();
        bool        readDigitalIn(uint16_t number);
        
    private:
        
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1000;
        static const uint16_t   BUFFERED_IN_SIZE = 1;
        static const uint16_t   MAX_NUMBER_OF_DIGITAL_INPUTS = 8;
        
        EtherCAT&   etherCAT;               // reference to EtherCAT stack
        CoE&        coe;                    // reference to CANopen over EtherCAT driver
        Mutex       mutex;                  // mutex to lock critical sections
        uint8_t     inputBuffer;
        
        EtherCAT::Datagram*     txPDO;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL1000_H_ */
