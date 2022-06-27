/*
 * SpaceMouseWireless.cpp
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 03.09.2021
 *      Author: Marcel Honegger
 */

#include "SpaceMouseWireless.h"

using namespace std;

float SpaceMouseWireless::x = 0.0f;
float SpaceMouseWireless::y = 0.0f;
float SpaceMouseWireless::z = 0.0f;
float SpaceMouseWireless::a = 0.0f;
float SpaceMouseWireless::b = 0.0f;
float SpaceMouseWireless::c = 0.0f;
uint16_t SpaceMouseWireless::buttons = 0;

#if defined __QNX__

usbd_connection* SpaceMouseWireless::usbConnection = NULL;
usbd_device* SpaceMouseWireless::usbDevice = NULL;
usbd_pipe* SpaceMouseWireless::usbPipe = NULL;
void* SpaceMouseWireless::address = NULL;
usbd_urb* SpaceMouseWireless::usbUrb = NULL;

#endif

/**
 * Creates a SpaceMouseWireless device driver object and initializes local values.
 */
SpaceMouseWireless::SpaceMouseWireless() {

    #if defined __QNX__

    usbd_device_ident_t interest = {VENDOR_ID, DEVICE_ID, 0x03, 0x00, 0x00};
    usbd_funcs_t functions = {_USBDI_NFUNCS, &SpaceMouseWireless::insertion, &SpaceMouseWireless::removal, NULL};
    usbd_connect_parm_t cparms = {NULL, USB_VERSION, USBD_VERSION, 0, 0, NULL, 0, &interest, &functions, USBD_CONNECT_WAIT};

    int32_t error = usbd_connect(&cparms, &usbConnection);

    if (error != EOK) throw runtime_error("SpaceMouseWireless: couldn't connect device to the USB!");

    #else

    usbDevice = NULL;
    usbDevices = NULL;

    libusb_init(NULL);

    ssize_t count = libusb_get_device_list(NULL, &usbDevices);

    if (count > 0) {

        for (ssize_t i = 0; (i < count) && (usbDevice == NULL); i++) {

            libusb_device_descriptor descriptor;

            if (libusb_get_device_descriptor(usbDevices[i], &descriptor) == 0) {

                if ((descriptor.idVendor == VENDOR_ID) && (descriptor.idProduct == DEVICE_ID)) {

                    if (libusb_open(usbDevices[i], &usbDevice) == 0) {

                        if (libusb_kernel_driver_active(usbDevice, 0)) {

                            libusb_detach_kernel_driver(usbDevice, 0);
                        }

                        if (libusb_claim_interface(usbDevice, 0) == 0) {

                            // start private handler thread

                            keepAlive = true;
                            setName("SpaceMouseWireless");
                            start();

                        } else {

                            cerr << "SpaceMouseWireless: libusb_claim_interface() failed!" << endl;

                            libusb_close(usbDevice);
                            usbDevice = NULL;
                        }

                    } else {

                        cerr << "SpaceMouseWireless: libusb_open() failed!" << endl;

                        usbDevice = NULL;
                    }

                } else {

                    // VENDOR_ID or DEVICE_ID does not match, try next...
                }

            } else {

                cerr << "SpaceMouseWireless: libusb_get_device_descriptor() failed!" << endl;
            }
        }

    } else {

        cerr << "SpaceNavigator: libusb_get_device_list() failed!" << endl;
    }

    #endif
}

/**
 * Deletes the SpaceMouseWireless device driver object and releases all allocated resources.
 */
SpaceMouseWireless::~SpaceMouseWireless() {

    #if defined __QNX__

    usbd_disconnect(SpaceMouseWireless::usbConnection);

    #else

    if (usbDevice != NULL) {

        // stop private handler thread

        keepAlive = false;
        join();

        // close usb connection

        libusb_release_interface(usbDevice, 0);
        libusb_close(usbDevice);
        usbDevice = NULL;
    }

    if (usbDevices != NULL) {

        // free list of usb devices

        libusb_free_device_list(usbDevices, 1);
        usbDevices = NULL;
    }

    libusb_exit(NULL);

    #endif
}

