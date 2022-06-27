/*
 * IMSServocontroller.h
 * Copyright (c) 2020, ZHAW
 * All rights reserved.
 *
 *  Created on: 27.01.2020
 *      Author: Marcel Honegger
 */

#ifndef IMS_SERVOCONTROLLER_H_
#define IMS_SERVOCONTROLLER_H_

#include <cstdlib>
#include <stdint.h>
#include "Module.h"
#include "RealtimeThread.h"
#include "CANopen.h"

/**
 * The IMS Servocontroller class is a device driver for the high performance servo controller developed at the IMS of ZHAW.
 * <br/>
 * <div style="text-align:center"><img src="imsservocontroller.png" width="400"/></div>
 * <div style="text-align:center"><b>The IMS servocontroller</b></div>
 * <br/>
 * This device driver uses a CANopen object to communicate with the servo controller.
 * It offers methods to enable or disable the device, and to set a desired torque value.
 * The following example shows how to use this device driver:
 * <pre><code>
 * PCI pci;
 * PCANpci pcanPCI(pci, 0, 0);                  <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(pcanPCI);                    <span style="color:#008000">// create a CANopen stack</span>
 * IMSServocontroller servocontroller(canOpen, 20, 0.001);  <span style="color:#008000">// create a driver for node ID 20</span>
 *
 * DigitalOut enable(servocontroller, 0);       <span style="color:#008000">// get the digital output</span>
 * DigitalIn isEnabled(servocontroller, 0);     <span style="color:#008000">// get the 1st digital input</span>
 *
 * enable = true;             <span style="color:#008000">// enable the power stage of the servo controller</span>
 * if (isEnabled) {           <span style="color:#008000">// check if the power stage is enabled and operational</span>
 *
 *     servocontroller.writeTargetCurrent(500);  <span style="color:#008000">// write a desired current value</span>
 * }
 * </code></pre>
 * Also see the documentation of the DigitalIn and DigitalOut classes for more information.
 */
class IMSServocontroller : public Module, RealtimeThread, CANopen::Delegate {

    public:

                    IMSServocontroller(CANopen& canOpen, uint32_t nodeID, double period);
        virtual     ~IMSServocontroller();
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        void        writeTargetCurrent(int16_t targetCurrent);
        int32_t     readPositionActualValue();

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

        CANopen&    canOpen;
        uint32_t    nodeID;
        bool        enable;
        int16_t     targetCurrent;
        uint16_t    statusword;
        int32_t     positionActualValue;

        void        receiveObject(uint32_t functionCode, uint8_t object[]);
        void        run();
};

#endif /* IMS_SERVOCONTROLLER_H_ */
