#include <stdio.h>
#include <string.h>
#include "db/entities.h"
#include "db/db.h"
#include "client/manage.h"
#include "utils/std_out.h"
#include "test_utils/test_objects_creation.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static void _clear_db_file();
static void _test_CRUD_for_nodes();

int main(int argc, char** argv) {
	
	_test_CRUD_for_nodes();

	return 0;
}

static void _test_CRUD_for_nodes() {
	_clear_db_file();
	Database* db = init_database("db_file.txt");

	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "tag_name",
		.properties_size = 1,
		.property_types = &((Type[1]) { STRING }),
		.property_names = &((char* [1]) { "description" })
	};
	Create_tag create_tag_query = { tag };
	create_tag(db, create_tag_query);

	create_animals_tag(db);

	Get_tag get_query = { "tag_name" };
	Tag getted_tag = tag_info(db, get_query);
	print_tag(getted_tag);
	get_query = (Get_tag){ "animals" };
	getted_tag = tag_info(db, get_query);
	print_tag(getted_tag);

	create_sharik(db);
	create_matroskin(db);

	///////////////////////////////////////////////////////
	// Get all <animals>
	///////////////////////////////////////////////////////


	Select_nodes get_nodes_query = { .selection_mode = ALL_NODES, .tag_name = "animals"};

	Array_node getted_nodes = nodes(db, get_nodes_query);

	printf("\n\n");

	printf("Nodes of tag \"animals\" size -> %d", getted_nodes.size);
	for (size_t i = 0; i < getted_nodes.size; i++) {
		print_node(getted_nodes.values[i]);
	}


	///////////////////////////////////////////////////////
	// Delete Sharik
	///////////////////////////////////////////////////////

	printf("Deleting 'Sharik'...");
	delete_nodes(db, (Select_nodes) { .selection_mode = IDS, .tag_name = "animals", .target_ids_size = 1, .ids = &getted_nodes.values[0].id });


	///////////////////////////////////////////////////////
	// Get all <animals>
	///////////////////////////////////////////////////////

	printf("\n\nGetting all animals again...");
	get_nodes_query = (Select_nodes){ .selection_mode = ALL_NODES, "animals"};

	getted_nodes = nodes(db, get_nodes_query);

	printf("\n\n");

	printf("Nodes of tag \"animals\" size -> %d", getted_nodes.size);
	for (size_t i = 0; i < getted_nodes.size; i++) {
		print_node(getted_nodes.values[i]);
	}

	///////////////////////////////////////////////////////
	// Inserting "Sharik" again
	///////////////////////////////////////////////////////

	printf("\n\nInserting 'Sharik' again...");
	create_sharik(db);

	///////////////////////////////////////////////////////
	// Get all <animals>
	///////////////////////////////////////////////////////

	printf("\n\nGetting all animals again...");

	getted_nodes = nodes(db, get_nodes_query);

	printf("\n\n");

	printf("Nodes of tag \"animals\" size -> %d", getted_nodes.size);
	for (size_t i = 0; i < getted_nodes.size; i++) {
		print_node(getted_nodes.values[i]);
	}

	///////////////////////////////////////////////////////
	// Updating "Matroskin"
	///////////////////////////////////////////////////////

	printf("\n\nUpdating age of 'Matroskin'...");
	grow_matroskin_for_1_age(db);

	///////////////////////////////////////////////////////
	// Get all <animals>
	///////////////////////////////////////////////////////

	printf("\n\nGetting all animals again...");

	getted_nodes = nodes(db, get_nodes_query);

	printf("\n\n");

	printf("Nodes of tag \"animals\" size -> %d", getted_nodes.size);
	for (size_t i = 0; i < getted_nodes.size; i++) {
		print_node(getted_nodes.values[i]);
	}


	close_database(db);
}

static void _clear_db_file() {
	fclose(fopen("db_file.txt", "w"));
}