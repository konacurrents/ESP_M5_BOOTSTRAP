//
//  BLEServerNetworking.h
//  ESP_M5_BOOTSTRAP
//
//  Created by Scott Moody on 6/7/26.
//

#ifndef BLEServerNetworking_h
#define BLEServerNetworking_h
#include "../../Defines.h"


//!the 'setup' for this module BLEServerNetworking. Here the service name is added (and potentially more later)
void setup_BLEServerNetworking(char *serviceName, char * deviceName, char *serviceUUID, char *characteristicUUID);

void loop_BLEServerNetworking();

#endif
