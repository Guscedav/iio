/*
 * AdvantechPCIe1680.cpp
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 20.07.2017
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "PCI.h"
#include "AdvantechPCIe1680.h"

using namespace std;

const int32_t AdvantechPCIe1680::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-10;
const double AdvantechPCIe1680::PERIOD = 0.0005;
const uint16_t AdvantechPCIe1680::CHIP_OFFSET[] = {0x0000, 0x0400};

/**
 * Creates a CAN device driver object and initializes the CAN controller.
 * @param pci a reference to a PCI device driver this CAN driver depends on.
 * @param number the occurence number of this type of board on the PCI
 * bus. '0' is the first board of this type, '1' the second, and so on.
 * @param port the port number to use, either 0 or 1.
 */
AdvantechPCIe1680::AdvantechPCIe1680(PCI& pci, uint16_t number, uint16_t port) : RealtimeThread("AdvantechPCIe1680", STACK_SIZE, PRIORITY, PERIOD), pci(pci) {
    
    // get base address
    
    deviceHandle = pci.openDevice(VENDOR_ID, DEVICE_ID, number);
    baseAddress = pci.getBaseAddress(0, VENDOR_ID, DEVICE_ID, number);
    
    if (baseAddress == 0) throw invalid_argument("AdvantechPCIe1680: this board was not found on the PCI bus.");
    if ((port != 0) && (port != 1)) throw invalid_argument("AdvantechPCIe1680: invalid port number, expected 0 or 1.");
    
    baseAddress += CHIP_OFFSET[port];
    
    // initialize the SJA1000 CAN controller
    
    pci.out8(baseAddress+CR, 0x01);         // put the SJA1000 into reset mode
    
    do {
        Thread::sleep(1);
    } while ((pci.in8(baseAddress+CR) & 0x01) != 1);
    
    pci.out8(baseAddress+AC, 0x00);         // acceptance code
    pci.out8(baseAddress+AM, 0xFF);         // acceptace mask set to 'don't care'
    
    // set baudrate to 125 kbaud
    
    pci.out8(baseAddress+BTR0, 0x03);       // synch jump width = 1 TQ, baud rate prescaler value = 2*4
    pci.out8(baseAddress+BTR1, 0x2B);       // one sample per bit, time seg 2 = 3 TQ, time seg 1 = 12 TQ
    
    pci.out8(baseAddress+OC, 0xDA);         // put the SJA1000 to normal operation
    pci.out8(baseAddress+CDR, 0x48);        // basic can mode
    
    pci.out8(baseAddress+CR, 0x00);         // put the SJA1000 into operating mode
    
    do {
        Thread::sleep(1);
    } while ((pci.in8(baseAddress+CR) & 0x01) != 0);
    
    // start handler
    
    start();
}

/**
 * Deletes the CAN device driver object and releases all allocated resources.
 */
AdvantechPCIe1680::~AdvantechPCIe1680() {
    
    // stop handler
    
    stop();
    
    // release pci device resources
    
    pci.closeDevice(deviceHandle);
}

/**
 * Sets the frequency of the CAN bus, given in [Hz].
 * @param hz the frequency of the CAN bus, given in [Hz].
 * Posible values are: 2000000, 1000000, 800000, 500000, 250000, 125000, 100000, 50000 or 20000.
 * If other values are passed, the default frequency of 125000 Hz is used.
 */
