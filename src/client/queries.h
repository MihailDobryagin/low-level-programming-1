#ifndef queries_h
#define queries_h

#include "../db/entities.h"

typedef enum {
	ALL_NODES,
	NODE_IDS,
	NODE_FILTER
} Node_selection_mode;

typedef enum {
	ALL_EDGES,
	EDGE_IDS,
	BY_LINKED_NODE,
	EDGE_FILTER
} Edge_selection_mode;

typedef struct {
	Tag tag;
} Create_tag;

typedef struct {
	char* tag_name;
} Delete_tag;

typedef struct {
	char* tag_name;
} Get_tag;

typedef struct {
	Node node;
} Create_node;

typedef struct {
	Node_selection_mode selection_mode;
	char* tag_name;
	union {
		struct {
			uint32_t target_ids_size;
			Field* ids;
		};
		bool (*predicate)(Node);
	};
} Select_nodes;

typedef struct {
	Node changed_node;
} Change_node;

typedef struct {
	Edge edge;
} Create_edge;

typedef struct {
	Edge_selection_mode selection_mode;
	char* tag_name;
	union {
		struct {
			uint32_t target_ids_size;
			Field* ids;
		};
		Field node_id;
		bool (*predicate)(Edge);
	};
} Select_edges;

typedef struct {
	bool (*predicate)(Edge);
} Delete_edges;

typedef struct {
	Edge changed_edge;
} Change_edge;

#endif // !queries_h