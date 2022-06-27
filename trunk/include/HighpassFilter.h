/*
 * HighpassFilter.h
 * Copyright (c) 2019, ZHAW
 * All rights reserved.
 *
 *  Created on: 21.02.2019
 *      Author: Marcel Honegger
 */

#ifndef HIGHPASS_FILTER_H_
#define HIGHPASS_FILTER_H_

#include <cstdlib>
#include <cmath>

/**
 * This class implements a time-discrete 2nd order highpass filter for a series of data values.
 * This filter can typically be used within a periodic task that takes measurements that need
 * to be filtered, like gyro values.
 */
class HighpassFilter {

    public:

                HighpassFilter();
        virtual ~HighpassFilter();
        void    reset();
        void    reset(double value);
        void    setPeriod(double period);
        void    setFrequency(double frequency);
        double  getFrequency();
        double  filter(double value);

    private:

        double  period;
        double  frequency;
        double  a11, a12, a21, a22, b1, b2;
        double  x1, x2;
};

#endif /* HIGHPASS_FILTER_H_ */
