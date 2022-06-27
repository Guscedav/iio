/*
 * RtelligentECR60.cpp
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 18.02.2022
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "RtelligentECR60.h"

using namespace std;

/**
 * Creates a <code>RtelligentECR60</code> device driver object.
 * @param etherCAT a reference to an EtherCAT object this device driver depends on.
 * @param coe a reference to a CoE object this device driver depends on.
 * @param deviceAddress the relative device address, i.e. 0x0000, 0xFFFF, 0xFFFE, etc.
 */
RtelligentECR60::RtelligentECR60(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress) : etherCAT(etherCAT), coe(coe) {

    // initialize local values

    enable = false;
    newSetpoint = false;
    controlword = 0x0000;
    statusword = 0x0000;
    modesOfOperation = PROFILE_POSITION_MODE;
    modesOfOperationDisplay = 0;
    targetPosition = 0;
    profileVelocity = 1000;
    profileAcceleration = 1000;
    profileDeceleration = 1000;
    positionActualValue = 0;
    digitalInputs = 0x00000000;

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
            cerr << "RtelligentECR60: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("RtelligentECR60: couldn't enter state INIT.");
    }

    Thread::sleep(10);

    // initialize SYNC managers

    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+0*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+0*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_OUT_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+0*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_OUT_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+0*EtherCAT::SYNC_MANAGER_OFFSET, 0x26);   // mailbox, write, interrupt in PDI
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+0*EtherCAT::SYNC_MANAGER_OFFSET, 1);
    Thread::sleep(10);

    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+1*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+1*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_IN_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+1*EtherCAT::SYNC_MANAGER_OFFSET, MAILBOX_IN_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+1*EtherCAT::SYNC_MANAGER_OFFSET, 0x22);   // mailbox, read, interrupt in PDI
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+1*EtherCAT::SYNC_MANAGER_OFFSET, 1);
    Thread::sleep(10);

    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+2*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+2*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_OUT_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+2*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_OUT_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+2*EtherCAT::SYNC_MANAGER_OFFSET, 0x64);   // buffered, write, interrupt in PDI
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+2*EtherCAT::SYNC_MANAGER_OFFSET, 1);
    Thread::sleep(10);

    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_ACTIVATE+3*EtherCAT::SYNC_MANAGER_OFFSET, 0);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER+3*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_IN_ADDRESS);
    Thread::sleep(10);
    etherCAT.write16(deviceAddress, EtherCAT::SYNC_MANAGER_LENGTH+3*EtherCAT::SYNC_MANAGER_OFFSET, BUFFERED_IN_SIZE);
    Thread::sleep(10);
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+3*EtherCAT::SYNC_MANAGER_OFFSET, 0x20);   // buffered, read, interrupt in PDI
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
            cerr << "RtelligentECR60: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("RtelligentECR60: couldn't enter state PRE OPERATIONAL.");
    }

    // register process datagram

    coe.registerDatagram(txPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APRD, deviceAddress, BUFFERED_IN_ADDRESS, BUFFERED_IN_SIZE));
    coe.registerDatagram(rxPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APWR, deviceAddress, BUFFERED_OUT_ADDRESS, BUFFERED_OUT_SIZE));

    coe.addSlaveDevice(this);

    // configure PDO communication

    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x00, 0, 1);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x01, 0x60400010, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x02, 0x607A0020, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x03, 0x60810020, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x04, 0x60830020, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x05, 0x60840020, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x06, 0x60600008, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1601, 0x00, 6, 1);

    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1A00, 0x00, 0, 1);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1A00, 0x01, 0x60410010, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1A00, 0x02, 0x60610008, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1A00, 0x03, 0x60640020, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1A00, 0x04, 0x60FD0020, 4);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1A00, 0x00, 4, 1);

    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C12, 0x00, 0, 1);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C12, 0x01, 0x1601, 2);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C12, 0x00, 1, 1);

    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C13, 0x00, 0, 1);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C13, 0x01, 0x1A00, 2);
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C13, 0x00, 1, 1);

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
            cerr << "RtelligentECR60: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("RtelligentECR60: couldn't enter state SAFE OPERATIONAL.");
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
            cerr << "RtelligentECR60: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("RtelligentECR60: couldn't enter state OPERATIONAL.");
    }
}

/**
 * Deletes the <code>RtelligentECR60</code> device driver object and releases all allocated resources.
 */
RtelligentECR60::~RtelligentECR60() {}

/**
* This method writes an analog output. It is usually called by an AnalogOut object.
* <br/>
* This device driver offers 4 analog outputs, to set the target position (output number 0), to set
* the profile velocity (number 1), to set the profile acceleration (number 2) and to set the profile
* deceleration (output number 3).
* @param number the index number of the analog output. This number must be 0, 1, 2 or 3.
* @param value the value of the analog output.
*/
void RtelligentECR60::writeAnalogOut(uint16_t number, float value) {

    if (number == 0) {

        targetPosition = static_cast<int32_t>(value);
        newSetpoint = true;

    } else if (number == 1) {

        profileVelocity = static_cast<uint32_t>(value);

    } else if (number == 2) {

        profileAcceleration = static_cast<uint32_t>(value);

    } else if (number == 3) {

        profileDeceleration = static_cast<uint32_t>(value);
    }
}

