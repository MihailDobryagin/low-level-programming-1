#ifndef db_h
#define db_h

#include "entities.h"
#include "file.h"

typedef struct {
	Storage* storage;
	
} Database;

Database* init_database(char* file_name);

void create_tag(Database* db, Tag tag);
void delete_tag(Database* db, char* tag_name);



#endif // !db_h