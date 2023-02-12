#include "manage.h"
#include "../db/db.h"
#include <assert.h>



void create_tag(Database* db, Create_tag query) {
	insert_tag(db, query.tag);
}

Tag tag_info(Database* db, Get_tag query) {
	return get_tag(db, query.tag_name);
}

void delete_tag(Database* db, Delete_tag query) {
	drop_tag(db, query.tag_name);
}

void create_node(Database* db, Create_node query) {
	return insert_node(db, query.node);
}

Array_node nodes(Database* db, Select_nodes query) {
	Array_node nodes_for_tag = get_nodes(db, query.tag_name);
	Array_node result = { 0, (Node*) malloc(sizeof(Node) * nodes_for_tag.size) };

	switch (query.selection_mode) {
		case ALL_NODES: result = nodes_for_tag;  break;
		case NODE_IDS:
			for (uint32_t i = 0; i < nodes_for_tag.size; i++) {
				for (uint32_t id_idx = 0; id_idx < query.target_ids_size; id_idx++) {
					if (compare_fields(nodes_for_tag.values[i].id, query.ids[id_idx])) {
						result.values[result.size++] = nodes_for_tag.values[i];
					}
				}
			}
			break;

		case NODE_FILTER:
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

void delete_nodes(Database* db, Select_nodes query) {
	Array_node getted_nodes = nodes(db, query);
	for (int i = 0; i < getted_nodes.size; i++) {
		drop_node(db, getted_nodes.values[i].tag, getted_nodes.values[i].id);
	}
}

void change_node(Database* db, Change_node query) {
	update_node(db, query.changed_node);
}

Array_edge edges(Database* db, Select_edges query) {
	Array_edge edges_for_tag = get_edges(db, query.tag_name);
	Array_edge result = { 0, (Edge*)malloc(sizeof(Edge) * edges_for_tag.size) };

	switch (query.selection_mode) {
		case ALL_NODES: result = edges_for_tag;  break;
		case EDGE_IDS:
			for (uint32_t i = 0; i < edges_for_tag.size; i++) {
				for (uint32_t id_idx = 0; id_idx < query.target_ids_size; id_idx++) {
					if (compare_fields(edges_for_tag.values[i].id, query.ids[id_idx])) {
						result.values[result.size++] = edges_for_tag.values[i];
					}
				}
			}
			break;
		case BY_LINKED_NODE:
			for (uint32_t i = 0; i < edges_for_tag.size; i++) {
				if (compare_fields(edges_for_tag.values[i].node1_id, query.node_id) || compare_fields(edges_for_tag.values[i].node2_id, query.node_id)) {
					result.values[result.size++] = edges_for_tag.values[i];
				}
			}
			break;
		case EDGE_FILTER:
			for (uint32_t i = 0; i < edges_for_tag.size; i++) {
				if (query.predicate(edges_for_tag.values[i])) {
					result.values[result.size++] = edges_for_tag.values[i];
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

void create_edge(Database* db, Create_edge query) {
	insert_edge(db, query.edge);
}

void delete_edges(Database* db, Select_edges query) {
	Array_edge getted_edges = edges(db, query);
	for (int i = 0; i < getted_edges.size; i++) {
		drop_edge(db, getted_edges.values[i].tag, getted_edges.values[i].id);
	}
}

void change_edge(Database* db, Change_edge query) {
	update_edge(db, query.changed_edge);
}
