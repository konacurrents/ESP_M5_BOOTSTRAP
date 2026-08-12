//
//  main.cpp
//  ESP_M5_BOOTSTRAP
//
//  Created by Scott Moody on 8/12/26.
//

#include "main.h"

#include "../Defines.h"
//! TRICK:  has to compile with Arduino .. and the loop() is in .ino file
//! MUST: define PLATFORM_IO  (or not)

/*
 Name:        ESP_BOOTSTRAP.ino
 Created:    6.19.25
 Author:    scott
 */
#//include "Defines.h"

#ifdef USE_PLATFORM_IO
//! 8.12.26 need to be named loop() but not
void loop()
#else
    void mainLoop()
#endif
{
    
#ifdef ESP_M5_ATOM_S3
#ifdef M5UNIFIED
    M5.update();
#else
    AtomS3.update();
#endif
#else
    M5.update();
#endif
    
    // Serial.printf(".");
    //! loop
    loop_mainModule();
    
}

#ifdef USE_PLATFORM_IO
void setup()
#else
void mainSetup()
#endif
{
    
#ifdef ESP_M5_ATOM_S3
#ifdef M5UNIFIED
    M5.begin();
#else
    AtomS3.begin();
#endif
#else
    M5.begin();
#endif
    
    
    Serial.begin(115200);
    SerialDebug.println("*** setup ****");
    SerialDebug.println(VERSION);
    
    //! setup
    setup_mainModule();
}
