/*
 * HighpassFilter.cpp
 * Copyright (c) 2019, ZHAW
 * All rights reserved.
 *
 *  Created on: 21.02.2019
 *      Author: Marcel Honegger
 */

#include "HighpassFilter.h"

using namespace std;

/**
 * Creates a HighpassFilter object with a default cutoff frequency of 10 [rad/s].
 */
HighpassFilter::HighpassFilter() {

    period = 1.0;
    frequency = 10.0;

    a11 = (1.0+frequency*period)*exp(-frequency*period);
    a12 = period*exp(-frequency*period);
    a21 = -frequency*frequency*period*exp(-frequency*period);
    a22 = (1.0-frequency*period)*exp(-frequency*period);
    b1 = (1.0-(1.0+frequency*period)*exp(-frequency*period))/frequency/frequency;
    b2 = period*exp(-frequency*period);

    x1 = 0.0;
    x2 = 0.0;
}

/**
 * Deletes the LowpassFilter object.
 */
HighpassFilter::~HighpassFilter() {}

/**
 * Resets the filtered value to zero.
 */
void HighpassFilter::reset() {

    x1 = 0.0;
    x2 = 0.0;
}

/**
 * Resets the filtered value to a given value.
 * @param value the value to reset the filter to.
 */
void HighpassFilter::reset(double value) {

    x1 = -value/frequency/frequency;
    x2 = 0.0;
}

/**
 * Sets the sampling period of the filter.
 * This is typically the sampling period of the periodic task of a controller that uses this filter.
 * @param the sampling period, given in [s].
 */
void HighpassFilter::setPeriod(double period) {

    this->period = period;

    a11 = (1.0+frequency*period)*exp(-frequency*period);
    a12 = period*exp(-frequency*period);
    a21 = -frequency*frequency*period*exp(-frequency*period);
    a22 = (1.0-frequency*period)*exp(-frequency*period);
    b1 = (1.0-(1.0+frequency*period)*exp(-frequency*period))/frequency/frequency;
    b2 = period*exp(-frequency*period);
}

/**
 * Sets the cutoff frequency of this filter.
 * @param frequency the cutoff frequency of the filter in [rad/s].
 */
void HighpassFilter::setFrequency(double frequency) {

    this->frequency = frequency;

    a11 = (1.0+frequency*period)*exp(-frequency*period);
    a12 = period*exp(-frequency*period);
    a21 = -frequency*frequency*period*exp(-frequency*period);
    a22 = (1.0-frequency*period)*exp(-frequency*period);
    b1 = (1.0-(1.0+frequency*period)*exp(-frequency*period))/frequency/frequency;
    b2 = period*exp(-frequency*period);
}

/**
 * Gets the current cutoff frequency of this filter.
 * @return the current cutoff frequency in [rad/s].
 */
double HighpassFilter::getFrequency() {

    return frequency;
}

/**
 * Filters a value.
 * @param value the original unfiltered value.
 * @return the filtered value.
 */
double HighpassFilter::filter(double value) {

    double x1old = x1;
    double x2old = x2;

    x1 = a11*x1old+a12*x2old+b1*value;
    x2 = a21*x1old+a22*x2old+b2*value;

    return -frequency*frequency*x1-2.0*frequency*x2+value;
}
