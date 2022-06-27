/*
 * EncoderCounter.cpp
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 03.08.2021
 *      Author: Marcel Honegger
 */

#include "Module.h"
#include "EncoderCounter.h"

using namespace std;

/**
 * Creates an encoder counter object and defines a module this counter belongs to,
 * as well as the index number of this counter on that module.
 * @param module a reference to the module this encoder counter belongs to.
 * @param number the index number of this encoder counter on the module.
 */
EncoderCounter::EncoderCounter(Module& module, uint16_t number) : Channel(module, number) {

    offset = 0;

    module.configureEncoderCounter(number);
}

/**
 * Deletes the encoder counter object.
 */
EncoderCounter::~EncoderCounter() {}

/**
 * Sets the offset parameter of this encoder counter object.
 * @param offset the offset parameter.
 */
void EncoderCounter::setOffset(int32_t offset) {

    this->offset = offset;
}

/**
 * Gets the offset parameter of this encoder counter object.
 * @return the offset parameter.
 */
int32_t EncoderCounter::getOffset() {

    return offset;
}

/**
 * Resets this encoder counter to zero.
 * This sets the offset value so that the value returned by the read() method is zero.
 */
void EncoderCounter::reset() {
    
    offset = -read();
}

/**
 * Resets this encoder counter to a given value.
 * This sets the offset value so that the value returned by the read()
 * method is equal to this given value.
 */
void EncoderCounter::reset(int32_t value) {
    
    offset = value-read();
}

/**
 * Gets the encoder counter value.
 * @return an integer number representing the encoder counter value.
 */
int32_t EncoderCounter::read() {

    return module.readEncoderCounter(number)+offset;
}

/**
 * The empty operator is a shorthand notation of the <code>read()</code> method.
 */
EncoderCounter::operator int32_t() {

    return read();
}
