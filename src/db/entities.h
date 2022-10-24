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

struct Tag {
	char* name;
	struct Field* fields;
};

struct Node {
	char* tag;
	struct Field id;
	struct Property* properties;
};

struct Edge {
	struct Field node1_id;
	struct Field node2_id;
	struct Property* properties;
};


#endif // !entities_h