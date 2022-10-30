#ifndef file_h
#define file_h

#include "local_entities.h"

Storage* init_storage(char* file_name);

Header_block allocate_block(Storage* storage, Header_block header);

void add(Storage* storage, Header_block* header);

void collapse(Storage* storage);

void close_storage(Storage* storage);

#endif // !file_h