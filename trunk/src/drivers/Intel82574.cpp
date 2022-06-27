/*
 * Intel82574.cpp
 * Copyright (c) 2019, ZHAW
 * All rights reserved.
 *
 *  Created on: 10.01.2019
 *      Author: Marcel Honegger
 */

#include "PCI.h"
#include "Thread.h"
#include "Intel82574.h"

using namespace std;

const uint8_t Intel82574::MAC_ADDRESS[] = {0x00, 0x1b, 0x21, 0xd2, 0xd2, 0x70}; // ethernet address to use

/**
 * Creates a device driver object for the Intel 82574 Ethernet controller.
 * @param pci a reference to a PCI device driver this Ethernet driver depends on.
 * @param number the occurence number of this type of board on the PCI
 * bus. '0' is the first board of this type, '1' the second, and so on.
 * @param speed the baudrate of the Ethernet link in [Mbit/s], either 10, 100 or 1000.
 * @param loopback a flag to define if sent Ethernet frames should be looped back to the receiver.
 */
Intel82574::Intel82574(PCI& pci, uint16_t number, uint32_t speed, bool loopback) : pci(pci) {
    
    // get base address
    
    deviceHandle = pci.openDevice(VENDOR_ID, DEVICE_ID, number);
    baseAddress = pci.getBaseAddress(0, VENDOR_ID, DEVICE_ID, number);
    
    if (baseAddress == 0) throw invalid_argument("Intel82574: this board was not found on the PCI bus.");
    
    // initialize the Intel 82574 Ethernet controller

    uint32_t ctrl = pci.in32(baseAddress+CTRL);
    pci.out32(baseAddress+CTRL, ctrl | 0x84000000);   // reset the Intel 82574 and the PHY

    Thread::sleep(10);

    uint64_t macAddress = ((uint64_t)MAC_ADDRESS[0] << 0) | ((uint64_t)MAC_ADDRESS[1] << 8) | ((uint64_t)MAC_ADDRESS[2] << 16) | ((uint64_t)MAC_ADDRESS[3] << 24) | ((uint64_t)MAC_ADDRESS[4] << 32) | ((uint64_t)MAC_ADDRESS[5] << 40);
    pci.out32(baseAddress+RAL0, static_cast<uint32_t>(macAddress & 0xFFFFFFFF));                  // set 4 low bytes of ethernet address
    pci.out32(baseAddress+RAH0, static_cast<uint32_t>((macAddress >> 32) & 0xFFFF) | 0x80000000); // set 2 high bytes of ethernet address
    for (uint16_t i = 0; i < 128; i++) pci.out32(baseAddress+MTA+i*4, 0);                         // reset multicast table array
    pci.out32(baseAddress+IMS, 0);                                                                // reset interrupt mask bits
    pci.out32(baseAddress+IMC, 0xFFFFEBF7);

    // initialize receive buffers

    uint64_t receiveBufferPhysicalAddress;
    allocateDMA(RECEIVE_BUFFER_SIZE, receiveBufferAddress, receiveBufferPhysicalAddress);
    
    for (uint32_t i = 0; i < DESCRIPTOR_LENGTH; i++) {
        for (uint32_t j = 0; j < 4; j++) out8(receiveBufferAddress+i*16+j, static_cast<uint8_t>(((receiveBufferPhysicalAddress+1024+i*2048) >> (j*8)) & 0xFF));
        for (uint32_t j = 4; j < 8; j++) out8(receiveBufferAddress+i*16+j, static_cast<uint8_t>(0x00));
        for (uint32_t j = 8; j < 16; j++) out8(receiveBufferAddress+i*16+j, static_cast<uint8_t>(0x00));
    }
    
    pci.out32(baseAddress+RCTL, 0x0000801C);          // receive control register, disable reception
    
    pci.out32(baseAddress+RDBAL0, receiveBufferPhysicalAddress & 0xFFFFFFFF);
    pci.out32(baseAddress+RDBAH0, 0x00000000);
    pci.out32(baseAddress+RDLEN0, DESCRIPTOR_LENGTH*16);
    pci.out32(baseAddress+RDH0, 0x00000000);
    pci.out32(baseAddress+RDT0, DESCRIPTOR_LENGTH-1);
    pci.out32(baseAddress+RDTR, 0x00000000);          // disable packet timer
    pci.out32(baseAddress+RADV, 0x00000000);          // disable absolute delay timer
    pci.out32(baseAddress+RSRPD, 0x00000000);         // disable small packet detect interrupt
    
    if (loopback) {
        pci.out32(baseAddress+RCTL, 0x000082DE);      // receive control register, enable with loopback (00000000 00000000 10000010 11011110)
    } else {
        pci.out32(baseAddress+RCTL, 0x0000821E);      // receive control register, enable reception     (00000000 00000000 10000010 00011110)
    }

    // initialize transmit buffers

    uint64_t transmitBufferPhysicalAddress;
    allocateDMA(TRANSMIT_BUFFER_SIZE, transmitBufferAddress, transmitBufferPhysicalAddress);
    
    for (uint32_t i = 0; i < DESCRIPTOR_LENGTH; i++) {
        for (uint32_t j = 0; j < 4; j++) out8(transmitBufferAddress+i*16+j, static_cast<uint8_t>(((transmitBufferPhysicalAddress+1024+i*2048) >> (j*8)) & 0xFF));
        for (uint32_t j = 4; j < 8; j++) out8(transmitBufferAddress+i*16+j, static_cast<uint8_t>(0x00));
        for (uint32_t j = 8; j < 16; j++) out8(transmitBufferAddress+i*16+j, static_cast<uint8_t>(0x00));
        out8(transmitBufferAddress+i*16+11, 0x0B);
        out8(transmitBufferAddress+i*16+12, 0x00);
    }
    
    pci.out32(baseAddress+TCTL, 0x00040008);  // transmit control register, disable transmission (00000000 00000100 00000000 00001000)
    
    pci.out32(baseAddress+TDBAL, transmitBufferPhysicalAddress & 0xFFFFFFFF);
    pci.out32(baseAddress+TDBAH, 0x00000000);
    pci.out32(baseAddress+TDLEN, DESCRIPTOR_LENGTH*16);
    pci.out32(baseAddress+TDH, 0x00000000);
    pci.out32(baseAddress+TDT, 0x00000000);
    pci.out32(baseAddress+TIPG, (10 << 20) | (10 << 10) | 10);
    pci.out32(baseAddress+TXDCTL, (4) | (1 << 8) | (1 << 24) | (DESCRIPTOR_LENGTH << 25));
    
    pci.out32(baseAddress+TCTL, 0x0004000A);  // transmit control register, enable transmission (00000000 00000100 00000000 00001010)

    // set link up

    Thread::sleep(10);
    
    if (speed == 10) pci.out32(baseAddress+CTRL, (pci.in32(baseAddress+CTRL) & ~0x00001B49) | 0x00101849);
    else if (speed == 100) pci.out32(baseAddress+CTRL, (pci.in32(baseAddress+CTRL) & ~0x00001B49) | 0x00101949);
    else if (speed == 1000) pci.out32(baseAddress+CTRL, (pci.in32(baseAddress+CTRL) & ~0x00001B49) | 0x00101A49);
    else pci.out32(baseAddress+CTRL, (pci.in32(baseAddress+CTRL) & ~0x00001B49) | 0x00000349);
    
    uint32_t counter = 0;
    while (((pci.in32(baseAddress+STATUS) & 0x00000002) == 0) && (counter++ < LINK_UP_TIMEOUT)) Thread::sleep(1);
    
    if ((pci.in32(baseAddress+STATUS) & 0x00000002) == 0) cerr << "Intel82574: link not up!" << endl;
}

