#include "db.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "entities.h"

Database* init_database(char* file_name) {
	Database* db = (Database*)malloc(sizeof(Database));
	Storage* storage = init_storage(file_name);
	db->storage = storage;
	
	return db;
}

void close_database(Database* db) {
	close_storage(db->storage);
}

Tag get_tag(Database* db, char* tag_name) {
	const uint32_t idx_delta = 100;
	uint32_t cur_idx = 0;
	
	Getted_entities* selected = NULL;
	while(selected == NULL) {
		selected = get_entities(db->storage, ALL, TAG_ENTITY, cur_idx, idx_delta);
		Tag* tags = (Tag*)selected->entities;

		if(selected->size == 0) {
			printf("No any tags were found");
			assert(1);
		}
		
		for(uint32_t i = 0; i < selected->size; i++) {
			if(strcmp(tags[i].name, tag_name) == 0) {
				Tag result = tags[i];
				free(selected);
				return result;
			}
		}
		cur_idx += idx_delta;
		free(selected);
		selected = NULL;
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
	int idx_delta = 100;
	int cur_idx = 0;
	
	Getted_entities* selected = NULL;
	while(selected == NULL) {
		selected = get_entities(db->storage, ALL, TAG_ENTITY, cur_idx, idx_delta);
		Tag* tags = (Tag*)selected->entities;
		if (selected->size == 0) {
			printf("No any tags were found");
			assert(0);
		}
		
		for(uint32_t i = 0; i < selected->size; i++) {
			if(strcmp(tags[i].name, tag_name) == 0) {
				uint32_t block_id = selected->block_ids[0];
				delete_entitites(db->storage, 1, &block_id);
				free(selected);
				return;
			}
		}
		cur_idx += idx_delta;
		free(selected);
		selected = NULL;
	}
}

Array_node get_nodes(Database* db, char* tag_name) {
	const uint32_t idx_delta = 100;
	const uint32_t cur_idx = 0;
	const uint32_t result_initial_size = 100;

	Getted_entities* selected = NULL;

	Node* result = (Node*)malloc(sizeof(Node) * result_initial_size);
	uint32_t current_size = 0;
	uint32_t current_capacity = result_initial_size;
	
	while (selected == NULL) {
		selected = get_entities(db->storage, ALL, NODE_ENTITY, cur_idx, idx_delta);
		Node* nodes = (Node*)selected->entities;

		if (selected->size == 0) {
			break;
		}

		for (uint32_t i = 0; i < selected->size; i++) {
			if (strcmp(nodes[i].tag, tag_name) == 0) {
				result[current_size++] = nodes[i];
				if (current_size == current_capacity) {
					current_capacity = current_capacity * 3 / 2;
					result = (Node*) realloc(result, current_capacity);
				}
			}
		}
		free(selected);
		selected = NULL;
	}

	return (Array_node) { current_size, result };
}

void insert_node(Database* db, Node node) {
	Data_to_add data = {
		.node= node,
		.type = NODE_ENTITY
	};

	add_entity(db->storage, &data);
}

void drop_node(Database* db, char* tag_name, Field id) {
	int idx_delta = 100;
	int cur_idx = 0;

	Getted_entities* selected = NULL;
	while (selected == NULL) {
		selected = get_entities(db->storage, ALL, NODE_ENTITY, cur_idx, idx_delta);
		Node* nodes = (Node*)selected->entities;
		if (selected->size == 0) {
			printf("No any nodes were found");
			assert(0);
		}

		for (uint32_t i = 0; i < selected->size; i++) {
			if (strcmp(nodes[i].tag, tag_name) == 0 && compare_fields(id, nodes[i].id)) {
				uint32_t block_id = selected->block_ids[0];
				delete_entitites(db->storage, 1, &block_id);
				free(selected);
				return;
			}
		}
	}
}