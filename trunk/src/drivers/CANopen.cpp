/*
 * CANopen.cpp
 * Copyright (c) 2015, ZHAW
 * All rights reserved.
 *
 *  Created on: 04.11.2015
 *      Author: Marcel Honegger
 */

#include "Thread.h"
#include "CAN.h"
#include "CANopen.h"

using namespace std;

template <class T> inline string type2String(const T& t) {
	
	stringstream out;
	out << t;
	
	return out.str();
}

const int32_t CANopen::PRIORITY = RealtimeThread::RT_MAX_PRIORITY-11;   // priority level of private thread
const double CANopen::PERIOD = 0.0001;                                  // period of private thread in [s]

void CANopen::Delegate::receiveObject(uint32_t functionCode, uint8_t object[]) {}

/**
 * Creates a CANopen device driver object and initializes local values.
 */
CANopen::CANopen(CAN& can) : RealtimeThread("CANopen", STACK_SIZE, PRIORITY, PERIOD), can(can) {
    
    for (uint32_t i = 0; i < 128; i++) delegate[i] = NULL;
    
    // initialize local message buffer
    
    for (uint32_t i = 0; i < 128; i++) {
        
        emergencyObjectReceived[i] = false;
        tpdo1Received[i] = false;
        tpdo2Received[i] = false;
        tpdo3Received[i] = false;
        tpdo4Received[i] = false;
        tsdoReceived[i] = false;
        nodeguardObjectReceived[i] = false;
        
        for (uint8_t j = 0; j < 8; j++) {
            
            emergencyObject[i][j] = 0;
            tpdo1[i][j] = 0;
            tpdo2[i][j] = 0;
            tpdo3[i][j] = 0;
            tpdo4[i][j] = 0;
            tsdo[i][j] = 0;
            nodeguardObject[i][j] = 0;
        }
    }
    
    // start handler
    
    start();
}

/**
 * Deletes the CANopen device driver object and releases all allocated resources.
 */
CANopen::~CANopen() {
    
    // stop handler
    
    stop();
}

/**
 * This method must be called by device drivers that implement the CANopen::Delegate
 * interface to register themselves with the CANopen device driver.
 */
void CANopen::registerCANopenSlave(uint32_t nodeID, Delegate* delegate) {
    
    this->delegate[nodeID] = delegate;
}

/**
 * Transmits an object with a given function code and node ID. This method calls
 * the <code>write()</code> method of the CAN device driver.
 * @param functionCode the function code of this message, i.e. <code>TPDO1</code>,
 * <code>RPDO2</code>, <code>EMERGENCY</code>, ...
 * @param nodeID the identifier of the node. This ID must be in the range 0..127.
 * @param object an array of bytes to transmit. The length of this array must be in the
 * range 0..8.
 * @param length the length of the object to transmit, given in [bytes].
 * @param type the type of the object, either CANData or CANRemote.
 */
void CANopen::transmitObject(uint32_t functionCode, uint32_t nodeID, uint8_t object[], uint8_t length, CANType type) {
    
    if (nodeID > 127) throw invalid_argument("CANopen: wrong node identifier!");
    
    CANMessage canMessage(functionCode | nodeID, object, length, type);
    
    can.write(canMessage);
}

/**
 * Transmits a network management object.
 * @param command an NMT command specifier. This is either <code>START_REMOTE_NODE</code>,
 * <code>STOP_REMOTE_NODE</code>, <code>ENTER_PREOPERATIONAL_STATE</code>,
 * <code>RESET_NODE</code> or <code>RESET_COMMUNICATION</code>.
 * @param nodeID the ID of the node to transmit this command to. If the node
 * ID is 0, this command is valid for all nodes.
 */
void CANopen::transmitNMTObject(uint8_t command, uint32_t nodeID) {
    
    CANMessage canMessage;
    canMessage.id = NMT;
    canMessage.data[0] = command;
    canMessage.data[1] = static_cast<uint8_t>(nodeID);
    canMessage.len = 2;
    
    can.write(canMessage);
}

/**
 * Transmits a synchronization object.
 */
void CANopen::transmitSYNCObject() {
    
    CANMessage canMessage;
    canMessage.id = SYNC;
    canMessage.len = 0;
    
    can.write(canMessage);
}

/**
 * Transmits a request for a nodeguard object.
 * @param nodeID the identifier of the node. This ID must be in the range 0..127.
 */
void CANopen::requestNodeguardObject(uint32_t nodeID) {
    
    nodeguardObjectReceived[nodeID] = false;
    
    CANMessage canMessage;
    canMessage.id = NODEGUARD | nodeID;
    canMessage.data[0] = 0;
    canMessage.len = 1;
    canMessage.type = CANRemote;
    
    can.write(canMessage);
}

/**
 * Receives an object with a given function code and node ID.
 * @param functionCode the function code of this message, i.e. <code>EMERGENCY</code>,
 * <code>TPDO1</code>, <code>RPDO2</code>, ...
 * @param nodeID the identifier of the node. This ID must be in the range 0..127.
 * @param object an array of at least 8 bytes to copy the requested object into.
 * @return <code>true</code> when the requested object was received, <code>false</code> otherwise.
 */
