#include "../src/db/file.h"
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

void check_init_storage() {
	FILE* file = fopen("test_file", "wb+");
	Metadata metadata = {1, 2, 3, 4, 5};
	fwrite(&metadata, sizeof(Metadata), 1, file);
	fclose(file);

	Storage* storage = init_storage("test_file");
	
	assert(storage->metadata.blocks_size == 1);
	assert(storage->metadata.blocks_capacity == 2);
	assert(storage->metadata.data_size == 3);
	assert(storage->metadata.headers_offset == 4);
	assert(storage->metadata.data_offset == 5);
	
	close_storage(storage);
}

void check_init_storage_when_file_is_empty() {
	FILE* file = fopen("test_file", "w");
	fclose(file);

	Storage* storage = init_storage("test_file");
	
	assert(storage->metadata.blocks_size == 0);
	assert(storage->metadata.blocks_capacity == 10);
	assert(storage->metadata.data_size == 0);
	assert(storage->metadata.headers_offset == sizeof(Metadata));
	assert(storage->metadata.data_offset == sizeof(Metadata));
	
	close_storage(storage);
}

// TODO Complete after testing get-queries
void check_add_entity() {
	FILE* file = fopen("test_file", "w");
	fclose(file);
	
	Storage* storage = init_storage("test_file");
	Type property_type = BYTE;
	char* property_name = "prop_name";
	
	Tag tag = {NODE_TAG_TYPE, "tag1", 1, &property_type, &property_name};
	Extended_tag extended_tag = {
		.tag = tag, 
		.id = 1
	};
	Data_to_add data_to_add = {
		.tag = tag,
		.type = TAG_ENTITY
	};
	
	add_entity(storage, &data_to_add);
	
	close_storage(storage);
	
	
}

void main() {
	check_init_storage();
	check_init_storage_when_file_is_empty();
	check_add_entity();
}