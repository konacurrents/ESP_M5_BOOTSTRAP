//
//  BLETest.h
//  ESP_M5_BOOTSTRAP
//
//  Created by Scott Moody on 11/21/25.
//
#ifndef BLEClientNetworking_h
#define BLEClientNetworking_h
#include "../../Defines.h"

void setup_BLEClientNetworking();
void loop_BLEClientNetworking();

//! send command like  feed or JSON
void sendCommand(char* cmd);



//! https://esp32.com/viewtopic.php?t=2291
/*!< Scan interval. This is defined as the time interval from
 when the Controller started its last LE scan until it begins the subsequent LE scan.
 Range: 0x0004 to 0x4000 Default: 0x0010 (10 ms)
 Time = N * 0.625 msec
 Time Range: 2.5 msec to 10.24 seconds*/
/** Set scan interval (how often) and window (how long) in milliseconds */

/*!< Scan window. The duration of the LE scan. LE_Scan_Window
 shall be less than or equal to LE_Scan_Interval
 Range: 0x0004 to 0x4000 Default: 0x0010 (10 ms)
 Time = N * 0.625 msec
 Time Range: 2.5 msec to 10240 msec */
#define PSCAN_INTERVAL 45
#define PSCAN_WINDOW   15
#ifdef M5UNIFIED
#define PSCAN_TIME 10000 // 5 sec
                         //! 6.17.26 shortened time. 0 DOESN"T WORK FOR SURE
                         //#define PSCAN_TIME 1000 // 1 sec .. seems to work

#else
#define PSCAN_TIME   0  /* scan forever*/
#endif

#endif
