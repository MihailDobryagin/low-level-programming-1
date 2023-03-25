#ifndef queries_h
#define queries_h

#include "../db/entities.h"

typedef enum {
	ALL_NODES,
	NODE_IDS,
	NODES_BY_LINKED_NODE,
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

typedef enum {
	HARDCODED_FILTER,
	PROPERTY_FILTER
} Filter_type;

typedef enum {
	EQ,
	LESS,
	GREATER,
	L_EQ,
	GT_EQ
} Property_filter_type;

typedef struct {
	uint32_t properties_size;
	Property_filter_type* types;
	Property* values_to_compare;
} Properties_filter;

typedef struct {
	Filter_type type;
	union {
		Properties_filter properties_filter;
		bool (*hardcoded_predicate)(Node);
	};
} Filter_container;

typedef struct {
	Node_selection_mode selection_mode;
	struct {
		bool has_filter;
		Filter_container container;
	} filter;
	char* tag_name;
	union {
		struct {
			uint32_t target_ids_size;
			Field* ids;
			Field linked_node_id;
		};
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