#include "file.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

static uint32_t _calc_field_size(Field field);
static uint32_t _calc_property_size(Property field);
static void _put_field(uint8_t* buff, Field field);
static void _put_property(uint8_t* buff, Property prop);
static Field _scan_field(uint8_t** stream);
static Property _scan_property(uint8_t** stream);
typedef struct {
	uint64_t size;
	uint8_t* data;
} Serialized;
static Serialized _serialize_tag(Tag* tag);
static Serialized _serialize_node(Node* node);
static Serialized _serialize_edge(Edge* edge);
static Tag _parse_tag(uint64_t data_size, uint8_t* data);
static Node _parse_node(uint64_t data_size, uint8_t* data);
static Edge _parse_edge(uint64_t data_size, uint8_t* data);
static Tag* _parse_tags(uint32_t blocks_number, uint64_t* sizes, uint8_t* data);
static Node* _parse_nodes(uint32_t blocks_number, uint64_t* sizes, uint8_t* data);
static Edge* _parse_edges(uint32_t blocks_number, uint64_t* sizes, uint8_t* data);
static void _store_entity(Storage* storage, Entity_type type, Serialized* serialized);
static void _update_entity(Storage* storage, uint32_t header_number, Header_block header, Serialized* serialized);
static void _delete_entity(Storage* storage, uint32_t header_number);
static void _update_metadata(Storage* storage);
static void _expand_storage(Storage* storage);
static void _collapse_storage(Storage* storage);
static uint32_t _generate_block_unique_id();
static bool _is_in_entity_ids(uint32_t id, uint32_t size, uint32_t* entity_ids);

static const double LIMIT_COEF_OF_DRAFT_BLOCKS = 0.5;

Storage* init_storage(char* file_name) {
	FILE* file = fopen(file_name, "rb+");
	Metadata* metadata_buff = (Metadata*)malloc(sizeof(Metadata)); // FREE
	size_t readen_for_metadata = fread(metadata_buff, sizeof(Metadata), 1, file);
	
	Storage* storage = (Storage*)malloc(sizeof(Storage));
	storage->file = file;
	if(readen_for_metadata == 0) {
		printf("Metadata was not found\n");
		const uint32_t init_capacity = 10;
		Metadata metadata = {
			.blocks_size = 0,
			.draft_blocks_size = 0,
			.blocks_capacity = init_capacity,
			.data_size = 0,
			.headers_offset = sizeof(Metadata),
			.data_offset = sizeof(Metadata) + sizeof(Header_block) * init_capacity
		};
		free(metadata_buff);
		metadata_buff = &metadata;
		fwrite(metadata_buff, sizeof(Metadata), 1, file);
		storage->metadata = metadata;
	}
	else {
		storage->metadata = *metadata_buff;
		free(metadata_buff);
	}
	
	return storage;
}

void close_storage(Storage* storage) {
	fclose(storage->file);
	free(storage);
}

void add_entity(Storage* storage, Data_to_add* data) {
	Metadata* metadata = &storage->metadata;
	
	assert (metadata->blocks_size < metadata->blocks_capacity);
	
	Serialized serialized; // FREE
	switch(data->type) {
		case TAG_ENTITY: serialized = _serialize_tag(&data->tag); break;
		case NODE_ENTITY: serialized = _serialize_node(&data->node); break;
		case EDGE_ENTITY: serialized = _serialize_edge(&data->edge); break;
		default: assert(0);
	}
	
	_store_entity(storage, data->type, &serialized);
	free(serialized.data);
	
	if(metadata->blocks_size == metadata->blocks_capacity) _expand_storage(storage);
}

