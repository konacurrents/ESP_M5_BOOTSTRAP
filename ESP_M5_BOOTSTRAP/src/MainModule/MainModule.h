#ifndef MainModule_h
#define MainModule_h
#include "../../Defines.h"

void setup_mainModule();
void loop_mainModule();


//! 5.3.25 add a central clearing house for defining PIN use
//! central clearing house for all pins used to we can analyze if there are overlaps
//! pin is the actual number, pinName is the local name (eg. IN1_PIN or VIN_PIN).
//! moduleName is the module in the code,
//! isI2C is whether this is a I2C bus (which we aren't using much yet)
void registerPinUse_mainModule(long pin, String pinName, String moduleName, boolean isI2C);

//! note these are the sets of pin use
//! 8.18.25 OUCH>.. 10 .. now 20 max
#define PIN_USE_MAX 20
typedef struct  {
    int pinUseCount;
    char *pinUseArray[PIN_USE_MAX];
} PinUseStruct;
//! get the pin use array
PinUseStruct getPinUseStruct_mainModule();

#endif