/**
 * Deletes the device driver object for the Intel 82574 Ethernet controller and releases all allocated resources.
 */
Intel82574::~Intel82574() {
    
    deallocateDMA(receiveBufferAddress);
    deallocateDMA(transmitBufferAddress);
}

/**
 * Returns if an ethernet link is present or not.
 * @return <code>false</code> if no ethernet link is present, <code>true</code> if an ethernet link is present.
 */
bool Intel82574::link() {
    
    return ((pci.in32(baseAddress+STATUS) & 0x00000002) != 0);
}

/**
 * Gives the ethernet MAC address of this hardware interface.
 * @param macAddress A 6 byte array to copy the ethernet MAC address into.
 */
void Intel82574::address(uint8_t macAddress[6]) {
    
    for (uint16_t i = 0; i < 6; i++) macAddress[i] = MAC_ADDRESS[i];
}

/**
 * Sends an ethernet frame to transmit on the network.
 * @param destinationMACAddress an array of 6 bytes representing the MAC address
 * of the destination of this frame.
 * @param etherType the ether type value of this frame, i.e. <code>ETHERTYPE_IPV4</code>
 * for an IPv4 packet.
 * @param data a buffer with the payload of this frame.
 * @param length the size of the given buffer pointed to by <code>data</code>, given in [bytes].
 * @return the number of bytes actually sent.
 */