/**
 * This method reads a digital input. It is usually called by a DigitalIn object.
 * @param number the index number of the digital input. This number must be 0.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool RtelligentECR60::readDigitalIn(uint16_t number) {

    if (number == 0) {

        return (statusword & OPERATION_ENABLED_MASK) == OPERATION_ENABLED;

    } else {

        return false;
    }
}

/**
 * This method writes a digital output. It is usually called by a DigitalOut object.
 * @param number the index number of the digital output. This number must be 0.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void RtelligentECR60::writeDigitalOut(uint16_t number, bool value) {

    if (number == 0) {

        enable = value;
    }
}

/**
 * This method reads an encoder counter. It is usually called by an EncoderCounter object.
 * @param number the index number of the encoder counter. This number must be 0.
 * @return the value of the encoder counter.
 */
int32_t RtelligentECR60::readEncoderCounter(uint16_t number) {

    if (number == 0) {

        return positionActualValue;

    } else {

        return 0;
    }
}

/**
 * This method is called by the communication handler just before a new
 * EtherCAT frame is transmitted on the fieldbus. It allows this device
 * driver to update its datagram with output process data.
 */
void RtelligentECR60::writeDatagram() {

    mutex.lock();

    // set new controlword

    controlword = 0x0000;

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

    if (newSetpoint) {

        controlword |= NEW_SETPOINT;
        controlword |= CHANGE_SET_IMMEDIATELY;

        newSetpoint = false;
    }

    // write RxPDO

    rxPDO->data[10] = static_cast<uint8_t>(controlword & 0xFF);             // Control word
    rxPDO->data[11] = static_cast<uint8_t>((controlword >> 8) & 0xFF);
    rxPDO->data[12] = static_cast<uint8_t>(targetPosition & 0xFF);          // Target Position
    rxPDO->data[13] = static_cast<uint8_t>((targetPosition >> 8) & 0xFF);
    rxPDO->data[14] = static_cast<uint8_t>((targetPosition >> 16) & 0xFF);
    rxPDO->data[15] = static_cast<uint8_t>((targetPosition >> 24) & 0xFF);
    rxPDO->data[16] = static_cast<uint8_t>(profileVelocity & 0xFF);         // Profile Velocity
    rxPDO->data[17] = static_cast<uint8_t>((profileVelocity >> 8) & 0xFF);
    rxPDO->data[18] = static_cast<uint8_t>((profileVelocity >> 16) & 0xFF);
    rxPDO->data[19] = static_cast<uint8_t>((profileVelocity >> 24) & 0xFF);
    rxPDO->data[20] = static_cast<uint8_t>(profileAcceleration & 0xFF);         // Profile Acceleration
    rxPDO->data[21] = static_cast<uint8_t>((profileAcceleration >> 8) & 0xFF);
    rxPDO->data[22] = static_cast<uint8_t>((profileAcceleration >> 16) & 0xFF);
    rxPDO->data[23] = static_cast<uint8_t>((profileAcceleration >> 24) & 0xFF);
    rxPDO->data[24] = static_cast<uint8_t>(profileDeceleration & 0xFF);         // Profile Deceleration
    rxPDO->data[25] = static_cast<uint8_t>((profileDeceleration >> 8) & 0xFF);
    rxPDO->data[26] = static_cast<uint8_t>((profileDeceleration >> 16) & 0xFF);
    rxPDO->data[27] = static_cast<uint8_t>((profileDeceleration >> 24) & 0xFF);
    rxPDO->data[28] = static_cast<uint8_t>(modesOfOperation);                   // Modes of Operation

    mutex.unlock();
}

/**
 * This method is called by the communication handler just after a new
 * EtherCAT frame was received from the fieldbus. It allows this device
 * driver to read its datagram with input process data.
 */
void RtelligentECR60::readDatagram() {

    mutex.lock();

    statusword = (static_cast<uint16_t>(txPDO->data[10]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[11]) & 0xFF) << 8);
    modesOfOperationDisplay = static_cast<int8_t>(txPDO->data[12]);
    positionActualValue = (static_cast<int32_t>(txPDO->data[13]) & 0xFF) | ((static_cast<int32_t>(txPDO->data[14]) & 0xFF) << 8) | ((static_cast<int32_t>(txPDO->data[15]) & 0xFF) << 16) | ((static_cast<int32_t>(txPDO->data[16]) & 0xFF) << 24);
    digitalInputs = (static_cast<uint32_t>(txPDO->data[17]) & 0xFF) | ((static_cast<uint32_t>(txPDO->data[18]) & 0xFF) << 8) | ((static_cast<uint32_t>(txPDO->data[19]) & 0xFF) << 16) | ((static_cast<uint32_t>(txPDO->data[20]) & 0xFF) << 24);

    mutex.unlock();
}