Getted_entities get_entities(Storage* storage, Getting_mode mode, Entity_type type, uint32_t start_index, uint32_t number_of_blocks) {
	FILE* file = storage->file;
	Metadata metadata = storage->metadata;
	
	if (mode == ALL) {
		Header_block* headers_buff = (Header_block*)malloc(sizeof(Header_block) * metadata.blocks_size);                       // FREE
		fseek(file, metadata.headers_offset, SEEK_SET);
		fread(headers_buff, sizeof(Header_block), metadata.blocks_size, file);

		uint32_t passed_blocks = 0;
		uint32_t matched_blocks_number = 0;
		uint32_t* matched_blocks = (uint32_t*)malloc(sizeof(uint32_t) * number_of_blocks); // indexes        // FREE
		uint64_t* matched_blocks_sizes = (uint64_t*)malloc(sizeof(uint64_t) * number_of_blocks);             // FREE
		uint32_t* block_ids = (uint32_t*)malloc(sizeof(uint32_t) * number_of_blocks);
		uint64_t matched_data_size = 0;

		for (uint32_t i = 0; i < metadata.blocks_size && matched_blocks_number < number_of_blocks; i++) {
			const Header_block header = headers_buff[i];
			if (header.status != WORKING || header.type != type) continue;
			if (passed_blocks++ < start_index) continue;

			matched_data_size += header.data_size;
			matched_blocks[matched_blocks_number] = i;
			matched_blocks_sizes[matched_blocks_number] = header.data_size;
			block_ids[matched_blocks_number] = header.block_unique_id;
			matched_blocks_number++;
		}

		uint8_t* data_buff = (uint8_t*)malloc(matched_data_size);                                             // FREE
		uint8_t* cur_data_buff_addr = data_buff;

		for (uint32_t i = 0; i < matched_blocks_number; i++) {
			const Header_block header = headers_buff[matched_blocks[i]];
			fseek(file, header.data_offset, SEEK_SET);
			fread(cur_data_buff_addr, header.data_size, 1, file);
			cur_data_buff_addr += header.data_size;
		}
		free(matched_blocks);
		free(headers_buff);

		void* entities;

		switch (type) {
		case TAG_ENTITY:
			entities = _parse_tags(matched_blocks_number, matched_blocks_sizes, data_buff); break;
		case NODE_ENTITY:
			entities = _parse_nodes(matched_blocks_number, matched_blocks_sizes, data_buff); break;
		case EDGE_ENTITY:
			entities = _parse_edges(matched_blocks_number, matched_blocks_sizes, data_buff); break;
		default:
			assert(0);
		}
		free(matched_blocks_sizes);
		free(data_buff);

		
		return (Getted_entities) { .size = matched_blocks_number, .block_ids = block_ids, .entities = (void*)entities };
	}
	else assert(0);
}

void delete_entitites(Storage* storage, uint32_t to_delete_amount, uint32_t* entity_ids) {
	const uint32_t blocks_size = storage->metadata.blocks_size;
	const uint32_t default_headers_buff_size = 20;
	uint32_t amount_of_deleted = 0;
	uint32_t headers_buff_size = default_headers_buff_size;
	Header_block* headers_buff = (Header_block*)malloc(sizeof(Header_block)*headers_buff_size);                               // FREE
	
	fseek(storage->file, storage->metadata.headers_offset, SEEK_SET);
	for(uint32_t i = 0; i * default_headers_buff_size < blocks_size && amount_of_deleted != to_delete_amount; i++) {
		if(headers_buff_size > blocks_size - i * headers_buff_size) headers_buff_size = blocks_size - i * headers_buff_size;
		fread(headers_buff, sizeof(Header_block), headers_buff_size, storage->file);
		
		for(uint32_t buff_idx = 0; buff_idx < headers_buff_size && amount_of_deleted != to_delete_amount; buff_idx++) {
			const Header_block* header = headers_buff + buff_idx;
			if(_is_in_entity_ids(header->block_unique_id, to_delete_amount, entity_ids)) {
				_delete_entity(storage, i * default_headers_buff_size + buff_idx);
				amount_of_deleted++;
			}
		}
	}
	free(headers_buff);

	_collapse_storage(storage);
}

