#ifndef file_h
#define file_h

#include "local_entities.h"

typedef struct {
	uint32_t blocks_number;
	uint32_t headers_offset;
	uint64_t data_offset;
} Metadata;

typedef enum {
	TAG_HEADER,
	NODE_HEADER,
	EDGE_HEADER
} Header_type;

typedef enum {
	GOOD,
	EMPTY
} Block_status;

typedef struct {
	Header_type type;
	Block_status status;
	uint32_t id;
	uint64_t data_offset;
	uint64_t data_size;
	uint32_t next_block_offset;
} Header_block;

typedef struct {
	Type
} Data_block;

Storage* init_storage(char* file_name);

Header_block add(Storage* storage, Header_block header, );

void add(Storage* storage, Header_block* header);

void collapse(Storage* storage);

void close_storage(Storage* storage);

#endif // !file_h