bool CANopen::receiveObject(uint32_t functionCode, uint32_t nodeID, uint8_t object[]) {
    
    if (nodeID > 127) throw invalid_argument("CANopen: wrong node identifier!");
    
    if ((functionCode == EMERGENCY) && !emergencyObjectReceived[nodeID]) return false;
    else if ((functionCode == TPDO1) && !tpdo1Received[nodeID]) return false;
    else if ((functionCode == TPDO2) && !tpdo2Received[nodeID]) return false;
    else if ((functionCode == TPDO3) && !tpdo3Received[nodeID]) return false;
    else if ((functionCode == TPDO4) && !tpdo4Received[nodeID]) return false;
    else if ((functionCode == TSDO) && !tsdoReceived[nodeID]) return false;
    else if ((functionCode == NODEGUARD) && !nodeguardObjectReceived[nodeID]) return false;
    
	for (uint8_t i = 0; i < 8; i++) object[i] = 0;
    
    mutex.lock();
    
    if (functionCode == EMERGENCY) for (uint8_t i = 0; i < 8; i++) object[i] = emergencyObject[nodeID][i];
    else if (functionCode == TPDO1) for (uint8_t i = 0; i < 8; i++) object[i] = tpdo1[nodeID][i];
    else if (functionCode == TPDO2) for (uint8_t i = 0; i < 8; i++) object[i] = tpdo2[nodeID][i];
    else if (functionCode == TPDO3) for (uint8_t i = 0; i < 8; i++) object[i] = tpdo3[nodeID][i];
    else if (functionCode == TPDO4) for (uint8_t i = 0; i < 8; i++) object[i] = tpdo4[nodeID][i];
    else if (functionCode == TSDO) for (uint8_t i = 0; i < 8; i++) object[i] = tsdo[nodeID][i];
    else if (functionCode == NODEGUARD) for (uint8_t i = 0; i < 8; i++) object[i] = nodeguardObject[nodeID][i];
    
    mutex.unlock();
    
    return true;
}

/**
 * Resets the internal buffer for an object with a given function code and node ID.
 * @param functionCode the function code of this message, i.e. <code>EMERGENCY</code>,
 * <code>TPDO1</code>, <code>RPDO2</code>, ...
 * @param nodeID the identifier of the node. This ID must be in the range 0..127.
 */
void CANopen::resetObject(uint32_t functionCode, uint32_t nodeID) {
    
    if (nodeID > 127) throw invalid_argument("CANopen: wrong node identifier!");
    
    if (functionCode == EMERGENCY) emergencyObjectReceived[nodeID] = false;
    else if (functionCode == TPDO1) tpdo1Received[nodeID] = false;
    else if (functionCode == TPDO2) tpdo2Received[nodeID] = false;
    else if (functionCode == TPDO3) tpdo3Received[nodeID] = false;
    else if (functionCode == TPDO4) tpdo4Received[nodeID] = false;
    else if (functionCode == TSDO) tsdoReceived[nodeID] = false;
    else if (functionCode == NODEGUARD) nodeguardObjectReceived[nodeID] = false;
}

/**
 * Writes an expedited service data object (SDO) to a CANopen node.
 * @param nodeID the identifier of the node. This ID must be in the range 1..127.
 * @param index the index of the service data object (16 bit).
 * @param subindex the subindex of the service data object entry (8 bit).
 * @param value the value to write into this service data object (8 - 32 bit).
 * @param length the number of bytes to write, usually 1, 2 or 4.
 */
void CANopen::writeSDO(uint32_t nodeID, uint16_t index, uint8_t subindex, uint32_t value, uint8_t length) {
    
    if ((nodeID < 1) || (nodeID > 127)) throw invalid_argument("CANopen: wrong node identifier!");
    
    tsdoReceived[nodeID] = false;
    for (uint8_t i = 0; i < 8; i++) tsdo[nodeID][i] = 0;    // reset current tsdo
    
    uint32_t retries = 0;
    while (!tsdoReceived[nodeID] && (retries < RETRIES)) {
        
        CANMessage canMessage;
        canMessage.id = RSDO | nodeID;
        canMessage.data[0] = 0x23+((4-length) << 2);
        canMessage.data[1] = index & 0xFF;
        canMessage.data[2] = (index >> 8) & 0xFF;
        canMessage.data[3] = subindex;
        canMessage.data[4] = value & 0xFF;
        canMessage.data[5] = (value >> 8) & 0xFF;
        canMessage.data[6] = (value >> 16) & 0xFF;
        canMessage.data[7] = (value >> 24) & 0xFF;
        canMessage.len = 8;
        canMessage.type = CANData;
        
        can.write(canMessage);
        
        uint32_t counter = 0;
        while (!tsdoReceived[nodeID] && (counter < TIMEOUT/RETRIES)) {
            Thread::sleep(1);
            counter++;
        }
        
        retries++;
    }
    
    if (!tsdoReceived[nodeID]) throw runtime_error("CANopen: no response from node "+type2String(nodeID)+"!");
    if ((tsdo[nodeID][0] & 0x80) > 0) throw runtime_error("CANopen: error message from node "+type2String(nodeID)+": class="+type2String(tsdo[nodeID][7])+", code="+type2String(tsdo[nodeID][6])+".");
}

