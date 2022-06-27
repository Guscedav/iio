/*
 * BeagleBone.cpp
 * Copyright (c) 2016, ZHAW
 * All rights reserved.
 *
 *  Created on: 10.08.2016
 *      Author: Marcel Honegger
 */

#include <pthread.h>
#include <iostream>

#if defined __QNX__

#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <hw/inout.h>

#endif

#include "CANMessage.h"
#include "BeagleBone.h"

using namespace std;

const uint32_t  BeagleBone::SPI::CONTROL_MODULE_MOSI_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_SPI0_D1, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_UART0_RTSN, CONTROL_MODULE_GMII1_RXER, CONTROL_MODULE_MCASP0_AXR0_MUX0, 0x0, 0x0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_MISO_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_SPI0_D0, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_UART0_CTSN, CONTROL_MODULE_GMII1_CRS, CONTROL_MODULE_MCASP0_FSX_MUX0, 0x0, 0x0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_SCLK_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_SPI0_SCLK, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_ECAP0_IN_PWM0_OUT, CONTROL_MODULE_GMII1_COL, CONTROL_MODULE_MCASP0_ACLKX_MUX0, 0x0, 0x0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_CS0_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_SPI0_CS0, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_UART1_CTSN, CONTROL_MODULE_RMII1_REFCLK, CONTROL_MODULE_UART0_RTSN, CONTROL_MODULE_UART0_RXD, CONTROL_MODULE_MCASP0_AHCLKR_MUX0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_CS1_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_SPI0_CS1, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_UART1_RTSN, CONTROL_MODULE_ECAP0_IN_PWM0_OUT, CONTROL_MODULE_XDMA_EVENT_INTR0, CONTROL_MODULE_UART0_TXD, 0x0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_MOSI_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_MUX_MODE0, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_MUX_MODE4, CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE3, 0x0, 0x0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_MISO_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_MUX_MODE0, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_MUX_MODE4, CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE3, 0x0, 0x0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_SCLK_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_MUX_MODE0, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_MUX_MODE4, CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE3, 0x0, 0x0}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_CS0_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_MUX_MODE0, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_MUX_MODE4, CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE5, CONTROL_MODULE_MUX_MODE1, CONTROL_MODULE_MUX_MODE3}};
const uint32_t  BeagleBone::SPI::CONTROL_MODULE_CS1_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX] = {{CONTROL_MODULE_MUX_MODE0, 0x0, 0x0, 0x0, 0x0}, {CONTROL_MODULE_MUX_MODE4, CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE4, CONTROL_MODULE_MUX_MODE1, 0x0}};
const uint32_t  BeagleBone::SPI::CLOCK_MODULE_PERIPHERAL_SPI_CLKCTRL[] = {0x004C, 0x0050};
const uint32_t  BeagleBone::SPI::CLOCK_MODULE_PERIPHERAL_SPI_CLKCTRL_ENABLE[] = {0x00000002, 0x00000002};
const uint32_t  BeagleBone::SPI::MCSPI_ADDRESS[] = {0x48030000, 0x481A0000};
const size_t    BeagleBone::SPI::MCSPI_LENGTH[] = {0x1000, 0x1000};

const uint32_t  BeagleBone::DCAN::CONTROL_MODULE_DCAN_RAMINIT_START[] = {0x00000001, 0x00000002};
const uint32_t  BeagleBone::DCAN::CONTROL_MODULE_DCAN_RAMINIT_DONE[] = {0x00000100, 0x00000200};
const uint32_t  BeagleBone::DCAN::CONTROL_MODULE_RX_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_DCAN_MUX] = {{0x920, 0x974, 0x97C}, {0x96C, 0x984, 0x904}};
const uint32_t  BeagleBone::DCAN::CONTROL_MODULE_TX_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_DCAN_MUX] = {{0x91C, 0x970, 0x978}, {0x968, 0x980, 0x900}};
const uint32_t  BeagleBone::DCAN::CONTROL_MODULE_DCAN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_DCAN_MUX] = {{CONTROL_MODULE_MUX_MODE1, CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE2}, {CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE4}};
const uint32_t  BeagleBone::DCAN::CLOCK_MODULE_PERIPHERAL_DCAN_CLKCTRL[] = {0x00C0, 0x00C4};
const uint32_t  BeagleBone::DCAN::CLOCK_MODULE_PERIPHERAL_DCAN_CLKCTRL_ENABLE[] = {0x00000002, 0x00000002};
const uint32_t  BeagleBone::DCAN::DCAN_ADDRESS[] = {0x481CC000, 0x481D0000};
const size_t    BeagleBone::DCAN::DCAN_LENGTH[] = {0x2000, 0x2000};

const uint32_t  BeagleBone::QEP::CONTROL_MODULE_A_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX] = {{0x9A0, 0x924}, {0x8D0, 0x850}, {0x8B0, 0x830}};
const uint32_t  BeagleBone::QEP::CONTROL_MODULE_B_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX] = {{0x9A4, 0x928}, {0x8D4, 0x854}, {0x8B4, 0x834}};
const uint32_t  BeagleBone::QEP::CONTROL_MODULE_INDEX_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX] = {{0x9A8, 0x914}, {0x8D8, 0x858}, {0x8B8, 0x838}};
const uint32_t  BeagleBone::QEP::CONTROL_MODULE_STROBE_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX] = {{0x9AC, 0x93C}, {0x8DC, 0x85C}, {0x8BC, 0x83C}};
const uint32_t  BeagleBone::QEP::CONTROL_MODULE_QEP_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_QEP_MUX] = {{CONTROL_MODULE_MUX_MODE1, CONTROL_MODULE_MUX_MODE5}, {CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE6}, {CONTROL_MODULE_MUX_MODE3, CONTROL_MODULE_MUX_MODE4}};
const uint32_t  BeagleBone::QEP::CLOCK_MODULE_PERIPHERAL_EPWMSS_CLKCTRL[] = {0x00D4, 0x00CC, 0x00D8};
const uint32_t  BeagleBone::QEP::QEP_ADDRESS[] = {0x48300180, 0x48302180, 0x48304180};
const size_t    BeagleBone::QEP::QEP_LENGTH[] = {0x0080, 0x0080, 0x0080};

const uint32_t  BeagleBone::PWM::CONTROL_MODULE_OFFSET_ADDRESS[][2][2] = {{{0x990, 0x950}, {0x994, 0x954}}, {{0x8C8, 0x848}, {0x8CC, 0x84C}}, {{0x8A0, 0x820}, {0x8A4, 0x824}}};
const uint32_t  BeagleBone::PWM::CONTROL_MODULE_PWM_MUX_MODE[][2] = {{CONTROL_MODULE_MUX_MODE1, CONTROL_MODULE_MUX_MODE3}, {CONTROL_MODULE_MUX_MODE2, CONTROL_MODULE_MUX_MODE6}, {CONTROL_MODULE_MUX_MODE3, CONTROL_MODULE_MUX_MODE4}};
const uint32_t  BeagleBone::PWM::CLOCK_MODULE_PER_EPWMSS_CLKCTRL[] = {0x00D4, 0x00CC, 0x00D8};
const uint32_t  BeagleBone::PWM::PWMSS_ADDRESS[] = {0x48300000, 0x48302000, 0x48304000};
const uint32_t  BeagleBone::PWM::EPWM_ADDRESS[] = {0x48300200, 0x48302200, 0x48304200};

const uint32_t BeagleBone::CONTROL_MODULE_OFFSET_ADDRESS[] = {0x0854, 0x0858, 0x085C, 0x0860, 0x0818, 0x081C, 0x0808, 0x080C, 0x0890, 0x0894, 0x089C, 0x0898, 0x0834, 0x0830, 0x0824, 0x0828, 0x083C, 0x0838, 0x082C, 0x088C, 0x0820, 0x0884, 0x0880, 0x0814, 0x0810, 0x0804, 0x0800, 0x087C, 0x08E0, 0x08E8, 0x08E4, 0x08EC, 0x08D8, 0x08DC, 0x08D4, 0x08CC, 0x08D0, 0x08C8, 0x08C0, 0x08C4, 0x08B8, 0x08BC, 0x08B0, 0x08B4, 0x08A8, 0x08AC, 0x08A0, 0x08A4, 0x0870, 0x0878, 0x0874, 0x0848, 0x0840, 0x084C, 0x095C, 0x0958, 0x097C, 0x0978, 0x0954, 0x0950, 0x0844, 0x0984, 0x09AC, 0x0980, 0x09A4, 0x099C, 0x0994, 0x0998, 0x0990, 0x09A8, 0x09A0};
const uint8_t BeagleBone::GPIO_DEVICE_NUMBER[] = {1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 0, 0, 1, 1, 0, 2, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 3, 0, 3, 3, 3, 3, 3, 3, 3};
const uint8_t BeagleBone::GPIO_BIT_NUMBER[] = {21, 22, 23, 24, 6, 7, 2, 3, 2, 3, 5, 4, 13, 12, 23, 26, 15, 14, 27, 1, 22, 31, 30, 5, 4, 1, 0, 29, 22, 24, 23, 25, 10, 11, 9, 17, 8, 16, 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 30, 28, 31, 18, 16, 19, 5, 4, 13, 12, 3, 2, 17, 15, 21, 14, 19, 17, 15, 16, 14, 20, 18};
const uint32_t BeagleBone::GPIO_ADDRESS[] = {0x44E07000, 0x4804C000, 0x481AC000, 0x481AE000};
const size_t BeagleBone::GPIO_LENGTH[] = {0x1000, 0x1000, 0x1000, 0x1000};

/**
 * Creates a <code>BeagleBone::SPI</code> object with given pin numbers.
 * This constructor throws an <code>invalid_argument</code> exception if the given pin numbers are wrong.
 * @param mosiPin the BeagleBone Black pin number for the mosi signal.
 * @param misoPin the BeagleBone Black pin number for the miso signal.
 * @param sclkPin the BeagleBone Black pin number for the clock signal.
 * @param cs0Pin the BeagleBone Black pin number for the chip select 0 signal.
 * @param cs1Pin the BeagleBone Black pin number for the chip select 1 signal.
 */
