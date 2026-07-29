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

//! reads the preferences. Save is everytime the savePreference is called
void readPreferences_mainModule();

//! note these are the sets of pin use
//! 8.18.25 OUCH>.. 10 .. now 20 max
#define PIN_USE_MAX 20
typedef struct  {
    int pinUseCount;
    char *pinUseArray[PIN_USE_MAX];
} PinUseStruct;
//! get the pin use array
PinUseStruct getPinUseStruct_mainModule();

//! test for sending a FEED message
void sendCommand_main(char* cmd);

//gets unix time..
int getTimeStamp_mainModule();


// **** Helper Methods
//!If nil it create one with just the null, so strlen = 0
//!NOTE: the strdup() might be used later..
char* createCopy(char * stringA);

//!If nil it create one with just the null, so strlen = 0
//!NOTE: the strdup() might be used later..
char* createCopy2(const char * stringA);

//!10000 == no poweroff
#define NO_POWEROFF_AMOUNT_MAIN 10000
#define NO_POWEROFF_AMOUNT_STRING_MAIN (char*)"10000"


#endif
