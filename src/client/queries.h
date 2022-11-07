#ifndef queries_h
#define queries_h

#include "../db/entities.h"

typedef enum {
	IDS,
	FILTER,
	BY_RELATION
} Node_selection_mode;

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
} Insert_node;

typedef struct {
	Node_selection_mode selection_mode;
	union {
		Field* ids;
		bool (*predicate)(Node);
		struct {
			bool (*related_node_predicate)(Node);
			char* edge_tag;
		} relation;
	};
} Get_nodes;

typedef struct {
	Node_selection_mode selection_mode;
	union {
		Field* ids;
		bool (*predicate)(Node);
		struct {
			bool (*related_node_predicate)(Node);
			char* edge_tag;
		} relation;
	};
} Delete_nodes;

typedef struct {
	Node_selection_mode selection_mode;
	union {
		Field* ids;
		bool (*predicate)(Node);
		struct {
			bool (*related_node_predicate)(Node);
			char* edge_tag;
		} relation;
	};
	Property* new_properties;
} Update_nodes;

typedef struct {
	Edge edge;
} Insert_edge;

typedef struct {
	bool (*predicate)(Edge);
} Get_edges;

typedef struct {
	bool (*predicate)(Edge);
} Delete_edges;


typedef struct {
	bool (*predicate)(Edge);
	Property* new_properties;
} Update_edges;

#endif // !queries_h