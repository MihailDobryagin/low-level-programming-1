#include "manage.h"
#include "../db/db.h"
#include <assert.h>

void create_tag(Database* db, Create_tag query) {
	insert_tag(db, query.tag);
}

Tag tag_info(Database* db, Get_tag query) {
	return get_tag(db, query.tag_name);
}

void create_node(Database* db, Create_node query) {
	return insert_node(db, query.node);
}

Array_node nodes(Database* db, Get_nodes query) {
	Array_node nodes_for_tag = get_nodes(db, query.tag_name);
	Array_node result = { 0, (Node*) malloc(sizeof(Node) * nodes_for_tag.size) };

	switch (query.selection_mode) {
		case ALL_NODES: result = nodes_for_tag;  break;
		case IDS:
			for (uint32_t i = 0; i < nodes_for_tag.size; i++) {
				for (uint32_t id_idx = 0; id_idx < query.target_ids_size; id_idx++) {
					if (compare_fields(nodes_for_tag.values[i].id, query.ids[id_idx])) {
						result.values[result.size++] = nodes_for_tag.values[i];
					}
				}
			}
			break;

		case FILTER:
			for (uint32_t i = 0; i < nodes_for_tag.size; i++) {
				if (query.predicate(nodes_for_tag.values[i])) {
					result.values[result.size++] = nodes_for_tag.values[i];
				}
			}
			break;
		default:
			printf("No available modes for get nodes");
			assert(0);
			break;
	}

	return result;
}
// void delete_node(Delete_nodes query);
// void update_nodes(Update_nodes query);

// void insert_edge(Insert_edge query);
// Edge* edges(Get_edges query);
// void delete_edges(Delete_edges query);
// void update_edges(Update_edges query);
