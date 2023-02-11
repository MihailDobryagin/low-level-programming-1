#ifndef strings_h
#define strings_h

#include "../db/entities.h"

char* type_as_str(Type type);
char* field_as_str(Field field);
char* num_as_str(int64_t value);

#endif //!strings_h