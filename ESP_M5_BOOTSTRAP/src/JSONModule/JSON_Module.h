#ifndef JSONModule_h
#define JSONModule_h
#include "../../Defines.h"

#ifdef TEST_JSON

//! add a mapping (eg. SM:uuid.flow, https://somewhere)
void addMapping(char *name, char *mapping);

//! gets a mapping (eg. SM:uuid.flow, https://somewhere)
char *getMapping(char *name);

//! prints mappings (eg. SM:uuid.flow, https://somewhere)
void showMappings();
#endif

#endif
