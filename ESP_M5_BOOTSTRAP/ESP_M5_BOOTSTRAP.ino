/*
  Name:		ESP_BOOTSTRAP.ino
  Created:	6.19.25
  Author:	scott
*/
#include "Defines.h"
//#define useMAIN
#ifdef  USE_PLATFORM_IO
//! nothing .. it's all in src/main.cpp
#else
export void mainLoop();
export void mainSetup();
void loop()
{
    mainLoop();
}
void setup()
{
    mainSetup();
}
#endif

//! moved to src/main.cpp
#ifdef useMAIN

void loop() {
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

void setup() {


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

#endif
