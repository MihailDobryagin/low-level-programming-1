#ifndef manage_h
#define manage_h

#include "../db/entities.h"
#include "queries.h"
#include "../db/db.h"

Tag tag_info(Database* db, Get_tag query);
void create_tag(Database* db, Create_tag query);
void delete_tag(Database* db, Delete_tag query);

 Array_node nodes(Database* db, Get_nodes query);
 void create_node(Database* db, Create_node query);
 void delete_node(Database* db, Delete_nodes query);
 void update_nodes(Database* db, Update_nodes query);

// void insert_edge(Insert_edge query);
// Edge* edges(Get_edges query);
// void delete_edges(Delete_edges query);
// void update_edges(Update_edges query);

#endif // !manage_h