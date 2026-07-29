/*
  Name:		ESP_BOOTSTRAP.ino
  Created:	6.19.25
  Author:	scott
*/
#include "Defines.h"

void loop() {
#ifdef ESP_M5_ATOM_S3
  AtomS3.update();
#else
    M5.update();
#endif

 // Serial.printf(".");
  //! loop
  loop_mainModule();

//#ifdef NOT_HERE

    M5.update();
#ifdef ESP_M5_ATOM_S3
  if (AtomS3.BtnA.wasPressed()) {

    Serial.println("Pressed");
  }
  if (AtomS3.BtnA.wasReleased()) {

    Serial.println("Released");
  }
#else
    
    //!NOTE: ths issue is the timer is interruped by the scanner.. so make long-long very long..
    //was 1000  (from 500)
    if (M5.BtnB.wasPressed())
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** wasPressed ***");
        //longLongPress_MainModule = true;
    }
     if (M5.BtnB.wasReleased())
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** wasReleased ***");
        //longLongPress_MainModule = true;
    }
#ifdef M5UNIFIED
     if (M5.BtnA.wasClicked())
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** wasClicked ***");
        //longLongPress_MainModule = true;
    }
     if (M5.BtnA.wasHold())
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** wasHold ***");
        //longLongPress_MainModule = true;
    }
#endif

    if (M5.BtnA.wasReleasefor(4500))
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** LONG LONG PRESS ***");
        //longLongPress_MainModule = true;
    }
    else if (M5.BtnA.wasReleasefor(1000))
    {
        //        buttonA_longPress_MainModule();
        SerialDebug.println("MainModule **** LONG PRESS ***");
        //_longPress_MainModule = true;
    }
    else if (M5.BtnA.wasReleased())
    {
        //        buttonA_shortPress_MainModule();
        SerialDebug.println("MainModule **** SHORT PRESS ***");
        //_shortPress_MainModule = true;
    }
#endif
  //! see if data on the serial input
  if (Serial.available()) {
    // read string until meet newline character
    String command = Serial.readStringUntil('\n');
    SerialDebug.println(command);

    SerialDebug.println(command);

    if (command == "help" || command == ".") {
      SerialDebug.println("HELP");
    }
  }
//#endif
}

void setup() {
    
    
#ifdef ESP_M5_ATOM_S3
    AtomS3.begin();
#else
    M5.begin();
#endif
    
    Serial.begin(115200);
    SerialDebug.println("*** setup ****");
    SerialDebug.println(VERSION);
    
    //! setup
    setup_mainModule();
}
