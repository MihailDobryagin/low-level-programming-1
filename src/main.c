#include <stdio.h>
#include <string.h>
#include "db/entities.h"
#include "db/db.h"
#include "client/manage.h"
#include "utils/std_out.h"
#include "utils/strings.h"
#include "test_utils/test_objects_creation.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

static int64_t _calc_time_diff(struct timespec start, struct timespec finish);
static void _clear_db_file();
static void _test_CRUD_for_nodes();
static void _test_CRUD_for_edges();
static void _test_expand_and_collapse();
static void _test_insert_metrics();
static void _test_all_metrics();
static void _test_update_metrics();
static void _test_delete_edges_after_node_deletion();
static void _test_nodes_linking();

int main(int argc, char** argv) {
	//_test_insert_metrics();
	//_test_all_metrics();
	//_test_update_metrics();
	//_test_delete_edges_after_node_deletion();
	_test_nodes_linking();
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


	Select_nodes get_nodes_query = { .selection_mode = ALL_NODES, .tag_name = "animals", .filter.has_filter = false};

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
	delete_nodes(db, (Select_nodes) { .selection_mode = NODE_IDS, .tag_name = "animals", .target_ids_size = 1, .ids = &getted_nodes.values[0].id, .filter.has_filter = false });


	///////////////////////////////////////////////////////
	// Get all <animals>
	///////////////////////////////////////////////////////

	printf("\n\nGetting all animals again...");
	get_nodes_query = (Select_nodes){ .selection_mode = ALL_NODES, .tag_name = "animals", .filter.has_filter = false};

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
	Array_node getted_nodes = nodes(db, (Select_nodes) { 
		.tag_name = "animals", .selection_mode = ALL_NODES, .filter.has_filter = true, 
		.filter.container = (Filter_container){ .type = HARDCODED_FILTER, .hardcoded_predicate = matroskin_filter }
	});
	Node matroskin = getted_nodes.values[0];
	getted_nodes = nodes(db, (Select_nodes) { 
		.tag_name = "animals", .selection_mode = ALL_NODES, .filter.has_filter = true, 
		.filter.container = (Filter_container){ .type = HARDCODED_FILTER, .hardcoded_predicate = sharik_filter }
	});
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

	close_database(db);
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

	close_database(db);
}

static void _test_insert_metrics() {
	FILE* insert_metrics_file = fopen("insert_metrics.txt", "w+");
	_clear_db_file();
	Database* db = init_database("db_file.txt");
	char* long_long_property_of_tag_name = (char*)malloc((1 << (20)) + 1); // 1 Mb
	for (uint32_t i = 0; i < (1 << 20); i++) {
		long_long_property_of_tag_name[i] = 'a' + (i % 26);
	}
	long_long_property_of_tag_name[(1 << 20) + 1] = '\0';

	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "", // INIT LATER
		.properties_size = 1,
		.property_types = (Type[1]) {STRING},
		.property_names = (char* [1]) { long_long_property_of_tag_name}
	};
	
	for (uint32_t i = 0; i < 10000; i++) {
		if (i % 1000 == 0) {
			printf("%d\n", i);
		}
		tag.name = num_as_str(i);
		const struct timespec start_time; clock_gettime(CLOCK_REALTIME, &start_time);
		create_tag(db, (Create_tag) { .tag = tag });
		const struct timespec finish_time; clock_gettime(CLOCK_REALTIME, &finish_time);
		const int64_t time_diff = _calc_time_diff(start_time, finish_time);
		if (time_diff < 0) {
			printf("FINISH TIME < START_TIME");
			return;
		}
		char* time_diff_as_str = num_as_str(time_diff);
		fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, insert_metrics_file);
		fwrite("\n", 1, 1, insert_metrics_file);
		free(tag.name);
		free(time_diff_as_str);
	}

	fclose(insert_metrics_file);
	close_database(db);
}

