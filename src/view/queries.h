#ifndef queries_h
#define queries_h

#include "entities.h"

enum Node_selection_mode {
	IDS,
	FILTER
	BY_RELATION,
};

struct Insert_node {
	struct Node node;
};

struct Get_nodes {
	Node_selection_mode selection_mode;
	union {
		struct Field* ids;
		bool (*predicate)(struct Node);
		struct {
			boolean (*related_node_predicate)(struct Node);
			char* edge_tag;
		} relation;
	};
};

struct Delete_nodes {
	Node_selection_mode selection_mode;
	union {
		Field* ids;
		bool (*predicate)(struct Node);
		struct {
			boolean (*related_node_predicate)(struct Node);
			char* edge_tag;
		} relation;
	};
};

struct Update_nodes {
	Node_selection_mode selection_mode;
	union {
		Field* ids;
		bool (*predicate)(struct Node);
		struct {
			boolean (*related_node_predicate)(struct Node);
			char* edge_tag;
		} relation;
	};
	struct Property* new_properties;
};

struct Insert_edge {
	struct Edge edge;
};

struct Get_edges {
	bool (*predicate)(struct Edge);
};

struct Delete_edges {
	bool (*predicate)(struct Edge);
};


struct Update_edges {
	bool (*predicate)(struct Edge);
	struct Property* new_properties;
};

#endif // !queries_h