void update_entities(Storage* storage, uint32_t size, uint32_t* entity_ids, Data_to_add* modified_entities) {
	const uint32_t blocks_size = storage->metadata.blocks_size;
	const uint32_t default_headers_buff_size = 20;
	uint32_t amount_of_updated = 0;
	
	uint32_t headers_buff_size = default_headers_buff_size;

	Header_block* headers_buff = (Header_block*)malloc(sizeof(Header_block) * headers_buff_size);                              // FREE
	
	fseek(storage->file, storage->metadata.headers_offset, SEEK_SET);
	for(uint32_t i = 0; i * default_headers_buff_size < blocks_size && amount_of_updated != size; i++) {
		if (headers_buff_size > blocks_size - i * headers_buff_size) headers_buff_size = blocks_size - i * headers_buff_size;
		fread(headers_buff, sizeof(Header_block), headers_buff_size, storage->file);
		
		for (uint32_t buff_idx = 0; buff_idx < headers_buff_size && amount_of_updated != size; buff_idx++) {
			Header_block* header = headers_buff + buff_idx;
			for(uint32_t modified_idx = 0; modified_idx < size; modified_idx++) {
				if(entity_ids[modified_idx] == header->block_unique_id) {
					Serialized serialized;                                                                                     // FREE
					switch(header->type) {
						case NODE_ENTITY: serialized = _serialize_node(&modified_entities[modified_idx].node); break;
						case EDGE_ENTITY: serialized = _serialize_edge(&modified_entities[modified_idx].edge); break;
						default: 
							printf("Entity does not allowed to update\n");
							assert(0);
					}
					_update_entity(storage, i * default_headers_buff_size + buff_idx, *header, &serialized);
					free(serialized.data);
					
					amount_of_updated++;
				}
			}
		}
	}
	free(headers_buff);
	
}

static void _update_entity(Storage* storage, uint32_t header_number, Header_block header, Serialized* serialized) {
	if(serialized->size > header.data_size) {
		_delete_entity(storage, header_number);
		_store_entity(storage, header.type, serialized);
		return;
	}
	
	storage->metadata.data_size += serialized->size - header.data_size;
	header.data_size = serialized->size;
	fseek(storage->file, storage->metadata.headers_offset + sizeof(Header_block) * header_number, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, storage->file);
	fseek(storage->file, header.data_offset, SEEK_SET);
	fwrite(serialized->data, sizeof(uint8_t), serialized->size, storage->file);
	_update_metadata(storage);
}

static void _update_metadata(Storage* storage) {
	fseek(storage->file, 0, SEEK_SET);
	fwrite(&(storage->metadata), sizeof(Metadata), 1, storage->file);
}

