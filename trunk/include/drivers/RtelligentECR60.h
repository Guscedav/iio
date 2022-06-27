/*
 * RtelligentECR60.h
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 18.02.2022
 *      Author: Marcel Honegger
 */

#ifndef RTELLIGENT_ECR60_H_
#define RTELLIGENT_ECR60_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Rtelligent ECR60 stepper motor driver.
 * The ECR60 is a driver module that allows to control one stepper motor.
 * <br/>
 * <div style="text-align:center"><img src="rtelligentecr60.png" width="285"/></div>
 * <br/>
 * <div style="text-align:center"><b>The Rtelligent ECR60 module for one stepper motor</b></div>
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * EtherCAT etherCAT("192.168.1.130");  <span style="color:#008000">// create EtherCAT object with given interface</span>
 * CoE coe(etherCAT, 0.005);            <span style="color:#008000">// create CANopen over EtherCAT driver</span>
 * RtelligentECR60 rtelligentECR60(etherCAT, coe, 0xFFFF); <span style="color:#008000">// create ECR60 driver</span>
 *
 * DigitalOut enable(rtelligentECR60, 0);   <span style="color:#008000">// create a digital output</span>
 * DigitalIn isEnabled(rtelligentECR60, 0); <span style="color:#008000">// create a digital input</span>
 * AnalogOut position(rtelligentECR60, 0);     <span style="color:#008000">// create an analog output for the target position</span>
 * AnalogOut velocity(rtelligentECR60, 1);     <span style="color:#008000">// create an analog output for the profile velocity</span>
 * AnalogOut acceleration(rtelligentECR60, 2); <span style="color:#008000">// create an analog output for the profile acceleration</span>
 * AnalogOut deceleration(rtelligentECR60, 3); <span style="color:#008000">// create an analog output for the profile deceleration</span>
 * EncoderCounter counter(rtelligentECR60, 0); <span style="color:#008000">// create an encoder counter to read the position</span>
 *
 * enable = true;    <span style="color:#008000">// enable the motor driver</span>
 * position = 10000; <span style="color:#008000">// set the target position</span>
 * ...
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class RtelligentECR60 : public Module, CoE::SlaveDevice {

    public:

                    RtelligentECR60(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~RtelligentECR60();
        void        writeAnalogOut(uint16_t number, float value);
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        int32_t     readEncoderCounter(uint16_t number);

    private:

        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1000;
        static const uint16_t   MAILBOX_OUT_SIZE = 64;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x1080;
        static const uint16_t   MAILBOX_IN_SIZE = 64;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_OUT_SIZE = 19;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1400;
        static const uint16_t   BUFFERED_IN_SIZE = 11;

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

        static const int8_t     PROFILE_POSITION_MODE = 1;              // modes of operation
        static const int8_t     PROFILE_VELOCITY_MODE = 3;

        EtherCAT&   etherCAT;           // reference to EtherCAT stack
        CoE&        coe;                // reference to CANopen over EtherCAT driver
        Mutex       mutex;              // mutex to lock critical sections
        bool        enable;
        bool        newSetpoint;
        uint16_t    controlword;
        uint16_t    statusword;
        int8_t      modesOfOperation;
        int8_t      modesOfOperationDisplay;
        int32_t     targetPosition;
        uint32_t    profileVelocity;
        uint32_t    profileAcceleration;
        uint32_t    profileDeceleration;
        int32_t     positionActualValue;
        uint32_t    digitalInputs;

        EtherCAT::Datagram*     txPDO;
        EtherCAT::Datagram*     rxPDO;

        void        writeDatagram();
        void        readDatagram();
};

#endif /* RTELLIGENT_ECR60_H_ */
