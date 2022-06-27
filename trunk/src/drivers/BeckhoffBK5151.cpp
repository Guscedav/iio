/*
 * BeckhoffBK5151.cpp
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.11.2015
 *      Author: Marcel Honegger
 */

#include "BeckhoffBK5151.h"

using namespace std;

const int32_t BeckhoffBK5151::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-12;    // priority level of private thread

/**
 * Creates a BeckhoffBK5151 device driver object and initializes the device and local values.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the handler thread of this driver, given in [s].
 * @param heartbeat the period of the heartbeat messages from the master to the slave,
 * and from the slave to the master, given in [s]. A value of 0.0 disables the heartbeat.
 */
BeckhoffBK5151::BeckhoffBK5151(CANopen& canOpen, uint32_t nodeID, double period, double heartbeat) : RealtimeThread("BeckhoffBK5151", STACK_SIZE, PRIORITY, period), canOpen(canOpen) {
    
    this->nodeID = nodeID;
    
    // register this device with the CANopen device driver
    
    canOpen.registerCANopenSlave(nodeID, this);
    
    // set device into preoperational state
    
    canOpen.transmitNMTObject(CANopen::ENTER_PREOPERATIONAL_STATE, nodeID);
    
    // enable analog sensors
    
    try {
        
        canOpen.writeSDO(nodeID, 0x6423, 0x00, 1, 1);
        
    } catch (exception& e) {
        
        cerr << "BeckhoffBK5151: " << e.what() << endl;
	}
    
    // get number of channels
    
    numberOfAnalogInputs = canOpen.readSDO(nodeID, 0x6401, 0x00);
    numberOfAnalogOutputs = canOpen.readSDO(nodeID, 0x6411, 0x00);
    numberOfDigitalInputs = canOpen.readSDO(nodeID, 0x6000, 0x00)*8;
    numberOfDigitalOutputs = canOpen.readSDO(nodeID, 0x6200, 0x00)*8;
    
    // reconfigure communication with TPDOs
    
    if (numberOfDigitalInputs > 0) {
        canOpen.writeSDO(nodeID, 0x1800, 0x01, 0x80000000, 4);
        canOpen.writeSDO(nodeID, 0x1800, 0x02, 0xFF, 1);
        canOpen.writeSDO(nodeID, 0x1800, 0x03, static_cast<int32_t>(period*10000), 2);
        canOpen.writeSDO(nodeID, 0x1800, 0x05, static_cast<int32_t>(period*1000), 2);
        canOpen.writeSDO(nodeID, 0x1800, 0x01, CANopen::TPDO1+nodeID, 4);
    }
    if (numberOfAnalogInputs > 0) {
        canOpen.writeSDO(nodeID, 0x1801, 0x01, 0x80000000, 4);
        canOpen.writeSDO(nodeID, 0x1801, 0x02, 0xFF, 1);
        canOpen.writeSDO(nodeID, 0x1801, 0x03, static_cast<int32_t>(period*10000), 2);
        canOpen.writeSDO(nodeID, 0x1801, 0x05, static_cast<int32_t>(period*1000), 2);
        canOpen.writeSDO(nodeID, 0x1801, 0x01, CANopen::TPDO2+nodeID, 4);
    }
    if (numberOfAnalogInputs > 4) {
        canOpen.writeSDO(nodeID, 0x1802, 0x01, 0x80000000, 4);
        canOpen.writeSDO(nodeID, 0x1802, 0x02, 0xFF, 1);
        canOpen.writeSDO(nodeID, 0x1802, 0x03, static_cast<int32_t>(period*10000), 2);
        canOpen.writeSDO(nodeID, 0x1802, 0x05, static_cast<int32_t>(period*1000), 2);
        canOpen.writeSDO(nodeID, 0x1802, 0x01, CANopen::TPDO3+nodeID, 4);
    }
    if (numberOfAnalogInputs > 8) {
        canOpen.writeSDO(nodeID, 0x1803, 0x01, 0x80000000, 4);
        canOpen.writeSDO(nodeID, 0x1803, 0x02, 0xFF, 1);
        canOpen.writeSDO(nodeID, 0x1803, 0x03, static_cast<int32_t>(period*10000), 2);
        canOpen.writeSDO(nodeID, 0x1803, 0x05, static_cast<int32_t>(period*1000), 2);
        canOpen.writeSDO(nodeID, 0x1803, 0x01, CANopen::TPDO4+nodeID, 4);
    }
    
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
    
    // initialize private values
    
    for (uint16_t i = 0; i < MAX_NUMBER_OF_ANALOG_INPUTS; i++) analogIn[i] = 0.0f;
    for (uint16_t i = 0; i < MAX_NUMBER_OF_ANALOG_OUTPUTS; i++) analogOut[i] = 0.0f;
    for (uint16_t i = 0; i < MAX_NUMBER_OF_DIGITAL_INPUTS; i++) digitalIn[i] = false;
    for (uint16_t i = 0; i < MAX_NUMBER_OF_DIGITAL_OUTPUTS; i++) digitalOut[i] = false;
    
    // set device into operational state
    
    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);
    
    // start handler
    
    start();
}

