#include "test_objects_creation.h"

#include <stdio.h>
#include "../db/entities.h"
#include "../client/manage.h"

void create_animals_tag(Database* db) {
	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "animals",
		.properties_size = 1,
		.property_types = &((Type[1]) { BOOLEAN }),
		.property_names = &((char* [1]) { "is_alive" })
	};

	Create_tag create_tag_query = { tag };
	create_tag(db, create_tag_query);
}

void create_sharik(Database* db) {
	Property node_properties[3] = {
		(Property) {
			"name", (Field) { .type = STRING, .string = "Sharik" }
		},
		(Property) {
			"type", (Field) { .type = STRING, .string = "dog" }
		},
		(Property) {
			"age", (Field) { .type = BYTE, .byte = 3 }
		}
	};

	Node node = {
		.tag = "animals",
		.id = (Field) {.type = CHARACTER, .character = '$'},
		.properties_size = 2,
		.properties = node_properties,
	};

	Create_node create_node_query = { node };
	create_node(db, create_node_query);
}

void create_matroskin(Database* db) {
	Property node_properties[3] = {
		(Property) {
			"name", (Field) { .type = STRING, .string = "Matroskin" }
		},
		(Property) {
			"type", (Field) { .type = STRING, .string = "cat" }
		},
		(Property) {
			"age", (Field) { .type = BYTE, .byte = 5 }
		}
	};

	Node node = {
		.tag = "animals",
		.id = (Field) {.type = STRING, .string = "||"},
		.properties_size = 3,
		.properties = node_properties,
	};

	Create_node create_node_query = { node };
	create_node(db, create_node_query);
}