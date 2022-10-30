#include "../src/db/file.h"
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

void check_init_storage() {
	FILE* file = fopen("test_file", "wb+");
	Metadata metadata = {
		.blocks_number = (uint32_t)1,
		.headers_offset = (uint32_t) 2,
		.data_offset = (uint64_t)3
	};
	fwrite(&metadata, sizeof(Metadata), 1, file);
	fclose(file);


	Storage* storage = init_storage("test_file");
	
	assert(storage->metadata.blocks_number == 1);
	assert(storage->metadata.headers_offset == 2);
	assert(storage->metadata.data_offset == 3);
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