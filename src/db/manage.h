#ifndef manage_h
#define manage_h

#include "entities.h"
#include "local_entities.h"
#include "queries.h"

struct Node* nodes(struct Get_nodes query);
struct Node insert_node(struct Insert_node query);
void delete_node(struct Delete_nodes query);
void update_nodes(struct Update_nodes query);

void insert_edge(struct Insert_edge query);
struct Edge* edges(struct Get_edges query);
void delete_edges(struct Delete_edges query);
void update_edges(struct Update_edges query);

#endif // !manage_h