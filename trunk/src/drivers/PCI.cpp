/*
 * PCI.cpp
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 29.10.2015
 *      Author: Marcel Honegger
 */

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <stdexcept>

#include "PCI.h"

using namespace std;

/**
 * Creates a PCI object.
 */
PCI::PCI() {
    
    #if defined __QNX__
    
    ThreadCtl(_NTO_TCTL_IO, 0);
    
    #if _NTO_VERSION >= 700
    
    pciHandle = 0;
    
    #else
    
    pciHandle = pci_attach(0);
    
    #endif
    
    #else
    
    iopl(3);
    
    #endif
}

/**
 * Deletes the PCI object and releases all allocated resources.
 */
PCI::~PCI() {
    
    #if defined __QNX__
    
    #if _NTO_VERSION >= 700
    
    #else
    
    pci_detach(static_cast<uint32_t>(pciHandle));
    
    #endif
    
    #endif
}

/**
 * Opens access to the PCI bus for a given PCI board.
 * @param vendorID the vendor ID of the PCI board.
 * @param deviceID the device ID of the PCI board.
 * @param number the occurence number of this type of device on the PCI
 * bus. '0' is the first board of this type, '1' the second, and so on.
 * @return a device handle.
 */
void* PCI::openDevice(uint16_t vendorID, uint16_t deviceID, uint16_t number) {
    
    #if defined __QNX__
    
	#if _NTO_VERSION >= 700
    /*
    pci_bdf_t pciDevice = pci_device_find(number, vendorID, deviceID, PCI_CCODE_ANY);

    if (pciDevice == PCI_BDF_NONE) {

        return NULL;

    } else {

        return pci_device_attach(pciDevice, pci_attachFlags_DEFAULT, NULL);
    }
    */
    return NULL;

	#else

    pci_dev_info info;
    
    memset(&info, 0, sizeof(info));
    info.VendorId = vendorID;
    info.DeviceId = deviceID;
    
    return pci_attach_device(NULL, PCI_SHARE | PCI_INIT_ALL, number, &info);
    
    #endif
    
    #else
    
    int32_t index = 0;
    
    ifstream fileStream("/proc/bus/pci/devices", ios::in);
    int32_t indexCounter = 0;
    int32_t numberCounter = 0;
    string str;
    if (fileStream) while (getline(fileStream, str)) {
        istringstream stringParser(str, ios_base::in);
        uint32_t bus, vendor;
        stringParser >> bus >> vendor;
        indexCounter++;
        if ((vendorID == static_cast<uint16_t>(vendor >> 16U)) && (deviceID == static_cast<uint16_t>(vendor & 0xFFFF))) {
            if (number == numberCounter) index = indexCounter;
            numberCounter++;
        }
    }
    fileStream.close();
    
    return reinterpret_cast<void*>(index);
    
    #endif
}

/**
 * Closes access to the PCI bus for this device.
 * @param deviceHandle the device handle returned by <code>openDevice()</code>.
 */
int32_t PCI::closeDevice(void* deviceHandle) {
    
    #if defined __QNX__
    
    #if _NTO_VERSION >= 700

    return 0;
    //return pci_device_detach(static_cast<pci_devhdl_t>(deviceHandle));

    #else

    return pci_detach_device(deviceHandle);
    
    #endif

    #else
    
    return 0;
    
    #endif
}

/**
 * Gets one of the base addresses of this PCI board.
 * @param type the type of the base address. This must be in the range [0..5].
 * @param vendorID the vendor ID of the PCI board.
 * @param deviceID the device ID of the PCI board.
 * @param number the occurence number of this type of device on the PCI
 * bus. '0' is the first board of this type, '1' the second, and so on.
 * @return the base address of this PCI board.
 */
