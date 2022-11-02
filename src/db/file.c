// TODO NOT TESTED
// TODO Make different user-optimizations
#include "file.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

uint32_t _calc_field_size(Field field);
uint32_t _calc_property_size(Property field);
void _put_field(uint8_t* buff, Field field);
void _put_property(uint8_t* buff, Property prop);
Field _scan_field(uint8_t** stream);
Property _scan_property(uint8_t** stream);
void _store_tag(Storage* storage, uint32_t header_offset, uint32_t data_offset, Tag* tag);
void _store_node(Storage* storage, uint32_t header_offset, uint32_t data_offset, Node* node);
void _store_edge(Storage* storage, uint32_t header_offset, uint32_t data_offset, Edge* edge);
Node* _create_nodes(uint32_t blocks_number, uint32_t* sizes, uint8_t* data);
Edge* _create_edges(uint32_t blocks_number, uint32_t* sizes, uint8_t* data);
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
		uint32_t init_capacity = 10;
		Metadata metadata = {
			.blocks_size = 0,
			.draft_blocks_size = 0,
			.blocks_capacity = init_capacity,
			.data_size = 0,
			.headers_offset = sizeof(Metadata),
			.data_offset = sizeof(Metadata) + sizeof(Header_block) * init_capacity
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
	
	uint32_t new_header_offset = metadata->headers_offset + sizeof(Header_block) * metadata->blocks_size;
	uint32_t new_data_offset = metadata->data_offset + metadata->data_size;
	
	switch(data->type) {
		case TAG_ENTITY: _store_tag(storage, new_header_offset, new_data_offset, &data->tag); break;
		case NODE_ENTITY: _store_node(storage, new_header_offset, new_data_offset, &data->node); break;
		case EDGE_ENTITY: _store_edge(storage, new_header_offset, new_data_offset, &data->edge); break;
		default: assert(0);
	}
	
	if(metadata->blocks_size == metadata->blocks_capacity) _expand_storage(storage);
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
			case NODE_ENTITY: return _create_nodes(matched_blocks_number, matched_blocks_sizes, data_buff);
			case EDGE_ENTITY: return _create_edges(matched_blocks_number, matched_blocks_sizes, data_buff);
			default:
				assert(0);
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
	
	_update_metadata(storage);
}

void _collapse_storage(Storage* storage) {
	uint32_t left_idx = 0;
	uint32_t right_idx = storage->metadata.draft_blocks_size - 1;
	uint32_t headers_offset = storage->metadata.headers_offset;
	
	// uint32_t* newIndexesOfReplacedHeaders = (uint32_t*)malloc()
	fseek(storage->file, headers_offset, SEEK_SET);
	Header_block* header_buff = (Header_block*)malloc(sizeof(Header_block)); // FREE
	
	for(;left_idx < right_idx; left_idx++) {
		fread(header_buff, sizeof(Header_block), 1, storage->file);
		if(header_buff->status == DRAFT) {
			for(;left_idx < right_idx; right_idx--) {
				fseek(storage->file, headers_offset + sizeof(Header_block) * right_idx, SEEK_SET);
				fread(header_buff, sizeof(Header_block), 1, storage->file);
				if(header_buff->status == WORKING) {
					fseek(storage->file, headers_offset + sizeof(Header_block) * left_idx, SEEK_SET); // mb, set EMPTY status?
					fwrite(header_buff, sizeof(Header_block), 1, storage->file);
					right_idx--;
					break;
				}
			}
		}
	}
	
	uint32_t new_blocks_size = left_idx;
	
	// when left_idx == right_idx it can point to one header, and we don't know its status
	fseek(storage->file, headers_offset + sizeof(Header_block) * left_idx, SEEK_SET);
	fread(header_buff, sizeof(Header_block), 1, storage->file);
	if(header_buff->status == WORKING) new_blocks_size++;
	free(header_buff);
	
	storage->metadata.blocks_size = new_blocks_size;
	storage->metadata.draft_blocks_size = 0;
	
	_update_metadata(storage);
}


