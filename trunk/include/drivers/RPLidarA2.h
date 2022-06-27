/*
 * RPLidarA2.h
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 26.07.2017
 *      Author: Marcel Honegger
 */

#ifndef RP_LIDAR_A2_H_
#define RP_LIDAR_A2_H_

#include <string>
#include <stdint.h>
#include "Thread.h"
#include "Timer.h"
#include "Mutex.h"

class Serial;

/**
 * This class implements a simple device driver for the Slamtec RoboPeak lidar version A2.
 * It receives measurements from the serial interface and sends these measurements
 * to a registered delegate object.
 * <br/>
 * This device driver uses the RoboPeak lidar in simple scan mode, with a measurement
 * frequency of 2 kHz. The express scan mode with a sample frequency of 4 kHz is not
 * supported yet.
 * <br/>
 * <div style="text-align:center"><img src="rplidara2.png" width="400"/></div>
 * <div style="text-align:center"><b>The Slamtec RoboPeak lidar Version A2</b></div>
 * <br/>
 * Another object that wishes to receive measurements from this device driver must implement the
 * <code>receiveMeasurement()</code> method of the delegate class. An example of a user class that
 * implements this method is given below:
 * <pre><code>
 * <span style="color:#008000">// declaration of delegate class</span>
 * class MyDelegate : public RPLidarA2::Delegate {
 *
 *   public:
 *      
 *     void receiveMeasurement(float quality, float angle, float distance);
 * };
 *
 * <span style="color:#008000">// implementation of delegate class method</span>
 * void MyDelegate::receiveMeasurement(float quality, float angle, float distance) {
 *
 *   <span style="color:#008000">// do something with these values, i.e. copy into data buffer</span>
 * }
 * </code></pre>
 * An actual delegate class usually needs more variables and methods, i.e. to copy the measurements into local
 * buffers or to write them into a data file.
 * <br/>
 * An example that shows how to use this device driver is given below:
 * <pre><code>
 * Serial serial("/dev/ttyUSB0", 115200, Serial::PARITY_NONE);  <span style="color:#008000">// create a serial interface</span>
 * RPLidarA2 lidar(serial);  <span style="color:#008000">// create an RPLidarA2 object</span>
 *
 * MyDelegate lidarDelegate;  <span style="color:#008000">// create and register a delegate object</span>
 * lidar.setDelegate(&lidarDelegate);
 *
 * lidar.startScan();  <span style="color:#008000">// start the lidar motor and the measurements</span>
 * </code></pre>
 * Also see the documentation about the Serial class for more information.
 */
class RPLidarA2 : public Thread {
    
    public:
        
        /**
         * The <code>Delegate</code> class implements a callback method for another object to receive measurements.
         */
        class Delegate {
            
            public:
                
                virtual void    receiveMeasurement(float quality, float angle, float distance);
        };
        
                    RPLidarA2(Serial& serial);
        virtual     ~RPLidarA2();
        void        setDelegate(Delegate* delegate);
        void        startScan();
        void        stopScan();
        
    private:
        
        static const size_t     STACK_SIZE = 64*1024;   // stack size of private thread in [bytes]
        static const int32_t    PRIORITY;               // priority level of private thread
        
        static const int16_t    STATE_OFF = 0;          // states of the state machine
        static const int16_t	STATE_START = 1;
        static const int16_t	STATE_SCAN = 2;
        static const int16_t    STATE_STOP = 3;         // states of the state machine
        
        static const int32_t    HEADER_SIZE = 7;
        static const int32_t    DATA_SIZE = 5;
        
        static const uint8_t    START_FLAG = 0xA5;      // RPLidar control bytes
        static const uint8_t    STOP = 0x25;
        static const uint8_t    RESET = 0x40;
        static const uint8_t    SCAN = 0x20;
        static const uint8_t    EXPRESS_SCAN = 0x82;
        static const uint8_t    FORCE_SCAN = 0x21;
        static const uint8_t    GET_INFO = 0x50;
        static const uint8_t    GET_HEALTH = 0x52;
        static const uint8_t    GET_SAMPLERATE = 0x59;
        static const uint8_t    SET_MOTOR_PWM = 0xF0;
        
        static const uint16_t   DEFAULT_MOTOR_PWM = 200;    // max motor pwm: 1023
        static const uint32_t   START_DELAY = 500;          // delay after starting motor, given in [ms]
        static const uint32_t   STOP_DELAY = 500;           // delay after stopping motor, given in [ms]
        
        static const float      QUALITY_THRESHOLD;
        static const float      DISTANCE_THRESHOLD;
        
        Serial&     serial;
        Delegate*   delegate;
        int16_t     state;
        int16_t     stateDemand;
        int8_t      header[HEADER_SIZE];    // receive buffer for header information
        uint8_t     headerCounter;
        int8_t      data[DATA_SIZE];        // receive buffer for measurement data packets
        uint8_t     dataCounter;
        Timer       timer;
        Mutex       mutex;
        
        void        run();
};

#endif /* RP_LIDAR_A2_H_ */
