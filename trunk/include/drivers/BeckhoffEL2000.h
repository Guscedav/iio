/*
 * BeckhoffEL2000.h
 * Copyright (c) 2018, ZHAW
 * All rights reserved.
 *
 *  Created on: 12.11.2018
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL2000_H_
#define BECKHOFF_EL2000_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for a range of Beckhoff EL2000 digital output modules.
 * The EL2000 are modules that offer 2, 4 or 8 digital outputs at a level of 24 V.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel2000.png" width="280"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL2004 module with 4 digital output channels</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL2000 beckhoffEL2000(etherCAT, coe, 0xFFFF, 1000); <span style="color:#008000">// create EL2004 driver</span>
 * 
 * DigitalOut digitalOut0(beckhoffEL2000, 0);  <span style="color:#008000">// create the digital outputs</span>
 * DigitalOut digitalOut1(beckhoffEL2000, 1);
 * DigitalOut digitalOut2(beckhoffEL2000, 2);
 * DigitalOut digitalOut3(beckhoffEL2000, 3);
 * 
 * digitalOut0 = true;       <span style="color:#008000">// set the output to logical 1</span>
 * digitalOut1 = false;      <span style="color:#008000">// set the output to logical 0</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL2000 : public Module, CoE::SlaveDevice {
    
    public:
        
                    BeckhoffEL2000(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress, uint16_t watchdogTime);
        virtual     ~BeckhoffEL2000();
        void        writeDigitalOut(uint16_t number, bool value);
        
    private:
                
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x0F00;
        static const uint16_t   BUFFERED_OUT_SIZE = 1;
        static const uint16_t   MAX_NUMBER_OF_DIGITAL_OUTPUTS = 8;
        
        EtherCAT&   etherCAT;               // reference to EtherCAT stack
        CoE&        coe;                    // reference to CANopen over EtherCAT driver
        Mutex       mutex;                  // mutex to lock critical sections
        uint8_t     outputBuffer;
        
        EtherCAT::Datagram*     rxPDO;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL2000_H_ */
