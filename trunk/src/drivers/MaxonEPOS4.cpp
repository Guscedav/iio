/*
 * MaxonEPOS4.cpp
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 09.02.2017
 *      Author: Marcel Honegger
 */

#include "MaxonEPOS4.h"

using namespace std;

const int32_t MaxonEPOS4::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-12;    // priority level of private thread

/**
 * Create a MaxonEPOS4 device driver object and initialize the device and local values.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the handler thread of this driver, given in [s].
 */
MaxonEPOS4::MaxonEPOS4(CANopen& canOpen, uint32_t nodeID, double period) : RealtimeThread("MaxonEPOS4", STACK_SIZE, PRIORITY, period), canOpen(canOpen) {
    
    // initialize local values
    
    this->nodeID = nodeID;
    
    enable = false;
    newSetpoint = false;
    modesOfOperation = CYCLIC_SYNCHRONOUS_TORQUE_MODE;
    modesOfOperationDisplay = 0;
    targetPosition = 0;
    targetVelocity = 0;
    targetTorque = 0;
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
    canOpen.writeSDO(nodeID, 0x1600, 0x03, 0x607A0020, 4);  // target position, 4 bytes
    canOpen.writeSDO(nodeID, 0x1600, 0x00, 0x03, 1);        // enable RPDO1
    
    // reconfigure communication with RPDO1
    
    canOpen.writeSDO(nodeID, 0x1400, 0x01, CANopen::RPDO1+nodeID, 4);
    
    // configure RPDO2
    
    canOpen.writeSDO(nodeID, 0x1601, 0x00, 0x00, 1);        // disable RPDO2
    canOpen.writeSDO(nodeID, 0x1601, 0x01, 0x60400010, 4);  // controlword, 2 bytes
    canOpen.writeSDO(nodeID, 0x1601, 0x02, 0x60600008, 4);  // modes of operation, 1 byte
    canOpen.writeSDO(nodeID, 0x1601, 0x03, 0x60FF0020, 4);  // target velocity, 4 bytes
    canOpen.writeSDO(nodeID, 0x1601, 0x00, 0x03, 1);        // enable RPDO2
    
    // reconfigure communication with RPDO2
    
    canOpen.writeSDO(nodeID, 0x1401, 0x01, CANopen::RPDO2+nodeID, 4);
    
    // configure RPDO3
    
    canOpen.writeSDO(nodeID, 0x1602, 0x00, 0x00, 1);        // disable RPDO3
    canOpen.writeSDO(nodeID, 0x1602, 0x01, 0x60400010, 4);  // controlword, 2 bytes
    canOpen.writeSDO(nodeID, 0x1602, 0x02, 0x60600008, 4);  // modes of operation, 1 byte
    canOpen.writeSDO(nodeID, 0x1602, 0x03, 0x60710010, 4);  // target torque, 2 bytes
    canOpen.writeSDO(nodeID, 0x1602, 0x00, 0x03, 1);        // enable RPDO3
    
    // reconfigure communication with RPDO3
    
    canOpen.writeSDO(nodeID, 0x1402, 0x01, CANopen::RPDO3+nodeID, 4);

    // configure TPDO1
    
    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x00, 1);        // disable TPDO1
    canOpen.writeSDO(nodeID, 0x1A00, 0x01, 0x60410010, 4);  // statusword, 2 bytes
    canOpen.writeSDO(nodeID, 0x1A00, 0x02, 0x60610008, 4);  // modes of operation display, 1 byte
    canOpen.writeSDO(nodeID, 0x1A00, 0x03, 0x60640020, 4);  // position actual value, 4 bytes
    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x03, 1);        // enable TPDO1
    
    // reconfigure communication with TPDO1
    
    canOpen.writeSDO(nodeID, 0x1800, 0x01, 0x80000000, 4);
    canOpen.writeSDO(nodeID, 0x1800, 0x02, 253, 1);
    canOpen.writeSDO(nodeID, 0x1800, 0x03, static_cast<uint16_t>(period*10000/2), 2);
    canOpen.writeSDO(nodeID, 0x1800, 0x01, CANopen::TPDO1+nodeID, 4);
    
    // read inital object values
    
    targetPosition = static_cast<int32_t>(canOpen.readSDO(nodeID, 0x607A, 0x00));
    targetVelocity = static_cast<int32_t>(canOpen.readSDO(nodeID, 0x60FF, 0x00));
    targetTorque = static_cast<int16_t>(canOpen.readSDO(nodeID, 0x6071, 0x00));
    
    // set device into operational state
    
    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);
    
    // start handler
    
    start();
}

