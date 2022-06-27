/*
 * MaxonIDX.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.08.2021
 *      Author: Marcel Honegger
 */

#ifndef MAXON_IDX_H_
#define MAXON_IDX_H_

#include <cstdlib>
#include <stdint.h>
#include "Module.h"
#include "RealtimeThread.h"
#include "CANopen.h"

/**
 * The MaxonIDX class is a device driver for maxon motor IDX integrated servo motors.
 * <br/>
 * <div style="text-align:center"><img src="maxonidx.jpg" width="400"/></div>
 * <div style="text-align:center"><b>The maxon motor IDX servo motors</b></div>
 * <br/>
 * This device driver uses a CANopen object to communicate with the servo motor.
 * It offers methods to enable or disable the device, to set a desired velocity value
 * and to read the actual position value of the servo motor. The following example shows
 * how to use this device driver:
 * <pre><code>
 * PCI pci;
 * PCANpci pcanPCI(pci, 0, 0);                   <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(pcanPCI);                     <span style="color:#008000">// create a CANopen stack</span>
 * MaxonIDX maxonIDX(canOpen, 20, 0.002);        <span style="color:#008000">// create a driver for node ID 20</span>
 *
 * DigitalOut enable(maxonIDX, 0);               <span style="color:#008000">// get the digital output</span>
 * DigitalIn isEnabled(maxonIDX, 0);             <span style="color:#008000">// get the digital input</span>
 *
 * AnalogOut targetVelocity(maxonIDX, 0);       <span style="color:#008000">// get an analog output</span>
 * EncoderCounter encoderCounter(maxonIDX, 0);  <span style="color:#008000">// get the encoder counter</span>
 *
 * enable = true;               <span style="color:#008000">// enable the power stage of the servo controller</span>
 * if (isEnabled) {             <span style="color:#008000">// check if the power stage is enabled and operational</span>
 * 
 *     targetVelocity = 300.0f;            <span style="color:#008000">// write a desired velocity value</span>
 *     int32_t position = encoderCounter;  <span style="color:#008000">// read the actual position</span>
 * }
 * </code></pre>
 * Also see the documentation about the DigitalIn, DigitalOut, AnalogOut and EncoderCounter classes for more information.
 */
class MaxonIDX : public Module, RealtimeThread, CANopen::Delegate {
    
    public:
        
                    MaxonIDX(CANopen& canOpen, uint32_t nodeID, double period);
        virtual     ~MaxonIDX();
        void        writeAnalogOut(uint16_t number, float value);
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        int32_t     readEncoderCounter(uint16_t number);
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        
        static const uint16_t   SHUTDOWN = 0x0006;      // predefined controlwords (object 0x6040)
        static const uint16_t   SWITCH_ON = 0x0007;
        static const uint16_t   DISABLE_VOLTAGE = 0x0000;
        static const uint16_t   QUICK_STOP = 0x0002;
        static const uint16_t   DISABLE_OPERATION = 0x0007;
        static const uint16_t   ENABLE_OPERATION = 0x000F;
        static const uint16_t   FAULT_RESET = 0x0080;
        
        static const uint16_t   HOMING_OPERATION_START = 0x0010;    // controlword bits (operation mode dependent)
        
        static const uint16_t   NOT_READY_TO_SWITCH_ON = 0x0000;    // predefined statuswords (object 0x6041)
        static const uint16_t   SWITCH_ON_DISABLED = 0x0040;
        static const uint16_t   READY_TO_SWITCH_ON = 0x0021;
        static const uint16_t   SWITCHED_ON = 0x0023;
        static const uint16_t   OPERATION_ENABLED = 0x0027;
        static const uint16_t   QUICK_STOP_ACTIVE = 0x0007;
        static const uint16_t   FAULT_REACTION_ACTIVE = 0x000F;
        static const uint16_t   FAULT = 0x0008;
        
        static const uint16_t   NOT_READY_TO_SWITCH_ON_MASK = 0x006F;   // bitmask for statusword
        static const uint16_t   SWITCH_ON_DISABLED_MASK = 0x006F;
        static const uint16_t   READY_TO_SWITCH_ON_MASK = 0x006F;
        static const uint16_t   SWITCHED_ON_MASK = 0x006F;
        static const uint16_t   OPERATION_ENABLED_MASK = 0x006F;
        static const uint16_t   QUICK_STOP_ACTIVE_MASK = 0x006F;
        static const uint16_t   FAULT_REACTION_ACTIVE_MASK = 0x006F;
        static const uint16_t   FAULT_MASK = 0x006F;
        
        static const uint16_t   TARGET_REACHED = 0x0400;            // statusword bits (operation mode dependent)
        static const uint16_t   HOMING_ATTAINED = 0x1000;
        static const uint16_t   HOMING_ERROR = 0x2000;
        
        static const int8_t     HOMING_MODE = 6;                    // modes of operation
        static const int8_t     CYCLIC_SYNCHRONOUS_VELOCITY_MODE = 9;
        
        static const int16_t    STATE_OFF = 0;
        static const int16_t    STATE_SWITCH_ON = 1;
        static const int16_t    STATE_ON = 2;
        static const int16_t    STATE_HOMING = 3;
        static const int16_t    STATE_RUNNING = 4;
        
        CANopen&    canOpen;
        uint32_t    nodeID;
        uint16_t    controlword;
        uint16_t    statusword;
        int8_t      modesOfOperation;
        int8_t      modesOfOperationDisplay;
        int32_t     targetVelocity;
        int32_t     positionActualValue;
        int16_t     state;
        int16_t     stateDemand;
        
        void        receiveObject(uint32_t functionCode, uint8_t object[]);
        void        run();
};

#endif /* MAXON_IDX_H_ */
