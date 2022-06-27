/*
 * ElmoWhistle.cpp
 * Copyright (c) 2019, ZHAW
 * All rights reserved.
 *
 *  Created on: 29.04.2019
 *      Author: Marcel Honegger
 */

#include "ElmoWhistle.h"

using namespace std;

const int32_t ElmoWhistle::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-12;   // priority level of private thread

/**
 * Create an ElmoWhistle device driver object and initialize the device and local values.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the handler thread of this driver, given in [s].
 */
ElmoWhistle::ElmoWhistle(CANopen& canOpen, uint32_t nodeID, double period) : RealtimeThread("ElmoWhistle", STACK_SIZE, PRIORITY, period), canOpen(canOpen) {
    
    // initialize local values
    
    this->nodeID = nodeID;
    
    enable = false;
    modesOfOperation = PROFILE_VELOCITY_MODE;
    targetVelocity = 0;
    statusword = 0x0000;
    positionActualValue = 0;
    
    // register this device with the CANopen device driver
    
    canOpen.registerCANopenSlave(nodeID, this);
    
    // set slave into preoperational state
    
    canOpen.transmitNMTObject(CANopen::ENTER_PREOPERATIONAL_STATE, nodeID);

    // configure RPDO1

    canOpen.writeSDO(nodeID, 0x1600, 0x00, 0x00, 1);        // disable RPDO1
    canOpen.writeSDO(nodeID, 0x1600, 0x01, 0x60400010, 4);  // controlword, 2 bytes
    canOpen.writeSDO(nodeID, 0x1600, 0x02, 0x60600008, 4);  // modes of operation, 1 byte
    canOpen.writeSDO(nodeID, 0x1600, 0x03, 0x60FF0020, 4);  // target velocity, 4 bytes
    canOpen.writeSDO(nodeID, 0x1600, 0x00, 0x03, 1);        // enable RPDO1

    // reconfigure communication with RPDO1

    canOpen.writeSDO(nodeID, 0x1400, 0x01, CANopen::RPDO1+nodeID, 4);

    // configure TPDO1

    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x00, 1);        // disable TPDO1
    canOpen.writeSDO(nodeID, 0x1A00, 0x01, 0x60410010, 4);  // statusword, 2 bytes
    canOpen.writeSDO(nodeID, 0x1A00, 0x02, 0x60640020, 4);  // position actual value, 4 bytes
    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x02, 1);        // enable TPDO1

    // reconfigure communication with TPDO1

    uint32_t inhibitTime = static_cast<uint32_t>(period*10000.0);
    if (inhibitTime < 1) inhibitTime = 1;
    uint32_t eventTimer = static_cast<uint32_t>(period*1000.0);
    if (eventTimer < 1) eventTimer = 1;

    canOpen.writeSDO(nodeID, 0x1800, 0x02, 254, 1);
    canOpen.writeSDO(nodeID, 0x1800, 0x03, inhibitTime, 2);
    canOpen.writeSDO(nodeID, 0x1800, 0x05, eventTimer, 2);

    // read inital object values

    targetVelocity = static_cast<int32_t>(canOpen.readSDO(nodeID, 0x60FF, 0x00));

    // set device into operational state
    
    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);
    
    // start handler
    
    start();
}

/**
 * Delete the ElmoWhistle device driver object and releases all allocated resources.
 */
ElmoWhistle::~ElmoWhistle() {
    
    // stop handler
    
    stop();
}

/**
 * Read a virtual digital input of this servo controller.
 * This method is usually called by a DigitalIn object.
 * This driver offers 1 digital input that allows to check if the power stage of this servo controller
 * is enabled and operational.
 * @param number the index number of the digital input, this must be 0.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool ElmoWhistle::readDigitalIn(uint16_t number) {
    
    if (number == 0) {
        
        return (statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED;
        
    } else {
        
        return false;
    }
}

/**
 * Write a digital output to enable or disable the power stage of this servo controller.
 * This method is usually called by a DigitalOut object.
 * @param number the index number of the digital output, this must be 0.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void ElmoWhistle::writeDigitalOut(uint16_t number, bool value) {
    
    if (number == 0) {
        
        enable = value;
    }
}

/**
 * Gets the actual position value of the motor conntected to the servo controller.
 * @return the actual position value, given in [counts].
 */
int32_t ElmoWhistle::readPositionActualValue() {

    return positionActualValue;
}

