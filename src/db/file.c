#include "file.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

Storage* init_storage(char* file_name) {
	FILE* file = fopen(file_name, "rb+");
	Metadata* metadata_buff = (Metadata*)malloc(sizeof(Metadata));
	size_t readen_for_metadata = fread(metadata_buff, sizeof(Metadata), 1, file);
	
	Storage* storage = (Storage*)malloc(sizeof(Storage*));
	storage->file = file;
	
	if(readen_for_metadata == 0) {
		printf("Metadata was not found");
		
		Metadata metadata = {
			.blocks_size = 0,
			.blocks_capacity = 0,
			.data_size = 0,
			.headers_offset = sizeof(Metadata),
			.data_offset = 0
		};
		
		metadata_buff = &metadata;
		fwrite(metadata_buff, sizeof(Metadata), 1, file);
		storage->metadata = metadata;
	}
	else {
		storage->metadata = *metadata_buff;
	}
	
	return storage;
}

void add_entity(Storage* storage, Data_to_add* data) {
	FILE* file = storage->file;
	Metadata* metadata = &storage->metadata;
	
	assert (metadata->blocks_size < metadata->blocks_capacity);
	
	uint32_t header_offset = metadata->headers_offset + metadata->blocks_size * sizeof(Header_block);
	uint32_t data_offset = metadata->data_offset + metadata->data_size;
	
	Entity_type data_type = data->type;
}

void expand_storage(Storage* storage) {
	Metadata metadata = storage->metadata;
	
	uint32_t capacity_diff = metadata.blocks_capacity / 2;
	uint32_t new_capacity = metadata.blocks_capacity + capacity_diff;
	
	
}

void collapse_storage(Storage* storage) {
	
}

void close_storage(Storage* storage) {
	fclose(storage->file);
	free(storage);
}

Header_block store_tag(FILE* file, uint32_t header_offset, uint32_t data_offset, Extended_tag* extended_tag) {
	Tag* tag = (Tag*) tag;
	
	uint32_t type_size = sizeof(Tag_type);
	uint32_t name_size = strlen(tag->name);
	uint32_t properties_size = tag->properties_size;
	uint32_t property_types_size = sizeof(Type) * properties_size;
	uint32_t property_names_size = 0;
	
	for(uint32_t i = 0; i < properties_size; i++) {
		property_names_size += strlen(tag->property_names[i]);
	}
	
	uint32_t data_size = type_size + name_size + properties_size + property_types_size + property_names_size;
	
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	
	// Write _type
	uint8_t* cur_buff_addr = data_buff;
	*(cur_buff_addr) = tag->type;
	
	// Write _name
	cur_buff_addr += type_size;
	strcpy(cur_buff_addr, tag->name); // We have \0 at the end
	
	// Write _properties_size
	cur_buff_addr += name_size;
	*(cur_buff_addr) = properties_size;
	
	// Write _property_types + _property_names
	cur_buff_addr += sizeof(uint32_t);
	property_names_size = 0;
	uint32_t property_names_offset = sizeof(Type) * properties_size;
	for(uint32_t i = 0; i < properties_size; i++) {
		*(cur_buff_addr + sizeof(Type) * i) = tag->property_types[i];
		property_names_size += strlen(tag->property_names[i]);
	}
	
	for(uint32_t i = 0; i < properties_size; i++) {
		if(i + 1 == properties_size) {
			uint32_t name_len = strlen(tag->property_names[i]);
			for(uint32_t charIdx = 0; charIdx < name_len; charIdx++) {
				*(cur_buff_addr + property_names_offset + property_names_size + charIdx) = tag->property_names[charIdx];
			}
		}
		else {
			strcpy(cur_buff_addr + property_names_offset + property_names_size, tag->property_names[i]);
		}
		property_names_size += strlen(tag->property_names[i]);
	}
	
	// Store
	
	Header_block header = {TAG_ENTITY, WORKING, data_offset, data_size};
	
	fseek(file, header_offset, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(&data_buff, sizeof(uint8_t), data_size, file);
}

