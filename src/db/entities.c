#include "entities.h"
#include <string.h>

static void _free_property_internal(Property property);
static void _free_field_internal(Field field);

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
	_free_field_internal(node.id);
	for (uint32_t i = 0; i < node.properties_size; i++) {
		_free_property_internal(node.properties[i]);
	}
	free(node.properties);
}

void free_edge_internal(Edge edge) {
	free(edge.tag);
	_free_field_internal(edge.id);
	_free_field_internal(edge.node1_id);
	_free_field_internal(edge.node2_id);
	for (uint32_t i = 0; i < edge.properties_size; i++) {
		_free_property_internal(edge.properties[i]);
	}
	free(edge.properties);
}

static void _free_property_internal(Property property) {
	free(property.name);
	_free_field_internal(property.field);
}

static void _free_field_internal(Field field) {
	if (field.type == STRING) free(field.string);
}

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