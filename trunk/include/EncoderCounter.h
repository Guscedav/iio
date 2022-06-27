/*
 * EncoderCounter.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 03.08.2021
 *      Author: Marcel Honegger
 */

#ifndef ENCODER_COUNTER_H_
#define ENCODER_COUNTER_H_

#include <cstdlib>
#include <stdint.h>
#include "Channel.h"

class Module;

/**
 * The <code>EncoderCounter</code> class is used to read the value of an encoder counter.
 * This class might either be implemented by a specific device driver, or given
 * a reference to a driver module with the constructor, so that it can read an
 * actual encoder counter from that module.
 */
class EncoderCounter : public Channel {

    public:

                        EncoderCounter(Module& module, uint16_t number);
        virtual         ~EncoderCounter();
        virtual void    setOffset(int32_t offset);
        virtual int32_t getOffset();
        virtual void    reset();
        virtual void    reset(int32_t value);
        virtual int32_t read();
                        operator int32_t();

    protected:

        int32_t         offset;
};

#endif /* ENCODER_COUNTER_H_ */
