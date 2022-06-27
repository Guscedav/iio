/*
 * MaxonEPOS4.h
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 09.02.2017
 *      Author: Marcel Honegger
 */

#ifndef MAXON_EPOS4_H_
#define MAXON_EPOS4_H_

#include <cstdlib>
#include <stdint.h>
#include "Module.h"
#include "RealtimeThread.h"
#include "CANopen.h"

/**
 * The MaxonEPOS4 class is a device driver for maxon motor EPOS4 servo controllers.
 * <br/>
 * <div style="text-align:center"><img src="maxonEPOS4.jpg" width="400"/></div>
 * <div style="text-align:center"><b>The maxon motor EPOS4 servo controller</b></div>
 * <br/>
 * This device driver uses a CANopen object to communicate with the servo controller.
 * It offers methods to enable or disable the device, to set a desired position, velocity or
 * torque value and to read the actual position value of the drive. The following example shows
 * how to use this device driver:
 * <pre><code>
 * PCI pci;
 * PCANpci pcanPCI(pci, 0, 0);                   <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(pcanPCI);                     <span style="color:#008000">// create a CANopen stack</span>
 * MaxonEPOS4 maxonEPOS4(canOpen, 20, 0.002);    <span style="color:#008000">// create a driver for node ID 20</span>
 *
 * DigitalOut enable(maxonEPOS4, 0);             <span style="color:#008000">// get the digital output</span>
 * DigitalIn isEnabled(maxonEPOS4, 0);           <span style="color:#008000">// get the 1st digital input</span>
 * DigitalIn targetReached(maxonEPOS4, 1);       <span style="color:#008000">// get the 2nd digital input</span>
 * DigitalIn internalLimitActive(maxonEPOS4, 2); <span style="color:#008000">// get the 3rd digital input</span>
 *
 * enable = true;             <span style="color:#008000">// enable the power stage of the servo controller</span>
 * if (isEnabled) {           <span style="color:#008000">// check if the power stage is enabled and operational</span>
 * 
 *     maxonEPOS4.writeTargetTorque(500);                       <span style="color:#008000">// write a desired torque value</span>
 *     int32_t position = maxonEPOS4.readPositionActualValue(); <span style="color:#008000">// read the actual position</span>
 * }
 * </code></pre>
 * Also see the documentation of the DigitalIn and DigitalOut classes for more information.
 */
class MaxonEPOS4 : public Module, RealtimeThread, CANopen::Delegate {
    
    public:
        
                    MaxonEPOS4(CANopen& canOpen, uint32_t nodeID, double period);
        virtual     ~MaxonEPOS4();
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        int32_t     readPositionActualValue();
        int32_t     readTargetPosition();
        void        writeTargetPosition(int32_t targetPosition);
        int32_t     readTargetVelocity();
        void        writeTargetVelocity(int32_t targetVelocity);
        int16_t     readTargetTorque();
        void        writeTargetTorque(int16_t targetTorque);
        void        writePosition(int32_t targetPosition);
        void        writeVelocity(int32_t targetVelocity);
        void        writeTorque(int16_t targetTorque);
        int32_t     readPosition();
        
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
        
        static const uint16_t   NEW_SETPOINT = 0x0010;              // controlword bits (operation mode dependent)
        static const uint16_t   CHANGE_SET_IMMEDIATELY = 0x0020;
        
        static const uint16_t   NOT_READY_TO_SWITCH_ON = 0x0000;    // predefined statuswords (object 0x6041)
        static const uint16_t   SWITCH_ON_DISABLED = 0x0040;
        static const uint16_t   READY_TO_SWITCH_ON = 0x0021;
        static const uint16_t   SWITCHED_ON = 0x0023;
        static const uint16_t   OPERATION_ENABLED = 0x0027;
        static const uint16_t   QUICK_STOP_ACTIVE = 0x0007;
        static const uint16_t   FAULT_REACTION_ACTIVE = 0x000F;
        static const uint16_t   FAULT = 0x0008;
        
        static const uint16_t   NOT_READY_TO_SWITCH_ON_MASK = 0x004F;   // bitmask for statusword
        static const uint16_t   SWITCH_ON_DISABLED_MASK = 0x004F;
        static const uint16_t   READY_TO_SWITCH_ON_MASK = 0x006F;
        static const uint16_t   SWITCHED_ON_MASK = 0x006F;
        static const uint16_t   OPERATION_ENABLED_MASK = 0x006F;
        static const uint16_t   QUICK_STOP_ACTIVE_MASK = 0x006F;
        static const uint16_t   FAULT_REACTION_ACTIVE_MASK = 0x004F;
        static const uint16_t   FAULT_MASK = 0x004F;
        
        static const uint16_t   TARGET_REACHED = 0x0400;            // statusword bits (operation mode dependent)
        static const uint16_t   INTERNAL_LIMIT_ACTIVE = 0x0800;
        
        static const int8_t     PROFILE_POSITION_MODE = 1;              // modes of operation
        static const int8_t     PROFILE_VELOCITY_MODE = 3;
        static const int8_t     CYCLIC_SYNCHRONOUS_TORQUE_MODE = 10;
        
        CANopen&    canOpen;
        uint32_t    nodeID;
        bool        enable;
        bool        newSetpoint;
        int8_t      modesOfOperation;
        int8_t      modesOfOperationDisplay;
        int32_t     targetPosition;
        int32_t     targetVelocity;
        int16_t     targetTorque;
        uint16_t    statusword;
        int32_t     positionActualValue;
        
        void        receiveObject(uint32_t functionCode, uint8_t object[]);
        void        run();
};

#endif /* MAXON_EPOS4_H_ */
