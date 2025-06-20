
#include "JSON_Module.h"
#include <ArduinoJson.h>

#ifdef TEST_JSON

//! storage of JSON
DynamicJsonDocument _myJSON(1024);
//! for returning values in the JSON
char _returnBuffer[100];

//! will be an array..
String _mappingJSON =
"{\"mappings\"[{\"set\":\"SM:QHmwUurxC3.170380669dsfa\",\"val\":\"https://iDogWatc.com/bot/cmddevice/x/y/z/togglesocket\"},{\"set\":\"SM:QHmwUurxC.1703806697279\",\"val\":\"https://iDogWatch.com/bot/cmddevice/x/y/z/togglesocket\"}]}";
 
//! EPROM value... for now
String _mappingJSON2 =
"[{\"set\":\"SM:QHmwUurxC3.170380669dsfa\",\"val\":\"https://iDogWatch.com/bot/cmddevice/x/y/z/togglesocket\"},{\"set\":\"SM:QHmwUurxC.1703806697279\",\"val\":\"https://iDogWatch.com/bot/cmddevice/x/y/z/togglesocket\"}]";

void setup_JSON_Module()
{
    //! read from memory...
    //! doesn't like syntax of array..
    if (false)
    {
        DynamicJsonDocument myObject(1024);

        //StaticJsonDocument myObject(1024);
        SerialDebug.println(_mappingJSON);
        deserializeJson(myObject, _mappingJSON);
        SerialDebug.print("JSON parsed.1 = ");
        String output1;
        serializeJsonPretty(myObject, output1);
        SerialDebug.println(output1);
    }
    
    //! don't touch the EPROM (updated below in jsonPersist
    {
        //! parses the string _mappingJSON2
        deserializeJson(_myJSON, _mappingJSON2);
        SerialDebug.print("JSON parsed.1 = ");
        String output1;
        serializeJsonPretty(_myJSON, output1);
        SerialDebug.println(output1);
    }
}
//! add a mapping (eg. SM:uuid.flow, https://somewhere
void addMapping(char *name, char *mapping)
{
   
    DynamicJsonDocument newMapping(200);

    SerialDebug.printf("Adding mapping(%s,%s)\n", name, mapping);
    newMapping["set"] = name;
    newMapping["val"] = mapping;
    
    serializeJsonPretty(newMapping,Serial);
    //!TODO new addNested()
    //!
    //! now add to global
    _myJSON.add(newMapping);
    serializeJsonPretty(_myJSON,Serial);

}

//! save in EPROM
void jsonPersist()
{
    String j;
    //! output _myJSON into _mappingJSON2 string
    serializeJsonPretty(_myJSON, j);
    //! save globally
    _mappingJSON2 = j;
    SerialDebug.printf("EPROM WILL BE: \n%s\n", _mappingJSON2.c_str());
    
    //! save in EPROM..
}


//! read from EPROM
void jsonInit()
{
    setup_JSON_Module();
}

//! gets a mapping (eg. SM:uuid.flow, https://somewhere
char *getMapping(char *name)
{
    SerialDebug.printf("getMapping(%s)\n", name);
    
    // extract the values
    //!@see https://forum.arduino.cc/t/how-to-iterate-through-json-or-otherwise-check-if-value-exists-in-json-solved/969396
        
    for (JsonObject elem : _myJSON.as<JsonArray>()) {
        const char* set = elem["set"];
        const char* val = elem["val"];
        //Serial.printf("%s -> %s\n", set, val);
        
        if (strcmp(set, name) == 0)
        {
            SerialDebug.printf("***** FOUND: %s -> %s\n", name, val);
            strcpy(_returnBuffer, val);
            return _returnBuffer;
        }
        
        // or the dreaded String versions
        //String set2 = elem["set"];
        //Serial.println(set2);
    }
    
    return (char*)"unknown";
}

//! prints mappings (eg. SM:uuid.flow, https://somewhere
void showMappings()
{
    for (JsonObject elem : _myJSON.as<JsonArray>()) {
        const char* set = elem["set"];
        const char* val = elem["val"];
        Serial.printf("%s -> %s\n", set, val);
        
        
        // or the dreaded String versions
        //String set2 = elem["set"];
        //Serial.println(set2);
    }
}

#endif
