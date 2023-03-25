#ifndef entities_h
#define entities_h

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	BYTE,
	STRING,
	NUMBER, // int32
	BOOLEAN,
	CHARACTER
} Type;

typedef enum {
	BIDIRECTIONAL,
	UNIDIRECTIONAL
} Edge_type;
	
typedef struct {
	Type type;
	union {
		int8_t byte;
		char* string;
		int32_t number;
		bool boolean;
		char character;
	};
} Field;

typedef struct {
	char* name;
	Field field;
} Property;

typedef enum {
	NODE_TAG_TYPE,
	EDGE_TAG_TYPE
} Tag_type;

typedef struct {
	Tag_type type;
	char* name; // id
	uint32_t properties_size;
	Type* property_types;
    char** property_names;
} Tag;

typedef struct {
	char* tag;
	Field id;
	uint32_t properties_size;
	Property* properties;
} Node;

typedef struct {
	char* tag;
	Field id;
	Field node1_id;
	Field node2_id;
	uint32_t properties_size;
	Property* properties;
} Edge;

typedef struct {
	uint32_t size;
	Node* values;
} Array_node;

typedef struct {
	uint32_t size;
	Edge* values;
} Array_edge;

void free_tag_internal(Tag tag);
void free_node_internal(Node node);
void free_edge_internal(Edge edge);
void free_field_internal(Field field);
Field copy_field(Field field);
bool compare_fields(Field f1, Field f2);
int8_t force_compare_fields(Field f1, Field f2);

#endif // !entities_h