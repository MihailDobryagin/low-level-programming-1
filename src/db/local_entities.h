#ifndef local_entities_h
#define local_entities_h

#include "entities.h"

struct {
	struct Node node;
	int32_t local_id;
} Extended_node;

struct {
	struct Edge edge;
	int32_t local_id;
} Extended_edge;

struct {
	struct Tag tag;
	
} Extended_tag;

struct {
	uint32_t blocks_number;
	uint32_t headers_offset;
	uint64_t data_offset;
} Metadata;

enum Header_type {
	TAG,
	NODE,
	EDGE
};

enum Block_status {
	GOOD,
	EMPTY
};

struct {
	enum HeaderType type;
	enum Block_status status;
	uint32_t id;
	uint64_t data_offset;
	uint32_t next_block_offset;
} Header_block;

struct {
	FILE* file;
	struct Metadata metadata;
} Storage;


#endif // !local_entities_h