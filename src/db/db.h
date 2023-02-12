#ifndef db_h
#define db_h

#include "file.h"

typedef struct {
	Storage* storage;
} Database;

Database* init_database(char* file_name);
void close_database(Database* db);

Tag get_tag(Database* db, char* tag_name);
void insert_tag(Database* db, Tag tag);
void drop_tag(Database* db, char* tag_name);

Array_node get_nodes(Database* db, char* tag_name);
void insert_node(Database* db, Node node);
void drop_node(Database* db, char* tag_name, Field id);
void update_node(Database* db, Node changed_node);

Array_edge get_edges(Database* db, char* tag_name);
void insert_edge(Database* db, Edge edge);
void drop_edge(Database* db, char* tag_name, Field id);
void update_edge(Database* db, Edge changed_edge);

#endif // !db_h