#include <stdio.h>
#include <string.h>
#include "db/entities.h"
#include "db/db.h"
#include "client/manage.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

char* _num_as_str(int64_t value);
void _print_tag(Tag tag);
void _print_node(Node node);
char* _type_as_str(Type type);
char* _field_as_str(Field field);

int main(int argc, char** argv) {
	Database* db = init_database("db_file.txt");

	Type property_types[1] = { STRING };
	char* property_names[1] = { "description" };

	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "tag_name",
		.properties_size = 1,
		.property_types = &property_types,
		.property_names = &property_names
	};

	Create_tag create_tag_query = { tag };
	//create_tag(db, create_tag_query);
	Get_tag get_query = { "tag_name" };
	//Tag getted_tag = tag_info(db, get_query);
	//_print_tag(tag);

	Property node_properties[2] = {
		(Property) {
			"name", (Field) { .type = STRING, .string = "bobik" }
		},
		(Property) {
		"age", (Field) { .type = BYTE, .byte = 3 }
		}
	};

	Node node = {
		.tag = "tag_name",
		.id = (Field) {.type = CHARACTER, .character = '$'},
		.properties_size = 2,
		.properties = node_properties,
	};

	Create_node create_node_query = {node};
	create_node(db, create_node_query);

	Get_nodes get_nodes_query = { "tag_name", ALL };

	Array_node getted_nodes = nodes(db, get_nodes_query);

	printf("\n\n");

	printf("Nodes of tag \"tag_name\" size -> %d", getted_nodes.size);
	_print_node(getted_nodes.values[0]);

	close_database(db);
	while (true);
	return 0;
}

void _print_tag(Tag tag) {
	char* type_as_str;
	switch(tag.type) {
		case NODE_TAG_TYPE: type_as_str = "Node"; break;
		case EDGE_TAG_TYPE: type_as_str = "Edge"; break;
		default: assert(0);
	}
	printf("\n\n--------------------TAG--------------------\n");
	printf("type: %s\n", type_as_str);
	printf("name: %s\n", tag.name);
	printf("properties:\n");
	for(uint32_t i = 0; i < tag.properties_size; i++) {
		printf("\t%15s -> %s\n", _type_as_str(tag.property_types[i]), tag.property_names[i]);
	}
	printf("\n\n--------------------TAG--------------------\n");
}

void _print_node(Node node) {
	printf("\n\n--------------------NODE-------------------\n");
	printf("Tag: %s\n", node.tag);
	printf("Id: %s\n", _field_as_str(node.id));
	
	printf("\t ------------------------------\n");
	for(uint32_t i = 0; i < node.properties_size; i++) {
		printf("\t|%15s -> %10s|\n", node.properties[i].name, _field_as_str(node.properties[i].field));
	}
	printf("\t ------------------------------\n");
	printf("\n\n--------------------NODE-------------------\n");
}

void _print_tag_nice(Tag tag) {
	printf(" --------------------TAG--------------------");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf(" -------------------------------------------");
}

char* _num_as_str(int64_t value) {
	bool is_negative = value < 0;
	char* res = (char*)malloc(32);
	uint8_t pos = 0;
	if (is_negative) res[pos++] = '-';
	value = abs(value);

	while (value) {
		res[pos++] = '0' + value % 10;
		value /= 10;
	}

	res[pos++] = '\0';

	for (int i = is_negative; i < (is_negative + pos - 1) / 2; i++) {
		char tmp = res[i];
		res[i] = res[pos - 1 - is_negative];
		res[pos - 1 - is_negative] = tmp;
	}

	return res;
}

char* _type_as_str(Type type) {
	switch(type) {
		case BYTE: return "byte";
		case STRING: return "string";
		case NUMBER: return "number";
		case BOOLEAN: return "boolean";
		case CHARACTER: return "character";
		default: assert(0);
	}
}

char* _field_as_str(Field field) {
	switch (field.type) {
		case BYTE: return _num_as_str(field.byte);
		case NUMBER: return _num_as_str(field.byte);
		case STRING: return field.string;
		case BOOLEAN: if (field.boolean) return "true"; else return "false";
		case CHARACTER: return (char[1]) { field.character };
		default: 
			printf("BAD FIELD TYPE %s", _type_as_str(field.type));
			assert(0);
	}
}