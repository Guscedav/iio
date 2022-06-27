/*
 * SchneiderAltivar31.cpp
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 07.03.2022
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "SchneiderAltivar31.h"

using namespace std;

const int32_t SchneiderAltivar31::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-12;  // priority level of private thread

/**
 * Create a SchneiderAltivar31 device driver object and initialize the device and local values.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the handler thread of this driver, given in [s].
 * @param heartbeat the period of the heartbeat messages from the master to the slave, given in [s].
 */
SchneiderAltivar31::SchneiderAltivar31(CANopen& canOpen, uint32_t nodeID, double period, double heartbeat) : RealtimeThread("SchneiderAltivar31", STACK_SIZE, PRIORITY, period), canOpen(canOpen) {
    
    // initialize private values
    
    this->nodeID = nodeID;
    
    controlword = 0x0000;
    statusword = 0x0000;
    
    for (uint16_t i = 0; i < NUMBER_OF_ANALOG_INPUTS; i++) analogIn[i] = 0.0f;
    analogOut = 0.0f;
    for (uint16_t i = 0; i < NUMBER_OF_DIGITAL_INPUTS; i++) digitalIn[i] = false;
    digitalOut = false;
    
    // register this device with the CANopen device driver
    
    canOpen.registerCANopenSlave(nodeID, this);
    
    // set slave into preoperational state
    
    canOpen.transmitNMTObject(CANopen::RESET_NODE, nodeID);
    
    Thread::sleep(1000);
    
    canOpen.transmitNMTObject(CANopen::ENTER_PREOPERATIONAL_STATE, nodeID);
    
    Thread::sleep(100);
    
    // reconfigure communication with TPDO1
    
    canOpen.writeSDO(nodeID, 0x1800, 0x01, CANopen::TPDO1+nodeID, 4);
    canOpen.writeSDO(nodeID, 0x1800, 0x03, static_cast<int32_t>(period*10000), 2);
    canOpen.writeSDO(nodeID, 0x1800, 0x05, static_cast<int32_t>(period*1000), 2);
    
    // reconfigure communication with TPDO2
    
    canOpen.writeSDO(nodeID, 0x1805, 0x01, CANopen::TPDO2+nodeID, 4);
    
    // configure TPDO2
    
    canOpen.writeSDO(nodeID, 0x1A05, 0x00, 0x00, 1);        // disable PDO
    canOpen.writeSDO(nodeID, 0x1A05, 0x01, 0x20162B10, 4);  // analogue input: 0x2016/0x2B, 2 bytes, 0..10V, in [mV]
    canOpen.writeSDO(nodeID, 0x1A05, 0x02, 0x20162C10, 4);  // analogue input: 0x2016/0x2C, 2 bytes, -10V..10V, in [mV]
    canOpen.writeSDO(nodeID, 0x1A05, 0x03, 0x20162D10, 4);  // analogue input: 0x2016/0x2D, 2 bytes, 4..20mA, in [uA]
    canOpen.writeSDO(nodeID, 0x1A05, 0x04, 0x20162910, 4);  // digital inputs: 0x2016/0x29, 2 bytes, bits 0 - 5
    canOpen.writeSDO(nodeID, 0x1A05, 0x00, 0x04, 1);        // enable PDO
    
    // reconfigure communication with TPDO2
    
    canOpen.writeSDO(nodeID, 0x1805, 0x03, static_cast<int32_t>(period*10000), 2);
    canOpen.writeSDO(nodeID, 0x1805, 0x05, static_cast<int32_t>(period*1000), 2);
    
    // configure RPDO1
    
    canOpen.writeSDO(nodeID, 0x1400, 0x01, CANopen::RPDO1+nodeID, 4);
    
    // configure RPDO2
    
    canOpen.writeSDO(nodeID, 0x1405, 0x01, CANopen::RPDO2+nodeID, 4);
    
    // configure consumer heartbeat time for master ID 1
    
    if (heartbeat > 0.0) {
        canOpen.writeSDO(nodeID, 0x1016, 0x01, 0x10000 | static_cast<uint16_t>(heartbeat*1000*2), 4);
    } else {
        canOpen.writeSDO(nodeID, 0x1016, 0x01, 0x10000 | 0, 4);
    }
    
    // configure producer heartbeat time
    
    if (heartbeat > 0.0) {
        canOpen.writeSDO(nodeID, 0x1017, 0x00, static_cast<uint16_t>(heartbeat*1000), 2);
    } else {
        canOpen.writeSDO(nodeID, 0x1017, 0x00, 0, 2);
    }
    
    // set device into operational state
    
    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);
    
    // start handler
    
    start();
}

/**
 * Delete the SchneiderAltivar31 device driver object and release all allocated resources.
 */
SchneiderAltivar31::~SchneiderAltivar31() {
    
    // stop handler
    
    stop();
}

/**
 * This method reads an analog input. It is usually called by an AnalogIn object.
 * @param number the index number of the analog input, this must be in the range 0..2.
 * @return the value of the analog input.
 */
float SchneiderAltivar31::readAnalogIn(uint16_t number) {
    
    if (number < NUMBER_OF_ANALOG_INPUTS) {
        
        return analogIn[number];
        
    } else {
        
        return 0.0f;
    }
}