uint16_t Intel82574::send(uint8_t destinationMACAddress[6], uint16_t etherType, uint8_t data[], uint16_t length) {
    
    uint32_t head = pci.in32(baseAddress+TDH);
    uint32_t tail = pci.in32(baseAddress+TDT);
    
    cout << "head=0x" << hex << head << " tail=0x" << tail << endl;

    if (head == ((tail == DESCRIPTOR_LENGTH-1) ? 0 : tail+1)) throw overflow_error("Intel82574: the transmit buffer is full.");
    
    for (uint16_t i = 0; i < 6; i++) out8(transmitBufferAddress+1024+tail*2048+i, destinationMACAddress[i]);
    for (uint16_t i = 0; i < 6; i++) out8(transmitBufferAddress+1024+tail*2048+6+i, MAC_ADDRESS[i]);
    out8(transmitBufferAddress+1024+tail*2048+12, (etherType >> 8) & 0xFF);
    out8(transmitBufferAddress+1024+tail*2048+13, etherType & 0xFF);
    memcpy((void*)(transmitBufferAddress+1024+tail*2048+14), (void*)data, length);
    
    length = (length < 46) ? 46 : (length > 1500) ? 1500 : length;
    
    out8(transmitBufferAddress+16*tail+8, (length+6+6+2) & 0xFF);
    out8(transmitBufferAddress+16*tail+9, ((length+6+6+2) >> 8) & 0xFF);
    out8(transmitBufferAddress+16*tail+11, 0xB);
    out8(transmitBufferAddress+16*tail+12, 0x0);
    
    pci.out32(baseAddress+TDT, (tail == DESCRIPTOR_LENGTH-1) ? 0 : tail+1);
    
    return length;
}

/**
 * Receives an ethernet frame from the network.
 * @param sourceMACAddress a buffer of 6 bytes to store the MAC address of the source of the frame.
 * @param etherType the ether type value of the received frame.
 * @param data a buffer to write the payload of the received frame into.
 * @param length the size of the given buffer pointed to by <code>data</code>.
 * @return the number of received bytes, or 0 if no frame was received.
 */
