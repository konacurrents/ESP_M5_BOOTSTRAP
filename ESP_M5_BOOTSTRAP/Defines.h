//
//  Defines.h
//
//  Created by Scott Moody on 3/8/22.
//  NOW: May 14, 2025         5/14/25 (3+ years later)
//NOW: 6.19.25 just the bootstrap version
//NOW: 6.7.26 another year ... upgrading to M5Unified .. and 2.x BLE

#ifndef Defines_h
#define Defines_h

//! ******* UPDATE THESE ********
#define SSID_NAME "SunnyWhiteriver"
#define SSID_PASSWORD "sunny2021"
//#define SSID_NAME "Verizon-RC400L-A0"
//#define SSID_PASSWORD "5e3ee45e"
//#define SSID_NAME "NeutraKuhns"
//#define SSID_PASSWORD "NeutraKuhns818"


//!Defines the name of the service of the server, which for M5 will be PTClicker
#define MAIN_BLE_SERVER_SERVICE_NAME_PTClicker (char*)"PTClicker"
#define MAIN_BLE_SERVER_SERVICE_NAME_PTFeeder (char*)"PTFeeder"

//! uncomment if you want to have SSID saved in EPROM
//#define SAVE_SSID_IN_EPROM
#define ESP_M5

//! 3.7.26 try for the S3
//go back for 7.13.26  
#define ESP_M5_ATOM_S3
#ifdef ESP_M5_ATOM_S3
#define M5UNIFIED
#ifdef M5UNIFIED
//#include <M5AtomS3.h> //wont compile as it pulls in FastLEZD.h

#include <M5Unified.h>
#else
#include "M5AtomS3.h"
#endif

#define PARTITION_SCHEME (char*)"Board=M5AtomS3 Unified,P= 8mb (3MB App 1.5 SPIFFS with OTA)"
//#define PARTITION_SCHEME (char*)"Board=M5AtomS3 Unified,P=1.9MB App Minimal SPIFFS with OTA"

#else
#define PARTITION_SCHEME (char*)"Board=M5StickCPlus P=1.9MB App Minimal SPIFFS with OTA"

#endif


//! turn off to minimizein
//#define TEST_JSON


//! ***** THIS IS MAIN "ATOM" ***** 3.29.25,  5.14.25   <<<<<<<<<<<< THIS ONE <<<<<<<<<<<<< M5Atom
#define VERSION "v7_31_26(3.0b)-BOOTSTRAP_UNIFIED_M5STACK_3_3_8"

//#define VERSION "v6_8_26(3.0)-BOOTSTRAP_UNIFIED_BLE_2"

//#define VERSION "v12_04_25(2.5)-BOOTSTRAP_M5CHAIN_M5STICK_COLOR_JSON_OTA_NTP_56"

//#define VERSION "v8_30_25(2.4)-BOOTSTRAP_COLOR_JSON_OTA_NTP_56"
//#define VERSION "v7_31_25(2.3c)-BOOTSTRAP_JSON_OTA_NTP_56"

//! for OTA
#define USE_MQTT_NETWORKING

//#define USE_FAST_LED

//https://forum.arduino.cc/t/single-line-define-to-disable-code/636044/4
// Turn on/off Serial printing being included in the executable
//NOTE: if multiple lines of code say are creating a string to print, wrap that
//code with #if (SERIAL_DEBUG_INFO) - or whatever level of printing
//Use:  SerialError.print...
#define SERIAL_DEBUG_ERROR true
#define SerialError \
  if (SERIAL_DEBUG_ERROR) Serial
#define SERIAL_DEBUG_LOTS false
#define SerialLots \
  if (SERIAL_DEBUG_LOTS) Serial
#define SERIAL_DEBUG_DEBUG true
#define SerialDebug \
  if (SERIAL_DEBUG_DEBUG) Serial
#define SERIAL_DEBUG_INFO false
#define SerialInfo \
  if (SERIAL_DEBUG_INFO) Serial
#define SERIAL_DEBUG_MINIMAL true
#define SerialMin \
  if (SERIAL_DEBUG_MINIMAL) Serial
// a temporary debug.. without having to set the above..
#define SERIAL_DEBUG_TEMP true
#define SerialTemp \
  if (SERIAL_DEBUG_TEMP) Serial
//turn on CALLS to see the methods called in order, etc
#define SERIAL_DEBUG_CALL false
#define SerialCall \
  if (SERIAL_DEBUG_CALL) Serial
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
#ifdef M5STICKCPLUS2
#include <M5StickCPlus2.h>
#else
//! 8.30.24 add M5AtomS3
#ifdef ESP_M5_ATOM_S3
//#include <M5AtomS3.h>
#else

#ifdef M5UNIFIED
nothere
#include "M5Unified.h"
#else
nothere
#include <M5StickCPlus.h>
#endif

#endif
#endif

//! see https://github.com/m5stack/M5Stack/issues/97
#undef min
#endif  //M5Core2
#else
//#include <Arduino.h>
#endif


//Turn this on 1st time.. and testing..  AND if something drastic causes it to reboot always
//#define BOOTSTRAP
//#define BOOTSTRAP_AP_MODE_STARTUP
//! set bootstrap_clean one time, then turn it off.. Sometimes get's it out of error funk.
//#define BOOTSTRAP_CLEAN

#include "src/MainModule/MainModule.h"
#include "src/JSONModule/JSON_Module.h"
#include "src/WIFI_APModule/WIFI_APModule.h"

//! use the library (not our codee)
#define USE_LIB_WEBSERVER // is the library

#ifdef USE_MQTT_NETWORKING
//#include "src/MQTTModule/MQTTNetworking.h"
#include "src/MQTTModule/OTAImageUpdate.h"
#endif

#endif  // Defines_h
