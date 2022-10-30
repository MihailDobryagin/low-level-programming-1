#ifndef local_entities_h
#define local_entities_h

#include <stdio.h>
#include "../view/entities.h"

typedef struct {
	struct Node node;
	int32_t local_id;
} Extended_node;

typedef struct {
	struct Edge edge;
	int32_t local_id;
} Extended_edge;

typedef struct {
	struct Tag tag;
	
} Extended_tag;

typedef struct {
	uint32_t blocks_number;
	uint32_t headers_offset;
	uint64_t data_offset;
} Metadata;

enum Header_type {
	TAG_HEADER,
	NODE_HEADER,
	EDGE_HEADER
};

enum Block_status {
	GOOD,
	EMPTY
};

typedef struct {
	enum Header_type type;
	enum Block_status status;
	uint32_t id;
	uint64_t data_offset;
	uint64_t data_size;
	uint32_t next_block_offset;
} Header_block;

typedef struct {
	FILE* file;
	Metadata metadata;
} Storage;


#endif // !local_entities_h