static void _expand_storage(Storage* storage) {
	Metadata* metadata = &(storage->metadata);
	
	const uint32_t capacity_diff = metadata->blocks_capacity / 2; // TODO Make dynamic coeff
	const uint32_t new_capacity = metadata->blocks_capacity + capacity_diff; 
	const uint64_t after_target_last_header_addr = metadata->headers_offset + sizeof(Header_block) * new_capacity; // excluding
	
	uint32_t count_of_matching_blocks = 0;
	uint32_t count_of_matching_blocks_cap = 5; // capacity
	uint32_t* blocks_to_move = (uint32_t*)malloc(sizeof(uint32_t) * count_of_matching_blocks_cap); // indexes of headers          // FREE
	
	fseek(storage->file, metadata->headers_offset, SEEK_SET);
	Header_block* const header_buff = (Header_block*)malloc(sizeof(Header_block));                                                // FREE
	for(uint32_t i = 0; i < metadata->blocks_size; i++) {
		fread(header_buff, sizeof(Header_block), 1, storage->file);
		if(header_buff->data_offset < after_target_last_header_addr) {
			if(count_of_matching_blocks == count_of_matching_blocks_cap) {
				count_of_matching_blocks_cap += count_of_matching_blocks_cap / 2; 
				blocks_to_move = (uint32_t*)realloc(blocks_to_move, sizeof(uint32_t) * count_of_matching_blocks_cap);
			}
			blocks_to_move[count_of_matching_blocks++] = i;
		}
	}

	const uint64_t new_data_offset = after_target_last_header_addr;
	uint64_t new_current_data_offset = metadata->data_offset + metadata->data_size;
	uint64_t size_of_moved_data = 0;
	for(uint32_t i = 0; i < count_of_matching_blocks; i++) {
		const uint32_t index_of_block_to_move = blocks_to_move[i];
		const uint32_t header_addr = metadata->headers_offset + sizeof(Header_block) * index_of_block_to_move;
		fseek(storage->file, header_addr, SEEK_SET);
		fread(header_buff, sizeof(Header_block), 1, storage->file);
		size_of_moved_data += header_buff->data_size;
		uint8_t* data = (uint8_t*)malloc(header_buff->data_size);
		fseek(storage->file, header_buff->data_offset, SEEK_SET);
		fread(data, header_buff->data_size, 1, storage->file);
		const uint64_t new_data_addr = new_current_data_offset;
		fseek(storage->file, new_data_addr, SEEK_SET);
		fwrite(data, header_buff->data_size, 1, storage->file);
		header_buff->data_offset = new_data_addr;
		fseek(storage->file, header_addr, SEEK_SET);
		fwrite(header_buff, sizeof(Header_block), 1, storage->file);
		new_current_data_offset += header_buff->data_size;
	}

	free(blocks_to_move);
	free(header_buff);

	const uint64_t new_data_size = metadata->data_size - sizeof(Header_block) * capacity_diff + size_of_moved_data;
	metadata->blocks_capacity = new_capacity;
	metadata->data_offset = new_data_offset;
	metadata->data_size = new_data_size;
	_update_metadata(storage);
}

typedef struct {
	uint32_t idx;
	uint64_t data_offset;
} Header_for_sorting_by_off;

static int comp_for_sorting_headers(const void* p1, const void* p2) {
	Header_for_sorting_by_off* h1 = (Header_for_sorting_by_off*)p1;
	Header_for_sorting_by_off* h2 = (Header_for_sorting_by_off*)p2;

	if (h1->data_offset < h2->data_offset) return -1;
	if (h1->data_offset > h2->data_offset) return 1;
	assert(0);
}

