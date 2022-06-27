/*
 * MaxonIDX.cpp
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.08.2021
 *      Author: Marcel Honegger
 */

#include "MaxonIDX.h"

using namespace std;

const int32_t MaxonIDX::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-12;  // priority level of private thread

/**
 * Create a MaxonIDX device driver object and initialize the device and local values.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the handler thread of this driver, given in [s].
 */
MaxonIDX::MaxonIDX(CANopen& canOpen, uint32_t nodeID, double period) : RealtimeThread("MaxonIDX", STACK_SIZE, PRIORITY, period), canOpen(canOpen) {
    
    // initialize local values
    
    this->nodeID = nodeID;
    
    controlword = 0x0000;
    statusword = 0x0000;
    modesOfOperation = HOMING_MODE;
    modesOfOperationDisplay = 0;
    targetVelocity = 0;
    positionActualValue = 0;
    state = STATE_OFF;
    stateDemand = STATE_OFF;
    
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
    canOpen.writeSDO(nodeID, 0x1A00, 0x02, 0x60610008, 4);  // modes of operation display, 1 byte
    canOpen.writeSDO(nodeID, 0x1A00, 0x03, 0x60640020, 4);  // position actual value, 4 bytes
    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x03, 1);        // enable TPDO1
    
    // reconfigure communication with TPDO1
    
    canOpen.writeSDO(nodeID, 0x1800, 0x01, 0x80000000, 4);
    canOpen.writeSDO(nodeID, 0x1800, 0x02, 253, 1);
    canOpen.writeSDO(nodeID, 0x1800, 0x03, static_cast<uint16_t>(period*10000/2), 2);
    canOpen.writeSDO(nodeID, 0x1800, 0x01, CANopen::TPDO1+nodeID, 4);
    
    // configure miscellaneous objects
    
    canOpen.writeSDO(nodeID, 0x60B1, 0x00, 0, 4);   // velocity offset
    canOpen.writeSDO(nodeID, 0x60C2, 0x01, static_cast<uint8_t>(period*1000.0+0.5), 1); // interpolation time period
    
    // read inital object values
    
    targetVelocity = static_cast<int32_t>(canOpen.readSDO(nodeID, 0x60FF, 0x00));
    
    // set device into operational state
    
    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);
    
    // start handler
    
    start();
}

/**
 * Delete the MaxonIDX device driver object and release all allocated resources.
 */
MaxonIDX::~MaxonIDX() {
    
    // stop handler
    
    stop();
}

/**
 * Write an analog output to set the target velocity of this servo motor.
 * @param number the index number of the analog output, this must be 0.
 * @param value the desired target velocity, given in [rpm].
 */
void MaxonIDX::writeAnalogOut(uint16_t number, float value) {

    targetVelocity = static_cast<int32_t>(value);
}

/**
 * Read a virtual digital input of this servo motor.
 * This method is usually called by a DigitalIn object.
 * This driver offers two digital inputs. The first input allows to check if the power stage of this servo motor
 * is enabled and operational, and the second input checks if the homing procedure is completed.
 * @param number the index number of the digital input, this must be 0 or 1.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool MaxonIDX::readDigitalIn(uint16_t number) {
    
    if (number == 0) {
        
        return (state == STATE_ON) || (state == STATE_HOMING) || (state == STATE_RUNNING);
        
    } else if (number == 1) {
        
        return (state == STATE_RUNNING);
        
    } else {
        
        return false;
    }
}

/**
 * Write a digital output to enable or disable the power stage of this servo motor.
 * This method is usually called by a DigitalOut object.
 * @param number the index number of the digital output, this must be 0.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void MaxonIDX::writeDigitalOut(uint16_t number, bool value) {
    
    if (number == 0) {
        
        if (value) {
            
            stateDemand = STATE_RUNNING;
            
        } else {
            
            stateDemand = STATE_OFF;
        }
    }
}

/**
 * Read the encoder counter of this servo motor.
 * This method is usually called by an EncoderCounter object.
 * @param number the index number of the encoder counter, this must be 0.
 * @return the actual position value, given in [counts].
 */
