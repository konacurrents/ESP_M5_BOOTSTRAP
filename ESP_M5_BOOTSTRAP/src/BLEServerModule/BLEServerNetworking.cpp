//
//  BLEServerNetworking.cpp
//  ESP_M5_BOOTSTRAP
//
//  Created by Scott Moody on 6/7/26.
//

/** Parts from: NimBLE_Client Demo:
 * @see https://github.com/h2zero/NimBLE-Arduino/blob/master/examples/NimBLE_Client/NimBLE_Client.ino
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 */
#include "BLEServerNetworking.h"


#define USE_FAST_LED
//! 7.24.25 Hot Day, Ballon last night, Mt Out
//! for the 'C' option of atom color
#ifdef USE_FAST_LED
#include "../ATOM_LED_Module/M5Display.h"
#include "../ATOM_LED_Module/LED_DisPlay.h"
#endif

#include <NimBLEDevice.h>
