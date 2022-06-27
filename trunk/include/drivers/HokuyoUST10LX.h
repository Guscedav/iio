/*
 * HokuyoUST10LX.h
 * Copyright (c) 2017, ZHAW
 * All rights reserved.
 *
 *  Created on: 27.07.2017
 *      Author: Marcel Honegger
 */

#ifndef HOKUYO_UST_10LX_H_
#define HOKUYO_UST_10LX_H_

#include <cstdlib>
#include <string>
#include <cstdint>
#include "Mutex.h"

/**
 * This class implements a device driver for the Hokuyo UST-10LX laser scanner.
 * The UST-10LX has a scan range of 270&deg;, with an angular resolution of 0.25&deg;.
 * Apart from distance measurements, it also measures the reflectance of objects. This
 * allows to easily detect reflecting beacons.
 * <br/>
 * <div style="text-align:center"><img src="hokuyoust10lx.jpg" width="300"/></div>
 * <br/>
 * <div style="text-align:center"><b>The Hokuyo UST-10LX laser scanner</b></div>
 * <br/>
 * This laser scanner communicates with a host computer over ethernet with a TCP/IP
 * connection. The host computer requests new scans, and the laser scanner responds
 * with an array of the latest measurements.
 * <br/>
 * The following example shows how to use this device driver:
 * <pre><code>
 * HokuyoUST10LX lidar("192.168.0.10", 10940);  <span style="color:#008000">// create device driver object</span>
 *
 * float distance[lidar.LENGTH];
 * float reflectance[lidar.LENGTH];
 * lidar.get(distance, reflectance);  <span style="color:#008000">// get the measurements</span>
 * </code></pre>
 * 
 */
class HokuyoUST10LX {
    
    public:
        
        static const uint16_t   LENGTH = 270*4+1;   /**< The length of arrays with measurements. */
        
                    HokuyoUST10LX(std::string ipAddress, uint16_t portNumber);
                    HokuyoUST10LX(std::string interfaceAddress, std::string ipAddress, uint16_t portNumber);
        virtual     ~HokuyoUST10LX();
        void        get(float distance[]);
        void        get(float distance[], float reflectance[]);
        
    private:
        
        static const uint32_t   RECEIVE_TIMEOUT = 1000; // receive timeout in [ms]
        
        int32_t         clientSocket;
        Mutex           mutex;
        
        void            writeLine(std::string line);
        std::string     readLine();
};

#endif /* HOKUYO_UST_10LX_H_ */
