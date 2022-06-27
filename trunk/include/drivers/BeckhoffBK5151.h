/*
 * BeckhoffBK5151.h
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.11.2015
 *      Author: Marcel Honegger
 */

#ifndef BECKHOFF_BK5151_H_
#define BECKHOFF_BK5151_H_

#include <cstdlib>
#include <stdint.h>
#include "CANopen.h"
#include "Module.h"
#include "RealtimeThread.h"

/**
 * The BeckhoffBK5151 class is a device driver for the Beckhoff BK 5151 CANopen bus coupler.
 * Up to 12 analog inputs and 12 analog outputs, as well as up to 64 digital inputs and
 * 64 digital outputs can be connected to this bus coupler.
 * <br/>
 * <div style="text-align:center"><img src="beckhoffbk5151.jpg" width="175"/></div>
 * <br/>
 * <div style="text-align:center"><b>The Beckhoff BK 5151 CANopen bus coupler</b></div>
 * <br/>
 * To read these inputs and write these outputs, this class offers 4 methods defined by the
 * <code>Module</code> class. These methods are usually called by specific channel objects,
 * like <code>AnalogIn</code> or <code>DigitalOut</code> objects. The following example
 * shows how to use this device driver:
 * <pre><code>
 * PCI pci;
 * PCANpci pcanPCI(pci, 0, 0);              <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(pcanPCI);                <span style="color:#008000">// create a CANopen stack</span>
 * BeckhoffBK5151 bk5151(canOpen, 10, 0.005, 0.0);  <span style="color:#008000">// create a driver for node ID 10</span>
 *
 * DigitalOut led(bk5151, 0);      <span style="color:#008000">// get the 1st digital output</span>
 * DigitalIn switch(bk5151, 6);    <span style="color:#008000">// get the 7th digital input</span>
 *
 * led = true;                     <span style="color:#008000">// use of the output and input channels</span>
 * if (switch) { ... }
 * </code></pre>
 * See the documentation of the AnalogIn, AnalogOut, DigitalIn and DigitalOut classes for more information.
 */
class BeckhoffBK5151 : public Module, RealtimeThread, CANopen::Delegate {
    
    public:
        
                    BeckhoffBK5151(CANopen& canOpen, uint32_t nodeID, double period, double heartbeat);
        virtual     ~BeckhoffBK5151();
        float       readAnalogIn(uint16_t number);
        void        writeAnalogOut(uint16_t number, float value);
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        
        static const uint16_t   MAX_NUMBER_OF_ANALOG_INPUTS = 12;
        static const uint16_t   MAX_NUMBER_OF_ANALOG_OUTPUTS = 12;
        static const uint16_t   MAX_NUMBER_OF_DIGITAL_INPUTS = 64;
        static const uint16_t   MAX_NUMBER_OF_DIGITAL_OUTPUTS = 64;
        
        CANopen&    canOpen;        // reference to a CANopen stack this device driver depends on
        uint32_t    nodeID;         // the CANopen node ID of this device
        
        uint16_t    numberOfAnalogInputs;
        uint16_t    numberOfAnalogOutputs;
        uint16_t    numberOfDigitalInputs;
        uint16_t    numberOfDigitalOutputs;
        
        uint8_t     tpdo1[8];
        uint8_t     tpdo2[8];
        uint8_t     tpdo3[8];
        uint8_t     tpdo4[8];
        uint8_t     rpdo1[8];
        uint8_t     rpdo2[8];
        uint8_t     rpdo3[8];
        uint8_t     rpdo4[8];
        
        float       analogIn[MAX_NUMBER_OF_ANALOG_INPUTS];      // local buffers
        float       analogOut[MAX_NUMBER_OF_ANALOG_OUTPUTS];
        bool        digitalIn[MAX_NUMBER_OF_DIGITAL_INPUTS];
        bool        digitalOut[MAX_NUMBER_OF_DIGITAL_OUTPUTS];
        
        void        receiveObject(uint32_t functionCode, uint8_t object[]);
        void        run();
};

#endif /* BECKHOFF_BK5151_H_ */
