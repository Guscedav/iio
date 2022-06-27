/*
 * TPMC901.cpp
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 01.03.2022
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "PCI.h"
#include "TPMC901.h"

using namespace std;

const int32_t TPMC901::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-10;
const double TPMC901::PERIOD = 0.0005;
const uint16_t TPMC901::CHIP_OFFSET[] = {0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500};

/**
 * Creates a CAN device driver object and initializes the CAN controller.
 * @param pci a reference to a PCI device driver this CAN driver depends on.
 * @param number the occurence number of this type of board on the PCI
 * bus. '0' is the first board of this type, '1' the second, and so on.
 * @param port the port number to use, either 0, 1, 2, 3, 4 or 5.
 */
TPMC901::TPMC901(PCI& pci, uint16_t number, uint16_t port) : RealtimeThread("TPMC901", STACK_SIZE, PRIORITY, PERIOD), pci(pci) {

    // get base address

    deviceHandle = pci.openDevice(VENDOR_ID, DEVICE_ID, number);
    baseAddress = pci.getBaseAddress(2, VENDOR_ID, DEVICE_ID, number);

    if (baseAddress == 0) throw invalid_argument("TPMC901: this board was not found on the PCI bus.");
    if (port >= NUMBER_OF_PORTS) throw invalid_argument("TPMC901: invalid port number, expected 0, 1, 2, 3, 4 or 5.");
    
    for (uint16_t i = 0; i < NUMBER_OF_PORTS; i++) {
        
        pci.out8(baseAddress+CHIP_OFFSET[i]+CR, 0x40);      // change configuration enable
        pci.out8(baseAddress+CHIP_OFFSET[i]+CPUIR, 0x01);   // set clock dividers to 1
        pci.out8(baseAddress+CHIP_OFFSET[i]+CLKOUT, 0x20);  // CLOUT = 8 MHz (-> 1 tq = 125 ns)
    }
    
    baseAddress += CHIP_OFFSET[port];

    // initialize the i82527 CAN controller

    pci.out8(baseAddress+CR, 0x41);         // enable software initialization

    Thread::sleep(10);

    pci.out8(baseAddress+GM0, 0x00);        // set the global mask to 'don't care'
    pci.out8(baseAddress+GM1, 0x00);

    pci.out8(baseAddress+M15M0, 0x00);      // set the message 15 mask to 'don't care'
    pci.out8(baseAddress+M15M1, 0x00);
    pci.out8(baseAddress+M15M2, 0x00);
    pci.out8(baseAddress+M15M3, 0x00);

    // set baudrate to 125 kbaud
    
    pci.out8(baseAddress+BTR0, 0x03);       // synch jump width = 1 TQ, baud rate prescaler value = 4
    pci.out8(baseAddress+BTR1, 0x1C);       // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 13 TQ

    Thread::sleep(10);

    pci.out8(baseAddress+MSG2, 0x57);       // invalidate message objects
    pci.out8(baseAddress+MSG3, 0x57);
    pci.out8(baseAddress+MSG4, 0x57);
    pci.out8(baseAddress+MSG5, 0x57);
    pci.out8(baseAddress+MSG6, 0x57);
    pci.out8(baseAddress+MSG7, 0x57);
    pci.out8(baseAddress+MSG8, 0x57);
    pci.out8(baseAddress+MSG9, 0x57);
    pci.out8(baseAddress+MSG10, 0x57);
    pci.out8(baseAddress+MSG11, 0x57);
    pci.out8(baseAddress+MSG12, 0x57);
    pci.out8(baseAddress+MSG13, 0x57);
    pci.out8(baseAddress+MSG14, 0x57);

    pci.out8(baseAddress+MSG1, 0xB7);       // validate messages 1 & 15
    pci.out8(baseAddress+MSG15, 0xB7);

    pci.out8(baseAddress+MSG15+2, 0x00);    // reset can arbitration
    pci.out8(baseAddress+MSG15+3, 0x00);
    pci.out8(baseAddress+MSG15+4, 0x00);
    pci.out8(baseAddress+MSG15+5, 0x00);
    pci.out8(baseAddress+MSG15+6, 0x00);

    Thread::sleep(10);

    pci.out8(baseAddress+CR, 0x00);         // disable write access to configuration registers

    Thread::sleep(10);

    pci.out8(baseAddress+MSG1+2, (1 >> 3) & 0xFF);  // clear transmit buffer
    pci.out8(baseAddress+MSG1+3, (1 << 5) & 0xFF);
    pci.out8(baseAddress+MSG1+6, 1 << 4 | 8);
    pci.out8(baseAddress+MSG1+7, 0);
    pci.out8(baseAddress+MSG1+1, 0xE6);

    Thread::sleep(10);
    
    // start handler
    
    start();
}