static void _test_all_metrics() {
	const int tags_amount = 1000;
	FILE* insert_metrics_file = fopen("insert_metrics.txt", "w+");
	FILE* get_metrics_file = fopen("get_metrics.txt", "w+");
	FILE* update_metrics_file = fopen("update_metrics.txt", "w+");

	_clear_db_file();
	Database* db = init_database("db_file.txt");

	char* long_long_property_of_tag_name = (char*)malloc((1 << (20)) + 1); // 1 Mb
	for (uint32_t i = 0; i < (1 << 20); i++) {
		long_long_property_of_tag_name[i] = 'a' + (i % 26);
	}
	long_long_property_of_tag_name[(1 << 20) + 1] = '\0';

	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "", // INIT LATER
		.properties_size = 1,
		.property_types = (Type[1]) {STRING},
		.property_names = (char* [1]) { long_long_property_of_tag_name}
	};

	const struct timespec start_time, finish_time;
	int64_t time_diff; char* time_diff_as_str;
	for (int i = 0; i < tags_amount; i++) {
		if (i % 50 == 0) {
			printf("%d\n", i);
		}
		tag.name = num_as_str(i);
		clock_gettime(CLOCK_REALTIME, &start_time);
		create_tag(db, (Create_tag) { .tag = tag });
		clock_gettime(CLOCK_REALTIME, &finish_time);
		time_diff = _calc_time_diff(start_time, finish_time);
		time_diff_as_str = num_as_str(time_diff);
		fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, insert_metrics_file);
		fwrite("\n", 1, 1, insert_metrics_file);
		free(time_diff_as_str);


		//////////////////////////////////////////////////
		// GET
		//////////////////////////////////////////////////
		Get_tag get_query = { .tag_name = tag.name };
		clock_gettime(CLOCK_REALTIME, &start_time);
		Tag getted_tag = tag_info(db, get_query);
		clock_gettime(CLOCK_REALTIME, &finish_time);
		time_diff = _calc_time_diff(start_time, finish_time);
		time_diff_as_str = num_as_str(time_diff);
		fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, get_metrics_file);
		fwrite("\n", 1, 1, get_metrics_file);
		free(time_diff_as_str);

		free_tag_internal(getted_tag);
	}

	fclose(insert_metrics_file);
	fclose(get_metrics_file);

	printf("\n\nStart collecting DELETE metrics\n");
	//////////////////////////////////////////////////
	// DELETE
	//////////////////////////////////////////////////
	FILE* delete_metrics_file = fopen("delete_metrics.txt", "w+");
	for (int i = tags_amount - 1; i >= 0; i--) {
		if (i % 50 == 0) {
			printf("%d\n", i);
		}
		char* tag_name = num_as_str(i);
		clock_gettime(CLOCK_REALTIME, &start_time);
		delete_tag(db, (Delete_tag) { .tag_name = tag_name });
		clock_gettime(CLOCK_REALTIME, &finish_time);
		time_diff = _calc_time_diff(start_time, finish_time);
		time_diff_as_str = num_as_str(time_diff);
		fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, delete_metrics_file);
		fwrite("\n", 1, 1, delete_metrics_file);
		free(time_diff_as_str);
		free(tag_name);
	}
	fclose(delete_metrics_file);

	close_database(db);
}

static void _test_update_metrics() {
	const int nodes_amount = 1000;
	FILE* update_metrics_file = fopen("update_metrics.txt", "w+");
	_clear_db_file();
	Database* db = init_database("db_file.txt");

	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "nodes",
		.properties_size = 1,
		.property_types = (Type[1]) {STRING},
		.property_names = (char* [1]) { "long_str"}
	};
	create_tag(db, (Create_tag) { tag });

	Node node = {
		.tag = "nodes",
		.id = (Field){.type = NUMBER, .number = 0},
		.properties_size = 1,
		.properties = &((Property[1]) { (Property) { .name = "long_str", .field = (Field){.type = STRING, .string = "naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaame"} } })
	};

	for (uint32_t i = 0; i < nodes_amount; i++) {
		if (i % 1000 == 0) {
			printf("%d\n", i);
		}
		node.id.number = i;
		node.properties = &((Property[1]) { (Property) { .name = "long_str", .field = (Field){ .type = STRING, .string = "naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaame" } } });
		create_node(db, (Create_node) { .node = node });

		node.properties[0].field.string = num_as_str(i);
		const struct timespec start_time; clock_gettime(CLOCK_REALTIME, &start_time);
		change_node(db, (Change_node) { .changed_node = node});
		const struct timespec finish_time; clock_gettime(CLOCK_REALTIME, &finish_time);
		const int64_t time_diff = _calc_time_diff(start_time, finish_time);
		char* time_diff_as_str = num_as_str(time_diff);
		fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, update_metrics_file);
		fwrite("\n", 1, 1, update_metrics_file);
		free(node.properties[0].field.string);
		free(time_diff_as_str);
	}

	fclose(update_metrics_file);
	close_database(db);
}

