#include "db.h"
#include <stdlib.h>
#include <string.h>

Database* init_database(char* file_name) {
	Database* db = (Database*)malloc(sizeof(Database));
	Storage* storage = init_storage(file_name);
	db->storage = storage;
	
	return db;
}

Tag get_tag(Database* db, char* tag_name) {
	int idx_delta = 100;
	int cur_idx = 0;
	
	Getted_entities* selected = NULL;
	while(selected == NULL) {
		selected = get_entities(db->storage, ALL, TAG_ENTITY, cur_idx, idx_delta);
		Tag* tags = (Tag*)selected->entities;
		if(selected->size == 0) {
			selected = NULL;
			continue;
		}
		
		for(uint32_t i = 0; i < selected->size; i++) {
			if(strcmp(tags[i].name, tag_name) == 0) {
				Tag result = tags[i];
				free(selected);
				return result;
			}
		}
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
		if(selected->size == 0) {
			selected = NULL;
			continue;
		}
		
		for(uint32_t i = 0; i < selected->size; i++) {
			if(strcmp(tags[i].name, tag_name) == 0) {
				uint32_t block_id = selected->block_ids[0];
				delete_entitites(db->storage, 1, &block_id);
				free(selected);
				return;
			}
		}
	}
}