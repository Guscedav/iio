/*
 * Mecca500.cpp
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 23.04.2021
 *      Author: Diego Molteni
 */

#include <cstring>
#include "Thread.h"
#include "Mecca500.h"

using namespace std;

float uint32_2_ieee_float(uint32_t value) {
    
    float result;
    memcpy(&result, &value, sizeof(float));
    
    return result;
}

uint32_t ieee_float_2_uint32(float value) {
    
    uint32_t result;
    memcpy(&result, &value, sizeof(float));
    
    return result;
}

/**
 * Creates a <code>Mecca500</code> device driver object.
 * @param etherCAT a reference to an EtherCAT object this device driver depends on.
 * @param coe a reference to a CoE object this device driver depends on.
 * @param deviceAddress the relative device address, i.e. 0x0000, 0xFFFF, 0xFFFE, etc.
 */
Mecca500::Mecca500(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress) : etherCAT(etherCAT), coe(coe) {
    
    // initialise local variables
    
    robotControl = 0;
    motionControl = 0;
    moveCommand = 0;
    for (uint16_t i = 0; i < 6; i++) moveArgument[i] = ieee_float_2_uint32(0.0f);
    
    robotStatus = 0;
    robotStatusError = 0;
    motionStatusCheckpoint = 0;
    motionStatusMoveID = 0;
    motionStatusFIFOspace = 0;
    motionStatus = 0;
    for (uint16_t i = 0; i < 6; i++) jointSet[i] = 0;
    for (uint16_t i = 0; i < 6; i++) endEffectorPose[i] = 0;
    
    // set EtherCAT state machine to state INIT
    
    uint16_t loop = 0;
    uint16_t state = 0;
    while ((state != EtherCAT::STATE_INIT) && (loop++ < 100)) try {
        state = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS);
        Thread::sleep(10);
        if ((state & EtherCAT::STATE_ERROR_MASK) == EtherCAT::STATE_ERROR) {
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state & EtherCAT::STATE_MASK);
            Thread::sleep(10);
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state);
        } else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_BOOTSTRAP) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_INIT);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_PRE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_INIT);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_SAFE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_PRE_OPERATIONAL);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_SAFE_OPERATIONAL);
        Thread::sleep(10);
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
    if (state != EtherCAT::STATE_INIT) {
        try {
            uint16_t statusCode = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS_CODE);
            cerr << "Mecca500: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("Mecca500: couldn't enter state INIT.");
    }
    
    Thread::sleep(10);
    
    // initialize SYNC managers
    
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+0*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+0*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_OUT_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+0*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_OUT_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+0*EtherCAT::SYNC_MANAGER_OFFSET, 0x26);    // mailbox, write, interrupt in PDI
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+0*EtherCAT::SYNC_MANAGER_OFFSET, 1);
    Thread::sleep(10);
    
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+1*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+1*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_IN_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+1*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_IN_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+1*EtherCAT::SYNC_MANAGER_OFFSET, 0x22);    // mailbox, read, interrupt in PDI
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+1*EtherCAT::SYNC_MANAGER_OFFSET, 1);
    Thread::sleep(10);
    
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+2*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+2*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_OUT_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+2*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_OUT_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+2*EtherCAT::SYNC_MANAGER_OFFSET, 0x64);    // buffered, write, interrupt in PDI
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+2*EtherCAT::SYNC_MANAGER_OFFSET, 1);
    Thread::sleep(10);
    
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+3*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+3*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_IN_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+3*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_IN_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+3*EtherCAT::SYNC_MANAGER_OFFSET, 0x20);    // buffered, read, interrupt in PDI
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+3*EtherCAT::SYNC_MANAGER_OFFSET, 1);
    Thread::sleep(10);
    
    // set EtherCAT state machine to state PRE OPERATIONAL
    
    loop = 0;
    state = 0;
    while ((state != EtherCAT::STATE_PRE_OPERATIONAL) && (loop++ < 100)) try {
        state = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS);
        Thread::sleep(10);
        if ((state & EtherCAT::STATE_ERROR_MASK) == EtherCAT::STATE_ERROR) {
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state & EtherCAT::STATE_MASK);
            Thread::sleep(10);
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state);
        } else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_INIT) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_PRE_OPERATIONAL);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_BOOTSTRAP) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_INIT);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_SAFE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_PRE_OPERATIONAL);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_SAFE_OPERATIONAL);
        Thread::sleep(10);
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
    if (state != EtherCAT::STATE_PRE_OPERATIONAL) {
        try {
            uint16_t statusCode = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS_CODE);
            cerr << "Mecca500: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("Mecca500: couldn't enter state PRE OPERATIONAL.");
    }
    
    // register process datagram
    
    coe.registerDatagram(rxPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APWR, deviceAddress, BUFFERED_OUT_ADDRESS, BUFFERED_OUT_SIZE));
    coe.registerDatagram(txPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APRD, deviceAddress, BUFFERED_IN_ADDRESS, BUFFERED_IN_SIZE));
    
    coe.addSlaveDevice(this);
    
    // configure PDOs
    
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C12, 0x00, 3, 1); // configure number of RxPDOs
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C13, 0x00, 4, 1); // configure number of TxPDOs
    
    // set EtherCAT state machine to state SAFE OPERATIONAL
    
    loop = 0;
    state = 0;
    while ((state != EtherCAT::STATE_SAFE_OPERATIONAL) && (loop++ < 100)) try {
        state = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS);
        Thread::sleep(10);
        if ((state & EtherCAT::STATE_ERROR_MASK) == EtherCAT::STATE_ERROR) {
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state & EtherCAT::STATE_MASK);
            Thread::sleep(10);
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state);
        } else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_INIT) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_PRE_OPERATIONAL);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_BOOTSTRAP) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_INIT);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_PRE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_SAFE_OPERATIONAL);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_SAFE_OPERATIONAL);
        Thread::sleep(10);
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
    if (state != EtherCAT::STATE_SAFE_OPERATIONAL) {
        try {
            uint16_t statusCode = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS_CODE);
            cerr << "Mecca500: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("Mecca500: couldn't enter state SAFE OPERATIONAL.");
    }
    
    // set EtherCAT state machine to state OPERATIONAL
    
    loop = 0;
    state = 0;
    while ((state != EtherCAT::STATE_OPERATIONAL) && (loop++ < 100)) try {
        state = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS);
        Thread::sleep(10);
        if ((state & EtherCAT::STATE_ERROR_MASK) == EtherCAT::STATE_ERROR) {
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state & EtherCAT::STATE_MASK);
            Thread::sleep(10);
            etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, state);
        } else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_INIT) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_PRE_OPERATIONAL);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_BOOTSTRAP) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_INIT);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_PRE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_SAFE_OPERATIONAL);
        else if ((state & EtherCAT::STATE_MASK) == EtherCAT::STATE_SAFE_OPERATIONAL) etherCAT.write16(deviceAddress, EtherCAT::APPLICATION_LAYER_CONTROL, EtherCAT::STATE_OPERATIONAL);
        Thread::sleep(10);
    } catch (exception& e) {
        cerr << e.what() << endl;
    }
    if (state != EtherCAT::STATE_OPERATIONAL) {
        try {
            uint16_t statusCode = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS_CODE);
            cerr << "Mecca500: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("Mecca500: couldn't enter state OPERATIONAL.");
    }
}

