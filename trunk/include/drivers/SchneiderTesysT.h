/*
 * SchneiderTesysT.h
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.03.2022
 *      Author: Marcel Honegger
 */

#ifndef SCHNEIDER_TESYS_T_H_
#define SCHNEIDER_TESYS_T_H_

#include <cstdlib>
#include <stdint.h>
#include "CANopen.h"
#include "Module.h"
#include "RealtimeThread.h"

/**
 * This class is a device driver for the Schneider Electric Tesys T LTM motor management controller.
 * This module driver implements 8 analog inputs and 64 digital inputs.
 * <br/>
 * <div style="text-align:center"><img src="schneidertesyst.jpg" width="400"/></div>
 * <div style="text-align:center"><b>Schneider Electric Tesys T LTM motor management controller</b></div>
 * <br/>
 * To read these inputs, this class offers 2 methods defined by the <code>Module</code> class.
 * These methods are usually called by specific channel objects, like <code>AnalogIn</code> or
 * <code>DigitalIn</code> objects. The following example shows how to use this device driver:
 * <pre><code>
 * PCI pci;
 * TPMC901 tpmc901(pci, 0, 0);                <span style="color:#008000">// create CAN device driver for 1st port on 1st board</span>
 * CANopen canOpen(tpmc901);                  <span style="color:#008000">// create a CANopen stack</span>
 * SchneiderTesysT tesys(canOpen, 20, 0.005); <span style="color:#008000">// create a driver for node ID 20</span>
 *
 * AnalogIn averageVoltage(tesys, 0);   <span style="color:#008000">// get the analog inputs</span>
 * AnalogIn voltageL3L1(tesys, 1);
 * AnalogIn voltageL1L2(tesys, 2);
 * AnalogIn voltageL2L3(tesys, 3);
 * AnalogIn averageCurrent(tesys, 4);
 * AnalogIn currentL1(tesys, 5);
 * AnalogIn currentL2(tesys, 6);
 * AnalogIn currentL3(tesys, 7);
 * 
 * DigitalIn systemReady(tesys, 0);     <span style="color:#008000">// get the digital inputs</span>
 * DigitalIn systemOn(tesys, 1);
 * DigitalIn systemFault(tesys, 2);
 * DigitalIn systemWarning(tesys, 3);
 * DigitalIn systemTripped(tesys, 4);
 * DigitalIn faultResetAuthorized(tesys, 5);
 * DigitalIn controllerPower(tesys, 6);
 * DigitalIn motorRunning(tesys, 7);
 * DigitalIn autoResetActive(tesys, 16);
 * DigitalIn faultPowerCycleRequested(tesys, 18);
 * DigitalIn motorRestartTimeUndefined(tesys, 19);
 * DigitalIn rapidCycleLockout(tesys, 20);
 * DigitalIn loadShedding(tesys, 21);
 * DigitalIn motorSpeed(tesys, 22);
 * DigitalIn hmiPortCommLoss(tesys, 23);
 * DigitalIn networkPortCommLoss(tesys, 24);
 * DigitalIn motorTransitionLockout(tesys, 25);
 * DigitalIn groundCurrentFault(tesys, 34);
 * DigitalIn thermalOverloadFault(tesys, 35);
 * DigitalIn longStartFault(tesys, 36);
 * DigitalIn jamFault(tesys, 37);
 * DigitalIn currentPhaseImbalanceFault(tesys, 38);
 * DigitalIn undercurrentFault(tesys, 39);
 * DigitalIn testFault(tesys, 41);
 * DigitalIn hmiPortFault(tesys, 42);
 * DigitalIn controllerInternalFault(tesys, 43);
 * DigitalIn internalPortFault(tesys, 44);
 * DigitalIn networkPortInternalFault(tesys, 45);
 * DigitalIn networkPortConfigFault(tesys, 46);
 * DigitalIn networkPortFault(tesys, 47);
 * DigitalIn diagnosticFault(tesys, 49);
 * DigitalIn wiringFault(tesys, 50);
 * DigitalIn overcurrentFault(tesys, 51);
 * DigitalIn currentPhaseLossFault(tesys, 52);
 * DigitalIn currentPhaseReversalFault(tesys, 53);
 * DigitalIn motorTemperatureSensorFault(tesys, 54);
 * DigitalIn voltagePhaseImbalanceFault(tesys, 55);
 * DigitalIn voltagePhaseLossFault(tesys, 56);
 * DigitalIn voltagePhaseReversalFault(tesys, 57);
 * DigitalIn undervoltageFault(tesys, 58);
 * DigitalIn overvoltageFault(tesys, 59);
 * DigitalIn underpowerFault(tesys, 60);
 * DigitalIn overpowerFault(tesys, 61);
 * DigitalIn underPowerFactorFault(tesys, 62);
 * DigitalIn overPowerFactorFault(tesys, 63);
 * 
 * if (systemFault) { ... }     <span style="color:#008000">// use of the input channels</span>
 * else if (undercurrentFault) { ... }
 * </code></pre>
 * See the documentation of the AnalogIn and DigitalIn classes for more information.
 */
class SchneiderTesysT : public Module, RealtimeThread, CANopen::Delegate {
    
    public:
        
                    SchneiderTesysT(CANopen& canOpen, uint32_t nodeID, double period);
        virtual     ~SchneiderTesysT();
        float       readAnalogIn(uint16_t number);
        bool        readDigitalIn(uint16_t number);
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        
        static const uint16_t   NUMBER_OF_ANALOG_INPUTS = 8;
        static const uint16_t   NUMBER_OF_DIGITAL_INPUTS = 64;
        
        CANopen&    canOpen;        // reference to a CANopen stack this device driver depends on
        uint32_t    nodeID;         // the CANopen node ID of this device
        
        uint8_t     tpdo1[8];
        uint8_t     tpdo2[8];
        uint8_t     tpdo3[8];
        uint8_t     tpdo4[8];
        uint8_t     rpdo1[8];
        uint8_t     rpdo2[8];
        uint8_t     rpdo3[8];
        uint8_t     rpdo4[8];
        
        float       analogIn[NUMBER_OF_ANALOG_INPUTS];      // local buffers
        bool        digitalIn[NUMBER_OF_DIGITAL_INPUTS];
        
        void        receiveObject(uint32_t functionCode, uint8_t object[]);
        void        run();
};

#endif /* SCHNEIDER_TESYS_T_H_ */
