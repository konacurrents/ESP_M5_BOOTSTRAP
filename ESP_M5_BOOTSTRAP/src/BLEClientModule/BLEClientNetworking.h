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

#endif