void AdvantechPCIe1680::frequency(uint32_t hz) {

    // stop handler
    
    stop();
    
    // initialize the SJA1000 CAN controller
    
    pci.out8(baseAddress+CR, 0x01);         // put the SJA1000 into reset mode
    
    do {
        Thread::sleep(1);
    } while ((pci.in8(baseAddress+CR) & 0x01) != 1);
        
    if (hz == 2000000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 2*1
        pci.out8(baseAddress+BTR1, 0x01);   // one sample per bit, time seg 2 = 1 TQ, time seg 1 = 2 TQ
    } else if (hz == 1600000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 2*1
        pci.out8(baseAddress+BTR1, 0x11);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 2 TQ
    } else if (hz == 1000000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 2*1
        pci.out8(baseAddress+BTR1, 0x14);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 5 TQ
    } else if (hz == 800000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 2*1
        pci.out8(baseAddress+BTR1, 0x16);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 7 TQ
    } else if (hz == 500000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 2*1
        pci.out8(baseAddress+BTR1, 0x2B);   // one sample per bit, time seg 2 = 3 TQ, time seg 1 = 12 TQ
    } else if (hz == 250000) {
        pci.out8(baseAddress+BTR0, 0x01);   // synch jump width = 1 TQ, baud rate prescaler value = 2*2
        pci.out8(baseAddress+BTR1, 0x2B);   // one sample per bit, time seg 2 = 3 TQ, time seg 1 = 12 TQ
    } else if (hz == 100000) {
        pci.out8(baseAddress+BTR0, 0x04);   // synch jump width = 1 TQ, baud rate prescaler value = 2*5
        pci.out8(baseAddress+BTR1, 0x2B);   // one sample per bit, time seg 2 = 3 TQ, time seg 1 = 12 TQ
    } else if (hz == 50000) {
        pci.out8(baseAddress+BTR0, 0x09);   // synch jump width = 1 TQ, baud rate prescaler value = 2*10
        pci.out8(baseAddress+BTR1, 0x2B);   // one sample per bit, time seg 2 = 3 TQ, time seg 1 = 12 TQ
    } else if (hz == 20000) {
        pci.out8(baseAddress+BTR0, 0x0F);   // synch jump width = 1 TQ, baud rate prescaler value = 2*16
        pci.out8(baseAddress+BTR1, 0x7F);   // one sample per bit, time seg 2 = 8 TQ, time seg 1 = 16 TQ
    } else {
        pci.out8(baseAddress+BTR0, 0x03);   // synch jump width = 1 TQ, baud rate prescaler value = 2*4
        pci.out8(baseAddress+BTR1, 0x2B);   // one sample per bit, time seg 2 = 3 TQ, time seg 1 = 12 TQ
    }
    
    pci.out8(baseAddress+CR, 0x00);         // put the SJA1000 into operating mode
    
    do {
        Thread::sleep(1);
    } while ((pci.in8(baseAddress+CR) & 0x01) != 0);
    
    // start handler
    
    start();
}

/**
 * Writes a CAN message for transmission on the CAN bus.
 * This method stores a copy of the given CAN message in a software transmit buffer.
 * @param canMessage a CAN message object to transmit.
 * @return 0 if this write command failed, 1 otherwise.
 */
int32_t AdvantechPCIe1680::write(CANMessage canMessage) {
    
    if (messagesToTransmit.size() < BUFFER_SIZE) {
        
        mutex.lock();
        
        messagesToTransmit.push_back(canMessage);
        
        mutex.unlock();
        
        return 1;
        
    } else {
        
        return 0;
    }
}

/**
 * Reads a CAN message received from the CAN bus.
 * @param canMessage a reference to a CAN message object to overwrite.
 * @return 0 if no message was received, 1 if a message could be read successfully.
 */

int32_t AdvantechPCIe1680::read(CANMessage& canMessage) {
    
    if (receivedMessages.size() > 0) {
        
        mutex.lock();
        
        CANMessage canMessageOnStack = receivedMessages.front();
        canMessage.id = canMessageOnStack.id;
        for (uint8_t i = 0; i < 8; i++) canMessage.data[i] = canMessageOnStack.data[i];
        canMessage.len = canMessageOnStack.len;
        canMessage.type = canMessageOnStack.type;
        receivedMessages.pop_front();
        
        mutex.unlock();
        
        return 1;
        
    } else {
        
        return 0;
    }
}

/**
 * Transmits a given CAN message on the CAN bus.
 * @param canMessage the message to transmit.
 */
