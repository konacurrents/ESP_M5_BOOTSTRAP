//
//  Defines.h
//
//  Created by Scott Moody on 3/8/22.
//  NOW: May 14, 2025         5/14/25 (3+ years later)
//NOW: 6.19.25 just the bootstrap version

#ifndef Defines_h
#define Defines_h

//! turn off to minimize
//#define TEST_JSON

//! 4. M5 Atom  This includes QR and Socket
#define ESP_M5_ATOM_LITE
//! 1.5.24 NOW the M5AtomLite is all the ATOM's but configured to 1 at a time
//This is an easy way to turn on the smart clicker configuration...
//! https://docs.m5stack.com/en/atom/atomic_qr
//! library:  not using M5Stack-ATOM (actually using M5Stick-C-Plus)
//! 12.26.23 day after xmas
//! https://docs.m5stack.com/en/atom/atom_socket
//! ALSO: use the Minimal SPIFF, 1.9m app, OTA and 190KB spiff
//! 2.25.23 Sketch uses 1497093 bytes (76%) of program storage space. Maximum is 1966080 bytes.
//! 8.22.24 (seems to generate m5stack_stickc_plus.bin)   .. and size now 65%
#ifdef ESP_M5_ATOM_LITE

//! ***** THIS IS MAIN "ATOM" ***** 3.29.25,  5.14.25   <<<<<<<<<<<< THIS ONE <<<<<<<<<<<<< M5Atom

#define VERSION "v7_21_25(2.3b)-BOOTSTRAP_JSON_OTA_NTP"


#define ESP_M5
#define M5_ATOM
#define USE_MQTT_NETWORKING
//#define USE_BUTTON_MODULE  ... ATOMQButtons.cpp
#define USE_WIFI_AP_MODULE
#define USE_BLE_SERVER_NETWORKING
#define USE_BLE_CLIENT_NETWORKING //try to be a smart clicker too..
//#define USE_DISPLAY_MODULE  .. no display

#define USE_STEPPER_MODULE
#define USE_UI_MODULE

//! 5.21.25 adding feed_on_startup .. as the M5Atom is powering the 
//! motor before we can talk to it.. so this will at least shorten that issue
#define FEED_ON_STARTUP

//! which ATOM plugs MODULEs are included in build:
#define ATOM_QRCODE_MODULE
//! including ATOM_SOCKET_MODULE now ...
#define ATOM_SOCKET_MODULE
//#else

//! the ATOM uses FAST_LED (the light on top button, but not M5AtomS3)
#define USE_FAST_LED

//! Sensors:  2.8.24 (30K above pacific)
#define KEY_UNIT_SENSOR_CLASS
#define USE_LED_BREATH
//! 11.14.23 try "https" secure web call (to SemanticMarker.org/bot/...)
//!  See https://GitHub.com/konacurrents/SemanticMarkerAPI for more info
//crashing .. 3.22.24 (just send DOCFOLLOW for now)
//#define USE_REST_MESSAGING
//! 3.25.24 try again ... (not working)


//! 4.4.24 (after glacier skiing at Crystal ice, but snowing) -- year ago now 4.4.25 (yesterday good. Today would have been amazing views)
#define USE_SPIFF_MODULE
#define USE_SPIFF_MQTT_SETTING
#define USE_SPIFF_QRATOM_SETTING


//! 4.10.24 GPS sensor
//#define USE_GPS_SENSOR_CLASS

//#endif // not M5AtomS3

#endif //ESP_M5_ATOM_LITE

//https://forum.arduino.cc/t/single-line-define-to-disable-code/636044/4
// Turn on/off Serial printing being included in the executable
//NOTE: if multiple lines of code say are creating a string to print, wrap that
//code with #if (SERIAL_DEBUG_INFO) - or whatever level of printing
//Use:  SerialError.print...
#define SERIAL_DEBUG_ERROR true
#define SerialError if (SERIAL_DEBUG_ERROR) Serial
#define SERIAL_DEBUG_LOTS false
#define SerialLots  if (SERIAL_DEBUG_LOTS) Serial
#define SERIAL_DEBUG_DEBUG true
#define SerialDebug  if (SERIAL_DEBUG_DEBUG) Serial
#define SERIAL_DEBUG_INFO false
#define SerialInfo  if (SERIAL_DEBUG_INFO) Serial
#define SERIAL_DEBUG_MINIMAL true
#define SerialMin  if (SERIAL_DEBUG_MINIMAL) Serial
// a temporary debug.. without having to set the above..
#define SERIAL_DEBUG_TEMP true
#define SerialTemp  if (SERIAL_DEBUG_TEMP) Serial
//turn on CALLS to see the methods called in order, etc
#define SERIAL_DEBUG_CALL false
#define SerialCall  if (SERIAL_DEBUG_CALL) Serial
// *** REST ARE NORMAL INCLUDES based on these #defines ****

#define NOTSET_STRING (char*)"notset"

//!This is to debug the BLE ACK happening or not happening. CURRENT is the correct one, but the OLD can be tested
//! since OLD discovers the GEN3 (or actually ESP32) feeders from the _ESP_32 string being returned..
//! Code is in BLEServer.cpp
#define CURRENT_BLE_ACK_APPROACH
//#define OLD_BLE_NO_ACK_APPROACH

#ifdef ESP_M5
#ifdef M5CORE2_MODULE
#include <M5Core2.h>
#else
//fix this... 4.25.24
#ifdef  M5STICKCPLUS2
#include <M5StickCPlus2.h>
#else
//! 8.30.24 add M5AtomS3
#ifdef ESP_M5_ATOM_S3
#include <M5AtomS3.h>
#else
#include <M5StickCPlus.h>
#endif
#endif

//! see https://github.com/m5stack/M5Stack/issues/97
#undef min
#endif //M5Core2
#else
#include <Arduino.h>
#endif


//Turn this on 1st time.. and testing..  AND if something drastic causes it to reboot always
//#define BOOTSTRAP
//#define BOOTSTRAP_AP_MODE_STARTUP
//! set bootstrap_clean one time, then turn it off.. Sometimes get's it out of error funk.
//#define BOOTSTRAP_CLEAN

#include "src/MainModule/MainModule.h"
#include "src/JSONModule/JSON_Module.h"

#ifdef USE_MQTT_NETWORKING
//#include "src/MQTTModule/MQTTNetworking.h"
#include "src/MQTTModule/OTAImageUpdate.h"
#endif

#endif // Defines_h