/**
 * Reads an expedited service data object (SDO) from a CANopen node.
 * @param nodeID the identifier of the node. This ID must be in the range 1..127.
 * @param index the index of the service data object (16 bit).
 * @param subindex the subindex of the service data object entry (8 bit).
 * @return the value of this service data object entry.
 */
uint32_t CANopen::readSDO(uint32_t nodeID, uint16_t index, uint8_t subindex) {
    
    if ((nodeID < 1) || (nodeID > 127)) throw invalid_argument("CANopen: wrong node identifier!");
    
    tsdoReceived[nodeID] = false;
    for (uint8_t i = 0; i < 8; i++) tsdo[nodeID][i] = 0;    // reset current tsdo
    
    uint32_t retries = 0;
    while (!tsdoReceived[nodeID] && (retries < RETRIES)) {
        
        CANMessage canMessage;
        canMessage.id = RSDO | nodeID;
        canMessage.data[0] = 0x40;
        canMessage.data[1] = index & 0xFF;
        canMessage.data[2] = (index >> 8) & 0xFF;
        canMessage.data[3] = subindex;
        canMessage.data[4] = 0;
        canMessage.data[5] = 0;
        canMessage.data[6] = 0;
        canMessage.data[7] = 0;
        canMessage.len = 8;
        canMessage.type = CANData;
        
        can.write(canMessage);
        
        uint32_t counter = 0;
        while (!tsdoReceived[nodeID] && (counter < TIMEOUT/RETRIES)) {
            Thread::sleep(1);
            counter++;
        }
        
        retries++;
    }
    
    if (!tsdoReceived[nodeID]) throw runtime_error("CANopen: no response from node "+type2String(nodeID)+"!");
    if ((tsdo[nodeID][0] & 0x80) > 0) throw runtime_error("CANopen: error message from node "+type2String(nodeID)+": class="+type2String(tsdo[nodeID][7])+", code="+type2String(tsdo[nodeID][6])+".");
    
    uint8_t length = 4-((tsdo[nodeID][0] >> 2) & 0x03);     // number of used bytes
    
    if (length == 1) return static_cast<uint32_t>(tsdo[nodeID][4] & 0xFF);
    else if (length == 2) return static_cast<uint32_t>(tsdo[nodeID][4] & 0xFF) | (static_cast<uint32_t>(tsdo[nodeID][5] & 0xFF) << 8);
    return static_cast<uint32_t>(tsdo[nodeID][4] & 0xFF) | (static_cast<uint32_t>(tsdo[nodeID][5] & 0xFF) << 8) | (static_cast<uint32_t>(tsdo[nodeID][6] & 0xFF) << 16) | (static_cast<uint32_t>(tsdo[nodeID][7] & 0xFF) << 24);
}

/**
 * This method is the handler of this CANopen device driver.
 */
void CANopen::run() {
    
    CANMessage canMessage;
    
    while (waitForNextPeriod()) {
        
        if (can.read(canMessage) != 0) {
            
            if (canMessage.type == CANData) {
                
                uint32_t id = canMessage.id;
                uint32_t functionCode = id & FUNCTION_CODE_BITMASK;
                uint32_t nodeID = id & NODE_ID_BITMASK;
                
                if (delegate[nodeID] != NULL) delegate[nodeID]->receiveObject(functionCode, canMessage.data);
                
                mutex.lock();
                
                if (functionCode == EMERGENCY) {
                    for (uint8_t i = 0; i < 8; i++) emergencyObject[nodeID][i] = canMessage.data[i];
                    emergencyObjectReceived[nodeID] = true;
                } else if (functionCode == TPDO1) {
                    for (uint8_t i = 0; i < 8; i++) tpdo1[nodeID][i] = canMessage.data[i];
                    tpdo1Received[nodeID] = true;
                } else if (functionCode == TPDO2) {
                    for (uint8_t i = 0; i < 8; i++) tpdo2[nodeID][i] = canMessage.data[i];
                    tpdo2Received[nodeID] = true;
                } else if (functionCode == TPDO3) {
                    for (uint8_t i = 0; i < 8; i++) tpdo3[nodeID][i] = canMessage.data[i];
                    tpdo3Received[nodeID] = true;
                } else if (functionCode == TPDO4) {
                    for (uint8_t i = 0; i < 8; i++) tpdo4[nodeID][i] = canMessage.data[i];
                    tpdo4Received[nodeID] = true;
                } else if (functionCode == TSDO) {
                    for (uint8_t i = 0; i < 8; i++) tsdo[nodeID][i] = canMessage.data[i];
                    tsdoReceived[nodeID] = true;
                } else if (functionCode == NODEGUARD) {
                    for (uint8_t i = 0; i < 8; i++) nodeguardObject[nodeID][i] = canMessage.data[i];
                    nodeguardObjectReceived[nodeID] = true;
                }
                
                mutex.unlock();
                
            } else {
                
                // remote message received, ignore message
            }
            
        } else {
            
            // no message received
        }
    }
}
