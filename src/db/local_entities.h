#ifndef local_entities_h
#define local_entities_h

#include "entities.h"

struct Extended_node {
	struct Node node;
	int32_t local_id;
};

struct Extended_edge {
	struct Edge edge;
	int32_t local_id;
};


#endif // !local_entities_h