/**
 * Deletes the <code>Mecca500</code> device driver object and releases all allocated resources.
 */
Mecca500::~Mecca500() {}

/**
 * Reset the protective stop.
 */
void Mecca500::reset() {
    
    robotControl = 0x00;
    motionControl = 0x00080000; // setPoint = false, resetPStop = true;
    moveCommand = 0;
}

/**
 * Execute the homing sequence.
 */
void Mecca500::home() {
    
    robotControl = 0x06;
    motionControl = 0x00010000; // setPoint = true
    moveCommand = 0;
}

/**
 * Enable the drives of the robot.
 */
void Mecca500::enable() {
    
    robotControl = 0x02;
    motionControl = 0x00010000; // setPoint = true
    moveCommand = 21;           // MoveJointsVel: moveArguments are in °/s
}

/**
 * Disable the drives of the robot.
 */
void Mecca500::disable() {
    
    robotControl = 0x01;
    motionControl = 0x00000000; // setPoint = false
    moveCommand = 0;
}

/**
 * Sets the velocities of the joint angles.
 * @param jointVelocity an array of joint velocities, given in [rad/s].
 */
void Mecca500::setJointVelocity(const float jointVelocity[]) {
    
    for (uint16_t i = 0; i < 6; i++) {
        
        moveArgument[i] = ieee_float_2_uint32(jointVelocity[i]/3.14159265f*180.0f);
    }
}

/**
 * Checks if the robot is currently busy.
 * @return true if the robot is busy, false otherwise.
 */
bool Mecca500::isRobotBusy() {
    
    return robotStatus & 0x0001;
}

/**
 * Checks if the robot is already activated.
 * @return true if the robot is activated, false otherwise.
 */
bool Mecca500::isRobotActivated() {
    
    return robotStatus & 0x0002;
}

/**
 * Checks if the robot has been homed already.
 * @return true if the robot is homed, false otherwise.
 */
bool Mecca500::isRobotHomed() {
    
    return robotStatus & 0x0004;
}

/**
 * Gets the error number, if an error occured.
 * @return the actual error number.
 */
uint16_t Mecca500::getErrorNumber() {
    
    return robotStatusError;
}

/**
 * Gets the motion status word.
 * @return the actual motion status, see object 0x6015/0x04..0x08.
 */
uint32_t Mecca500::getMotionStatus() {
    
    return motionStatus;
}

/**
 * Gets the ID of the motion command.
 * @return the ID of the latest motion command.
 */
uint16_t Mecca500::getMoveID() {
    
    return motionStatusMoveID;
}

/**
 * Gets the actual joint angles.
 * @param jointAngles an array of joint angles to set. The values are given in [rad].
 */
