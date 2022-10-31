#ifndef local_entities_h
#define local_entities_h

#include <stdio.h>
#include "../view/entities.h"

typedef struct {
	Node node;
	int32_t local_id;
} Extended_node;

typedef struct {
	Edge edge;
	int32_t local_id;
} Extended_edge;

typedef struct {
	Tag tag;
	
} Extended_tag;

typedef struct {
	FILE* file;
	Metadata metadata;
} Storage;


#endif // !local_entities_h