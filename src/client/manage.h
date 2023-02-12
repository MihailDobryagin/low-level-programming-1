#ifndef manage_h
#define manage_h

#include "../db/entities.h"
#include "queries.h"
#include "../db/db.h"

Tag tag_info(Database* db, Get_tag query);
void create_tag(Database* db, Create_tag query);
void delete_tag(Database* db, Delete_tag query);

Array_node nodes(Database* db, Select_nodes query);
void create_node(Database* db, Create_node query);
void delete_nodes(Database* db, Select_nodes query);
void change_node(Database* db, Change_node query);

Array_edge edges(Database* db, Select_edges query);
void create_edge(Database* db, Create_edge query);
void delete_edges(Database* db, Select_edges query);
void change_edge(Database* db, Change_edge query);

#endif // !manage_h