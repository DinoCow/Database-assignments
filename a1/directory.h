#ifndef DIRECTORY_H
#define DIRECTORY_H

typedef struct{
	int offset;
	int free_space;
} Entry;

typedef struct{
	int capacity;
	int* n_entries;
	int* next_directory;
	Entry* entries;
	void* data;
	int offset;
} Directory;

/**
 * Initializes a directory using the given page size
 */
void init_directory_page(Directory *directory, int page_size, int offset);

/**
 * Initializes a directory using the given page size and data
 */
void init_directory_page(Directory *directory, int page_size, int offset, char *data);

/**
 * Check if the directory is full
 */
bool directory_is_full(Directory *directory);

/**
 * Append an empty entry to directory and return its pointer
 */
Entry* next_entry(Directory *directory);

/**
 * Find a page in the directory with some free space
 */
Entry* find_free_page(Directory *directory);

#endif