int32_t MaxonIDX::readEncoderCounter(uint16_t number) {

    return positionActualValue;
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void MaxonIDX::receiveObject(uint32_t functionCode, uint8_t object[]) {
    
    if (functionCode == CANopen::TPDO1) {
        
        statusword = (static_cast<uint16_t>(object[0]) & 0xFF) | ((static_cast<uint16_t>(object[1]) & 0xFF) << 8);
        modesOfOperationDisplay = static_cast<int8_t>(object[2]);
        positionActualValue = (static_cast<int32_t>(object[3]) & 0xFF) | ((static_cast<int32_t>(object[4]) & 0xFF) << 8) | ((static_cast<int32_t>(object[5]) & 0xFF) << 16) | ((static_cast<int32_t>(object[6]) & 0xFF) << 24);
    }
}

/**
 * This method is the handler of the MaxonIDX device driver.
 */
void MaxonIDX::run() {
    
    while (waitForNextPeriod()) {
        
        // handle internal state machine
        
        switch (state) {
        
            case STATE_OFF:
                
                     if ((statusword & NOT_READY_TO_SWITCH_ON_MASK) == NOT_READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
                else if ((statusword & SWITCH_ON_DISABLED_MASK) == SWITCH_ON_DISABLED) controlword = DISABLE_VOLTAGE;
                else if ((statusword & READY_TO_SWITCH_ON_MASK) == READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
                else if ((statusword & SWITCHED_ON_MASK) == SWITCHED_ON) controlword = DISABLE_VOLTAGE;
                else if ((statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED) controlword = DISABLE_VOLTAGE;
                else if ((statusword & QUICK_STOP_ACTIVE_MASK) == QUICK_STOP_ACTIVE) controlword = DISABLE_VOLTAGE;
                else if ((statusword & FAULT_REACTION_ACTIVE_MASK) == FAULT_REACTION_ACTIVE) controlword = DISABLE_VOLTAGE;
                else if ((statusword & FAULT_MASK) == FAULT) controlword = FAULT_RESET;
                
                modesOfOperation = HOMING_MODE;
                
                if (stateDemand == STATE_RUNNING) {
                    
                    state = STATE_SWITCH_ON;
                    
                } else {
                    
                    // do nothing
                }
                
                break;
                
            case STATE_SWITCH_ON:
                
                if (stateDemand == STATE_RUNNING) {
                    
                         if ((statusword & NOT_READY_TO_SWITCH_ON_MASK) == NOT_READY_TO_SWITCH_ON) controlword = DISABLE_VOLTAGE;
                    else if ((statusword & SWITCH_ON_DISABLED_MASK) == SWITCH_ON_DISABLED) controlword = SHUTDOWN;
                    else if ((statusword & READY_TO_SWITCH_ON_MASK) == READY_TO_SWITCH_ON) controlword = SWITCH_ON;
                    else if ((statusword & SWITCHED_ON_MASK) == SWITCHED_ON) controlword = ENABLE_OPERATION;
                    else if ((statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED) {
                        
                        controlword = ENABLE_OPERATION;
                        state = STATE_ON;
                        
                    } else if ((statusword & QUICK_STOP_ACTIVE_MASK) == QUICK_STOP_ACTIVE) controlword = DISABLE_VOLTAGE;
                    else if ((statusword & FAULT_REACTION_ACTIVE_MASK) == FAULT_REACTION_ACTIVE) controlword = DISABLE_VOLTAGE;
                    else if ((statusword & FAULT_MASK) == FAULT) controlword = FAULT_RESET;
                    
                } else {
                    
                    state = STATE_OFF;
                }
                
                break;
                
            case STATE_ON:
                
                if (stateDemand == STATE_RUNNING) {
                    
                    if ((statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED) {
                        
                        controlword = ENABLE_OPERATION | HOMING_OPERATION_START;
                        state = STATE_HOMING;
                        
                    } else {
                        
                        state = STATE_OFF;
                        stateDemand = STATE_OFF;
                    }
                    
                } else {
                    
                    state = STATE_OFF;
                }
                
                break;
                
            case STATE_HOMING:
                
                if (stateDemand == STATE_RUNNING) {
                    
                    if ((statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED) {
                        
                        if ((statusword & HOMING_ATTAINED) > 0) {
                            
                            controlword = ENABLE_OPERATION;
                            modesOfOperation = CYCLIC_SYNCHRONOUS_VELOCITY_MODE;
                            state = STATE_RUNNING;
                            
                        } else {
                            
                            // do nothing
                        }
                        
                    } else {
                        
                        state = STATE_OFF;
                        stateDemand = STATE_OFF;
                    }
                    
                } else {
                    
                    state = STATE_OFF;
                }
                
                break;
                
            case STATE_RUNNING:
                
                if (stateDemand == STATE_RUNNING) {
                    
                    if ((statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED) {
                        
                        // do nothing
                        
                    } else {
                        
                        state = STATE_OFF;
                        stateDemand = STATE_OFF;
                    }
                    
                } else {
                    
                    state = STATE_OFF;
                }
                
                break;
                
            default:
                
                state = STATE_OFF;
                stateDemand = STATE_OFF;
        }
        
        // transmit RPDO1
        
        uint8_t rpdo1[7];
        
        rpdo1[0] = static_cast<uint8_t>(controlword & 0xFF);
        rpdo1[1] = static_cast<uint8_t>((controlword >> 8) & 0xFF);
        rpdo1[2] = static_cast<uint8_t>(modesOfOperation);
        rpdo1[3] = static_cast<uint8_t>(targetVelocity & 0xFF);
        rpdo1[4] = static_cast<uint8_t>((targetVelocity >> 8) & 0xFF);
        rpdo1[5] = static_cast<uint8_t>((targetVelocity >> 16) & 0xFF);
        rpdo1[6] = static_cast<uint8_t>((targetVelocity >> 24) & 0xFF);
        
        canOpen.transmitObject(CANopen::RPDO1, nodeID, rpdo1, 7);
        
        // request TPDO1
        
        uint8_t tpdo1[7];
        
        canOpen.transmitObject(CANopen::TPDO1, nodeID, tpdo1, 7, CANRemote);
    }
}