static void _test_delete_edges_after_node_deletion() {
	_clear_db_file();
	Database* db = init_database("db_file.txt");
	create_animals_tag(db);
	create_matroskin(db);
	create_sharik(db);
	Array_node getted_nodes = nodes(db, (Select_nodes) { 
		.tag_name = "animals", .selection_mode = ALL_NODES, .filter.has_filter = true, 
		.filter.container = (Filter_container){ .type = HARDCODED_FILTER, .hardcoded_predicate = matroskin_filter } 
	});
	Node matroskin = getted_nodes.values[0];
	getted_nodes = nodes(db, (Select_nodes) { 
		.tag_name = "animals", .selection_mode = ALL_NODES, .filter.has_filter = true, 
		.filter.container = (Filter_container){ .type = HARDCODED_FILTER, .hardcoded_predicate = sharik_filter } 
	});
	Node sharik = getted_nodes.values[0];

	create_friendship(db);
	create_friendship_between_matroskin_and_sharik(db, matroskin, sharik);

	Array_edge getted_edges = edges(db, (Select_edges) { .tag_name = "friendship", .selection_mode = BY_LINKED_NODE, .node_id = matroskin.id });
	Edge actual_friendship_edge = getted_edges.values[0];

	delete_nodes(db, (Select_nodes) {
		.tag_name = "animals", .selection_mode = ALL_NODES, .filter.has_filter = true,
		.filter.container = (Filter_container){ .type = HARDCODED_FILTER, .hardcoded_predicate = matroskin_filter } 
	});
	getted_nodes = nodes(db, (Select_nodes) { 
		.tag_name = "animals", .selection_mode = ALL_NODES, .filter.has_filter = true,
		.filter.container = (Filter_container){ .type = HARDCODED_FILTER, .hardcoded_predicate = matroskin_filter }
	});

	getted_edges = edges(db, (Select_edges) { .tag_name = "friendship", .selection_mode = BY_LINKED_NODE, .node_id = matroskin.id });
	assert(getted_edges.size == 0);
}

