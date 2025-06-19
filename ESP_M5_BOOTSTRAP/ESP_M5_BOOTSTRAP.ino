/*
  Name:		ESP_BOOTSTRAP.ino
  Created:	6.19.25
  Author:	scott
*/
#include "Defines.h"

void loop()
{
	//! loop
   loop_mainModule();
}

void setup()
{
   Serial.begin(115200);
   SerialDebug.println();
   SerialDebug.println(VERSION);

	//! setup
   setup_mainModule();
}