/**
 * Gets the desired velocity of the motor.
 * @return the desired velocity, given in [rpm].
 */
int32_t ElmoWhistle::readTargetVelocity() {

    return targetVelocity;
}

/**
 * Sets the desired velocity of the motor.
 * @param targetVelocity the desired velocity, given in [rpm].
 */
void ElmoWhistle::writeTargetVelocity(int32_t targetVelocity) {
    
    this->modesOfOperation = PROFILE_VELOCITY_MODE;
    this->targetVelocity = targetVelocity;
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void ElmoWhistle::receiveObject(uint32_t functionCode, uint8_t object[]) {
    
    if (functionCode == CANopen::TPDO1) {
        
        statusword = (static_cast<uint16_t>(object[0]) & 0xFF) | ((static_cast<uint16_t>(object[1]) & 0xFF) << 8);
        positionActualValue = (static_cast<int32_t>(object[2]) & 0xFF) | ((static_cast<int32_t>(object[3]) & 0xFF) << 8) | ((static_cast<int32_t>(object[4]) & 0xFF) << 16) | ((static_cast<int32_t>(object[5]) & 0xFF) << 24);
    }
}

/**
 * This method is the handler of the ElmoWhistle device driver.
 */
void ElmoWhistle::run() {
    
    while (waitForNextPeriod()) {
        
        // set new controlword
        
        uint16_t controlword = 0x0000;
        
        if (enable) {
            
                 if ((statusword & NOT_READY_TO_SWITCH_ON_MASK) == NOT_READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
            else if ((statusword & SWITCH_ON_DISABLED_MASK) == SWITCH_ON_DISABLED) controlword = SHUTDOWN;
            else if ((statusword & READY_TO_SWITCH_ON_MASK) == READY_TO_SWITCH_ON) controlword = SWITCH_ON;
            else if ((statusword & SWITCHED_ON_MASK) == SWITCHED_ON) controlword = ENABLE_OPERATION;
            else if ((statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED) controlword = ENABLE_OPERATION;
            else if ((statusword & QUICK_STOP_ACTIVE_MASK) == QUICK_STOP_ACTIVE) controlword = DISABLE_VOLTAGE;
            else if ((statusword & FAULT_REACTION_ACTIVE_MASK) == FAULT_REACTION_ACTIVE) controlword = DISABLE_VOLTAGE;
            else if ((statusword & FAULT_MASK) == FAULT) controlword = FAULT_RESET;
            
        } else {
            
                 if ((statusword & NOT_READY_TO_SWITCH_ON_MASK) == NOT_READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
            else if ((statusword & SWITCH_ON_DISABLED_MASK) == SWITCH_ON_DISABLED) controlword = DISABLE_VOLTAGE;
            else if ((statusword & READY_TO_SWITCH_ON_MASK) == READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
            else if ((statusword & SWITCHED_ON_MASK) == SWITCHED_ON) controlword = DISABLE_VOLTAGE;
            else if ((statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED) controlword = DISABLE_VOLTAGE;
            else if ((statusword & QUICK_STOP_ACTIVE_MASK) == QUICK_STOP_ACTIVE) controlword = DISABLE_VOLTAGE;
            else if ((statusword & FAULT_REACTION_ACTIVE_MASK) == FAULT_REACTION_ACTIVE) controlword = DISABLE_VOLTAGE;
            else if ((statusword & FAULT_MASK) == FAULT) controlword = FAULT_RESET;
        }
        
        // transmit RPDO2
        
        uint8_t rpdo1[7];
        
        rpdo1[0] = static_cast<uint8_t>(controlword & 0xFF);
        rpdo1[1] = static_cast<uint8_t>((controlword >> 8) & 0xFF);
        rpdo1[2] = static_cast<uint8_t>(modesOfOperation);
        rpdo1[3] = static_cast<uint8_t>(targetVelocity & 0xFF);
        rpdo1[4] = static_cast<uint8_t>((targetVelocity >> 8) & 0xFF);
        rpdo1[5] = static_cast<uint8_t>((targetVelocity >> 16) & 0xFF);
        rpdo1[6] = static_cast<uint8_t>((targetVelocity >> 24) & 0xFF);
        
        canOpen.transmitObject(CANopen::RPDO1, nodeID, rpdo1, 7);
    }
}