static void _test_nodes_linking() {
	const int nodes_amount = 100;
	FILE* linked_nodes_metrics_file = fopen("linked_nodes_metrics.txt", "w+");
	_clear_db_file();
	Database* db = init_database("db_file.txt");
	Tag tag = {
		.type = NODE_TAG_TYPE,
		.name = "nodes",
		.properties_size = 2,
		.property_types = &((Type[2]) { NUMBER, NUMBER }),
		.property_names = &((char* [3]) { "num1", "num2" })
	};
	Create_tag create_tag_query = { tag };
	create_tag(db, create_tag_query);
	create_tag(db, (Create_tag) { (Tag) { EDGE_TAG_TYPE, "edges", 0, NULL, NULL } });

	Node node_for_create = {
			.tag = "nodes",
			.id = (Field){.type = NUMBER, .number = 0},
			.properties_size = 2,
			.properties = &((Property[2]) {
				{.name = "num1", .field = (Field){.type = NUMBER, .number = 0} },
				{.name = "num2", .field = (Field){.type = NUMBER, .number = 0} }
			})
	};
	
	Node node_for_link_1 = {
			.tag = "nodes",
			.id = (Field){.type = NUMBER, .number = 0},
			.properties_size = 2,
			.properties = &((Property[2]) {
				{.name = "num1", .field = (Field){.type = NUMBER, .number = 0} },
				{.name = "num2", .field = (Field){.type = NUMBER, .number = 0} },
			})
	};
	Node node_for_link_2 = {
			.tag = "nodes",
			.id = (Field){.type = NUMBER, .number = 0},
			.properties_size = 2,
			.properties = &((Property[2]) {
				{.name = "num1", .field = (Field){.type = NUMBER, .number = 0} },
				{.name = "num2", .field = (Field){.type = NUMBER, .number = 0} },
			})
	};

	const struct timespec start_time, finish_time;
	for (int32_t i = 1; i <= nodes_amount; i++) {
		if (i % (nodes_amount / 10) == 0 ) printf("%d\n", i);

		node_for_create.id.number = i;
		node_for_link_1.id.number = -i;
		node_for_link_2.id.number = i * nodes_amount + 1; // BE SAFE BECAUSE OF OVERFLOW

		node_for_create.properties[0].field.number = i;
		if (i % 2) {
			node_for_link_1.properties[0].field.number = -i;
			node_for_link_2.properties[1].field.number = -i;
		}
		else {
			node_for_link_1.properties[1].field.number = -i;
			node_for_link_2.properties[0].field.number = -i;
		}

		create_node(db, (Create_node) { .node = node_for_create });
		create_node(db, (Create_node) { .node = node_for_link_1 });
		create_node(db, (Create_node) { .node = node_for_link_2 });
		link_simple_nodes(db, "edges", node_for_link_1.id.number, i, node_for_link_1.id.number);
		link_simple_nodes(db, "edges", node_for_link_2.id.number, i, node_for_link_2.id.number);

		const Properties_filter main_node_num1_filter = {
			.properties_size = 1,
			.types = &((Property_filter_type[1]) { EQ }),
			.values_to_compare = &((Property[1]) { (Property) { .name = "num1", .field = (Field){.type = NUMBER, .number = i} } })
		};
		Properties_filter one_of_linked_node_num2_filter = {
			.properties_size = 1,
			.types = &((Property_filter_type[1]) { EQ }),
			.values_to_compare = &((Property[1]) { (Property) { .name = "num2", .field = (Field){.type = NUMBER, .number = -i} } }) // correct for i%2 or i%2 == 0 cases
		};

		const Select_nodes select_main_node_query = {
			.tag_name = "nodes",
			.selection_mode = ALL_NODES,
			.filter = {
				.has_filter = true,
				.container = (Filter_container){
					.type = PROPERTY_FILTER, 
					.properties_filter = main_node_num1_filter
				}
			}
		};

		Select_nodes select_linked_nodes_query = {
			.tag_name = "nodes",
			.selection_mode = NODES_BY_LINKED_NODE,
			.linked_node_id = {.type = NUMBER /*.number = <after getting>*/},
			.filter.has_filter = true,
			.filter.container = (Filter_container){.type = PROPERTY_FILTER, .properties_filter = one_of_linked_node_num2_filter}
		};

		clock_gettime(CLOCK_REALTIME, &start_time);
		const Array_node getted_main_nodes = nodes(db, select_main_node_query);
		select_linked_nodes_query.linked_node_id.number = getted_main_nodes.values[0].id.number;
		Array_node getted_linked_nodes = nodes(db, select_linked_nodes_query);
		clock_gettime(CLOCK_REALTIME, &finish_time);
		assert(getted_main_nodes.size == 1);
		assert(getted_linked_nodes.size == 1);
		assert(i % 2 ? getted_linked_nodes.values[0].id.number == -i : getted_linked_nodes.values[0].id.number == i * nodes_amount + 1);
		free_node_internal(getted_main_nodes.values[0]);
		free(getted_main_nodes.values);
		free_node_internal(getted_linked_nodes.values[0]);
		free(getted_linked_nodes.values);

		const int64_t time_diff = _calc_time_diff(start_time, finish_time);
		char* time_diff_as_str = num_as_str(time_diff);
		fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, linked_nodes_metrics_file);
		fwrite("\n", 1, 1, linked_nodes_metrics_file);
		free(time_diff_as_str);
	}

	fclose(linked_nodes_metrics_file);
	close_database(db);
}

static int64_t _calc_time_diff(struct timespec start, struct timespec finish) {
	int64_t seconds_diff = finish.tv_sec - start.tv_sec;
	int64_t nanoseconds_diff = finish.tv_nsec - start.tv_nsec;
	int64_t mks_diff = seconds_diff * 1000000 + nanoseconds_diff / 1000;
	return mks_diff;
}

static void _clear_db_file() {
	fclose(fopen("db_file.txt", "w"));
}