void AdvantechPCIe1680::transmit(CANMessage canMessage) {
    
    uint8_t length = canMessage.len;
    
    pci.out8(baseAddress+TXID0, (canMessage.id >> 3) & 0xFF);
    pci.out8(baseAddress+TXID1, (canMessage.id << 5) | ((canMessage.type == CANType::CANRemote) ? 0x10 : 0x00) | length);
    
    if (length > 0) pci.out8(baseAddress+TXDT0, canMessage.data[0]);
    if (length > 1) pci.out8(baseAddress+TXDT1, canMessage.data[1]);
    if (length > 2) pci.out8(baseAddress+TXDT2, canMessage.data[2]);
    if (length > 3) pci.out8(baseAddress+TXDT3, canMessage.data[3]);
    if (length > 4) pci.out8(baseAddress+TXDT4, canMessage.data[4]);
    if (length > 5) pci.out8(baseAddress+TXDT5, canMessage.data[5]);
    if (length > 6) pci.out8(baseAddress+TXDT6, canMessage.data[6]);
    if (length > 7) pci.out8(baseAddress+TXDT7, canMessage.data[7]);
    
    pci.out8(baseAddress+CMR, 0x01);    // set transmission request bit
}

/**
 * Reads a received CAN message from the CAN bus.
 * @param canMessage a reference to a CAN message object to overwrite.
 * @return 0 if no message was received, 1 if a message could be read successfully.
 */
int32_t AdvantechPCIe1680::receive(CANMessage& canMessage) {
    
    if ((pci.in8(baseAddress+SR) & 0x01) > 0) {    // receive buffer is full
        
        canMessage.id = (pci.in8(baseAddress+RXID0) << 3) | ((pci.in8(baseAddress+RXID1) >> 5) & 0x07);
        canMessage.type = (pci.in8(baseAddress+RXID1) & 0x10) > 0 ? CANType::CANRemote : CANType::CANData;
        canMessage.len = (canMessage.type == CANType::CANRemote) ? 0 : (pci.in8(baseAddress+RXID1) & 0x0F);
        
        if (canMessage.len > 0) canMessage.data[0] = pci.in8(baseAddress+RXDT0); else canMessage.data[0] = 0;
        if (canMessage.len > 1) canMessage.data[1] = pci.in8(baseAddress+RXDT1); else canMessage.data[1] = 0;
        if (canMessage.len > 2) canMessage.data[2] = pci.in8(baseAddress+RXDT2); else canMessage.data[2] = 0;
        if (canMessage.len > 3) canMessage.data[3] = pci.in8(baseAddress+RXDT3); else canMessage.data[3] = 0;
        if (canMessage.len > 4) canMessage.data[4] = pci.in8(baseAddress+RXDT4); else canMessage.data[4] = 0;
        if (canMessage.len > 5) canMessage.data[5] = pci.in8(baseAddress+RXDT5); else canMessage.data[5] = 0;
        if (canMessage.len > 6) canMessage.data[6] = pci.in8(baseAddress+RXDT6); else canMessage.data[6] = 0;
        if (canMessage.len > 7) canMessage.data[7] = pci.in8(baseAddress+RXDT7); else canMessage.data[7] = 0;
        
        pci.out8(baseAddress+CMR, 0x04);    // release the receive buffer
        
        return 1;
        
    } else {
        
        return 0;
    }
}

/**
 * This method is the handler of this CAN device driver.
 */
void AdvantechPCIe1680::run() {
    
    while (waitForNextPeriod()) {
        
        // tries to write a message from the software transmit buffer to the CAN bus
        
        if (messagesToTransmit.size() > 0) {
            
            if ((pci.in8(baseAddress+SR) & 0x04) != 0) {
                
                mutex.lock();
                
                CANMessage canMessage(messagesToTransmit.front());
				messagesToTransmit.pop_front();
                
                mutex.unlock();
                
                transmit(canMessage);
            }
        }
        
        // tries to read a messages from the hardware receive buffer
        
        CANMessage canMessage;
        int32_t received = receive(canMessage);
        
        while (received) {
            
            if (receivedMessages.size() < BUFFER_SIZE) {
            
                mutex.lock();
                
                receivedMessages.push_back(canMessage);
                
                mutex.unlock();
                
            } else {
                
                // software receive buffer is full, discard message
            }
            
            received = receive(canMessage);
        }
    }
}
