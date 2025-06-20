#include "MainModule.h"

#include <string>
//#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>


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

char _ssid[100];
char _password[100];

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

void setup_mainModule()
{
    SerialDebug.println("**** type on the serial monitor. Start with 'help'  ****");
#ifdef TEST_JSON
    setup_JSON_Module();
#endif
    
#pragma mark HARDCODE -- MODIFY THIS TO MAKE IT EASIER
    strcpy(_ssid,(char*)"SSID");
    strcpy(_password, (char*)"PASSWORD");
}

//! uses the SSID and PASSWORD
void tryConnect()
{
    int count = 0;
    SerialDebug.printf("Trying Connection: %s, %s\n", _ssid, _password);

    //!setup the WIFI.begin
    setupWIFI(_ssid, _password);
    
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
    }
}

//! main loop
void loop_mainModule()
{
    //! see if data on the serial input
    if (Serial.available())
    {
        // read string until meet newline character
        String command = Serial.readStringUntil('\n');
        
        SerialDebug.println(command);
        
        if (command == "help")
        {
            SerialDebug.println("Boostrap OTA, type one of the following:");
            SerialDebug.println("WIFI INFO:");
            SerialDebug.println("ssid:<ssid>");
            SerialDebug.println("wifi:<password>  -- this will try to connect");
            SerialDebug.println("connect -- try to connect");

            SerialDebug.println("status  -- shows status");
            SerialDebug.println("r - reboot");

            
            SerialDebug.println();
            SerialDebug.println("OTA UPDATE:");
            
            SerialDebug.println("m5atom");
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
        else if (command.startsWith("status"))
        {
            SerialDebug.printf("ssid=%s, wifi=%s\n", _ssid, _password);
            SerialDebug.println(wifiStatus_MQTT());
            String get_WIFIInfoString();
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
            strcpy(_password,  subset.c_str());
            
            //! try connecting
            tryConnect();
        }
        else if (command == "m5atom")
        {
            SerialDebug.println(" *** performing m5atom OTA Update");
            
            //!retrieves from constant location
            performOTAUpdate((char*)"http://KnowledgeShark.org", (char*)"OTA/TEST/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin");
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
