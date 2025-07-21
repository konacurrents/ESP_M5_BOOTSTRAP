#include "MainModule.h"

#include <string>
//#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>

//! 7.21.25
#include "../Time/NTPClient.h"
#include <WiFiUdp.h>
// NTP server to request epoch time
//const char* _ntpServerURL = "pool.ntp.org";
boolean _ntpServerInit = false;
WiFiUDP _ntpUDP;
NTPClient *_timeClient; //(_ntpUDP, "pool.ntp.org", 36000, 60000);
// initialized to a time offset of 10 hours
//                           HH:MM:SS
// timeClient initializes to 10:00:00 if it does not receive an NTP packet
// before the 100ms timeout.
// without isTimeSet() the LED would be switched on, although the time
// was not yet set correctly.

//!The WIFI client
WiFiClient _espClient;

/*
 Set: otafile, Val: http://KnowledgeShark.org/OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin
 main_dispatchAsyncCommandWithString:1: http://KnowledgeShark.org/OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin
 ASYNC_CALL_OTA_FILE_UPDATE_PARAMETER: http://KnowledgeShark.org/OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin
 will performOTAUpdate: http://KnowledgeShark.org  with httpAddress: OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin
 Connecting to: KnowledgeShark.org, bin = /OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin
 Fetching Bin: /OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin
 
 */

//! globals for WIFI
char _ssid[100];
char _wifiPassword[100];

//! #4 specify host and bin
char _hostOTA[100];
char _binOTA[200];

//! #4 specify host and bin
char _hostOTA_bootstrap[100];
char _binOTA_bootstrap[200];

void setupWIFI(char *arg_ssid, char* arg_password)
{
    //!start the WIFI mode and begin
    WiFi.mode(WIFI_STA);
    WiFi.begin(arg_ssid, arg_password);
    SerialDebug.printf("WiFi.begin(%s,%s)\n", arg_ssid, arg_password);
}

String _WIFIInfoString;
//! retrieve the WIFIInfoString
String get_WIFIInfoString()
{
    long rssi = WiFi.RSSI();
    
    _WIFIInfoString = "IP Address: " + WiFi.localIP();
    _WIFIInfoString += "\n WIFI SSID" + String(WiFi.SSID());
    _WIFIInfoString += "\n RSSI" + rssi;
    // _WIFIInfoString += "\n WIFI Status = " + String(wifiStatus_MQTT());
    
    SerialDebug.println(_WIFIInfoString.c_str());
    return _WIFIInfoString;
}

//!show the status in string form (from Library/Adruino... WiFiType.h)
char *wifiStatus_MQTT()
{
    switch (WiFi.status())
    {
        case WL_NO_SHIELD:return (char*)"WL_NO_SHIELD";
        case WL_IDLE_STATUS: return (char*)"WL_IDLE_STATUS";
        case WL_NO_SSID_AVAIL: return (char*)"WL_NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED: return (char*)"WL_SCAN_COMPLETED";
        case WL_CONNECTED :return (char*)"WL_CONNECTED";
        case WL_CONNECT_FAILED: return (char*)"WL_CONNECT_FAILED";
        case WL_CONNECTION_LOST: return (char*)"WL_CONNECTION_LOST";
        default:
        case WL_DISCONNECTED: return (char*)"WL_DISCONNECTED";
    }
}


//!the EPROM is in preferences.h
#include <Preferences.h>
//!name of main prefs eprom
#define PREFERENCES_EPROM_MAIN_NAME "MainPrefs"
#define PREFERENCES_SSID "SSID"
#define PREFERENCES_WIFI "WIFI"
char _preferenceBuffer[100];

//! preferences for MAIN
Preferences _preferencesMainModule;

//! save a preference
void savePreference(char* preferenceID, char* preferenceValue)
{
    //save in EPROM
    _preferencesMainModule.begin(PREFERENCES_EPROM_MAIN_NAME, false);  //readwrite..
    _preferencesMainModule.putString(preferenceID, preferenceValue);
    
    // Close the Preferences
    _preferencesMainModule.end();
}
//! return the preference (this has to be copied)
char * getPreference(char* preferenceID)
{
    //!get from EPROM
    _preferencesMainModule.begin(PREFERENCES_EPROM_MAIN_NAME, true);  //read
    
    strcpy(_preferenceBuffer, _preferencesMainModule.getString(preferenceID).c_str());

    // Close the Preferences
    _preferencesMainModule.end();
    return _preferenceBuffer;
}
void setup_mainModule()
{
    SerialDebug.println("**** type on the serial monitor. Start with 'help'  ****");
#ifdef TEST_JSON
    setup_JSON_Module();
#endif
    
#pragma mark HARDCODE -- MODIFY THIS TO MAKE IT EASIER
    strcpy(_ssid,getPreference((char*)PREFERENCES_SSID));
    strcpy(_wifiPassword, getPreference((char*)PREFERENCES_WIFI));
    
    //! let user modify these..
    strcpy(_hostOTA,(char*)"http://KnowledgeShark.org");
    strcpy(_binOTA, (char*)"OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin");
        
}