/**
 * Deletes the BeckhoffBK5151 device driver object and releases all allocated resources.
 */
BeckhoffBK5151::~BeckhoffBK5151() {

    // stop handler
    
    stop();
}

/**
 * This method reads the actual analog input. It is usually called by an AnalogIn object.
 * @param number the index number of the analog input.
 * @return the value of the analog input.
 */
float BeckhoffBK5151::readAnalogIn(uint16_t number) {

    return analogIn[number];
}

/**
 * This method writes the actual analog output. It is usually called by an AnalogOut object.
 * @param number the index number of the analog output.
 * @param value the value of the analog output.
 */
void BeckhoffBK5151::writeAnalogOut(uint16_t number, float value) {
    
    analogOut[number] = value;
}

/**
 * This method reads the actual digital input. It is usually called by a DigitalIn object.
 * @param number the index number of the digital input.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool BeckhoffBK5151::readDigitalIn(uint16_t number) {

    return digitalIn[number];
}

/**
 * This method writes the actual digital output. It is usually called by a DigitalOut object.
 * @param number the index number of the digital output.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void BeckhoffBK5151::writeDigitalOut(uint16_t number, bool value) {
    
    digitalOut[number] = value;
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void BeckhoffBK5151::receiveObject(uint32_t functionCode, uint8_t object[]) {
    
    // process received TPDOs
    
    switch (functionCode) {
        
        case CANopen::TPDO1:
            
            for (uint16_t i = 0; i < MAX_NUMBER_OF_DIGITAL_INPUTS; i++) digitalIn[i] = (object[i/8] & (1 << (i%8))) > 0;
            
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
            
        case CANopen::TPDO4:
            
            analogIn[8] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[0]) | (static_cast<uint32_t>(object[1]) << 8)));
            analogIn[9] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[2]) | (static_cast<uint32_t>(object[3]) << 8)));
            analogIn[10] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[4]) | (static_cast<uint32_t>(object[5]) << 8)));
            analogIn[11] = static_cast<float>(static_cast<int16_t>(static_cast<uint32_t>(object[6]) | (static_cast<uint32_t>(object[7]) << 8)));
            
            break;
            
        default:
            
            break;
    }
}

/**
 * This method is the handler of this BeckhoffBK5151 device driver.
 */
void BeckhoffBK5151::run() {
    
    while (waitForNextPeriod()) {
        
        // transmit RPDO1
        
        if (numberOfDigitalOutputs > 0) {
            for (uint16_t i = 0; i < MAX_NUMBER_OF_DIGITAL_OUTPUTS; i++) {
                if (digitalOut[i]) rpdo1[i/8] = static_cast<uint8_t>(rpdo1[i/8] | (1 << (i%8)));
                else rpdo1[i/8] = static_cast<uint8_t>(rpdo1[i/8] & ~(1 << (i%8)));
            }
            canOpen.transmitObject(CANopen::RPDO1, nodeID, rpdo1);
		}
        
        // transmit RPDO2
        
        if (numberOfAnalogOutputs > 0) {
            for (uint16_t i = 0; i < 4; i++) {
                float value = analogOut[i];
                value = (value < -32760.0f) ? -32760.0f : (value > 32760.0f) ? 32760.0f : value;
                rpdo2[0+2*(i%4)] = static_cast<uint8_t>(static_cast<int16_t>(value) & 0xFF);
                rpdo2[1+2*(i%4)] = static_cast<uint8_t>((static_cast<int16_t>(value) >> 8) & 0xFF);
            }
            canOpen.transmitObject(CANopen::RPDO2, nodeID, rpdo2);
        }
        
        // transmit RPDO3
        
        if (numberOfAnalogOutputs > 4) {
            for (uint16_t i = 4; i < 8; i++) {
                float value = analogOut[i];
                value = (value < -32760.0f) ? -32760.0f : (value > 32760.0f) ? 32760.0f : value;
                rpdo3[0+2*(i%4)] = static_cast<uint8_t>(static_cast<int16_t>(value) & 0xFF);
                rpdo3[1+2*(i%4)] = static_cast<uint8_t>((static_cast<int16_t>(value) >> 8) & 0xFF);
            }
            canOpen.transmitObject(CANopen::RPDO3, nodeID, rpdo3);
        }
        
        // transmit RPDO4
        
        if (numberOfAnalogOutputs > 8) {
            for (uint16_t i = 8; i < 12; i++) {
                float value = analogOut[i];
                value = (value < -32760.0f) ? -32760.0f : (value > 32760.0f) ? 32760.0f : value;
                rpdo4[0+2*(i%4)] = static_cast<uint8_t>(static_cast<int16_t>(value) & 0xFF);
                rpdo4[1+2*(i%4)] = static_cast<uint8_t>((static_cast<int16_t>(value) >> 8) & 0xFF);
            }
            canOpen.transmitObject(CANopen::RPDO4, nodeID, rpdo4);
        }
    }
}
