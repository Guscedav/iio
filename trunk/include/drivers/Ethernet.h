/*
 * Ethernet.h
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 14.08.2017
 *      Author: Marcel Honegger
 */

#ifndef ETHERNET_H_
#define ETHERNET_H_

#include <cstdlib>
#include <cstdint>

/**
 * The <code>Ethernet</code> class implements an abstract driver for an Ethernet controller.
 * It offers methods to send and receive Ethernet frames.
 */
class Ethernet {
    
    public:
        
        static const uint16_t   ETHERTYPE_IPV4 = 0x0800;                /**< Ether type. */
        static const uint16_t   ETHERTYPE_ARP = 0x0806;                 /**< Ether type. */
        static const uint16_t   ETHERTYPE_IPV6 = 0x86DD;                /**< Ether type. */
        static const uint16_t   ETHERTYPE_ETHER_CAT = 0x88A4;           /**< Ether type. */
        static const uint16_t   ETHERTYPE_ETHERNET_POWERLINK = 0x88AB;  /**< Ether type. */
        static const uint16_t   ETHERTYPE_SERCOS = 0x88CD;              /**< Ether type. */
        
                            Ethernet();
        virtual             ~Ethernet();
        virtual bool        link();
        virtual void        address(uint8_t macAddress[6]);
        virtual uint16_t    send(uint8_t destinationMACAddress[6], uint16_t etherType, uint8_t data[], uint16_t length);
        virtual uint16_t    receive(uint8_t sourceMACAddress[6], uint16_t& etherType, uint8_t data[], uint16_t length);
};

#endif /* ETHERNET_H_ */
