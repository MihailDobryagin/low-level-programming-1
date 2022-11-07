#include "manage.h"
#include "../db/db.h"

void create_tag(Database* db, Create_tag query) {
	insert_tag(db, query.tag);
}

Tag tag_info(Database* db, Get_tag query) {
	return get_tag(db, query.tag_name);
}

// Node insert_node(Insert_node query);
// void delete_node(Delete_nodes query);
// void update_nodes(Update_nodes query);

// void insert_edge(Insert_edge query);
// Edge* edges(Get_edges query);
// void delete_edges(Delete_edges query);
// void update_edges(Update_edges query);
