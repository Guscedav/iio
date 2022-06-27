/*
 * PCI.h
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 29.10.2015
 *      Author: Marcel Honegger
 */

#ifndef PCI_H_
#define PCI_H_

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <sys/mman.h>

#if defined __QNX__

#include <sys/neutrino.h>

#if _NTO_VERSION >= 700

//#include <pci/pci.h>

#else

#include <hw/pci.h>
#include <hw/pci_devices.h>
#include <hw/inout.h>

#endif

#else

#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <sys/io.h>
#include <linux/pci.h>

#endif

/**
 * This class implements a device driver for the Peripheral Component Interconnect (PCI)
 * bus. It works with all types and form factors of PCI busses found in industrial and
 * embedded computers, including compactPCI and PC104+ systems.
 * <br/>
 * This class offers methods to locate PCI boards on the bus based on their vendor
 * and device ID, to obtain the boards base addresses, and to access the boards in
 * either the memory space or the IO space.
 */
class PCI {

    public:

                    PCI();
        virtual     ~PCI();
        void*       openDevice(uint16_t vendorID, uint16_t deviceID, uint16_t number);
        int32_t     closeDevice(void* deviceHandle);
        uint64_t    getBaseAddress(uint8_t type, uint16_t vendorID, uint16_t deviceID, uint16_t number);
        uint8_t     in8(uint64_t address);
        void        out8(uint64_t address, uint8_t value);
        uint16_t    in16(uint64_t address);
        void        out16(uint64_t address, uint16_t value);
        uint32_t    in32(uint64_t address);
        void        out32(uint64_t address, uint32_t value);
        uint8_t     inIO8(uint64_t address);
        void        outIO8(uint64_t address, uint8_t value);
        uint16_t    inIO16(uint64_t address);
        void        outIO16(uint64_t address, uint16_t value);
        uint32_t    inIO32(uint64_t address);
        void        outIO32(uint64_t address, uint32_t value);

    private:

        int32_t     pciHandle;
};

#endif /* PCI_H_ */
