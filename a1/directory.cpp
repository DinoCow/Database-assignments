#include "directory.h"

#include <cassert>
/**
 * Initializes a directory using the given page size
 */
void init_directory_page(Directory *directory, int page_size, int offset){
	// sizeof(int) is the size of next_directory
	int capacity = (page_size - sizeof(int) * 2) / sizeof(Entry);
	assert (capacity > 0);
	
	directory->data = new char[page_size]();
	directory->n_entries = (int*) directory->data + sizeof(int);
	*directory->n_entries = 0;
	directory->capacity = capacity;
	directory->next_directory = (int*) directory->data;
	directory->entries = (Entry*) directory->data + sizeof(int) * 2;
	directory->offset = offset;
}

/**
 * Initializes a directory using the given page size and data
 */
void init_directory_page(Directory *directory, int page_size, int offset, char *data){
		// sizeof(int) is the size of next_directory
	int capacity = (page_size - sizeof(int) * 2) / sizeof(Entry);
	assert (capacity > 0);

	directory->data = data;
	directory->n_entries = (int*) directory->data + sizeof(int);
	directory->capacity = capacity;
	directory->next_directory = (int*) directory->data;
	directory->entries = (Entry*) directory->data + sizeof(int) * 2;
	directory->offset = offset;
}

bool directory_is_full(Directory* directory){
	return (*directory->n_entries == directory->capacity);
}

/**
 * Append an empty entry to directory and return its pointer
 */
Entry* next_entry(Directory* directory){
	if (directory_is_full(directory)){
		return NULL;
	}
	directory->n_entries++;
	return &directory->entries[*directory->n_entries];
}

/**
 * Find a page in the directory with some free space
 */
Entry* find_free_page(Directory* directory){
	for(int i=0; i<*directory->n_entries; i++){
		if (directory->entries[i].free_space > 0) {
			return &directory->entries[i];
		}	
	}
	return NULL;
}