/**
 * This method reads the actual analog input. It is usually called by an AnalogIn object.
 * @param index the index number of the analog input. This value must be in the range 0 to 5.
 * @return the value of the analog input, given as a floating point number in the range -1.0 to +1.0.
 */
float SpaceMouseWireless::readAnalogIn(uint16_t number) {

    switch (number) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return a;
        case 4: return b;
        case 5: return c;
        default: return 0.0f;
    }
}

/**
 * This method reads the actual digital input. It is usually called by a DigitalIn object.
 * @param index the index number of the digital input. This value must be in the range 0 to 1.
 * @return the value of the digital input, given as a bool value (either <code>true</code> or <code>false</code>).
 */
bool SpaceMouseWireless::readDigitalIn(uint16_t number) {

    return (buttons & (1 << number));
}

#if defined __QNX__

/**
 * This is a callback method of the QNX USB stack.
 */
void SpaceMouseWireless::insertion(struct usbd_connection* connection, usbd_device_instance_t* instance) {

    int32_t error = usbd_attach(connection, instance, 0, &usbDevice);
    if (error == EOK) {

        struct usbd_desc_node* interface;

        usbd_interface_descriptor_t* descriptor = usbd_interface_descriptor(usbDevice, (*instance).config, (*instance).iface, (*instance).alternate, &interface);
        if (descriptor != NULL) {

            struct usbd_desc_node* endpoint;

            usbd_descriptors_t* descriptors = usbd_parse_descriptors(usbDevice, interface, USB_DESC_ENDPOINT, 1, &endpoint);
            if (descriptors != NULL) {

                error = usbd_open_pipe(usbDevice, descriptors, &usbPipe);
                if (error == EOK) {

                    address = usbd_alloc(8);
                    usbUrb = usbd_alloc_urb(NULL);

                    error = usbd_reset_pipe(usbPipe);
                    if (error == EOK) {

                        SpaceMouseWireless::callback(usbUrb, usbPipe, NULL);

                    } else cerr << "SpaceMouseWireless: usbd_reset_pipe error!\n";

                } else {

                    address = NULL;
                    usbUrb = NULL;

                    cerr << "SpaceMouseWireless: usbd_open_pipe error!\n";
                }

            } else {

                address = NULL;
                usbUrb = NULL;

                cerr << "SpaceMouseWireless: usbd_parse_descriptors error!\n";
            }

        } else {

            address = NULL;
            usbUrb = NULL;

            cerr << "SpaceMouseWireless: usbd_interface_descriptor error!\n";
        }

    } else {

        usbDevice = NULL;
        address = NULL;
        usbUrb = NULL;

        cerr << "SpaceMouseWireless: usbd_attach error " << error << "!\n";
    }
}

/**
 * This is a callback method of the QNX USB stack.
 */
void SpaceMouseWireless::removal(struct usbd_connection* connection, usbd_device_instance_t* instance) {

    if (usbUrb != NULL) {
        usbd_free_urb(usbUrb);
        usbUrb = NULL;
    }
    if (address != NULL) {
        usbd_free(address);
        address = NULL;
    }

    if (usbDevice != NULL) usbd_detach(usbDevice);

    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    a = 0.0f;
    b = 0.0f;
    c = 0.0f;
    buttons = 0;
}

/**
 * This is a callback method of the QNX USB stack.
 */
