#include <stdio.h>
#include <string.h>
#include "db/entities.h"
#include "db/db.h"
#include "client/manage.h"
#include <assert.h>


void _print_tag(Tag tag);
char* _type_as_str(Type type);

int main(int argc, uint8_t** argv) {
	// char* filename = argv[0];
	Database* db = init_database("test_file");
	
	
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