static void _force_collapse(Storage* storage) {
	if (storage->metadata.draft_blocks_size == 0) return;
	Metadata* const metadata = &storage->metadata;
	const uint32_t new_capacity = metadata->blocks_size > 10 ? metadata->blocks_size : 10;
	const uint32_t working_blocks_amount = metadata->blocks_size - metadata->draft_blocks_size;
	const uint64_t new_data_offset = metadata->headers_offset + sizeof(Header_block) * new_capacity;
	// Идём слева и ищем первый попавшийся draft, потом идём справа и ищем первый working. Вставляем w в d
	uint32_t right_idx = metadata->blocks_size; // out of bound
	Header_block* header_buff = (Header_block*)malloc(sizeof(Header_block));                                                   // FREE
	uint64_t new_data_size = 0;
                                                                                                                               // FREE working_data_offs 
	Header_for_sorting_by_off* working_data_offs = (Header_for_sorting_by_off*)malloc(sizeof(Header_for_sorting_by_off) * working_blocks_amount);
	uint64_t* working_data_sizes = (uint64_t*)malloc(sizeof(uint64_t) * working_blocks_amount);                                // FREE

	for (uint32_t working_blocks_at_beginning = 0; working_blocks_at_beginning < working_blocks_amount; working_blocks_at_beginning++) {
		fseek(storage->file, metadata->headers_offset + sizeof(Header_block) * working_blocks_at_beginning, SEEK_SET);
		fread(header_buff, sizeof(Header_block), 1, storage->file);
		if (header_buff->status == DRAFT) {
			do {
				fseek(storage->file, metadata->headers_offset + sizeof(Header_block) * (--right_idx), SEEK_SET);
				fread(header_buff, sizeof(Header_block), 1, storage->file);
			} while (header_buff->status != WORKING);
			fseek(storage->file, metadata->headers_offset + sizeof(Header_block) * working_blocks_at_beginning, SEEK_SET);
			fwrite(header_buff, sizeof(Header_block), 1, storage->file);
		}
		working_data_offs[working_blocks_at_beginning].idx = working_blocks_at_beginning;
		working_data_offs[working_blocks_at_beginning].data_offset = header_buff->data_offset;
		working_data_sizes[working_blocks_at_beginning] = header_buff->data_size;
		new_data_size += header_buff->data_size;
	}

	free(header_buff);

	// move data
	qsort(working_data_offs, working_blocks_amount, sizeof(Header_for_sorting_by_off), comp_for_sorting_headers);
	uint64_t current_block_data_offset = new_data_offset;
	for (uint32_t i = 0; i < working_blocks_amount; current_block_data_offset += working_data_sizes[working_data_offs[i].idx], i++) {
		const uint64_t data_offset = working_data_offs[i].data_offset;
		const uint32_t idx = working_data_offs[i].idx;
		fseek(storage->file, data_offset, SEEK_SET);
		uint8_t* data = malloc(working_data_sizes[idx]);
		fread(data, working_data_sizes[idx], 1, storage->file);
		fseek(storage->file, current_block_data_offset, SEEK_SET);
		fwrite(data, working_data_sizes[idx], 1, storage->file);
		Header_block stub_hb = {};
		// rewrite data_offset
		const uint64_t header_off = metadata->headers_offset + sizeof(Header_block) * idx;
		const uint64_t field_offset = (uint64_t)&stub_hb.data_offset - (uint64_t)&stub_hb;
		fseek(storage->file, header_off + field_offset, SEEK_SET);
		fwrite(&current_block_data_offset, sizeof(uint64_t), 1, storage->file);
	}

	free(working_data_offs);
	free(working_data_sizes);

	metadata->blocks_size = working_blocks_amount;
	metadata->draft_blocks_size= 0;
	metadata->blocks_capacity = new_capacity;
	metadata->data_offset = new_data_offset;
	metadata->data_size = new_data_size;

	_update_metadata(storage);

	int fd = fileno(storage->file);
	const uint64_t new_size_of_file = metadata->data_offset + metadata->data_size;
	ftruncate(fd, new_size_of_file);
}

static void _collapse_storage(Storage* storage) {
	if (storage->metadata.draft_blocks_size >= storage->metadata.blocks_size * LIMIT_COEF_OF_DRAFT_BLOCKS) _force_collapse(storage);
}

static Serialized _serialize_tag(Tag* tag) {
	const uint32_t type_size = sizeof(Tag_type);
	const uint32_t name_size = strlen(tag->name);
	const uint32_t properties_size_size = sizeof(uint32_t);
	const uint32_t property_types_size = sizeof(Type) * tag->properties_size;
	uint32_t property_names_size = 0;
	
	for(uint32_t i = 0; i < tag->properties_size; i++) {
		property_names_size += strlen(tag->property_names[i]) + 1;
	}
	
	const uint32_t data_size = type_size + (name_size + 1) + properties_size_size + property_types_size + property_names_size;
	
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	uint8_t* cur_buff_addr = data_buff;
	
	// Serialize _type
	*(Tag_type*)(cur_buff_addr) = tag->type;
	cur_buff_addr += type_size;
	
	// Serialize _name
	strcpy((char*)cur_buff_addr, tag->name);
	cur_buff_addr += name_size + 1;
	
	// Serialize _properties_size
	*(uint32_t*)(cur_buff_addr) = tag->properties_size;
	cur_buff_addr += properties_size_size;
	
	// Serialize _property_types + _property_names
	uint32_t prev_property_names_size = 0;
	const uint32_t property_names_offset = sizeof(Type) * tag->properties_size;
	for(uint32_t i = 0; i < tag->properties_size; i++) {
		*(Type*)(cur_buff_addr + sizeof(Type) * i) = tag->property_types[i];
		strcpy((char*)cur_buff_addr + property_names_offset + prev_property_names_size, tag->property_names[i]);
		prev_property_names_size += strlen(tag->property_names[i]) + 1;
	}
	cur_buff_addr += property_types_size + property_names_size;
	
	return (Serialized){data_size, data_buff};
}

