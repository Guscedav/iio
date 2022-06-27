/*
 * Mecca500.h
 * Copyright (c) 2021, ZHAW
 * All rights reserved.
 *
 *  Created on: 23.04.2021
 *      Author: Diego Molteni
 */

#ifndef MECCA_500_H_
#define MECCA_500_H_

#include <cstdlib>
#include <cstdint>
#include "Module.h"
#include "CoE.h"
#include "EtherCAT.h"
#include "Mutex.h"

/**
 * This class implements a device driver for the Mecademic Mecca500 industrial robot.
 * </code></pre>
 * Also see the documentation about the EtherCAT and the CoE classes for more information.
 */
class Mecca500 : public Module, CoE::SlaveDevice {
    
    public:
        
                    Mecca500(EtherCAT& etherCAT, CoE& coe, uint16_t deviceAddress);
        virtual     ~Mecca500();
        void        reset();
        void        home();
        void        enable();
        void        disable();
        void        setJointVelocity(const float jointVelocity[]);
        bool        isRobotBusy();
        bool        isRobotActivated();
        bool        isRobotHomed();
        uint16_t    getErrorNumber();
        uint32_t    getMotionStatus();
        uint16_t    getMoveID();
        void        getJointAngles(float jointAngles[]);
        
    private:
        
        static const uint16_t   MAILBOX_OUT_ADDRESS = 0x1000;
        static const uint16_t   MAILBOX_OUT_SIZE = 0x80;
        static const uint16_t   MAILBOX_IN_ADDRESS = 0x1080;
        static const uint16_t   MAILBOX_IN_SIZE = 0x80;
        static const uint16_t   BUFFERED_OUT_ADDRESS = 0x1100;
        static const uint16_t   BUFFERED_OUT_SIZE = 36;
        static const uint16_t   BUFFERED_IN_ADDRESS = 0x1400;
        static const uint16_t   BUFFERED_IN_SIZE = 64; // max 132 bytes with all 9 PDOs
        
        EtherCAT&   etherCAT;           // reference to EtherCAT stack
        CoE&        coe;                // reference to CANopen over EtherCAT driver
        Mutex       mutex;              // mutex to lock critical sections
        
        uint32_t    robotControl;       // output objects
        uint32_t    motionControl;
        uint32_t    moveCommand;
        uint32_t    moveArgument[6];
        
        uint16_t    robotStatus;        // input objects
        uint16_t    robotStatusError;
        uint32_t    motionStatusCheckpoint;
        uint16_t    motionStatusMoveID;
        uint16_t    motionStatusFIFOspace;
        uint32_t    motionStatus;
        uint32_t    jointSet[6];
        uint32_t    endEffectorPose[6];
        
        EtherCAT::Datagram*     rxPDO;
        EtherCAT::Datagram*     txPDO;
        
        void        writeDatagram();
        void        readDatagram();
};

#endif /* MECCA_500_H_ */
