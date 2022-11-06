#include "db.h"

Database* init_database(char* file_name) {
	Database* db = (Database*)malloc(sizeof(Database*));
	Storage* storage = init_storage(file_name);
	db->storage = storage;
	
	return db;
}

void create_tag(Database* db, Tag tag) {
	Data_to_add data = {
		.tag = tag,
		.type = TAG_ENTITY
	};
	
	add_entity(db->storage, &data);
}

void delete_tag(Database* db, char* tag_name) {
	int idxDelta = 100;
	
	Getted_entities* = get_entities(db->storage, ALL, TAG_ENTITY, 0, idxDelta);
}