static Serialized _serialize_node(Node* node) {
	const uint32_t tag_name_size = strlen(node->tag);
	const uint32_t id_size = _calc_field_size(node->id);
	const uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t properties_result_size = 0;
	
	for(uint32_t i = 0; i < node->properties_size; i++) {
		properties_result_size += _calc_property_size(node->properties[i]);
	}
	
	const uint32_t data_size = tag_name_size + 1 + id_size + properties_size_size + properties_result_size;
	
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	uint8_t* cur_buff_addr = data_buff;
	
	// Serialize tag_name
	strcpy((char*)cur_buff_addr, node->tag);
	cur_buff_addr += tag_name_size + 1;
	
	// Serialize id
	_put_field(cur_buff_addr, node->id);
	cur_buff_addr += id_size;
	
	// Serialize _properties_size
	*(uint32_t*)(cur_buff_addr) = node->properties_size;
	cur_buff_addr += properties_size_size;
	
	// Serialize _properties
	for(uint32_t i = 0; i < node->properties_size; i++) {
		_put_property(cur_buff_addr, node->properties[i]);
		cur_buff_addr += _calc_property_size(node->properties[i]);
	}
	
	return (Serialized){data_size, data_buff};
}

static Serialized _serialize_edge(Edge* edge) {
	const uint32_t tag_name_size = strlen(edge->tag);
	const uint32_t id_size = _calc_field_size(edge->id);
	const uint32_t node1_id_size = _calc_field_size(edge->node1_id);
	const uint32_t node2_id_size = _calc_field_size(edge->node2_id);
	const uint32_t properties_size_size = sizeof(uint32_t);
	uint32_t properties_result_size = 0;
	
	for(uint32_t i = 0; i < edge->properties_size; i++) {
		properties_result_size += _calc_property_size(edge->properties[i]);
	}
	
	const uint32_t data_size = (tag_name_size + 1) + id_size + node1_id_size + node2_id_size  + properties_size_size + properties_result_size;
	uint8_t* data_buff = (uint8_t*)malloc(data_size);
	uint8_t* cur_buff_addr = data_buff;
	
	// Serialize tag_name
	strcpy((char*)cur_buff_addr, edge->tag);
	cur_buff_addr += tag_name_size + 1;
	
	// Serialize _id
	_put_field(cur_buff_addr, edge->id);
	cur_buff_addr += id_size;
	
	// Serialize _node_ids
	_put_field(cur_buff_addr, edge->node1_id);
	cur_buff_addr += node1_id_size;
	_put_field(cur_buff_addr, edge->node2_id);
	cur_buff_addr += node2_id_size;
	
	// Serialize _properties_size
	*(uint32_t*)(cur_buff_addr) = edge->properties_size;
	cur_buff_addr += properties_size_size;
	
	// Serialize _properties
	for(uint32_t i = 0; i < edge->properties_size; i++) {
		_put_property(cur_buff_addr, edge->properties[i]);
		cur_buff_addr += _calc_property_size(edge->properties[i]);
	}
	
	return (Serialized){data_size, data_buff};
}

static uint32_t _calc_field_size(Field field) {
	const uint32_t type_size = sizeof(Type);
	uint32_t value_size;
	switch(field.type) {
		case STRING: value_size = strlen(field.string) + 1; break;
		case BYTE: value_size = sizeof(int8_t); break;
		case NUMBER: value_size = sizeof(int32_t); break;
		case BOOLEAN: value_size = sizeof(bool); break;
		case CHARACTER: value_size = sizeof(char); break;
		default: assert(0);
	}

	return type_size + value_size;
}

