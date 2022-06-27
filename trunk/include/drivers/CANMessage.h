/*
 * CANMessage.h
 * Copyright (c) 2014, ZHAW
 * All rights reserved.
 *
 *  Created on: 23.12.2014
 *      Author: Marcel Honegger
 */

#ifndef CAN_MESSAGE_H_
#define CAN_MESSAGE_H_

#include <cstdlib>
#include <stdint.h>

/**
 * The CANType enumerates the various CAN message types.
 */
enum CANType {
    
    CANData = 0,
    CANRemote = 1
};

/**
 * The <code>CANMessage</code> class implements a message on a CAN bus.
 */
class CANMessage {
    
    public:
        
        uint32_t    id;
        uint8_t     data[8];
        uint8_t     len;
        CANType     type;
        
                    CANMessage();
                    CANMessage(uint32_t id, uint8_t data[], uint8_t len = 8, CANType type = CANData);
                    CANMessage(uint32_t id);
                    CANMessage(const CANMessage& canMessage);
        virtual     ~CANMessage();
};

#endif /* CAN_MESSAGE_H_ */
