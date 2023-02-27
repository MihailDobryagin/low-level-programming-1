#include "strings.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

char* type_as_str(Type type) {
	switch (type) {
	case BYTE: return "byte";
	case STRING: return "string";
	case NUMBER: return "number";
	case BOOLEAN: return "boolean";
	case CHARACTER: return "character";
	default: assert(0);
	}
}

char* field_as_str(Field field) {
	switch (field.type) {
	case BYTE: return num_as_str(field.byte);
	case NUMBER: return num_as_str(field.byte);
	case STRING: return field.string;
	case BOOLEAN: if (field.boolean) return "true"; else return "false";
	case CHARACTER: return (char[1]) { field.character };
	default:
		printf("BAD FIELD TYPE %s", type_as_str(field.type));
		assert(0);
	}
}

char* num_as_str(int64_t value) {
	
	char* res = (char*)malloc(65);
	if (value == 0) {
		res[0] = '0';
		res[1] = '\0';
		return res;
	}
	bool is_negative = value < 0;
	uint8_t pos = 0;
	if (is_negative) res[pos++] = '-';
	value = abs(value);

	while (value) {
		res[pos++] = '0' + value % 10;
		value /= 10;
	}

	res[pos] = '\0';

	for (int i = 0; i < pos / 2; i++) {
		char tmp = res[is_negative + i];
		res[i] = res[pos - 1 - i];
		res[pos - 1 - i] = tmp;
	}

	return res;
}