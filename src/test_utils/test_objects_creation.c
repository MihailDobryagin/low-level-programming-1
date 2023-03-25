#include "test_objects_creation.h"

#include <stdio.h>
#include "../db/entities.h"
#include "../client/manage.h"

void create_simple_tag(Database* db, char* tag_name) {
	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = tag_name,
		.properties_size = 0,
		.property_types = NULL,
		.property_names = NULL
	};

	Create_tag create_tag_query = { tag };
	create_tag(db, create_tag_query);
}

void create_simple_edge_tag(Database* db, char* tag_name) {
	Tag tag = {
		.type = EDGE_TAG_TYPE,
		.name = tag_name,
		.properties_size = 0,
		.property_types = NULL,
		.property_names = NULL
	};

	Create_tag create_tag_query = { tag };
	create_tag(db, create_tag_query);
}

void create_simple_node(Database* db, char* tag_name, int32_t id) {
	Node node = {
		.tag = tag_name,
		.id = (Field) {.type = NUMBER, .number = id},
		.properties_size = 0,
		.properties = NULL
	};

	Create_node create_node_query = { node };
	create_node(db, create_node_query);
}

void link_simple_nodes(Database* db, char* edge_tag_name, int32_t edge_id, int32_t id1, int32_t id2) {
	Edge edge = {
		.tag = edge_tag_name,
		.id = (Field){.type = NUMBER, .number= edge_id},
		.node1_id = (Field){.type = NUMBER, .number = id1},
		.node2_id = (Field){.type = NUMBER, .number = id2},
		.properties_size = 0,
		.properties = NULL
	};

	create_edge(db, (Create_edge) { edge });
}

void create_animals_tag(Database* db) {
	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "animals",
		.properties_size = 1,
		.property_types = &((Type[3]) { STRING, STRING, BYTE }),
		.property_names = &((char* [3]) { "name", "type", "age" })
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
		.properties_size = 3,
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

void grow_matroskin_for_1_age(Database* db) {
	Property node_properties[3] = {
		(Property) {
			"name", (Field) { .type = STRING, .string = "Matroskin" }
		},
		(Property) {
			"type", (Field) { .type = STRING, .string = "cat" }
		},
		(Property) {
			"age", (Field) { .type = BYTE, .byte = 6 }
		}
	};

	Node node = {
		.tag = "animals",
		.id = (Field) {.type = STRING, .string = "||"},
		.properties_size = 3,
		.properties = node_properties,
	};

	Change_node update_node_query = { node };
	change_node(db, update_node_query);
}

bool matroskin_filter(Node node) {
	Field expected_name_field = { .type = STRING, .string = "Matroskin" };
	for (int i = 0; i < node.properties_size; i++) {
		Property property = node.properties[i];
		if (strcmp(property.name, "name") == 0 && compare_fields(expected_name_field, property.field)) return true;
	}

	return false;
}

bool sharik_filter(Node node) {
	Field expected_name_field = { .type = STRING, .string = "Sharik" };
	for (int i = 0; i < node.properties_size; i++) {
		Property property = node.properties[i];
		if (strcmp(property.name, "name") == 0 && compare_fields(expected_name_field, property.field)) return true;
	}

	return false;
}

void create_friendship(Database* db) {
	Tag friendship_tag = {
		.type = EDGE_ENTITY,
		.name = "friendship",
		.properties_size = 2,
		.property_types = &((Type[2]) { BYTE, BYTE }),
		.property_names = &((char* [3]) { "time", "leve" })
	};
	create_tag(db, (Create_tag) { friendship_tag });
}

void create_friendship_between_matroskin_and_sharik(Database* db, Node matroskin, Node sharik) {
	Property friendship_properties[2] = {
		(Property) {
		.name = "time", .field = (Field){.type = BYTE, .byte = 2}
		},
		(Property) {
		.name = "level", .field = (Field){.type = BYTE, .byte = 10}
		}
	};

	Edge friendship_edge = {
		.tag = "friendship",
		.id = (Field){.type = STRING, .string = "/'-'\\"},
		.node1_id = matroskin.id,
		.node2_id = sharik.id,
		.properties_size = 2,
		.properties = friendship_properties
	};

	create_edge(db, (Create_edge) { friendship_edge });
}

void make_quarrel(Database* db, Edge edge) {
	Property new_friendship_properties[2] = {
		(Property) {
		.name = "time", .field = (Field){.type = BYTE, .byte = 2}
		},
		(Property) {
		.name = "level", .field = (Field){.type = BYTE, .byte = 4}
		}
	};

	edge.properties = new_friendship_properties;

	change_edge(db, (Change_edge) { edge });
}