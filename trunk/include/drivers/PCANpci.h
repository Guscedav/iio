/*
 * PCANpci.h
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 30.10.2015
 *      Author: Marcel Honegger
 */

#ifndef PCAN_PCI_H_
#define PCAN_PCI_H_

#include <cstdlib>
#include <stdexcept>
#include <deque>
#include <stdint.h>
#include "Mutex.h"
#include "CAN.h"
#include "CANMessage.h"
#include "RealtimeThread.h"

class PCI;

/**
 * This class implements a specific CAN device driver for the Peak System PCAN-PCIexpress interface board.
 * The PCAN-PCIexpress board features two SJA1000 CAN bus controllers that allow to transmit and receive
 * CAN 2.0B compliant frames. They offer a receive buffer with a size of 64 bytes.
 * <div style="text-align:center"><img src="pcanpci.jpg" width="500"/></div>
 * <div style="text-align:center"><b>The Peak PCAN-PCIexpress CAN interface board</b></div>
 */
class PCANpci : public CAN, public RealtimeThread {
    
    public:
        
                        PCANpci(PCI& pci, uint16_t number, uint16_t port);
        virtual         ~PCANpci();
        void            frequency(uint32_t hz);
        int32_t         write(CANMessage canMessage);
        int32_t         read(CANMessage& canMessage);
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread polling for messages in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        static const double     PERIOD;                 // period of private thread in [s]
        
        static const uint16_t   VENDOR_ID = 0x001C;     // vendor ID of this board
        static const uint16_t   DEVICE_ID = 0x0003;     // device ID of this board
        
        static const uint16_t   CHIP_OFFSET[];          // offset for the base address of the two can controllers
        
        static const uint16_t   CR = 0x00;              // control register
        static const uint16_t   CMR = 0x04;             // command register
        static const uint16_t   SR = 0x08;              // status register
        static const uint16_t   IR = 0x0C;              // interrupt register
        static const uint16_t   AC = 0x10;              // acceptance code register
        static const uint16_t   AM = 0x14;              // acceptance mask register
        static const uint16_t   BTR0 = 0x18;            // bus timing register 0
        static const uint16_t   BTR1 = 0x1C;            // bus timing register 1
        static const uint16_t   OC = 0x20;              // output control register
        static const uint16_t   TXID0 = 0x28;           // transmit buffer identifier 0
        static const uint16_t   TXID1 = 0x2C;           // transmit buffer identifier 1
        static const uint16_t   TXDT0 = 0x30;           // transmit buffer data byte 1
        static const uint16_t   TXDT1 = 0x34;           // transmit buffer data byte 2
        static const uint16_t   TXDT2 = 0x38;           // transmit buffer data byte 3
        static const uint16_t   TXDT3 = 0x3C;           // transmit buffer data byte 4
        static const uint16_t   TXDT4 = 0x40;           // transmit buffer data byte 5
        static const uint16_t   TXDT5 = 0x44;           // transmit buffer data byte 6
        static const uint16_t   TXDT6 = 0x48;           // transmit buffer data byte 7
        static const uint16_t   TXDT7 = 0x4C;           // transmit buffer data byte 8
        static const uint16_t   RXID0 = 0x50;           // receive buffer identifier 0
        static const uint16_t   RXID1 = 0x54;           // receive buffer identifier 1
        static const uint16_t   RXDT0 = 0x58;           // receive buffer data byte 1
        static const uint16_t   RXDT1 = 0x5C;           // receive buffer data byte 2
        static const uint16_t   RXDT2 = 0x60;           // receive buffer data byte 3
        static const uint16_t   RXDT3 = 0x64;           // receive buffer data byte 4
        static const uint16_t   RXDT4 = 0x68;           // receive buffer data byte 5
        static const uint16_t   RXDT5 = 0x6C;           // receive buffer data byte 6
        static const uint16_t   RXDT6 = 0x70;           // receive buffer data byte 7
        static const uint16_t   RXDT7 = 0x74;           // receive buffer data byte 8
        static const uint16_t   CDR = 0x7C;             // clock divider register
        
        static const uint16_t   BUFFER_SIZE = 64;       // size of the software transmit and receive buffers
        
        PCI&                    pci;                    // reference to the PCI driver
        void*                   deviceHandle;           // pci handle of this board
        uint64_t                baseAddress;            // base address of this board
        std::deque<CANMessage>  messagesToTransmit;     // buffer with CAN messages to transmit
        std::deque<CANMessage>  receivedMessages;       // buffer with received CAN messages
        Mutex                   mutex;                  // mutex to lock critical sections
        
        void            transmit(CANMessage canMessage);
        int32_t         receive(CANMessage& canMessage);
        void            run();
};

#endif /* PCAN_PCI_H_ */