//! uses the SSID and PASSWORD
void tryConnect()
{
    int count = 0;
    SerialDebug.printf("Trying Connection: %s, %s\n", _ssid, _wifiPassword);

    //!setup the WIFI.begin
    setupWIFI(_ssid, _wifiPassword);
    
    //! check status
    while (count < 10 && WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
        count ++;
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        SerialDebug.println("\n*** Try again with new values");
    }
    else
    {
        SerialDebug.println("**** CONNECTED ****");
        //String s = get_WIFIInfoString();
        
        _ntpServerInit = true;
        
        _timeClient = new NTPClient(_ntpUDP, "pool.ntp.org", 36000, 60000);
        // initialized to a time offset of 10 hours
        //                           HH:MM:SS
        // - 7 hrs GMT (PST)
        _timeClient->setTimeOffset(-(7*3600));
        //! time
        _timeClient->begin();
    }
}

//!On the esp32, sec is all we can handle.  We can return as a double if milisecond resolution is needed.
//!This is the time since app started..
//!https://randomnerdtutorials.com/epoch-unix-time-esp32-arduino/
int getTimeStamp_mainModule()
{
    
    time_t now;
    struct tm timeinfo;
    time(&now);
    if (_ntpServerInit)
        now = _timeClient->getEpochTime();

    SerialMin.printf("Unix Time: %d\n", now);
    return now;
}

