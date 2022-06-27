/*
 * Intel82574.h
 * Copyright (c) 2019, ZHAW
 * All rights reserved.
 *
 *  Created on: 10.01.2019
 *      Author: Marcel Honegger
 */

#ifndef INTEL_82574_H_
#define INTEL_82574_H_

#include <string>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cerrno>

#if defined __QNX__

#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#endif

#include "Ethernet.h"

using namespace std;

class PCI;

/**
 * This class implements a specific Ethernet device driver for PCI boards with an Intel 82574 Ethernet controller.
 * An example is the Intel Gigabit CT Desktop PCIe board depicted below.
 * <div style="text-align:center"><img src="intel82574.jpg" width="500" style="text-align:center"/></div>
 * <br/>
 * <div style="text-align:center"><b>An Intel Gigabit CT Desktop PCIe board</b></div>
 */
class Intel82574 : public Ethernet {
    
    public:
        
                    Intel82574(PCI& pci, uint16_t number, uint32_t speed, bool loopback);
        virtual     ~Intel82574();
        bool        link();
        void        address(uint8_t macAddress[6]);
        uint16_t    send(uint8_t destinationMACAddress[6], uint16_t etherType, uint8_t data[], uint16_t length);
        uint16_t    receive(uint8_t sourceMACAddress[6], uint16_t& etherType, uint8_t data[], uint16_t length);
        
    private:
        
        static const uint16_t   VENDOR_ID = 0x8086;     // vendor ID of this board
        static const uint16_t   DEVICE_ID = 0x10D3;     // device ID of this board
        
        static const uint8_t    MAC_ADDRESS[];                                          // ethernet address to use
        static const uint32_t   DESCRIPTOR_LENGTH = 8;                                  // length of receive and transmit descriptor ring buffer
        static const uint32_t   RECEIVE_BUFFER_SIZE = 1024+DESCRIPTOR_LENGTH*2048;      // size of the receive buffer, descriptors & data, in [bytes]
        static const uint32_t   TRANSMIT_BUFFER_SIZE = 1024+DESCRIPTOR_LENGTH*2048;     // size of the transmit buffer, descriptors & data, in [bytes]
        static const uint32_t   LINK_UP_TIMEOUT = 5000;                                 // time to wait until link is up, in [ms]
        
        static const uint32_t   CTRL = 0x0000;          // device control register
        static const uint32_t   STATUS = 0x0008;        // device status
        static const uint32_t   CTRL_EXT = 0x0018;      // extended device control register
        static const uint32_t   MDIC = 0x0020;          // management data interface control register
        static const uint32_t   IMS = 0x00D0;           // interrupt mask set/read register
        static const uint32_t   IMC = 0x00D8;           // interrupt mask clear register
        static const uint32_t   RCTL = 0x0100;          // receive control register
        static const uint32_t   TCTL = 0x0400;          // transmit control register
        static const uint32_t   TIPG = 0x0410;          // transmit inter packet gap register
        static const uint32_t   LEDCTL = 0x0E00;        // led control
        static const uint32_t   RDBAL0 = 0x2800;        // receive descriptor base address low
        static const uint32_t   RDBAH0 = 0x2804;        // receive descriptor base address high
        static const uint32_t   RDLEN0 = 0x2808;        // receive descriptor length
        static const uint32_t   RDH0 = 0x2810;          // receive descriptor head
        static const uint32_t   RDT0 = 0x2818;           // receive descriptor tail
        static const uint32_t   RDTR = 0x2820;          // receive delay timer register
        static const uint32_t   RADV = 0x282C;          // receive interrupt absolute delay timer
        static const uint32_t   RSRPD = 0x2C00;         // receive small packet detect interrupt;
        static const uint32_t   TDBAL = 0x3800;         // transmit descriptor base address low
        static const uint32_t   TDBAH = 0x3804;         // transmit descriptor base address high
        static const uint32_t   TDLEN = 0x3808;         // transmit descriptor length
        static const uint32_t   TDH = 0x3810;           // transmit descriptor head
        static const uint32_t   TDT = 0x3818;           // transmit descriptor tail
        static const uint32_t   TXDCTL = 0x3828;        // transmit descriptor control
        static const uint32_t   MTA = 0x5200;           // multicast table array
        static const uint32_t   RAL0 = 0x5400;          // receive address low
        static const uint32_t   RAH0 = 0x5404;          // receive address high
        
        PCI&        pci;                    // reference to the PCI driver
        void*       deviceHandle;           // pci handle of this board
        uint64_t    baseAddress;            // base address of this board
        uint64_t    receiveBufferAddress;   // address of the receive buffer
        uint64_t    transmitBufferAddress;  // address of the transmit buffer

        void        allocateDMA(uint32_t size, uint64_t& address, uint64_t& physicalAddress);
        void        deallocateDMA(uint64_t address);
        uint8_t     in8(uint64_t address);
        void        out8(uint64_t address, uint8_t value);
        uint16_t    readPHY(uint8_t address);
        void        writePHY(uint8_t address, uint16_t value);
};

#endif /* INTEL_82574_H_ */
