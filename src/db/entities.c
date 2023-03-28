#include "entities.h"
#include <string.h>
#include <stdlib.h>

static void _free_property_internal(Property property);

void free_field_internal(Field field) {
	if (field.type == STRING) free(field.string);
}

void free_tag_internal(Tag tag) {
	free(tag.name);
	free(tag.property_types);
	for (uint32_t i = 0; i < tag.properties_size; i++) {
		free(tag.property_names[i]);
	}
	free(tag.property_names);
}

void free_node_internal(Node node) {
	free(node.tag);
	free_field_internal(node.id);
	for (uint32_t i = 0; i < node.properties_size; i++) {
		_free_property_internal(node.properties[i]);
	}
	free(node.properties);
}

void free_edge_internal(Edge edge) {
	free(edge.tag);
	free_field_internal(edge.id);
	free_field_internal(edge.node1_id);
	free_field_internal(edge.node2_id);
	for (uint32_t i = 0; i < edge.properties_size; i++) {
		_free_property_internal(edge.properties[i]);
	}
	free(edge.properties);
}

static void _free_property_internal(Property property) {
	free(property.name);
	free_field_internal(property.field);
}

Field copy_field(Field field) {
	if (field.type == STRING) {
		char* new_string = (char*)malloc(strlen(field.string) + 1);
		strcpy(new_string, field.string);
		return (Field) { .type = STRING, .string = new_string };
	}
	return field;
}

bool compare_fields(Field f1, Field f2) {
	if (f1.type != f2.type) return false;
	Type type = f1.type;
	switch (type) {
		case BYTE: return f1.byte == f2.byte;
		case STRING: return strcmp(f1.string, f2.string) == 0;
		case NUMBER: return f1.number == f2.number;
		case BOOLEAN: return f1.boolean == f2.boolean;
		case CHARACTER: return f1.character == f2.character;
	}
}

int8_t force_compare_fields(Field f1, Field f2) {
	if (f1.type != f2.type) return false;
	Type type = f1.type;
	switch (type) {
		case BYTE: return f1.byte == f2.byte ? 0 : (f1.byte < f2.byte ? -1 : 1);
		case STRING: return strcmp(f1.string, f2.string);
		case NUMBER: return f1.number == f2.number ? 0 : (f1.number < f2.number ? -1 : 1);
		case BOOLEAN: return f1.boolean == f2.boolean ? 0 : (f1.boolean ? 1 : -1);
		case CHARACTER: return f1.character == f2.character ? 0 : (f1.character < f2.character ? -1 : 1);
	}
}