static uint32_t _calc_property_size(Property prop) {
	return strlen(prop.name) + 1 + _calc_field_size(prop.field);
}

static void _put_field(uint8_t* buff, Field field) {
	*(Type*)buff = field.type;
	uint8_t* val_addr = buff + sizeof(Type);
	switch(field.type) {
		case STRING: strcpy((char*)val_addr, field.string); break;
		case BYTE: *(int8_t*)val_addr = field.byte; break;
		case NUMBER: *(int32_t*)val_addr = field.number; break;
		case BOOLEAN: *val_addr = (uint8_t) field.boolean; break;
		case CHARACTER: *(char*)val_addr = (uint8_t) field.character; break;
		default: assert(0);
	}
}

static void _put_property(uint8_t* buff, Property prop) {
	strcpy((char*)buff, prop.name);
	_put_field(buff + strlen(prop.name) + 1, prop.field);
}

static Field _scan_field(uint8_t** stream) {
	uint8_t* cur_addr = *stream;
	const Type type = *(Type*)cur_addr;
	cur_addr += sizeof(Type);
	
	union {
		int8_t byte;
		char* string;
		int32_t number;
		bool boolean;
		char character;
	} value;
	
	uint32_t string_len;
	Field result;
	switch(type) {
		case BYTE: 
			value.byte = *(uint8_t*)cur_addr; 
			cur_addr += sizeof(int8_t); 
			result = (Field){type, .byte = value.byte};
			break;
		case CHARACTER: 
			value.character = *(char*)cur_addr;
			cur_addr += sizeof(char);
			result = (Field){type, .character = value.character};
			break;
		case BOOLEAN: 
			value.boolean = *(uint8_t*)cur_addr != 0;
			cur_addr += sizeof(uint8_t);
			result = (Field){type, .boolean = value.boolean};
			break;
		case NUMBER: 
			value.number = *(int32_t*)cur_addr;
			cur_addr += sizeof(int32_t);
			result = (Field){type, .number = value.number};
			break;
		case STRING: 
			string_len = strlen((char*)cur_addr);
			value.string = (char*)malloc(string_len + 1);
			strcpy(value.string, (char*)cur_addr);
			value.string[string_len] = '\0';
			cur_addr += string_len + 1;
			result = (Field){type, .string = value.string};
			break;
		default: assert(0);
	}

	*stream = cur_addr;

	return result;
}

static Property _scan_property(uint8_t** stream) {
	uint8_t* cur_addr = *stream;
	const uint32_t name_len = strlen((char*)cur_addr);
	char* name = (char*)malloc(name_len + 1);
	strcpy(name, (char*)cur_addr);
	*stream += name_len + 1;
	Field field = _scan_field(stream);
	return (Property){name, field};
}

static Tag _parse_tag(uint64_t data_size, uint8_t* data) {
	Tag_type type = *((Tag_type*)data);
	data += sizeof(Tag_type);

	uint32_t name_len = strlen((char*)data);
	char* name = (char*)malloc(name_len + 1);
	strcpy(name, (char*)data);
	data += name_len + 1;
	
	uint32_t properties_size = *((uint32_t*)data);
	data += sizeof(uint32_t);
	
	Type* property_types = (Type*)malloc(sizeof(Type) * properties_size);
	for(uint32_t i = 0; i < properties_size; i++) {
		property_types[i] = *((Type*)data);
		data += sizeof(Type);
	}
	
	char** property_names = (char**)malloc(sizeof(char*) * properties_size);
	
	for(uint32_t i = 0; i < properties_size; i++) {
		uint32_t len = strlen((char*)data);
		property_names[i] = (char*)malloc(sizeof(char) * len + 1);
		strcpy(property_names[i], (char*)data);
		data += len + 1;
	}
	
	return (Tag) {type, name, properties_size, property_types, property_names};
}

