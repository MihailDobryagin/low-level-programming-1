#include "entities.h"

bool compare_fields(Field f1, Field f2) {
	if (f1.type != f2.type) return false;
	Type type = f1.type;
	switch (type) {
		case BYTE: return f1.byte == f2.byte;
		case STRING: return strcmp(f1.string, f2.string);
		case NUMBER: return f1.number == f2.number;
		case BOOLEAN: return f1.boolean == f2.boolean;
		case CHARACTER: return f1.character == f2.character;
	}
}