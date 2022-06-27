/*
 * TPMC901.h
 * Copyright (c) 2022, ZHAW
 * All rights reserved.
 *
 *  Created on: 01.03.2022
 *      Author: Marcel Honegger
 */

#ifndef TPMC901_H_
#define TPMC901_H_

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
 * This class implements a specific CAN device driver for the Tews Datentechnik TPMC901 PCI Mezzanine Card.
 * The TPMC901 card features 6 Intel 82527 CAN bus controllers that allow to transmit and
 * receive CAN 2.0B compliant frames.
 * <div style="text-align:center"><img src="tpmc901.jpg" width="400"/></div>
 * <div style="text-align:center"><b>The Tews Datentechnik TPMC901</b></div>
 */
class TPMC901 : public CAN, public RealtimeThread {

    public:

                        TPMC901(PCI& pci, uint16_t number, uint16_t port);
        virtual         ~TPMC901();
        void            frequency(uint32_t hz);
        int32_t         write(CANMessage canMessage);
        int32_t         read(CANMessage& canMessage);

    private:

        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread polling for messages in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        static const double     PERIOD;                 // period of private thread in [s]

        static const uint16_t   VENDOR_ID = 0x10B5;     // vendor ID of this board (PLX Technology)
        static const uint16_t   DEVICE_ID = 0x9050;     // device ID of this board

        static const uint16_t   NUMBER_OF_PORTS = 6;    // the number of CAN ports on this board
        static const uint16_t   CHIP_OFFSET[];          // offset for the base address of the two can controllers

        static const uint16_t   CR = 0x00;              // control register
        static const uint16_t   SR = 0x01;              // status register
        static const uint16_t   CPUIR = 0x02;           // CPU interface register
        static const uint16_t   CLKOUT = 0x1F;          // clock out register
        static const uint16_t   GM0 = 0x06;             // global mask register 0
        static const uint16_t   GM1 = 0x07;             // global mask register 1
        static const uint16_t   M15M0 = 0x0C;           // message 15 mask register 0
        static const uint16_t   M15M1 = 0x0D;           // message 15 mask register 1
        static const uint16_t   M15M2 = 0x0E;           // message 15 mask register 2
        static const uint16_t   M15M3 = 0x0F;           // message 15 mask register 3
        static const uint16_t   BTR0 = 0x3F;            // bus timing register 0
        static const uint16_t   BTR1 = 0x4F;            // bus timing register 1
        static const uint16_t   MSG1 = 0x10;            // address of 1st message object
        static const uint16_t   MSG2 = 0x20;            // address of 2nd message object
        static const uint16_t   MSG3 = 0x30;            // address of 3rd message object
        static const uint16_t   MSG4 = 0x40;            // address of 4th message object
        static const uint16_t   MSG5 = 0x50;            // address of 5th message object
        static const uint16_t   MSG6 = 0x60;            // address of 6th message object
        static const uint16_t   MSG7 = 0x70;            // address of 7th message object
        static const uint16_t   MSG8 = 0x80;            // address of 8th message object
        static const uint16_t   MSG9 = 0x90;            // address of 9th message object
        static const uint16_t   MSG10 = 0xA0;           // address of 10th message object
        static const uint16_t   MSG11 = 0xB0;           // address of 11th message object
        static const uint16_t   MSG12 = 0xC0;           // address of 12th message object
        static const uint16_t   MSG13 = 0xD0;           // address of 13th message object
        static const uint16_t   MSG14 = 0xE0;           // address of 14th message object
        static const uint16_t   MSG15 = 0xF0;           // address of 15th message object

        static const uint16_t   BUFFER_SIZE = 128;      // size of the software transmit and receive buffers

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

#endif /* TPMC901_H_ */
