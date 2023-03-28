#include <stdlib.h>
#include <string.h>
#include "manage.h"
#include "../db/db.h"
#include <assert.h>

typedef struct {
	uint32_t size;
	uint32_t* indexes;
} Indexes;

static Indexes _filter_nodes(Array_node input_nodes, Filter_container filter_container);
static Array_node _nodes_by_linked_node(Database* db, const Field linked_node_id);

void create_tag(Database* db, Create_tag query) {
	insert_tag(db, query.tag);
}

Tag tag_info(Database* db, Get_tag query) {
	return get_tag(db, query.tag_name);
}

void delete_tag(Database* db, Delete_tag query) {
	drop_tag(db, query.tag_name);
}

void create_node(Database* db, Create_node query) {
	return insert_node(db, query.node);
}

Array_node nodes(Database* db, Select_nodes query) {
	Array_node result;
	uint32_t* indexes_of_matched = NULL;
	uint32_t cur_size_of_matched = 0;
	switch (query.selection_mode) {
		case ALL_NODES:
			result = get_nodes(db, query.tag_name);
			indexes_of_matched = (uint32_t*)malloc(sizeof(uint32_t) * result.size);
			for (uint32_t i = 0; i < result.size; i++) indexes_of_matched[cur_size_of_matched++] = i;
			break;
		case NODE_IDS:
			result = get_nodes(db, query.tag_name);
			indexes_of_matched = (uint32_t*)malloc(sizeof(uint32_t) * result.size);
			for (uint32_t i = 0; i < result.size; i++) {
				const Node node = result.values[i];
				for (uint32_t id_idx = 0; id_idx < query.target_ids_size; id_idx++) {
					if (compare_fields(node.id, query.ids[id_idx])) {
						indexes_of_matched[cur_size_of_matched++] = i;
						break;
					}
				}
			}
			break;
		case NODES_BY_LINKED_NODE:
			result = _nodes_by_linked_node(db, query.linked_node_id);
			indexes_of_matched = (uint32_t*)malloc(sizeof(uint32_t) * result.size);
			for (uint32_t i = 0; i < result.size; i++) {
				indexes_of_matched[cur_size_of_matched++] = i;
			}
			break;
		default:
			printf("No available modes for get nodes");
			assert(0);
			break;
	}

	{
		const uint32_t size_of_nodes_for_filter_before_filter = result.size;
		result.size = 0;
		uint32_t cur_index_of_matched_IDX = 0;
		for (uint32_t i = 0; i < size_of_nodes_for_filter_before_filter; i++) {
			while (cur_index_of_matched_IDX != cur_size_of_matched - 1 && indexes_of_matched[cur_index_of_matched_IDX] < i) {
				cur_index_of_matched_IDX++;
			}

			if (indexes_of_matched[cur_index_of_matched_IDX] == i) {
				result.values[result.size++] = result.values[i];
			}
			else {
				free_node_internal(result.values[i]);
			}
		}
	}
	free(indexes_of_matched);

	if (query.filter.has_filter) {
		{
			Indexes indexes_of_matched_as_struct = _filter_nodes(result, query.filter.container);
			cur_size_of_matched = indexes_of_matched_as_struct.size;
			indexes_of_matched = indexes_of_matched_as_struct.indexes;
		}
		const uint32_t size_of_nodes_for_filter_before_filter = result.size;
		result.size = 0;
		uint32_t cur_index_of_matched_IDX = 0;
		for (uint32_t i = 0; i < size_of_nodes_for_filter_before_filter; i++) {
			while (cur_index_of_matched_IDX != cur_size_of_matched - 1 && indexes_of_matched[cur_index_of_matched_IDX] < i) {
				cur_index_of_matched_IDX++;
			}

			if (indexes_of_matched[cur_index_of_matched_IDX] == i) {
				result.values[result.size++] = result.values[i];
			}
			else {
				free_node_internal(result.values[i]);
			}
		}
		free(indexes_of_matched);
	}

	return result;
}

void delete_nodes(Database* db, Select_nodes query) {
	Array_node getted_nodes = nodes(db, query);
	for (int i = 0; i < getted_nodes.size; i++) {
		Node node = getted_nodes.values[i];
		Field node_id = node.id;
		drop_node(db, node.tag, node_id);
		delete_edges(db, (Select_edges) { .selection_mode = BY_LINKED_NODE, .node_id = node_id });
	}
}

void change_node(Database* db, Change_node query) {
	update_node(db, query.changed_node);
}

Array_edge edges(Database* db, Select_edges query) {
	Array_edge edges_for_tag = get_edges(db, query.tag_name);
	Array_edge result = { 0, (Edge*)malloc(sizeof(Edge) * edges_for_tag.size) };

	switch (query.selection_mode) {
		case ALL_NODES: result = edges_for_tag;  break;
		case EDGE_IDS:
			for (uint32_t i = 0; i < edges_for_tag.size; i++) {
				for (uint32_t id_idx = 0; id_idx < query.target_ids_size; id_idx++) {
					if (compare_fields(edges_for_tag.values[i].id, query.ids[id_idx])) {
						result.values[result.size++] = edges_for_tag.values[i];
					}
				}
			}
			break;
		case BY_LINKED_NODE:
			for (uint32_t i = 0; i < edges_for_tag.size; i++) {
				if (compare_fields(edges_for_tag.values[i].node1_id, query.node_id) || compare_fields(edges_for_tag.values[i].node2_id, query.node_id)) {
					result.values[result.size++] = edges_for_tag.values[i];
				}
			}
			break;
		case EDGE_FILTER:
			for (uint32_t i = 0; i < edges_for_tag.size; i++) {
				if (query.predicate(edges_for_tag.values[i])) {
					result.values[result.size++] = edges_for_tag.values[i];
				}
			}
			break;
		default:
			printf("No available modes for get nodes");
			assert(0);
			break;
	}

	return result;
}