/**
 * Deletes the CAN device driver object and releases all allocated resources.
 */
TPMC901::~TPMC901() {

    // stop handler

    stop();

    // release pci device resources

    pci.closeDevice(deviceHandle);
}

/**
 * Sets the frequency of the CAN bus, given in [Hz].
 * @param hz the frequency of the CAN bus, given in [Hz].
 * Posible values are: 1000000, 800000, 500000, 250000 or 125000.
 * If other values are passed, the default frequency of 125000 Hz is used.
 */
void TPMC901::frequency(uint32_t hz) {

    // stop handler

    stop();

    // initialize the i82527 CAN controller

    pci.out8(baseAddress+CR, 0x41);         // enable software initialization

    Thread::sleep(10);

    if (hz == 1000000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 1
        pci.out8(baseAddress+BTR1, 0x14);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 5 TQ
    } else if (hz == 800000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 1
        pci.out8(baseAddress+BTR1, 0x16);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 7 TQ
    } else if (hz == 500000) {
        pci.out8(baseAddress+BTR0, 0x00);   // synch jump width = 1 TQ, baud rate prescaler value = 1
        pci.out8(baseAddress+BTR1, 0x1C);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 13 TQ
    } else if (hz == 250000) {
        pci.out8(baseAddress+BTR0, 0x01);   // synch jump width = 1 TQ, baud rate prescaler value = 2
        pci.out8(baseAddress+BTR1, 0x1C);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 13 TQ
    } else {
        pci.out8(baseAddress+BTR0, 0x03);   // synch jump width = 1 TQ, baud rate prescaler value = 4
        pci.out8(baseAddress+BTR1, 0x1C);   // one sample per bit, time seg 2 = 2 TQ, time seg 1 = 13 TQ
    }

    Thread::sleep(10);

    pci.out8(baseAddress+CR, 0x00);         // disable write access to configuration registers

    // start handler

    start();
}

/**
 * Writes a CAN message for transmission on the CAN bus.
 * This method stores a copy of the given CAN message in a software transmit buffer.
 * @param canMessage a CAN message object to transmit.
 * @return 0 if this write command failed, 1 otherwise.
 */
int32_t TPMC901::write(CANMessage canMessage) {

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
int32_t TPMC901::read(CANMessage& canMessage) {

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
void TPMC901::transmit(CANMessage canMessage) {
    
    pci.out8(baseAddress+MSG1+2, (canMessage.id >> 3) & 0xFF);
    pci.out8(baseAddress+MSG1+3, (canMessage.id << 5) & 0xFF);
    
    uint8_t length = canMessage.len;
    pci.out8(baseAddress+MSG1+6, length << 4 | 8); // direction = transmit (receive for RTR: length << 4)

    for (uint8_t i = 0; i < length; i++) pci.out8(baseAddress+MSG1+7+i, canMessage.data[i]);

    pci.out8(baseAddress+MSG1+1, 0xE6); // set transmit request & NewDat bits
}

/**
 * Reads a received CAN message from the CAN bus.
 * @param canMessage a reference to a CAN message object to overwrite.
 * @return 0 if no message was received, 1 if a message could be read successfully.
 */
int32_t TPMC901::receive(CANMessage& canMessage) {

    if ((pci.in8(baseAddress+SR) & 0x10) > 0) { // receive buffer is full

        canMessage.id = (pci.in8(baseAddress+MSG15+2) << 3) | ((pci.in8(baseAddress+MSG15+3) >> 5) & 0x07);
        canMessage.type = CANType::CANData;
        canMessage.len = (pci.in8(baseAddress+MSG15+6) >> 4) & 0x0F;

        for (uint8_t i = 0; i < canMessage.len; i++) canMessage.data[i] = pci.in8(baseAddress+MSG15+7+i);

        pci.out8(baseAddress+MSG15, 0xFD);                              // reset IntPnd
        pci.out8(baseAddress+MSG15+1, 0x7D);                            // reset RmtPnd & NewDat bits
        pci.out8(baseAddress+SR, pci.in8(baseAddress+SR) & (~0x10));    // reset message received bit
        
        return 1;
        
    } else {

        return 0; // no message in receive buffer
    }
}

/**
 * This method is the handler of this CAN device driver.
 */
void TPMC901::run() {

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