uint64_t PCI::getBaseAddress(uint8_t type, uint16_t vendorID, uint16_t deviceID, uint16_t number) {
    
    #if defined __QNX__
    
    #if _NTO_VERSION >= 700
    /*
    pci_bdf_t pciDevice = pci_device_find(number, vendorID, deviceID, PCI_CCODE_ANY);

    if (pciDevice == PCI_BDF_NONE) {

        return 0;

    } else {

        pci_devhdl_t deviceHandle = pci_device_attach(pciDevice, pci_attachFlags_DEFAULT, NULL);

        int_t number = 6;
        pci_ba_t baseAddress[6];

        pci_err_t error = pci_device_read_ba(deviceHandle, &number, baseAddress, pci_reqType_e_UNSPECIFIED);
        if (error == PCI_ERR_OK) {
            for (int_t i = 0; i < number; i++) {
                if (baseAddress[i].bar_num == type) {
                    if (baseAddress[i].type == pci_asType_e_MEM) {
                        return reinterpret_cast<uint64_t>(mmap_device_memory(NULL, baseAddress[i].size, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, baseAddress[i].addr));
                    } else if (baseAddress[i].type == pci_asType_e_IO) {
                        pci_ba_t mappedAddress;
                        error = pci_device_map_as(deviceHandle, &baseAddress[i], &mappedAddress);
                        if (error == PCI_ERR_OK) {
                            return static_cast<uint64_t>(mappedAddress.addr);
                        }
                    }
                }
            }
        }

        return 0;
    }
    */
    return 0;

    #else

    uint32_t bus, deviceFunction;
    uint64_t bar = 0;

    if (pci_find_device(deviceID, vendorID, number, &bus, &deviceFunction) == PCI_SUCCESS) {
        if (pci_read_config32(bus, deviceFunction, 0x10+type*4, 1, &bar) == PCI_SUCCESS) {
            if (PCI_IS_MEM(bar)) {
                return reinterpret_cast<uint64_t>(mmap_device_memory(NULL, 4*1024*1024, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, bar));
            } else if (PCI_IS_IO(bar)) {
                return static_cast<uint64_t>(PCI_IO_ADDR(bar));
            }
        }
    }

    return 0;
    
    #endif

    #else
    
    uint64_t memoryAddr = 0;
    uint64_t memorySize = 0;
    
    ifstream fileStream("/proc/bus/pci/devices", ios::in);
    int32_t counter = 0;
    int32_t counterNumber = 0;
    string str;
    if (fileStream) while (getline(fileStream, str)) {
        istringstream stringParser(str, ios_base::in);
        uint32_t bus, vendor, irq;
        uint64_t addr[7];
        uint64_t size[7];
        stringParser >> hex >> bus >> vendor >> irq >> addr[0] >> addr[1] >> addr[2] >> addr[3] >> addr[4] >> addr[5] >> addr[6] >> size[0] >> size[1] >> size[2] >> size[3] >> size[4] >> size[5] >> size[6];
        counter++;
        if ((vendorID == static_cast<uint16_t>(vendor >> 16U)) && (deviceID == static_cast<uint16_t>(vendor & 0xffff))) {
            if (number == counterNumber) {
                memoryAddr = addr[type];
                memorySize = size[type];
            }
            counterNumber++;
        }
    }
    fileStream.close();

    if ((memoryAddr & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_IO) {
        
        return static_cast<uint64_t>(memoryAddr & PCI_BASE_ADDRESS_IO_MASK);

    } else {

        uint64_t alignedMemoryAddr = memoryAddr & ~(getpagesize()-1);
        uint64_t memoryAddrOffset = memoryAddr-alignedMemoryAddr;

        FILE* memoryFile = fopen("/dev/mem", "r+");
        memoryAddr = reinterpret_cast<uint64_t>(mmap(NULL, (size_t)(memorySize+memoryAddrOffset), PROT_READ | PROT_WRITE, MAP_SHARED, fileno(memoryFile), (off_t)alignedMemoryAddr));
        fclose(memoryFile);

        return memoryAddr+memoryAddrOffset;
    }
    
    return 0;
    
    #endif
}

/**
 * Reads an 8 bit value at a given memory space address.
 * @param address the memory space address.
 * @return the value at the specified address.
 */
uint8_t PCI::in8(uint64_t address) {
    
    volatile uint8_t in = *(uint8_t*)address;

    return in;
}

/**
 * Writes an 8 bit value to a given memory space address.
 * @param address the memory space address.
 * @param value the value to write.
 */
void PCI::out8(uint64_t address, uint8_t value) {

    *(uint8_t*)address = value;
}

/**
 * Reads a 16 bit value at a given memory space address.
 * @param address the memory space address.
 * @return the value at the specified address.
 */
uint16_t PCI::in16(uint64_t address) {

    volatile uint16_t in = *(uint16_t*)address;

    return in;
}

/**
 * Writes a 16 bit value to a given memory space address.
 * @param address the memory space address.
 * @param value the value to write.
 */
void PCI::out16(uint64_t address, uint16_t value) {

    *(uint16_t*)address = value;
}

/**
 * Reads a 32 bit value at a given memory space address.
 * @param address the memory space address.
 * @return the value at the specified address.
 */
uint32_t PCI::in32(uint64_t address) {

    volatile uint32_t in = *(uint32_t*)address;

    return in;
}

/**
 * Writes a 32 bit value to a given memory space address.
 * @param address the memory space address.
 * @param value the value to write.
 */
void PCI::out32(uint64_t address, uint32_t value) {

    *(uint32_t*)address = value;
}

/**
 * Reads an 8 bit value at a given IO space address.
 * @param address the IO space address.
 * @return the value at the specified address.
 */
uint8_t PCI::inIO8(uint64_t address) {
    
    #if defined __QNX__
    
    return static_cast<uint8_t>(in8((uintptr_t)address));
    
    #else
    
    return static_cast<uint8_t>(inb(address));
    
    #endif
}

/**
 * Writes an 8 bit value to a given IO space address.
 * @param address the IO space address.
 * @param value the value to write.
 */
void PCI::outIO8(uint64_t address, uint8_t value) {
    
    #if defined __QNX__
    
    out8((uintptr_t)address, value);
    
    #else
    
    outb(value, address);
    
    #endif
}

/**
 * Reads a 16 bit value at a given IO space address.
 * @param address the IO space address.
 * @return the value at the specified address.
 */
uint16_t PCI::inIO16(uint64_t address) {
    
    #if defined __QNX__
    
    return static_cast<uint16_t>(in16((uintptr_t)address));
    
    #else
    
    return static_cast<uint16_t>(inw(address));
    
    #endif
}

/**
 * Writes a 16 bit value to a given IO space address.
 * @param address the IO space address.
 * @param value the value to write.
 */
void PCI::outIO16(uint64_t address, uint16_t value) {
    
    #if defined __QNX__
    
    out16((uintptr_t)address, value);
    
    #else
    
    outw(value, address);
    
    #endif
}

/**
 * Reads a 32 bit value at a given IO space address.
 * @param address the IO space address.
 * @return the value at the specified address.
 */
uint32_t PCI::inIO32(uint64_t address) {
    
    #if defined __QNX__
    
    return static_cast<uint32_t>(in32((uintptr_t)address));
    
    #else
    
    return static_cast<uint32_t>(inl(address));
    
    #endif
}

/**
 * Writes a 32 bit value to a given IO space address.
 * @param address the IO space address.
 * @param value the value to write.
 */
void PCI::outIO32(uint64_t address, uint32_t value) {
    
    #if defined __QNX__
    
    out32((uintptr_t)address, value);
    
    #else
    
    outl(value, address);
    
    #endif
}
