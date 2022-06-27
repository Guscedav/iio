/*
 * RPLidar.cpp
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 17.07.2017
 *      Author: Marcel Honegger
 */

#include "Serial.h"
#include "RPLidar.h"

using namespace std;

const int32_t RPLidar::PRIORITY = Thread::MAX_PRIORITY; // priority level of private thread
const float RPLidar::QUALITY_THRESHOLD = 10.0f;         // threshold for the quality value of a measurement
const float RPLidar::DISTANCE_THRESHOLD = 0.01f;        // threshold for the measured distance to accept the measurement

/**
 * This delegate method must be implemented by objects that wish to receive measurements.
 * @param quality a parameter that describes the quality of the measurement.
 * @param angle the angle of the lidar for this measurement, given in degrees [°].
 * @param distance the measured distance, given in [m].
 */
void RPLidar::Delegate::receiveMeasurement(float quality, float angle, float distance) {}

/**
 * Creates an RPLidar device driver object, initializes local values and start a handler thread.
 * @param serial a reference to a serial object this device driver depends on.
 * This serial interface must be configured with a baud rate of 115200 bits/sec.
 */
RPLidar::RPLidar(Serial& serial) : Thread("RPLidar", STACK_SIZE, PRIORITY), serial(serial) {
    
    // initialize local variables
    
    delegate = NULL;
    state = STATE_STOP;
    stateDemand = STATE_STOP;
    headerCounter = 0;
    dataCounter = 0;
    
    // set DTR signal to stop lidar motor
    
    serial.setDTR();
    
    // start handler
    
    start();
}

/**
 * Deletes the <code>RPLidar</code> device driver object and releases all allocated resources.
 */
RPLidar::~RPLidar() {}

void RPLidar::setDelegate(Delegate* delegate) {
    
    mutex.lock();
    
    this->delegate = delegate;
    
    mutex.unlock();
}

/**
 * Starts the lidar motor and the distance measurements.
 */
void RPLidar::startScan() {
    
    stateDemand = STATE_SCAN;
}

/**
 * Stops the distance measurements and the lidar motor.
 */
void RPLidar::stopScan() {
    
    stateDemand = STATE_STOP;
}

/**
 * This run method implements the logic of this device driver.
 */
void RPLidar::run() {
    
    while (true) {
        
        switch (state) {
            
            case STATE_STOP:
                
                if (stateDemand == STATE_SCAN) {
                    
                    // clear DTR signal to start lidar motor
                    
                    serial.clearDTR();
                    
                    // clear serial input buffer
                    
                    while (serial.readable()) serial.getc();
                    
                    // reset local variables
                    
                    headerCounter = 0;
                    dataCounter = 0;
                    
                    // start measurements
                    
                    serial.putc(START_FLAG);
                    serial.putc(SCAN);
                    
                    // set new state
                    
                    state = STATE_SCAN;
                }
                
                break;
                
            case STATE_SCAN:
                
                if (stateDemand == STATE_STOP) {
                    
                    // stop measurements
                    
                    serial.putc(START_FLAG);
                    serial.putc(STOP);
                    
                    // set DTR signal to stop lidar motor
                    
                    serial.setDTR();
                    
                    // set new state
                    
                    state = STATE_STOP;
                    
                } else {
                    
                    // read single character from serial interface
                    
                    int8_t c = serial.getc();
                    
                    // add this character to the header or to the data buffer
                    
                    if (headerCounter < HEADER_SIZE) {
                        
                        header[headerCounter] = c;
                        headerCounter++;
                        
                    } else {
                        
                        if (dataCounter < DATA_SIZE) {
                            
                            data[dataCounter] = c;
                            dataCounter++;
                        }
                        
                        if (dataCounter >= DATA_SIZE) {
                            
                            dataCounter = 0;
                            
                            // decode data buffer
                            
                            float quality = static_cast<float>((static_cast<uint16_t>(data[0]) & 0xFF) >> 2);
                            float angle = 360.0f-static_cast<float>(((static_cast<uint16_t>(data[1]) & 0xFF) | ((static_cast<uint16_t>(data[2]) & 0xFF) << 8)) >> 1)/64.0f;
                            float distance = static_cast<float>((static_cast<uint16_t>(data[3]) & 0xFF) | ((static_cast<uint16_t>(data[4]) & 0xFF) << 8))/4000.0f;
                            
                            if ((quality < QUALITY_THRESHOLD) || (distance < DISTANCE_THRESHOLD)) {
                                
                                // bad measurement, discard result
                                
                            } else {
                                
                                // call delegate method, if implemented
                                
                                mutex.lock();
                                
                                if (delegate != NULL) delegate->receiveMeasurement(quality, angle, distance);
                                
                                mutex.unlock();
                            }
                        }
                    }
                }
                
                break;
                
            default:
                
                state = STATE_STOP;
                
                break;
        }
    }
}
