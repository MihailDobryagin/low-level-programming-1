#include "../src/db/file.h"
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

bool header_block_equals(Header_block b1, Header_block b2) {
	return b1.type == b2.type
		&& b1.status == b2.status
		&& b1.data_offset == b2.data_offset
		&& b1.data_size == b2.data_size;
}

bool tag_equals(Tag t1, Tag t2) {
	bool result = 
		t1.type == t2.type
		&& strcmp(t1.name, t2.name) == 0
		&& t1.properties_size == t2.properties_size;
	if(result == false) return false;
	
	uint32_t properties_size = t1.properties_size;
	
	for(int i = 0; i < property_size && result; i++) {
		result &&= 
			t1.property_types[i] == t2.property_types[i]
			&& strcmp(t1.property_names[i], t2.property_names[i]) == 0;
	}
	
	return result;
}

bool extended_tag_equals(Extended_tag t1, Extended_tag t2) {
	return tag_equals((Tag)t1, (Tag)t2) && t1.id == t2.id;
}

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
	assert(storage->metadata.data_offset == sizeof(Metadata) + sizeof(Header_block) * storage->metadata.blocks_capacity);
	
	close_storage(storage);
}

void check_get_nodes() {
	FILE* file = fopen("test_file", "wb+");
	Property* properties = (Property*)malloc(sizeof(Property) * 2);
	properties[0] = (Property){
		.name = "name1",
		.field = (Field) {CHARACTER, '-'}
	};
	
	properties[1] = (Property){
		.name = "name2",
		.field = (Field) {CHARACTER, '+'}
	};
	
	Extended_node node = {
		.tag_id = 123,
		.id = {NUMBER, .number = -123456},
		.properties_size = 2,
		.properties = properties
	};
	
	// Write tag_id
	fwrite(&node.tag_id, sizeof(uint32_t), 1, file);
	
	// Write id
	fwrite(&node.id.type, sizeof(Type), 1, file);
	fwrite(&node.id.number, sizeof(int32_t), 1, file);
	
	// Write properties
	fwrite(properties[0].name, 5, 1, file); // 1
	char symbol = '\0';
	fwrite(&symbol, 1, 1, file);
	fwrite(&properties[0].field.type, sizeof(Type), 1, file);
	symbol = '-';
	fwrite(&symbol, 1, 1, file);
	
	fwrite(properties[1].name, 5, 1, file); // 2
	symbol = '\0';
	fwrite(&symbol, 1, 1, file);
	fwrite(&properties[1].field.type, sizeof(Type), 1, file);
	symbol = '+';
	fwrite(&symbol, 1, 1, file);
	
	fclose(file);
	
	Storage* storage = init_storage("test_file");
	
	// TODO
}

void check_get_entities() {
	FILE* file = fopen("test_file", "w");
	fclose(file);
}

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
	
	file = fopen("test_file", "rb");
	Metadata metadata;
	fread(&metadata, sizeof(Metadata), 1, file);
	printf("blocks_num -> %ld\n", metadata.blocks_size);
	
	uint32_t capacity = 10;
	
	uint32_t expected_data_size = sizeof(Tag_type) + 5 + sizeof(uint32_t) + sizeof(Type)*1 + 10 + sizeof(Entity_type);
	
	Header_block expected_header = {TAG_ENTITY, WORKING, sizeof(Metadata) + sizeof(Header_block) * capacity, expected_data_size};
	fseek(file, sizeof(Metadata), SEEK_SET);
	Header_block actual_header;
	fread(&actual_header, sizeof(Header_block), 1, file);
	
	assert(header_block_equals(actual_header, expected_header));
	
	Extended_tag tag;

	uint8_t* actual_data = (uint8_t*)malloc(expected_header.data_size);
	fseek(file, expected_header.data_offset, SEEK_SET);
	fread(actual_data, expected_header.data_size, 1, file);
	
	uint8_t* expected_data = (uint8_t*)malloc(expected_header.data_size);
	
	// TODO May be we dont need to check it. Only result operations?
}

void main() {
	check_init_storage();
	check_init_storage_when_file_is_empty();
	check_add_entity();
	check_get_nodes();
}