void create_edge(Database* db, Create_edge query) {
	insert_edge(db, query.edge);
}

void delete_edges(Database* db, Select_edges query) {
	Array_edge getted_edges = edges(db, query);
	for (int i = 0; i < getted_edges.size; i++) {
		drop_edge(db, getted_edges.values[i].tag, getted_edges.values[i].id);
	}
}

void change_edge(Database* db, Change_edge query) {
	update_edge(db, query.changed_edge);
}

static bool _filter_terminal(Node node, Terminal_property_filter filter) {
	for (uint32_t prop_idx = 0; prop_idx < node.properties_size; prop_idx++) {
		if (strcmp(node.properties[prop_idx].name, filter.value_to_compare.name) == 0) {
			const Field in_node_value = node.properties[prop_idx].field;
			const Field filter_value = filter.value_to_compare.field;
			assert(filter_value.type == in_node_value.type);
			const int8_t force_comparing_result = force_compare_fields(in_node_value, filter_value);
			bool is_suitable = false;
			switch (filter.type) {
				case EQ:
					if (force_comparing_result == 0) return true;
					break;
				case LESS:
					if (force_comparing_result < 0) return true;
					break;
				case GREATER:
					if (force_comparing_result > 0) return true;
					break;
				case L_EQ:
					if (force_comparing_result <= 0) return true;
					break;
				case GT_EQ:
					if (force_comparing_result >= 0) return true;
					break;
				default: assert(0);
			}

			return false;
		}
	}
}

static bool _filter_node_properties(Node node, Properties_filter filter) {
	if (filter.is_terminal) {
		const bool tmp_result = _filter_terminal(node, filter.terminal_filter);
		if (filter.logical_operation_type == NOT_LO_TYPE) 
			return !tmp_result; 
		else 
			return tmp_result;
	}

	Properties_filter* subfilters = filter.subfilters.filters;

	if (filter.logical_operation_type == NOT_LO_TYPE) {
		assert(filter.subfilters.size == 1);
		return !_filter_node_properties(node, subfilters[0]);
	}
	
	for (uint32_t i= 0; i < filter.subfilters.size; i++) {
		const bool cur_result = _filter_node_properties(node, subfilters[i]);
		if ((cur_result == true) == (filter.logical_operation_type == OR_LO_TYPE)) return cur_result;
	}
	return filter.logical_operation_type == AND_LO_TYPE;
}

static Indexes _filter_nodes(Array_node nodes, Filter_container filter_container) {
	uint32_t* matched_indexes = (uint32_t*)malloc(sizeof(uint32_t) * nodes.size);
	uint32_t matched_size = 0;

	for (uint32_t i = 0; i < nodes.size; i++) {
		const Node node = nodes.values[i];
		bool filter_result = false;

		switch (filter_container.type) {
			case HARDCODED_FILTER:
				filter_result = filter_container.hardcoded_predicate(node);
				break;
			case PROPERTY_FILTER:
				filter_result = _filter_node_properties(node, filter_container.properties_filter);
				break;
			default:
				assert(0);
		}

		if (filter_result) matched_indexes[matched_size++] = i;
	}

	return (Indexes) { .size = matched_size, .indexes = matched_indexes };
}

static Array_node _nodes_by_linked_node(Database* db, const Field linked_node_id) {
	const Select_edges linked_edges_query = { .selection_mode = BY_LINKED_NODE, .tag_name = NULL, .node_id = linked_node_id };
	const Array_edge linked_edges = edges(db, linked_edges_query);

	if (linked_edges.size == 0) {
		return (Array_node) { 0, NULL };
	}

	const uint32_t target_nodes_size = linked_edges.size;
	Field* target_nodes_ids = (Field*)malloc(sizeof(Field) * target_nodes_size);
	for (uint32_t i = 0; i < target_nodes_size; i++) {
		Edge edge = linked_edges.values[i];
		if (compare_fields(edge.node1_id, linked_node_id)) {
			target_nodes_ids[i] = copy_field(edge.node2_id);
		}
		else {
			target_nodes_ids[i] = copy_field(edge.node1_id);
		}

		free_edge_internal(edge);
	}

	const Select_nodes nodes_query = {
		.selection_mode = NODE_IDS, .tag_name = NULL, .target_ids_size = target_nodes_size, .ids = target_nodes_ids , .filter.has_filter = false
	};
	Array_node target_nodes = nodes(db, nodes_query);

	for (uint32_t i = 0; i < target_nodes_size; i++) {
		free_field_internal(target_nodes_ids[i]);
	}
	free(target_nodes_ids);

	return target_nodes;
}