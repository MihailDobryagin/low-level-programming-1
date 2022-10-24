#ifndef entities_h
#define entities_h

#include <stdint.h>

enum Type {
	BYTE,
	STRING,
	NUMBER, // int32
	BOOLEAN,
	CHARACTER
};

struct Field {
	Type type;
	union {
		int8_t byte;
		char* string;
		int32_t number;
		bool boolean;
		char character;
	};
};

struct Property {
	char* name;
	struct Field field;
};

enum Tag_type {
	NODE,
	EDGE
};

struct Tag {
	Tag_type type;
	char* name; // id
    char** property_names;
};

struct Node {
	char* tag;
	struct Field id;
	struct Property* properties;
};

struct Edge {
	char* tag;
	struct Field node1_id;
	struct Field node2_id;
	struct Property* properties;
};


#endif // !entities_h