uint16_t Intel82574::receive(uint8_t sourceMACAddress[6], uint16_t& etherType, uint8_t data[], uint16_t length) {
    
    uint32_t head = pci.in32(baseAddress+RDH0);
    uint32_t tail = pci.in32(baseAddress+RDT0); tail = (tail == DESCRIPTOR_LENGTH-1) ? 0 : tail+1;
    uint8_t status = in8(receiveBufferAddress+12+tail*16);
    
    cout << "head=0x" << hex << head << " tail=0x" << tail << " status=0x" << static_cast<uint32_t>(status) << endl;

    if ((tail != head) && ((status & 0x01) > 0)) {
        
        out8(receiveBufferAddress+12+tail*16, 0x00);    // reset status byte in descriptor
        pci.out32(baseAddress+RDT0, tail);              // move tail pointer ahead
        
        uint16_t size = static_cast<uint16_t>(in8(receiveBufferAddress+8+tail*16)) | (static_cast<uint16_t>(in8(receiveBufferAddress+9+tail*16)) << 8);
        
        if (size >= 64) {
            
            for (uint16_t i = 0; i < 6; i++) sourceMACAddress[i] = in8(receiveBufferAddress+1024+tail*2048+6+i);
            etherType = (static_cast<uint16_t>(in8(receiveBufferAddress+1024+tail*2048+12)) << 8) | static_cast<uint16_t>(in8(receiveBufferAddress+1024+tail*2048+13));
            if (length > size-6-6-2-4) length = size-6-6-2-4;
            memcpy((void*)data, (void*)(receiveBufferAddress+1024+tail*2048+14), length);
            
            return length;
        }
    }
    
    return 0;
}

/**
 * Allocates a memory buffer for Ethernet frames.
 * @param size the size of the buffer to allocate, given in [bytes].
 * @param address a reference to the virtual address of the allocated buffer.
 * @param physicalAddress a reference to the physical address of the allocated buffer.
 */
void Intel82574::allocateDMA(uint32_t size, uint64_t& address, uint64_t& physicalAddress) {
    
    #if defined __QNX__
    
    address = reinterpret_cast<uint64_t>(mmap(0, (size_t)size, PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_PHYS|MAP_ANON, NOFD, 0));

    off64_t dmaAddress = 0;
    int32_t error = mem_offset64((void*)address, NOFD, 1, &dmaAddress, 0);

    if (error != 0) {
        cerr << "Intel82574: mem_offset=" << error << " errno=" << errno << endl;
        cerr << "            EACCES=" << EACCES << endl;
        cerr << "            EBADF=" << EBADF << endl;
        cerr << "            EINVAL=" << EINVAL << endl;
        cerr << "            ENODEV=" << ENODEV << endl;
        cerr << "            ENOSYS=" << ENOSYS << endl;
        cerr << "            EOVERFLOW=" << EOVERFLOW << endl;
    }

    physicalAddress = static_cast<uint64_t>(dmaAddress);
    
    #endif
}

/**
 * Releases the memory buffer for Ethernet frames.
 * @param address the virtual address of the buffer to release.
 */
void Intel82574::deallocateDMA(uint64_t address) {
    
    #if defined __QNX__
    
    munmap((void*)address, 1);
    
    #endif
}

/**
 * Gets a character at a given address.
 * @param address the address to read a character at.
 * @return the character at the given address.
 */
uint8_t Intel82574::in8(uint64_t address) {
    
    volatile uint8_t in = *(uint8_t*)address;
    
    return in;
}

/**
 * Gets a character at a given address.
 * @param address the address to read a character at.
 * @return the character at the given address.
 */
void Intel82574::out8(uint64_t address, uint8_t value) {
    
    *(uint8_t*)address = value;
}

/**
 * Utility function to read a PHY register.
 * @param address the PHY register address to read.
 * @return the value of the addressed PHY register.
 */
uint16_t Intel82574::readPHY(uint8_t address) {
    
    pci.out32(baseAddress+MDIC, static_cast<uint32_t>((1 << 27) | (1 << 21) | (address << 16)));
    
    Thread::sleep(1);
    
    while ((pci.in32(baseAddress+MDIC) & 0x10000000) != 0x10000000) Thread::sleep(1);
    
    return static_cast<uint16_t>(pci.in32(baseAddress+MDIC) & 0x0000FFFF);
}

/**
 * Utility function to write a PHY register.
 * @param address the PHY register address to write.
 * @param value the value to write to the addressed PHY register.
 */
void Intel82574::writePHY(uint8_t address, uint16_t value) {
    
    pci.out32(baseAddress+MDIC, static_cast<uint32_t>((1 << 26) | (1 << 21) | (address << 16) | value));
    
    Thread::sleep(1);
    
    while ((pci.in32(baseAddress+MDIC) & 0x10000000) != 0x10000000) Thread::sleep(1);
}