void SpaceMouseWireless::callback(struct usbd_urb* urb, struct usbd_pipe* pipe, void* hdl) {

    int32_t error = usbd_setup_interrupt(urb, URB_DIR_IN, address, 8);
    if (error == EOK) {

        error = usbd_io(urb, pipe, SpaceMouseWireless::callback, NULL, USBD_TIME_INFINITY);
        if (error == EOK) {

            uint8_t ch0 = *((uint8_t*)(address)+0);
            uint8_t ch1 = *((uint8_t*)(address)+1);
            uint8_t ch2 = *((uint8_t*)(address)+2);
            uint8_t ch3 = *((uint8_t*)(address)+3);
            uint8_t ch4 = *((uint8_t*)(address)+4);
            uint8_t ch5 = *((uint8_t*)(address)+5);
            uint8_t ch6 = *((uint8_t*)(address)+6);

            if (ch0 == 1) {
                x = -static_cast<float>(static_cast<int16_t>((ch4 << 8) | ch3))/350.0f;
                y = -static_cast<float>(static_cast<int16_t>((ch2 << 8) | ch1))/350.0f;
                z = -static_cast<float>(static_cast<int16_t>((ch6 << 8) | ch5))/350.0f;
            } else if (ch0 == 2) {
                a = -static_cast<float>(static_cast<int16_t>((ch4 << 8) | ch3))/350.0f;
                b = -static_cast<float>(static_cast<int16_t>((ch2 << 8) | ch1))/350.0f;
                c = -static_cast<float>(static_cast<int16_t>((ch6 << 8) | ch5))/350.0f;
            } else if (ch0 == 3) {
                buttons = static_cast<uint16_t>((ch2 << 8) | ch1);
            }

        } else {

            x = 0.0f;
            y = 0.0f;
            z = 0.0f;
            a = 0.0f;
            b = 0.0f;
            c = 0.0f;
            buttons = 0;

            cerr << "SpaceMouseWireless: usbd_io error!\n";
        }

    } else {

        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        a = 0.0f;
        b = 0.0f;
        c = 0.0f;
        buttons = 0;

        cerr << "SpaceMouseWireless: usbd_setup_interrupt error!\n";
    }
}

#else

/**
 * This is the run method for the libusb driver on Linux.
 */
void SpaceMouseWireless::run() {

    while (keepAlive) {

        uint8_t buffer[32];
        int32_t read = 0;

        int32_t error = libusb_interrupt_transfer(usbDevice, 0x81, buffer, sizeof(buffer), &read, 100);

        if (error == 0) {

            if ((read == 13) && (buffer[0] == 1)) {
                x = -static_cast<float>(static_cast<int16_t>((static_cast<uint16_t>(buffer[4]) << 8) | static_cast<uint16_t>(buffer[3])))/350.0f;
                y = -static_cast<float>(static_cast<int16_t>((static_cast<uint16_t>(buffer[2]) << 8) | static_cast<uint16_t>(buffer[1])))/350.0f;
                z = -static_cast<float>(static_cast<int16_t>((static_cast<uint16_t>(buffer[6]) << 8) | static_cast<uint16_t>(buffer[5])))/350.0f;
                a = -static_cast<float>(static_cast<int16_t>((static_cast<uint16_t>(buffer[10]) << 8) | static_cast<uint16_t>(buffer[9])))/350.0f;
                b = -static_cast<float>(static_cast<int16_t>((static_cast<uint16_t>(buffer[8]) << 8) | static_cast<uint16_t>(buffer[7])))/350.0f;
                c = -static_cast<float>(static_cast<int16_t>((static_cast<uint16_t>(buffer[12]) << 8) | static_cast<uint16_t>(buffer[11])))/350.0f;
            } else if ((read == 3) && (buffer[0] == 3)) {
                buttons = (static_cast<uint16_t>(buffer[2]) << 8) | static_cast<uint16_t>(buffer[1]);
            }

        } else {
            /*
            if (error == LIBUSB_ERROR_TIMEOUT) cerr << "SpaceMouseWireless: libusb_interrupt_transfer() = LIBUSB_ERROR_TIMEOUT" << endl;
            else if (error == LIBUSB_ERROR_PIPE) cerr << "SpaceMouseWireless: libusb_interrupt_transfer() = LIBUSB_ERROR_PIPE" << endl;
            else if (error == LIBUSB_ERROR_OVERFLOW) cerr << "SpaceMouseWireless: libusb_interrupt_transfer() = LIBUSB_ERROR_OVERFLOW" << endl;
            else if (error == LIBUSB_ERROR_NO_DEVICE) cerr << "SpaceMouseWireless: libusb_interrupt_transfer() = LIBUSB_ERROR_NO_DEVICE" << endl;
            else cerr << "SpaceMouseWireless: libusb_interrupt_transfer() = LIBUSB_ERROR (" << error << ")" << endl;
            */
        }
    }
}

#endif
