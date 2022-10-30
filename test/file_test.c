#include "../src/db/file.h"

void init_storage() {
	FILE* file = fopen("test_file", "wb+");
	Metadata* metadata = (Metadata*)malloc(sizeof(Metadata));
	metadata->blocks_number = 123;
	fwrite(metadata, sizeof(Metadata), 1, file);
	fclose(file);
	
	Storage* storage = init_storage("test_file");
	
	assert(storage->metadata.blocks_number == 123);
}

// void allocate_block() {
	// FILE* file = fopen("test_file", "wb+");
	// Metadata metadata = {
		// .
	// }
	// Storage storage = {
		// .
	// }
// }