#include "../src/db/file.h"
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

void check_init_storage() {
	FILE* file = fopen("test_file", "wb+");
	Metadata metadata = {1, 2, 3, 4, 5};
	fwrite(&metadata, sizeof(Metadata), 1, file);
	fclose(file);


	Storage* storage = init_storage("test_file");
	
	assert(storage->metadata.blocks_size == 1);
	assert(storage->metadata.blocks_capacity == 2);
	assert(storage->metadata.data_size == 3);
	assert(storage->metadata.headers_offset == 4);
	assert(storage->metadata.data_offset == 5);
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

void main() {
	check_init_storage();
}