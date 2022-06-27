/*
 * RPLidarA2.cpp
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 26.07.2017
 *      Author: Marcel Honegger
 */

#include "Serial.h"
#include "RPLidarA2.h"

using namespace std;

const int32_t RPLidarA2::PRIORITY = Thread::MAX_PRIORITY; // priority level of private thread
const float RPLidarA2::QUALITY_THRESHOLD = 10.0f;         // threshold for the quality value of a measurement
const float RPLidarA2::DISTANCE_THRESHOLD = 0.01f;        // threshold for the measured distance to accept the measurement

/**
 * This delegate method must be implemented by objects that wish to receive measurements.
 * @param quality a parameter that describes the quality of the measurement.
 * @param angle the angle of the lidar for this measurement, given in degrees [°].
 * @param distance the measured distance, given in [m].
 */
void RPLidarA2::Delegate::receiveMeasurement(float quality, float angle, float distance) {}

/**
 * Creates an RPLidarA2 device driver object, initializes local values and start a handler thread.
 * @param serial a reference to a serial object this device driver depends on.
 * This serial interface must be configured with a baud rate of 115200 bits/sec.
 */
RPLidarA2::RPLidarA2(Serial& serial) : Thread("RPLidarA2", STACK_SIZE, PRIORITY), serial(serial) {
    
    // initialize local variables
    
    delegate = NULL;
    state = STATE_STOP;
    stateDemand = STATE_STOP;
    headerCounter = 0;
    dataCounter = 0;
    timer.start();
    
    // start handler
    
    start();
}

/**
 * Deletes the <code>RPLidarA2</code> device driver object and releases all allocated resources.
 */
RPLidarA2::~RPLidarA2() {}

void RPLidarA2::setDelegate(Delegate* delegate) {
    
    mutex.lock();
    
    this->delegate = delegate;
    
    mutex.unlock();
}

/**
 * Starts the lidar motor and the distance measurements.
 */
void RPLidarA2::startScan() {
    
    stateDemand = STATE_SCAN;
}

/**
 * Stops the distance measurements and the lidar motor.
 */
void RPLidarA2::stopScan() {
    
    stateDemand = STATE_OFF;
}

/**
 * This run method implements the logic of this device driver.
 */
void RPLidarA2::run() {
    
    while (true) {
        
        switch (state) {
            
            case STATE_OFF:
                
                if (stateDemand == STATE_SCAN) {
                    
                    // clear DTR signal
                    
                    serial.clearDTR();
                    
                    // start lidar motor
                    
                    serial.putc(START_FLAG);
                    serial.putc(SET_MOTOR_PWM);
                    serial.putc(2);                                 // size of payload (2 bytes)
                    serial.putc(DEFAULT_MOTOR_PWM & 0xFF);          // first byte of payload (pwm duty cycle)
                    serial.putc((DEFAULT_MOTOR_PWM >> 8) & 0xFF);   // second byte of payload
                    
                    uint8_t checksum = 0;               // checksum of all transmitted bytes
                    checksum ^= START_FLAG;
                    checksum ^= SET_MOTOR_PWM;
                    checksum ^= 2;
                    checksum ^= DEFAULT_MOTOR_PWM & 0xFF;
                    checksum ^= (DEFAULT_MOTOR_PWM >> 8) & 0xFF;
                    
                    serial.putc(checksum);
                    
                    // reset timer
                    
                    timer.reset();
                    
                    // set new state
                    
                    state = STATE_START;
                }
                
                break;
                
            case STATE_START:
                
                if (stateDemand == STATE_SCAN) {
                    
                    if (timer > START_DELAY) {
                        
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
                        
                    } else {
                        
                        // wait a bit longer for lidar motor to start
                    }
                    
                } else if (stateDemand == STATE_OFF) {
                    
                    // stop lidar motor
                    
                    serial.putc(START_FLAG);
                    serial.putc(SET_MOTOR_PWM);
                    serial.putc(2);                 // size of payload (2 bytes)
                    serial.putc(0);                 // first byte of payload (pwm duty cycle)
                    serial.putc(0);                 // second byte of payload
                    
                    uint8_t checksum = 0;
                    checksum ^= START_FLAG;
                    checksum ^= SET_MOTOR_PWM;
                    checksum ^= 2;
                    checksum ^= 0;
                    checksum ^= 0;
                    
                    serial.putc(checksum);
                    
                    // reset timer
                    
                    timer.reset();
                    
                    // set new state
                    
                    state = STATE_STOP;
                }
                
                break;
                
            case STATE_SCAN:
                
                if (stateDemand == STATE_OFF) {
                    
                    // stop measurements
                    
                    serial.putc(START_FLAG);
                    serial.putc(STOP);
                    
                    // stop lidar motor
                    
                    serial.putc(START_FLAG);
                    serial.putc(SET_MOTOR_PWM);
                    serial.putc(2);                 // size of payload (2 bytes)
                    serial.putc(0);                 // first byte of payload (pwm duty cycle)
                    serial.putc(0);                 // second byte of payload
                    
                    uint8_t checksum = 0;
                    checksum ^= START_FLAG;
                    checksum ^= SET_MOTOR_PWM;
                    checksum ^= 2;
                    checksum ^= 0;
                    checksum ^= 0;
                    
                    serial.putc(checksum);
                    
                    // reset timer
                    
                    timer.reset();
                    
                    // set new state
                    
                    state = STATE_STOP;
                    
                } else {
                    
                    // try to read single character from serial interface
                    
                    if (serial.readable()) {
                        
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
                        
                    } else {
                        
                        // no characters to read from serial interface
                    }
                }
                
                break;
                
            case STATE_STOP:
                
                if (timer > STOP_DELAY) {
                    
                    // set DTR signal
                    
                    serial.setDTR();
                    
                    // set new state
                    
                    state = STATE_OFF;
                    
                } else {
                    
                    // wait a bit longer for lidar motor to stop
                }
                
                break;
                
            default:
                
                state = STATE_STOP;
                
                break;
        }
    }
}