/**
 * Delete the MaxonEPOS4 device driver object and release all allocated resources.
 */
MaxonEPOS4::~MaxonEPOS4() {
    
    // stop handler
    
    stop();
}

/**
 * Read a virtual digital input of this servo controller.
 * This method is usually called by a DigitalIn object.
 * This driver offers 3 digital inputs: the 1st input allows to check if the power stage of this servo controller
 * is enabled and operational. The 2nd input checks if the target is reached in profile position or profile velocity mode.
 * The 3rd input checks if the internal current limitation of the power stage is active.
 * @param number the index number of the digital input, this must be 0, 1 or 2.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool MaxonEPOS4::readDigitalIn(uint16_t number) {
    
    if (number == 0) {
        
        return (statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED;
        
    } else if (number == 1) {
        
        return (statusword & TARGET_REACHED) > 0;
        
    } else if (number == 2) {
        
        return (statusword & INTERNAL_LIMIT_ACTIVE) > 0;
        
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
void MaxonEPOS4::writeDigitalOut(uint16_t number, bool value) {
    
    if (number == 0) {
        
        enable = value;
    }
}

/**
 * Gets the actual position value of the motor conntected to the servo controller.
 * @return the actual position value, given in [counts].
 */
int32_t MaxonEPOS4::readPositionActualValue() {

    return positionActualValue;
}

/**
 * Gets the desired position of the motor.
 * @return the desired position, given in [counts].
 */
int32_t MaxonEPOS4::readTargetPosition() {
    
    return targetPosition;
}

/**
 * Sets the desired position of the motor.
 * @param targetPosition the desired position, given in [counts].
 */
void MaxonEPOS4::writeTargetPosition(int32_t targetPosition) {
    
    this->modesOfOperation = PROFILE_POSITION_MODE;
    this->targetPosition = targetPosition;
    this->newSetpoint = true;
}

/**
 * Gets the desired velocity of the motor.
 * @return the desired velocity, given in [rpm].
 */
int32_t MaxonEPOS4::readTargetVelocity() {

    return targetVelocity;
}

/**
 * Sets the desired velocity of the motor.
 * @param targetVelocity the desired velocity, given in [rpm].
 */
void MaxonEPOS4::writeTargetVelocity(int32_t targetVelocity) {
    
    this->modesOfOperation = PROFILE_VELOCITY_MODE;
    this->targetVelocity = targetVelocity;
}

/**
 * Gets the desired torque of the motor connected to the servo controller.
 * @return the desired torque, given in [&permil;] of the rated torque.
 */
int16_t MaxonEPOS4::readTargetTorque() {
    
    return targetTorque;
}

/**
 * Sets the desired torque of the motor connected to the servo controller.
 * @param targetTorque the desired torque, given in [&permil;] of the rated torque.
 */
void MaxonEPOS4::writeTargetTorque(int16_t targetTorque) {
    
    this->modesOfOperation = CYCLIC_SYNCHRONOUS_TORQUE_MODE;
    this->targetTorque = targetTorque;
}

/**
 * @deprecated This method is no longer supported. Use <code>writeTargetPosition()</code> instead.
 */
void MaxonEPOS4::writePosition(int32_t targetPosition) {
    
    writeTargetPosition(targetPosition);
}

/**
 * @deprecated This method is no longer supported. Use <code>writeTargetVelocity()</code> instead.
 */
void MaxonEPOS4::writeVelocity(int32_t targetVelocity) {
    
    writeTargetVelocity(targetVelocity);
}

/**
 * @deprecated This method is no longer supported. Use <code>writeTargetTorque()</code> instead.
 */
