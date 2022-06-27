/*
 * SchneiderAltivar31.h
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 07.03.2022
 *      Author: Marcel Honegger
 */

#ifndef SCHNEIDER_ALTIVAR_31_H_
#define SCHNEIDER_ALTIVAR_31_H_

#include <cstdlib>
#include <stdint.h>
#include "Module.h"
#include "RealtimeThread.h"
#include "CANopen.h"

/**
 * This class is a device driver for the Schneider Electric Altivar 31 frequency converter.
 * This converter implements the CANopen device profile DSP-402.
 * <br/>
 * <div style="text-align:center"><img src="schneideraltivar31.jpg" width="400"/></div>
 * <div style="text-align:center"><b>The Schneider Electric Altivar 31 frequency converter</b></div>
 * <br/>
 * this driver offers methods to access the following channels: one analog output to set the desired speed,
 * a digital output to enable and disable the frequency converter, 3 general purpose analog inputs, one digital
 * input to check if the frequency converter is enabled and another 6 general purpose digital inputs.
 * <br/><br/>
 * The following example shows how to use this device driver:
 * <pre><code>
 * PCI pci;
 * TPMC901 tpmc901(pci, 0, 0);   <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(tpmc901);     <span style="color:#008000">// create a CANopen stack</span>
 * SchneiderAltivar31 altivar(canOpen, 20, 0.002, 0.1); <span style="color:#008000">// create a driver for node ID 20</span>
 *
 * DigitalOut enable(altivar, 0);           <span style="color:#008000">// get inputs and outputs</span>
 * DigitalIn isEnabled(altivar, 0);
 * DigitalIn digitalIn0(altivar, 1);
 * DigitalIn digitalIn1(altivar, 2);
 * DigitalIn digitalIn2(altivar, 3);
 * DigitalIn digitalIn3(altivar, 4);
 * DigitalIn digitalIn4(altivar, 5);
 * DigitalIn digitalIn5(altivar, 6);
 * AnalogOut speed(altivar, 0);
 * AnalogIn analogIn0(altivar, 0);
 * AnalogIn analogIn1(altivar, 1);
 * AnalogIn analogIn2(altivar, 2);
 *
 * enable = true;               <span style="color:#008000">// enable the power stage of the frequency converter</span>
 * if (isEnabled) {             <span style="color:#008000">// check if the power stage is enabled and operational</span>
 * 
 *     speed = 300.0f;          <span style="color:#008000">// write a desired speed value</span>
 * }
 * </code></pre>
 * Also see the documentation about the DigitalOut, DigitalIn, AnalogOut and AnalogIn classes for more information.
 */
class SchneiderAltivar31 : public Module, RealtimeThread, CANopen::Delegate {
    
    public:
        
                    SchneiderAltivar31(CANopen& canOpen, uint32_t nodeID, double period, double heartbeat);
        virtual     ~SchneiderAltivar31();
        float       readAnalogIn(uint16_t number);
        void        writeAnalogOut(uint16_t number, float value);
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        
        static const uint16_t   NUMBER_OF_ANALOG_INPUTS = 3;
        static const uint16_t   NUMBER_OF_DIGITAL_INPUTS = 7;
        
        static const uint16_t   SHUTDOWN = 0x0006;      // predefined controlwords (object 0x6040)
        static const uint16_t   SWITCH_ON = 0x0007;
        static const uint16_t   DISABLE_VOLTAGE = 0x0000;
        static const uint16_t   DISABLE_OPERATION = 0x0007;
        static const uint16_t   ENABLE_OPERATION = 0x000F;
        static const uint16_t   FAULT_RESET = 0x0080;
        
        static const uint16_t   NOT_READY_TO_SWITCH_ON = 0x0000;    // predefined statuswords (object 0x6041)
        static const uint16_t   SWITCH_ON_DISABLED = 0x0040;
        static const uint16_t   READY_TO_SWITCH_ON = 0x0021;
        static const uint16_t   SWITCHED_ON = 0x0023;
        static const uint16_t   OPERATION_ENABLED = 0x0027;
        static const uint16_t   QUICK_STOP_ACTIVE = 0x0007;
        static const uint16_t   FAULT_REACTION_ACTIVE = 0x000F;
        static const uint16_t   FAULT = 0x0008;
        
        static const uint16_t   STATUSWORD_MASK = 0x006F;           // bitmask for statusword
        static const uint16_t   FAULT_MASK = 0x006F;                // bitmask for fault statusword
        
        CANopen&    canOpen;
        uint32_t    nodeID;
        uint16_t    controlword;
        uint16_t    statusword;
        
        float       analogIn[NUMBER_OF_ANALOG_INPUTS];  // local buffers
        float       analogOut;
        bool        digitalIn[NUMBER_OF_DIGITAL_INPUTS];
        bool        digitalOut;
        
        void        receiveObject(uint32_t functionCode, uint8_t object[]);
        void        run();
};

#endif /* SCHNEIDER_ALTIVAR_31_H_ */
