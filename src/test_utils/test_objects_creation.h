#ifndef test_objects_creation_h
#define test_objects_creation_h

#include "../db/db.h"

void create_simple_tag(Database* db, char* tag_name);
void create_simple_edge_tag(Database* db, char* tag_name);
void create_simple_node(Database* db, char* tag_name, int32_t id);
void link_simple_nodes(Database* db, char* edge_tag_name, int32_t edge_id, int32_t id1, int32_t id2);

void create_animals_tag(Database* db);
void create_sharik(Database* db);
void create_matroskin(Database* db);
void grow_matroskin_for_1_age(Database* db);
bool matroskin_filter(Node node);
bool sharik_filter(Node node);

void create_friendship(Database* db);
void create_friendship_between_matroskin_and_sharik(Database* db, Node matroskin, Node sharik);
void make_quarrel(Database* db, Edge edge);

#endif //!test_objects_creation_h