void Mecca500::getJointAngles(float jointAngles[]) {
    
    for (uint16_t i = 0; i < 6; i++) {
        
        jointAngles[i] = uint32_2_ieee_float(jointSet[i])*3.14159265f/180.0f;
    }
}

/**
 * This method is called by the communication handler just before a new
 * EtherCAT frame is transmitted on the fieldbus. It allows this device
 * driver to update its datagram with output process data.
 */
void Mecca500::writeDatagram() {
    
    mutex.lock();
    
    rxPDO->data[10] = static_cast<uint8_t>(robotControl & 0xFF); // Robot Control, 0x7200/0x01..0x05, bits: deactivate, activate, home, reset error, sim mode
    rxPDO->data[11] = static_cast<uint8_t>((robotControl >> 8) & 0xFF);
    rxPDO->data[12] = static_cast<uint8_t>((robotControl >> 16) & 0xFF);
    rxPDO->data[13] = static_cast<uint8_t>((robotControl >> 24) & 0xFF);
    
    rxPDO->data[14] = static_cast<uint8_t>(motionControl & 0xFF); // Motion Control, 0x7310/0x01..0x05, bits: move ID (16 bits), set point, pause, clear move, reset pstop
    rxPDO->data[15] = static_cast<uint8_t>((motionControl >> 8) & 0xFF);
    rxPDO->data[16] = static_cast<uint8_t>((motionControl >> 16) & 0xFF);
    rxPDO->data[17] = static_cast<uint8_t>((motionControl >> 24) & 0xFF);
    
    rxPDO->data[18] = static_cast<uint8_t>(moveCommand & 0xFF); // Move Command, 0x7305/0x00
    rxPDO->data[19] = static_cast<uint8_t>((moveCommand >> 8) & 0xFF);
    rxPDO->data[20] = static_cast<uint8_t>((moveCommand >> 16) & 0xFF);
    rxPDO->data[21] = static_cast<uint8_t>((moveCommand >> 24) & 0xFF);
    
    for (uint16_t i = 0; i < 6; i++) {
        
        rxPDO->data[22+4*i] = static_cast<uint8_t>(moveArgument[i] & 0xFF); // Move Argument 1..6, 0x7306/0x01..0x06
        rxPDO->data[23+4*i] = static_cast<uint8_t>((moveArgument[i] >> 8) & 0xFF);
        rxPDO->data[24+4*i] = static_cast<uint8_t>((moveArgument[i] >> 16) & 0xFF);
        rxPDO->data[25+4*i] = static_cast<uint8_t>((moveArgument[i] >> 24) & 0xFF);
    }
    
    mutex.unlock();
}

/**
 * This method is called by the communication handler just after a new
 * EtherCAT frame was received from the fieldbus. It allows this device
 * driver to read its datagram with input process data.
 */
void Mecca500::readDatagram() {
    
    mutex.lock();
    
    robotStatus = (static_cast<uint16_t>(txPDO->data[10]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[11]) & 0xFF) << 8);
    robotStatusError = (static_cast<uint16_t>(txPDO->data[12]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[13]) & 0xFF) << 8);
    motionStatusCheckpoint = (static_cast<uint32_t>(txPDO->data[14]) & 0xFF) | ((static_cast<uint32_t>(txPDO->data[15]) & 0xFF) << 8) | ((static_cast<uint32_t>(txPDO->data[16]) & 0xFF) << 16) | ((static_cast<uint32_t>(txPDO->data[17]) & 0xFF) << 24);
    motionStatusMoveID = (static_cast<uint16_t>(txPDO->data[18]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[19]) & 0xFF) << 8);
    motionStatusFIFOspace = (static_cast<uint16_t>(txPDO->data[20]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[21]) & 0xFF) << 8);
    motionStatus = (static_cast<uint32_t>(txPDO->data[22]) & 0xFF) | ((static_cast<uint32_t>(txPDO->data[23]) & 0xFF) << 8) | ((static_cast<uint32_t>(txPDO->data[24]) & 0xFF) << 16) | ((static_cast<uint32_t>(txPDO->data[25]) & 0xFF) << 24);
    
    for (uint16_t i = 0; i < 6; i++) {
        
        jointSet[i] = (static_cast<uint32_t>(txPDO->data[26+4*i]) & 0xFF) | ((static_cast<uint32_t>(txPDO->data[27+4*i]) & 0xFF) << 8) | ((static_cast<uint32_t>(txPDO->data[28+4*i]) & 0xFF) << 16) | ((static_cast<uint32_t>(txPDO->data[29+4*i]) & 0xFF) << 24);
    }
    
    for (uint16_t i = 0; i < 6; i++) {
        
        endEffectorPose[i] = (static_cast<uint32_t>(txPDO->data[50+4*i]) & 0xFF) | ((static_cast<uint32_t>(txPDO->data[51+4*i]) & 0xFF) << 8) | ((static_cast<uint32_t>(txPDO->data[52+4*i]) & 0xFF) << 16) | ((static_cast<uint32_t>(txPDO->data[53+4*i]) & 0xFF) << 24);
    }
    
    mutex.unlock();
}
