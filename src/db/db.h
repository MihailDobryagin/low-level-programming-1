#ifndef db_h
#define db_h

#include "entities.h"
#include "file.h"

typedef struct {
	Storage* storage;
} Database;

Database* init_database(char* file_name);

Tag get_tag(Database* db, char* tag_name);
void insert_tag(Database* db, Tag tag);
void drop_tag(Database* db, char* tag_name);

#endif // !db_h