void _store_tag(Storage* storage, uint32_t header_offset, uint32_t data_offset, Tag* tag) {
	uint32_t type_size = sizeof(Tag_type);
	uint32_t name_size = strlen(tag->name);
	uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t property_types_size = sizeof(Type) * tag->properties_size;
	uint32_t property_names_size = 0;
	
	for(uint32_t i = 0; i < tag->properties_size; i++) {
		property_names_size += strlen(tag->property_names[i]) + 1;
	}
	
	uint32_t data_size = type_size + (name_size + 1) + properties_size_size + property_types_size + property_names_size;
	
	uint8_t* data_buff = (uint8_t*)malloc(data_size); // FREE
	uint8_t* cur_buff_addr = data_buff;
	
	// Write _type
	*(cur_buff_addr) = tag->type;
	cur_buff_addr += type_size;
	
	// Write _name
	strcpy(cur_buff_addr, tag->name);
	cur_buff_addr += name_size + 1;
	
	// Write _properties_size
	*(cur_buff_addr) = tag->properties_size;
	cur_buff_addr += sizeof(uint32_t);
	
	// Write _property_types + _property_names
	uint32_t prev_property_names_size = 0;
	uint32_t property_names_offset = sizeof(Type) * tag->properties_size;
	for(uint32_t i = 0; i < tag->properties_size; i++) {
		*(cur_buff_addr + sizeof(Type) * i) = tag->property_types[i];
		strcpy(cur_buff_addr + property_names_offset + prev_property_names_size, tag->property_names[i]);
		prev_property_names_size += strlen(tag->property_names[i]) + 1;
	}
	cur_buff_addr += property_types_size + property_names_size;
	
	// Store
	FILE* file = storage->file;
	
	Header_block header = {TAG_ENTITY, WORKING, data_offset, data_size};
	fseek(file, header_offset, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(data_buff, sizeof(uint8_t), data_size, file);
	free(data_buff);
	
	storage->metadata.blocks_size++;
	storage->metadata.data_size += data_size;
	_update_metadata(storage);
}

void _store_node(Storage* storage, uint32_t header_offset, uint32_t data_offset, Node* node) {
	uint32_t tag_name_size = strlen(node->tag);
	uint32_t id_size = _calc_field_size(node->id);
	uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t properties_result_size = 0;
	
	for(uint32_t i = 0; i < node->properties_size; i++) {
		properties_result_size += _calc_property_size(node->properties[i]);
	}
	
	uint32_t data_size = tag_name_size + 1 + id_size + properties_size_size + properties_result_size;
	
	uint8_t* data_buff = (uint8_t*)malloc(data_size); // FREE
	uint8_t* cur_buff_addr = data_buff;
	
	// Write tag_name
	strcpy(cur_buff_addr, node->tag);
	cur_buff_addr += tag_name_size + 1;
	
	// Write id
	_put_field(cur_buff_addr, node->id);
	cur_buff_addr += id_size;
	
	// Write _properties_size
	*(cur_buff_addr) = node->properties_size;
	cur_buff_addr += properties_size_size;
	
	// Write _properties
	for(uint32_t i = 0; i < node->properties_size; i++) {
		_put_property(cur_buff_addr, node->properties[i]);
		cur_buff_addr += _calc_property_size(node->properties[i]);
	}
	
	// Store
	Header_block header = {NODE_ENTITY, WORKING, data_offset, data_size};
	
	FILE* file = storage->file;
	fseek(file, header_offset, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(data_buff, sizeof(uint8_t), data_size, file);
	free(data_buff);
}

void _store_edge(Storage* storage, uint32_t header_offset, uint32_t data_offset, Edge* edge) {
	uint32_t tag_name_size = strlen(edge->tag);
	uint32_t id_size = _calc_field_size(edge->id);
	uint32_t node1_id_size = _calc_field_size(edge->node1_id);
	uint32_t node2_id_size = _calc_field_size(edge->node2_id);
	uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t properties_result_size = 0;
	
	for(uint32_t i = 0; i < edge->properties_size; i++) {
		properties_result_size += _calc_property_size(edge->properties[i]);
	}
	
	uint32_t data_size = (tag_name_size + 1) + id_size + node1_id_size + node2_id_size  + properties_size_size + properties_result_size;
	uint8_t* data_buff = (uint8_t*)malloc(data_size); // FREE
	uint8_t* cur_buff_addr = data_buff;
	
	// Write tag_name
	strcpy(cur_buff_addr, edge->tag);
	cur_buff_addr += tag_name_size + 1;
	
	// Write _id
	_put_field(cur_buff_addr, edge->id);
	cur_buff_addr += id_size;
	
	// Write _node_ids
	_put_field(cur_buff_addr, edge->node1_id);
	cur_buff_addr += node1_id_size;
	_put_field(cur_buff_addr, edge->node2_id);
	cur_buff_addr += node2_id_size;
	
	// Write _properties_size
	*(cur_buff_addr) = edge->properties_size;
	cur_buff_addr += properties_size_size;
	
	// Write _properties
	for(uint32_t i = 0; i < edge->properties_size; i++) {
		_put_property(cur_buff_addr, edge->properties[i]);
		cur_buff_addr += _calc_property_size(edge->properties[i]);
	}
	
	// Store
	FILE* file = storage->file;
	
	Header_block header = {NODE_ENTITY, WORKING, data_offset, data_size};
	fseek(file, header_offset, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(data_buff, sizeof(uint8_t), data_size, file);
	free(data_buff);
}

uint32_t _calc_field_size(Field field) {
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
	return strlen(prop.name) + _calc_field_size(prop.field);
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
	_put_field(buff + strlen(prop.name), prop.field);
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

Property _scan_property(uint8_t** stream) {
	uint32_t name_len = strlen(((Property*)stream)->name);
	char* name = (char*)malloc(name_len + 1);
	strcpy(name, ((Property*)stream)->name);
	*stream += name_len + 1;
	Field field = _scan_field(stream);
	return (Property){name, field};
}

Node _create_node(uint8_t* data) {
	uint32_t tag_name_len = strlen((char*)data);
	char* tag_name = (char*)malloc(tag_name_len);
	strcpy(tag_name, (char*)data);
	data += tag_name_len;
	
	Field id = _scan_field(&data);
	
	uint32_t properties_size = *((uint32_t*)data);
	data += sizeof(uint32_t);
	
	Property* properties = (Property*)malloc(sizeof(Property) * properties_size);
	
	for(int i = 0; i < properties_size; i++) {
		properties[i] = _scan_property(&data);
	}
	
	return (Node) {tag_name, id, properties_size, properties};
}

Node* _create_nodes(uint32_t blocks_number, uint32_t* sizes, uint8_t* data) {
	Node* nodes = (Node*)malloc(sizeof(Node) * blocks_number);
	Node* cur_node_addr = nodes;
	
	for(uint32_t i = 0; i < blocks_number; i++, cur_node_addr += sizeof(Node)) {
		uint8_t* param_addr = (uint8_t*)cur_node_addr;
		
		nodes[i] = _create_node((uint8_t*)cur_node_addr);
	}
	
	return nodes;
}

Edge _create_edge(uint8_t* data) {
	uint32_t tag_name_len = strlen((char*)data);
	char* tag_name = (char*)malloc(tag_name_len);
	strcpy(tag_name, (char*)data);
	data += tag_name_len;
	
	Field id = _scan_field(&data);
	
	Field node1_id = _scan_field(&data);
	Field node2_id = _scan_field(&data);
	
	uint32_t properties_size = *((uint32_t*)data);
	data += sizeof(uint32_t);
	
	Property* properties = (Property*)malloc(sizeof(Property) * properties_size);
	
	for(int i = 0; i < properties_size; i++) {
		properties[i] = _scan_property(&data);
	}
	
	return (Edge) {tag_name, id, node1_id, node2_id, properties_size, properties};
}

Edge* _create_edges(uint32_t blocks_number, uint32_t* sizes, uint8_t* data) {
	Edge* edges = (Edge*)malloc(sizeof(Edge) * blocks_number);
	Edge* cur_edge_addr = edges;
	
	for(uint32_t i = 0; i < blocks_number; i++, cur_edge_addr += sizeof(Edge)) {
		uint8_t* param_addr = (uint8_t*)cur_edge_addr;
		
		edges[i] = _create_edge((uint8_t*)cur_edge_addr);
	}
	
	return edges;
}