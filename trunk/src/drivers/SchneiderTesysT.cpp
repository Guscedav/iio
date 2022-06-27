/*
 * SchneiderTesysT.cpp
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.03.2022
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "SchneiderTesysT.h"

using namespace std;

const int32_t SchneiderTesysT::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-12;  // priority level of private thread

/**
 * Creates a Schneider Electric Tesys T device driver object and initializes the device and local values.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the handler thread of this driver, given in [s].
 */
SchneiderTesysT::SchneiderTesysT(CANopen& canOpen, uint32_t nodeID, double period) : RealtimeThread("SchneiderTesysT", STACK_SIZE, PRIORITY, period), canOpen(canOpen) {
    
    this->nodeID = nodeID;
    
    // register this device with the CANopen device driver
    
    canOpen.registerCANopenSlave(nodeID, this);
    
    // set device into preoperational state
    
    canOpen.transmitNMTObject(CANopen::ENTER_PREOPERATIONAL_STATE, nodeID);
    
    Thread::sleep(100);
    
    // configure TPDO 1
    
    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x00, 1);        // disable PDO
    canOpen.writeSDO(nodeID, 0x1A00, 0x01, 0x20040610, 4);  // system status register 1, 2 bytes, default configuration
    canOpen.writeSDO(nodeID, 0x1A00, 0x02, 0x20040710, 4);  // system status register 2, 2 bytes, default configuration
    canOpen.writeSDO(nodeID, 0x1A00, 0x03, 0x20040310, 4);  // fault register 1, 2 bytes, custom configuration
    canOpen.writeSDO(nodeID, 0x1A00, 0x04, 0x20040410, 4);  // fault register 2, 2 bytes, custom configuration
    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x04, 1);        // enable PDO
    
    // reconfigure communication with TPDO 1
    
    canOpen.writeSDO(nodeID, 0x1800, 0x01, CANopen::TPDO1+nodeID, 4);               // enable TPDO 1
    canOpen.writeSDO(nodeID, 0x1800, 0x03, static_cast<int32_t>(period*10000), 2);  // inhibit time in [0.1 ms]
    canOpen.writeSDO(nodeID, 0x1800, 0x05, static_cast<int32_t>(period*1000), 2);   // event timer in [0.1 ms]
    
    // configure TPDO 2
    
    canOpen.writeSDO(nodeID, 0x1A01, 0x00, 0x00, 1);        // disable PDO
    canOpen.writeSDO(nodeID, 0x1A01, 0x01, 0x20041B10, 4);  // average voltage, 2 bytes
    canOpen.writeSDO(nodeID, 0x1A01, 0x02, 0x20041C10, 4);  // L3-L1 voltage, 2 bytes
    canOpen.writeSDO(nodeID, 0x1A01, 0x03, 0x20041D10, 4);  // L1-L2 voltage, 2 bytes
    canOpen.writeSDO(nodeID, 0x1A01, 0x04, 0x20041E10, 4);  // L2-L3 voltage, 2 bytes
    canOpen.writeSDO(nodeID, 0x1A01, 0x00, 0x04, 1);        // enable PDO
    
    // reconfigure communication with TPDO 2
    
    canOpen.writeSDO(nodeID, 0x1801, 0x01, CANopen::TPDO2+nodeID, 4);               // enable TPDO 2
    canOpen.writeSDO(nodeID, 0x1801, 0x03, static_cast<int32_t>(period*10000), 2);  // inhibit time in [0.1 ms]
    canOpen.writeSDO(nodeID, 0x1801, 0x05, static_cast<int32_t>(period*1000), 2);   // event timer in [0.1 ms]
    
    // configure TPDO 3
    
    canOpen.writeSDO(nodeID, 0x1A02, 0x00, 0x00, 1);        // disable PDO
    canOpen.writeSDO(nodeID, 0x1A02, 0x01, 0x20043410, 4);  // average current, 2 lower bytes
    canOpen.writeSDO(nodeID, 0x1A02, 0x02, 0x20043610, 4);  // L1 current, 2 lower bytes
    canOpen.writeSDO(nodeID, 0x1A02, 0x03, 0x20043810, 4);  // L2 current, 2 lower bytes
    canOpen.writeSDO(nodeID, 0x1A02, 0x04, 0x20043A10, 4);  // L3 current, 2 lower bytes
    canOpen.writeSDO(nodeID, 0x1A02, 0x00, 0x04, 1);        // enable PDO
    
    // reconfigure communication with TPDO 3
    
    canOpen.writeSDO(nodeID, 0x1802, 0x01, CANopen::TPDO3+nodeID, 4);               // enable TPDO 3
    canOpen.writeSDO(nodeID, 0x1802, 0x03, static_cast<int32_t>(period*10000), 2);  // inhibit time in [0.1 ms]
    canOpen.writeSDO(nodeID, 0x1802, 0x05, static_cast<int32_t>(period*1000), 2);   // event timer in [0.1 ms]
    
    // configure device
    
    canOpen.writeSDO(nodeID, 0x2008, 0x05, 0x08, 2);        // fault reset command
    
    // configure producer heartbeat time
    
    canOpen.writeSDO(nodeID, 0x1017, 0x00, 200, 2);         // set producer heartbeat time to 200 ms
    
    // set device into operational state
    
    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);
    
    // start handler
    
    start();
}

/**
 * Deletes the SchneiderTesysT device driver object and releases all allocated resources.
 */
SchneiderTesysT::~SchneiderTesysT() {

    // stop handler
    
    stop();
}

/**
 * This method reads the actual analog input. It is usually called by an AnalogIn object.
 * @param number the index number of the analog input.
 * @return the value of the analog input.
 */
float SchneiderTesysT::readAnalogIn(uint16_t number) {

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
bool SchneiderTesysT::readDigitalIn(uint16_t number) {

    if (number < NUMBER_OF_DIGITAL_INPUTS) {
        
        return digitalIn[number];
        
    } else {
        
        return false;
    }
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void SchneiderTesysT::receiveObject(uint32_t functionCode, uint8_t object[]) {
    
    // process received TPDOs
    
    switch (functionCode) {
        
        case CANopen::TPDO1:
            
            for (uint16_t i = 0; i < NUMBER_OF_DIGITAL_INPUTS; i++) digitalIn[i] = (object[i/8] & (1 << (i%8))) > 0;
            
            break;
            
        case CANopen::TPDO2:
            
            analogIn[0] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[0]) | (static_cast<uint32_t>(object[1]) << 8)));
            analogIn[1] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[2]) | (static_cast<uint32_t>(object[3]) << 8)));
            analogIn[2] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[4]) | (static_cast<uint32_t>(object[5]) << 8)));
            analogIn[3] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[6]) | (static_cast<uint32_t>(object[7]) << 8)));
            
            break;
            
        case CANopen::TPDO3:
            
            analogIn[4] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[0]) | (static_cast<uint32_t>(object[1]) << 8)));
            analogIn[5] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[2]) | (static_cast<uint32_t>(object[3]) << 8)));
            analogIn[6] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[4]) | (static_cast<uint32_t>(object[5]) << 8)));
            analogIn[7] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[6]) | (static_cast<uint32_t>(object[7]) << 8)));
            
            break;
            
        default:
            
            break;
    }
}

/**
 * This method is the handler of this device driver.
 */
void SchneiderTesysT::run() {
    
    while (waitForNextPeriod()) {
        
    }
}
