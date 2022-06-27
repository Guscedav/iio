/*
 * BeckhoffEL5101.cpp
 * Copyright (c) 2020, ZHAW
 * All rights reserved.
 *
 *  Created on: 09.01.2020
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "BeckhoffEL5101.h"

using namespace std;

/**
 * Creates a <code>BeckhoffEL5101</code> device driver object.
 * @param etherCAT a reference to an EtherCAT object this device driver depends on.
 * @param coe a reference to a CoE object this device driver depends on.
 * @param deviceAddress the relative device address, i.e. 0x0000, 0xFFFF, 0xFFFE, etc.
 */
BeckhoffEL5101::BeckhoffEL5101(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress) : etherCAT(etherCAT), coe(coe) {
    
    // initialize process data values
    
    status = 0;
    value = 0;
    latch = 0;
    
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
            cerr << "BeckhoffEL5101: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL5101: couldn't enter state INIT.");
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
    etherCAT.write8(deviceAddress, EtherCAT::SYNC_MANAGER_CONTROL+2*EtherCAT::SYNC_MANAGER_OFFSET, 0x24);    // buffered, write, interrupt in PDI
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
            cerr << "BeckhoffEL5101: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL5101: couldn't enter state PRE OPERATIONAL.");
    }
    
    // register process datagram
    
    coe.registerDatagram(rxPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APWR, deviceAddress, BUFFERED_OUT_ADDRESS, BUFFERED_OUT_SIZE));
    coe.registerDatagram(txPDO = new EtherCAT::Datagram(EtherCAT::COMMAND_APRD, deviceAddress, BUFFERED_IN_ADDRESS, BUFFERED_IN_SIZE));
    
    coe.addSlaveDevice(this);
    
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
            cerr << "BeckhoffEL5101: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL5101: couldn't enter state SAFE OPERATIONAL.");
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
            cerr << "BeckhoffEL5101: APPLICATION_LAYER_STATUS_CODE=0x" << hex << statusCode << endl;
        } catch (exception& e) {}
        throw runtime_error("BeckhoffEL5101: couldn't enter state OPERATIONAL.");
    }
}

/**
 * Deletes the <code>BeckhoffEL5101</code> device driver object and releases all allocated resources.
 */
BeckhoffEL5101::~BeckhoffEL5101() {}

/**
 * This method reads the current position value.
 * @return the current position given as an integer value.
 */
int16_t BeckhoffEL5101::readPosition() {

    int16_t position = 0;
    
    mutex.lock();
        
    position = static_cast<int16_t>(value);
        
    mutex.unlock();
    
    return position;
}

/**
 * This method is called by the communication handler just before a new
 * EtherCAT frame is transmitted on the fieldbus. It allows this device
 * driver to update its datagram with output process data.
 */
void BeckhoffEL5101::writeDatagram() {}

/**
 * This method is called by the communication handler just after a new
 * EtherCAT frame was received from the fieldbus. It allows this device
 * driver to read its datagram with input process data.
 */
void BeckhoffEL5101::readDatagram() {
    
    mutex.lock();
    
    status = txPDO->data[10];
    value = (static_cast<uint16_t>(txPDO->data[11]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[12]) & 0xFF) << 8);
    latch = (static_cast<uint16_t>(txPDO->data[13]) & 0xFF) | ((static_cast<uint16_t>(txPDO->data[14]) & 0xFF) << 8);
    
    mutex.unlock();
}
