#include "directory.h"

#include <cassert>
#include <cstdio>
/**
 * Initializes a directory using the given page size
 */
void init_directory_page(Directory *directory, int page_size, int offset){
	// sizeof(int) is the size of next_directory
	int capacity = (page_size - sizeof(int) * 2) / sizeof(Entry);
	assert (capacity > 0);
	
	directory->data = new char[page_size]();
	directory->n_entries = (int*) ((char *) directory->data + sizeof(int));
	*directory->n_entries = 0;
	directory->capacity = capacity;
	directory->next_directory = (int*) directory->data;
	directory->entries = (Entry*) ((char *) directory->data + sizeof(int) * 2);
	//fprintf(stdout, "INIT DIRECTORY data:%p\n",directory->data);
	//fprintf(stdout, "INIT DIRECTORY entry:%p\n",directory->entries);
	directory->offset = offset;
}

bool is_full(Directory* directory){
	fprintf(stderr, "is full %d/%d\n",*directory->n_entries, directory->capacity);
		
	return (*directory->n_entries == directory->capacity);
}

/**
 * Append an empty entry to directory and return its index
 */
int next_vacant_slot(Directory* directory){
	assert(!is_full(directory));
	(*directory->n_entries)++;
	return *directory->n_entries-1;
}

/**
 * Find a page in the directory with some free space
 */
int find_free_page(Directory* directory){
	for(int i=0; i<*directory->n_entries; i++){
		if (directory->entries[i].free_space > 0) {
			fprintf(stderr, "find_free_page dir->entries[%d].free_space=%d\n",i, directory->entries[i].free_space);
		
			return i;
		}
	}
	return -1;
}