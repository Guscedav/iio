/*
 * BeagleBone.h
 * Copyright (c) 2016, ZHAW
 * All rights reserved.
 *
 *  Created on: 10.08.2016
 *      Author: Marcel Honegger
 */

#ifndef BEAGLE_BONE_H_
#define BEAGLE_BONE_H_

#include <cstdlib>
#include <string>
#include <iostream>
#include <stdint.h>
#include <pthread.h>
#include "Module.h"
#include "CAN.h"
#include "Mutex.h"

class CANMessage;

/**
 * The <code>BeagleBone</code> class implements the device drivers for several
 * periphery controllers of the Texas Instruments BeagleBone Black CPU module.
 * <br/>
 * <div style="text-align:center"><img src="beaglebone.jpg" width="400"/></div>
 * <br/>
 * <div style="text-align:center"><b>The TI BeagleBone Black CPU module</b></div>
 * <br/>
 * The pin assignment of the various device drivers on the BeagleBone Black module is
 * shown in the illustration below:
 * <br/>
 * <div style="text-align:center"><img src="beaglebonepins.png" width="800"/></div>
 * <br/>
 * <div style="text-align:center"><b>BeagleBone Black pin assignment</b></div>
 * <br/>
 * <br/>
 * The following example shows how to use digital input and output channels on this module:
 * <pre><code>
 * BeagleBone beagleBone;
 * DigitalOut led(beagleBone, BeagleBone::LED0);     <span style="color:#008000">// create a digital output</span>
 * DigitalIn switch(beagleBone, BeagleBone::P8_20);  <span style="color:#008000">// create a digital input</span>
 *
 * led = true;            <span style="color:#008000">// use of the output and input channels</span>
 * if (switch) { ... }
 * </code></pre>
 * See the documentation of the DigitalIn and DigitalOut classes for more information.
 */
class BeagleBone : public Module {
    
    public:
        
        /**
         * The <code>BeagleBone::SPI</code> class implements the driver for the serial port interface (McSPI)
         * controller in the TI Sitara processor of the BeagleBone module.
         */
        class SPI {
            
            public:
                
                            SPI(uint16_t mosiPin, uint16_t misoPin, uint16_t sclkPin, uint16_t cs0Pin, uint16_t cs1Pin);
                virtual     ~SPI();
                void        format(uint8_t bits, uint8_t mode);
                void        frequency(uint32_t hz);
                uint32_t    write(uint32_t value);
                
            private:
                
                // control module registers
                
                static const uint32_t   CONTROL_MODULE_ADDRESS = 0x44E10000;
                static const size_t     CONTROL_MODULE_LENGTH = 0x2000;
                
                static const uint32_t   CONTROL_MODULE_SPI0_SCLK = 0x0950;
                static const uint32_t   CONTROL_MODULE_SPI0_CS0 = 0x095C;
                static const uint32_t   CONTROL_MODULE_SPI0_CS1 = 0x0960;
                static const uint32_t   CONTROL_MODULE_MCASP0_AXR0_MUX0 = 0x0998;
                static const uint32_t   CONTROL_MODULE_MCASP0_AHCLKR_MUX0 = 0x099C;
                static const uint32_t   CONTROL_MODULE_MCASP0_FSX_MUX0 = 0x0994;
                static const uint32_t   CONTROL_MODULE_UART0_TXD = 0x0974;
                static const uint32_t   CONTROL_MODULE_UART0_RXD = 0x0970;
                static const uint32_t   CONTROL_MODULE_SPI0_D0 = 0x0954;
                static const uint32_t   CONTROL_MODULE_SPI0_D1 = 0x0958;
                static const uint32_t   CONTROL_MODULE_UART1_CTSN = 0x0978;
                static const uint32_t   CONTROL_MODULE_UART1_RTSN = 0x097C;
                static const uint32_t   CONTROL_MODULE_MCASP0_ACLKX_MUX0 = 0x0990;
                static const uint32_t   CONTROL_MODULE_ECAP0_IN_PWM0_OUT = 0x0964;
                static const uint32_t   CONTROL_MODULE_UART0_RTSN = 0x096C;
                static const uint32_t   CONTROL_MODULE_UART0_CTSN = 0x0968;
                static const uint32_t   CONTROL_MODULE_GMII1_RXER = 0x0910;
                static const uint32_t   CONTROL_MODULE_GMII1_COL = 0x0908;
                static const uint32_t   CONTROL_MODULE_GMII1_CRS = 0x090C;
                static const uint32_t   CONTROL_MODULE_RMII1_REFCLK = 0x0944;
                static const uint32_t   CONTROL_MODULE_XDMA_EVENT_INTR0 = 0x09B0;
                
                static const uint32_t   CONTROL_MODULE_PU_TYPE_SEL = 0x00000010;
                static const uint32_t   CONTROL_MODULE_RX_ACTIVE = 0x00000020;
                static const uint32_t   CONTROL_MODULE_MUX_MODE0 = 0x00000000;
                static const uint32_t   CONTROL_MODULE_MUX_MODE1 = 0x00000001;
                static const uint32_t   CONTROL_MODULE_MUX_MODE2 = 0x00000002;
                static const uint32_t   CONTROL_MODULE_MUX_MODE3 = 0x00000003;
                static const uint32_t   CONTROL_MODULE_MUX_MODE4 = 0x00000004;
                static const uint32_t   CONTROL_MODULE_MUX_MODE5 = 0x00000005;
                static const uint32_t   CONTROL_MODULE_MUX_MODE6 = 0x00000006;
                static const uint32_t   CONTROL_MODULE_MUX_MODE7 = 0x00000007;
                
                static const uint8_t    CONTROL_MODULE_NUMBER_OF_MCSPI_MUX = 5;
                
