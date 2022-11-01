// TODO Tests
// TODO Make different user-optimizations
#include "file.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

Storage* init_storage(char* file_name) {
	FILE* file = fopen(file_name, "rb+");
	Metadata* metadata_buff = (Metadata*)malloc(sizeof(Metadata));
	size_t readen_for_metadata = fread(metadata_buff, sizeof(Metadata), 1, file);
	
	Storage* storage = (Storage*)malloc(sizeof(Storage));
	storage->file = file;
	
	if(readen_for_metadata == 0) {
		printf("Metadata was not found\n");
		
		Metadata metadata = {
			.blocks_size = 0,
			.blocks_capacity = 10,
			.data_size = 0,
			.headers_offset = sizeof(Metadata),
			.data_offset = sizeof(Metadata)
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

Header_block store_tag(FILE* file, uint32_t header_offset, uint32_t data_offset, Extended_tag* extended_tag);
Header_block store_node(FILE* file, uint32_t header_offset, uint32_t data_offset, Extended_node* extended_node);
Header_block store_edge(FILE* file, uint32_t header_offset, uint32_t data_offset, Extended_edge* extended_edge);

void add_entity(Storage* storage, Data_to_add* data) {
	FILE* file = storage->file;
	Metadata* metadata = &storage->metadata;
	
	assert (metadata->blocks_size < metadata->blocks_capacity);
	
	uint32_t header_offset = metadata->headers_offset + metadata->blocks_size * sizeof(Header_block);
	uint32_t data_offset = metadata->data_offset + metadata->data_size;
	
	switch(data->type) {
		case TAG_ENTITY: store_tag(file, header_offset, data_offset, &data->tag); break;
		case NODE_ENTITY: store_node(file, header_offset, data_offset, &data->node); break;
		case EDGE_ENTITY: store_edge(file, header_offset, data_offset, &data->edge); break;
		default: assert(0);
	}
}

void expand_storage(Storage* storage) {
	Metadata* metadata = &(storage->metadata);
	
	uint32_t capacity_diff = metadata->blocks_capacity / 4; // TODO Make dynamic coeff
	uint32_t new_capacity = metadata->blocks_capacity + capacity_diff;
	
	uint32_t target_last_header_addr = metadata->headers_offset + metadata->blocks_size * new_capacity; // excluding
	
	uint32_t count_of_matching_blocks = 0;
	uint32_t count_of_matching_blocks_cap = 5; // capacity
	uint32_t* blocks_to_move = (uint32_t*)malloc(sizeof(uint32_t) * count_of_matching_blocks_cap); // indexes of headers
	
	fseek(storage->file, metadata->headers_offset, SEEK_SET);
	Header_block* header_buff = (Header_block*)malloc(sizeof(Header_block));
	for(uint32_t i = 0; i < metadata->blocks_size; i++) {
		fread(header_buff, sizeof(Header_block), 1, storage->file);
		if(header_buff->data_offset < target_last_header_addr) {
			if(count_of_matching_blocks == count_of_matching_blocks_cap) {
				count_of_matching_blocks_cap += count_of_matching_blocks_cap / 2; 
				blocks_to_move = (uint32_t*)realloc(blocks_to_move, sizeof(uint32_t) * count_of_matching_blocks_cap);
			}
			blocks_to_move[count_of_matching_blocks++] = i;
		}
	}
	
	for(int i = 0; i < count_of_matching_blocks; i++) {
		fseek(storage->file, metadata->headers_offset, SEEK_SET);
		fread(header_buff, sizeof(Header_block), 1, storage->file);
		uint8_t* data = (uint8_t*)malloc(header_buff->data_size);
		fseek(storage->file, header_buff->data_offset, SEEK_SET);
		fread(data, header_buff->data_size, 1, storage->file);
		fseek(storage->file, metadata->data_offset + metadata->data_size, SEEK_SET);
		fwrite(data, header_buff->data_size, 1, storage->file);
		metadata->data_size += header_buff->data_size;
	}
}

void collapse_storage(Storage* storage) {
	
}

void close_storage(Storage* storage) {
	fclose(storage->file);
	free(storage);
}

uint32_t calc_field_size(Field field);
uint32_t calc_property_size(Property field);
void put_field(uint8_t* buff, Field field);
void put_property(uint8_t* buff, Property prop);

Header_block store_tag(FILE* file, uint32_t header_offset, uint32_t data_offset, Extended_tag* extended_tag) {
	Tag* tag = (Tag*) extended_tag;
	
	uint32_t id_size = sizeof(uint32_t);
	uint32_t type_size = sizeof(Tag_type);
	uint32_t name_size = strlen(tag->name);
	uint32_t properties_size = tag->properties_size;
	uint32_t property_types_size = sizeof(Type) * properties_size;
	uint32_t property_names_size = 0;
	
	for(uint32_t i = 0; i < properties_size; i++) {
		property_names_size += strlen(tag->property_names[i]);
	}
	
	uint32_t data_size = id_size + type_size + name_size + properties_size + property_types_size + property_names_size;
	
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	uint8_t* cur_buff_addr = data_buff;
	
	// Write id
	*(cur_buff_addr) = tag->type;
	cur_buff_addr += sizeof(uint32_t);
	
	// Write _type
	*(cur_buff_addr) = extended_tag->id;
	cur_buff_addr += type_size;
	
	// Write _name
	strcpy(cur_buff_addr, tag->name); // We have \0 at the end
	cur_buff_addr += name_size;
	
	// Write _properties_size
	*(cur_buff_addr) = properties_size;
	cur_buff_addr += sizeof(uint32_t);
	
	// Write _property_types + _property_names
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
				*(cur_buff_addr + property_names_offset + property_names_size + charIdx) = (uint8_t)tag->property_names[i][charIdx];
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

Header_block store_node(FILE* file, uint32_t header_offset, uint32_t data_offset, Extended_node* extended_node) {
	uint32_t tag_id_size = sizeof(uint32_t);
	uint32_t id_size = calc_field_size(extended_node->id);
	uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t properties_result_size = 0;
	
	for(uint32_t i = 0; i < extended_node->properties_size; i++) {
		properties_result_size += calc_property_size(extended_node->properties[i]);
	}
	
	uint32_t data_size = tag_id_size + id_size + properties_size_size + properties_result_size;
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	uint8_t* cur_buff_addr = data_buff;
	
	// Write _tag_id
	*(cur_buff_addr) = extended_node->tag_id;
	cur_buff_addr += tag_id_size;
	
	// Write _id
	put_field(cur_buff_addr, extended_node->id);
	cur_buff_addr += id_size;
	
	// Write _properties_size
	*(cur_buff_addr) = extended_node->properties_size;
	cur_buff_addr += properties_size_size;
	
	// Write _properties
	for(uint32_t i = 0; i < extended_node->properties_size; i++) {
		put_property(cur_buff_addr, extended_node->properties[i]);
		cur_buff_addr += calc_property_size(extended_node->properties[i]);
	}
	
	// Store
	Header_block header = {NODE_ENTITY, WORKING, data_offset, data_size};
	
	fseek(file, header_offset, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(&data_buff, sizeof(uint8_t), data_size, file);
}

Header_block store_edge(FILE* file, uint32_t header_offset, uint32_t data_offset, Extended_edge* extended_edge) {
	uint32_t tag_id_size = sizeof(uint32_t);
	uint32_t id_size = calc_field_size(extended_edge->id);
	uint32_t node_id_size = calc_field_size(extended_edge->node1_id); // *2 in result calculating
	uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t properties_result_size = 0;
	
	for(uint32_t i = 0; i < extended_edge->properties_size; i++) {
		properties_result_size += calc_property_size(extended_edge->properties[i]);
	}
	
	uint32_t data_size = tag_id_size + id_size + node_id_size * 2 + properties_size_size + properties_result_size;
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	uint8_t* cur_buff_addr = data_buff;
	
	// Write _tag_id
	*(cur_buff_addr) = extended_edge->tag_id;
	cur_buff_addr += tag_id_size;
	
	// Write _id
	put_field(cur_buff_addr, extended_edge->id);
	cur_buff_addr += id_size;
	
	// Write _node_ids
	put_field(cur_buff_addr, extended_edge->node1_id);
	cur_buff_addr += node_id_size;
	put_field(cur_buff_addr, extended_edge->node2_id);
	cur_buff_addr += node_id_size;
	
	// Write _properties_size
	*(cur_buff_addr) = extended_edge->properties_size;
	cur_buff_addr += properties_size_size;
	
	// Write _properties
	for(uint32_t i = 0; i < extended_edge->properties_size; i++) {
		put_property(cur_buff_addr, extended_edge->properties[i]);
		cur_buff_addr += calc_property_size(extended_edge->properties[i]);
	}
	
	// Store
	Header_block header = {NODE_ENTITY, WORKING, data_offset, data_size};
	
	fseek(file, header_offset, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(&data_buff, sizeof(uint8_t), data_size, file);
}

uint32_t calc_field_size(Field field) {
	switch(field.type) {
		case BYTE: return 1;
		case NUMBER: return 4;
		case BOOLEAN: return 1;
		case CHARACTER: return 1;
		case STRING: return strlen(field.string);
		default: assert(0);
	}
}

uint32_t calc_property_size(Property prop) {
	return strlen(prop.name) + calc_field_size(prop.field);
}

void put_field(uint8_t* buff, Field field) {
	*buff = field.type;
	uint8_t* val_addr = buff + sizeof(Type);
	switch(field.type) {
		case STRING:
			for(uint32_t offset = strlen(field.string) - 1; offset - 1; offset >= 0) {
				*(buff + sizeof(Type) + offset) = field.string[offset];
			}
			break;
		case BYTE: *val_addr = field.byte; break;
		case NUMBER: *val_addr = field.number; break;
		case BOOLEAN: *val_addr = (uint8_t) field.boolean; break;
		case CHARACTER: *val_addr = (uint8_t) field.character; break;
		default: assert(0);
	}
}

void put_property(uint8_t* buff, Property prop) {
	strcpy(buff, prop.name);
	put_field(buff + strlen(prop.name), prop.field);
}