static Node _parse_node(uint64_t data_size, uint8_t* data) {
	uint32_t tag_name_len = strlen((char*)data);
	char* tag_name = (char*)malloc(tag_name_len);
	strcpy(tag_name, (char*)data);
	data += tag_name_len + 1;
	
	Field id = _scan_field(&data);
	
	uint32_t properties_size = *((uint32_t*)data);
	data += sizeof(uint32_t);
	
	Property* properties = (Property*)malloc(sizeof(Property) * properties_size);
	
	for(int i = 0; i < properties_size; i++) {
		properties[i] = _scan_property(&data);
	}
	
	return (Node) {tag_name, id, properties_size, properties};
}

static Edge _parse_edge(uint64_t data_size, uint8_t* data) {
	uint32_t tag_name_len = strlen((char*)data);
	char* tag_name = (char*)malloc(tag_name_len);
	strcpy(tag_name, (char*)data);
	data += tag_name_len + 1;
	
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

static Tag* _parse_tags(uint32_t blocks_number, uint64_t* sizes, uint8_t* data) {
	Tag* tags = (Tag*)malloc(sizeof(Tag) * blocks_number);
	
	for(uint32_t i = 0; i < blocks_number; data += sizes[i], i++) {
		tags[i] = _parse_tag(sizes[i], (uint8_t*)data);
	}
	
	return tags;
}

static Node* _parse_nodes(uint32_t blocks_number, uint64_t* sizes, uint8_t* data) {
	Node* nodes = (Node*)malloc(sizeof(Node) * blocks_number);
	
	for(uint32_t i = 0; i < blocks_number; data += sizes[i], i++) {
		nodes[i] = _parse_node(sizes[i], data);
	}
	
	return nodes;
}

static Edge* _parse_edges(uint32_t blocks_number, uint64_t* sizes, uint8_t* data) {
	Edge* edges = (Edge*)malloc(sizeof(Edge) * blocks_number);
	
	for(uint32_t i = 0; i < blocks_number; data += sizes[i], i++) {
		edges[i] = _parse_edge(sizes[i], data);
	}
	
	return edges;
}

static void _delete_entity(Storage* storage, uint32_t header_number) {
	const uint32_t header_offset = storage->metadata.headers_offset + sizeof(Header_block) * header_number;
	Header_block* header = (Header_block*)malloc(sizeof(Header_block));                                                        // FREE
	assert(header->status == WORKING);
	fseek(storage->file, header_offset, SEEK_SET);
	fread(header, sizeof(Header_block), 1, storage->file);
	header->status = DRAFT;
	fseek(storage->file, header_offset, SEEK_SET);
	fwrite(header, sizeof(Header_block), 1, storage->file);
	free(header);

	storage->metadata.draft_blocks_size++;
	_update_metadata(storage);
}

static void _store_entity(Storage* storage, Entity_type type, Serialized* serialized) {
	FILE* file = storage->file;
	Metadata* metadata = &storage->metadata;
	const uint32_t header_offset = metadata->headers_offset + sizeof(Header_block) * metadata->blocks_size;
	const uint64_t data_offset = metadata->data_offset + metadata->data_size;
	const uint32_t block_unique_id = _generate_block_unique_id();
	const Header_block header = {block_unique_id, type, WORKING, data_offset, serialized->size};
	fseek(file, header_offset, SEEK_SET);
	fwrite(&header, sizeof(Header_block), 1, file);
	
	fseek(file, data_offset, SEEK_SET);
	fwrite(serialized->data, sizeof(uint8_t), serialized->size, file);
	
	storage->metadata.blocks_size++;
	storage->metadata.data_size += serialized->size;
	_update_metadata(storage);
}

static uint32_t _generate_block_unique_id() {
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return t.tv_nsec;
}

static bool _is_in_entity_ids(uint32_t id, uint32_t size, uint32_t* entity_ids) {
	for(int32_t i = 0; i < size; i++) {
		if(entity_ids[i] == id) return true;
	}
	
	return false;
}