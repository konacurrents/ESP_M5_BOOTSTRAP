#ifndef JSONModule_h
#define JSONModule_h
#include "../../Defines.h"

#ifdef TEST_JSON
void setup_JSON_Module();

//! add a mapping (eg. SM:uuid.flow, https://somewhere)
void addMapping(char *name, char *mapping);

//! gets a mapping (eg. SM:uuid.flow, https://somewhere)
char *getMapping(char *name);

//! prints mappings (eg. SM:uuid.flow, https://somewhere)
void showMappings();

//! save in EPROM
void jsonPersist();

//! read from EPROM
void jsonInit();
#endif

#endif