//! main loop
void loop_mainModule()
{
    
    if (_ntpServerInit)
    {
        _timeClient->update();
        
//        _timeClient->getDay()
//        _timeClient-> getHours()
//        _timeClient-> getMinutes()
//        _timeClient-> getSeconds()

        //! set the time in the TimeLib.h
       // setTime(_timeClient->getEpochTime());
        
    }
    
    //! see if data on the serial input
    if (Serial.available())
    {
        // read string until meet newline character
        String command = Serial.readStringUntil('\n');
        
        SerialDebug.println(command);
        
        if (command == "help" || command == ".")
        {
            //! print time
            if (_ntpServerInit)
                SerialDebug.println(_timeClient->getFormattedTime());
            
            getTimeStamp_mainModule();
            
            SerialDebug.println("Boostrap OTA, type one of the following:");
            SerialDebug.println("   status  -- shows status");

            SerialDebug.println("   r - reboot");
            SerialDebug.println("   help");
            SerialDebug.println("   . (also help)");
            SerialDebug.println();

            SerialDebug.println(" **** WIFI INFO ****");
            SerialDebug.println("    ** enter 'ssid:<ssid>' etc");

            SerialDebug.printf( "   ssid:%s\n", _ssid);
            SerialDebug.printf( "   wifi:%s\n", _wifiPassword);
            SerialDebug.println("   connect -- try to connect");
            SerialDebug.println("   NTP Time -- get current time");

            SerialDebug.println();
            SerialDebug.println(" **** OTA UPDATE ***");
            SerialDebug.println("   bootstrap - this program");

            SerialDebug.println("   m5atom");
            SerialDebug.println("   m5atomDaily");

            SerialDebug.println();
            SerialDebug.println(" *** Specify OTA manually ***");
            SerialDebug.println("   hostOTA:<url>");
            SerialDebug.println("   binOTA:<bin name>");
            SerialDebug.println("   grabOTA -- perform OTA");


            SerialDebug.println();
            SerialDebug.println(" *** STATUS ***");

            get_WIFIInfoString();

            // SerialDebug.println("m5");
#ifdef TEST_JSON
            SerialDebug.println();
            SerialDebug.println("** JSON Array Testing **");
            SerialDebug.println("jsonAdd:name,mapping");
            SerialDebug.println("jsonLookup:name");
            SerialDebug.println("jsonMappings");
            SerialDebug.println("jsonPersist"); //  writes to memory....
            SerialDebug.println("jsonInit"); //  reads from memory....

#endif
        }
        //! 7.20.25
        else if (command.startsWith("NTP") || command.startsWith("ntp"))
        {
            
            int timeNow = getTimeStamp_mainModule();

        }
        else if (command.startsWith("status"))
        {
            SerialDebug.printf("ssid=%s, wifi=%s\n", _ssid, _wifiPassword);
            SerialDebug.println(wifiStatus_MQTT());
            String get_WIFIInfoString();
            
            SerialDebug.println(" Specify OTA Host/Bin");
            SerialDebug.printf("hostOTA=%s, binOTA:%s\n", _hostOTA, _binOTA);

        }
        else if (command.startsWith("r"))
        {
            SerialDebug.println("REBOOT");
            //reboot
            ESP.restart();
        }
        //! https://docs.arduino.cc/language-reference/en/variables/data-types/stringObject/Functions/startsWith/
        else if (command.startsWith("ssid:"))
        {
            //String subset = "SunnyWhiteriver";
            int colon = command.indexOf(":");
            String subset = command.substring(colon+1);
            strcpy(_ssid, subset.c_str());
            //! save in EPROM
            savePreference((char*)PREFERENCES_SSID, (char*)_ssid);
        }
        else if (command.startsWith("connect"))
        {
            tryConnect();
        }
        else if (command.startsWith("wifi:"))
        {
            //String subset = "sunny2021";
            int colon = command.indexOf(":");
            String subset = command.substring(colon+1);
            strcpy(_wifiPassword,  subset.c_str());
            
            //! save in EPROM
            savePreference((char*)PREFERENCES_WIFI, (char*)_wifiPassword);
            
            //! try connecting
            tryConnect();
        }
        else if (command == "bootstrap")
        {
            SerialDebug.println(" *** performing OTA Update of this BOOTSTRAP program");
            
            //!retrieves from constant location
            performOTAUpdate((char*)"http://KnowledgeShark.org", (char*)"OTA/Bootstrap/ESP_M5_BOOTSTRAP.ino.m5stack_stickc_plus.bin");
        }
        else if (command == "m5atom")
        {
            SerialDebug.println(" *** performing m5atom OTA Update");
            
            //!retrieves from constant location
            performOTAUpdate((char*)"http://KnowledgeShark.org", (char*)"OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin");
        }
        else if (command == "m5atomDaily")
        {
            SerialDebug.println(" *** performing m5atom OTA Update - DAILY");
            
            //!retrieves from constant location
            performOTAUpdate((char*)"http://KnowledgeShark.org", (char*)"OTA/TEST/M5Atom/daily/ESP_IOT.ino.m5stick_c_plus.bin");
        }
        //! 7.18.25
        else if (command.startsWith("hostOTA:"))
        {
            int colon = command.indexOf(":");
            String subset = command.substring(colon+1);
            //!retrieves from constant location
            strcpy(_hostOTA, subset.c_str());
        }
        else if (command.startsWith("binOTA:"))
        {
            int colon = command.indexOf(":");
            String subset = command.substring(colon+1);
            //!retrieves from constant location
            strcpy(_binOTA, subset.c_str());
        }
        else if (command.startsWith("grabOTA"))
        {
            SerialDebug.printf(" *** performing custom OTA Update: %s/%s", _hostOTA, _binOTA);
            
            //!retrieves from constant location
            performOTAUpdate(_hostOTA, _binOTA);
        }
       
#ifdef TEST_JSON
        else if (command.startsWith("jsonAdd:"))
        {
            //SerialDebug.println("jsonAdd:name,mapping");
            int colon = command.indexOf(":");
            String subset = command.substring(colon+1);
            int comma = subset.indexOf(",");
            String name = subset.substring(0,comma);
            char *nameC = (char*) name.c_str();
            String mapping = subset.substring(comma+1);
            char *mappingC = (char*)mapping.c_str();
            //! strip spaces.. TODO
            
            //! call addMapping
            addMapping(nameC, mappingC);
        }
        else if (command.startsWith("jsonLookup:"))
        {
            //SerialDebug.println("jsonLookup:name");
            int colon = command.indexOf(":");
            String subset = command.substring(colon+1);
            char *nameC = (char*)subset.c_str();
            //! call getMapping
            char *mapping = getMapping(nameC);
            //! print result
            SerialDebug.printf("Mapping = %s\n", mapping);
        }
        else if (command.startsWith("jsonMappings"))
        {
            showMappings();
        }
        else if (command.startsWith("jsonPersist"))
        {
            jsonPersist();
        }
        else if (command.startsWith("jsonInit"))
        {
            jsonInit();
        }
#endif
        else
        {
            SerialDebug.printf("*** Unnown Command: %s\n", command);
        }
    }
}