BeagleBone::SPI::SPI(uint16_t mosiPin, uint16_t misoPin, uint16_t sclkPin, uint16_t cs0Pin, uint16_t cs1Pin) {
	
    #if defined __QNX__
    
    // check if the pin numbers are valid
    
    deviceNumber = 1;
    
    uint8_t mosiPinMuxNumber = 0;
    uint8_t misoPinMuxNumber = 0;
    uint8_t sclkPinMuxNumber = 0;
    uint8_t cs0PinMuxNumber = 0;
    uint8_t cs1PinMuxNumber = 0;
    
    if (mosiPin == BeagleBone::P9_30) mosiPinMuxNumber = 2;
    else throw invalid_argument("BeagleBone::SPI: wrong mosi pin number!");
    
    if (misoPin == BeagleBone::P9_29) misoPinMuxNumber = 2;
    else throw invalid_argument("BeagleBone::SPI: wrong miso pin number!");
    
    if (sclkPin == BeagleBone::P9_42) sclkPinMuxNumber = 0;
    else if (sclkPin == BeagleBone::P9_31) sclkPinMuxNumber = 2;
    else throw invalid_argument("BeagleBone::SPI: wrong sclk pin number!");
    
    if (cs0Pin == BeagleBone::P9_20) cs0PinMuxNumber = 0;
    else if (cs0Pin == BeagleBone::P9_28) cs0PinMuxNumber = 4;
    else throw invalid_argument("BeagleBone::SPI: wrong cs0 pin number!");
    
    if (cs1Pin == BeagleBone::P9_19) cs1PinMuxNumber = 0;
    else if ((cs1Pin == BeagleBone::P9_42) && (sclkPin != BeagleBone::P9_42)) cs1PinMuxNumber = 1;
    else throw invalid_argument("BeagleBone::SPI: wrong cs1 pin number!");
    
    // enable access to hardware IO
    
	ThreadCtl(_NTO_TCTL_IO, 0);
    ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
    
    // configure pin muxing & global clock
    
    baseAddress = mmap_device_io(CONTROL_MODULE_LENGTH, CONTROL_MODULE_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::SPI: couldn't remap control module register memory!");
    
    out32(baseAddress+CONTROL_MODULE_MOSI_PIN_OFFSET_ADDRESS[deviceNumber][mosiPinMuxNumber], CONTROL_MODULE_MOSI_PIN_MUX_MODE[deviceNumber][mosiPinMuxNumber]);    // confiuration of pin as mosi output
    out32(baseAddress+CONTROL_MODULE_MISO_PIN_OFFSET_ADDRESS[deviceNumber][misoPinMuxNumber], CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_MISO_PIN_MUX_MODE[deviceNumber][misoPinMuxNumber]);    // confiuration of pin as miso input
    out32(baseAddress+CONTROL_MODULE_SCLK_PIN_OFFSET_ADDRESS[deviceNumber][sclkPinMuxNumber], CONTROL_MODULE_SCLK_PIN_MUX_MODE[deviceNumber][sclkPinMuxNumber]);    // confiuration of pin as sclk output
    out32(baseAddress+CONTROL_MODULE_CS0_PIN_OFFSET_ADDRESS[deviceNumber][cs0PinMuxNumber], CONTROL_MODULE_CS0_PIN_MUX_MODE[deviceNumber][cs0PinMuxNumber]);        // confiuration of pin as cs0 output
    out32(baseAddress+CONTROL_MODULE_CS1_PIN_OFFSET_ADDRESS[deviceNumber][cs1PinMuxNumber], CONTROL_MODULE_CS1_PIN_MUX_MODE[deviceNumber][cs1PinMuxNumber]);        // confiuration of pin as cs1 output
    
    munmap_device_io(baseAddress, CONTROL_MODULE_LENGTH);
    
    // enable peripheral clock module
    
    baseAddress = mmap_device_io(CLOCK_MODULE_PERIPHERAL_LENGTH, CLOCK_MODULE_PERIPHERAL_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::SPI: couldn't remap clock module register memory!");
    
    out32(baseAddress+CLOCK_MODULE_PERIPHERAL_SPI_CLKCTRL[deviceNumber], CLOCK_MODULE_PERIPHERAL_SPI_CLKCTRL_ENABLE[deviceNumber]);
    
    munmap_device_io(baseAddress, CLOCK_MODULE_PERIPHERAL_LENGTH);
    
    // configure SPI control registers
    
    baseAddress = mmap_device_io(MCSPI_LENGTH[deviceNumber], MCSPI_ADDRESS[deviceNumber]);
    
    while (in32(baseAddress+MCSPI_SYSSTATUS) == 0) {}   // wait until reset is done
    
    out32(baseAddress+MCSPI_SYSCONFIG, 0x00000000);     // reset sysconfig register
    out32(baseAddress+MCSPI_IRQENABLE, 0x00000000);     // disable interrupts
    out32(baseAddress+MCSPI_SYST, 0x00000151);          // sets SPIEN (bit 10) = 0 (master), SPIDATDIR1 (bit 9) = 0 (output), SPIDATDIR0 (bit 8) = 1 (input), SPICLK (bit 6) = 1 (output), SPIDAT_1 (bit 5) = 0, SPIDAT_0 (bit 4) = 1, SPIEN_0 (bit 0) = 1
    out32(baseAddress+MCSPI_MODULCTRL, 0x00000000);     // master mode, no delay for first transfer
    out32(baseAddress+MCSPI_CH0CONF, 0x02010FC0);       // TCS (bit 25) = 1 (chip select time 1.5 clock cycles), DPE0 (bit 16) = 1 (no transmission on data line 0), WL (bits 11-7) = 0x1F (32 bit word length), EPOL (bit 6) = 1 (inverse polarity of cs signal)
    out32(baseAddress+MCSPI_CH0CTRL, 0x00000000);       // disable channel 0
    out32(baseAddress+MCSPI_CH1CTRL, 0x00000000);       // disable channel 1
    out32(baseAddress+MCSPI_CH2CTRL, 0x00000000);       // disable channel 2
    out32(baseAddress+MCSPI_CH3CTRL, 0x00000000);       // disable channel 3
    
    #endif
}

/**
 * Deletes the <code>BeagleBone::SPI</code> object and releases all allocated resources.
 */
BeagleBone::SPI::~SPI() {
    
    #if defined __QNX__
    
    munmap_device_io(baseAddress, MCSPI_LENGTH[deviceNumber]);
    
    #endif
}

/**
 * The <code>format()</code> method sets the bit length of the SPI word and
 * the polarity and phase of the SPI clock signal.
 * @param bits the bit length of the SPI word, possible values are 32, 24, 16 and 8.
 * @param mode the polarity and phase mode of the SPI clock. Possible values are:
 * - mode=0: polarity=0 & phase=0
 * - mode=1: polarity=0 & phase=1
 * - mode=2: polarity=1 & phase=0
 * - mode=3: polarity=1 & phase=1
 */
void BeagleBone::SPI::format(uint8_t bits, uint8_t mode) {
    
    #if defined __QNX__
    
    uint32_t channelConfig = in32(baseAddress+MCSPI_CH0CONF);
    channelConfig &= ~(0x1F << 7);
    
    switch (bits) {
        case 32:
            channelConfig |= 0x1F << 7;
            break;
        case 24:
            channelConfig |= 0x17 << 7;
            break;
        case 16:
            channelConfig |= 0x0F << 7;
            break;
        case 8:
            channelConfig |= 0x07 << 7;
            break;
        default:
            channelConfig |= 0x1F << 7;
            break;
    }
    
    switch (mode) {
        case 0:
            channelConfig &= ~0x00000002;
            channelConfig &= ~0x00000001;
            break;
        case 1:
            channelConfig &= ~0x00000002;
            channelConfig |= 0x00000001;
            break;
        case 2:
            channelConfig |= 0x00000002;
            channelConfig &= ~0x00000001;
            break;
        case 3:
            channelConfig |= 0x00000002;
            channelConfig |= 0x00000001;
            break;
        default:
            channelConfig &= ~0x00000002;
            channelConfig &= ~0x00000001;
            break;
    }
    
    out32(baseAddress+MCSPI_CH0CONF, channelConfig);
    
    #endif
}

/**
 * Selects the frequency of the SPI clock.
 * @param hz the desired frequency given in [Hz]. Possible values are:
 * - 48000000 (48 MHz)
 * - 24000000 (24 MHz)
 * - 12000000 (12 MHz)
 * -  6000000 (6 MHz)
 * -  3000000 (3 MHz)
 * -  1500000 (1.5 MHz)
 * -   750000 (750 kHz)
 * -   375000 (375 kHz)
 * -   187500 (187.5 kHz)
 * -    93750 (93.75 kHz)
 */
void BeagleBone::SPI::frequency(uint32_t hz) {
    
    #if defined __QNX__
    
    uint32_t channelConfig = in32(baseAddress+MCSPI_CH0CONF);
    channelConfig &= ~0x0000003C;
    
    switch (hz) {
        case 48000000:
            channelConfig |= 0 << 2;
            break;
        case 24000000:
            channelConfig |= 1 << 2;
            break;
        case 12000000:
            channelConfig |= 2 << 2;
            break;
        case 6000000:
            channelConfig |= 3 << 2;
            break;
        case 3000000:
            channelConfig |= 4 << 2;
            break;
        case 1500000:
            channelConfig |= 5 << 2;
            break;
        case 750000:
            channelConfig |= 6 << 2;
            break;
        case 375000:
            channelConfig |= 7 << 2;
            break;
        case 187500:
            channelConfig |= 8 << 2;
            break;
        case 93750:
            channelConfig |= 9 << 2;
            break;
        default:
            channelConfig |= 9 << 2;
            break;
    }
    
    out32(baseAddress+MCSPI_CH0CONF, channelConfig);
    
    #endif
}

/**
 * Writes an SPI word to the slave and returns the response.
 * @param value the SPI word to write.
 * @param return the response from the SPI slave.
 */
uint32_t BeagleBone::SPI::write(uint32_t value) {
    
    #if defined __QNX__
    
    out32(baseAddress+MCSPI_CH0CTRL, 0x00000001);                   // enable channel 0
    while ((in32(baseAddress+MCSPI_CH0STAT) & 0x00000002) == 0) {}  // wait until the transmit register is empty
    out32(baseAddress+MCSPI_TX0, value);                            // write values for transfer to the shift register
    while ((in32(baseAddress+MCSPI_CH0STAT) & 0x00000001) == 0) {}  // wait until receiver register is full
    out32(baseAddress+MCSPI_CH0CTRL, 0x00000000);                   // disable channel 0
    
    return static_cast<uint32_t>(in32(baseAddress+MCSPI_RX0));
    
    #else
    
    return 0;
    
    #endif
}

/**
 * Creates a <code>BeagleBone::DCAN</code> object with given rx and tx pin numbers.
 * This constructor throws an <code>invalid_argument</code> exception if the given pin numbers are wrong.
 * @param rxPin the BeagleBone Black pin number to receive data.
 * @param txPin the BeagleBone Black pin number to transmit data.
 */
BeagleBone::DCAN::DCAN(uint16_t rxPin, uint16_t txPin) {
	
    #if defined __QNX__
    
    // check if the pin number is valid
    
    deviceNumber = 0;
    uint8_t rxMuxNumber = 0;
    uint8_t txMuxNumber = 0;
    
    if (rxPin == BeagleBone::P9_19) {
        deviceNumber = 0;
        rxMuxNumber = 2;
        if (txPin == BeagleBone::P9_20) txMuxNumber = 2;
        else throw invalid_argument("BeagleBone::DCAN: wrong tx pin number!");
    } else if (rxPin == BeagleBone::P9_24) {
        deviceNumber = 1;
        rxMuxNumber = 1;
        if (txPin == BeagleBone::P9_26) txMuxNumber = 1;
        else throw invalid_argument("BeagleBone::DCAN: wrong tx pin number!");
    } else {
        throw invalid_argument("BeagleBone::DCAN: wrong rx pin number!");
    }
    
    // enable access to hardware IO
    
	ThreadCtl(_NTO_TCTL_IO, 0);
    ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
    
    // configure pin muxing and init dcan message ram
    
    baseAddress = mmap_device_io(CONTROL_MODULE_LENGTH, CONTROL_MODULE_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleboneDCAN: couldn't remap control module register memory!");
    
    out32(baseAddress+CONTROL_MODULE_RX_PIN_OFFSET_ADDRESS[deviceNumber][rxMuxNumber], CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_DCAN_MUX_MODE[deviceNumber][rxMuxNumber]);    // configuration of dcan_rx
    out32(baseAddress+CONTROL_MODULE_TX_PIN_OFFSET_ADDRESS[deviceNumber][txMuxNumber], CONTROL_MODULE_DCAN_MUX_MODE[deviceNumber][txMuxNumber]);                                                            // configuration of dcan_tx
    
    out32(baseAddress+CONTROL_MODULE_DCAN_RAMINIT, CONTROL_MODULE_DCAN_RAMINIT_START[deviceNumber]);
    while ((in32(baseAddress+CONTROL_MODULE_DCAN_RAMINIT) & CONTROL_MODULE_DCAN_RAMINIT_DONE[deviceNumber]) != CONTROL_MODULE_DCAN_RAMINIT_DONE[deviceNumber]) {}
    
    munmap_device_io(baseAddress, CONTROL_MODULE_LENGTH);
    
    // enable peripheral clock module
    
    baseAddress = mmap_device_io(CLOCK_MODULE_PERIPHERAL_LENGTH, CLOCK_MODULE_PERIPHERAL_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleboneDCAN: couldn't remap clock module register memory!");
    
    out32(baseAddress+CLOCK_MODULE_PERIPHERAL_DCAN_CLKCTRL[deviceNumber], CLOCK_MODULE_PERIPHERAL_DCAN_CLKCTRL_ENABLE[deviceNumber]);
    
    munmap_device_io(baseAddress, CLOCK_MODULE_PERIPHERAL_LENGTH);
    
    // configure can controller, enter init mode
    
    baseAddress = mmap_device_io(DCAN_LENGTH[deviceNumber], DCAN_ADDRESS[deviceNumber]);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleboneDCAN: couldn't remap dcan register memory!");
    
    uint32_t ctl = in32(baseAddress+DCAN_CTL);
    ctl |= DCAN_CTL_INIT;
    out32(baseAddress+DCAN_CTL, ctl);   // enter initialization mode
    ctl |= DCAN_CTL_CCE;
    out32(baseAddress+DCAN_CTL, ctl);   // enable configuration change
    
    while ((in32(baseAddress+DCAN_CTL) & DCAN_CTL_INIT) != DCAN_CTL_INIT) {}    // wait for initialization mode
    
    // configure bit timing with can clock of 24 MHz to default of 1 MHz
    
    uint32_t brp = 0;       // range 0 - 63 (+1 = prescaler down from 25 Mhz)
    uint32_t tseg1 = 14;    // range 1 - 15 (+1 = tq)
    uint32_t tseg2 = 7;     // range 0 -  7 (+1 = tq)
    uint32_t sjw = 3;       // range 0 -  3 (+1 = tq), must not be longer than tseg2!
    
    out32(baseAddress+DCAN_BTR, (tseg2 << DCAN_BTR_TSEG2_SHIFT) | (tseg1 << DCAN_BTR_TSEG1_SHIFT) | (sjw << DCAN_BTR_SJW_SHIFT) | brp);
    
    // configure transmit messages in message ram
    
    for (uint8_t message = DCAN_IFXCMD_MESSAGE_TX_MIN; message <= DCAN_IFXCMD_MESSAGE_TX_MAX; message++) {
        
        out32(baseAddress+DCAN_IF1MSK, DCAN_IFXMSK_DEFAULT);        // set acceptance filter mask to don't care
        out32(baseAddress+DCAN_IF1ARB, DCAN_IFXARB_DIR_TRANSMIT);   // set direction bit (transmit) & message ID (0)
        out32(baseAddress+DCAN_IF1MCTL, 0);
        out32(baseAddress+DCAN_IF1DATA, 0);
        out32(baseAddress+DCAN_IF1DATB, 0);
        
        out32(baseAddress+DCAN_IF1CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
        
        while ((in32(baseAddress+DCAN_IF1CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
    }
    
    // configure transmit messages for remote frames in message ram
    
    for (uint8_t message = DCAN_IFXCMD_MESSAGE_TX_REMOTE_MIN; message <= DCAN_IFXCMD_MESSAGE_TX_REMOTE_MAX; message++) {
        
        out32(baseAddress+DCAN_IF1MSK, DCAN_IFXMSK_DEFAULT);                        // set acceptance filter mask to don't care
        out32(baseAddress+DCAN_IF1ARB, 0);                                          // set message not valid, direction bit (receive) & message ID (0)
        out32(baseAddress+DCAN_IF1MCTL, DCAN_IFXMCTL_UMASK | DCAN_IFXMCTL_RX_IE);   // use filter mask (even when it's "don't care")
        out32(baseAddress+DCAN_IF1DATA, 0);
        out32(baseAddress+DCAN_IF1DATB, 0);
        
        out32(baseAddress+DCAN_IF1CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
        
        while ((in32(baseAddress+DCAN_IF1CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
    }
    
    // configure receive messages for remote frames in message ram
    
    for (uint8_t message = DCAN_IFXCMD_MESSAGE_RX_REMOTE_MIN; message <= DCAN_IFXCMD_MESSAGE_RX_REMOTE_MAX; message++) {
        
        out32(baseAddress+DCAN_IF1MSK, DCAN_IFXMSK_DEFAULT);                            // set acceptance filter mask to don't care
        out32(baseAddress+DCAN_IF1ARB, DCAN_IFXARB_MSG_VAL | DCAN_IFXARB_DIR_TRANSMIT); // set message valid, direction bit (transmit) & message ID (0)
        out32(baseAddress+DCAN_IF1MCTL, DCAN_IFXMCTL_UMASK | DCAN_IFXMCTL_RX_IE);       // use filter mask (even when it's "don't care")
        out32(baseAddress+DCAN_IF1DATA, 0);
        out32(baseAddress+DCAN_IF1DATB, 0);
        
        out32(baseAddress+DCAN_IF1CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
        
        while ((in32(baseAddress+DCAN_IF1CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
    }
    
    // configure receive messages in message ram
    
    for (uint8_t message = DCAN_IFXCMD_MESSAGE_RX_MIN; message <= DCAN_IFXCMD_MESSAGE_RX_MAX; message++) {
        
        out32(baseAddress+DCAN_IF1MSK, DCAN_IFXMSK_DEFAULT);                        // set acceptance filter mask to don't care
        out32(baseAddress+DCAN_IF1ARB, DCAN_IFXARB_MSG_VAL);                        // set message valid, direction bit (receive) & message ID (0)
        out32(baseAddress+DCAN_IF1MCTL, DCAN_IFXMCTL_UMASK | DCAN_IFXMCTL_RX_IE);   // use filter mask (even when it's "don't care")
        out32(baseAddress+DCAN_IF1DATA, 0);
        out32(baseAddress+DCAN_IF1DATB, 0);
        
        out32(baseAddress+DCAN_IF1CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
        
        while ((in32(baseAddress+DCAN_IF1CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
    }
    
    out32(baseAddress+DCAN_IF1MCTL, DCAN_IFXMCTL_UMASK | DCAN_IFXMCTL_RX_IE | DCAN_IFXMCTL_EOB);
    out32(baseAddress+DCAN_IF1CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_MESSAGE_RX_MAX);
    
    while ((in32(baseAddress+DCAN_IF1CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
    
    // finish init and exit init mode
    
    ctl = in32(baseAddress+DCAN_CTL);
    ctl &= ~DCAN_CTL_INIT;
    out32(baseAddress+DCAN_CTL, ctl);   // enter normal operation
    ctl |= DCAN_CTL_ABO;
    out32(baseAddress+DCAN_CTL, ctl);   // enable 'auto bus on'
    in32(baseAddress+DCAN_ES);          // clear interrupts
    
    #endif
}

/**
 * Deletes the <code>BeagleBone::DCAN</code> object and releases all allocated resources.
 */
BeagleBone::DCAN::~DCAN() {
	
    #if defined __QNX__
    
    munmap_device_io(baseAddress, DCAN_LENGTH[deviceNumber]);
    
    #endif
}

/**
 * Sets the frequency of the CAN bus, given in [Hz].
 * @param hz the frequency of the CAN bus, given in [Hz]. This value must be either
 * 2000000, 1000000, 800000, 500000, 250000, 125000 or 100000.
 */
void BeagleBone::DCAN::frequency(uint32_t hz) {
    
    #if defined __QNX__
    
    mutex.lock();
    
    // configure can controller, enter init mode
    
    uint32_t ctl = in32(baseAddress+DCAN_CTL);
    ctl |= DCAN_CTL_INIT;
    out32(baseAddress+DCAN_CTL, ctl);   // enter initialization mode
    ctl |= DCAN_CTL_CCE;
    out32(baseAddress+DCAN_CTL, ctl);   // enable configuration change
    
    while ((in32(baseAddress+DCAN_CTL) & DCAN_CTL_INIT) != DCAN_CTL_INIT) {}    // wait for initialization mode
    
    // configure bit timing with can clock of 24 MHz
    
    uint32_t brp = 0;   // range 0 - 63 (+1 = prescaler down from 25 Mhz)
    uint32_t tseg1 = 1; // range 1 - 15 (+1 = tq)
    uint32_t tseg2 = 0; // range 0 -  7 (+1 = tq)
    uint32_t sjw = 3;   // range 0 -  3 (+1 = tq), must not be longer than tseg2!
    
    switch (hz) {
        case 2000000:
            brp = 0;
            tseg1 = 6;
            tseg2 = 3;
            break;
        case 1000000:
            brp = 0;
            tseg1 = 14;
            tseg2 = 7;
            break;
        case 800000:
            brp = 1;
            tseg1 = 8;
            tseg2 = 4;
            break;
        case 500000:
            brp = 1;
            tseg1 = 14;
            tseg2 = 7;
            break;
        case 250000:
            brp = 3;
            tseg1 = 14;
            tseg2 = 7;
            break;
        case 125000:
            brp = 7;
            tseg1 = 14;
            tseg2 = 7;
            break;
        case 100000:
            brp = 9;
            tseg1 = 14;
            tseg2 = 7;
            break;
        default:
            brp = 7;
            tseg1 = 14;
            tseg2 = 7;
            break;
    }
    
    out32(baseAddress+DCAN_BTR, (tseg2 << DCAN_BTR_TSEG2_SHIFT) | (tseg1 << DCAN_BTR_TSEG1_SHIFT) | (sjw << DCAN_BTR_SJW_SHIFT) | brp);
    
    // finish init and exit init mode
    
    ctl = in32(baseAddress+DCAN_CTL);
    ctl &= ~DCAN_CTL_INIT;
    out32(baseAddress+DCAN_CTL, ctl);   // enter normal operation
    ctl |= DCAN_CTL_ABO;
    out32(baseAddress+DCAN_CTL, ctl);   // enable 'auto bus on'
    in32(baseAddress+DCAN_ES);          // clear interrupts
    
    mutex.unlock();
    
    #endif
}

/**
 * Writes a CAN message for transmission on the CAN bus.
 * @param canMessage a CAN message object to transmit.
 * @return 0 if this write command failed, 1 otherwise.
 */
int32_t BeagleBone::DCAN::write(CANMessage canMessage) {
    
    #if defined __QNX__
    
    mutex.lock();
    
    if (canMessage.type == CANRemote) {
        
        uint32_t nwdat34 = in32(baseAddress+DCAN_NWDAT34);
        
        for (uint8_t message = DCAN_IFXCMD_MESSAGE_TX_REMOTE_MIN; message <= DCAN_IFXCMD_MESSAGE_TX_REMOTE_MAX; message++) {
            
            if ((nwdat34 & (1 << (message-DCAN_IFXCMD_MESSAGE_TX_REMOTE_MIN))) == 0) {
                
                // an unused buffer for a remote receive message found
                
                out32(baseAddress+DCAN_IF1MSK, DCAN_IFXMSK_DEFAULT);                                                        // set acceptance filter mask to don't care
                out32(baseAddress+DCAN_IF1ARB, DCAN_IFXARB_MSG_VAL | ((canMessage.id & 0x7FF) << DCAN_IFXARB_ID_SHIFT));    // set message valid, direction bit (receive) & message ID
                out32(baseAddress+DCAN_IF1MCTL, DCAN_IFXMCTL_UMASK | DCAN_IFXMCTL_RX_IE | DCAN_IFXMCTL_TX_RQST);            // use filter mask (even when it's "don't care") & request transmission
                out32(baseAddress+DCAN_IF1DATA, 0);
                out32(baseAddress+DCAN_IF1DATB, 0);
                
                out32(baseAddress+DCAN_IF1CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
                
                while ((in32(baseAddress+DCAN_IF1CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
                
                mutex.unlock();
                
                return 1;
            }
        }
        
    } else {    // can message type must be regular CAN data message
        
        uint32_t txrq12 = in32(baseAddress+DCAN_TXRQ12);
        
        for (uint8_t message = DCAN_IFXCMD_MESSAGE_TX_MIN; message <= DCAN_IFXCMD_MESSAGE_TX_MAX; message++) {
            
            if ((txrq12 & (1 << (message-DCAN_IFXCMD_MESSAGE_TX_MIN))) == 0) {
                
                // an empty transmit buffer was found
                
                out32(baseAddress+DCAN_IF1ARB, DCAN_IFXARB_MSG_VAL | DCAN_IFXARB_DIR_TRANSMIT | ((canMessage.id & 0x7FF) << DCAN_IFXARB_ID_SHIFT));     // set message valid, direction bit & message ID
                out32(baseAddress+DCAN_IF1MCTL, DCAN_IFXMCTL_NEW_DAT | DCAN_IFXMCTL_TX_RQST | DCAN_IFXMCTL_EOB | canMessage.len);                       // new data written & data length
                out32(baseAddress+DCAN_IF1DATA, canMessage.data[0] | (canMessage.data[1] << 8) | (canMessage.data[2] << 16) | (canMessage.data[3] << 24));
                out32(baseAddress+DCAN_IF1DATB, canMessage.data[4] | (canMessage.data[5] << 8) | (canMessage.data[6] << 16) | (canMessage.data[7] << 24));
                
                out32(baseAddress+DCAN_IF1CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_TX_RQST | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
                
                while ((in32(baseAddress+DCAN_IF1CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
                
                mutex.unlock();
                
                return 1;
            }
        }
    }
    
    mutex.unlock();
    
    return 0;
    
    #else
    
    return 0;
    
    #endif
}

/**
 * Reads a CAN message received from the CAN bus.
 * @param canMessage a reference to a CAN message object to overwrite.
 * @return 0 if no message was received, 1 if a message could be read successfully.
 */
int32_t BeagleBone::DCAN::read(CANMessage& canMessage) {
    
    #if defined __QNX__
    
    mutex.lock();
    
    uint32_t nwdat34 = in32(baseAddress+DCAN_NWDAT34);
    
    for (uint8_t message = DCAN_IFXCMD_MESSAGE_TX_REMOTE_MIN; message <= DCAN_IFXCMD_MESSAGE_TX_REMOTE_MAX; message++) {
        
        if ((nwdat34 & (1 << (message-DCAN_IFXCMD_MESSAGE_TX_REMOTE_MIN))) > 0) {
            
            // read a remote message that was received
            
            out32(baseAddress+DCAN_IF2CMD, DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_CLR_INT_PND | DCAN_IFXCMD_TX_RQST | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB);
            out32(baseAddress+DCAN_IF2CMD, DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_CLR_INT_PND | DCAN_IFXCMD_TX_RQST | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
            
            uint32_t if2cmd = in32(baseAddress+DCAN_IF2CMD);
            while ((if2cmd & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) if2cmd = in32(baseAddress+DCAN_IF2CMD);
            
            uint32_t if2arb = in32(baseAddress+DCAN_IF2ARB);
            uint32_t if2mctl = in32(baseAddress+DCAN_IF2MCTL);
            uint32_t if2data = in32(baseAddress+DCAN_IF2DATA);
            uint32_t if2datb = in32(baseAddress+DCAN_IF2DATB);
            
            canMessage.id = (if2arb >> DCAN_IFXARB_ID_SHIFT) & DCAN_IFXARB_ID_MASK;
            canMessage.type = CANData;
            canMessage.len = if2mctl & DCAN_IFXMCTL_LEN_MASK;
            
            uint64_t data = if2data | (static_cast<uint64_t>(if2datb) << 32);
            
            for (uint8_t i = 0; i < canMessage.len; i++) canMessage.data[i] = static_cast<uint8_t>((data >> (i*8)) & 0xFF);
            for (uint8_t i = canMessage.len; i < 8; i++) canMessage.data[i] = 0;
            
            // reconfigure buffer for transmit messages for remote frames
            
            out32(baseAddress+DCAN_IF2MSK, DCAN_IFXMSK_DEFAULT);                        // set acceptance filter mask to don't care
            out32(baseAddress+DCAN_IF2ARB, 0);                                          // set message not valid, direction bit (receive) & message ID (0)
            out32(baseAddress+DCAN_IF2MCTL, DCAN_IFXMCTL_UMASK | DCAN_IFXMCTL_RX_IE);   // use filter mask (even when it's "don't care")
            out32(baseAddress+DCAN_IF2DATA, 0);
            out32(baseAddress+DCAN_IF2DATB, 0);
            
            out32(baseAddress+DCAN_IF2CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
            
            while ((in32(baseAddress+DCAN_IF2CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
            
            mutex.unlock();
            
            return 1;
        }
    }
    
    for (uint8_t message = DCAN_IFXCMD_MESSAGE_RX_REMOTE_MIN; message <= DCAN_IFXCMD_MESSAGE_RX_MAX; message++) {
        
        if ((nwdat34 & (1 << (message-DCAN_IFXCMD_MESSAGE_TX_REMOTE_MIN))) > 0) {
            
            // read a standard message that was received
            
            out32(baseAddress+DCAN_IF2CMD, DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_CLR_INT_PND | DCAN_IFXCMD_TX_RQST | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB);
            out32(baseAddress+DCAN_IF2CMD, DCAN_IFXCMD_MASK | DCAN_IFXCMD_ARB | DCAN_IFXCMD_CONTROL | DCAN_IFXCMD_CLR_INT_PND | DCAN_IFXCMD_TX_RQST | DCAN_IFXCMD_DATA | DCAN_IFXCMD_DATB | message);
            
            uint32_t if2cmd = in32(baseAddress+DCAN_IF2CMD);
            while ((if2cmd & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) if2cmd = in32(baseAddress+DCAN_IF2CMD);
            
            uint32_t if2arb = in32(baseAddress+DCAN_IF2ARB);
            uint32_t if2mctl = in32(baseAddress+DCAN_IF2MCTL);
            uint32_t if2data = in32(baseAddress+DCAN_IF2DATA);
            uint32_t if2datb = in32(baseAddress+DCAN_IF2DATB);
            
            canMessage.id = (if2arb >> DCAN_IFXARB_ID_SHIFT) & DCAN_IFXARB_ID_MASK;
            canMessage.type = ((message >= DCAN_IFXCMD_MESSAGE_RX_REMOTE_MIN) && (message <= DCAN_IFXCMD_MESSAGE_RX_REMOTE_MAX)) ? CANRemote : CANData;
            canMessage.len = if2mctl & DCAN_IFXMCTL_LEN_MASK;
            
            uint64_t data = if2data | (static_cast<uint64_t>(if2datb) << 32);
            
            for (uint8_t i = 0; i < canMessage.len; i++) canMessage.data[i] = static_cast<uint8_t>((data >> (i*8)) & 0xFF);
            for (uint8_t i = canMessage.len; i < 8; i++) canMessage.data[i] = 0;
            
            // reconfigure buffer for receive messages
            
            out32(baseAddress+DCAN_IF2MCTL, DCAN_IFXMCTL_UMASK | DCAN_IFXMCTL_RX_IE);
            out32(baseAddress+DCAN_IF2CMD, DCAN_IFXCMD_WRITE | DCAN_IFXCMD_CONTROL | message);
            
            while ((in32(baseAddress+DCAN_IF2CMD) & DCAN_IFXCMD_BUSY) == DCAN_IFXCMD_BUSY) {}
            
            mutex.unlock();
            
            return 1;
        }
    }
    
    mutex.unlock();
    
    return 0;
    
    #else
    
    return 0;
    
    #endif
}

/**
 * Creates a <code>BeagleBone::QEP</code> object with given pin numbers.
 * @param aPin the BeagleBone Black pin number with the A channel of the incremental encoder.
 * @param bPin the BeagleBone Black pin number with the B channel of the incremental encoder.
 * @param indexPin the BeagleBone Black pin number with the index channel of the incremental encoder.
 * @param strobePin the BeagleBone Black pin number with a strobe input.
 */
BeagleBone::QEP::QEP(uint16_t aPin, uint16_t bPin, uint16_t indexPin, uint16_t strobePin) {
	
    #if defined __QNX__
    
    // check if the pin number is valid
    
    deviceNumber = 0;
    uint8_t aMuxNumber = 0;
    uint8_t bMuxNumber = 0;
    uint8_t indexMuxNumber = 0;
    uint8_t strobeMuxNumber = 0;
    
    if (aPin == BeagleBone::P9_42) {
        deviceNumber = 0;
        aMuxNumber = 0;
        if (bPin == BeagleBone::P9_27) bMuxNumber = 0;
        else throw invalid_argument("BeagleBone::QEP: wrong B pin number!");
        if (indexPin == BeagleBone::P9_41) indexMuxNumber = 0;
        else throw invalid_argument("BeagleBone::QEP: wrong index pin number!");
        if (strobePin == BeagleBone::P9_25) strobeMuxNumber = 0;
        else throw invalid_argument("BeagleBone::QEP: wrong strobe pin number!");
    } else if (aPin == BeagleBone::P8_35) {
        deviceNumber = 1;
        aMuxNumber = 0;
        if (bPin == BeagleBone::P8_33) bMuxNumber = 0;
        else throw invalid_argument("BeagleBone::QEP: wrong B pin number!");
        if (indexPin == BeagleBone::P8_31) indexMuxNumber = 0;
        else throw invalid_argument("BeagleBone::QEP: wrong index pin number!");
        if (strobePin == BeagleBone::P8_32) strobeMuxNumber = 0;
        else throw invalid_argument("BeagleBone::QEP: wrong strobe pin number!");
    } else if (aPin == BeagleBone::P8_41) {
        deviceNumber = 2;
        aMuxNumber = 0;
        if (bPin == BeagleBone::P8_42) bMuxNumber = 0;
        else if (bPin == BeagleBone::P8_11) bMuxNumber = 1;
        else throw invalid_argument("BeagleBone::QEP: wrong B pin number!");
        if (indexPin == BeagleBone::P8_39) indexMuxNumber = 0;
        else if (indexPin == BeagleBone::P8_16) indexMuxNumber = 1;
        else throw invalid_argument("BeagleBone::QEP: wrong index pin number!");
        if (strobePin == BeagleBone::P8_40) strobeMuxNumber = 0;
        else if (strobePin == BeagleBone::P8_15) strobeMuxNumber = 1;
        else throw invalid_argument("BeagleBone::QEP: wrong strobe pin number!");
    } else if (aPin == BeagleBone::P8_12) {
        deviceNumber = 2;
        aMuxNumber = 1;
        if (bPin == BeagleBone::P8_42) bMuxNumber = 0;
        else if (bPin == BeagleBone::P8_11) bMuxNumber = 1;
        else throw invalid_argument("BeagleBone::QEP: wrong B pin number!");
        if (indexPin == BeagleBone::P8_39) indexMuxNumber = 0;
        else if (indexPin == BeagleBone::P8_16) indexMuxNumber = 1;
        else throw invalid_argument("BeagleBone::QEP: wrong index pin number!");
        if (strobePin == BeagleBone::P8_40) strobeMuxNumber = 0;
        else if (strobePin == BeagleBone::P8_15) strobeMuxNumber = 1;
        else throw invalid_argument("BeagleBone::QEP: wrong strobe pin number!");
    } else {
        throw invalid_argument("BeagleBone::QEP: wrong A pin number!");
    }
    
    // enable access to hardware IO
    
	ThreadCtl(_NTO_TCTL_IO, 0);
    ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
    
    // configure pin muxing
    
    baseAddress = mmap_device_io(CONTROL_MODULE_LENGTH, CONTROL_MODULE_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::QEP: couldn't remap control module register memory!");
    
    out32(baseAddress+CONTROL_MODULE_A_PIN_OFFSET_ADDRESS[deviceNumber][aMuxNumber], CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_QEP_MUX_MODE[deviceNumber][aMuxNumber]);    // configuration of A channel
    out32(baseAddress+CONTROL_MODULE_B_PIN_OFFSET_ADDRESS[deviceNumber][bMuxNumber], CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_QEP_MUX_MODE[deviceNumber][bMuxNumber]);    // configuration of B channel
    out32(baseAddress+CONTROL_MODULE_INDEX_PIN_OFFSET_ADDRESS[deviceNumber][indexMuxNumber], CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_QEP_MUX_MODE[deviceNumber][indexMuxNumber]);    // configuration of index channel
    out32(baseAddress+CONTROL_MODULE_STROBE_PIN_OFFSET_ADDRESS[deviceNumber][strobeMuxNumber], CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_QEP_MUX_MODE[deviceNumber][strobeMuxNumber]); // configuration of strobe input
    
    munmap_device_io(baseAddress, CONTROL_MODULE_LENGTH);
    
    // enable peripheral clock module
    
    baseAddress = mmap_device_io(CLOCK_MODULE_PERIPHERAL_LENGTH, CLOCK_MODULE_PERIPHERAL_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::QEP: couldn't remap clock module register memory!");
    
    out32(baseAddress+CLOCK_MODULE_PERIPHERAL_EPWMSS_CLKCTRL[deviceNumber], CLOCK_MODULE_PERIPHERAL_EPWMSS_CLKCTRL_ENABLE);
    
    munmap_device_io(baseAddress, CLOCK_MODULE_PERIPHERAL_LENGTH);
    
    // configure quadrature encoder pulse module
    
    baseAddress = mmap_device_io(QEP_LENGTH[deviceNumber], QEP_ADDRESS[deviceNumber]);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::QEP: couldn't remap qep module register memory!");
    
    out32(baseAddress+QEP_QPOSCNT, 0x00000000); // initialize counter to 0
    out32(baseAddress+QEP_QPOSMAX, 0xFFFFFFFF); // set the maximum position to max uint32
    out16(baseAddress+QEP_QDECCTL, 0x0000);     // disable position compare, normal polarity of A, B, index and strobe signals
    out16(baseAddress+QEP_QEPCTL, 0x1018);      // position counter reset on the maximum position, latches position counter on rising edge of the index signal, position counter is enabled
    
    #endif

    gain = 1;
}

/**
 * Deletes the <code>BeagleBone::QEP</code> object and releases all allocated resources.
 */
BeagleBone::QEP::~QEP() {
    
    #if defined __QNX__
    
    munmap_device_io(baseAddress, QEP_LENGTH[deviceNumber]);
    
    #endif
}

/**
 * Sets a gain the raw encoder value is multiplied with.
 * @param gain the gain to multiply the raw encoder value with.
 */
void BeagleBone::QEP::setGain(int32_t gain) {

    this->gain = gain;
}

/**
 * Reads the value of this quadrature encoder counter.
 * @return the current value of the encoder counter.
 */
int32_t BeagleBone::QEP::read() {
    
    #if defined __QNX__
    
    return static_cast<int32_t>(in32(baseAddress+QEP_QPOSCNT))*gain;
    
    #else
    
    return 0;
    
    #endif
}

/**
 * Reads the value this quadrature encoder counter had when the index pulse was received.
 * This value may be used for applications that require a precise absolute position.
 * @return the value of the encoder counter when the index pulse was received.
 */
int32_t BeagleBone::QEP::readCounterAtIndexPulse() {
    
    #if defined __QNX__
    
    return static_cast<int32_t>(in32(baseAddress+QEP_QPOSILAT))*gain;
    
    #else
    
    return 0;
    
    #endif
}

/**
 * Creates a <code>BeagleBone::PWM</code> object with a given pin number.
 * @param pin the BeagleBone Black pin number to control.
 */
BeagleBone::PWM::PWM(uint16_t pin) {
	
    #if defined __QNX__
    
    // check if the pin number is valid
    
    uint8_t deviceNumber = 0;
    channelNumber = 0;
    uint8_t muxNumber = 0;
    
    switch (pin) {
        case BeagleBone::P8_13:
            deviceNumber = 2;
            channelNumber = 1;
            muxNumber = 1;
            break;
        case BeagleBone::P8_19:
            deviceNumber = 2;
            channelNumber = 0;
            muxNumber = 1;
            break;
        case BeagleBone::P8_34:
            deviceNumber = 1;
            channelNumber = 1;
            muxNumber = 0;
            break;
        case BeagleBone::P8_36:
            deviceNumber = 1;
            channelNumber = 0;
            muxNumber = 0;
            break;
        case BeagleBone::P8_45:
            deviceNumber = 2;
            channelNumber = 0;
            muxNumber = 0;
            break;
        case BeagleBone::P8_46:
            deviceNumber = 2;
            channelNumber = 1;
            muxNumber = 0;
            break;
        case BeagleBone::P9_14:
            deviceNumber = 1;
            channelNumber = 0;
            muxNumber = 1;
            break;
        case BeagleBone::P9_16:
            deviceNumber = 1;
            channelNumber = 1;
            muxNumber = 1;
            break;
        case BeagleBone::P9_21:
            deviceNumber = 0;
            channelNumber = 1;
            muxNumber = 1;
            break;
        case BeagleBone::P9_22:
            deviceNumber = 0;
            channelNumber = 0;
            muxNumber = 1;
            break;
        case BeagleBone::P9_29:
            deviceNumber = 0;
            channelNumber = 1;
            muxNumber = 0;
            break;
        case BeagleBone::P9_31:
            deviceNumber = 0;
            channelNumber = 0;
            muxNumber = 0;
            break;
        default:
            throw invalid_argument("BeagleBone::PWM: wrong pin number!");
            break;
    }
    
    // enable access to hardware IO
    
	ThreadCtl(_NTO_TCTL_IO, 0);
    ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
    
    // configure control module (pin muxing & global clock)
    
    baseAddress = mmap_device_io(CONTROL_MODULE_LENGTH, CONTROL_MODULE_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::PWM: couldn't remap control module register memory!");
    
    out32(baseAddress+CONTROL_MODULE_OFFSET_ADDRESS[deviceNumber][channelNumber][muxNumber], CONTROL_MODULE_PWM_MUX_MODE[deviceNumber][muxNumber]);
    
    uint32_t pwmSSCtrl = in32(baseAddress+CONTROL_MODULE_PWMSS_CTRL);
    pwmSSCtrl |= 1 << deviceNumber;
    out32(baseAddress+CONTROL_MODULE_PWMSS_CTRL, pwmSSCtrl);    // enable timebase clock for selected device
    
    munmap_device_io(baseAddress, CONTROL_MODULE_LENGTH);
    
    // enable peripheral clock module
    
    baseAddress = mmap_device_io(CLOCK_MODULE_PERIPHERAL_LENGTH, CLOCK_MODULE_PERIPHERAL_ADDRESS);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::PWM: couldn't remap clock module register memory!");
    
    out32(baseAddress+CLOCK_MODULE_PER_L4LS_CLKCTRL, CLOCK_MODULE_PER_L4LS_CLKCTRL_ENABLE);
    out32(baseAddress+CLOCK_MODULE_PER_EPWMSS_CLKCTRL[deviceNumber], CLOCK_MODULE_PER_EPWMSS_CLKCTRL_ENABLE);
    
    munmap_device_io(baseAddress, CLOCK_MODULE_PERIPHERAL_LENGTH);
    
    // configure pwm subsystem
    
    baseAddress = mmap_device_io(PWMSS_LENGTH, PWMSS_ADDRESS[deviceNumber]);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::PWM: couldn't remap PWM subsystem register memory!");
    
    out32(baseAddress+PWMSS_SYSCONFIG, 0x002B);
    out32(baseAddress+PWMSS_CLKCONFIG, 0x0111);
    
    munmap_device_io(baseAddress, PWMSS_LENGTH);
    
    // configure pwm controller
    
    baseAddress = mmap_device_io(EPWM_LENGTH, EPWM_ADDRESS[deviceNumber]);
    if (baseAddress == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone::PWM: couldn't remap PWM register memory!");
    
    period = DEFAULT_PERIOD;
    
    // configure time-base submodule
    
    out16(baseAddress+EPWM_TBCTL, 0x00B0);      // up-count mode
    out16(baseAddress+EPWM_TBSTS, 0x0006);
    out16(baseAddress+EPWM_TBPHSHR, 0x0000);
    out16(baseAddress+EPWM_TBPHS, 0x0000);      // set phase of this pwm
    out16(baseAddress+EPWM_TBCNT, 0x0000);      // clear actual counter
    out16(baseAddress+EPWM_TBPRD, period/NS_TO_TBCLK-1);
    
    // configure counter-compare submodule
    
    out16(baseAddress+EPWM_CMPCTL, 0x0000);     // load on CTR = 0
    out16(baseAddress+EPWM_CMPAHR, 0x0100);
    out16(baseAddress+EPWM_CMPA, period/NS_TO_TBCLK/2); // set this value by pulsewidth method
    out16(baseAddress+EPWM_CMPB, period/NS_TO_TBCLK/2); // set this value by pulsewidth method
    
    // configure action-qualifier submodule
    
    out16(baseAddress+EPWM_AQCTLA, 0x0012);     // set output high on zero, low on CMPA
    out16(baseAddress+EPWM_AQCTLB, 0x0102);     // set output high on zero, low on CMPB
    out16(baseAddress+EPWM_AQSFRC, 0x0000);
    out16(baseAddress+EPWM_AQCSFRC, 0x0000);
    
    // configure dead-band generator submodule
    
    out16(baseAddress+EPWM_DBCTL, 0x0000);
    out16(baseAddress+EPWM_DBRED, 0x0000);
    out16(baseAddress+EPWM_DBFED, 0x0000);
    
    // configure trip-zone submodule
    
    out16(baseAddress+EPWM_TZSEL, 0x0000);
    out16(baseAddress+EPWM_TZCTL, 0x000F);
    out16(baseAddress+EPWM_TZEINT, 0x0000);
    out16(baseAddress+EPWM_TZCLR, 0x0007);      // clear interrupts
    out16(baseAddress+EPWM_TZFRC, 0x0000);
    
    // configure interrupt enable
    
    out16(baseAddress+EPWM_ETSEL, 0x0000);
    out16(baseAddress+EPWM_ETPS, 0x0000);
    out16(baseAddress+EPWM_ETCLR, 0x0001);
    
    // configure chopper submodule
    
    out16(baseAddress+EPWM_PCCTL, 0x0000);
    
    // configure high resolution pwm submodule
    
    out16(baseAddress+EPWM_HRCTL, 0x0000);
    
    #endif
}

/**
 * Deletes the <code>BeagleBone::PWM</code> object and releases all allocated resources.
 */
BeagleBone::PWM::~PWM() {
    
    #if defined __QNX__
    
    munmap_device_io(baseAddress, EPWM_LENGTH);
    
    #endif
}

/**
 * Sets the period of the PWM signal.
 * @param ns the period of the signal, given in [ns].
 * Note that the maximum value for the period is 1310000 ns, which equals to 1.3 ms.
 */
void BeagleBone::PWM::setPeriod(uint32_t ns) {
    
    #if defined __QNX__
    
    period = ns;
    
    out16(baseAddress+EPWM_TBPRD, period/NS_TO_TBCLK-1);
    
    #endif
}

/**
 * Sets the pulsewidth of the PWM signal.
 * The resulting duty-cycle is the division of the pulsewidth with the period.
 * @param ns the pulsewidth of the signal, given in [ns].
 */
void BeagleBone::PWM::pulsewidth(uint32_t ns) {
    
    #if defined __QNX__
    
    if (channelNumber == 0) out16(baseAddress+EPWM_CMPA, ns/NS_TO_TBCLK);
    else if (channelNumber == 1) out16(baseAddress+EPWM_CMPB, ns/NS_TO_TBCLK);
    
    #endif
}

/**
 * Creates an object of the BeagleBone Black module.
 */
BeagleBone::BeagleBone() {
    
    #if defined __QNX__
    
    for (uint16_t number = 0; number < NUMBER_OF_PINS; number++) {
        
        gpioIndex[number] = 0;
        baseAddress[number] = 0;
        baseAddressLength[number] = 0;
    }
    
    #endif
}

/**
 * Deletes the <code>BeagleBone</code> object and releases all allocated resources.
 */
BeagleBone::~BeagleBone() {
    
    #if defined __QNX__
    
    for (uint16_t number = 0; number < NUMBER_OF_PINS; number++) {
        
        if (baseAddressLength[number] > 0) {
            
            munmap_device_io(baseAddress[number], baseAddressLength[number]);
        }
    }
    
    #endif
}

/**
 * This method is usually called by a DigitalIn object after it was created.
 * It configures the module for that DigitalIn channel. An internal pull-up resistor is enabled by default.
 * @param number the pin number of the digital input on the BeagleBone Black module.
 */
void BeagleBone::configureDigitalIn(uint16_t number) {
    
    #if defined __QNX__
    
    // check if the pin number is valid
    
    switch (number) {
        case P8_3: gpioIndex[number] = 4; break;
        case P8_4: gpioIndex[number] = 5; break;
        case P8_5: gpioIndex[number] = 6; break;
        case P8_6: gpioIndex[number] = 7; break;
        case P8_7: gpioIndex[number] = 8; break;
        case P8_8: gpioIndex[number] = 9; break;
        case P8_9: gpioIndex[number] = 10; break;
        case P8_10: gpioIndex[number] = 11; break;
        case P8_11: gpioIndex[number] = 12; break;
        case P8_12: gpioIndex[number] = 13; break;
        case P8_13: gpioIndex[number] = 14; break;
        case P8_14: gpioIndex[number] = 15; break;
        case P8_15: gpioIndex[number] = 16; break;
        case P8_16: gpioIndex[number] = 17; break;
        case P8_17: gpioIndex[number] = 18; break;
        case P8_18: gpioIndex[number] = 19; break;
        case P8_19: gpioIndex[number] = 20; break;
        case P8_20: gpioIndex[number] = 21; break;
        case P8_21: gpioIndex[number] = 22; break;
        case P8_22: gpioIndex[number] = 23; break;
        case P8_23: gpioIndex[number] = 24; break;
        case P8_24: gpioIndex[number] = 25; break;
        case P8_25: gpioIndex[number] = 26; break;
        case P8_26: gpioIndex[number] = 27; break;
        case P8_27: gpioIndex[number] = 28; break;
        case P8_28: gpioIndex[number] = 29; break;
        case P8_29: gpioIndex[number] = 30; break;
        case P8_30: gpioIndex[number] = 31; break;
        case P8_31: gpioIndex[number] = 32; break;
        case P8_32: gpioIndex[number] = 33; break;
        case P8_33: gpioIndex[number] = 34; break;
        case P8_34: gpioIndex[number] = 35; break;
        case P8_35: gpioIndex[number] = 36; break;
        case P8_36: gpioIndex[number] = 37; break;
        case P8_37: gpioIndex[number] = 38; break;
        case P8_38: gpioIndex[number] = 39; break;
        case P8_39: gpioIndex[number] = 40; break;
        case P8_40: gpioIndex[number] = 41; break;
        case P8_41: gpioIndex[number] = 42; break;
        case P8_42: gpioIndex[number] = 43; break;
        case P8_43: gpioIndex[number] = 44; break;
        case P8_44: gpioIndex[number] = 45; break;
        case P8_45: gpioIndex[number] = 46; break;
        case P8_46: gpioIndex[number] = 47; break;
        case P9_11: gpioIndex[number] = 48; break;
        case P9_12: gpioIndex[number] = 49; break;
        case P9_13: gpioIndex[number] = 50; break;
        case P9_14: gpioIndex[number] = 51; break;
        case P9_15: gpioIndex[number] = 52; break;
        case P9_16: gpioIndex[number] = 53; break;
        case P9_17: gpioIndex[number] = 54; break;
        case P9_18: gpioIndex[number] = 55; break;
        case P9_19: gpioIndex[number] = 56; break;
        case P9_20: gpioIndex[number] = 57; break;
        case P9_21: gpioIndex[number] = 58; break;
        case P9_22: gpioIndex[number] = 59; break;
        case P9_23: gpioIndex[number] = 60; break;
        case P9_24: gpioIndex[number] = 61; break;
        case P9_25: gpioIndex[number] = 62; break;
        case P9_26: gpioIndex[number] = 63; break;
        case P9_27: gpioIndex[number] = 64; break;
        case P9_28: gpioIndex[number] = 65; break;
        case P9_29: gpioIndex[number] = 66; break;
        case P9_30: gpioIndex[number] = 67; break;
        case P9_31: gpioIndex[number] = 68; break;
        case P9_41: gpioIndex[number] = 69; break;
        case P9_42: gpioIndex[number] = 70; break;
        default: throw invalid_argument("BeagleBone: wrong pin number for digital input!");
    }
    
    // enable access to hardware IO
    
	ThreadCtl(_NTO_TCTL_IO, 0);
    ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
    
    // configure pin muxing
    
    baseAddress[number] = mmap_device_io(CONTROL_MODULE_LENGTH, CONTROL_MODULE_ADDRESS);
    if (baseAddress[number] == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone: couldn't remap control module register memory!");
    
    out32(baseAddress[number]+CONTROL_MODULE_OFFSET_ADDRESS[gpioIndex[number]], CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_MUX_MODE7);  // confiuration of gpio on selected pin as input
    
    munmap_device_io(baseAddress[number], CONTROL_MODULE_LENGTH);
    
    // configure gpio controller
    
    baseAddress[number] = mmap_device_io(GPIO_LENGTH[GPIO_DEVICE_NUMBER[gpioIndex[number]]], GPIO_ADDRESS[GPIO_DEVICE_NUMBER[gpioIndex[number]]]);
    if (baseAddress[number] == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone: couldn't remap gpio register memory!");
    
    baseAddressLength[number] = GPIO_LENGTH[GPIO_DEVICE_NUMBER[gpioIndex[number]]];
    
    uint32_t gpioOE = in32(baseAddress[number]+GPIO_OE);
    gpioOE = gpioOE | (1 << GPIO_BIT_NUMBER[gpioIndex[number]]);
    out32(baseAddress[number]+GPIO_OE, gpioOE);     // configure selected gpio as input
    
    #endif
}

/**
 * This method is usually called by a DigitalIn object after it was created.
 * It configures the module for that DigitalIn channel.
 * @param number the pin number of the digital input on the BeagleBone Black module.
 * @param type the type of the digital input, either Floating, PullUp or PullDown.
 */
void BeagleBone::configureDigitalIn(uint16_t number, InputType type) {
    
    #if defined __QNX__
    
    // check if the pin number is valid
    
    switch (number) {
        case P8_3: gpioIndex[number] = 4; break;
        case P8_4: gpioIndex[number] = 5; break;
        case P8_5: gpioIndex[number] = 6; break;
        case P8_6: gpioIndex[number] = 7; break;
        case P8_7: gpioIndex[number] = 8; break;
        case P8_8: gpioIndex[number] = 9; break;
        case P8_9: gpioIndex[number] = 10; break;
        case P8_10: gpioIndex[number] = 11; break;
        case P8_11: gpioIndex[number] = 12; break;
        case P8_12: gpioIndex[number] = 13; break;
        case P8_13: gpioIndex[number] = 14; break;
        case P8_14: gpioIndex[number] = 15; break;
        case P8_15: gpioIndex[number] = 16; break;
        case P8_16: gpioIndex[number] = 17; break;
        case P8_17: gpioIndex[number] = 18; break;
        case P8_18: gpioIndex[number] = 19; break;
        case P8_19: gpioIndex[number] = 20; break;
        case P8_20: gpioIndex[number] = 21; break;
        case P8_21: gpioIndex[number] = 22; break;
        case P8_22: gpioIndex[number] = 23; break;
        case P8_23: gpioIndex[number] = 24; break;
        case P8_24: gpioIndex[number] = 25; break;
        case P8_25: gpioIndex[number] = 26; break;
        case P8_26: gpioIndex[number] = 27; break;
        case P8_27: gpioIndex[number] = 28; break;
        case P8_28: gpioIndex[number] = 29; break;
        case P8_29: gpioIndex[number] = 30; break;
        case P8_30: gpioIndex[number] = 31; break;
        case P8_31: gpioIndex[number] = 32; break;
        case P8_32: gpioIndex[number] = 33; break;
        case P8_33: gpioIndex[number] = 34; break;
        case P8_34: gpioIndex[number] = 35; break;
        case P8_35: gpioIndex[number] = 36; break;
        case P8_36: gpioIndex[number] = 37; break;
        case P8_37: gpioIndex[number] = 38; break;
        case P8_38: gpioIndex[number] = 39; break;
        case P8_39: gpioIndex[number] = 40; break;
        case P8_40: gpioIndex[number] = 41; break;
        case P8_41: gpioIndex[number] = 42; break;
        case P8_42: gpioIndex[number] = 43; break;
        case P8_43: gpioIndex[number] = 44; break;
        case P8_44: gpioIndex[number] = 45; break;
        case P8_45: gpioIndex[number] = 46; break;
        case P8_46: gpioIndex[number] = 47; break;
        case P9_11: gpioIndex[number] = 48; break;
        case P9_12: gpioIndex[number] = 49; break;
        case P9_13: gpioIndex[number] = 50; break;
        case P9_14: gpioIndex[number] = 51; break;
        case P9_15: gpioIndex[number] = 52; break;
        case P9_16: gpioIndex[number] = 53; break;
        case P9_17: gpioIndex[number] = 54; break;
        case P9_18: gpioIndex[number] = 55; break;
        case P9_19: gpioIndex[number] = 56; break;
        case P9_20: gpioIndex[number] = 57; break;
        case P9_21: gpioIndex[number] = 58; break;
        case P9_22: gpioIndex[number] = 59; break;
        case P9_23: gpioIndex[number] = 60; break;
        case P9_24: gpioIndex[number] = 61; break;
        case P9_25: gpioIndex[number] = 62; break;
        case P9_26: gpioIndex[number] = 63; break;
        case P9_27: gpioIndex[number] = 64; break;
        case P9_28: gpioIndex[number] = 65; break;
        case P9_29: gpioIndex[number] = 66; break;
        case P9_30: gpioIndex[number] = 67; break;
        case P9_31: gpioIndex[number] = 68; break;
        case P9_41: gpioIndex[number] = 69; break;
        case P9_42: gpioIndex[number] = 70; break;
        default: throw invalid_argument("BeagleBone: wrong pin number for digital input!");
    }
    
    // enable access to hardware IO
    
	ThreadCtl(_NTO_TCTL_IO, 0);
    ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
    
    // configure pin muxing
    
    baseAddress[number] = mmap_device_io(CONTROL_MODULE_LENGTH, CONTROL_MODULE_ADDRESS);
    if (baseAddress[number] == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone: couldn't remap control module register memory!");
    
    switch (type) {
        case Floating:
            out32(baseAddress[number]+CONTROL_MODULE_OFFSET_ADDRESS[gpioIndex[number]], CONTROL_MODULE_PU_DEN | CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_MUX_MODE7);  // confiuration of gpio on selected pin as input
            break;
        case PullUp:
            out32(baseAddress[number]+CONTROL_MODULE_OFFSET_ADDRESS[gpioIndex[number]], CONTROL_MODULE_PU_TYPE_SEL | CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_MUX_MODE7);  // confiuration of gpio on selected pin as input
            break;
        case PullDown:
            out32(baseAddress[number]+CONTROL_MODULE_OFFSET_ADDRESS[gpioIndex[number]], CONTROL_MODULE_RX_ACTIVE | CONTROL_MODULE_MUX_MODE7);  // confiuration of gpio on selected pin as input
            break;
    }
    
    munmap_device_io(baseAddress[number], CONTROL_MODULE_LENGTH);
    
    // configure gpio controller
    
    baseAddress[number] = mmap_device_io(GPIO_LENGTH[GPIO_DEVICE_NUMBER[gpioIndex[number]]], GPIO_ADDRESS[GPIO_DEVICE_NUMBER[gpioIndex[number]]]);
    if (baseAddress[number] == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone: couldn't remap gpio register memory!");
    
    baseAddressLength[number] = GPIO_LENGTH[GPIO_DEVICE_NUMBER[gpioIndex[number]]];
    
    uint32_t gpioOE = in32(baseAddress[number]+GPIO_OE);
    gpioOE = gpioOE | (1 << GPIO_BIT_NUMBER[gpioIndex[number]]);
    out32(baseAddress[number]+GPIO_OE, gpioOE);     // configure selected gpio as input
    
    #endif
}

/**
 * This method is usually called by a DigitalOut object after it was created.
 * It configures the module for that DigitalOut channel.
 * @param number the pin number of the digital output on the BeagleBone Black module.
 */
void BeagleBone::configureDigitalOut(uint16_t number) {
    
    #if defined __QNX__
    
    // check if the pin number is valid
    
    switch (number) {
        case LED0: gpioIndex[number] = 0; break;
        case LED1: gpioIndex[number] = 1; break;
        case LED2: gpioIndex[number] = 2; break;
        case LED3: gpioIndex[number] = 3; break;
        case P8_3: gpioIndex[number] = 4; break;
        case P8_4: gpioIndex[number] = 5; break;
        case P8_5: gpioIndex[number] = 6; break;
        case P8_6: gpioIndex[number] = 7; break;
        case P8_7: gpioIndex[number] = 8; break;
        case P8_8: gpioIndex[number] = 9; break;
        case P8_9: gpioIndex[number] = 10; break;
        case P8_10: gpioIndex[number] = 11; break;
        case P8_11: gpioIndex[number] = 12; break;
        case P8_12: gpioIndex[number] = 13; break;
        case P8_13: gpioIndex[number] = 14; break;
        case P8_14: gpioIndex[number] = 15; break;
        case P8_15: gpioIndex[number] = 16; break;
        case P8_16: gpioIndex[number] = 17; break;
        case P8_17: gpioIndex[number] = 18; break;
        case P8_18: gpioIndex[number] = 19; break;
        case P8_19: gpioIndex[number] = 20; break;
        case P8_20: gpioIndex[number] = 21; break;
        case P8_21: gpioIndex[number] = 22; break;
        case P8_22: gpioIndex[number] = 23; break;
        case P8_23: gpioIndex[number] = 24; break;
        case P8_24: gpioIndex[number] = 25; break;
        case P8_25: gpioIndex[number] = 26; break;
        case P8_26: gpioIndex[number] = 27; break;
        case P8_27: gpioIndex[number] = 28; break;
        case P8_28: gpioIndex[number] = 29; break;
        case P8_29: gpioIndex[number] = 30; break;
        case P8_30: gpioIndex[number] = 31; break;
        case P8_31: gpioIndex[number] = 32; break;
        case P8_32: gpioIndex[number] = 33; break;
        case P8_33: gpioIndex[number] = 34; break;
        case P8_34: gpioIndex[number] = 35; break;
        case P8_35: gpioIndex[number] = 36; break;
        case P8_36: gpioIndex[number] = 37; break;
        case P8_37: gpioIndex[number] = 38; break;
        case P8_38: gpioIndex[number] = 39; break;
        case P8_39: gpioIndex[number] = 40; break;
        case P8_40: gpioIndex[number] = 41; break;
        case P8_41: gpioIndex[number] = 42; break;
        case P8_42: gpioIndex[number] = 43; break;
        case P8_43: gpioIndex[number] = 44; break;
        case P8_44: gpioIndex[number] = 45; break;
        case P8_45: gpioIndex[number] = 46; break;
        case P8_46: gpioIndex[number] = 47; break;
        case P9_11: gpioIndex[number] = 48; break;
        case P9_12: gpioIndex[number] = 49; break;
        case P9_13: gpioIndex[number] = 50; break;
        case P9_14: gpioIndex[number] = 51; break;
        case P9_15: gpioIndex[number] = 52; break;
        case P9_16: gpioIndex[number] = 53; break;
        case P9_17: gpioIndex[number] = 54; break;
        case P9_18: gpioIndex[number] = 55; break;
        case P9_19: gpioIndex[number] = 56; break;
        case P9_20: gpioIndex[number] = 57; break;
        case P9_21: gpioIndex[number] = 58; break;
        case P9_22: gpioIndex[number] = 59; break;
        case P9_23: gpioIndex[number] = 60; break;
        case P9_24: gpioIndex[number] = 61; break;
        case P9_25: gpioIndex[number] = 62; break;
        case P9_26: gpioIndex[number] = 63; break;
        case P9_27: gpioIndex[number] = 64; break;
        case P9_28: gpioIndex[number] = 65; break;
        case P9_29: gpioIndex[number] = 66; break;
        case P9_30: gpioIndex[number] = 67; break;
        case P9_31: gpioIndex[number] = 68; break;
        case P9_41: gpioIndex[number] = 69; break;
        case P9_42: gpioIndex[number] = 70; break;
        default: throw invalid_argument("BeagleBone: wrong pin number for digital output!");
    }
    
    // enable access to hardware IO
    
    ThreadCtl(_NTO_TCTL_IO, 0);
    ThreadCtl(_NTO_TCTL_IO_PRIV, 0);
    
    // configure pin muxing
    
    baseAddress[number] = mmap_device_io(CONTROL_MODULE_LENGTH, CONTROL_MODULE_ADDRESS);
    if (baseAddress[number] == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone: couldn't remap control module register memory!");
    
    out32(baseAddress[number]+CONTROL_MODULE_OFFSET_ADDRESS[gpioIndex[number]], CONTROL_MODULE_MUX_MODE7);  // confiuration of gpio on selected pin as output
    
    munmap_device_io(baseAddress[number], CONTROL_MODULE_LENGTH);
    
    // configure gpio controller
    
    baseAddress[number] = mmap_device_io(GPIO_LENGTH[GPIO_DEVICE_NUMBER[gpioIndex[number]]], GPIO_ADDRESS[GPIO_DEVICE_NUMBER[gpioIndex[number]]]);
    if (baseAddress[number] == MAP_DEVICE_FAILED) throw runtime_error("BeagleBone: couldn't remap gpio register memory!");
    
    baseAddressLength[number] = GPIO_LENGTH[GPIO_DEVICE_NUMBER[gpioIndex[number]]];
    
    out32(baseAddress[number]+GPIO_CLEARDATAOUT, 1 << GPIO_BIT_NUMBER[gpioIndex[number]]);  // set default output to low
    
    uint32_t gpioOE = in32(baseAddress[number]+GPIO_OE);
    gpioOE = gpioOE & ~(1 << GPIO_BIT_NUMBER[gpioIndex[number]]);
    out32(baseAddress[number]+GPIO_OE, gpioOE);     // configure selected gpio as output
    
    #endif
}

/**
 * This method reads a digital input. It is usually called by a DigitalIn object.
 * @param number the pin number of the digital input on the BeagleBone Black module.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool BeagleBone::readDigitalIn(uint16_t number) {
    
    #if defined __QNX__
    
    uint32_t dataIn = in32(baseAddress[number]+GPIO_DATAIN);
    
    return (dataIn & (1 << GPIO_BIT_NUMBER[gpioIndex[number]])) > 0;
    
    #else
    
    return false;
    
    #endif
}

/**
 * This method writes a digital output. It is usually called by a DigitalOut object.
 * @param number the pin number of the digital output on the BeagleBone Black module.
 * @param value the value of the digital output, given as a bool value (either <code>true</code> or <code>false</code>).
 */
void BeagleBone::writeDigitalOut(uint16_t number, bool value) {
    
    #if defined __QNX__
    
    if (value) {
        out32(baseAddress[number]+GPIO_SETDATAOUT, 1 << GPIO_BIT_NUMBER[gpioIndex[number]]);
    } else {
        out32(baseAddress[number]+GPIO_CLEARDATAOUT, 1 << GPIO_BIT_NUMBER[gpioIndex[number]]);
    }
    
    #endif
}
