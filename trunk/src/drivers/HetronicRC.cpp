/*
 * HetronicRC.cpp
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 03.03.2022
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "HetronicRC.h"

using namespace std;

const int32_t HetronicRC::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-12;    // priority level of private thread

/**
 * Creates a HetronicRC device driver object and initializes the device and local values.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the handler thread of this driver, given in [s].
 */
HetronicRC::HetronicRC(CANopen& canOpen, uint32_t nodeID, double period) : RealtimeThread("HetronicRC", STACK_SIZE, PRIORITY, period), canOpen(canOpen) {
    
    this->nodeID = nodeID;
    
    // register this device with the CANopen device driver
    
    canOpen.registerCANopenSlave(nodeID, this);
    
    // set device into preoperational state
    
    canOpen.transmitNMTObject(CANopen::ENTER_PREOPERATIONAL_STATE, nodeID);
    
    Thread::sleep(100);
    
    // set device into operational state
    
    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);
    
    // start handler
    
    start();
}

/**
 * Deletes the HetronicRC device driver object and releases all allocated resources.
 */
HetronicRC::~HetronicRC() {

    // stop handler
    
    stop();
}

/**
 * This method reads the actual analog input. It is usually called by an AnalogIn object.
 * @param number the index number of the analog input.
 * @return the value of the analog input.
 */
float HetronicRC::readAnalogIn(uint16_t number) {

    if (number < NUMBER_OF_ANALOG_INPUTS) {
        
        return analogIn[number];
        
    } else {
        
        return 0.0f;
    }
}

/**
 * This method reads the actual digital input. It is usually called by a DigitalIn object.
 * @param number the index number of the digital input.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool HetronicRC::readDigitalIn(uint16_t number) {
    
    if (number < NUMBER_OF_DIGITAL_INPUTS) {
        
        return digitalIn[number];
        
    } else {
        
        return false;
    }
}

/**
 * This method writes the actual digital output. It is usually called by a DigitalOut object.
 * @param number the index number of the digital output.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void HetronicRC::writeDigitalOut(uint16_t number, bool value) {
    
    if (number < NUMBER_OF_DIGITAL_OUTPUTS) {
        
        digitalOut[number] = value;
    }
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void HetronicRC::receiveObject(uint32_t functionCode, uint8_t object[]) {
    
    // process received TPDOs
    
    switch (functionCode) {
        
        case CANopen::TPDO1:
            
            for (uint16_t i = 0; i < NUMBER_OF_ANALOG_INPUTS; i++) analogIn[i] = static_cast<float>((static_cast<uint32_t>(object[i]) & 0xFF)-0x7F)/static_cast<float>(0x80-0x29)*2.0f;
            
            break;
            
        case CANopen::TPDO2:
            
            for (uint16_t i = 0; i < NUMBER_OF_DIGITAL_INPUTS; i++) digitalIn[i] = (object[i/8] & (1 << (i%8))) > 0;
            
            break;
            
        default:
            
            break;
    }
}

/**
 * This method is the handler of this device driver.
 */
void HetronicRC::run() {
    
    while (waitForNextPeriod()) {
        
        // transmit RPDO1 (used for heartbeat)
        
        for (uint16_t i = 0; i < 8; i++) rpdo1[i] = 0x00;
        canOpen.transmitObject(CANopen::RPDO1, nodeID, rpdo1);
        
        // transmit RPDO2
        
        for (uint16_t i = 0; i < 8; i++) rpdo2[i] = 0x00;
        for (uint16_t i = 0; i < NUMBER_OF_DIGITAL_OUTPUTS; i++) {
            if (!digitalOut[i]) rpdo2[i/8+1] = rpdo2[i/8+1] | (1 << (i%8));
        }
        canOpen.transmitObject(CANopen::RPDO2, nodeID, rpdo2);
    }
}
