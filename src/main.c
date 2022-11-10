#include <stdio.h>
#include <string.h>
#include "db/entities.h"
#include "db/db.h"
#include "client/manage.h"
#include <assert.h>
#include <stdint.h>

void _print_tag(Tag tag);
char* _type_as_str(Type type);

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
	create_tag(db, create_tag_query);
	Get_tag get_query = { "tag_name" };
	Tag getted_tag = tag_info(db, get_query);
	_print_tag(tag);
	close_database(db);
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