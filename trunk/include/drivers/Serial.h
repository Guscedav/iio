/*
 * Serial.h
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 16.07.2017
 *      Author: Marcel Honegger
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <string>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include "Mutex.h"

/**
 * The Serial class implements a high level device driver for serial ports.
 * It opens and configures a given serial port and offers methods to read
 * and write characters through that port.
 */
class Serial {
    
    public:
        
        static const uint8_t   PARITY_NONE = 0;     /**< Serial communication parity bit. */
        static const uint8_t   PARITY_EVEN = 1;     /**< Serial communication parity bit. */
        static const uint8_t   PARITY_ODD = 2;      /**< Serial communication parity bit. */
        
                    Serial(std::string port, uint32_t baud, uint8_t parity);
        virtual     ~Serial();
        void        setRTS();
        void        clearRTS();
        void        setDTR();
        void        clearDTR();
        bool        readable();
        bool        writeable();
        int8_t      getc();
        void        putc(int8_t c);
        
    private:
        
        int32_t     serialPort;
        termios     settings;
        termios     previousSettings;
        Mutex       mutex;
};

#endif /* SERIAL_H_ */
