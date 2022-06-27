/*
 * CANMessage.cpp
 * Copyright (c) 2014, ZHAW
 * All rights reserved.
 *
 *  Created on: 23.12.2014
 *      Author: Marcel Honegger
 */

#include "CANMessage.h"

using namespace std;

/**
 * Creates an empty can message.
 */
CANMessage::CANMessage() {
    
    id = 0;
    for (uint8_t i = 0; i < 8; i++) data[i] = 0;
    len = 8;
    type = CANData;
}

/**
 * Creates a CAN message with given ID, message data, message length and message type.
 * @param id the ID of the CAN message.
 * @param data an array of bytes with the message content.
 * @param len the length of the message in bytes, a value between (and including) 0 and 8.
 * @param type the type of the message, either <code>CANData</code> for a regular data
 * message, or <code>CANRemote</code> for a remote message.
 */
CANMessage::CANMessage(uint32_t id, uint8_t data[], uint8_t len, CANType type) {
    
    this->id = id;
    for (uint8_t i = 0; i < len; i++) this->data[i] = data[i];
    for (uint8_t i = len; i < 8; i++) this->data[i] = 0;
    this->len = (len <= 8) ? len : 8;
    this->type = type;
}

/**
 * Creates a remote CAN message with given ID.
 * @param id the ID of the CAN message.
 */
CANMessage::CANMessage(uint32_t id) {
    
    this->id = id;
    for (uint8_t i = 0; i < 8; i++) this->data[i] = 0;
    this->len = 0;
    this->type = CANRemote;
}

/**
 * Creates a copy of a given CAN message.
 * @param canMessage the CAN message to copy.
 */
CANMessage::CANMessage(const CANMessage& canMessage) {
    
    this->id = canMessage.id;
    for (uint8_t i = 0; i < 8; i++) this->data[i] = canMessage.data[i];
    this->len = canMessage.len;
    this->type = canMessage.type;
}

CANMessage::~CANMessage() {}