                static const uint32_t   CONTROL_MODULE_MOSI_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_MISO_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_SCLK_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_CS0_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_CS1_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                
                static const uint32_t   CONTROL_MODULE_MOSI_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_MISO_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_SCLK_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_CS0_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                static const uint32_t   CONTROL_MODULE_CS1_PIN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_MCSPI_MUX];
                
                // clock module registers
            
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_ADDRESS = 0x44E00000;
                static const size_t     CLOCK_MODULE_PERIPHERAL_LENGTH = 0x4000;
                
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_SPI_CLKCTRL[];
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_SPI_CLKCTRL_ENABLE[];
                
                // mcspi controller registers

                static const uint32_t   MCSPI_ADDRESS[];
                static const size_t     MCSPI_LENGTH[];
                
                static const uint32_t   MCSPI_REVISION = 0x0000;
                static const uint32_t   MCSPI_SYSCONFIG = 0x0110;
                static const uint32_t   MCSPI_SYSSTATUS = 0x0114;
                static const uint32_t   MCSPI_IRQSTATUS = 0x0118;
                static const uint32_t   MCSPI_IRQENABLE = 0x011C;
                static const uint32_t   MCSPI_SYST = 0x0124;
                static const uint32_t   MCSPI_MODULCTRL = 0x0128;
                static const uint32_t   MCSPI_CH0CONF = 0x012C;
                static const uint32_t   MCSPI_CH0STAT = 0x0130;
                static const uint32_t   MCSPI_CH0CTRL = 0x0134;
                static const uint32_t   MCSPI_TX0 = 0x0138;
                static const uint32_t   MCSPI_RX0 = 0x013C;
                static const uint32_t   MCSPI_CH1CONF = 0x0140;
                static const uint32_t   MCSPI_CH1STAT = 0x0144;
                static const uint32_t   MCSPI_CH1CTRL = 0x0148;
                static const uint32_t   MCSPI_TX1 = 0x014C;
                static const uint32_t   MCSPI_RX1 = 0x0150;
                static const uint32_t   MCSPI_CH2CONF = 0x0154;
                static const uint32_t   MCSPI_CH2STAT = 0x0158;
                static const uint32_t   MCSPI_CH2CTRL = 0x015C;
                static const uint32_t   MCSPI_TX2 = 0x0160;
                static const uint32_t   MCSPI_RX2 = 0x0164;
                static const uint32_t   MCSPI_CH3CONF = 0x0168;
                static const uint32_t   MCSPI_CH3STAT = 0x016C;
                static const uint32_t   MCSPI_CH3CTRL = 0x0170;
                static const uint32_t   MCSPI_TX3 = 0x0174;
                static const uint32_t   MCSPI_RX3 = 0x0178;
                static const uint32_t   MCSPI_XFERLEVEL = 0x017C;
                static const uint32_t   MCSPI_DAFTX = 0x0180;
                static const uint32_t   MCSPI_DAFRX = 0x01A0;
                
