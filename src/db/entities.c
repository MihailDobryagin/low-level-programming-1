#include "entities.h"
#include <string.h>

bool compare_fields(Field f1, Field f2) {
	if (f1.type != f2.type) return false;
	Type type = f1.type;
	uint32_t f1_sz, f2_sz;
	int32_t comp_res;
	switch (type) {
		case BYTE: return f1.byte == f2.byte;
		case STRING: return strcmp(f1.string, f2.string) == 0;
		case NUMBER: return f1.number == f2.number;
		case BOOLEAN: return f1.boolean == f2.boolean;
		case CHARACTER: return f1.character == f2.character;
	}
}