/*
 * BeckhoffEL4732.h
 * Copyright (c) 2020, ZHAW
 * All rights reserved.
 *
 *  Created on: 09.01.2020
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_EL4732_H_
#define BECKHOFF_EL4732_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Beckhoff EL4732 analog output module.
 * The EL4732 is a module that offers 2 analog outputs in the range ±10 V at a resolution of 16 bits.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffel4732.png" width="280"/></div>
 * <br/>
 * <div style="text-align:center"><b>A Beckhoff EL4732 module with 2 analog output channels</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * BeckhoffEL4732 beckhoffEL4732(etherCAT, coe, 0xFFFF, 0.005); <span style="color:#008000">// create EL4732 driver</span>
 * 
 * AnalogOut analogOut0(beckhoffEL4732, 0);  <span style="color:#008000">// create the analog outputs</span>
 * AnalogOut analogOut1(beckhoffEL4732, 1);
 * 
 * analogOut0 = 0.25;       <span style="color:#008000">// set the output to 2.5 V</span>
 * analogOut1 = -0.75;     <span style="color:#008000">// set the output to -7.5 V</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class BeckhoffEL4732 : public Module, CoE::SlaveDevice {
    
    public:
        
                    BeckhoffEL4732(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress, double period);
        virtual     ~BeckhoffEL4732();
        void        writeAnalogOut(uint16_t number, float value);
        
    private:
        
        static const uint16_t   BUFFERED_OUT_ADDRESS_1 = 0x1000;
        static const uint16_t   BUFFERED_OUT_SIZE_1 = 4;
        static const uint16_t   BUFFERED_OUT_ADDRESS_2 = 0x1400;
        static const uint16_t   BUFFERED_OUT_SIZE_2 = 4;
        static const uint16_t   NUMBER_OF_ANALOG_OUTPUTS = 2;

        EtherCAT&   etherCAT;                               // reference to EtherCAT stack
        CoE&        coe;                                    // reference to CANopen over EtherCAT driver
        Mutex       mutex;                                  // mutex to lock critical sections
        int16_t     outputBuffer[NUMBER_OF_ANALOG_OUTPUTS]; // buffer for output values

        EtherCAT::Datagram*     rxPDO1;
        EtherCAT::Datagram*     rxPDO2;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* BECKHOFF_EL4732_H_ */
