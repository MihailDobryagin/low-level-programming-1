// TODO Tests
// TODO Make different user-optimizations
// TODO NOT TESTED
// TODO memory NOT MANAGED
#include "file.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

uint32_t _calc_field_size(Field field);
uint32_t _calc_property_size(Property field);
void _put_field(uint8_t* buff, Field field);
void _put_property(uint8_t* buff, Property prop);
Header_block _store_tag(Storage* storage, uint32_t header_offset, uint32_t data_offset, Tag* tag);
Header_block _store_node(FILE* file, uint32_t header_offset, uint32_t data_offset, Node* node);
Header_block _store_edge(FILE* file, uint32_t header_offset, uint32_t data_offset, Edge* edge);
Node* _create_nodes(uint32_t blocks_number, uint32_t* sizes, uint8_t* data);
void _update_metadata(Storage* storage);
void _expand_storage(Storage* storage);
void _collapse_storage(Storage* storage);

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
			.data_offset = sizeof(Metadata) + sizeof(Header_block)
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

void close_storage(Storage* storage) {
	fclose(storage->file);
	free(storage);
}

void add_entity(Storage* storage, Data_to_add* data) {
	FILE* file = storage->file;
	Metadata* metadata = &storage->metadata;
	
	assert (metadata->blocks_size < metadata->blocks_capacity);
	
	uint32_t header_offset = metadata->headers_offset + metadata->blocks_size * sizeof(Header_block);
	uint32_t data_offset = metadata->data_offset + metadata->data_size;
	
	switch(data->type) {
		case TAG_ENTITY: store_tag(storage, header_offset, data_offset, &data->tag); break;
		case NODE_ENTITY: store_node(file, header_offset, data_offset, &data->node); break;
		case EDGE_ENTITY: store_edge(file, header_offset, data_offset, &data->edge); break;
		default: assert(0);
	}
	
	update_metadata(storage);
}

void* get_entities(Storage* storage, Getting_mode mode, Entity_type type, uint32_t start_index, uint32_t number_of_blocks) {
	FILE* file = storage->file;
	Metadata metadata = storage->metadata;
	
	uint32_t current_size = 0;
	Header_block* headers = (Header_block*)malloc(sizeof(Header_block) * number_of_blocks);
	
	if(mode == ALL) {
		Header_block* headers_buff = (Header_block*)malloc(sizeof(Header_block) * metadata.blocks_size);
		fread(headers_buff, sizeof(Header_block), metadata.blocks_size, file);
		
		uint32_t passed_blocks = 0;
		uint32_t matched_blocks_number = 0;
		uint32_t* matched_blocks_sizes = (uint32_t*)malloc(sizeof(uint32_t) * number_of_blocks);
		uint32_t* matched_blocks = (uint32_t*)malloc(sizeof(uint32_t) * number_of_blocks); // indexes
		uint32_t matched_data_size = 0;
		
		for(uint32_t i = 0; i < metadata.blocks_size && current_size < number_of_blocks; i++) {
			Header_block header = headers_buff[i];
			if(header.status != WORKING || header.type != type) continue;
			if(passed_blocks++ != start_index) continue;
			
			matched_data_size += header.data_size;
			matched_blocks[matched_blocks_number] = i;
			matched_blocks_sizes[matched_blocks_number] = header.data_size;
			matched_blocks_number++;
		}
		
		assert(matched_blocks_number == number_of_blocks); // TODO
		
		uint8_t* data_buff = (uint8_t*)malloc(matched_data_size);
		uint8_t* cur_data_buff_addr = data_buff;
		
		for(uint32_t i = 0; i < number_of_blocks; i++) {
			Header_block header = headers_buff[matched_blocks[i]];
			fseek(file, header.data_offset, SEEK_SET);
			fread(cur_data_buff_addr, header.data_size, 1, file);
			cur_data_buff_addr += header.data_size;
		}
		
		switch(type) {
			case NODE_ENTITY: return create_nodes(matched_blocks_number, matched_blocks_sizes, data_buff);
		}
		
	}	
}

void _update_metadata(Storage* storage) {
	fseek(storage->file, 0, SEEK_SET);
	fwrite(&(storage->metadata), sizeof(Metadata), 1, storage->file);
}

void _expand_storage(Storage* storage) {
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
	
	update_metadata();
}

void _collapse_storage(Storage* storage) {
	
}


