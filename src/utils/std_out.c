#include <stdio.h>
#include "std_out.h"
#include "strings.h"
#include "assert.h"

void print_tag(Tag tag) {
	char* type_as_string;
	switch (tag.type) {
		case NODE_TAG_TYPE: type_as_string = "Node"; break;
		case EDGE_TAG_TYPE: type_as_string = "Edge"; break;
		default: assert(0);
	}

	printf("\n\n++++++++++++++++++++TAG++++++++++++++++++++\n");
	printf("type: %s\n", type_as_string);
	printf("name: %s\n", tag.name);

	printf("properties:\n");
	for (uint32_t i = 0; i < tag.properties_size; i++) {
		printf("\t<%s> : %s\n", type_as_str(tag.property_types[i]), tag.property_names[i]);
	}
	printf("\n\n--------------------TAG--------------------\n");
}

void print_node(Node node) {
	printf("\n\n++++++++++++++++++++NODE+++++++++++++++++++\n");
	printf("Tag: '%s'\n", node.tag);
	printf("Id: %s\n", field_as_str(node.id));

	printf("\t --------------------------------\n");
	for (uint32_t i = 0; i < node.properties_size; i++) {
		printf("\t|%15s -> %10s\t|\n", node.properties[i].name, field_as_str(node.properties[i].field));
	}
	printf("\t --------------------------------\n");
	printf("\n\n--------------------NODE-------------------\n");
}

void print_tag_nice(Tag tag) {
	printf(" --------------------TAG--------------------");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf("|                                           |");
	printf(" -------------------------------------------");
}