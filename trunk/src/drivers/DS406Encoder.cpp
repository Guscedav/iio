/*
 * DS406Encoder.cpp
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 15.10.2021
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "DS406Encoder.h"

using namespace std;

/**
 * Create a DS406Encoder device driver object and initialize the device and
 * the communication protocol.
 * @param canOpen a reference to a CANopen stack this device driver depends on.
 * @param nodeID the CANopen node ID of this device.
 * @param period the period of the communication cycle of this device, given in [s].
 */
DS406Encoder::DS406Encoder(CANopen& canOpen, uint32_t nodeID, double period) : canOpen(canOpen) {

    // set the stopped status
    /*
    uint8_t object0[] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    canOpen.transmitObject(CANopen::LSS_MASTER, 0, object0);

    // select device for LSS configuration state

    uint8_t object1[] = {0x40, 0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    //canOpen.transmitObject(CANopen::LSS_MASTER, 0, object1);

    uint8_t object2[] = {0x41, 0xE0, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00};
    //canOpen.transmitObject(CANopen::LSS_MASTER, 0, object2);

    uint8_t object3[] = {0x42, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    //canOpen.transmitObject(CANopen::LSS_MASTER, 0, object3);

    uint8_t object4[] = {0x43, 0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00};
    //canOpen.transmitObject(CANopen::LSS_MASTER, 0, object4);

    // select all devices for LSS configuration state

    uint8_t object5[] = {0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    canOpen.transmitObject(CANopen::LSS_MASTER, 0, object5);

    // set the nodeID

    uint8_t object6[] = {0x11, static_cast<uint8_t>(nodeID), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    canOpen.transmitObject(CANopen::LSS_MASTER, 0, object6);

    // store configuration data

    uint8_t object7[] = {0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    canOpen.transmitObject(CANopen::LSS_MASTER, 0, object7);
    */
    // initialize local values

    positionValue = 0;

    // register this device with the CANopen device driver

    canOpen.registerCANopenSlave(nodeID, this);

    // restore default parameters

    //canOpen.writeSDO(nodeID, 0x1011, 0x01, 0x64616F6C, 1);

    //Thread::sleep(1000);

    // set slave into preoperational state

    canOpen.transmitNMTObject(CANopen::ENTER_PREOPERATIONAL_STATE, nodeID);

    // disable communication with TPDO1

    canOpen.writeSDO(nodeID, 0x1800, 0x01, 0xC0000000+CANopen::TPDO1+nodeID, 4);

    // configure TPDO1

    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x00, 1);        // disable TPDO1
    canOpen.writeSDO(nodeID, 0x1A00, 0x01, 0x60200120, 4);  // position value channel 1, 4 bytes
    canOpen.writeSDO(nodeID, 0x1A00, 0x00, 0x01, 1);        // enable TPDO1

    // reconfigure communication with TPDO1

    canOpen.writeSDO(nodeID, 0x1800, 0x02, 254, 1);  // use event timer
    canOpen.writeSDO(nodeID, 0x1800, 0x03, static_cast<uint16_t>(period*10000/2+0.5), 2);
    canOpen.writeSDO(nodeID, 0x1800, 0x05, static_cast<uint16_t>(period*1000+0.5), 2);
    canOpen.writeSDO(nodeID, 0x1800, 0x01, 0x40000000+CANopen::TPDO1+nodeID, 4);

    // set device into operational state

    canOpen.transmitNMTObject(CANopen::START_REMOTE_NODE, nodeID);

    cout << "0x2000/0x00: " << dec << canOpen.readSDO(nodeID, 0x2000, 0x00) << endl;
    cout << "0x1800/0x01: 0x" << hex << canOpen.readSDO(nodeID, 0x1800, 0x01) << endl;
    cout << "0x1800/0x02: " << dec << canOpen.readSDO(nodeID, 0x1800, 0x02) << endl;
    cout << "0x1800/0x03: " << dec << canOpen.readSDO(nodeID, 0x1800, 0x03) << endl;
    cout << "0x1800/0x05: " << dec << canOpen.readSDO(nodeID, 0x1800, 0x05) << endl;
    cout << "0x1A00/0x00: 0x" << hex << canOpen.readSDO(nodeID, 0x1A00, 0x00) << endl;
    cout << "0x1A00/0x01: 0x" << hex << canOpen.readSDO(nodeID, 0x1A00, 0x01) << endl;

    cout << "0x1001/0x00: 0x" << hex << canOpen.readSDO(nodeID, 0x1001, 0x00) << endl;
    cout << "0x1002/0x00: 0x" << hex << canOpen.readSDO(nodeID, 0x1002, 0x00) << endl;
    cout << "0x6503/0x00: 0x" << hex << canOpen.readSDO(nodeID, 0x6503, 0x00) << endl;
    cout << "0x6505/0x00: 0x" << hex << canOpen.readSDO(nodeID, 0x6505, 0x00) << endl;

    // save all parameters

    //canOpen.writeSDO(nodeID, 0x1010, 0x01, 0x65766173, 4);
}

/**
 * Delete the DS406Encoder device driver object and release all allocated resources.
 */
DS406Encoder::~DS406Encoder() {}

/**
 * Read the encoder counter of this device.
 * This method is usually called by an EncoderCounter object.
 * @param number the index number of the encoder counter, this must be 0.
 * @return the position value, given in [counts].
 */
int32_t DS406Encoder::readEncoderCounter(uint16_t number) {

    return positionValue;
}

/**
 * Implements the interface of the CANopen delegate class to receive
 * CANopen messages targeted to this device driver.
 */
void DS406Encoder::receiveObject(uint32_t functionCode, uint8_t object[]) {

    if (functionCode == CANopen::TPDO1) {

        positionValue = (static_cast<int32_t>(object[0]) & 0xFF) | ((static_cast<int32_t>(object[1]) & 0xFF) << 8) | ((static_cast<int32_t>(object[2]) & 0xFF) << 16) | ((static_cast<int32_t>(object[3]) & 0xFF) << 24);
    }
}
