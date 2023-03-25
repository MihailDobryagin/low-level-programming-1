#include "db.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "entities.h"

static const uint32_t idx_delta = 20;

Database* init_database(char* file_name) {
	Database* db = (Database*)malloc(sizeof(Database));
	Storage* storage = init_storage(file_name);
	db->storage = storage;
	
	return db;
}

void close_database(Database* db) {
	close_storage(db->storage);
	free(db);
}

Tag get_tag(Database* db, char* tag_name) {
	uint32_t cur_idx = 0;
	
	Getted_entities selected;                                                                                          // FREE_INTERNAL
	while(true) {
		selected = get_entities(db->storage, ALL, TAG_ENTITY, cur_idx, idx_delta);
		Tag* tags = (Tag*)selected.entities;

		if(selected.size == 0) {
			printf("No any tags were found");
			assert(1);
		}
		
		for(uint32_t i = 0; i < selected.size; i++) {
			if(strcmp(tags[i].name, tag_name) == 0) {
				Tag result = tags[i];
				for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) {
					if (i_to_free == i) continue;
					free_tag_internal(tags[i_to_free]);
				}
				free(selected.entities);
				return result;
			}
		}
		for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) {
			free_tag_internal(tags[i_to_free]);
		}
		free(selected.entities);
		cur_idx += idx_delta;
	}
}

void insert_tag(Database* db, Tag tag) {
	Data_to_add data = {
		.tag = tag,
		.type = TAG_ENTITY
	};
	
	add_entity(db->storage, &data);
}

void drop_tag(Database* db, char* tag_name) {
	int cur_idx = 0;
	
	Getted_entities selected;                                                                                          // FREE_INTERNAL
	while(true) {
		selected = get_entities(db->storage, ALL, TAG_ENTITY, cur_idx, idx_delta);
		Tag* tags = (Tag*)selected.entities;
		if (selected.size == 0) {
			printf("No any tags were found");
			assert(0);
		}
		
		for(uint32_t i = 0; i < selected.size; i++) {
			if(strcmp(tags[i].name, tag_name) == 0) {
				const uint32_t block_id = selected.block_ids[i];
				delete_entitites(db->storage, 1, &block_id);
				for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) free_tag_internal(tags[i_to_free]);
				free(selected.entities);
				return;
			}
		}
		for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) free_tag_internal(tags[i_to_free]);
		free(selected.entities);
		cur_idx += idx_delta;
	}
}

Array_node get_nodes(Database* db, char* tag_name) {
	uint32_t cur_idx = 0;
	const uint32_t result_initial_size = 100;

	Getted_entities selected;                                                                                          // FREE_INTERNAL

	Node* result = (Node*)malloc(sizeof(Node) * result_initial_size);
	uint32_t current_size = 0;
	uint32_t current_capacity = result_initial_size;
	
	while (true) {
		selected = get_entities(db->storage, ALL, NODE_ENTITY, cur_idx, idx_delta);
		Node* nodes = (Node*)selected.entities;

		if (selected.size == 0) {
			break;
		}
		
		for (uint32_t i = 0; i < selected.size; i++) {
			if (tag_name == NULL || strcmp(nodes[i].tag, tag_name) == 0) {
				result[current_size++] = nodes[i];
				if (current_size == current_capacity) {
					current_capacity = current_capacity * 3 / 2;
					result = (Node*)realloc(result, sizeof(Node) * current_capacity);
				}
			}
			else free_node_internal(nodes[i]);
		}
		free(selected.entities);
		cur_idx += idx_delta;
	}

	return (Array_node) { current_size, result };
}

void insert_node(Database* db, Node node) {
	Data_to_add data = {
		.node = node,
		.type = NODE_ENTITY
	};

	add_entity(db->storage, &data);
}

void drop_node(Database* db, char* tag_name, Field id) {
	int cur_idx = 0;

	Getted_entities selected;                                                                                          // FREE_INTERNAL
	while (true) {
		selected = get_entities(db->storage, ALL, NODE_ENTITY, cur_idx, idx_delta);
		Node* nodes = (Node*)selected.entities;
		if (selected.size == 0) {
			printf("No any nodes were found");
			assert(0);
		}

		for (uint32_t i = 0; i < selected.size; i++) {
			if (strcmp(nodes[i].tag, tag_name) == 0 && compare_fields(id, nodes[i].id)) {
				uint32_t block_id = selected.block_ids[0];
				delete_entitites(db->storage, 1, &block_id);
				for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) {
					if (i_to_free == i) continue;
					free_node_internal(nodes[i_to_free]);
				}
				free(selected.entities);
				return;
			}
		}

		for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) free_node_internal(nodes[i_to_free]);
		free(selected.entities);
		cur_idx += idx_delta;
	}
}

