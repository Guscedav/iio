/*
 * HetronicRC.h
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 03.03.2022
 *      Author: Marcel Honegger
 */

#ifndef HETRONIC_RC_H_
#define HETRONIC_RC_H_

#include <cstdlib>
#include <stdint.h>
#include "CANopen.h"
#include "Module.h"
#include "RealtimeThread.h"

/**
 * The HetronicRC class is a device driver for the Hetronic radio remote control panel with a CANopen
 * interface, made by Hetronic Steuersysteme GmbH. The CANopen communication objects used by this device
 * are definded by Hetronic Steuersysteme GmbH and comply to the CANopen specification DS-301.
 * <br/>
 * This module driver implements up to 8 analogue sensors of joysticks, 64 digital sensors (in two
 * groups) of buttons and switches and 32 digital actuators for leds.
 * <br/>
 * To read these inputs and write these outputs, this class offers 3 methods defined by the
 * <code>Module</code> class. These methods are usually called by specific channel objects,
 * like <code>AnalogIn</code> or <code>DigitalOut</code> objects. The following example
 * shows how to use this device driver:
 * <pre><code>
 * PCI pci;
 * TPMC901 tpmc901(pci, 0, 0);        <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(tpmc901);          <span style="color:#008000">// create a CANopen stack</span>
 * HetronicRC rc(canOpen, 10, 0.005); <span style="color:#008000">// create a driver for node ID 10</span>
 *
 * AnalogIn joystickX(rc, 0);    <span style="color:#008000">// get the 1st analog input</span>
 * AnalogIn joystickY(rc, 1);    <span style="color:#008000">// get the 2nd analog input</span>
 * DigitalOut ledGreen(rc, 0);   <span style="color:#008000">// get the 1st digital output</span>
 * DigitalOut ledRed(rc, 1);     <span style="color:#008000">// get the 2nd digital output</span>
 * DigitalIn switch(rc, 6);      <span style="color:#008000">// get the 7th digital input</span>
 *
 * ledGreen = true;              <span style="color:#008000">// use of the output and input channels</span>
 * if (switch) { ... }
 * </code></pre>
 * See the documentation of the AnalogIn, DigitalIn and DigitalOut classes for more information.
 */
class HetronicRC : public Module, RealtimeThread, CANopen::Delegate {
    
    public:
        
                    HetronicRC(CANopen& canOpen, uint32_t nodeID, double period);
        virtual     ~HetronicRC();
        float       readAnalogIn(uint16_t number);
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        
        static const uint16_t   NUMBER_OF_ANALOG_INPUTS = 8;
        static const uint16_t   NUMBER_OF_DIGITAL_INPUTS = 64;
        static const uint16_t   NUMBER_OF_DIGITAL_OUTPUTS = 32;
        
        CANopen&    canOpen;        // reference to a CANopen stack this device driver depends on
        uint32_t    nodeID;         // the CANopen node ID of this device
        
        uint8_t     rpdo1[8];
        uint8_t     rpdo2[8];
        
        float       analogIn[NUMBER_OF_ANALOG_INPUTS];      // local buffers
        bool        digitalIn[NUMBER_OF_DIGITAL_INPUTS];
        bool        digitalOut[NUMBER_OF_DIGITAL_OUTPUTS];
        
        void        receiveObject(uint32_t functionCode, uint8_t object[]);
        void        run();
};

#endif /* HETRONIC_RC_H_ */
