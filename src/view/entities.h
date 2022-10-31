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
	Field node1_id;
	Field node2_id;
	uint32_t properties_size;
	Property* properties;
} Edge;


#endif // !entities_h