                uint8_t     deviceNumber;
                uintptr_t   baseAddress;
        };
        
        /**
         * The <code>BeagleBone::DCAN</code> class implements a driver for the DCAN controller in the
         * TI Sitara processor. It offers methods to transmit and receive CAN messages.
         */
        class DCAN : public CAN {
            
            public:
                
                                DCAN(uint16_t rxPin, uint16_t txPin);
                virtual         ~DCAN();
                void            frequency(uint32_t hz);
                int32_t         write(CANMessage canMessage);
                int32_t         read(CANMessage& canMessage);
                
            private:
                
                // control module registers
                
                static const uint32_t   CONTROL_MODULE_ADDRESS = 0x44E10000;
                static const size_t     CONTROL_MODULE_LENGTH = 0x2000;
                
                static const uint32_t   CONTROL_MODULE_DCAN_RAMINIT = 0X0644;
                static const uint32_t   CONTROL_MODULE_DCAN_RAMINIT_START[];
                static const uint32_t   CONTROL_MODULE_DCAN_RAMINIT_DONE[];
                static const uint8_t    CONTROL_MODULE_NUMBER_OF_DCAN_MUX = 3;
                static const uint32_t   CONTROL_MODULE_RX_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_DCAN_MUX];
                static const uint32_t   CONTROL_MODULE_TX_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_DCAN_MUX];
                static const uint32_t   CONTROL_MODULE_DCAN_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_DCAN_MUX];
                
                static const uint32_t   CONTROL_MODULE_PU_TYPE_SEL = 0x00000010;
                static const uint32_t   CONTROL_MODULE_RX_ACTIVE = 0x00000020;
                static const uint32_t   CONTROL_MODULE_MUX_MODE0 = 0x00000000;
                static const uint32_t   CONTROL_MODULE_MUX_MODE1 = 0x00000001;
                static const uint32_t   CONTROL_MODULE_MUX_MODE2 = 0x00000002;
                static const uint32_t   CONTROL_MODULE_MUX_MODE3 = 0x00000003;
                static const uint32_t   CONTROL_MODULE_MUX_MODE4 = 0x00000004;
                static const uint32_t   CONTROL_MODULE_MUX_MODE5 = 0x00000005;
                static const uint32_t   CONTROL_MODULE_MUX_MODE6 = 0x00000006;
                static const uint32_t   CONTROL_MODULE_MUX_MODE7 = 0x00000007;
                
                // clock module registers
                
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_ADDRESS = 0x44E00000;
                static const size_t     CLOCK_MODULE_PERIPHERAL_LENGTH = 0x4000;
                
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_DCAN_CLKCTRL[];
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_DCAN_CLKCTRL_ENABLE[];
                
                // dcan controller registers
                
                static const uint32_t   DCAN_ADDRESS[];
                static const size_t     DCAN_LENGTH[];
                
                static const uint32_t   DCAN_CTL = 0x0000;
                static const uint32_t   DCAN_ES = 0x0004;
                static const uint32_t   DCAN_ERRC = 0x0008;
                static const uint32_t   DCAN_BTR = 0x000C;
                static const uint32_t   DCAN_INT = 0x0010;
                static const uint32_t   DCAN_TEST = 0x0014;
                static const uint32_t   DCAN_PERR = 0x001C;
                static const uint32_t   DCAN_ABOTR = 0x0080;
                static const uint32_t   DCAN_TXRQ_X = 0x0084;
                static const uint32_t   DCAN_TXRQ12 = 0x0088;
                static const uint32_t   DCAN_TXRQ34 = 0x008C;
                static const uint32_t   DCAN_TXRQ56 = 0x0090;
                static const uint32_t   DCAN_TXRQ78 = 0x0094;
                static const uint32_t   DCAN_NWDAT_X = 0x0098;
                static const uint32_t   DCAN_NWDAT12 = 0x009C;
                static const uint32_t   DCAN_NWDAT34 = 0x00A0;
                static const uint32_t   DCAN_NWDAT56 = 0x00A4;
                static const uint32_t   DCAN_NWDAT78 = 0x00A8;
                static const uint32_t   DCAN_INTPND_X = 0x00AC;
                static const uint32_t   DCAN_INTPND12 = 0x00B0;
                static const uint32_t   DCAN_INTPND34 = 0x00B4;
                static const uint32_t   DCAN_INTPND56 = 0x00B8;
                static const uint32_t   DCAN_INTPND78 = 0x00BC;
                static const uint32_t   DCAN_MSGVAL_X = 0x00C0;
                static const uint32_t   DCAN_MSGVAL12 = 0x00C4;
                static const uint32_t   DCAN_MSGVAL34 = 0x00C8;
                static const uint32_t   DCAN_MSGVAL56 = 0x00CC;
                static const uint32_t   DCAN_MSGVAL78 = 0x00D0;
                static const uint32_t   DCAN_INTMUX12 = 0x00D8;
                static const uint32_t   DCAN_INTMUX34 = 0x00DC;
                static const uint32_t   DCAN_INTMUX56 = 0x00E0;
                static const uint32_t   DCAN_INTMUX78 = 0x00E4;
                static const uint32_t   DCAN_IF1CMD = 0x0100;
                static const uint32_t   DCAN_IF1MSK = 0x0104;
                static const uint32_t   DCAN_IF1ARB = 0x0108;
                static const uint32_t   DCAN_IF1MCTL = 0x010C;
                static const uint32_t   DCAN_IF1DATA = 0x0110;
                static const uint32_t   DCAN_IF1DATB = 0x0114;
                static const uint32_t   DCAN_IF2CMD = 0x0120;
                static const uint32_t   DCAN_IF2MSK = 0x0124;
                static const uint32_t   DCAN_IF2ARB = 0x0128;
                static const uint32_t   DCAN_IF2MCTL = 0x012C;
                static const uint32_t   DCAN_IF2DATA = 0x0130;
                static const uint32_t   DCAN_IF2DATB = 0x0134;
                static const uint32_t   DCAN_IF3OBS = 0x0140;
                static const uint32_t   DCAN_IF3MSK = 0x0144;
                static const uint32_t   DCAN_IF3ARB = 0x0148;
                static const uint32_t   DCAN_IF3MCTL = 0x014C;
                static const uint32_t   DCAN_IF3DATA = 0x0150;
                static const uint32_t   DCAN_IF3DATB = 0x0154;
                static const uint32_t   DCAN_IF3UPD12 = 0x0160;
                static const uint32_t   DCAN_IF3UPD34 = 0x0164;
                static const uint32_t   DCAN_IF3UPD56 = 0x0168;
                static const uint32_t   DCAN_IF3UPD78 = 0x016C;
                static const uint32_t   DCAN_TIOC = 0x01E0;
                static const uint32_t   DCAN_RIOC = 0x01E4;
                
                static const uint32_t   DCAN_CTL_INIT = 0x00000001;
                static const uint32_t   DCAN_CTL_IE0 = 0x00000002;
                static const uint32_t   DCAN_CTL_SIE = 0x00000004;
                static const uint32_t   DCAN_CTL_EIE = 0x00000008;
                static const uint32_t   DCAN_CTL_DAR = 0x00000020;
                static const uint32_t   DCAN_CTL_CCE = 0x00000040;
                static const uint32_t   DCAN_CTL_TEST = 0x00000080;
                static const uint32_t   DCAN_CTL_ABO = 0x00000200;
                static const uint32_t   DCAN_CTL_IE1 = 0x00020000;
                
                static const uint32_t   DCAN_BTR_SJW_SHIFT = 6;
                static const uint32_t   DCAN_BTR_TSEG1_SHIFT = 8;
                static const uint32_t   DCAN_BTR_TSEG2_SHIFT = 12;
                
                static const uint32_t   DCAN_IFXCMD_MESSAGE_TX_MIN = 1;
                static const uint32_t   DCAN_IFXCMD_MESSAGE_TX_MAX = 32;
                static const uint32_t   DCAN_IFXCMD_MESSAGE_TX_REMOTE_MIN = 33;
                static const uint32_t   DCAN_IFXCMD_MESSAGE_TX_REMOTE_MAX = 36;
                static const uint32_t   DCAN_IFXCMD_MESSAGE_RX_REMOTE_MIN = 37;
                static const uint32_t   DCAN_IFXCMD_MESSAGE_RX_REMOTE_MAX = 40;
                static const uint32_t   DCAN_IFXCMD_MESSAGE_RX_MIN = 41;
                static const uint32_t   DCAN_IFXCMD_MESSAGE_RX_MAX = 64;
                static const uint32_t   DCAN_IFXCMD_WRITE = 1 << 23;
                static const uint32_t   DCAN_IFXCMD_MASK = 1 << 22;
                static const uint32_t   DCAN_IFXCMD_ARB = 1 << 21;
                static const uint32_t   DCAN_IFXCMD_CONTROL = 1 << 20;
                static const uint32_t   DCAN_IFXCMD_CLR_INT_PND = 1 << 19;
                static const uint32_t   DCAN_IFXCMD_TX_RQST = 1 << 18;
                static const uint32_t   DCAN_IFXCMD_DATA = 1 << 17;
                static const uint32_t   DCAN_IFXCMD_DATB = 1 << 16;
                static const uint32_t   DCAN_IFXCMD_BUSY = 1 << 15;
                
                static const uint32_t   DCAN_IFXMSK_DEFAULT = 0x60000000;
                
                static const uint32_t   DCAN_IFXARB_MSG_VAL = 0x80000000;
                static const uint32_t   DCAN_IFXARB_DIR_TRANSMIT = 0x20000000;
                static const uint32_t   DCAN_IFXARB_ID_SHIFT = 18;
                static const uint32_t   DCAN_IFXARB_ID_MASK = 0x000007FF;
                
                static const uint32_t   DCAN_IFXMCTL_NEW_DAT = 0x00008000;
                static const uint32_t   DCAN_IFXMCTL_UMASK = 0x00001000;
                static const uint32_t   DCAN_IFXMCTL_RX_IE = 0x00000400;
                static const uint32_t   DCAN_IFXMCTL_TX_RQST = 0x00000100;
                static const uint32_t   DCAN_IFXMCTL_EOB = 0x00000080;
                static const uint32_t   DCAN_IFXMCTL_LEN_MASK = 0x0000000F;
                
                uint8_t             deviceNumber;
                uintptr_t           baseAddress;
                Mutex               mutex;          // mutex to lock critical sections
        };
        
        /**
         * The <code>BeagleBone::QEP</code> class is used to read the value of a quadrature encoder
         * counter in the TI Sitara processor of the BeagleBone module.
         */
        class QEP {
            
            public:
                
                            QEP(uint16_t aPin, uint16_t bPin, uint16_t indexPin, uint16_t strobePin);
                virtual		~QEP();
                void        setGain(int32_t gain);
                int32_t     read();
                int32_t     readCounterAtIndexPulse();
                
            private:
                
                // control module registers
                
                static const uint32_t   CONTROL_MODULE_ADDRESS = 0x44E10000;
                static const size_t     CONTROL_MODULE_LENGTH = 0x2000;
                
                static const uint8_t    CONTROL_MODULE_NUMBER_OF_QEP_MUX = 2;
                static const uint32_t   CONTROL_MODULE_A_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX];
                static const uint32_t   CONTROL_MODULE_B_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX];
                static const uint32_t   CONTROL_MODULE_INDEX_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX];
                static const uint32_t   CONTROL_MODULE_STROBE_PIN_OFFSET_ADDRESS[][CONTROL_MODULE_NUMBER_OF_QEP_MUX];
                static const uint32_t   CONTROL_MODULE_QEP_MUX_MODE[][CONTROL_MODULE_NUMBER_OF_QEP_MUX];
                
                static const uint32_t   CONTROL_MODULE_PU_TYPE_SEL = 0x00000010;
                static const uint32_t   CONTROL_MODULE_RX_ACTIVE = 0x00000020;
                static const uint32_t   CONTROL_MODULE_MUX_MODE0 = 0x00000000;
                static const uint32_t   CONTROL_MODULE_MUX_MODE1 = 0x00000001;
                static const uint32_t   CONTROL_MODULE_MUX_MODE2 = 0x00000002;
                static const uint32_t   CONTROL_MODULE_MUX_MODE3 = 0x00000003;
                static const uint32_t   CONTROL_MODULE_MUX_MODE4 = 0x00000004;
                static const uint32_t   CONTROL_MODULE_MUX_MODE5 = 0x00000005;
                static const uint32_t   CONTROL_MODULE_MUX_MODE6 = 0x00000006;
                static const uint32_t   CONTROL_MODULE_MUX_MODE7 = 0x00000007;
                
                // clock module registers
                
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_ADDRESS = 0x44E00000;
                static const size_t     CLOCK_MODULE_PERIPHERAL_LENGTH = 0x4000;
                
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_EPWMSS_CLKCTRL[];
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_EPWMSS_CLKCTRL_ENABLE = 0x00000002;
                
                // qep module registers
                
                static const uint32_t   QEP_ADDRESS[];
                static const size_t     QEP_LENGTH[];
                
                static const uint32_t   QEP_QPOSCNT = 0x0000;
                static const uint32_t   QEP_QPOSINIT = 0x0004;
                static const uint32_t   QEP_QPOSMAX = 0x0008;
                static const uint32_t   QEP_QPOSCMP = 0x000C;
                static const uint32_t   QEP_QPOSILAT = 0x0010;
                static const uint32_t   QEP_QPOSSLAT = 0x0014;
                static const uint32_t   QEP_QPOSLAT = 0x0018;
                static const uint32_t   QEP_QUTMR = 0x001C;
                static const uint32_t   QEP_QUPRD = 0x0020;
                static const uint32_t   QEP_QWDTMR = 0x0024;
                static const uint32_t   QEP_QWDPRD = 0x0026;
                static const uint32_t   QEP_QDECCTL = 0x0028;
                static const uint32_t   QEP_QEPCTL = 0x002A;
                static const uint32_t   QEP_QCAPCTL = 0x002C;
                static const uint32_t   QEP_QPOSCTL = 0x002E;
                static const uint32_t   QEP_QEINT = 0x0030;
                static const uint32_t   QEP_QFLG = 0x0032;
                static const uint32_t   QEP_QCLR = 0x0034;
                static const uint32_t   QEP_QFRC = 0x0036;
                static const uint32_t   QEP_QEPSTS = 0x0038;
                static const uint32_t   QEP_QCTMR = 0x003A;
                static const uint32_t   QEP_QCPRD = 0x003C;
                static const uint32_t   QEP_QCTMRLAT = 0x003E;
                static const uint32_t   QEP_QCPRDLAT = 0x0040;
                static const uint32_t   QEP_REVID = 0x005C;
                
                uint8_t         deviceNumber;
                uintptr_t       baseAddress;
                int32_t         gain;
        };
        
        /**
         * The <code>BeagleBone::PWM</code> class is used to configure and set a PWM output pin
         * of the BeagleBone module.
         */
        class PWM {
            
            public:
                
                            PWM(uint16_t pin);
                virtual		~PWM();
                void        setPeriod(uint32_t ns);
                void        pulsewidth(uint32_t ns);

            private:
                
                // control module registers
                
                static const uint32_t   CONTROL_MODULE_ADDRESS = 0x44E10000;
                static const size_t     CONTROL_MODULE_LENGTH = 0x2000;
                
                static const uint32_t   CONTROL_MODULE_PU_TYPE_SEL = 0x00000010;
                static const uint32_t   CONTROL_MODULE_RX_ACTIVE = 0x00000020;
                static const uint32_t   CONTROL_MODULE_MUX_MODE0 = 0x00000000;
                static const uint32_t   CONTROL_MODULE_MUX_MODE1 = 0x00000001;
                static const uint32_t   CONTROL_MODULE_MUX_MODE2 = 0x00000002;
                static const uint32_t   CONTROL_MODULE_MUX_MODE3 = 0x00000003;
                static const uint32_t   CONTROL_MODULE_MUX_MODE4 = 0x00000004;
                static const uint32_t   CONTROL_MODULE_MUX_MODE5 = 0x00000005;
                static const uint32_t   CONTROL_MODULE_MUX_MODE6 = 0x00000006;
                static const uint32_t   CONTROL_MODULE_MUX_MODE7 = 0x00000007;
                
                static const uint32_t   CONTROL_MODULE_CONTROL_REVISION = 0x0;
                static const uint32_t   CONTROL_MODULE_CONTROL_HWINFO = 0x4;
                static const uint32_t   CONTROL_MODULE_CONTROL_SYSCONFIG = 0x10;
                static const uint32_t   CONTROL_MODULE_CONTROL_STATUS = 0x40;
                static const uint32_t   CONTROL_MODULE_CONTROL_EMIF_SDRAM_CONFIG = 0x110;
                static const uint32_t   CONTROL_MODULE_CORE_SLDO_CTRL = 0x428;
                static const uint32_t   CONTROL_MODULE_MPU_SLDO_CTRL = 0x42C;
                static const uint32_t   CONTROL_MODULE_CLK32KDIVRATIO_CTRL = 0x444;
                static const uint32_t   CONTROL_MODULE_BANDGAP_CTRL = 0x448;
                static const uint32_t   CONTROL_MODULE_BANDGAP_TRIM = 0x44C;
                static const uint32_t   CONTROL_MODULE_PLL_CLKINPULOW_CTRL = 0x458;
                static const uint32_t   CONTROL_MODULE_MOSC_CTRL = 0x468;
                static const uint32_t   CONTROL_MODULE_DEEPSLEEP_CTRL = 0x470;
                static const uint32_t   CONTROL_MODULE_DPLL_PWR_SW_STATUS = 0x50C;
                static const uint32_t   CONTROL_MODULE_DEVICE_ID = 0x600;
                static const uint32_t   CONTROL_MODULE_DEV_FEATURE = 0x604;
                static const uint32_t   CONTROL_MODULE_INIT_PRIORITY_0 = 0x608;
                static const uint32_t   CONTROL_MODULE_INIT_PRIORITY_1 = 0x60C;
                static const uint32_t   CONTROL_MODULE_MMU_CFG = 0x610;
                static const uint32_t   CONTROL_MODULE_TPTC_CFG = 0x614;
                static const uint32_t   CONTROL_MODULE_USB_CTRL0 = 0x620;
                static const uint32_t   CONTROL_MODULE_USB_STS0 = 0x624;
                static const uint32_t   CONTROL_MODULE_USB_CTRL1 = 0x628;
                static const uint32_t   CONTROL_MODULE_USB_STS1 = 0x62C;
                static const uint32_t   CONTROL_MODULE_MAC_ID0_LO = 0x630;
                static const uint32_t   CONTROL_MODULE_MAC_ID0_HI = 0x634;
                static const uint32_t   CONTROL_MODULE_MAC_ID1_LO = 0x638;
                static const uint32_t   CONTROL_MODULE_MAC_ID1_HI = 0x63C;
                static const uint32_t   CONTROL_MODULE_DCAN_RAMINIT = 0x644;
                static const uint32_t   CONTROL_MODULE_USB_WKUP_CTRL = 0x648;
                static const uint32_t   CONTROL_MODULE_GMII_SEL = 0x650;
                static const uint32_t   CONTROL_MODULE_PWMSS_CTRL = 0x664;
                static const uint32_t   CONTROL_MODULE_MREQPRIO_0 = 0x670;
                static const uint32_t   CONTROL_MODULE_MREQPRIO_1 = 0x674;
                static const uint32_t   CONTROL_MODULE_HW_EVENT_SEL_GRP1 = 0x690;
                static const uint32_t   CONTROL_MODULE_HW_EVENT_SEL_GRP2 = 0x694;
                static const uint32_t   CONTROL_MODULE_HW_EVENT_SEL_GRP3 = 0x698;
                static const uint32_t   CONTROL_MODULE_HW_EVENT_SEL_GRP4 = 0x69C;
                static const uint32_t   CONTROL_MODULE_SMRT_CTRL = 0x6A0;
                static const uint32_t   CONTROL_MODULE_MPUSS_HW_DEBUG_SEL = 0x6A4;
                static const uint32_t   CONTROL_MODULE_MPUSS_HW_DBG_INFO = 0x6A8;
                static const uint32_t   CONTROL_MODULE_VDD_MPU_OPP_050 = 0x770;
                static const uint32_t   CONTROL_MODULE_VDD_MPU_OPP_100 = 0x774;
                static const uint32_t   CONTROL_MODULE_VDD_MPU_OPP_120 = 0x778;
                static const uint32_t   CONTROL_MODULE_VDD_MPU_OPP_TURBO = 0x77C;
                static const uint32_t   CONTROL_MODULE_VDD_CORE_OPP_050 = 0x7B8;
                static const uint32_t   CONTROL_MODULE_VDD_CORE_OPP_100 = 0x7BC;
                static const uint32_t   CONTROL_MODULE_BB_SCALE = 0x7D0;
                static const uint32_t   CONTROL_MODULE_USB_VID_PID = 0x7F4;
                static const uint32_t   CONTROL_MODULE_EFUSE_SMA = 0x7FC;
                
                static const uint32_t   CONTROL_MODULE_OFFSET_ADDRESS[][2][2];
                static const uint32_t   CONTROL_MODULE_PWM_MUX_MODE[][2];
                
                // clock module registers
                
                static const uint32_t   CLOCK_MODULE_PERIPHERAL_ADDRESS = 0x44E00000;
                static const size_t     CLOCK_MODULE_PERIPHERAL_LENGTH = 0x0400;
                
                static const uint32_t   CLOCK_MODULE_PER_L4LS_CLKSTCTRL = 0x00;
                static const uint32_t   CLOCK_MODULE_PER_L3S_CLKSTCTRL = 0x04;
                static const uint32_t   CLOCK_MODULE_PER_L3_CLKSTCTRL = 0x0C;
                static const uint32_t   CLOCK_MODULE_PER_CPGMAC0_CLKCTRL = 0x14;
                static const uint32_t   CLOCK_MODULE_PER_LCDC_CLKCTRL = 0x18;
                static const uint32_t   CLOCK_MODULE_PER_USB0_CLKCTRL = 0x1C;
                static const uint32_t   CLOCK_MODULE_PER_TPTC0_CLKCTRL = 0x24;
                static const uint32_t   CLOCK_MODULE_PER_EMIF_CLKCTRL = 0x28;
                static const uint32_t   CLOCK_MODULE_PER_OCMCRAM_CLKCTRL = 0x2C;
                static const uint32_t   CLOCK_MODULE_PER_GPMC_CLKCTRL = 0x30;
                static const uint32_t   CLOCK_MODULE_PER_MCASP0_CLKCTRL = 0x34;
                static const uint32_t   CLOCK_MODULE_PER_UART5_CLKCTRL = 0x38;
                static const uint32_t   CLOCK_MODULE_PER_MMC0_CLKCTRL = 0x3C;
                static const uint32_t   CLOCK_MODULE_PER_ELM_CLKCTRL = 0x40;
                static const uint32_t   CLOCK_MODULE_PER_I2C2_CLKCTRL = 0x44;
                static const uint32_t   CLOCK_MODULE_PER_I2C1_CLKCTRL = 0x48;
                static const uint32_t   CLOCK_MODULE_PER_SPI0_CLKCTRL = 0x4C;
                static const uint32_t   CLOCK_MODULE_PER_SPI1_CLKCTRL = 0x50;
                static const uint32_t   CLOCK_MODULE_PER_L4LS_CLKCTRL = 0x60;
                static const uint32_t   CLOCK_MODULE_PER_MCASP1_CLKCTRL = 0x68;
                static const uint32_t   CLOCK_MODULE_PER_UART1_CLKCTRL = 0x6C;
                static const uint32_t   CLOCK_MODULE_PER_UART2_CLKCTRL = 0x70;
                static const uint32_t   CLOCK_MODULE_PER_UART3_CLKCTRL = 0x74;
                static const uint32_t   CLOCK_MODULE_PER_UART4_CLKCTRL = 0x78;
                static const uint32_t   CLOCK_MODULE_PER_TIMER7_CLKCTRL = 0x7C;
                static const uint32_t   CLOCK_MODULE_PER_TIMER2_CLKCTRL = 0x80;
                static const uint32_t   CLOCK_MODULE_PER_TIMER3_CLKCTRL = 0x84;
                static const uint32_t   CLOCK_MODULE_PER_TIMER4_CLKCTRL = 0x88;
                static const uint32_t   CLOCK_MODULE_PER_GPIO1_CLKCTRL = 0xAC;
                static const uint32_t   CLOCK_MODULE_PER_GPIO2_CLKCTRL = 0xB0;
                static const uint32_t   CLOCK_MODULE_PER_GPIO3_CLKCTRL = 0xB4;
                static const uint32_t   CLOCK_MODULE_PER_TPCC_CLKCTRL = 0xBC;
                static const uint32_t   CLOCK_MODULE_PER_DCAN0_CLKCTRL = 0xC0;
                static const uint32_t   CLOCK_MODULE_PER_DCAN1_CLKCTRL = 0xC4;
                static const uint32_t   CLOCK_MODULE_PER_EPWMSS1_CLKCTRL = 0xCC;
                static const uint32_t   CLOCK_MODULE_PER_EPWMSS0_CLKCTRL = 0xD4;
                static const uint32_t   CLOCK_MODULE_PER_EPWMSS2_CLKCTRL = 0xD8;
                static const uint32_t   CLOCK_MODULE_PER_L3_INSTR_CLKCTRL = 0xDC;
                static const uint32_t   CLOCK_MODULE_PER_L3_CLKCTRL = 0xE0;
                static const uint32_t   CLOCK_MODULE_PER_IEEE5000_CLKCTRL = 0xE4;
                static const uint32_t   CLOCK_MODULE_PER_PRU_ICSS_CLKCTRL = 0xE8;
                static const uint32_t   CLOCK_MODULE_PER_TIMER5_CLKCTRL = 0xEC;
                static const uint32_t   CLOCK_MODULE_PER_TIMER6_CLKCTRL = 0xF0;
                static const uint32_t   CLOCK_MODULE_PER_MMC1_CLKCTRL = 0xF4;
                static const uint32_t   CLOCK_MODULE_PER_MMC2_CLKCTRL = 0xF8;
                static const uint32_t   CLOCK_MODULE_PER_TPTC1_CLKCTRL = 0xFC;
                static const uint32_t   CLOCK_MODULE_PER_TPTC2_CLKCTRL = 0x100;
                static const uint32_t   CLOCK_MODULE_PER_SPINLOCK_CLKCTRL = 0x10C;
                static const uint32_t   CLOCK_MODULE_PER_MAILBOX0_CLKCTRL = 0x110;
                static const uint32_t   CLOCK_MODULE_PER_L4S_CLKSTCTRL = 0x11C;
                static const uint32_t   CLOCK_MODULE_PER_L4S_CLKCTRL = 0x120;
                static const uint32_t   CLOCK_MODULE_PER_OCPWP_L3_CLKSTCTRL = 0x12C;
                static const uint32_t   CLOCK_MODULE_PER_OCPWP_CLKCTRL = 0x130;
                static const uint32_t   CLOCK_MODULE_PER_PRU_ICSS_CLKSTCTRL = 0x140;
                static const uint32_t   CLOCK_MODULE_PER_CPSW_CLKSTCTRL = 0x144;
                static const uint32_t   CLOCK_MODULE_PER_LCDC_CLKSTCTRL = 0x148;
                static const uint32_t   CLOCK_MODULE_PER_CLKDIV32K_CLKCTRL = 0x14C;
                static const uint32_t   CLOCK_MODULE_PER_CLK_24MZ_CLKSTCTRL = 0x150;
                
                static const uint32_t   CLOCK_MODULE_PER_EPWMSS_CLKCTRL[];
                static const uint32_t   CLOCK_MODULE_PER_EPWMSS_CLKCTRL_ENABLE = 0x00000002;
                static const uint32_t   CLOCK_MODULE_PER_L4LS_CLKCTRL_ENABLE = 0x00000002;
                
                // pwm subsystem registers
                
                static const uint32_t   PWMSS_ADDRESS[];
                static const size_t     PWMSS_LENGTH = 0x0100;
                
                static const uint32_t   PWMSS_SYSCONFIG = 0x0004;
                static const uint32_t   PWMSS_CLKCONFIG = 0x0008;
                
                static const uint32_t   EPWM_ADDRESS[];
                static const size_t     EPWM_LENGTH = 0x0060;
                
                static const uint32_t   EPWM_TBCTL = 0x0000;
                static const uint32_t   EPWM_TBSTS = 0x0002;
                static const uint32_t   EPWM_TBPHSHR = 0x0004;
                static const uint32_t   EPWM_TBPHS = 0x0006;
                static const uint32_t   EPWM_TBCNT = 0x0008;
                static const uint32_t   EPWM_TBPRD = 0x000A;
                static const uint32_t   EPWM_CMPCTL = 0x000E;
                static const uint32_t   EPWM_CMPAHR = 0x0010;
                static const uint32_t   EPWM_CMPA = 0x0012;
                static const uint32_t   EPWM_CMPB = 0x0014;
                static const uint32_t   EPWM_AQCTLA = 0x0016;
                static const uint32_t   EPWM_AQCTLB = 0x0018;
                static const uint32_t   EPWM_AQSFRC = 0x001A;
                static const uint32_t   EPWM_AQCSFRC = 0x001C;
                static const uint32_t   EPWM_DBCTL = 0x001E;
                static const uint32_t   EPWM_DBRED = 0x0020;
                static const uint32_t   EPWM_DBFED = 0x0022;
                static const uint32_t   EPWM_TZSEL = 0x0024;
                static const uint32_t   EPWM_TZCTL = 0x0028;
                static const uint32_t   EPWM_TZEINT = 0x002A;
                static const uint32_t   EPWM_TZFLG = 0x002C;
                static const uint32_t   EPWM_TZCLR = 0x002E;
                static const uint32_t   EPWM_TZFRC = 0x0030;
                static const uint32_t   EPWM_ETSEL = 0x0032;
                static const uint32_t   EPWM_ETPS = 0x0034;
                static const uint32_t   EPWM_ETCLR = 0x0038;
                static const uint32_t   EPWM_PCCTL = 0x003C;
                static const uint32_t   EPWM_HRCTL = 0x0040;
                
                static const uint32_t   NS_TO_TBCLK = 20;
                static const uint32_t   DEFAULT_PERIOD = 1000000;
                
                uint8_t     channelNumber;
                uintptr_t   baseAddress;
                uint32_t    period;
        };
        
        /**
         * The InputType enumerates the input types for digital inputs.
         */
        enum InputType {
            
            Floating = 0,
            PullUp = 1,
            PullDown = 2
        };
        
        static const uint16_t   LED0 = 0;       /**< BeagleBone Black pin number. */
        static const uint16_t   LED1 = 1;       /**< BeagleBone Black pin number. */
        static const uint16_t   LED2 = 2;       /**< BeagleBone Black pin number. */
        static const uint16_t   LED3 = 3;       /**< BeagleBone Black pin number. */
        
        static const uint16_t   P8_3 = 4;       /**< BeagleBone Black pin number. */
        static const uint16_t   P8_4 = 5;       /**< BeagleBone Black pin number. */
        static const uint16_t   P8_5 = 6;       /**< BeagleBone Black pin number. */
        static const uint16_t   P8_6 = 7;       /**< BeagleBone Black pin number. */
        static const uint16_t   P8_7 = 8;       /**< BeagleBone Black pin number. */
        static const uint16_t   P8_8 = 9;       /**< BeagleBone Black pin number. */
        static const uint16_t   P8_9 = 10;      /**< BeagleBone Black pin number. */
        static const uint16_t   P8_10 = 11;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_11 = 12;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_12 = 13;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_13 = 14;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_14 = 15;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_15 = 16;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_16 = 17;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_17 = 18;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_18 = 19;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_19 = 20;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_20 = 21;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_21 = 22;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_22 = 23;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_23 = 24;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_24 = 25;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_25 = 26;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_26 = 27;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_27 = 28;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_28 = 29;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_29 = 30;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_30 = 31;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_31 = 32;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_32 = 33;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_33 = 34;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_34 = 35;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_35 = 36;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_36 = 37;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_37 = 38;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_38 = 39;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_39 = 40;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_40 = 41;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_41 = 42;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_42 = 43;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_43 = 44;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_44 = 45;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_45 = 46;     /**< BeagleBone Black pin number. */
        static const uint16_t   P8_46 = 47;     /**< BeagleBone Black pin number. */
        
        static const uint16_t   P9_11 = 48;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_12 = 49;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_13 = 50;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_14 = 51;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_15 = 52;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_16 = 53;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_17 = 54;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_18 = 55;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_19 = 56;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_20 = 57;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_21 = 58;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_22 = 59;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_23 = 60;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_24 = 61;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_25 = 62;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_26 = 63;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_27 = 64;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_28 = 65;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_29 = 66;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_30 = 67;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_31 = 68;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_33 = 69;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_35 = 70;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_36 = 71;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_37 = 72;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_38 = 73;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_39 = 74;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_40 = 75;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_41 = 76;     /**< BeagleBone Black pin number. */
        static const uint16_t   P9_42 = 77;     /**< BeagleBone Black pin number. */
        
        static const uint16_t   NUMBER_OF_PINS = 78;
        
                    BeagleBone();
        virtual     ~BeagleBone();
        void        configureDigitalIn(uint16_t number);
        void        configureDigitalIn(uint16_t number, InputType type);
        void        configureDigitalOut(uint16_t number);
        bool        readDigitalIn(uint16_t number);
        void        writeDigitalOut(uint16_t number, bool value);
        
    private:
        
        // control module registers
        
        static const uint32_t   CONTROL_MODULE_ADDRESS = 0x44E10000;
        static const size_t     CONTROL_MODULE_LENGTH = 0x2000;
        
        static const uint32_t   CONTROL_MODULE_OFFSET_ADDRESS[];
        
        static const uint32_t   CONTROL_MODULE_PU_DEN = 0x00000008;
        static const uint32_t   CONTROL_MODULE_PU_TYPE_SEL = 0x00000010;
        static const uint32_t   CONTROL_MODULE_RX_ACTIVE = 0x00000020;
        static const uint32_t   CONTROL_MODULE_MUX_MODE0 = 0x00000000;
        static const uint32_t   CONTROL_MODULE_MUX_MODE1 = 0x00000001;
        static const uint32_t   CONTROL_MODULE_MUX_MODE2 = 0x00000002;
        static const uint32_t   CONTROL_MODULE_MUX_MODE3 = 0x00000003;
        static const uint32_t   CONTROL_MODULE_MUX_MODE4 = 0x00000004;
        static const uint32_t   CONTROL_MODULE_MUX_MODE5 = 0x00000005;
        static const uint32_t   CONTROL_MODULE_MUX_MODE6 = 0x00000006;
        static const uint32_t   CONTROL_MODULE_MUX_MODE7 = 0x00000007;
        
        // gpio registers
        
        static const uint8_t    GPIO_DEVICE_NUMBER[];
        static const uint8_t    GPIO_BIT_NUMBER[];
        
        static const uint32_t   GPIO_ADDRESS[];
        static const size_t     GPIO_LENGTH[];
        
        static const uint32_t   GPIO_REVISION = 0x0000;
        static const uint32_t   GPIO_SYSCONFIG = 0x0010;
        static const uint32_t   GPIO_EOI = 0x0020;
        static const uint32_t   GPIO_IRQSTATUS_RAW_0 = 0x0024;
        static const uint32_t   GPIO_IRQSTATUS_RAW_1 = 0x0028;
        static const uint32_t   GPIO_IRQSTATUS_0 = 0x002C;
        static const uint32_t   GPIO_IRQSTATUS_1 = 0x0030;
        static const uint32_t   GPIO_IRQSTATUS_SET_0 = 0x0034;
        static const uint32_t   GPIO_IRQSTATUS_SET_1 = 0x0038;
        static const uint32_t   GPIO_IRQSTATUS_CLR_0 = 0x003C;
        static const uint32_t   GPIO_IRQSTATUS_CLR_1 = 0x0040;
        static const uint32_t   GPIO_SYSSTATUS = 0x0114;
        static const uint32_t   GPIO_CTRL = 0x0130;
        static const uint32_t   GPIO_OE = 0x0134;
        static const uint32_t   GPIO_DATAIN = 0x0138;
        static const uint32_t   GPIO_DATAOUT = 0x013C;
        static const uint32_t   GPIO_LEVELDETECT0 = 0x0140;
        static const uint32_t   GPIO_LEVELDETECT1 = 0x0144;
        static const uint32_t   GPIO_RISINGDETECT = 0x0148;
        static const uint32_t   GPIO_FALLINGDETECT = 0x014C;
        static const uint32_t   GPIO_DEBOUNCENABLE = 0x0150;
        static const uint32_t   GPIO_DEBOUNCINGTIME = 0x0154;
        static const uint32_t   GPIO_CLEARDATAOUT = 0x0190;
        static const uint32_t   GPIO_SETDATAOUT = 0x0194;
        
        uint16_t    gpioIndex[NUMBER_OF_PINS];
        uintptr_t   baseAddress[NUMBER_OF_PINS];
        size_t      baseAddressLength[NUMBER_OF_PINS];
};

#endif /* BEAGLE_BONE_H_ */
