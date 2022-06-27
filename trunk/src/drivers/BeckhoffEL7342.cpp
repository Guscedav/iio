/*
 * BeckhoffEL7342.cpp
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 22.11.2021
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "BeckhoffEL7342.h"

using namespace std;

/**
 * Creates a <code>BeckhoffEL7342</code> device driver object.
 * @param etherCAT a reference to an EtherCAT object this device driver depends on.
 * @param coe a reference to a CoE object this device driver depends on.
 * @param deviceAddress the relative device address, i.e. 0x0000, 0xFFFF, 0xFFFE, etc.
 */
BeckhoffEL7342::BeckhoffEL7342(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress) : etherCAT(etherCAT), coe(coe) {

    // initialize output buffers

    controlChannel1 = 0x0000;
    controlChannel2 = 0x0000;
    velocityChannel1 = 0;
    velocityChannel2 = 0;

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
            cerr << "BeckhoffEL7342: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL7342: couldn't enter state INIT.");
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
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+2*EtherCAT::SYNC_MANAGER_OFFSET, 0x24);   // buffered, write
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
            cerr << "BeckhoffEL7342: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL7342: couldn't enter state PRE OPERATIONAL.");
    }

    // register process datagram

    coe.registerDatagram(txPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APRD, deviceAddress, BUFFERED_IN_ADDRESS, BUFFERED_IN_SIZE));
    coe.registerDatagram(rxPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APWR, deviceAddress, BUFFERED_OUT_ADDRESS, BUFFERED_OUT_SIZE));

    coe.addSlaveDevice(this);

    // configure PDO communication

    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C32, 0x01, 0, 2); // Sync mode: FreeRun
    coe.writeSDO(deviceAddress, MAILBOX_OUT_ADDRESS, MAILBOX_OUT_SIZE, MAILBOX_IN_ADDRESS, MAILBOX_IN_SIZE, 0x1C32, 0x02, 1000000, 4); // Cycle time
    
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
            cerr << "BeckhoffEL7342: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL7342: couldn't enter state SAFE OPERATIONAL.");
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
            cerr << "BeckhoffEL7342: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
            state = etherCAT.read16(deviceAddress, EtherCAT::APPLICATION_LAYER_STATUS);
            cerr << "BeckhoffEL7342: APPLICATION_LAYER_STATUS=0x" << hex << state << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL7342: couldn't enter state OPERATIONAL.");
    }
}

/**
 * Deletes the <code>BeckhoffEL7342</code> device driver object and releases all allocated resources.
 */
BeckhoffEL7342::~BeckhoffEL7342() {}

/**
* This method writes an analog output. It is usually called by an AnalogOut object.
* @param number the index number of the analog output.
* @param value the value of the analog output.
*/
void BeckhoffEL7342::writeAnalogOut(uint16_t number, float value) {

    if (value > 1.0f) value = 1.0f;
    else if (value < -1.0f) value = -1.0f;

    if (number == 0) {

        velocityChannel1 = static_cast<int16_t>(value*32767.0f);

    } else if (number == 1) {

        velocityChannel2 = static_cast<int16_t>(value*32767.0f);
    }
}

/**
 * This method writes a digital output. It is usually called by a DigitalOut object.
 * @param number the index number of the digital output.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void BeckhoffEL7342::writeDigitalOut(uint16_t number, bool value) {

    if (number == 0) {

        controlChannel1 = value ? 0x0001 : 0x0002;

    } else if (number == 1) {

        controlChannel2 = value ? 0x0001 : 0x0002;
    }
}

/**
 * This method reads an encoder counter. It is usually called by an EncoderCounter object.
 * @param number the index number of the encoder counter.
 * @return the value of the encoder counter.
 */
int32_t BeckhoffEL7342::readEncoderCounter(uint16_t number) {

    if (number == 0) {

        return positionChannel1;

    } else if (number == 1) {

        return positionChannel2;

    } else {

        return 0;
    }
}

/**
 * This method is called by the communication handler just before a new
 * EtherCAT frame is transmitted on the fieldbus. It allows this device
 * driver to update its datagram with output process data.
 */
void BeckhoffEL7342::writeDatagram() {

    mutex.lock();

    rxPDO->data[10] = 0x00; // ENC Outputs Ch.1
    rxPDO->data[11] = 0x00;
    rxPDO->data[12] = 0x00;
    rxPDO->data[13] = 0x00;
    rxPDO->data[14] = 0x00; // ENC Outputs Ch.2
    rxPDO->data[15] = 0x00;
    rxPDO->data[16] = 0x00;
    rxPDO->data[17] = 0x00;

    rxPDO->data[18] = static_cast<uint8_t>(controlChannel1 & 0xFF);
    rxPDO->data[19] = static_cast<uint8_t>((controlChannel1 >> 8) & 0xFF);
    rxPDO->data[20] = static_cast<uint8_t>(velocityChannel1 & 0xFF);
    rxPDO->data[21] = static_cast<uint8_t>((velocityChannel1 >> 8) & 0xFF);
    rxPDO->data[22] = static_cast<uint8_t>(controlChannel2 & 0xFF);
    rxPDO->data[23] = static_cast<uint8_t>((controlChannel2 >> 8) & 0xFF);
    rxPDO->data[24] = static_cast<uint8_t>(velocityChannel2 & 0xFF);
    rxPDO->data[25] = static_cast<uint8_t>((velocityChannel2 >> 8) & 0xFF);

    mutex.unlock();
}

/**
 * This method is called by the communication handler just after a new
 * EtherCAT frame was received from the fieldbus. It allows this device
 * driver to read its datagram with input process data.
 */
void BeckhoffEL7342::readDatagram() {

    mutex.lock();

    positionChannel1 = static_cast<int32_t>((static_cast<uint16_t>(txPDO->data[12]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[13]) & 0xFF) << 8));
    positionChannel2 = static_cast<int32_t>((static_cast<uint16_t>(txPDO->data[18]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[19]) & 0xFF) << 8));

    mutex.unlock();
}
