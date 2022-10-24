#ifndef queries_h
#define queries_h

#include "entities.h"

struct Insert_node {
	struct Node node;
};

struct Insert_edge {
	struct Edge edge;
};

enum Selection_mode {
	IDS,
	FILTER
};

struct Get_nodes {
	Selection_mode selection_mode;
	union {
		struct Field* ids;
		bool (*predicate)(struct Node);
	};
};

struct Get_edges {
	Selection_mode selection_mode;
	union {
		bool (*predicate)(struct Edge);
	};
};

struct Get_edges {
	bool (*predicate)(struct Edge);
};

struct Delete_nodes {
	Selection_mode selection_mode;
	union {
		Field* ids;
		bool (*predicate)(struct Node);
	};
};

struct Delete_edges {
	bool (*predicate)(struct Edge);
};

struct Update_nodes {
	Selection_mode selection_mode;
	union {
		Field* ids;
		bool (*predicate)(struct Node);
	};
	struct Property* new_properties;
};

struct Update_edges {
	bool (*predicate)(struct Edge);
	struct Property* new_properties;
};

#endif // !queries_h