/**
 * Write an analog output to set the target velocity of this frequency converter.
 * @param number the index number of the analog output, this must be 0.
 * @param value the desired target velocity, given in [rpm].
 */
void SchneiderAltivar31::writeAnalogOut(uint16_t number, float value) {

    if (number == 0) {
        
        analogOut = value;
        
    } else {
        
        // ignore output value
    }
}

/**
 * Read a digital input of this frequency converter.
 * This method is usually called by a DigitalIn object.
 * This driver offers seven digital inputs. The first input allows to check if the power stage of this frequency
 * converter is enabled and operational, and the other inputs correspond to general purpose input channels.
 * @param number the index number of the digital input, this must be in the range 0..6.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool SchneiderAltivar31::readDigitalIn(uint16_t number) {
    
    if (number < NUMBER_OF_DIGITAL_INPUTS) {
        
        return digitalIn[number];
        
    } else {
        
        return false;
    }
}

/**
 * Write a digital output to enable or disable the power stage of this frequency converter.
 * This method is usually called by a DigitalOut object.
 * @param number the index number of the digital output, this must be 0.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void SchneiderAltivar31::writeDigitalOut(uint16_t number, bool value) {
    
    if (number == 0) {
        
        digitalOut = value;
        
    } else {
        
        // ignore output value
    }
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void SchneiderAltivar31::receiveObject(uint32_t functionCode, uint8_t object[]) {
    
    if (functionCode == CANopen::TPDO1) {
        
        statusword = (static_cast<uint16_t>(object[0]) & 0xFF) | ((static_cast<uint16_t>(object[1]) & 0xFF) << 8);
        
        digitalIn[0] = ((statusword & STATUSWORD_MASK) == OPERATION_ENABLED);
        
    } else if (functionCode == CANopen::TPDO2) {
        
        for (uint16_t i = 0; i < NUMBER_OF_ANALOG_INPUTS; i++) analogIn[i] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[0+2*i]) | (static_cast<uint32_t>(object[1+2*i]) << 8)));
        for (uint16_t i = 1; i < NUMBER_OF_DIGITAL_INPUTS; i++) digitalIn[i] = (object[6] & (1 << (i-1))) > 0;
    }
}

/**
 * This method is the handler of the MaxonIDX device driver.
 */
void SchneiderAltivar31::run() {
    
    while (waitForNextPeriod()) {
        
        // set new controlword
        
        controlword = 0x0000;

        if (digitalOut) {

                 if ((statusword & STATUSWORD_MASK) == NOT_READY_TO_SWITCH_ON) controlword = SHUTDOWN;
            else if ((statusword & STATUSWORD_MASK) == SWITCH_ON_DISABLED) controlword = SHUTDOWN;
            else if ((statusword & STATUSWORD_MASK) == READY_TO_SWITCH_ON) controlword = SWITCH_ON;
            else if ((statusword & STATUSWORD_MASK) == SWITCHED_ON) controlword = ENABLE_OPERATION;
            else if ((statusword & STATUSWORD_MASK) == OPERATION_ENABLED) controlword = ENABLE_OPERATION;
            else if ((statusword & STATUSWORD_MASK) == QUICK_STOP_ACTIVE) controlword = DISABLE_VOLTAGE;
            else if ((statusword & FAULT_MASK) == FAULT_REACTION_ACTIVE) controlword = FAULT_RESET;
            else if ((statusword & FAULT_MASK) == FAULT) controlword = FAULT_RESET;

        } else {

                 if ((statusword & STATUSWORD_MASK) == NOT_READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
            else if ((statusword & STATUSWORD_MASK) == SWITCH_ON_DISABLED) controlword = DISABLE_VOLTAGE;
            else if ((statusword & STATUSWORD_MASK) == READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
            else if ((statusword & STATUSWORD_MASK) == SWITCHED_ON) controlword = SHUTDOWN;
            else if ((statusword & STATUSWORD_MASK) == OPERATION_ENABLED) controlword = DISABLE_OPERATION;
            else if ((statusword & STATUSWORD_MASK) == QUICK_STOP_ACTIVE) controlword = DISABLE_VOLTAGE;
            else if ((statusword & FAULT_MASK) == FAULT_REACTION_ACTIVE) controlword = FAULT_RESET;
            else if ((statusword & FAULT_MASK) == FAULT) controlword = FAULT_RESET;
        }
        
        // transmit RPDO2
        
        uint8_t rpdo2[4];
        
        rpdo2[0] = static_cast<uint8_t>(controlword & 0xFF);
        rpdo2[1] = static_cast<uint8_t>((controlword >> 8) & 0xFF);
        
        float value = 30.0f*analogOut;
        value = (value < -32760.0) ? -32760.0 : value;
        value = (value > 32760.0) ? 32760.0 : value;
        int16_t frequency = static_cast<int16_t>(value);
        rpdo2[2] = static_cast<uint8_t>(frequency & 0xFF);
        rpdo2[3] = static_cast<uint8_t>((frequency >> 8) & 0xFF);
        
        canOpen.transmitObject(CANopen::RPDO2, nodeID, rpdo2, 4);
    }
}
