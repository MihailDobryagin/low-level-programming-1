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
static void _test_get_metrics();

int main(int argc, char** argv) {
	//_test_insert_metrics();
	_test_get_metrics();
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
	char* long_long_property_of_tag_name = (char*)malloc((1 << (20)) + 1); // 10 Mb
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

	for (size_t i = 0; i < 10000; i++) {
		if (i % 1000 == 0) {
			printf("%d\n", i);
		}
		char* tag_name = num_as_str(i);
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
		free(tag_name);
		free(time_diff_as_str);
	}

	fclose(insert_metrics_file);
	close_database(db);
}

static void _test_get_metrics() {
	FILE* get_metrics_file = fopen("get_metrics.txt", "w+");
	Database* db = init_database("db_file.txt");
	
	Tag tag;
	struct timespec start_time, finish_time;
	clock_gettime(CLOCK_REALTIME, &start_time);
	tag= tag_info(db, (Get_tag) { .tag_name = "9999" });
	clock_gettime(CLOCK_REALTIME, &finish_time);
	print_tag(tag);
	printf("\n\n\n\n");

	free_tag_internal(tag);

	int64_t time_diff = _calc_time_diff(start_time, finish_time);
	if (time_diff < 0) {
		printf("FINISH TIME < START_TIME");
		return;
	}
	char* time_diff_as_str = num_as_str(time_diff);
	fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, get_metrics_file);
	fwrite("\n", 1, 1, get_metrics_file);
	free(time_diff_as_str);

	///////////////////////////////////////////////////////
	// second search
	///////////////////////////////////////////////////////
	
	clock_gettime(CLOCK_REALTIME, &start_time);
	tag = tag_info(db, (Get_tag) { .tag_name = "5000" });
	clock_gettime(CLOCK_REALTIME, &finish_time);
	print_tag(tag);
	printf("\n\n\n\n");

	free_tag_internal(tag);

	time_diff = _calc_time_diff(start_time, finish_time);
	if (time_diff < 0) {
		printf("FINISH TIME < START_TIME");
		return;
	}
	time_diff_as_str = num_as_str(time_diff);
	fwrite(time_diff_as_str, strlen(time_diff_as_str), 1, get_metrics_file);
	fwrite("\n", 1, 1, get_metrics_file);

	fclose(get_metrics_file);
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
