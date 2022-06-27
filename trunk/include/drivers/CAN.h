/*
 * CAN.h
 * Copyright (c) 2014, ZHAW
 * All rights reserved.
 *
 *  Created on: 23.12.2014
 *      Author: Marcel Honegger
 */

#ifndef CAN_H_
#define CAN_H_

#include <cstdlib>
#include <stdint.h>

class CANMessage;

/**
 * The <code>CAN</code> class implements an abstract driver for a CAN controller.
 * It offers methods to transmit and receive CAN messages.
 */
class CAN {
    
    public:
		
                                CAN();
		virtual                 ~CAN();
        virtual void            frequency(uint32_t hz);
		virtual int32_t         write(CANMessage canMessage);
		virtual int32_t         read(CANMessage& canMessage);
};

#endif /* CAN_H_ */
