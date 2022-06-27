/*
 * Serial.cpp
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 16.07.2017
 *      Author: Marcel Honegger
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "Serial.h"

using namespace std;

/**
 * Creates and configures a Serial device driver object.
 * @param port the name of the serial port to use, i.e. "/dev/ttyS0" or "/dev/ttyUSB1".
 * @param baud the baudrate to use, either 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600 or 115200.
 * @param parity the parity bit to use, this must be either PARITY_NONE, PARITY_EVEN or PARITY_ODD.
 */
Serial::Serial(string port, uint32_t baud, uint8_t parity) {
    
    serialPort = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC);
    
    if (serialPort >= 0) {
        
        fcntl(serialPort, F_SETFL, 0);
        
        tcgetattr(serialPort, &previousSettings);
        tcgetattr(serialPort, &settings);
        
        if (baud == 300) { cfsetispeed(&settings, B300); cfsetospeed(&settings, B300); }
        else if (baud == 600) { cfsetispeed(&settings, B600); cfsetospeed(&settings, B600); }
        else if (baud == 1200) { cfsetispeed(&settings, B1200); cfsetospeed(&settings, B1200); }
        else if (baud == 2400) { cfsetispeed(&settings, B2400); cfsetospeed(&settings, B2400); }
        else if (baud == 4800) { cfsetispeed(&settings, B4800); cfsetospeed(&settings, B4800); }
        else if (baud == 9600) { cfsetispeed(&settings, B9600); cfsetospeed(&settings, B9600); }
        else if (baud == 19200) { cfsetispeed(&settings, B19200); cfsetospeed(&settings, B19200); }
        else if (baud == 38400) { cfsetispeed(&settings, B38400); cfsetospeed(&settings, B38400); }
        else if (baud == 57600) { cfsetispeed(&settings, B57600); cfsetospeed(&settings, B57600); }
        else if (baud == 115200) { cfsetispeed(&settings, B115200); cfsetospeed(&settings, B115200); }
        
        if (parity == PARITY_NONE) {
            settings.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
            settings.c_cflag |= (CS8 | CLOCAL | CREAD);
        } else if (parity == PARITY_EVEN) {
            settings.c_cflag |= PARENB;
            settings.c_cflag &= ~(PARODD | CSTOPB | CSIZE);
            settings.c_cflag |= (CS7 | CLOCAL | CREAD);
        } else if (parity == PARITY_ODD) {
            settings.c_cflag |= (PARENB | PARODD);
            settings.c_cflag &= ~(CSTOPB | CSIZE);
            settings.c_cflag |= (CS7 | CLOCAL | CREAD);
        }
        
        #if defined __QNX__
        
        #else
        
        settings.c_cflag &= ~(CRTSCTS); // disable hardware flow control
        
        #endif
        
        settings.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHONL | IEXTEN);
        settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IXANY);
        settings.c_oflag &= ~(OPOST);
        settings.c_cc[VMIN] = 0;
        settings.c_cc[VTIME] = 10;  // read timeout of 1 second
        
        tcflush(serialPort, TCIFLUSH);
        
        tcsetattr(serialPort, TCSANOW, &settings);
        
    } else {
        
        throw runtime_error("Serial: couldn't open serial port.");
    }
}

/**
 * Deletes the Serial device driver object and releases all allocated resources.
 */
Serial::~Serial() {
    
    //tcsetattr(serialPort, TCSANOW, &previousSettings);
    
    close(serialPort);
}

/**
 * Sets the RTS pin of the serial port.
 */
void Serial::setRTS() {
    
    int32_t flag = TIOCM_RTS;
    ioctl(serialPort, TIOCMBIS, &flag);
}

/**
 * Clears the RTS pin of the serial port.
 */
void Serial::clearRTS() {
    
    int32_t flag = TIOCM_RTS;
    ioctl(serialPort, TIOCMBIC, &flag);     // clear DTR signal
}

/**
 * Sets the DTR pin of the serial port.
 */
void Serial::setDTR() {
    
    int32_t flag = TIOCM_DTR;
    ioctl(serialPort, TIOCMBIS, &flag);
}

/**
 * Clears the DTR pin of the serial port.
 */
void Serial::clearDTR() {
    
    int32_t flag = TIOCM_DTR;
    ioctl(serialPort, TIOCMBIC, &flag);
}

/**
 * Checks if data is available from the serial port.
 * @return <code>true</code> if at least one character can be read from the serial port, <code>false</code> otherwise.
 */
bool Serial::readable() {
    
    int32_t availableBytes = 0;
    if (ioctl(serialPort, FIONREAD, &availableBytes) != 0) throw runtime_error("Serial: couldn't check input buffer.");
    
    return (availableBytes > 0);
}

/**
 * Checks if data can be written to the serial port.
 * @return <code>true</code> if at least one character can be written to the serial port, <code>false</code> otherwise.
 * Note that this method currently always returns <code>true</code>. On typical systems, the serial output buffers are
 * large enought to write lots of characters, so that this method is not needed. It is merely present for compatibility
 * to device drivers for microcontrollers.
 */
bool Serial::writeable() {
    
    return true;
}

/**
 * Reads a character from the serial port.
 * If no data is available from the serial port within a timeout duration of 1 second,
 * this call throws a runtime error.
 * @return a character from the serial port.
 */
int8_t Serial::getc() {
    
    int8_t c = 0;
    int32_t n = read(serialPort, &c, 1);
    
    if (n == 0) throw runtime_error("Serial: no data received.");
    
    return c;
}

/**
 * Writes a character to the serial port.
 * @param c a character to write to the serial port.
 */
void Serial::putc(int8_t c) {
    
    write(serialPort, &c, 1);
}
