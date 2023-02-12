#ifndef test_objects_creation_h
#define test_objects_creation_h

#include "../db/db.h"

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