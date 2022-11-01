#ifndef local_entities_h
#define local_entities_h

#include <stdio.h>
#include "../view/entities.h"

typedef struct {
	int32_t tag_id;
	Field id;
	uint32_t properties_size;
	Property* properties;
} Extended_node;

typedef struct {
	int32_t tag_id;
	Field id;
	Field node1_id;
	Field node2_id;
	uint32_t properties_size;
	Property* properties;
} Extended_edge;

typedef struct {
	Tag tag;
	int32_t auto_id;
} Extended_tag;


#endif // !local_entities_h