void MaxonEPOS4::writeTorque(int16_t targetTorque) {
    
    writeTargetTorque(targetTorque);
}

/**
 * @deprecated This method is no longer supported. Use <code>readPositionActualValue()</code> instead.
 */
int32_t MaxonEPOS4::readPosition() {
    
    return readPositionActualValue();
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void MaxonEPOS4::receiveObject(uint32_t functionCode, uint8_t object[]) {
    
    if (functionCode == CANopen::TPDO1) {
        
        statusword = (static_cast<uint16_t>(object[0]) & 0xFF) | ((static_cast<uint16_t>(object[1]) & 0xFF) << 8);
        modesOfOperationDisplay = static_cast<int8_t>(object[2]);
        positionActualValue = (static_cast<int32_t>(object[3]) & 0xFF) | ((static_cast<int32_t>(object[4]) & 0xFF) << 8) | ((static_cast<int32_t>(object[5]) & 0xFF) << 16) | ((static_cast<int32_t>(object[6]) & 0xFF) << 24);
    }
}

/**
 * This method is the handler of the MaxonEPOS4 device driver.
 */
void MaxonEPOS4::run() {
    
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
        
        // transmit RPDO
        
        if (modesOfOperation == PROFILE_POSITION_MODE) {
            
            if (newSetpoint) {
                
                controlword |= NEW_SETPOINT;
                controlword |= CHANGE_SET_IMMEDIATELY;
                
                newSetpoint = false;
            }
            
            // transmit RPDO1
            
            uint8_t rpdo1[7];
            
            rpdo1[0] = static_cast<uint8_t>(controlword & 0xFF);
            rpdo1[1] = static_cast<uint8_t>((controlword >> 8) & 0xFF);
            rpdo1[2] = static_cast<uint8_t>(modesOfOperation);
            rpdo1[3] = static_cast<uint8_t>(targetPosition & 0xFF);
            rpdo1[4] = static_cast<uint8_t>((targetPosition >> 8) & 0xFF);
            rpdo1[5] = static_cast<uint8_t>((targetPosition >> 16) & 0xFF);
            rpdo1[6] = static_cast<uint8_t>((targetPosition >> 24) & 0xFF);
            
            canOpen.transmitObject(CANopen::RPDO1, nodeID, rpdo1, 7);
            
        } else if (modesOfOperation == PROFILE_VELOCITY_MODE) {
            
            // transmit RPDO2
            
            uint8_t rpdo2[7];
            
            rpdo2[0] = static_cast<uint8_t>(controlword & 0xFF);
            rpdo2[1] = static_cast<uint8_t>((controlword >> 8) & 0xFF);
            rpdo2[2] = static_cast<uint8_t>(modesOfOperation);
            rpdo2[3] = static_cast<uint8_t>(targetVelocity & 0xFF);
            rpdo2[4] = static_cast<uint8_t>((targetVelocity >> 8) & 0xFF);
            rpdo2[5] = static_cast<uint8_t>((targetVelocity >> 16) & 0xFF);
            rpdo2[6] = static_cast<uint8_t>((targetVelocity >> 24) & 0xFF);
            
            canOpen.transmitObject(CANopen::RPDO2, nodeID, rpdo2, 7);
            
        } else if (modesOfOperation == CYCLIC_SYNCHRONOUS_TORQUE_MODE) {
            
            // transmit RPDO3
            
            uint8_t rpdo3[5];
            
            rpdo3[0] = static_cast<uint8_t>(controlword & 0xFF);
            rpdo3[1] = static_cast<uint8_t>((controlword >> 8) & 0xFF);
            rpdo3[2] = static_cast<uint8_t>(modesOfOperation);
            rpdo3[3] = static_cast<uint8_t>(targetTorque & 0xFF);
            rpdo3[4] = static_cast<uint8_t>((targetTorque >> 8) & 0xFF);
            
            canOpen.transmitObject(CANopen::RPDO3, nodeID, rpdo3, 5);
        }
        
        // request TPDO1
        
        uint8_t tpdo1[7];
        
        canOpen.transmitObject(CANopen::TPDO1, nodeID, tpdo1, 7, CANRemote);
    }
}
