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
static void _test_CRUD_for_edges();
static void _test_expand_and_collapse();

int main(int argc, char** argv) {
	
	_test_expand_and_collapse();

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
	delete_nodes(db, (Select_nodes) { .selection_mode = NODE_IDS, .tag_name = "animals", .target_ids_size = 1, .ids = &getted_nodes.values[0].id });


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

static void _test_CRUD_for_edges() {
	_clear_db_file();
	Database* db = init_database("db_file.txt");
	create_animals_tag(db);
	create_matroskin(db);
	create_sharik(db);
	Array_node getted_nodes = nodes(db, (Select_nodes) { .tag_name = "animals", .selection_mode = NODE_FILTER, .predicate = matroskin_filter });
	Node matroskin = getted_nodes.values[0];
	getted_nodes = nodes(db, (Select_nodes) { .tag_name = "animals", .selection_mode = NODE_FILTER, .predicate = sharik_filter });
	Node sharik = getted_nodes.values[0];
	
	create_friendship(db);
	create_friendship_between_matroskin_and_sharik(db, matroskin, sharik);

	Array_edge getted_edges = edges(db, (Select_edges) { .tag_name = "friendship", .selection_mode = BY_LINKED_NODE, .node_id = matroskin.id });
	Edge actual_friendship_edge = getted_edges.values[0];

	delete_edges(db, (Select_edges) { .tag_name = "friendship", .selection_mode = BY_LINKED_NODE, .node_id = sharik.id });
	getted_edges = edges(db, (Select_edges) { .tag_name = "friendship", .selection_mode = BY_LINKED_NODE, .node_id = matroskin.id });
	
	create_friendship_between_matroskin_and_sharik(db, matroskin, sharik);

	getted_edges = edges(db, (Select_edges) { .tag_name = "friendship", .selection_mode = BY_LINKED_NODE, .node_id = matroskin.id });
	actual_friendship_edge = getted_edges.values[0];

	make_quarrel(db, actual_friendship_edge);
	getted_edges = edges(db, (Select_edges) { .tag_name = "friendship", .selection_mode = BY_LINKED_NODE, .node_id = matroskin.id });
	actual_friendship_edge = getted_edges.values[0];
}

static void _test_expand_and_collapse() {
	_clear_db_file();
	Database* db = init_database("db_file.txt");
	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "a",
		.properties_size = 2,
		.property_types = (Type[2]) {STRING, BYTE},
		.property_names = (char* [2]) { "name", "age" }
	};

	for (int i = 0; i < 9; i++) {
		create_tag(db, (Create_tag) { .tag = tag });
		tag.name = (char[2]){ 'a' + i + 1, '\0'};
	}

	// EXPAND_STORAGE
	create_tag(db, (Create_tag) { .tag = tag });
	tag.name = (char[2]){ 'a' + 11, '\0' };
	// EXPAND_STORAGE
	// Now capacity must be 12 and size -- 10
	// print_tag(tag_info(db, (Get_tag) { .tag_name = "a" }));
	// print_tag(tag_info(db, (Get_tag) { .tag_name = "f" }));

	// Delete 5 blocks
	delete_tag(db, (Delete_tag) { .tag_name = "a" });
	delete_tag(db, (Delete_tag) { .tag_name = "b" });
	delete_tag(db, (Delete_tag) { .tag_name = "c" });
	delete_tag(db, (Delete_tag) { .tag_name = "d" });
	delete_tag(db, (Delete_tag) { .tag_name = "e" });

	print_tag(tag_info(db, (Get_tag) { .tag_name = "f" }));
	print_tag(tag_info(db, (Get_tag) { .tag_name = "g" }));
	print_tag(tag_info(db, (Get_tag) { .tag_name = "h" }));
	print_tag(tag_info(db, (Get_tag) { .tag_name = "i" }));
	print_tag(tag_info(db, (Get_tag) { .tag_name = "j" }));

	// REPEAT COLLAPSING
	delete_tag(db, (Delete_tag) { .tag_name = "f" });
	print_tag(tag_info(db, (Get_tag) { .tag_name = "j" }));
	delete_tag(db, (Delete_tag) { .tag_name = "h" });
	print_tag(tag_info(db, (Get_tag) { .tag_name = "j" }));
	delete_tag(db, (Delete_tag) { .tag_name = "j" });
	///////////////////////////////////

	print_tag(tag_info(db, (Get_tag) { .tag_name = "g" }));
	print_tag(tag_info(db, (Get_tag) { .tag_name = "i" }));


}

static void _clear_db_file() {
	fclose(fopen("db_file.txt", "w"));
}
