/*
 * SpaceTraveler.h
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.11.2015
 *      Author: Marcel Honegger
 */

#ifndef SPACE_TRAVELER_H_
#define SPACE_TRAVELER_H_

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <stdint.h>
#include <errno.h>

#if defined __QNX__

#include <sys/usbdi.h>

#else

#include <libusb-1.0/libusb.h>

#endif

#include "Thread.h"
#include "Module.h"

/**
 * The SpaceTraveler class is a device driver for the 3Dconnexion SpaceTraveler
 * input device. This device offers 6 analog inputs from the 3D joystick, and
 * 8 digital inputs from the 8 buttons located around the device.
 * <br/>
 * To read these inputs, this class offers 2 methods defined by the <code>Module</code>
 * class. These methods are usually called by <code>AnalogIn</code> or <code>DigitalIn</code>
 * objects.
 * <div style="text-align:center"><img src="spacetraveler.jpg" width="265"/></div>
 * <div style="text-align:center"><b>The 3Dconnexion SpaceTraveler input device</b></div>
 * See the documentation of the AnalogIn and DigitalIn classes for more information.
 */
class SpaceTraveler : public Module, Thread {
    
    public:
        
                    SpaceTraveler();
        virtual     ~SpaceTraveler();
        float       readAnalogIn(uint16_t number);
        bool        readDigitalIn(uint16_t number);
        
    private:
    
        static const uint16_t   VENDOR_ID = 0x046D;             // vendor 3Dconnexion
        static const uint16_t   DEVICE_ID = 0xC623;             // device SpaceTraveler
        static const uint16_t   NUMBER_OF_ANALOG_INPUTS = 6;
        static const uint16_t   NUMBER_OF_DIGITAL_INPUTS = 8;
    
        static float            x;
        static float            y;
        static float            z;
        static float            a;
        static float            b;
        static float            c;
        static uint16_t         buttons;

        #if defined __QNX__

        static usbd_connection* usbConnection;
        static usbd_device*     usbDevice;
        static usbd_pipe*       usbPipe;
        static void*            address;
        static usbd_urb*        usbUrb;

        static void     insertion(struct usbd_connection* connection, usbd_device_instance_t* instance);
        static void     removal(struct usbd_connection* connection, usbd_device_instance_t* instance);
        static void     callback(struct usbd_urb* urb, struct usbd_pipe* pipe, void* hdl);
    
        #else
    
        libusb_device**         usbDevices;
        libusb_device_handle*   usbDevice;
        bool                    keepAlive;
    
        void            run();
    
        #endif
};

#endif /* SPACE_TRAVELER_H_ */
