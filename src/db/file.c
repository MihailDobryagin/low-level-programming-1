#include "file.h"
#include <stdlib.h>
#include <assert.h>

Storage* init_storage(char* file_name) {
	FILE* file = fopen(file_name, "rb+");
	Metadata* metadata_buff = (Metadata*)malloc(sizeof(Metadata));
	size_t readen_for_metadata = fread(metadata_buff, sizeof(Metadata), 1, file);
	
	Storage* storage = (Storage*)malloc(sizeof(Storage*));
	storage->file = file;
	
	if(readen_for_metadata == 0) {
		printf("Metadata не прочиталась\n");
		Metadata metadata = {
			.blocks_number = 0,
			.headers_offset = sizeof(Metadata),
			.data_offset = 0
		};
		metadata_buff = &metadata;
		fwrite(metadata_buff, sizeof(Metadata), 1, file);
		storage->metadata = metadata;
	}
	else {
		storage->metadata = *metadata_buff;
	}
	
	return storage;
}

Header_block allocate_block(Storage* storage, Header_block header) {
	FILE* file = storage->file;
	Metadata metadata = storage->metadata;
	
	uint32_t headers_offset = metadata.headers_offset;
	uint32_t blocks_number = metadata.blocks_number;
	
	Header_block* current_header = (Header_block*)malloc(sizeof(Header_block));
	
	for(uint32_t i = 0; i < blocks_number; i++) {
		fread(current_header, sizeof(Header_block), 1, file);
		
		
	}
}

void close_storage(Storage* storage) {
	fclose(storage->file);
	free(storage);
}