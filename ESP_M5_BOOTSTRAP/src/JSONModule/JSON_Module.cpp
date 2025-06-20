
#include "JSON_Module.h"
#include <ArduinoJson.h>

#ifdef TEST_JSON

DynamicJsonDocument _myObject(1024);

//! add a mapping (eg. SM:uuid.flow, https://somewhere
void addMapping(char *name, char *mapping)
{
    SerialDebug.printf("Adding mapping(%s,%s)\n", name, mapping);
}

//! gets a mapping (eg. SM:uuid.flow, https://somewhere
char *getMapping(char *name)
{
    return (char*)"unknown";
}

//! prints mappings (eg. SM:uuid.flow, https://somewhere
void showMappings()
{

}

#endif