Header_block _store_tag(Storage* storage, uint32_t header_offset, uint32_t data_offset, Tag* tag) {
	FILE* file = storage->file;
	
	uint32_t type_size = sizeof(Tag_type);
	uint32_t name_size = strlen(tag->name);
	uint32_t properties_size = tag->properties_size;
	uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t property_types_size = sizeof(Type) * properties_size;
	uint32_t property_names_size = 0;
	uint32_t id_size = sizeof(uint32_t);
	
	for(uint32_t i = 0; i < properties_size; i++) {
		property_names_size += strlen(tag->property_names[i]);
	}
	
	uint32_t data_size = id_size + type_size + name_size + 1 + properties_size_size + property_types_size + property_names_size + properties_size;
	
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	uint8_t* cur_buff_addr = data_buff;
	
	// Write _type
	*(cur_buff_addr) = tag->id;
	cur_buff_addr += type_size;
	
	// Write _name
	strcpy(cur_buff_addr, tag->name);
	cur_buff_addr += name_size + 1;
	
	// Write _properties_size
	*(cur_buff_addr) = properties_size;
	cur_buff_addr += sizeof(uint32_t);
	
	// Write _property_types + _property_names
	property_names_size = 0;
	uint32_t property_names_offset = sizeof(Type) * properties_size;
	for(uint32_t i = 0; i < properties_size; i++) {
		*(cur_buff_addr + sizeof(Type) * i) = tag->property_types[i];
		property_names_size += strlen(tag->property_names[i]) + 1;
	}
	
	for(uint32_t i = 0; i < properties_size; i++) {
		uint32_t name_len = strlen(tag->property_names[i]);
		strcpy(cur_buff_addr + property_names_offset + property_names_size, tag->property_names[i]);
		property_names_size += name_len + 1;
	}
	
	// Write id
	*(cur_buff_addr) = tag->type;
	cur_buff_addr += sizeof(Type);
	
	// Store
	Header_block header = {TAG_ENTITY, WORKING, data_offset, data_size};
	
	fseek(file, header_offset, SEEK_SET);
	printf("offset : %lld\n", data_offset);
	printf("size   : %lld\n", data_size);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(&data_buff, sizeof(uint8_t), data_size, file);
	
	storage->metadata.blocks_size++;
	storage->metadata.data_size += data_size;
}

Header_block _store_node(FILE* file, uint32_t header_offset, uint32_t data_offset, Node* node) {
	uint32_t tag_id_size = sizeof(uint32_t);
	uint32_t id_size = calc_field_size(node->id);
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

Header_block _store_edge(FILE* file, uint32_t header_offset, uint32_t data_offset, Edge* extended_edge) {
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

uint32_t _calc_property_size(Property prop) {
	return strlen(prop.name) + calc_field_size(prop.field);
}

void _put_field(uint8_t* buff, Field field) {
	*buff = field.type;
	uint8_t* val_addr = buff + sizeof(Type);
	switch(field.type) {
		case STRING: strcpy(val_addr, field.string); break;
		case BYTE: *val_addr = field.byte; break;
		case NUMBER: *val_addr = field.number; break;
		case BOOLEAN: *val_addr = (uint8_t) field.boolean; break;
		case CHARACTER: *val_addr = (uint8_t) field.character; break;
		default: assert(0);
	}
}

void _put_property(uint8_t* buff, Property prop) {
	strcpy(buff, prop.name);
	put_field(buff + strlen(prop.name), prop.field);
}

Field _scan_field(uint8_t** stream) {
	uint8_t* cur_addr = *stream;
	Type type = *(Type*)cur_addr;
	*stream += sizeof(Type);
	
	union {
		int8_t byte;
		char* string;
		int32_t number;
		bool boolean;
		char character;
	} value;
	
	uint32_t string_len;
	
	switch(type) {
		case BYTE: 
			value.byte = *(uint8_t*)stream; 
			*stream += sizeof(int8_t); 
			return (Field){type, .byte = value.byte};
		case CHARACTER: 
			value.character = *(char*)stream; 
			*stream += sizeof(char); 
			return (Field){type, .character = value.character};;
		case BOOLEAN: 
			value.boolean = *(uint8_t*)stream != 0;
			*stream += sizeof(uint8_t); 
			return (Field){type, .boolean = value.boolean};
		case NUMBER: 
			value.number = *(int32_t*)stream; 
			*stream += sizeof(int32_t); 
			return (Field){type, .number = value.number};
		case STRING: 
			string_len = strlen((char*)stream);
			value.string = (char*)malloc(string_len + 1);
			strcpy(value.string, *stream);
			value.string[string_len] = '\0';
			*stream += string_len + 1;
			return (Field){type, .string = value.string};
		default: assert(0);
	}
}

Property _scan_property(uint8_t* stream) {
	uint32_t name_len = strlen(((Property*)stream)->name);
	char* name = (char*)malloc(name_len + 1);
	strcpy(name, ((Property*)stream)->name);
	*stream += name_len + 1;
	Field field = scan_field(&stream);
	return (Property){name, field};
}

Node* _create_nodes(uint32_t blocks_number, uint32_t* sizes, uint8_t* data) {
	Node* nodes = (Node*)malloc(sizeof(Node) * blocks_number);
	
	Node* cur_node_addr = nodes;
	for(uint32_t i = 0; i < blocks_number; i++, cur_node_addr += sizeof(Node)) {
		uint8_t* param_addr = (uint8_t*)cur_node_addr;
		
		// Fill tag_id
		cur_node_addr->tag_id = *((uint32_t*)param_addr);
		param_addr += sizeof(uint32_t);
		
		// Fill id
		cur_node_addr->id = scan_field(&data);
		
		// Fill properties_size
		cur_node_addr->properties_size = *((uint32_t*)param_addr);
		param_addr += sizeof(uint32_t);
		
		// Fill properties
		cur_node_addr->properties = (Property*)malloc(sizeof(Property) * cur_node_addr->properties_size);
		for(uint32_t prop_idx = 0; prop_idx < cur_node_addr->properties_size; prop_idx++) {
			cur_node_addr->properties[prop_idx] = scan_property(data);
		}
	}
	
	return nodes;
}