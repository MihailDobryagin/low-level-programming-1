#ifndef manage_h
#define manage_h

#include "entities.h"
#include "local_entities.h"
#include "queries.h"

Node* nodes(Get_nodes query);
Node insert_node(Insert_node query);
void delete_node(Delete_nodes query);
void update_nodes(Update_nodes query);

void insert_edge(Insert_edge query);
Edge* edges(Get_edges query);
void delete_edges(Delete_edges query);
void update_edges(Update_edges query);

#endif // !manage_h