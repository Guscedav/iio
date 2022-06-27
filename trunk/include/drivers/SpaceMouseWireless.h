/*
 * SpaceMouseWireless.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 03.09.2021
 *      Author: Marcel Honegger
 */

#ifndef SPACE_MOUSE_WIRELESS_H_
#define SPACE_MOUSE_WIRELESS_H_

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
 * The SpaceMouseWireless class is a device driver for the 3Dconnexion SpaceMouse
 * Wireless input device. This device offers 6 analog inputs from the 3D joystick, and
 * 2 digital inputs from the buttons located on the left and the right of the device.
 * <br/>
 * To read these inputs, this class offers 2 methods defined by the <code>Module</code>
 * class. These methods are usually called by <code>AnalogIn</code> or <code>DigitalIn</code>
 * objects.
 * <div style="text-align:center"><img src="spacemousewireless.jpg" width="400"/></div>
 * <div style="text-align:center"><b>The 3Dconnexion SpaceMouse Wireless input device</b></div>
 * See the documentation of the AnalogIn and DigitalIn classes for more information.
 */
class SpaceMouseWireless : public Module, Thread {

    public:

                    SpaceMouseWireless();
        virtual     ~SpaceMouseWireless();
        float       readAnalogIn(uint16_t number);
        bool        readDigitalIn(uint16_t number);

    private:

        static const uint16_t   VENDOR_ID = 0x256F;             // vendor 3Dconnexion
        static const uint16_t   DEVICE_ID = 0xC652;             // device SpaceMouse Wireless
        static const uint16_t   NUMBER_OF_ANALOG_INPUTS = 6;
        static const uint16_t   NUMBER_OF_DIGITAL_INPUTS = 2;

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

#endif /* SPACE_MOUSE_WIRELESS_H_ */