void update_node(Database* db, Node node) {
	int cur_idx = 0;

	Getted_entities selected;                                                                                          // FREE_INTERNAL
	while (true) {
		selected = get_entities(db->storage, ALL, NODE_ENTITY, cur_idx, idx_delta);
		Node* nodes = (Node*)selected.entities;
		if (selected.size == 0) {
			printf("No any nodes were found");
			assert(0);
		}

		for (uint32_t i = 0; i < selected.size; i++) {
			if (strcmp(nodes[i].tag, node.tag) == 0 && compare_fields(node.id, nodes[i].id)) {
				uint32_t block_id = selected.block_ids[0];
				Data_to_add data_to_update = { .node = node, .type = NODE_ENTITY };
				update_entities(db->storage, 1, &block_id, &data_to_update);
				for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) {
					if (i_to_free == i) continue;
					free_node_internal(nodes[i_to_free]);
				}
				free(selected.entities);
				return;
			}
		}
		for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) free_node_internal(nodes[i_to_free]);
		free(selected.entities);
		cur_idx += idx_delta;
	}
}

Array_edge get_edges(Database* db, char* tag_name) {
	uint32_t cur_idx = 0;
	const uint32_t result_initial_size = 100;

	Getted_entities selected;

	Edge* result = (Edge*)malloc(sizeof(Edge) * result_initial_size);
	uint32_t current_size = 0;
	uint32_t current_capacity = result_initial_size;

	while (true) {
		selected = get_entities(db->storage, ALL, EDGE_ENTITY, cur_idx, idx_delta);
		Edge* edges = (Edge*)selected.entities;

		if (selected.size == 0) {
			break;
		}

		for (uint32_t i = 0; i < selected.size; i++) {
			if (tag_name == NULL || strcmp(edges[i].tag, tag_name) == 0) {
				result[current_size++] = edges[i];
				if (current_size == current_capacity) {
					current_capacity = current_capacity * 3 / 2;
					result = (Edge*)realloc(result, sizeof(Edge) * current_capacity);
				}
			}
			else free_edge_internal(edges[i]);
		}
		free(selected.entities);
		cur_idx += idx_delta;
	}

	return (Array_edge) { current_size, result };
}

void insert_edge(Database* db, Edge edge) {
	Data_to_add data = {
		.edge= edge,
		.type = EDGE_ENTITY
	};

	add_entity(db->storage, &data);
}

void drop_edge(Database* db, char* tag_name, Field id) {
	int cur_idx = 0;

	Getted_entities selected;
	while (true) {
		selected = get_entities(db->storage, ALL, EDGE_ENTITY, cur_idx, idx_delta);
		Edge* edges= (Edge*)selected.entities;
		if (selected.size == 0) {
			printf("No any nodes were found");
			assert(0);
		}

		for (uint32_t i = 0; i < selected.size; i++) {
			if (strcmp(edges[i].tag, tag_name) == 0 && compare_fields(id, edges[i].id)) {
				uint32_t block_id = selected.block_ids[0];
				delete_entitites(db->storage, 1, &block_id);
				for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) {
					if (i_to_free == i) continue;
					free_edge_internal(edges[i_to_free]);
				}
				free(selected.entities);
				return;
			}
		}
		for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) free_edge_internal(edges[i_to_free]);
		free(selected.entities);
		cur_idx += idx_delta;
	}
}

void update_edge(Database* db, Edge edge) {
	uint32_t cur_idx = 0;

	Getted_entities selected;
	while (true) {
		selected = get_entities(db->storage, ALL, EDGE_ENTITY, cur_idx, idx_delta);
		Edge* edges = (Edge*)selected.entities;
		if (selected.size == 0) {
			printf("No any nodes were found");
			assert(0);
		}

		for (uint32_t i = 0; i < selected.size; i++) {
			if (strcmp(edges[i].tag, edge.tag) == 0 && compare_fields(edge.id, edges[i].id)) {
				uint32_t block_id = selected.block_ids[0];
				Data_to_add data_to_update = { .edge = edge, .type = NODE_ENTITY };
				update_entities(db->storage, 1, &block_id, &data_to_update);
				for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) {
					free_edge_internal(edges[i_to_free]);
				}
				free(selected.entities);
				return;
			}
		}
		for (uint32_t i_to_free = 0; i_to_free < selected.size; i_to_free++) free_edge_internal(edges[i_to_free]);
		free(selected.entities);
		cur_idx += idx_delta;
	}
}