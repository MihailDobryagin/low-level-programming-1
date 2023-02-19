#ifndef file_h
#define file_h

#include "entities.h"
#include <stdio.h>

typedef struct {
	uint32_t blocks_size; // size of working+draft
	uint32_t draft_blocks_size; // size of draft
	uint32_t blocks_capacity;
	uint64_t data_size; // working+draft
	uint32_t headers_offset;
	uint64_t data_offset;
} Metadata;

typedef struct {
	FILE* file;
	Metadata metadata;
} Storage;

typedef enum {
	TAG_ENTITY,
	NODE_ENTITY,
	EDGE_ENTITY
} Entity_type;

typedef enum {
	WORKING,
	DRAFT,
	EMPTY,  // USELESS NOW
	DELETED // USELESS NOW
} Block_status;

typedef struct {
	uint32_t block_unique_id;
	Entity_type type;
	Block_status status;
	uint64_t data_offset;
	uint64_t data_size;
} Header_block;

typedef struct {
	Tag tag;
} Tag_data;

typedef struct {
	union {
		Tag tag;
		Node node;
		Edge edge;
	};
} Entity;

typedef struct {
	Entity_type type;
	union {
		Tag tag;
		Node node;
		Edge edge;
	};
} Data_to_add;

typedef enum {
	ALL,
	BUFFERED,
	ONE_BY_ONE
} Getting_mode;


typedef struct {
	uint32_t size;
	uint32_t* block_ids;
	void* entities;
} Getted_entities;

Storage* init_storage(char* file_name);

Getted_entities get_entities(Storage* storage, Getting_mode mode, Entity_type type, uint32_t start_index, uint32_t number_of_blocks); // Only WORKING

void add_entity(Storage* storage, Data_to_add* data);

void delete_entitites(Storage* storage, uint32_t to_delete_amount, uint32_t* entity_ids);

void update_entities(Storage* storage, uint32_t size, uint32_t* entity_ids, Data_to_add* modified_entities);

void close_storage(Storage* storage);

#endif // !file_h